#ifndef DALI_INTERNAL_ADAPTOR_VR_ENGINE_H
#define DALI_INTERNAL_ADAPTOR_VR_ENGINE_H

#include <dali/integration-api/vr-engine.h>
#include <base/interfaces/adaptor-internal-services.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class AdaptorInternalServices;

class VrEngine : public Dali::Integration::VrEngine
{
public:
  VrEngine();
  VrEngine( AdaptorInternalServices& internalServices  );

protected:

  AdaptorInternalServices& mAdaptorInternalServices;

};

} // Adaptor
} // Internal
} // Dali

#endif
