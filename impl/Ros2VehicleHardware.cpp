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
#include "Ros2VehicleHardware.h"

#include <utils/SystemClock.h>

namespace vendor::spyrosoft::vehicle::impl {

using aidl::android::hardware::automotive::vehicle::GetValueRequest;
using aidl::android::hardware::automotive::vehicle::GetValueResult;
using aidl::android::hardware::automotive::vehicle::RawPropValues;
using aidl::android::hardware::automotive::vehicle::SetValueRequest;
using aidl::android::hardware::automotive::vehicle::SetValueResult;
using aidl::android::hardware::automotive::vehicle::StatusCode;
using aidl::android::hardware::automotive::vehicle::VehiclePropConfig;
using aidl::android::hardware::automotive::vehicle::VehiclePropValue;
using android::hardware::automotive::vehicle::DumpResult;
using android::hardware::automotive::vehicle::VehiclePropertyStore;
using android::hardware::automotive::vehicle::VehiclePropValuePool;
using android::hardware::automotive::vehicle::defaultconfig::ConfigDeclaration;

void Ros2VehicleHardware::storePropInitialValue(const ConfigDeclaration& config)
{
  const VehiclePropConfig& vehiclePropConfig = config.config;
  int propId = vehiclePropConfig.prop;

  // A global property will have only a single area
  bool globalProp = android::hardware::automotive::vehicle::isGlobalProp(propId);
  size_t numAreas = globalProp ? 1 : vehiclePropConfig.areaConfigs.size();

  for (size_t i = 0; i < numAreas; i++) {
    int32_t curArea = globalProp ? 0 : vehiclePropConfig.areaConfigs[i].areaId;

    // Create a separate instance for each individual zone
    VehiclePropValue prop = {
        .areaId = curArea,
        .prop = propId,
        .timestamp = android::elapsedRealtimeNano(),
    };

    if (config.initialAreaValues.empty()) {
      if (config.initialValue == RawPropValues{}) {
        // Skip empty initial values.
        continue;
      }
      prop.value = config.initialValue;
    }
    else if (auto valueForAreaIt = config.initialAreaValues.find(curArea);
             valueForAreaIt != config.initialAreaValues.end()) {
      prop.value = valueForAreaIt->second;
    }
    else {
      ALOGW("failed to get default value for prop 0x%x area 0x%x", propId, curArea);
      continue;
    }

    auto result = mServerSidePropStore->writeValue(mValuePool->obtain(prop), /*updateStatus=*/true);
    if (!result.ok()) {
      ALOGE("failed to write default config value, error: %s, status: %d", getErrorMsg(result).c_str(),
            getIntErrorCode(result));
    }
  }
}

Ros2VehicleHardware::Ros2VehicleHardware() : Ros2VehicleHardware(std::make_unique<VehiclePropValuePool>()) {}

Ros2VehicleHardware::Ros2VehicleHardware(std::unique_ptr<VehiclePropValuePool> valuePool)
    : mValuePool(std::move(valuePool)),
      mServerSidePropStore(std::make_unique<VehiclePropertyStore>(mValuePool)),
      mPendingGetValueRequests(this),
      mPendingSetValueRequests(this)
{
  for (auto& it : android::hardware::automotive::vehicle::defaultconfig::getDefaultConfigs()) {
    mServerSidePropStore->registerProperty(it.config, nullptr);
    storePropInitialValue(it);
  }

  mServerSidePropStore->setOnValueChangeCallback([this](const VehiclePropValue& value) {
    std::scoped_lock<std::mutex> lockGuard(mLock);
    if (!mOnPropertyChangeCallback) {
      return;
    }
    else {
      std::vector<VehiclePropValue> updatedValues;
      updatedValues.push_back(value);
      (*mOnPropertyChangeCallback)(std::move(updatedValues));
    }
  });

  ALOGI("Ros2VehicleHardware created");
}

std::vector<VehiclePropConfig> Ros2VehicleHardware::getAllPropertyConfigs() const
{
  ALOGI("Ros2VehicleHardware::getAllPropertyConfigs");
  return mServerSidePropStore->getAllConfigs();
}

StatusCode Ros2VehicleHardware::setValues(std::shared_ptr<const SetValuesCallback> callback,
                                          const std::vector<SetValueRequest>& requests)
{
  for (auto& request : requests) {
    ALOGD("Set value for property ID: %d", request.value.prop);

    // In a real VHAL implementation, you could either send the setValue request to vehicle bus
    // here in the binder thread, or you could send the request in setValue which runs in
    // the handler thread. If you decide to send the setValue request here, you should not
    // wait for the response here and the handler thread should handle the setValue response.
    mPendingSetValueRequests.addRequest(request, callback);
  }

  return StatusCode::OK;
}

StatusCode Ros2VehicleHardware::getValues(std::shared_ptr<const GetValuesCallback> callback,
                                          const std::vector<GetValueRequest>& requests) const
{
  for (auto& request : requests) {
    ALOGD("getValues(%d)", request.prop.prop);

    // In a real VHAL implementation, you could either send the getValue request to vehicle bus
    // here in the binder thread, or you could send the request in getValue which runs in
    // the handler thread. If you decide to send the getValue request here, you should not
    // wait for the response here and the handler thread should handle the getValue response.
    mPendingGetValueRequests.addRequest(request, callback);
  }

  return StatusCode::OK;
}

DumpResult Ros2VehicleHardware::dump(const std::vector<std::string>& /*options*/)
{
  ALOGI("Ros2VehicleHardware::dump");
  return DumpResult{true, std::string{""}};
}

StatusCode Ros2VehicleHardware::checkHealth()
{
  ALOGI("Ros2VehicleHardware::checkHealth");
  return StatusCode::OK;
}

void Ros2VehicleHardware::registerOnPropertyChangeEvent(std::unique_ptr<const PropertyChangeCallback> /*callback*/)
{
  ALOGI("Ros2VehicleHardware::registerOnPropertyChangeEvent");
}

void Ros2VehicleHardware::registerOnPropertySetErrorEvent(std::unique_ptr<const PropertySetErrorCallback> /*callback*/)
{
  ALOGI("Ros2VehicleHardware::registerOnPropertySetErrorEvent");
}

StatusCode Ros2VehicleHardware::updateSampleRate(int32_t /*propId*/, int32_t /*areaId*/, float /*sampleRate*/)
{
  ALOGI("Ros2VehicleHardware::updateSampleRate");
  return StatusCode::OK;
}

GetValueResult Ros2VehicleHardware::handleGetValueRequest(const GetValueRequest& request)
{
  GetValueResult getValueResult;
  getValueResult.requestId = request.requestId;

  auto readResult = mServerSidePropStore->readValue(request.prop);
  if (!readResult.ok()) {
    getValueResult.status = StatusCode::INTERNAL_ERROR;
  }
  else {
    getValueResult.status = StatusCode::OK;
    getValueResult.prop = *readResult.value();
  }
  return getValueResult;
}

SetValueResult Ros2VehicleHardware::handleSetValueRequest(const SetValueRequest& request)
{
  SetValueResult setValueResult;
  setValueResult.requestId = request.requestId;

  auto updatedValue = mValuePool->obtain(request.value);
  updatedValue->timestamp = android::elapsedRealtimeNano();

  auto writeResult = mServerSidePropStore->writeValue(std::move(updatedValue));
  if (!writeResult.ok()) {
    setValueResult.status = StatusCode::INTERNAL_ERROR;
  }
  else {
    setValueResult.status = StatusCode::OK;
  }

  return setValueResult;
}

template <class CallbackType, class RequestType>
Ros2VehicleHardware::PendingRequestHandler<CallbackType, RequestType>::PendingRequestHandler(
    Ros2VehicleHardware* hardware)
    : mHardware(hardware)
{
  // Don't initialize mThread in initialization list because mThread depends on mRequests and we
  // want mRequests to be initialized first.
  mThread = std::thread([this] {
    while (mRequests.waitForItems()) {
      handleRequestsOnce();
    }
  });
}

template <class CallbackType, class RequestType>
void Ros2VehicleHardware::PendingRequestHandler<CallbackType, RequestType>::addRequest(
    RequestType request, std::shared_ptr<const CallbackType> callback)
{
  mRequests.push({
      request,
      callback,
  });
}

template <class CallbackType, class RequestType>
void Ros2VehicleHardware::PendingRequestHandler<CallbackType, RequestType>::stop()
{
  mRequests.deactivate();
  if (mThread.joinable()) {
    mThread.join();
  }
}

template <>
void Ros2VehicleHardware::PendingRequestHandler<Ros2VehicleHardware::GetValuesCallback,
                                                GetValueRequest>::handleRequestsOnce()
{
  std::unordered_map<std::shared_ptr<const GetValuesCallback>, std::vector<GetValueResult>> callbackToResults;
  for (const auto& rwc : mRequests.flush()) {
    auto result = mHardware->handleGetValueRequest(rwc.request);
    callbackToResults[rwc.callback].push_back(std::move(result));
  }
  for (const auto& [callback, results] : callbackToResults) {
    (*callback)(std::move(results));
  }
}

template <>
void Ros2VehicleHardware::PendingRequestHandler<Ros2VehicleHardware::SetValuesCallback,
                                                SetValueRequest>::handleRequestsOnce()
{
  std::unordered_map<std::shared_ptr<const SetValuesCallback>, std::vector<SetValueResult>> callbackToResults;
  for (const auto& rwc : mRequests.flush()) {
    auto result = mHardware->handleSetValueRequest(rwc.request);
    callbackToResults[rwc.callback].push_back(std::move(result));
  }
  for (const auto& [callback, results] : callbackToResults) {
    (*callback)(std::move(results));
  }
}

}  // namespace vendor::spyrosoft::vehicle::impl
