#ifndef DALI_INTERNAL_ADAPTOR_VR_ENGINE_TIZEN_H
#define DALI_INTERNAL_ADAPTOR_VR_ENGINE_TIZEN_H

#include <dali/integration-api/vr-engine.h>
#include <adaptors/common/vr-engine.h>

using namespace Dali::Integration::Vr;

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class GlImplementation;
// TizenVR engine backend
class VrEngineTizenVR : public Dali::Internal::Adaptor::VrEngine
{
public:

  VrEngineTizenVR( AdaptorInternalServices& internalServices );
  virtual ~VrEngineTizenVR();

  virtual bool Initialize( VrEngineInitParams* initParams );
  virtual void Start();
  virtual void Stop();
  virtual void PreRender();
  virtual void PostRender();
  virtual void SubmitFrame();
  virtual bool Get( const int property, void* output, int count );
  virtual bool Set( const int property, const void* input, int count );

private:
  bool SetupVREngine( VrEngineInitParams* params );
  bool CreateFramebufferTexture( GlImplementation& ctx, int fbo, int colorTexture, int depthTexture );

  struct Impl* mImpl;
};


} // Adaptor
} // Internal
} // Dali


#endif
