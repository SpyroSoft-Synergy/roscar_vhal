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

#ifndef VEHICLE_IMPL_ROS2VEHICLEHAL
#define VEHICLE_IMPL_ROS2VEHICLEHAL

#include <vhal_v2_0/VehicleClient.h>
#include <vhal_v2_0/VehicleHal.h>
#include <vhal_v2_0/VehiclePropertyStore.h>

namespace vendor {
namespace spyrosoft {
namespace vehicle {

namespace vhal_2_0 = android::hardware::automotive::vehicle::V2_0;

namespace impl {

class Ros2VehicleHal : public vhal_2_0::VehicleHal {
   public:
    Ros2VehicleHal(vhal_2_0::VehiclePropertyStore* propStore, vhal_2_0::IVehicleClient* client);
    virtual ~Ros2VehicleHal() = default;

    std::vector<vhal_2_0::VehiclePropConfig> listProperties() override;

    VehicleHal::VehiclePropValuePtr get(const vhal_2_0::VehiclePropValue& requestedPropValue,
                                        vhal_2_0::StatusCode* outStatus) override;

    vhal_2_0::StatusCode set(const vhal_2_0::VehiclePropValue& propValue) override;

    vhal_2_0::StatusCode subscribe(int32_t property, float sampleRate) override;

    vhal_2_0::StatusCode unsubscribe(int32_t property) override;

    void onCreate() override;

   private:
    void onPropertyValue(const vhal_2_0::VehiclePropValue& value, bool updateStatus);

   private:
    vhal_2_0::VehiclePropertyStore* mPropStore;
    vhal_2_0::IVehicleClient* mVehicleClient;
};

}  // namespace impl

}  // namespace vehicle
}  // namespace spyrosoft
}  // namespace vendor

#endif /* VEHICLE_IMPL_ROS2VEHICLEHAL */
