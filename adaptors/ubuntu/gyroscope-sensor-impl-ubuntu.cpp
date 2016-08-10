// CLASS HEADER
#include "gyroscope-sensor-impl-ubuntu.h"

#include <dali/integration-api/gyroscope-sensor.h>
#include <dali/dali.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

namespace
{

}

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

#define MAX_SAMPLES    3
#define SENSOR_SERVER_PORT 8888
#define GYRO_TO_ANGLE	(0.0000228f)

typedef struct
{
    int32_t x, y, z;
} SensorRaw;

typedef struct
{
    int32_t AccelX, AccelY, AccelZ;
    int32_t GyroX, GyroY, GyroZ;
} KTrackerSensorRawData;

typedef struct
{
    uint8_t SampleCount;
    uint16_t Timestamp;
    uint16_t LastCommandID;
    int16_t Temperature;
    KTrackerSensorRawData Samples[MAX_SAMPLES];
    int16_t MagX, MagY, MagZ;
    float TotalGyroX, TotalGyroY, TotalGyroZ;
} KTrackerSensorZip;


GyroscopeSensor::GyroscopeSensor()
  : mIsEnabled( false ),
    mPort( SENSOR_SERVER_PORT ),
    mClientSocket( -1 )
{

}

GyroscopeSensor::~GyroscopeSensor()
{
  if( mIsEnabled )
  {
    Disable();
  }
}

void GyroscopeSensor::Enable()
{
  if( !IsSupported() || mIsEnabled )
  {
    return;
  }

  mClientSocket = socket( AF_INET, SOCK_STREAM, 0 );
  sockaddr_in caddr;
  socklen_t caddr_len = sizeof( caddr );
  memset( &caddr, 0, sizeof( caddr ) );
  caddr.sin_family = AF_INET;
  caddr.sin_port = htons( mPort );
  caddr.sin_addr.s_addr = inet_addr( "127.0.0.1" );

  if( connect( mClientSocket, (sockaddr*)&caddr, caddr_len ) < 0 )
  {
    return; // handle error
  }
  mIsEnabled = true;
}

void GyroscopeSensor::Disable()
{
  if( mIsEnabled )
  {
    mIsEnabled = false;
  }
}

void GyroscopeSensor::Read( Dali::Vector4& outdata )
{
  if( mIsEnabled )
  {
    ReadPackets( &outdata, 1 );
  }
}

void GyroscopeSensor::ReadPackets( Dali::Vector4* outdata, int count )
{
  if( mIsEnabled )
  {
    KTrackerSensorZip data[256];
    unsigned char c = (unsigned char)count;
    char buf[2];
    buf[0] = 'r';
    buf[1] = c;
    send( mClientSocket, buf, 2, 0 );

    // receive only accumulated reading
    recv( mClientSocket, &data[0], sizeof(data), 0 );
    outdata[0].x = data[0].TotalGyroX;
    outdata[0].y = data[0].TotalGyroY;
    outdata[0].z = data[0].TotalGyroZ;
  }
}

bool GyroscopeSensor::IsEnabled()
{
  return mIsEnabled;
}

bool GyroscopeSensor::IsSupported()
{
  // will use socket connection with GearVR and Note4
  return true;
}


}
}
}
