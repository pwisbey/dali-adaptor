#ifndef DALI_INTERNAL_ADAPTOR_VR_ENGINE_TIZEN_H
#define DALI_INTERNAL_ADAPTOR_VR_ENGINE_TIZEN_H

//todor licence

#include <adaptors/common/vr-engine.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

// TizenVR engine Tizen backend.
class VrEngineTizenVR : public Dali::Internal::Adaptor::VrEngine
{
public:

  /**
   * @brief Constructor.
   * @param[in] internalServices Provides access to adaptor services
   */
  VrEngineTizenVR( AdaptorInternalServices& internalServices );

  /**
   * @brief Destructor
   */
  ~VrEngineTizenVR();

private:

  // Pure virtual method definitions:

  /**
   * @see Dali::Integration::VrEngine::SubmitFrame
   */
  void SubmitFrame();

  /**
   * @see Dali::Internal::Adaptor::VrEngine::GetCurrentEyePose
   */
  bool GetCurrentEyePose( Dali::Integration::Vr::VrEngineEyePose* eyePose );

};


} // Adaptor

} // Internal

} // Dali

#endif // DALI_INTERNAL_ADAPTOR_VR_ENGINE_TIZEN_H
