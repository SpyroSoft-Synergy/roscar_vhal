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

#ifndef IMPL_VHAL_V2_0_ROSVEHICLEHALSERVER
#define IMPL_VHAL_V2_0_ROSVEHICLEHALSERVER

#include <vhal_v2_0/VehicleObjectPool.h>
#include <vhal_v2_0/VehiclePropertyStore.h>
#include <vhal_v2_0/VehicleServer.h>

namespace vendor {
namespace spyrosoft {
namespace vehicle {

namespace impl {

// This contains the server operation for VHAL running in emulator.
class Ros2VehicleHalServer : public IVehicleServer {
   public:
    Ros2VehicleHalServer();

    std::vector<VehiclePropConfig> onGetAllPropertyConfig() const override;

    StatusCode onSetProperty(const VehiclePropValue& value, bool updateStatus) override;

    void onPropertyValueFromCar(const VehiclePropValue& value, bool updateStatus) override;

    DumpResult onDump(const std::vector<std::string>& options) override;
};

}  // namespace impl

}  // namespace V2_0
}  // namespace vehicle
}  // namespace automotive

#endif // IMPL_VHAL_V2_0_ROSVEHICLEHALSERVER
