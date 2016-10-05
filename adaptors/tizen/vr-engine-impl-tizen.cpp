//todor licence todor copy to ubuntu adaptor

// CLASS HEADER
#define TIZENVR_USE_DYNAMIC_LIBRARY

#include "vr-engine-impl-tizen.h"
#define GL_DEPTH_COMPONENT24              0x81A6

#ifdef TIZENVR_USE_DYNAMIC_LIBRARY

// These are types from the TizenVR Engine.
// This does not adhere to DALi coding standards as is a copy/paste from the TizenVR engine for maintainability.

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
    TZ_VR_SUCCESS = 0,
    /* Out of memory */
    TZ_VR_ERROR_OUT_OF_MEMORY = 1,
    /* Invalid parameter */
    TZ_VR_ERROR_INVALID_PARAMETER = 2,
    /* Invalid operation */
    TZ_VR_ERROR_INVALID_OPERATION = 3,

    /* General error */
    TZ_VR_ERROR_GENERAL = 4 + 0x1
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

// EXTERNAL INCLUDES
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//todor
#define VR_BUFFER_WIDTH  1024
#define VR_BUFFER_HEIGHT 1024

using namespace Dali::Integration;

extern "C"
{
// Ubuntu doesn't support ttrace, so defining missing functions
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
    int         fbos[2];
    int         colorTextures[2];
    int         depthTextures[2];
  };

  EyeBuffer*    eyeBuffers;
  int           eyeBufferCount;
  int           fbWidth, fbHeight;
  frame_buffer_handle tzvrFramebufferHandle; // For automatically created texture buffers
  int           frameIndex;

#ifdef TIZENVR_USE_DYNAMIC_LIBRARY

  void*         vrengineLib;

  // TZVR API type definitions:
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
// TizenVR symbols:
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

  // TODO: This code is required until a devel version of the TizenVR Engine package exists.
#ifdef TIZENVR_USE_DYNAMIC_LIBRARY
  mImpl->vrengineLib = dlopen( "libvrengine.so", RTLD_NOW | RTLD_GLOBAL );
  DALI_ASSERT_ALWAYS( mImpl->vrengineLib && "Can't load libvrengine.so library" );

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
  // todor: destruct eyebuffers & impl
}

bool VrEngineTizenVR::Initialize( VrEngineInitParams* initParams  )
{
  mImpl->screenWidth = initParams->screenWidth;
  mImpl->screenHeight = initParams->screenHeight;
  DALI_ASSERT_ALWAYS( TZ_VR_SUCCESS == TzVR_init( mImpl->screenWidth, mImpl->screenHeight, &mImpl->vrContext ) );
  mImpl->frameBufferDepth = TzVR_get_frame_buffer_depth();
  mImpl->eyeBuffers = new Impl::EyeBuffer[ mImpl->frameBufferDepth ];
  mImpl->eyeBufferCount = mImpl->frameBufferDepth;

  // Setup GL objects.
  return SetupVREngine( initParams );
}

void VrEngineTizenVR::Start()
{
  // This function must run on GL thread.
  EglFactory* eglFactory = static_cast<EglFactory*>( &mAdaptorInternalServices.GetEGLFactoryInterface() );
  if( !eglFactory )
  {
    DALI_LOG_ERROR( "Can't initialise VR Engine\n" );
  }
  EglImplementation* eglImplementation = static_cast<EglImplementation*>( eglFactory->GetImplementation() );
  eglImplementation->SetSurfacelessContext( true );

  EGLContext eglContext = eglImplementation->GetContext();
  // Start TzVR engine.
  TzVR_start_engine( mImpl->vrContext, eglImplementation->GetDisplay(), eglContext, eglImplementation->GetCurrentSurface() );
}

void VrEngineTizenVR::Stop()
{
  EglFactory* eglFactory = static_cast<EglFactory*>( &mAdaptorInternalServices.GetEGLFactoryInterface() );
  if( eglFactory )
  {
    EglImplementation* eglImplementation = static_cast<EglImplementation*>( eglFactory->GetImplementation() );
    eglImplementation->SetSurfacelessContext( false );
  }
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
  eyePose currentEyePose;
  memset( &currEyePose, 0, sizeof( eyePose_s ) );

  if( TzVR_get_current_pose )
  {
    TzVR_get_current_pose( mImpl->vrContext, &currentEyePose );
  }
  else
  {
    currEyePose.x = 0.0f;
    currEyePose.y = 0.0f;
    currEyePose.z = 0.0f;
    currEyePose.w = 1.0f;
  }

  Dali::Matrix poseMatrix;
  params.frame_index = mImpl->frameIndex;
  params.min_vsync_wait = 1u;
  params.render_pose[EYE_INDEX_LEFT] = currentEyePose;
  params.render_pose[EYE_INDEX_RIGHT] = currentEyePose;
  params.view = poseMatrix.AsFloat();
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
      *( reinterpret_cast<int*>( output ) ) = mImpl->eyeBufferCount;
      return true;
    }
    case EYE_BUFFER_WIDTH:
    {
      *( reinterpret_cast<int*>( output ) ) = mImpl->fbWidth;
      return true;
    }
    case EYE_BUFFER_HEIGHT:
    {
      *( reinterpret_cast<int*>( output ) ) = mImpl->fbHeight;
      return true;
    }
    case EYE_LEFT_CURRENT_TEXTURE_ID:
    {
      *( reinterpret_cast<int*>( output ) ) = mImpl->eyeBuffers[(int)( mImpl->frameIndex % mImpl->frameBufferDepth )].colorTextures[0];
      return true;
    }
    case EYE_RIGHT_CURRENT_TEXTURE_ID:
    {
      *( reinterpret_cast<int*>( output ) ) = mImpl->eyeBuffers[(int)( mImpl->frameIndex % mImpl->frameBufferDepth )].colorTextures[1];
      return true;
    }
    case EYE_LEFT_CURRENT_FBO_ID:
    {
      *( reinterpret_cast<int*>( output ) ) = mImpl->eyeBuffers[(int)( mImpl->frameIndex % mImpl->frameBufferDepth )].fbos[0];
      return true;
    }
    case EYE_RIGHT_CURRENT_FBO_ID:
    {
      *( reinterpret_cast<int*>( output ) ) = mImpl->eyeBuffers[(int)( mImpl->frameIndex % mImpl->frameBufferDepth )].fbos[1];
      return true;
    }
    case EYE_CURRENT_POSE:
    {
      VrEngineEyePose* eyePose = reinterpret_cast<VrEngineEyePose*>( output );
      eyePose_s tzvrPose;
      if( TzVR_get_current_pose( mImpl->vrContext, &tzvrPose ) )
      {
        return false;
      }

      eyePose->timestamp = tzvrPose.timestamp;
      eyePose->rotation = Quaternion( tzvrPose.w, tzvrPose.y, -tzvrPose.x, tzvrPose.z );
      return true;
    }
    default:
    {
      // Do nothing for unrecognized properties.
      break;
    }
  }

  // Return eye left/right texture ID per particular buffer.
  // Color textures.
  if( property >= EYE_LEFT_TEXTURE0_ID && property < ( EYE_LEFT_TEXTURE0_ID + 64 ) ) //todor define value 64
  {
    *( reinterpret_cast<int*>( output ) ) = mImpl->eyeBuffers[(int)property - EYE_LEFT_TEXTURE0_ID].colorTextures[0]; //todor remove c-style cast
    return true;
  }
  else if( property >= EYE_RIGHT_TEXTURE0_ID && property < ( EYE_RIGHT_TEXTURE0_ID + 64 ) )
  {
    *( reinterpret_cast<int*>( output ) ) = mImpl->eyeBuffers[(int)property - EYE_RIGHT_TEXTURE0_ID].colorTextures[1];
    return true;
  }

  // fbos
  else if( property >= EYE_LEFT_FBO_ID && property < ( EYE_LEFT_FBO_ID + 64 ) )
  {
    *( reinterpret_cast<int*>( output ) ) = mImpl->eyeBuffers[(int)property - EYE_LEFT_FBO_ID].fbos[0];
    return true;
  }
  else if( property >= EYE_RIGHT_FBO_ID && property < ( EYE_RIGHT_FBO_ID + 64 ) )
  {
    *( reinterpret_cast<int*>( output ) ) = mImpl->eyeBuffers[(int)property - EYE_RIGHT_FBO_ID].fbos[1];
    return true;
  }

  // depth textures
  else if( property >= EYE_LEFT_DEPTH_TEXTURE_ID && property < ( EYE_LEFT_DEPTH_TEXTURE_ID + 64 ) )
  {
    *( reinterpret_cast<int*>( output ) ) = mImpl->eyeBuffers[(int)property - EYE_LEFT_DEPTH_TEXTURE_ID].depthTextures[0];
    return true;
  }
  else if( property >= EYE_RIGHT_DEPTH_TEXTURE_ID && property < ( EYE_RIGHT_DEPTH_TEXTURE_ID + 64 ) )
  {
    *( reinterpret_cast<int*>( output ) ) = mImpl->eyeBuffers[(int)property - EYE_RIGHT_DEPTH_TEXTURE_ID].depthTextures[1];
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
      const VrEngineRenderTargetInfo* renderTargets = reinterpret_cast<const VrEngineRenderTargetInfo*>( input );
      DALI_ASSERT_ALWAYS( count <= mImpl->eyeBufferCount );
      for( int i = 0; i < count; ++i )
      {
        const VrEngineRenderTargetInfo* renderTarget = &renderTargets[i];
        // Frame buffer objects.
        mImpl->eyeBuffers[i].fbos[0] = renderTargets->fbos[0];
        mImpl->eyeBuffers[i].fbos[1] = renderTargets->fbos[1];

        // Color textures.
        // Check state for color texture.
        int leftColor = renderTarget->colorTextures[0];
        int rightColor = renderTarget->colorTextures[1];
        if( leftColor != 0 || rightColor != 0 )
        {
          DALI_ASSERT_ALWAYS( "Color textures ID are reserved" );
        }

        if( !mImpl->tzvrFramebufferHandle )
        {
          mImpl->tzvrFramebufferHandle = TzVR_create_texture_buffer( mImpl->vrContext, TEXTURE_TYPE_2D, mImpl->fbWidth, mImpl->fbHeight );
        }

        // Get texture IDs.
        mImpl->eyeBuffers[i].colorTextures[0] = TzVR_get_texture_id( mImpl->vrContext, mImpl->tzvrFramebufferHandle, i, 0 );
        mImpl->eyeBuffers[i].colorTextures[1] = TzVR_get_texture_id( mImpl->vrContext, mImpl->tzvrFramebufferHandle, i, 1 );

        // Depth textures.
        mImpl->eyeBuffers[i].depthTextures[0] = renderTarget->depthTextures[0];
        mImpl->eyeBuffers[i].depthTextures[1] = renderTarget->depthTextures[1];
      }
      break;
    }

    case EYE_BUFFER_COUNT:
    {
      // Not be settable.
      DALI_LOG_ERROR( "EYE_BUFFER_COUNT is not settable\n" );
      break;
    }

    case EYE_BUFFER_WIDTH:
    {
      mImpl->fbWidth = *reinterpret_cast<const int*>( input );
      break;
    }

    case EYE_BUFFER_HEIGHT:
    {
      mImpl->fbHeight = *reinterpret_cast<const int*>( input );
      break;
    }

    default:
    {
      // Do nothing.
      return false;
    }
  }
  return true;
}

bool VrEngineTizenVR::SetupVREngine( VrEngineInitializeParams* initializeParams )
{
  GlImplementation* glImplementation = static_cast<GlImplementation*>( &mAdaptorInternalServices.GetGlesInterface() );
  if( !glImplementation )
  {
    return false;
  }

  GlImplementation& context = *glImplementation;

  // Allocate GL objects, 2 objects per single buffer.
  const int totalBufferCount( mImpl->eyeBufferCount * 2 );
  uint32_t frameBufferObjects[totalBufferCount], depthTextures[totalBufferCount];
  context.GenFramebuffers( totalBufferCount, frameBufferObjects );
  context.GenTextures( totalBufferCount, depthTextures );

  // If necessary overwrite fbo sizes.
  mImpl->fbWidth = VR_BUFFER_WIDTH;//params->screenWidth;   //todorscnow - can we use params-> ?, can we get rect from here (for vr-manager).  //NOTE: params-> is surface size - not vrengine size
  mImpl->fbHeight = VR_BUFFER_HEIGHT;//params->screenHeight;

  // Set render targets info structure.
  // Iterate for each eye.
  VrEngineRenderTargetInfo renderTargetInfo[mImpl->eyeBufferCount];
  for( int i = 0; i < mImpl->eyeBufferCount; ++i )
  {
    // Iterate for each of the two buffers per eye.
    for( int k = 0; k < 2; ++k )
    {
      //todorscnow
      renderTargetInfo[i].fbos[k] = frameBufferObjects[( i * 2 ) + k];
      renderTargetInfo[i].colorTextures[k] = 0; // If not given by client side, this must be 0.
      renderTargetInfo[i].depthTextures[k] = depthTextures[( i * 2 ) + k];

      // Generate depth buffers.
      context.BindTexture( GL_TEXTURE_2D, renderTargetInfo[i].depthTextures[k] );
      context.TexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, VR_BUFFER_WIDTH, VR_BUFFER_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0 );
      context.TexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
      context.TexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    }
  }

  // Attach render targets.
  Set( VrEngine::EYE_RENDER_TARGETS, renderTargetInfo, mImpl->eyeBufferCount );

  // Attach textures to the fbos.
  int leftTexture( 0 );
  int rightTexture( 0 );
  for( int i = 0; i < mImpl->eyeBufferCount; ++i )
  {
    // left eye
    VrEngine::Get( VrEngine::EYE_LEFT_TEXTURE_ID+i, &leftTexture );
    DALI_ASSERT_ALWAYS( true == CreateFramebufferTexture( context, renderTargetInfo[i].fbos[0], leftTexture, renderTargetInfo[i].depthTextures[0] ) );

    // right eye
    VrEngine::Get( VrEngine::EYE_RIGHT_TEXTURE_ID+i, &rightTexture );
    DALI_ASSERT_ALWAYS( true == CreateFramebufferTexture( context, renderTargetInfo[i].fbos[1], rightTexture, renderTargetInfo[i].depthTextures[1] ) );
  }
  return true;
}

bool VrEngineTizenVR::CreateFramebufferTexture( GlImplementation& context, int frameBufferObject, int colorTexture, int depthTexture )
{
  GLenum frameBufferStatus;

  context.BindFramebuffer( GL_FRAMEBUFFER, frameBufferObject );
  context.FramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0 );
  context.FramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0 );

  frameBufferStatus = context.CheckFramebufferStatus( GL_FRAMEBUFFER );

  switch( frameBufferStatus )
  {
    case GL_FRAMEBUFFER_COMPLETE:
    {
      // Status OK.
      return true;
    }

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
    {
      DALI_LOG_ERROR( "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n" );
      break;
    }

    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
    {
      DALI_LOG_ERROR( "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS\n" );
      break;
    }

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
    {
      DALI_LOG_ERROR( "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n" );
      break;
    }

    case GL_FRAMEBUFFER_UNSUPPORTED:
    {
      DALI_LOG_ERROR( "GL_FRAMEBUFFER_UNSUPPORTED\n" );
      break;
    }
  }

  return false;
}

} // Adaptor

} // Internal

} // Dali
