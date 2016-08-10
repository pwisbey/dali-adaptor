// CLASS HEADER
#include "gyroscope-sensor-impl-tizen.h"

#include <dali/integration-api/gyroscope-sensor.h>
#include <dali/public-api/math/vector4.h>
// tizen stuff
#include <sensor/sensor.h>
#include <limits.h>
#include <stdio.h>
#include <inttypes.h>
#include <poll.h>
#include <unistd.h>
#include <stdint.h>
#include <linux/stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


#define VR_USE_GEARVR_SENSOR 1
#define VR_USE_GEARVR_SENSOR_PROXY 1
#define VR_OVR_PIPE_NAME "/home/owner/ovr.pipe"

// this is for GearVR gyro
#define GYRO_TO_ANGLE	(0.0000228f)
namespace
{
FILE* gLogFile( NULL );
#define printf(...) \
  { \
  check_stream(); \
  fprintf( gLogFile, __VA_ARGS__ );\
  fflush( gLogFile );\
  }
#define puts( x ) \
{ check_stream(); fputs( x, gLogFile ); }

void check_stream()
{
  if( !gLogFile )
    gLogFile = fopen( "/tmp/gyro.log", "wb" );
}

/*
void callback(sensor_h sensor, sensor_event_s *event, void *data)
{
  puts("event!");
}
*/


// GearVR sensor
#define MAX_SAMPLES    3
#define SENSOR_DEV_PATH    "/dev/ovr0"

static int fd = -1;
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

#if VR_USE_GEARVR_SENSOR_PROXY != 1
int open_sensor()
{
	dev_t sensordev;

	/* sh~$ mknod /dev/ovr0 c 247 0 */
	sensordev = makedev(248, 0);
	mknod(SENSOR_DEV_PATH, 0666 | S_IFCHR, sensordev);

	fd = open(SENSOR_DEV_PATH, O_RDONLY);
	if (fd < 0) {
		printf("open error %s\n", strerror(errno));
		return -1;
	}
	else
	{
		printf("open success\n");
	}
	return 0;
}

int close_sensor()
{
	close(fd);
  return 0;
}
#endif

int read_sensor(KTrackerSensorZip* data)
{
#if VR_USE_GEARVR_SENSOR_PROXY
  if( fd > -1 )
  {
    return read( fd, data, sizeof( KTrackerSensorZip ) );
  }
#else
	struct pollfd pfds;
	pfds.fd = fd;
	pfds.events = POLLIN | POLLHUP | POLLERR;

	uint8_t buffer[100];
	int n = 0, ret;
	int i;

	if (fd < 0)
		return -1;

	while (n <= 0)
	{
		n = poll(&pfds, 1, 100);
		if (!(pfds.revents & POLLIN))
			continue;

		ret = read(fd, buffer, 100);
		if (ret <= 0) {
			printf("k_sensor: read error %d\n", ret);
			return -1;
		}

		data->SampleCount = buffer[1];
		data->Timestamp = (uint16_t) (*(buffer + 3) << 8) | (uint16_t) (*(buffer + 2));
		data->LastCommandID = (uint16_t) (*(buffer + 5) << 8) | (uint16_t) (*(buffer + 4));
		data->Temperature = (int16_t) (*(buffer + 7) << 8) | (int16_t) (*(buffer + 6));

		for (i = 0; i < (data->SampleCount > MAX_SAMPLES ? MAX_SAMPLES : data->SampleCount); ++i) {
			struct
			{
				int32_t x :21;
			} s;

			data->Samples[i].AccelX = s.x = (buffer[0 + 8 + 16 * i] << 13) | (buffer[1 + 8 + 16 * i] << 5)
				| ((buffer[2 + 8 + 16 * i] & 0xF8) >> 3);
			data->Samples[i].AccelY = s.x = ((buffer[2 + 8 + 16 * i] & 0x07) << 18) | (buffer[3 + 8 + 16 * i] << 10)
				| (buffer[4 + 8 + 16 * i] << 2) | ((buffer[5 + 8 + 16 * i] & 0xC0) >> 6);
			data->Samples[i].AccelZ = s.x = ((buffer[5 + 8 + 16 * i] & 0x3F) << 15) | (buffer[6 + 8 + 16 * i] << 7)
				| (buffer[7 + 8 + 16 * i] >> 1);

			data->Samples[i].GyroX = s.x = (buffer[0 + 16 + 16 * i] << 13) | (buffer[1 + 16 + 16 * i] << 5)
				| ((buffer[2 + 16 + 16 * i] & 0xF8) >> 3);
			data->Samples[i].GyroY = s.x = ((buffer[2 + 16 + 16 * i] & 0x07) << 18) | (buffer[3 + 16 + 16 * i] << 10)
				| (buffer[4 + 16 + 16 * i] << 2) | ((buffer[5 + 16 + 16 * i] & 0xC0) >> 6);
			data->Samples[i].GyroZ = s.x = ((buffer[5 + 16 + 16 * i] & 0x3F) << 15) | (buffer[6 + 16 + 16 * i] << 7)
				| (buffer[7 + 16 + 16 * i] >> 1);
		}
	}
#endif
	return 0;
}


}

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

GyroscopeSensor::GyroscopeSensor()
  : mIsEnabled( false ),
    mSensor( NULL ),
    mListener( NULL )
{
  sensor_h *list = NULL;
  int count = 0;
  sensor_get_sensor_list( SENSOR_ALL, &list, &count );
  for( int i = 0; i < count; ++i )
  {
    char* name = NULL;
    int interval = 0;
    float minRange = 0.0f, maxRange = 0.0f;
    sensor_get_name( list[i], &name );
    sensor_get_min_interval( list[i], &interval );
    sensor_get_min_range(  list[i], &minRange );
    sensor_get_max_range(  list[i], &maxRange );

    printf("SENSOR: %d\n"
           "SENSOR: name       = %s\n"
           "SENSOR: min_ival   = %d\n"
           "SENSOR: min_range: = %f\n"
           "SENSOR: max_range: = %f\n\n",
           i, name, interval, minRange, maxRange );
  }
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
    puts("Sensor not supported!\n");
    return;
  }
#if VR_USE_GEARVR_SENSOR == 1
#if VR_USE_GEARVR_SENSOR_PROXY == 1
  // check if pipe exists ( server must be launched before DALi )
  fd = open( VR_OVR_PIPE_NAME, O_RDONLY );
  if( fd < 0 )
  {
    printf("open error %s\n", strerror(errno));
    return;
  }
  mIsEnabled = true;
#endif
#else
  puts("Sensor supported");

  if( sensor_get_default_sensor( SENSOR_GYROSCOPE, &mSensor ) )
  {
    puts("sensor_get_default_sensor() failed!");
    return;
  }
  if( sensor_create_listener( mSensor, &mListener ) )
  {
    puts( "sensor_create_listener() failed!");
    return;
  }
  if( sensor_listener_start( mListener ) )
  {
    puts( "sensor_listener_start() failed!");
    return;
  }
  char* name = NULL;
  int interval = 0;
  float minRange = 0.0f, maxRange = 0.0f;
  sensor_get_name( mSensor, &name );
  sensor_get_min_interval( mSensor, &interval );
  sensor_get_min_range(  mSensor, &minRange );
  sensor_get_max_range(  mSensor, &maxRange );

  printf("SENSOR: name       = %s\n"
         "SENSOR: min_ival   = %d\n"
         "SENSOR: min_range: = %f\n"
         "SENSOR: max_range: = %f\n",
         name, interval, minRange, maxRange );

  if( sensor_listener_set_event_cb( mListener, 8, callback, this ) )
  {
    puts("Callback failed");
  }

  mIsEnabled = true;
#endif
}

void GyroscopeSensor::Disable()
{
#if VR_USE_GEARVR_SENSOR == 1
#if VR_USE_GEARVR_SENSOR_PROXY == 1
  close( fd );
  fd = -1;
#else
  close_sensor();
#endif
#else
  if( mIsEnabled )
  {
    sensor_listener_stop( mListener );
    sensor_destroy_listener( mListener );
    mIsEnabled = false;
  }
#endif
}

void GyroscopeSensor::ReadPackets( Dali::Vector4* outdata, int count )
{
  if( mIsEnabled )
  {
    KTrackerSensorZip data;
    memset( &data, 0, sizeof( KTrackerSensorZip));
    read_sensor( &data );
    outdata[0].x = data.TotalGyroX;
    outdata[0].y = data.TotalGyroY;
    outdata[0].z = data.TotalGyroZ;
  }
}

void GyroscopeSensor::Read( Dali::Vector4& outdata )
{
#if VR_USE_GEARVR_SENSOR == 1
  ReadPackets( &outdata, 1 );
#else
  if( mIsEnabled )
  {
    sensor_event_s eventData;
    int errorcode = sensor_listener_read_data( mListener, &eventData );

    if( !errorcode )
    {
      puts("Obtaining data");

      data[0] = eventData.values[0];
      data[1] = eventData.values[1];
      data[2] = eventData.values[2];
      data[3] = eventData.values[3];
    }
    else
    {
      switch( errorcode )
      {
        case SENSOR_ERROR_INVALID_PARAMETER:    puts("Invalid parameter"); break;
        case SENSOR_ERROR_NOT_SUPPORTED:        puts("The sensor type is not supported in the current device"); break;
        case SENSOR_ERROR_IO_ERROR:             puts("I/O error") ; break;
        case SENSOR_ERROR_OPERATION_FAILED:     puts("Operation failed"); break;
        default:
          puts("Dunno :(");
      }

      printf("Error code: %d\n", errorcode );
    }
  }
#endif
}

bool GyroscopeSensor::IsEnabled()
{
  return mIsEnabled;
}

bool GyroscopeSensor::IsSupported()
{
#if VR_USE_GEARVR_SENSOR == 1
#if VR_USE_GEARVR_SENSOR_PROXY == 1

  return true;

#else // VR_USE_GEARVR_SENSOR
  if( fd >= 0 )
  {
    return true;
  }
  if( open_sensor() )
  {
    return false;
  }
  return true;
#endif    //VR_USE_GEARVR_SENSOR_PROXY
#else // VR_USE_GEARVR_SENSOR
  bool isSupported = false;
  int ret = sensor_is_supported( SENSOR_GYROSCOPE, &isSupported );
  if( !ret )
  {
    return isSupported;
  }
  else
  {
    puts( "sensor_is_supported() failed!");
  }
#endif
  return false;
}


}
}
}
