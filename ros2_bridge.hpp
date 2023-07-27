/*
 * Copyright (c) 2023 Spyrosoft Synergy S.A.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef AUTOMOTIVE_VEHICLE_ROS2_BRIDGE
#define AUTOMOTIVE_VEHICLE_ROS2_BRIDGE

#include <atomic>
#include <mutex>
#include <thread>

#include <vhal_v2_0/VehicleClient.h>

namespace vendor::spyrosoft::vehicle {

/**
 * @brief
 *
 */
class ROS2Bridge : public android::hardware::automotive::vehicle::V2_0::IVehicleClient {
   public:
    ROS2Bridge() = default;
    virtual ~ROS2Bridge();

    std::vector<android::hardware::automotive::vehicle::V2_0::VehiclePropConfig> getAllPropertyConfig() const override;

    android::hardware::automotive::vehicle::V2_0::StatusCode setProperty(
        const android::hardware::automotive::vehicle::V2_0::VehiclePropValue& value, bool updateStatus) override;
    
    void onPropertyValue(const android::hardware::automotive::vehicle::V2_0::VehiclePropValue& value, bool updateStatus) override;

   private:
    void run();

    std::thread mThread;
    std::atomic_bool mRunning{true};
};

}  // namespace vendor::spyrosoft::vehicle

#endif /* AUTOMOTIVE_VEHICLE_ROS2_BRIDGE */
