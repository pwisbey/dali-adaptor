#ifndef __DALI_INTERNAL_ADAPTOR_ENVIRONMENT_OPTIONS_H__
#define __DALI_INTERNAL_ADAPTOR_ENVIRONMENT_OPTIONS_H__

/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

/**
 * Contains environment options which define settings and the ability to install a log function.
 */
class EnvironmentOptions
{

public:

  /**
   * Constructor
   */
  EnvironmentOptions();

  /**
   * non-virtual destructor, not intended as a base class
   */
  ~EnvironmentOptions();

  /**
   * @param logFunction logging function
   * @param logFilterOptions bitmask of the logging options defined in intergration/debug.h (e.g.
   * @param logFrameRateFrequency frequency of how often FPS is logged out (e.g. 0 = off, 2 = every 2 seconds).
   * @param logupdateStatusFrequency frequency of how often the update status is logged in number of frames
   * @param logPerformanceLevel performance logging, 0 = disabled,  1+ =  enabled
   * @param logPanGestureLevel pan-gesture logging, 0 = disabled,  1 = enabled
   */
  void SetLogOptions( const Dali::Integration::Log::LogFunction& logFunction,
                       unsigned int logFrameRateFrequency,
                       unsigned int logupdateStatusFrequency,
                       unsigned int logPerformanceLevel,
                       unsigned int logPanGestureLevel );

  /**
   * Install the log function for the current thread.
   */
  void InstallLogFunction() const;

  /**
   * Un-install the log function for the current thread.
   */
  void UnInstallLogFunction() const;

  /**
   * @return frequency of how often FPS is logged out (e.g. 0 = off, 2 = every 2 seconds).
   */
  unsigned int GetFrameRateLoggingFrequency() const;

  /**
   * @return frequency of how often Update Status is logged out (e.g. 0 = off, 60 = log every 60 frames = 1 second @ 60FPS).
   */
  unsigned int GetUpdateStatusLoggingFrequency() const;

  /**
   * @return logPerformanceLevel performance log level ( 0 = off )
   */
  unsigned int GetPerformanceLoggingLevel() const;

  /**
   * @return pan-gesture logging level ( 0 == off )
   */
  unsigned int GetPanGestureLoggingLevel() const;

  /**
   * @return pan-gesture prediction mode ( -1 means not set so no prediction, 0 = no prediction )
   */
  int GetPanGesturePredictionMode() const;

  /**
   * @return pan-gesture prediction amount
   */
  float GetPanGesturePredictionAmount() const;

  /**
   * @return pan-gesture smoothing mode ( -1 means not set so no smoothing, 0 = no smoothing )
   */
  int GetPanGestureSmoothingMode() const;

  /**
   * @return pan-gesture smoothing amount
   */
  float GetPanGestureSmoothingAmount() const;

  /**
   * @return The minimum distance before a pan can be started (-1 means it's not set)
   */
  int GetMinimumPanDistance() const;

  /**
   * @return The minimum events before a pan can be started (-1 means it's not set)
   */
  int GetMinimumPanEvents() const;

  /**
   * @brief Sets the mode used to smooth pan gesture movement properties calculated on the Update thread
   *
   * @param[in] mode The smoothing mode to use
   */
  void SetPanGesturePredictionMode( unsigned int mode );

  /**
   * @brief Sets the prediction amount of the pan gesture
   *
   * @param[in] amount The prediction amount in milliseconds
   */
  void SetPanGesturePredictionAmount( unsigned int amount );

  /**
   * @brief Called to set how pan gestures smooth input
   *
   * @param[in] mode The smoothing mode to use
   */
  void SetPanGestureSmoothingMode( unsigned int mode );

  /**
   * @brief Sets the prediction amount of the pan gesture
   *
   * @param[in] amount The smoothing amount [0.0f,1.0f] - 0.0f would be no smoothing, 1.0f maximum smoothing
   */
  void SetPanGestureSmoothingAmount( float amount );

  /**
   * @brief Sets the minimum distance required before a pan starts
   *
   * @param[in] distance The minimum distance before a pan starts
   */
  void SetMinimumPanDistance( int distance );

  /**
   * @brief Sets the minimum number of events required before a pan starts
   *
   * @param[in] events The minimum events before a pan starts
   */
  void SetMinimumPanEvents( int events );

private:

  unsigned int mFpsFrequency;                     ///< how often fps is logged out in seconds
  unsigned int mUpdateStatusFrequency;            ///< how often update status is logged out in frames
  unsigned int mPerformanceLoggingLevel;          ///< performance log level
  unsigned int mPanGestureLoggingLevel;           ///< pan-gesture log level
  int mPanGesturePredictionMode;                  ///< prediction mode for pan gestures
  float mPanGesturePredictionAmount;              ///< prediction amount for pan gestures
  int mPanGestureSmoothingMode;                  ///< prediction mode for pan gestures
  float mPanGestureSmoothingAmount;              ///< prediction amount for pan gestures
  int mPanMinimumDistance;                        ///< minimum distance required before pan starts
  int mPanMinimumEvents;                          ///< minimum events required before pan starts

  Dali::Integration::Log::LogFunction mLogFunction;

  // Undefined copy constructor.
  EnvironmentOptions( const EnvironmentOptions& );

  // Undefined assignment operator.
  EnvironmentOptions& operator=( const EnvironmentOptions& );

};

} // Adaptor
} // Internal
} // Dali

#endif // __DALI_INTERNAL_ADAPTOR_ENVIRONMENT_OPTIONS_H__
