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
#include "RosVehicleHalServer.h"

#include <android/log.h>

namespace vendor {
namespace spyrosoft {
namespace vehicle {

namespace impl {

Ros2VehicleHalServer::Ros2VehicleHalServer() {  }

std::vector<VehiclePropConfig> Ros2VehicleHalServer::onGetAllPropertyConfig() const { return {}; }

StatusCode Ros2VehicleHalServer::onSetProperty(const VehiclePropValue& value, bool updateStatus) {
    (void)value;
    (void)updateStatus;

    // In the real vhal, the value will be sent to Car ECU.
    // We just pretend it is done here and send back to HAL
    // auto updatedPropValue = getValuePool()->obtain(value);
    // updatedPropValue->timestamp = elapsedRealtimeNano();

    // mServerSidePropStore.writeValue(*updatedPropValue, updateStatus);
    // onPropertyValueFromCar(*updatedPropValue, updateStatus);
    return StatusCode::OK;
}

void Ros2VehicleHalServer::onPropertyValueFromCar(const VehiclePropValue& value, bool updateStatus) {
    (void)value;
    (void)updateStatus;
}

IVehicleServer::DumpResult Ros2VehicleHalServer::onDump(const std::vector<std::string>& options) {
    DumpResult result;

    if (options.empty()) {
        // No options, dump all stored properties.
        result.callerShouldDumpState = true;
        result.buffer += "Server side properties: \n";
        // auto values = mServerSidePropStore.readAllValues();
        // size_t i = 0;
        // for (const auto& value : values) {
        //     result.buffer += fmt::format("[{}]: {}\n", i, toString(value));
        //     i++;
        // }
        return result;
    } else {
        // Handle custom options
    }

    return result;
}

}  // namespace impl

}  // namespace V2_0
}  // namespace vehicle
}  // namespace automotive