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

#include "Ros2Bridge.h"

#include <aidl/android/hardware/automotive/vehicle/VehicleProperty.h>
#include <rcl/error_handling.h>
#include <rmw_microros/discovery.h>
#include <rmw_microros/error_handling.h>
#include <rmw_microros/ping.h>
#include <rmw_microros/rmw_microros.h>
#include <rosidl_runtime_c/primitives_sequence_functions.h>
#include <rosidl_runtime_c/string_functions.h>
#include <unistd.h>

#include "common/logging.hpp"

#define RCCHECK(fn)                                                               \
  {                                                                               \
    rcl_ret_t temp_rc = fn;                                                       \
    if ((temp_rc != RCL_RET_OK)) {                                                \
      ALOGE("Failed status on line %d: %d. Aborting.\n", __LINE__, (int)temp_rc); \
      abort();                                                                    \
    }                                                                             \
  }
#define RCSOFTCHECK(fn)                                                             \
  {                                                                                 \
    rcl_ret_t temp_rc = fn;                                                         \
    if ((temp_rc != RCL_RET_OK)) {                                                  \
      ALOGE("Failed status on line %d: %d. Continuing.\n", __LINE__, (int)temp_rc); \
    }                                                                               \
  }

// helper types for the visitor
template <class... Ts>
struct overload : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
overload(Ts...) -> overload<Ts...>;

static ros2_android_vhal__srv__SetVehicleProperty_Response set_vehicle_property_resp;

void set_vehicle_property_callback(const void *msg)
{
  auto *resp = static_cast<const ros2_android_vhal__srv__SetVehicleProperty_Response *>(msg);
  ALOGD("set_vehicle_property_callback result: %d", resp->result);
}

namespace vendor::spyrosoft::vehicle::ros2 {

ROS2Bridge::ROS2Bridge()
    : m_init_options(rcl_get_zero_initialized_init_options()),
      m_allocator(rcutils_get_default_allocator()),
      m_node(rcl_get_zero_initialized_node()),
      m_executor(rclc_executor_get_zero_initialized_executor()),
      m_vehiclePropertyClient(rcl_get_zero_initialized_client())
{
  RCCHECK(rcl_init_options_init(&m_init_options, m_allocator));
  ALOGI("ROS 2 Bridge created");
}

ROS2Bridge::~ROS2Bridge()
{
  stop();
  destroyEntities();
  RCSOFTCHECK(rcl_init_options_fini(&m_init_options));
}

void ROS2Bridge::createEntities()
{
  ALOGI("ROS2Bridge - initializing node entities");

  if (m_rmw_options != nullptr) {
    RCCHECK(rclc_support_init_with_options(&m_support, 0, nullptr, &m_init_options, &m_allocator));
  }
  else {
    RCCHECK(rclc_support_init(&m_support, 0, nullptr, &m_allocator));
  }

  RCCHECK(rclc_node_init_default(&m_node, "android_vhal_node", "", &m_support));

  RCCHECK(rclc_client_init_default(&m_vehiclePropertyClient, &m_node,
                                   ROSIDL_GET_SRV_TYPE_SUPPORT(ros2_android_vhal, srv, SetVehicleProperty),
                                   "/set_vehicle_property"));

  RCCHECK(rclc_executor_init(&m_executor, &m_support.context, 1, &m_allocator));

  RCCHECK(rclc_executor_add_client(&m_executor, &m_vehiclePropertyClient, &set_vehicle_property_resp,
                                   set_vehicle_property_callback));
}

void ROS2Bridge::destroyEntities()
{
  ALOGI("ROS2Bridge - destroying node entities");

  RCSOFTCHECK(rclc_executor_fini(&m_executor));
  RCSOFTCHECK(rcl_client_fini(&m_vehiclePropertyClient, &m_node));
  RCSOFTCHECK(rcl_node_fini(&m_node));
  RCSOFTCHECK(rclc_support_fini(&m_support));
}

bool ROS2Bridge::pingAgent()
{
  if (m_rmw_options != nullptr) {
    return (rmw_uros_ping_agent_options(250, 5, m_rmw_options) == RMW_RET_OK);
  }
  else {
    return (rmw_uros_ping_agent(250, 5) == RMW_RET_OK);
  }
}

bool ROS2Bridge::setProperty(int64_t timestamp, int32_t areaId, int32_t propId, PropertyValue_t value)
{
  if (propId != 0x15400500) {
    return true;
  }

  std::lock_guard<std::mutex> lock(m_clientMutex);

  ros2_android_vhal__srv__SetVehicleProperty_Request req;
  ros2_android_vhal__srv__SetVehicleProperty_Request__init(&req);
  req.prop.timestamp = timestamp;
  req.prop.area_id = areaId;
  req.prop.prop_id = propId;

  // clang-format off
  std::visit(overload{
    [&](int64_t) {
      rosidl_runtime_c__int64__Sequence__init(&req.prop.int64_values, 1UL);
      req.prop.int64_values.data[0] = std::get<int64_t>(value);
    },
    [&](int32_t) { 
      rosidl_runtime_c__int32__Sequence__init(&req.prop.int32_values, 1UL);
      req.prop.int32_values.data[0] = std::get<int32_t>(value);
    },
    [&](uint8_t) { 
      rosidl_runtime_c__uint8__Sequence__init(&req.prop.uint8_values, 1UL);
      req.prop.uint8_values.data[0] = std::get<uint8_t>(value);
    },
    [&](float) { 
      rosidl_runtime_c__float__Sequence__init(&req.prop.float_values, 1UL);
      req.prop.float_values.data[0] = std::get<float>(value);
    },
    [&](std::string&) { 
      rosidl_runtime_c__String__Sequence__init(&req.prop.string_values, 1UL);
      rosidl_runtime_c__String__assignn(&req.prop.string_values.data[0], std::get<std::string>(value).c_str(), std::get<std::string>(value).size());
    }},
    value);
  // clang-format on

  int64_t sequence_number;
  const auto send_result = rcl_send_request(&m_vehiclePropertyClient, &req, &sequence_number);
  ros2_android_vhal__srv__SetVehicleProperty_Request__fini(&req);

  if (send_result != RMW_RET_OK) {
    ALOGE("rcl_send_request setProperty(%d) error", propId);
    return false;
  }
  else {
    ALOGD("rcl_send_request setProperty(%d) sent", propId);
  }

  return true;
}

void ROS2Bridge::start(std::chrono::seconds timeout)
{
  m_thread = std::thread([this, timeout]() {
    std::this_thread::sleep_for(timeout);

    while (m_running) {
      switch (m_AgentState) {
        case AgentConnectionState::DISCONNECTED:
          m_rmw_options = rcl_init_options_get_rmw_init_options(&m_init_options);
          ALOGD("ROS2Bridge - discovery agent");
          if (rmw_uros_discover_agent(m_rmw_options) == RMW_RET_OK) {
            ALOGD("ROS2Bridge - agent discovered");
          }
          else {
            m_rmw_options = nullptr;
          }

          if (pingAgent()) {
            ALOGD("ROS2Bridge - agent found");
            createEntities();
            m_AgentState = AgentConnectionState::CONNECTED;
          }
          else {
            ALOGD("ROS2Bridge - agent not found");
          }
          break;
        case AgentConnectionState::CONNECTED:
          if (!pingAgent()) {
            ALOGD("ROS2Bridge - agent lost");
            destroyEntities();
            m_AgentState = AgentConnectionState::DISCONNECTED;
          }
          else {
            RCSOFTCHECK(rclc_executor_spin_some(&m_executor, RCL_MS_TO_NS(100)));
          }
          break;
      }
    }

    destroyEntities();
    ALOGD("ROS2Bridge - Thread stopped");
  });
}

}  // namespace vendor::spyrosoft::vehicle::ros2
