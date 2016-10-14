//todor licence

// CLASS HEADER
#include "vr-engine-impl-ubuntu.h"

// INTERNAL INCLUDES
#include <adaptors/common/gl/egl-implementation.h>
#include <adaptors/common/gl/egl-factory.h>
#include <adaptors/common/gl/egl-implementation.h>
#include <adaptors/common/gl/gl-implementation.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/math/matrix.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// For GearVR server.
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

// set 1 if you want to read from real GearVR headset, enabling this flag will
// override TzVR_get_current_pose() with custom one which will take a feed from the
// OVR server running on the device.
const bool TIZENVR_USE_OVR_SERVER = true;
const int  OVR_PORT               = 55000;

extern "C"
{
// Ubuntu doesn't support ttrace, so we define these missing functions in case the TizenVR Engine library is built with ttrace.
void traceBegin( uint64_t tag, const char *name, ... ) {}
void traceEnd( uint64_t tag ) {}
void traceAsyncBegin( uint64_t tag, int cookie, const char *name, ... ) {}
void traceAsyncEnd( uint64_t tag, int cookie, const char *name, ... ) {}
void traceMark( uint64_t tag, const char *name, ... ) {}
void traceCounter( uint64_t tag, int value, const char *name, ... ) {}
}

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

//todor ifdef
/**
 * @brief OVR support
 */
struct OvrImpl
{
  OvrImpl()
  : ovrSock( -1 ),
    ovrEnabled( false )
  {
  }

  int           ovrSock;        ///< todor
  bool          ovrEnabled;
  eyePose_s     ovrLastEyePose;
};

VrEngineTizenVR::VrEngineTizenVR( AdaptorInternalServices& internalServices ) :
  VrEngine( internalServices ),
  mOvrImpl( NULL ),
  mUseOvrServer( TIZENVR_USE_OVR_SERVER )
{
  if( mUseOvrServer )
  {
    mOvrImpl = new OvrImpl();
  }
}

VrEngineTizenVR::~VrEngineTizenVR()
{
  // Delete the OvrImpl if it exists.
  delete mOvrImpl;
}

void VrEngineTizenVR::SubmitFrame()
{
  tzvr_submit_params_s params;
  eyePose currentEyePose;
  memset( &currentEyePose, 0, sizeof( eyePose_s ) );

  // The eye values submitted to the TizenVR engine here are for timewarp only.
  // The actual eye pose position is updated on the Head Node automatically.
  if( !mUseOvrServer )
  {
    TzVR_get_current_pose( mTizenVrData->vrContext, &currentEyePose );
  }
  else
  {
    // If using the OVR Server, we do not set a time-warp pose.
    currentEyePose.x = 0.0f;
    currentEyePose.y = 0.0f;
    currentEyePose.z = 0.0f;
    currentEyePose.w = 1.0f;
  }

  Dali::Matrix poseMatrix;
  params.frame_index = mTizenVrData->frameIndex;
  params.min_vsync_wait = 1u;
  params.render_pose[EYE_INDEX_LEFT] = currentEyePose;
  params.render_pose[EYE_INDEX_RIGHT] = currentEyePose;
  params.view = poseMatrix.AsFloat();
  params.chromatic_value = DISABLE_CHROMATIC_ABERRATION;

  TzVR_submit_frame( mTizenVrData->vrContext, &params );

  ++mTizenVrData->frameIndex;
}

bool VrEngineTizenVR::GetCurrentEyePose( Dali::Integration::Vr::VrEngineEyePose* eyePose )
{
  eyePose_s tzvrPose;

  // Read current position.
  if( !mUseOvrServer )
  {
    TzVR_get_current_pose( mTizenVrData->vrContext, &tzvrPose );
  }
  else // Read from the server, if connection enabled
  {
    if( !mOvrImpl->ovrEnabled && !ConnectToOVR( OVR_PORT ) )
    {
      DALI_LOG_ERROR( "OVR: cannot connect to the OVR server\n" );
      return false;
    }
    if( send( mOvrImpl->ovrSock, "rb", 2, 0 ) < 0 )
    {
      mOvrImpl->ovrEnabled = false;
      return false;
    }
    if( recv( mOvrImpl->ovrSock, &tzvrPose, sizeof( eyePose_s ), 0 ) < 0 )
    {
      mOvrImpl->ovrEnabled = false;
      return false;
    }
    mOvrImpl->ovrLastEyePose = tzvrPose;
  }

  eyePose->timestamp = tzvrPose.timestamp;
  eyePose->rotation = Quaternion( tzvrPose.w, tzvrPose.y, -tzvrPose.x, tzvrPose.z );

  return true;
}

bool VrEngineTizenVR::ConnectToOVR( int port )
{
  mOvrImpl->ovrSock = socket( AF_INET, SOCK_STREAM, 0 );
  sockaddr_in caddr;
  socklen_t caddr_len = sizeof( caddr );
  memset( &caddr, 0, sizeof( caddr ) );
  caddr.sin_family = AF_INET;
  caddr.sin_port = htons( port );
  caddr.sin_addr.s_addr = inet_addr( "127.0.0.1" );

  // Connect
  if( connect( mOvrImpl->ovrSock, (sockaddr*)&caddr, caddr_len ) < 0 )
  {
    mOvrImpl->ovrEnabled = false;
    mOvrImpl->ovrSock = -1;
    return false;
  }

  // Send ping to make sure server is up ( with port forwarding it's valid, that
  // connection will open even without server running ).
  char pingChar;
  send( mOvrImpl->ovrSock, "p\n", 2, 0 );
  if( recv( mOvrImpl->ovrSock, &pingChar, 1, 0 ) != 1 )
  {
    shutdown( mOvrImpl->ovrSock, SHUT_RDWR );
    mOvrImpl->ovrEnabled = false;
    mOvrImpl->ovrSock = -1;
    return false;
  }

  mOvrImpl->ovrEnabled = true;
  return true;
}


} // Adaptor

} // Internal

} // Dali
