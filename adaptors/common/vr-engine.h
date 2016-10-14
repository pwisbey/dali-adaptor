//todor licence + doxy
#ifndef DALI_INTERNAL_ADAPTOR_VR_ENGINE_H
#define DALI_INTERNAL_ADAPTOR_VR_ENGINE_H

//todor
#define TIZENVR_USE_DYNAMIC_LIBRARY

// CLASS HEADER
#include "dali/integration-api/vr-engine.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/vr-defaults.h>

// INTERNAL INCLUDES
#include <adaptors/common/gl/egl-implementation.h>
#include <adaptors/common/tizen-vr-platform-header.h>
#include <base/interfaces/adaptor-internal-services.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class AdaptorInternalServices;
class GlImplementation;

class VrEngine : public Dali::Integration::VrEngine
{
public:

  /**
   * @brief todor
   */
  VrEngine( AdaptorInternalServices& internalServices );

  /**
   * @brief todor
   */
  virtual ~VrEngine();


  // Methods that must be defined by the deriving class:

  /**
   * @brief todor
   */
  virtual bool GetCurrentEyePose( Dali::Integration::Vr::VrEngineEyePose* eyePose ) = 0;

public:

  // Optional virtual methods (definitions exist in this module by default):

  //todor doxy
  bool Initialize( unsigned int screenWidth, unsigned int screenHeight );

private:

  bool SetupVREngine();
  void SetEyeRenderTargets( const Dali::Integration::Vr::VrEngineRenderTargetInfo* renderTargets );
  bool CreateFramebufferTexture( GlImplementation& context, int frameBufferObject, int colorTexture, int depthTexture );
  bool Get( const int property, void* output );
  void GetEyeBufferInfo( EyeBufferInfo& eyeBufferInfo );
  void Start();
  void Stop();
  void PreRender();
  void PostRender();

protected:

  // todor TZVR API type definitions
#ifdef TIZENVR_USE_DYNAMIC_LIBRARY
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

  /**
   * @brief todor
   */
  EglImplementation* GetEglImplementation();

  struct TizenVrData
  {
    TizenVrData()
    : vrContext( 0 ),
      eglContext( 0 ),
      screenWidth( 0 ),
      screenHeight( 0 ),
      frameBufferDepth( 0 ),
      eyeBuffers( NULL ),
      eyeBufferCount( 0 ),
      frameBufferWidth( Dali::Integration::Vr::DEFAULT_VR_VIEWPORT_DIMENSIONS.width ),
      frameBufferHeight( Dali::Integration::Vr::DEFAULT_VR_VIEWPORT_DIMENSIONS.height ),
      tzvrFramebufferHandle( 0 ),
      frameIndex( 0 )
#ifdef TIZENVR_USE_DYNAMIC_LIBRARY
      , vrEngineLib( NULL )
#endif
    {
    }

    tzvr_context  vrContext;
    EGLContext    eglContext;
    int           screenWidth;
    int           screenHeight;
    int           frameBufferDepth;

    // An EyeBuffer is allocated for each eye. Each contain 2 sets of buffers.
    struct EyeBuffer
    {
      int         frameBufferObjects[2];
      int         colorTextures[2];
      int         depthTextures[2];
    };

    EyeBuffer*    eyeBuffers;
    unsigned int  eyeBufferCount;
    int           frameBufferWidth;
    int           frameBufferHeight;
    frame_buffer_handle tzvrFramebufferHandle; // For automatically created texture buffers
    int           frameIndex;

#ifdef TIZENVR_USE_DYNAMIC_LIBRARY
    void*         vrEngineLib;
#endif
  };

    // TizenVR Symbols
#ifdef TIZENVR_USE_DYNAMIC_LIBRARY
    VrEngine::TZVRINITPROC                  TzVR_init;
    VrEngine::TZVRDEINITPROC                TzVR_deinit;
    VrEngine::TZVRSTARTENGINEPROC           TzVR_start_engine;
    VrEngine::TZVRSTOPENGINEPROC            TzVR_stop_engine;
    VrEngine::TZVRSUBMITFRAMEPROC           TzVR_submit_frame;
    VrEngine::TZVRGETTEXTUREIDPROC          TzVR_get_texture_id;
    VrEngine::TZVRSETTEXTUREIDPROC          TzVR_set_texture_id;
    VrEngine::TZVRGETCURRENTPOSEPROC        TzVR_get_current_pose;
    VrEngine::TZVRGETFRAMEBUFFERDEPTHPROC   TzVR_get_frame_buffer_depth;
    VrEngine::TZVRCREATETEXTUREBUFFERPROC   TzVR_create_texture_buffer;
    VrEngine::TZVRDESTROYTEXTUREBUFFERPROC  TzVR_destroy_texture_buffer;
    VrEngine::TZVRRECENTERPOSEPROC          TzVR_recenter_pose;
#endif

    AdaptorInternalServices& mAdaptorInternalServices; ///< todor
    TizenVrData* mTizenVrData;                         ///< todor
    bool mEnabled:1;                                   ///< todor

};


} // Adaptor

} // Internal

} // Dali

#endif // DALI_INTERNAL_ADAPTOR_VR_ENGINE_H
