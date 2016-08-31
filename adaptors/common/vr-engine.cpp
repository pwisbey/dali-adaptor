#include "vr-engine.h"

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

VrEngine::VrEngine()
  : mAdaptorInternalServices( *(AdaptorInternalServices*)(NULL) )
{
  // Nothing to do here
}

VrEngine::VrEngine( AdaptorInternalServices& internalServices )
  : mAdaptorInternalServices( internalServices )
{

}

} // Adaptor
} // Internal
} // Dali

