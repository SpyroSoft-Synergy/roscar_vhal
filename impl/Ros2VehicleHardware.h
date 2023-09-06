/*
 * Copyright (c) 2023 Spyrosoft Synergy S.A.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "Ros2Bridge.h"

#include <ConcurrentQueue.h>
#include <IVehicleHardware.h>
#include <VehiclePropertyStore.h>
#include <DefaultConfig.h>

#include <memory>
#include <vector>
#include <mutex>

namespace vendor::spyrosoft::vehicle {

/**
 * @brief
 *
 */
class Ros2VehicleHardware : public android::hardware::automotive::vehicle::IVehicleHardware {
  /**
   * @brief
   *
   * @tparam CallbackType
   * @tparam RequestType
   */
  template <class CallbackType, class RequestType>
  struct RequestWithCallback {
    RequestType request;
    std::shared_ptr<const CallbackType> callback;
  };

  /**
   * @brief
   *
   * @tparam CallbackType
   * @tparam RequestType
   */
  template <class CallbackType, class RequestType>
  class PendingRequestHandler {
   public:
    PendingRequestHandler(Ros2VehicleHardware* hardware);

    void addRequest(RequestType request, std::shared_ptr<const CallbackType> callback);

    void stop();

   private:
    Ros2VehicleHardware* mHardware;
    std::thread mThread;
    android::hardware::automotive::vehicle::ConcurrentQueue<RequestWithCallback<CallbackType, RequestType>> mRequests;

    void handleRequestsOnce();
  };

 public:
  explicit Ros2VehicleHardware(std::unique_ptr<ros2::ROS2Bridge> ros_bridge);
  ~Ros2VehicleHardware() override = default;

  // Get all the property configs.
  std::vector<aidl::android::hardware::automotive::vehicle::VehiclePropConfig> getAllPropertyConfigs() const override;

  // Set property values asynchronously. Server could return before the property set requests
  // are sent to vehicle bus or before property set confirmation is received. The callback is
  // safe to be called after the function returns and is safe to be called in a different thread.
  aidl::android::hardware::automotive::vehicle::StatusCode setValues(
      std::shared_ptr<const SetValuesCallback> callback,
      const std::vector<aidl::android::hardware::automotive::vehicle::SetValueRequest>& requests) override;

  // Get property values asynchronously. Server could return before the property values are ready.
  // The callback is safe to be called after the function returns and is safe to be called in a
  // different thread.
  aidl::android::hardware::automotive::vehicle::StatusCode getValues(
      std::shared_ptr<const GetValuesCallback> callback,
      const std::vector<aidl::android::hardware::automotive::vehicle::GetValueRequest>& requests) const override;

  // Dump debug information in the server.
  android::hardware::automotive::vehicle::DumpResult dump(const std::vector<std::string>& options) override;

  // Check whether the system is healthy, return {@code StatusCode::OK} for healthy.
  aidl::android::hardware::automotive::vehicle::StatusCode checkHealth() override;

  // Register a callback that would be called when there is a property change event from vehicle.
  void registerOnPropertyChangeEvent(std::unique_ptr<const PropertyChangeCallback> callback) override;

  // Register a callback that would be called when there is a property set error event from
  // vehicle.
  void registerOnPropertySetErrorEvent(std::unique_ptr<const PropertySetErrorCallback> callback) override;

  // Update the sample rate for the [propId, areaId] pair.
  aidl::android::hardware::automotive::vehicle::StatusCode updateSampleRate(int32_t propId, int32_t areaId,
                                                                            float sampleRate) override;

 protected:
  void storePropInitialValue(const android::hardware::automotive::vehicle::defaultconfig::ConfigDeclaration& config);

  aidl::android::hardware::automotive::vehicle::GetValueResult handleGetValueRequest(
      const aidl::android::hardware::automotive::vehicle::GetValueRequest& request);

  aidl::android::hardware::automotive::vehicle::SetValueResult handleSetValueRequest(
      const aidl::android::hardware::automotive::vehicle::SetValueRequest& request);

 protected:
  std::unique_ptr<ros2::ROS2Bridge> mRos2Bridge;

  const std::shared_ptr<android::hardware::automotive::vehicle::VehiclePropValuePool> mValuePool;
  const std::unique_ptr<android::hardware::automotive::vehicle::VehiclePropertyStore> mServerSidePropStore;

  std::mutex mLock;
  std::unique_ptr<const PropertyChangeCallback> mOnPropertyChangeCallback;
  std::unique_ptr<const PropertySetErrorCallback> mOnPropertySetErrorCallback;

  mutable PendingRequestHandler<IVehicleHardware::GetValuesCallback,
                                aidl::android::hardware::automotive::vehicle::GetValueRequest>
      mPendingGetValueRequests;
  mutable PendingRequestHandler<IVehicleHardware::SetValuesCallback,
                                aidl::android::hardware::automotive::vehicle::SetValueRequest>
      mPendingSetValueRequests;
};

}  // namespace vendor::spyrosoft::vehicle
