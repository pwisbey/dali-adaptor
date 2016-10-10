//todor licence todor copy to ubuntu adaptor

//todor remove def
#define TIZENVR_USE_DYNAMIC_LIBRARY

// CLASS HEADER
#include "vr-engine-impl-tizen.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/math/matrix.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
//todor
#include <iostream>

// INTERNAL INCLUDES
#include <adaptors/common/gl/egl-implementation.h>
#include <adaptors/common/gl/egl-factory.h>
#include <adaptors/common/gl/egl-implementation.h>
#include <adaptors/common/gl/gl-implementation.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

VrEngineTizenVR::VrEngineTizenVR( AdaptorInternalServices& internalServices )
: VrEngine( internalServices )
{
}

VrEngineTizenVR::~VrEngineTizenVR()
{
}

void VrEngineTizenVR::SubmitFrame()
{
  tzvr_submit_params_s params;
  eyePose currentEyePose;
  memset( &currentEyePose, 0, sizeof( eyePose_s ) );

  TzVR_get_current_pose( mTizenVrData->vrContext, &currentEyePose );

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

bool VrEngineTizenVR::GetCurrentEyePose( VrEngineEyePose* eyePose )
{
  eyePose_s tzvrPose;
  if( TzVR_get_current_pose( mTizenVrData->vrContext, &tzvrPose ) )
  {
    return false;
  }

  eyePose->timestamp = tzvrPose.timestamp;
  eyePose->rotation = Quaternion( tzvrPose.w, tzvrPose.y, -tzvrPose.x, tzvrPose.z );
  return true;
}


} // Adaptor

} // Internal

} // Dali
