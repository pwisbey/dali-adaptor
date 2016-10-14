//todor licence + doxy

// CLASS HEADER
#include "vr-engine.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <adaptors/common/gl/egl-factory.h>
#include <adaptors/common/gl/gl-implementation.h>


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

const unsigned int EYE_COUNT = 2;

} // Anonymous namespace

VrEngine::VrEngine( AdaptorInternalServices& internalServices )
: mAdaptorInternalServices( internalServices ),
  mTizenVrData( NULL ),
  mEnabled( DEFAULT_VR_ENABLED_STATE )
{
  mTizenVrData = new TizenVrData();

  // There is currently no TizenVr Engine development library. We work around this by using dlopen to get access to methods at run-time.
#ifdef TIZENVR_USE_DYNAMIC_LIBRARY
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
    &TzVR_init,                       //todor using
    &TzVR_deinit,
    &TzVR_start_engine,            //todor using
    &TzVR_stop_engine,
    &TzVR_submit_frame,            //todor using
    &TzVR_get_texture_id,            //todor using
    &TzVR_set_texture_id,
    &TzVR_get_current_pose,            //todor using
    &TzVR_get_frame_buffer_depth,            //todor using
    &TzVR_create_texture_buffer,        //todor using
    &TzVR_destroy_texture_buffer,
    &TzVR_recenter_pose,
    NULL
  };

  for( int i = 0; i < 12; ++i )
  {
    ( *(void**)FUNCTION_PTRS[i] ) = dlsym( mTizenVrData->vrEngineLib, FUNCTION_NAMES[i] );
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
    mTizenVrData = NULL;
  }
}

bool VrEngine::Initialize( unsigned int screenWidth, unsigned int screenHeight )
{
  mTizenVrData->screenWidth = screenWidth;
  mTizenVrData->screenHeight = screenHeight;

  DALI_ASSERT_ALWAYS( TZ_VR_SUCCESS == TzVR_init( mTizenVrData->screenWidth, mTizenVrData->screenHeight, &mTizenVrData->vrContext ) );
  mTizenVrData->frameBufferDepth = TzVR_get_frame_buffer_depth();

  mTizenVrData->eyeBufferCount = mTizenVrData->frameBufferDepth;
  mTizenVrData->eyeBuffers = new TizenVrData::EyeBuffer[ mTizenVrData->frameBufferDepth ];

  // Setup GL objects.
  return SetupVREngine();
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

void VrEngine::Start()
{
  // This function must run on GL thread.
  EglImplementation* eglImplementation = GetEglImplementation();
  if( eglImplementation )
  {
    EGLContext eglContext = eglImplementation->GetContext();

    // Start the TzVR engine.
    TzVR_start_engine( mTizenVrData->vrContext, eglImplementation->GetDisplay(), eglContext, eglImplementation->GetCurrentSurface() );
  }
}

bool VrEngine::SetupVREngine()
{
  GlImplementation* glImplementation = static_cast<GlImplementation*>( &mAdaptorInternalServices.GetGlesInterface() );
  if( !glImplementation )
  {
    return false;
  }

  GlImplementation& context = *glImplementation;

  // Allocate GL objects, 2 objects per single buffer.
  const unsigned int totalBufferCount( mTizenVrData->eyeBufferCount * EYE_COUNT ); //todor 2=eyes
  uint32_t frameBufferObjects[totalBufferCount];
  uint32_t depthTextures[totalBufferCount];
  context.GenFramebuffers( totalBufferCount, frameBufferObjects );
  context.GenTextures( totalBufferCount, depthTextures );

  // Frame buffer object sizes.
  mTizenVrData->frameBufferWidth = Dali::Integration::Vr::DEFAULT_VR_VIEWPORT_DIMENSIONS.width;
  mTizenVrData->frameBufferHeight = Dali::Integration::Vr::DEFAULT_VR_VIEWPORT_DIMENSIONS.height;

  // Loop to set up all buffers per eye.
  Dali::Integration::Vr::VrEngineRenderTargetInfo renderTargetInfo[mTizenVrData->eyeBufferCount];
  for( unsigned int i = 0; i < mTizenVrData->eyeBufferCount; ++i )
  {
    // Loop for each eye.
    for( unsigned int k = 0; k < EYE_COUNT; ++k )
    {
      // Set up render target information.
      renderTargetInfo[i].frameBufferObjects[k] = frameBufferObjects[( i * EYE_COUNT ) + k];
      renderTargetInfo[i].colorTextures[k] = 0; // If not given by client side, this must be 0.
      renderTargetInfo[i].depthTextures[k] = depthTextures[( i * EYE_COUNT ) + k];

      // Generate depth buffers.
      context.BindTexture( GL_TEXTURE_2D, renderTargetInfo[i].depthTextures[k] );
      // TODO: change to depth 16 after testing
      context.TexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, mTizenVrData->frameBufferWidth, mTizenVrData->frameBufferHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0 );

      context.TexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
      context.TexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    }
  }

  // Attach render targets.
  SetEyeRenderTargets( renderTargetInfo );

  // Attach textures to the frame buffer objects.
  int leftTexture( 0 );
  int rightTexture( 0 );
  for( unsigned int i = 0; i < mTizenVrData->eyeBufferCount; ++i )
  {
    // Left eye
    Get( VrProperty::EYE_LEFT_TEXTURE_ID + i, &leftTexture );
    DALI_ASSERT_ALWAYS( CreateFramebufferTexture( context, renderTargetInfo[i].frameBufferObjects[Integration::Vr::Eye::LEFT], leftTexture, renderTargetInfo[i].depthTextures[Integration::Vr::Eye::LEFT] ) );

    // Right eye
    Get( VrProperty::EYE_RIGHT_TEXTURE_ID + i, &rightTexture );
    DALI_ASSERT_ALWAYS( CreateFramebufferTexture( context, renderTargetInfo[i].frameBufferObjects[Integration::Vr::Eye::RIGHT], rightTexture, renderTargetInfo[i].depthTextures[Integration::Vr::Eye::RIGHT] ) );
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

bool VrEngine::Get( const int property, void* output )
{
  switch( property )
  {
    //todor IsEnabled() ?
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
    case VrProperty::EYE_CURRENT_POSE:
    {
      return GetCurrentEyePose( reinterpret_cast<Dali::Integration::Vr::VrEngineEyePose*>( output ) );
    }
    default:
    {
      // Return eye left/right texture ID per particular buffer.
      // Color textures.
      if( property >= VrProperty::EYE_LEFT_TEXTURE_ID && property < ( VrProperty::EYE_LEFT_TEXTURE_ID + 64 ) ) //todor define value 64
      {
        *( reinterpret_cast<int*>( output ) ) = mTizenVrData->eyeBuffers[property - VrProperty::EYE_LEFT_TEXTURE_ID].colorTextures[0]; //todor remove c-style cast
        return true;
      }
      else if( property >= VrProperty::EYE_RIGHT_TEXTURE_ID && property < ( VrProperty::EYE_RIGHT_TEXTURE_ID + 64 ) )
      {
        *( reinterpret_cast<int*>( output ) ) = mTizenVrData->eyeBuffers[property - VrProperty::EYE_RIGHT_TEXTURE_ID].colorTextures[1];
        return true;
      }

      // Unrecognised.
      return false;
    }
  }

  return true;
}

void VrEngine::SetEyeRenderTargets( const Dali::Integration::Vr::VrEngineRenderTargetInfo* renderTargets )
{
  for( unsigned int i = 0; i < mTizenVrData->eyeBufferCount; ++i )
  {
    const Dali::Integration::Vr::VrEngineRenderTargetInfo* renderTarget = &( renderTargets[i] );
    // Frame buffer objects.
    mTizenVrData->eyeBuffers[i].frameBufferObjects[Integration::Vr::Eye::LEFT] = renderTarget->frameBufferObjects[Integration::Vr::Eye::LEFT];
    mTizenVrData->eyeBuffers[i].frameBufferObjects[Integration::Vr::Eye::RIGHT] = renderTarget->frameBufferObjects[Integration::Vr::Eye::RIGHT];

    // Check state of color textures.
    if( renderTarget->colorTextures[Integration::Vr::Eye::LEFT] != 0 || renderTarget->colorTextures[Integration::Vr::Eye::RIGHT] != 0 )
    {
      DALI_ASSERT_ALWAYS( "Color texture IDs are reserved\n" );
    }

    if( !mTizenVrData->tzvrFramebufferHandle )
    {
      mTizenVrData->tzvrFramebufferHandle = TzVR_create_texture_buffer( mTizenVrData->vrContext, TEXTURE_TYPE_2D, mTizenVrData->frameBufferWidth, mTizenVrData->frameBufferHeight );
    }

    // Get texture IDs.
    mTizenVrData->eyeBuffers[i].colorTextures[Integration::Vr::Eye::LEFT] = TzVR_get_texture_id( mTizenVrData->vrContext, mTizenVrData->tzvrFramebufferHandle, i, Integration::Vr::Eye::LEFT );
    mTizenVrData->eyeBuffers[i].colorTextures[Integration::Vr::Eye::RIGHT] = TzVR_get_texture_id( mTizenVrData->vrContext, mTizenVrData->tzvrFramebufferHandle, i, Integration::Vr::Eye::RIGHT );

    // Depth textures.
    mTizenVrData->eyeBuffers[i].depthTextures[Integration::Vr::Eye::LEFT] = renderTarget->depthTextures[Integration::Vr::Eye::LEFT];
    mTizenVrData->eyeBuffers[i].depthTextures[Integration::Vr::Eye::RIGHT] = renderTarget->depthTextures[Integration::Vr::Eye::RIGHT];
  }
}

void VrEngine::GetEyeBufferInfo( EyeBufferInfo& eyeBufferInfo )
{
  eyeBufferInfo.bufferWidth = mTizenVrData->frameBufferWidth;
  eyeBufferInfo.bufferHeight = mTizenVrData->frameBufferHeight;

  eyeBufferInfo.eye[Integration::Vr::Eye::LEFT].texture = mTizenVrData->eyeBuffers[ mTizenVrData->frameIndex % mTizenVrData->frameBufferDepth ].colorTextures[Integration::Vr::Eye::LEFT];
  eyeBufferInfo.eye[Integration::Vr::Eye::RIGHT].texture = mTizenVrData->eyeBuffers[ mTizenVrData->frameIndex % mTizenVrData->frameBufferDepth ].colorTextures[Integration::Vr::Eye::RIGHT];

  eyeBufferInfo.eye[Integration::Vr::Eye::LEFT].frameBufferObject = mTizenVrData->eyeBuffers[ mTizenVrData->frameIndex % mTizenVrData->frameBufferDepth ].frameBufferObjects[Integration::Vr::Eye::LEFT];
  eyeBufferInfo.eye[Integration::Vr::Eye::RIGHT].frameBufferObject = mTizenVrData->eyeBuffers[ mTizenVrData->frameIndex % mTizenVrData->frameBufferDepth ].frameBufferObjects[Integration::Vr::Eye::RIGHT];
}

void VrEngine::Stop()
{
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

