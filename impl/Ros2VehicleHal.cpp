/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "Ros2VehicleHal.h"

#include <android/log.h>

namespace vendor {
namespace spyrosoft {
namespace vehicle {

namespace impl {

Ros2VehicleHal::Ros2VehicleHal(vhal_2_0::VehiclePropertyStore* propStore, vhal_2_0::IVehicleClient* client)
    : mPropStore{propStore}, mVehicleClient{client} {
    (void)mPropStore;
    (void)mVehicleClient;
}

std::vector<vhal_2_0::VehiclePropConfig> Ros2VehicleHal::listProperties() { return {}; }

vhal_2_0::VehicleHal::VehiclePropValuePtr Ros2VehicleHal::get(const vhal_2_0::VehiclePropValue& requestedPropValue,
                                                              vhal_2_0::StatusCode* outStatus) {
    (void)requestedPropValue;
    (void)outStatus;
    return nullptr;
}

vhal_2_0::StatusCode Ros2VehicleHal::set(const vhal_2_0::VehiclePropValue& propValue) {
    (void)propValue;
    return vhal_2_0::StatusCode::OK;
}

vhal_2_0::StatusCode Ros2VehicleHal::subscribe(int32_t property, float sampleRate) {
    (void)property;
    (void)sampleRate;
    return vhal_2_0::StatusCode::OK;
}

vhal_2_0::StatusCode Ros2VehicleHal::unsubscribe(int32_t property) {
    (void)property;
    return vhal_2_0::StatusCode::OK;
}

void Ros2VehicleHal::onCreate() {}

void Ros2VehicleHal::onPropertyValue(const vhal_2_0::VehiclePropValue& value, bool updateStatus) {
    (void)value;
    (void)updateStatus;
}

}  // namespace impl

}  // namespace vehicle
}  // namespace spyrosoft
}  // namespace vendor
