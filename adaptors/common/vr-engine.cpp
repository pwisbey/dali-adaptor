//todor licence + doxy

//todor
#define TIZENVR_USE_DYNAMIC_LIBRARY

// CLASS HEADER
#include "vr-engine.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
//todor
#include <iostream>

// INTERNAL INCLUDES
#include <adaptors/common/gl/egl-factory.h>

//todor tmp
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

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{

// If building for VR, we default to enabled.
#ifdef TIZENVR_ENABLED
const bool DEFAULT_VR_ENABLED_STATE = true;
#else
const bool DEFAULT_VR_ENABLED_STATE = false;
#endif

} // Anonymous namespace

VrEngine::VrEngine( AdaptorInternalServices& internalServices )
: mAdaptorInternalServices( internalServices ),
  mTizenVrData( NULL ),
  mEnabled( DEFAULT_VR_ENABLED_STATE )
{
  std::cout << "todor: ...................................... VrEngine::Initialize" << std::endl;
  mTizenVrData = new TizenVrData();

#ifdef TIZENVR_USE_DYNAMIC_LIBRARY
  std::cout << "todor: tzvr: getting FUNCS" << std::endl;
  mTizenVrData->vrEngineLib = dlopen( "libvrengine.so", RTLD_NOW | RTLD_GLOBAL );
  DALI_ASSERT_ALWAYS( mTizenVrData->vrEngineLib && "Can't load libvrengine.so library" );

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
    (*(void**)FUNCTION_PTRS[i]) = dlsym( mTizenVrData->vrEngineLib, FUNCTION_NAMES[i] );
    if( !(*(void**)FUNCTION_PTRS[i]) )
    {
      DALI_LOG_ERROR( "Can't bind function: %s\n", FUNCTION_NAMES[i] );
    }
  }
#endif
}

VrEngine::~VrEngine()
{
  if( mTizenVrData )
  {
    delete [] mTizenVrData->eyeBuffers;
    delete mTizenVrData;
  }
}

bool VrEngine::Initialize( Dali::Integration::Vr::VrEngineInitializeParams* initializeParams )
{
  mTizenVrData->screenWidth = initializeParams->screenWidth;
  mTizenVrData->screenHeight = initializeParams->screenHeight;

  DALI_ASSERT_ALWAYS( TZ_VR_SUCCESS == TzVR_init( mTizenVrData->screenWidth, mTizenVrData->screenHeight, &mTizenVrData->vrContext ) );
  mTizenVrData->frameBufferDepth = TzVR_get_frame_buffer_depth();

  mTizenVrData->eyeBuffers = new TizenVrData::EyeBuffer[ mTizenVrData->frameBufferDepth ];
  mTizenVrData->eyeBufferCount = mTizenVrData->frameBufferDepth;

  // Setup GL objects.
  return SetupVREngine( initializeParams ); // todor combine funcs
}

EglImplementation* VrEngine::GetEglImplementation()
{
  EglImplementation* eglImplementation = NULL;

  EglFactory* eglFactory = static_cast<EglFactory*>( &mAdaptorInternalServices.GetEGLFactoryInterface() );
  if( eglFactory )
  {
    eglImplementation = static_cast<EglImplementation*>( eglFactory->GetImplementation() );
  }

  return eglImplementation;
}

//todor vr delete
void VrEngine::SetEnabled( bool enable )
{
  EglImplementation* eglImplementation = GetEglImplementation();
  if( eglImplementation )
  {
    eglImplementation->SetSurfacelessContext( enable );
  }
}

void VrEngine::Start()
{
  std::cout << "todor: ----------------------- VrEngineTizenVR::Start" << std::endl;
  // This function must run on GL thread.
  EglImplementation* eglImplementation = GetEglImplementation();
  if( eglImplementation )
  {
    EGLContext eglContext = eglImplementation->GetContext();

    // Start the TzVR engine.
    TzVR_start_engine( mTizenVrData->vrContext, eglImplementation->GetDisplay(), eglContext, eglImplementation->GetCurrentSurface() );
  }
}

bool VrEngine::SetupVREngine( Dali::Integration::Vr::VrEngineInitializeParams* initializeParams )
{
  GlImplementation* glImplementation = static_cast<GlImplementation*>( &mAdaptorInternalServices.GetGlesInterface() );
  if( !glImplementation )
  {
    return false;
  }

  GlImplementation& context = *glImplementation;

  // Allocate GL objects, 2 objects per single buffer.
  const int totalBufferCount( mTizenVrData->eyeBufferCount * 2 );
  uint32_t frameBufferObjects[totalBufferCount], depthTextures[totalBufferCount];
  context.GenFramebuffers( totalBufferCount, frameBufferObjects );
  context.GenTextures( totalBufferCount, depthTextures );

  // If necessary overwrite frame buffer object sizes.
  mTizenVrData->frameBufferWidth = VR_BUFFER_WIDTH;//initializeParams->viewportWidth;//todorscnow
  mTizenVrData->frameBufferHeight = VR_BUFFER_HEIGHT;// initializeParams->viewportHeight;

  // Loop to set up each eye.
  Dali::Integration::Vr::VrEngineRenderTargetInfo renderTargetInfo[mTizenVrData->eyeBufferCount];
  for( int i = 0; i < mTizenVrData->eyeBufferCount; ++i )
  {
    // Iterate for each of the two buffers per eye.
    for( int k = 0; k < 2; ++k )
    {
      // Set up render target information.
      renderTargetInfo[i].frameBufferObjects[k] = frameBufferObjects[( i * 2 ) + k];
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
  Set( VrProperty::EYE_RENDER_TARGETS, renderTargetInfo, mTizenVrData->eyeBufferCount );

  // Attach textures to the fbos.
  int leftTexture( 0 );
  int rightTexture( 0 );
  for( int i = 0; i < mTizenVrData->eyeBufferCount; ++i )
  {
    // Left eye
    Dali::Integration::VrEngine::Get( VrProperty::EYE_LEFT_TEXTURE_ID + i, &leftTexture );
    DALI_ASSERT_ALWAYS( CreateFramebufferTexture( context, renderTargetInfo[i].frameBufferObjects[0], leftTexture, renderTargetInfo[i].depthTextures[0] ) );

    // Right eye
    Dali::Integration::VrEngine::Get( VrProperty::EYE_RIGHT_TEXTURE_ID + i, &rightTexture );
    DALI_ASSERT_ALWAYS( CreateFramebufferTexture( context, renderTargetInfo[i].frameBufferObjects[1], rightTexture, renderTargetInfo[i].depthTextures[1] ) );
  }
  return true;
}

bool VrEngine::CreateFramebufferTexture( GlImplementation& context, int frameBufferObject, int colorTexture, int depthTexture )
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

bool VrEngine::Get( const int property, void* output, int )
{
  switch( property )
  {
    case VrProperty::ENABLED:
    {
      *( reinterpret_cast<bool*>( output ) ) = mEnabled;
      return true;
    }
    case VrProperty::EYE_BUFFER_COUNT:
    {
      *( reinterpret_cast<int*>( output ) ) = mTizenVrData->eyeBufferCount;
      return true;
    }
    case VrProperty::EYE_BUFFER_WIDTH:
    {
      *( reinterpret_cast<int*>( output ) ) = mTizenVrData->frameBufferWidth;
      return true;
    }
    case VrProperty::EYE_BUFFER_HEIGHT:
    {
      *( reinterpret_cast<int*>( output ) ) = mTizenVrData->frameBufferHeight;
      return true;
    }
    case VrProperty::EYE_LEFT_CURRENT_TEXTURE_ID:
    {
      *( reinterpret_cast<int*>( output ) ) = mTizenVrData->eyeBuffers[(int)( mTizenVrData->frameIndex % mTizenVrData->frameBufferDepth )].colorTextures[0];
      return true;
    }
    case VrProperty::EYE_RIGHT_CURRENT_TEXTURE_ID:
    {
      *( reinterpret_cast<int*>( output ) ) = mTizenVrData->eyeBuffers[(int)( mTizenVrData->frameIndex % mTizenVrData->frameBufferDepth )].colorTextures[1];
      return true;
    }
    case VrProperty::EYE_LEFT_CURRENT_FBO_ID:
    {
      *( reinterpret_cast<int*>( output ) ) = mTizenVrData->eyeBuffers[(int)( mTizenVrData->frameIndex % mTizenVrData->frameBufferDepth )].frameBufferObjects[0];
      return true;
    }
    case VrProperty::EYE_RIGHT_CURRENT_FBO_ID:
    {
      *( reinterpret_cast<int*>( output ) ) = mTizenVrData->eyeBuffers[(int)( mTizenVrData->frameIndex % mTizenVrData->frameBufferDepth )].frameBufferObjects[1];
      return true;
    }
    case VrProperty::EYE_CURRENT_POSE:
    {
      return GetCurrentEyePose( reinterpret_cast<Dali::Integration::Vr::VrEngineEyePose*>( output ) );
    }
    default:
    {
      // Do nothing for unrecognized properties.
      break;
    }
  }

  // Return eye left/right texture ID per particular buffer.
  // Color textures.
  if( property >= VrProperty::EYE_LEFT_TEXTURE0_ID && property < ( VrProperty::EYE_LEFT_TEXTURE0_ID + 64 ) ) //todor define value 64
  {
    *( reinterpret_cast<int*>( output ) ) = mTizenVrData->eyeBuffers[(int)property - VrProperty::EYE_LEFT_TEXTURE0_ID].colorTextures[0]; //todor remove c-style cast
    return true;
  }
  else if( property >= VrProperty::EYE_RIGHT_TEXTURE0_ID && property < ( VrProperty::EYE_RIGHT_TEXTURE0_ID + 64 ) )
  {
    *( reinterpret_cast<int*>( output ) ) = mTizenVrData->eyeBuffers[(int)property - VrProperty::EYE_RIGHT_TEXTURE0_ID].colorTextures[1];
    return true;
  }

  // fbos
  else if( property >= VrProperty::EYE_LEFT_FBO_ID && property < ( VrProperty::EYE_LEFT_FBO_ID + 64 ) )
  {
    *( reinterpret_cast<int*>( output ) ) = mTizenVrData->eyeBuffers[(int)property - VrProperty::EYE_LEFT_FBO_ID].frameBufferObjects[0];
    return true;
  }
  else if( property >= VrProperty::EYE_RIGHT_FBO_ID && property < ( VrProperty::EYE_RIGHT_FBO_ID + 64 ) )
  {
    *( reinterpret_cast<int*>( output ) ) = mTizenVrData->eyeBuffers[(int)property - VrProperty::EYE_RIGHT_FBO_ID].frameBufferObjects[1];
    return true;
  }

  // depth textures
  else if( property >= VrProperty::EYE_LEFT_DEPTH_TEXTURE_ID && property < ( VrProperty::EYE_LEFT_DEPTH_TEXTURE_ID + 64 ) )
  {
    *( reinterpret_cast<int*>( output ) ) = mTizenVrData->eyeBuffers[(int)property - VrProperty::EYE_LEFT_DEPTH_TEXTURE_ID].depthTextures[0];
    return true;
  }
  else if( property >= VrProperty::EYE_RIGHT_DEPTH_TEXTURE_ID && property < ( VrProperty::EYE_RIGHT_DEPTH_TEXTURE_ID + 64 ) )
  {
    *( reinterpret_cast<int*>( output ) ) = mTizenVrData->eyeBuffers[(int)property - VrProperty::EYE_RIGHT_DEPTH_TEXTURE_ID].depthTextures[1];
    return true;
  }
  return false;
}

bool VrEngine::Set( const int property, const void* input, int count )
{
  switch( property )
  {
    case VrProperty::EYE_RENDER_TARGETS:
    {
      const Dali::Integration::Vr::VrEngineRenderTargetInfo* renderTargets = reinterpret_cast<const Dali::Integration::Vr::VrEngineRenderTargetInfo*>( input );
      DALI_ASSERT_ALWAYS( count <= mTizenVrData->eyeBufferCount );
      for( int i = 0; i < count; ++i )
      {
        const Dali::Integration::Vr::VrEngineRenderTargetInfo* renderTarget = &renderTargets[i];
        // Frame buffer objects.
        mTizenVrData->eyeBuffers[i].frameBufferObjects[0] = renderTarget->frameBufferObjects[0];
        mTizenVrData->eyeBuffers[i].frameBufferObjects[1] = renderTarget->frameBufferObjects[1];

        // Color textures.
        // Check state for color texture.
        int leftColor = renderTarget->colorTextures[0];
        int rightColor = renderTarget->colorTextures[1];
        if( leftColor != 0 || rightColor != 0 )
        {
          DALI_ASSERT_ALWAYS( "Color textures ID are reserved\n" );
        }

        if( !mTizenVrData->tzvrFramebufferHandle )
        {
          mTizenVrData->tzvrFramebufferHandle = TzVR_create_texture_buffer( mTizenVrData->vrContext, TEXTURE_TYPE_2D, mTizenVrData->frameBufferWidth, mTizenVrData->frameBufferHeight );
        }

        // Get texture IDs.
        mTizenVrData->eyeBuffers[i].colorTextures[0] = TzVR_get_texture_id( mTizenVrData->vrContext, mTizenVrData->tzvrFramebufferHandle, i, 0 );
        mTizenVrData->eyeBuffers[i].colorTextures[1] = TzVR_get_texture_id( mTizenVrData->vrContext, mTizenVrData->tzvrFramebufferHandle, i, 1 );

        // Depth textures.
        mTizenVrData->eyeBuffers[i].depthTextures[0] = renderTarget->depthTextures[0];
        mTizenVrData->eyeBuffers[i].depthTextures[1] = renderTarget->depthTextures[1];
      }
      break;
    }

    case VrProperty::EYE_BUFFER_COUNT:
    {
      DALI_LOG_ERROR( "EYE_BUFFER_COUNT is not settable\n" );
      break;
    }

    case VrProperty::EYE_BUFFER_WIDTH:
    {
      mTizenVrData->frameBufferWidth = *reinterpret_cast<const int*>( input );
      break;
    }

    case VrProperty::EYE_BUFFER_HEIGHT:
    {
      mTizenVrData->frameBufferHeight = *reinterpret_cast<const int*>( input );
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

void VrEngine::Stop()
{
  SetEnabled( false );
}

void VrEngine::PreRender()
{
}

void VrEngine::PostRender()
{
}


} // Adaptor

} // Internal

} // Dali

