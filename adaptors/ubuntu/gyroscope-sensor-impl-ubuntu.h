#ifndef __DALI_INTERNAL_ADAPTOR_GYROSCOPE_SENSOR_H__
#define __DALI_INTERNAL_ADAPTOR_GYROSCOPE_SENSOR_H__

#include <dali/integration-api/gyroscope-sensor.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

class GyroscopeSensor : public Dali::Integration::GyroscopeSensor
{
public:
  GyroscopeSensor();
  virtual ~GyroscopeSensor();

  virtual void Enable();
  virtual void Disable();
  virtual void Read( Dali::Vector4& data );
  virtual void ReadPackets( Dali::Vector4* data, int count );
  virtual bool IsEnabled();
  virtual bool IsSupported();

private:

  bool  mIsEnabled;
  int   mPort;
  int   mClientSocket;
};

} // Internal

} // Adaptor

} // Dali


#endif
