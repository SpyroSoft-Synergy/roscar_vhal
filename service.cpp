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
#include "common/logging.hpp"

#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <DefaultVehicleHal.h>

#include "impl/Ros2VehicleHal.h"
#include "impl/Ros2VehicleHardware.h"
#include "ros2_bridge.hpp"
#include "ros2_logger.hpp"

using android::hardware::automotive::vehicle::DefaultVehicleHal;
using vendor::spyrosoft::vehicle::Logger;
using vendor::spyrosoft::vehicle::ROS2Bridge;
using vendor::spyrosoft::vehicle::impl::Ros2VehicleHal;
using vendor::spyrosoft::vehicle::impl::Ros2VehicleHardware;

int main(int /* argc */, char* /* argv */[])
{
  auto hardware = std::make_unique<Ros2VehicleHardware>();
  auto vhal = ::ndk::SharedRefBase::make<DefaultVehicleHal>(std::move(hardware));

  ALOGI("Registering as service...");
  auto err = AServiceManager_addService(vhal->asBinder().get(), "android.hardware.automotive.vehicle.IVehicle/default");
  if (err != EX_NONE) {
    ALOGE("failed to register android.hardware.automotive.vehicle service, exception: %d", err);
    return 1;
  }

  if (!ABinderProcess_setThreadPoolMaxThreadCount(4)) {
    ALOGE("%s", "failed to set thread pool max thread count");
    return 1;
  }
  ABinderProcess_startThreadPool();

  ALOGI("Creating ROS 2 logger");
  Logger logger{};

  ALOGI("Creating ROS 2 Bridge");
  ROS2Bridge bridge{};
  bridge.run();

  ALOGI("Vehicle Service Ready");
  ABinderProcess_joinThreadPool();

  ALOGI("Vehicle Service Exiting");
  return 0;
}