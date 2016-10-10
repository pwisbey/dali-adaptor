#ifndef DALI_INTERNAL_ADAPTOR_VR_ENGINE_TIZEN_H
#define DALI_INTERNAL_ADAPTOR_VR_ENGINE_TIZEN_H

//todor licence
//todor copy to vr-engine-impl-ubuntu.h
//#include <dali/integration-api/vr-engine.h>
#include <adaptors/common/vr-engine.h>

using namespace Dali::Integration::Vr;

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class GlImplementation;

// TizenVR engine backend.
class VrEngineTizenVR : public Dali::Internal::Adaptor::VrEngine
{
public:

    //todor doxygen
  VrEngineTizenVR( AdaptorInternalServices& internalServices );
  virtual ~VrEngineTizenVR();

  virtual bool Initialize( VrEngineInitializeParams* initializeParameters );
  virtual void SetEnabled( bool enabled ); //todor
  virtual void Start();
  virtual void Stop();
  virtual void PreRender();
  virtual void PostRender();
  virtual void SubmitFrame();
  virtual bool Get( const int property, void* output, int count );
  virtual bool Set( const int property, const void* input, int count );

private:

  bool SetupVREngine( VrEngineInitializeParams* initializeParameters );
  bool CreateFramebufferTexture( GlImplementation& context, int frameBufferObject, int colorTexture, int depthTexture );

  // Pure virtual method definitions:

  bool GetCurrentEyePose( VrEngineEyePose* eyePose );

  //struct Impl* mImpl;

};


} // Adaptor

} // Internal

} // Dali

#endif // DALI_INTERNAL_ADAPTOR_VR_ENGINE_TIZEN_H
