// CLASS HEADER
#define TIZENVR_USE_DYNAMIC_LIBRARY
#include "vr-engine-impl-tizen.h"
#define GL_DEPTH_COMPONENT24              0x81A6
#ifdef TIZENVR_USE_DYNAMIC_LIBRARY
//#include "vr-engine/inc/Core/tzvr_types.h"
// tzvr types inlined for now
#ifndef __TZVR_TYPES_H__
#define __TZVR_TYPES_H__
#include <tizen.h>
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef void *EGLDisplay;
typedef void *EGLSurface;
typedef void *EGLContext;

#define RAW_QUAT 1
typedef struct _tzvr_vec3f
{
	float x, y, z;
}tzvr_vector3f;

typedef struct tzvr_quat
{
	float w, x, y, z;
}tzvr_quaternionF;

typedef enum {
	EYE_INDEX_LEFT = 0,
	EYE_INDEX_RIGHT = 1,
	EYE_INDEX_MAX = 2,
}tzvr_eye_index_e;

typedef enum {
	TEXTURE_TYPE_2D = 0x0DE1,			/* GL_TEXUTRE_2D*/
	//TEXTURE_TYPE_2D_EXTERNAL = ?,
	TEXTURE_TYPE_2D_ARRAY = 0x8C1A,		/* GL_TEXUTRE_2D_ARRAY*/
	TEXTURE_TYPE_CUBE = 0x8513,			/* GL_TEXUTRE_CUBE_MAP*/
}tzvr_texture_type_e; /* TODO: add more supportable type*/

typedef enum
{
    /* To disable chromatic aberration */
    DISABLE_CHROMATIC_ABERRATION = 0x0,
    /* To enable chromatic aberration */
    ENABLE_CHROMATIC_ABERRATION
}tzvr_chromatic_aberration_e;

typedef struct eyePose {
	float x, y, z;
	float w;	/* For quaternion */
	double timestamp;
	int type;
}eyePose_s;

typedef struct tzvr_submit_params {
	eyePose_s render_pose[EYE_INDEX_MAX];
	long long frame_index;
	float *view;
	unsigned int min_vsync_wait;
	tzvr_chromatic_aberration_e chromatic_value;
}tzvr_submit_params_s;

/* Tizen VR Error enumaration */
typedef enum
{
    /* Successful */
    TZ_VR_SUCCESS = TIZEN_ERROR_NONE,
    /* Out of memory */
    TZ_VR_ERROR_OUT_OF_MEMORY = TIZEN_ERROR_OUT_OF_MEMORY,
    /* Invalid parameter */
    TZ_VR_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER,
    /* Invalid operation */
    TZ_VR_ERROR_INVALID_OPERATION = TIZEN_ERROR_INVALID_OPERATION,

    /* General error */
    TZ_VR_ERROR_GENERAL = TIZEN_ERROR_END_OF_COLLECTION + 0x1
}_tzvr_engine_error_e;

typedef _tzvr_engine_error_e tzvr_ret_type;
typedef void *frame_buffer_handle;

typedef size_t _tzvr_context_ptr; // Handle to hold pointer/address
typedef _tzvr_context_ptr tzvr_context;
#ifdef __cplusplus
}
#endif
#endif /* __TZVR_TYPES_H__ */

#include <dlfcn.h>
#else
#include <vr-engine/inc/Core/tzvr.h>
#endif // TIZENVR_USE_DYNAMIC_LIBRARY

#include <adaptors/common/gl/egl-implementation.h>
#include <adaptors/common/gl/egl-factory.h>
#include <adaptors/common/gl/egl-implementation.h>
#include <adaptors/common/gl/gl-implementation.h>

#include <dali/integration-api/debug.h>
#include <dali/integration-api/vr-engine.h>
#include <dali/public-api/math/matrix.h>

// EXTERNAL
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define VR_BUFFER_WIDTH 1024
#define VR_BUFFER_HEIGHT 1024

#define GL(x) { x; int err = ctx.GetError(); if(err) { DALI_LOG_ERROR( "GL_ERROR: [%d] '%s', %x\n", __LINE__, #x, (unsigned)err);fflush(stderr);fflush(stdout);} else { /*DALI_LOG_ERROR("GL Call: %s\n", #x); fflush(stdout);*/} }

using namespace Dali::Integration;

extern "C"
{
// ubuntu doesn't support ttrace, so defining missing functions
// to please the linker
void traceBegin(uint64_t tag, const char *name, ...) {}
void traceEnd(uint64_t tag) {}
void traceAsyncBegin(uint64_t tag, int cookie, const char *name, ...) {}
void traceAsyncEnd(uint64_t tag, int cookie, const char *name, ...) {}
void traceMark(uint64_t tag, const char *name, ...) {}
void traceCounter(uint64_t tag, int value, const char *name, ...) {}
}

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

struct Impl
{
  Impl()
    : vrContext( 0 ),
      eglContext( 0 ),
      screenWidth( 0 ),
      screenHeight( 0 ),
      eyeBufferCount( 0 ),
      fbWidth( VR_BUFFER_WIDTH ),
      fbHeight( VR_BUFFER_HEIGHT ),
      tzvrFramebufferHandle( 0 ),
      frameIndex( 0 )
  {
  }

  tzvr_context  vrContext;
  EGLContext    eglContext;
  int           screenWidth;
  int           screenHeight;

  int           frameBufferDepth;

  // one per eye
  struct EyeBuffer
  {
    int      fbos[2];
    int      colorTextures[2];
    int      depthTextures[2];
  };

  EyeBuffer*    eyeBuffers;
  int           eyeBufferCount;
  int           fbWidth, fbHeight;
  frame_buffer_handle tzvrFramebufferHandle; // for automatically created texture buffers
  int           frameIndex;

#ifdef TIZENVR_USE_DYNAMIC_LIBRARY

  void*         vrengineLib;

  // TZVR API type definitions
  typedef tzvr_ret_type (*TZVRINITPROC)             (int, int, tzvr_context*);
  typedef tzvr_ret_type (*TZVRDEINITPROC)           (tzvr_context);
  typedef tzvr_ret_type (*TZVRSTARTENGINEPROC)      (tzvr_context, EGLDisplay, EGLContext, EGLSurface);
  typedef tzvr_ret_type (*TZVRSTOPENGINEPROC)       (tzvr_context);
  typedef tzvr_ret_type (*TZVRSUBMITFRAMEPROC)      (tzvr_context, tzvr_submit_params_s*);
  typedef unsigned int  (*TZVRGETTEXTUREIDPROC)     (tzvr_context, frame_buffer_handle, int, int);
  typedef tzvr_ret_type (*TZVRSETTEXTUREIDPROC)     (tzvr_context, frame_buffer_handle, int, int, unsigned int);
  typedef tzvr_ret_type (*TZVRGETCURRENTPOSEPROC)   (tzvr_context, eyePose_s*);
  typedef int (*TZVRGETFRAMEBUFFERDEPTHPROC)        (void);
  typedef frame_buffer_handle (*TZVRCREATETEXTUREBUFFERPROC)(tzvr_context, tzvr_texture_type_e, int, int);
  typedef tzvr_ret_type (*TZVRDESTROYTEXTUREBUFFERPROC)(tzvr_context, frame_buffer_handle);
  typedef tzvr_ret_type (*TZVRRECENTERPOSEPROC)     (tzvr_context);



#endif
};

#ifdef TIZENVR_USE_DYNAMIC_LIBRARY
// tizenvr symbols
static Impl::TZVRINITPROC                  TzVR_init                  ( 0 );
static Impl::TZVRDEINITPROC                TzVR_deinit                ( 0 );
static Impl::TZVRSTARTENGINEPROC           TzVR_start_engine          ( 0 );
static Impl::TZVRSTOPENGINEPROC            TzVR_stop_engine           ( 0 );
static Impl::TZVRSUBMITFRAMEPROC           TzVR_submit_frame          ( 0 );
static Impl::TZVRGETTEXTUREIDPROC          TzVR_get_texture_id        ( 0 );
static Impl::TZVRSETTEXTUREIDPROC          TzVR_set_texture_id        ( 0 );
static Impl::TZVRGETCURRENTPOSEPROC        TzVR_get_current_pose      ( 0 );
static Impl::TZVRGETFRAMEBUFFERDEPTHPROC   TzVR_get_frame_buffer_depth( 0 );
static Impl::TZVRCREATETEXTUREBUFFERPROC   TzVR_create_texture_buffer ( 0 );
static Impl::TZVRDESTROYTEXTUREBUFFERPROC  TzVR_destroy_texture_buffer( 0 );
static Impl::TZVRRECENTERPOSEPROC          TzVR_recenter_pose         ( 0 );
#endif

VrEngineTizenVR::VrEngineTizenVR( AdaptorInternalServices& internalServices ) :
  VrEngine( internalServices )
{
  mImpl = new Impl();

#ifdef TIZENVR_USE_DYNAMIC_LIBRARY
  mImpl->vrengineLib = dlopen( "libvrengine.so", RTLD_NOW|RTLD_GLOBAL );
  DALI_ASSERT_ALWAYS( mImpl->vrengineLib && "Can't load libvrengine.so library!");

  const char* FUNCTION_NAMES[] =
  {
    "TzVR_init",
    "TzVR_deinit",
    "TzVR_start_engine",
    "TzVR_stop_engine",
    "TzVR_submit_frame",
    "TzVR_get_texture_id",
    "TzVR_set_texture_id",
    "TzVR_get_current_pose",
    "TzVR_get_frame_buffer_depth",
    "TzVR_create_texture_buffer",
    "TzVR_destroy_texture_buffer",
    "TzVR_recenter_pose",
  };

  void* FUNCTION_PTRS[] =
  {
    &TzVR_init,
    &TzVR_deinit,
    &TzVR_start_engine,
    &TzVR_stop_engine,
    &TzVR_submit_frame,
    &TzVR_get_texture_id,
    &TzVR_set_texture_id,
    &TzVR_get_current_pose,
    &TzVR_get_frame_buffer_depth,
    &TzVR_create_texture_buffer,
    &TzVR_destroy_texture_buffer,
    &TzVR_recenter_pose,
    NULL
  };

  for( int i = 0; i < 12; ++i )
  {
    (*(void**)FUNCTION_PTRS[i]) = dlsym( mImpl->vrengineLib, FUNCTION_NAMES[i] );
    if( !(*(void**)FUNCTION_PTRS[i]) )
    {
      DALI_LOG_ERROR( "Can't bind function: %s\n", FUNCTION_NAMES[i] );
    }
  }
#endif

}

VrEngineTizenVR::~VrEngineTizenVR()
{

}

bool VrEngineTizenVR::Initialize( VrEngineInitParams* initParams  )
{
  mImpl->screenWidth = initParams->screenWidth;
  mImpl->screenHeight = initParams->screenHeight;
  DALI_ASSERT_ALWAYS( TZ_VR_SUCCESS == TzVR_init( mImpl->screenWidth, mImpl->screenHeight, &mImpl->vrContext ) );
  mImpl->frameBufferDepth = TzVR_get_frame_buffer_depth();
  mImpl->eyeBuffers = new Impl::EyeBuffer[ mImpl->frameBufferDepth ];
  mImpl->eyeBufferCount = mImpl->frameBufferDepth;

  // setup GL objects
  return SetupVREngine( initParams );
}

void VrEngineTizenVR::Start()
{
  // this function must run on GL thread
  EglFactory* eglFactory = static_cast<EglFactory*>( &mAdaptorInternalServices.GetEGLFactoryInterface() );
  if( !eglFactory )
  {
    DALI_LOG_ERROR("Can't initialise VR Engine!");
  }
  EglImplementation* eglImpl = static_cast<EglImplementation*>( eglFactory->GetImplementation() );

  EGLContext ctx = eglImpl->GetContext();
  // start TzVR engine
  TzVR_start_engine( mImpl->vrContext, eglImpl->GetDisplay(), ctx, eglImpl->GetCurrentSurface() );
}

void VrEngineTizenVR::Stop()
{

}

void VrEngineTizenVR::PreRender()
{

}

void VrEngineTizenVR::PostRender()
{

}

void VrEngineTizenVR::SubmitFrame()
{
  tzvr_submit_params_s params;
  eyePose currEyePose;
  memset( &currEyePose, 0, sizeof(eyePose_s) );
  TzVR_get_current_pose( mImpl->vrContext, &currEyePose );

  Dali::Matrix mat;
  params.frame_index = mImpl->frameIndex;
  params.min_vsync_wait = 1;
  params.render_pose[EYE_INDEX_LEFT] = currEyePose;
  params.render_pose[EYE_INDEX_RIGHT] = currEyePose;
  params.view = mat.AsFloat();
  params.chromatic_value = DISABLE_CHROMATIC_ABERRATION;

  TzVR_submit_frame( mImpl->vrContext, &params );

  ++mImpl->frameIndex;
}

bool VrEngineTizenVR::Get( const int property, void* output, int )
{
  switch( property )
  {
    case EYE_BUFFER_COUNT:
      {
      *(reinterpret_cast<int*>(output)) = mImpl->eyeBufferCount;
        return true;
      }
    case EYE_BUFFER_WIDTH:
      {
        *(reinterpret_cast<int*>(output)) = mImpl->fbWidth;
        return true;
      }
    case EYE_BUFFER_HEIGHT:
      {
        *(reinterpret_cast<int*>(output)) = mImpl->fbHeight;
        return true;
      }
    case EYE_LEFT_CURRENT_TEXTURE_ID:
      {
        *(reinterpret_cast<int*>(output)) =
            mImpl->eyeBuffers[(int)(mImpl->frameIndex%mImpl->frameBufferDepth)].colorTextures[0];
        return true;
      }
    case EYE_RIGHT_CURRENT_TEXTURE_ID:
      {
        *(reinterpret_cast<int*>(output)) =
            mImpl->eyeBuffers[(int)(mImpl->frameIndex%mImpl->frameBufferDepth)].colorTextures[1];
        return true;
      }
    case EYE_LEFT_CURRENT_FBO_ID:
      {
        *(reinterpret_cast<int*>(output)) =
            mImpl->eyeBuffers[(int)(mImpl->frameIndex%mImpl->frameBufferDepth)].fbos[0];
        return true;
      }
    case EYE_RIGHT_CURRENT_FBO_ID:
      {
        *(reinterpret_cast<int*>(output)) =
            mImpl->eyeBuffers[(int)(mImpl->frameIndex%mImpl->frameBufferDepth)].fbos[1];
        return true;
      }
    case EYE_CURRENT_POSE:
      {
        VrEngineEyePose* eyePose = reinterpret_cast<VrEngineEyePose*>(output);
        eyePose_s tzvrPose;
        if( TzVR_get_current_pose( mImpl->vrContext, &tzvrPose ) )
          return false;

        eyePose->timestamp = tzvrPose.timestamp;
        eyePose->rotation = Quaternion(
              tzvrPose.w,
              tzvrPose.y,
              -tzvrPose.x,
              tzvrPose.z);
        return true;
      }
    default:
      {
      // do nothing if unrecognized property
      }
  }
  // return eye l/r texture id per particular buffer
  // color textures
  if( property >= EYE_LEFT_TEXTURE0_ID && property < EYE_LEFT_TEXTURE0_ID+64 )
  {
    *(reinterpret_cast<int*>(output)) = mImpl->eyeBuffers[(int)property-EYE_LEFT_TEXTURE0_ID].colorTextures[0];
    return true;
  }
  else if( property >= EYE_RIGHT_TEXTURE0_ID && property < EYE_RIGHT_TEXTURE0_ID+64 )
  {
    *(reinterpret_cast<int*>(output)) = mImpl->eyeBuffers[(int)property-EYE_RIGHT_TEXTURE0_ID].colorTextures[1];
    return true;
  }

  // fbos
  else if( property >= EYE_LEFT_FBO_ID && property < EYE_LEFT_FBO_ID+64 )
  {
    *(reinterpret_cast<int*>(output)) = mImpl->eyeBuffers[(int)property-EYE_LEFT_FBO_ID].fbos[0];
    return true;
  }
  else if( property >= EYE_RIGHT_FBO_ID && property < EYE_RIGHT_FBO_ID+64 )
  {
    *(reinterpret_cast<int*>(output)) = mImpl->eyeBuffers[(int)property-EYE_RIGHT_FBO_ID].fbos[1];
    return true;
  }

  // depth textures
  else if( property >= EYE_LEFT_DEPTH_TEXTURE_ID && property < EYE_LEFT_DEPTH_TEXTURE_ID+64 )
  {
    *(reinterpret_cast<int*>(output)) = mImpl->eyeBuffers[(int)property-EYE_LEFT_DEPTH_TEXTURE_ID].depthTextures[0];
    return true;
  }
  else if( property >= EYE_RIGHT_DEPTH_TEXTURE_ID && property < EYE_RIGHT_DEPTH_TEXTURE_ID+64 )
  {
    *(reinterpret_cast<int*>(output)) = mImpl->eyeBuffers[(int)property-EYE_RIGHT_DEPTH_TEXTURE_ID].depthTextures[1];
    return true;
  }
  return false;
}

bool VrEngineTizenVR::Set( const int property, const void* input, int count )
{
  switch( property )
  {
    case EYE_RENDER_TARGETS:
      {
        const VrEngineRenderTargetInfo* rts = reinterpret_cast<const VrEngineRenderTargetInfo*>( input );
        DALI_ASSERT_ALWAYS( count <= mImpl->eyeBufferCount );
        for( int i = 0; i < count; ++i )
        {
          const VrEngineRenderTargetInfo* rt = &rts[i];
          // fbos
          mImpl->eyeBuffers[i].fbos[0] = rt->fbos[0];
          mImpl->eyeBuffers[i].fbos[1] = rt->fbos[1];

          // color textures
          // check state for color texture
          int lcol = rt->colorTextures[0];
          int rcol = rt->colorTextures[1];
          if( lcol != 0 || rcol != 0 )
          {
            DALI_ASSERT_ALWAYS( "Color textures ID are reserved ( for now )");
          }

          if( !mImpl->tzvrFramebufferHandle )
          {
            mImpl->tzvrFramebufferHandle = TzVR_create_texture_buffer( mImpl->vrContext, TEXTURE_TYPE_2D, mImpl->fbWidth, mImpl->fbHeight );
          }

          // get texture ids
          mImpl->eyeBuffers[i].colorTextures[0] = TzVR_get_texture_id( mImpl->vrContext, mImpl->tzvrFramebufferHandle, i, 0 );
          mImpl->eyeBuffers[i].colorTextures[1] = TzVR_get_texture_id( mImpl->vrContext, mImpl->tzvrFramebufferHandle, i, 1 );

          // depth textures
          mImpl->eyeBuffers[i].depthTextures[0] = rt->depthTextures[0];
          mImpl->eyeBuffers[i].depthTextures[1] = rt->depthTextures[1];
        }
      }
      break;
    case EYE_BUFFER_COUNT:
      {
        // for now should not be settable
      }
      break;
    case EYE_BUFFER_WIDTH:
      {
        mImpl->fbWidth = *reinterpret_cast<const int*>( input );
      }
      break;
    case EYE_BUFFER_HEIGHT:
      {
        mImpl->fbHeight = *reinterpret_cast<const int*>( input );
      }
      break;

    default:
      {
        // do nothing
        return false;
      }
  }
  return true;
}

bool VrEngineTizenVR::SetupVREngine( VrEngineInitParams* params )
{
  GlImplementation* gl = static_cast<GlImplementation*>( &mAdaptorInternalServices.GetGlesInterface() );
  if( !gl )
  {
    return false;
  }

  GlImplementation& ctx = *gl;

  // allocate gl objects, 2 objects per single buffer
  const int totalBufferCount( mImpl->eyeBufferCount*2 );
  uint32_t fbos[totalBufferCount], depthTextures[totalBufferCount];
  GL( ctx.GenFramebuffers( totalBufferCount, fbos ) );
  GL( ctx.GenTextures( totalBufferCount, depthTextures ) );

  // if necessary overwrite fbo sizes
  mImpl->fbWidth = VR_BUFFER_WIDTH;//params->screenWidth;
  mImpl->fbHeight = VR_BUFFER_HEIGHT;//params->screenHeight;

  // set render targets info structure
  VrEngineRenderTargetInfo rtInfo[mImpl->eyeBufferCount];
  for( int i = 0; i < mImpl->eyeBufferCount; ++i )
  {
    rtInfo[i].fbos[0] = fbos[i*2];
    rtInfo[i].fbos[1] = fbos[(i*2)+1];
    rtInfo[i].colorTextures[0] = 0; // if not given by client side, must be 0
    rtInfo[i].colorTextures[1] = 0; // if not given by client side, must be 0
    rtInfo[i].depthTextures[0] = depthTextures[i*2];
    rtInfo[i].depthTextures[1] = depthTextures[(i*2)+1];

    // generate depth buffers
    for( int k = 0; k < 2; ++k )
    {
      GL( ctx.BindTexture( GL_TEXTURE_2D, rtInfo[i].depthTextures[k] ) );
      GL( ctx.TexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
                                 VR_BUFFER_WIDTH,
                                 VR_BUFFER_HEIGHT,
                                 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0 ) );
      GL( ctx.TexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ) );
      GL( ctx.TexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ) );
    }
  }

  // attach render targets
  Set( VrEngine::EYE_RENDER_TARGETS, rtInfo, mImpl->eyeBufferCount );

  // attach textures to the fbos
  int ltex(0), rtex(0);
  for( int i = 0; i < mImpl->eyeBufferCount; ++i )
  {
    // left eye
    VrEngine::Get( VrEngine::EYE_LEFT_TEXTURE_ID+i, &ltex );
    DALI_ASSERT_ALWAYS( true == CreateFramebufferTexture( ctx, rtInfo[i].fbos[0], ltex, rtInfo[i].depthTextures[0] ) );

    // right eye
    VrEngine::Get( VrEngine::EYE_RIGHT_TEXTURE_ID+i, &rtex );
    DALI_ASSERT_ALWAYS( true == CreateFramebufferTexture( ctx, rtInfo[i].fbos[1], rtex, rtInfo[i].depthTextures[1] ) );
  }
  return true;
}

bool VrEngineTizenVR::CreateFramebufferTexture( GlImplementation& ctx, int fbo, int colorTexture, int depthTexture )
{
  GLenum fbStatus;

  GL( ctx.BindFramebuffer( GL_FRAMEBUFFER, fbo ) );
  GL( ctx.FramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0 ) );
  GL( ctx.FramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0 ) );

  GL( fbStatus = ctx.CheckFramebufferStatus( GL_FRAMEBUFFER ) );
  if( fbStatus != GL_FRAMEBUFFER_COMPLETE )
  {
    if( fbStatus == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT )
      DALI_LOG_ERROR("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n");
    else if( fbStatus == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS )
      DALI_LOG_ERROR("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS\n");
    else if( fbStatus == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT )
      DALI_LOG_ERROR("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n");
    else if( fbStatus == GL_FRAMEBUFFER_UNSUPPORTED )
      DALI_LOG_ERROR("GL_FRAMEBUFFER_UNSUPPORTED\n");
    return false;
  }

  return true;
}

} // Adaptor
} // Internal
} // Dali
