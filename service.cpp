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
#define LOG_TAG "automotive.vehicle@2.0-service"

#include <android/log.h>
#include <hidl/HidlTransportSupport.h>
#include <vhal_v2_0/DefaultVehicleConnector.h>
#include <vhal_v2_0/DefaultVehicleHalServer.h>
#include <vhal_v2_0/VehicleHalManager.h>

#include <iostream>

#include "impl/Ros2VehicleHal.h"

namespace vhal_v2_0 = android::hardware::automotive::vehicle::V2_0;

using vendor::spyrosoft::vehicle::impl::Ros2VehicleHal;

int main(int /* argc */, char* /* argv */[]) {
    auto store = std::make_unique<vhal_v2_0::VehiclePropertyStore>();
    auto connector = std::make_unique<vhal_v2_0::impl::DefaultVehicleConnector>();
    auto hal = std::make_unique<Ros2VehicleHal>(store.get(), connector.get());
    auto service = std::make_unique<vhal_v2_0::VehicleHalManager>(hal.get());
    connector->setValuePool(hal->getValuePool());

    android::hardware::configureRpcThreadpool(4, true /* callerWillJoin */);

    ALOGI("Registering as service...");
    android::status_t status = service->registerAsService();

    if (status != android::OK) {
        ALOGE("Unable to register vehicle service (%d)", status);
        return 1;
    }

    ALOGI("Ready");
    android::hardware::joinRpcThreadpool();

    return 1;
}
