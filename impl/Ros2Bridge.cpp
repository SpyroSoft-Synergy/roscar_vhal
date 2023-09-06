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

#include <rcl/error_handling.h>
#include <rmw_microros/error_handling.h>
#include <rmw_microros/discovery.h>
#include <rmw_microros/ping.h>
#include <rmw_microros/rmw_microros.h>
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

namespace {
void setInt64PropertyClient_callback(const void* msg)
{
  if (msg != nullptr) {
    auto* msgin = static_cast<const ros2_android_vhal__srv__VhalSetInt64Property_Response*>(msg);
    ALOGI("ROS2Bridge - setInt64PropertyClient_callback: value == %ld", msgin->actual_value);
  }
  else {
    ALOGE("ROS2Bridge - setInt64PropertyClient_callback error msg == nullptr");
  }
}

void setInt32PropertyClient_callback(const void* msg)
{
  if (msg != nullptr) {
    auto* msgin = static_cast<const ros2_android_vhal__srv__VhalSetInt32Property_Response*>(msg);
    ALOGI("ROS2Bridge - setInt32PropertyClient_callback: value == %d", msgin->actual_value);
  }
  else {
    ALOGE("ROS2Bridge - setInt32PropertyClient_callback error msg == nullptr");
  }
}

void setFloatPropertyClient_callback(const void* msg)
{
  if (msg != nullptr) {
    auto* msgin = static_cast<const ros2_android_vhal__srv__VhalSetFloatProperty_Response*>(msg);
    ALOGI("ROS2Bridge - setIntFloatPropertyClient_callback: value == %f", msgin->actual_value);
  }
  else {
    ALOGE("ROS2Bridge - setIntFloatPropertyClient_callback error msg == nullptr");
  }
}

void setUint8PropertyClient_callback(const void* msg)
{
  if (msg != nullptr) {
    auto* msgin = static_cast<const ros2_android_vhal__srv__VhalSetUint8Property_Response*>(msg);
    ALOGI("ROS2Bridge - setUint8PropertyClient_callback: value == %u", msgin->actual_value);
  }
  else {
    ALOGE("ROS2Bridge - setUint8PropertyClient_callback error msg == nullptr");
  }
}

void setStringPropertyClient_callback(const void* msg)
{
  if (msg != nullptr) {
    auto* msgin = static_cast<const ros2_android_vhal__srv__VhalSetStringProperty_Response*>(msg);
    ALOGI("ROS2Bridge - setStringPropertyClient_callback: value == %s", msgin->actual_value.data);
  }
  else {
    ALOGE("ROS2Bridge - setStringPropertyClient_callback error msg == nullptr");
  }
}

}  // namespace

namespace vendor::spyrosoft::vehicle::ros2 {

ROS2Bridge::ROS2Bridge()
    : m_init_options(rcl_get_zero_initialized_init_options()),
      m_allocator(rcutils_get_default_allocator()),
      m_node(rcl_get_zero_initialized_node()),
      m_setInt64PropertyClient(rcl_get_zero_initialized_client()),
      m_setInt32PropertyClient(rcl_get_zero_initialized_client()),
      m_setUint8PropertyClient(rcl_get_zero_initialized_client()),
      m_setFloatPropertyClient(rcl_get_zero_initialized_client()),
      m_setStringPropertyClient(rcl_get_zero_initialized_client())
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

  RCCHECK(rclc_client_init_default(&m_setInt64PropertyClient, &m_node,
                                   ROSIDL_GET_SRV_TYPE_SUPPORT(ros2_android_vhal, srv, VhalSetInt64Property),
                                   "/VhalSetInt64Property"));
  RCCHECK(rclc_client_init_default(&m_setInt32PropertyClient, &m_node,
                                   ROSIDL_GET_SRV_TYPE_SUPPORT(ros2_android_vhal, srv, VhalSetInt32Property),
                                   "/VhalSetInt32Property"));
  RCCHECK(rclc_client_init_default(&m_setFloatPropertyClient, &m_node,
                                   ROSIDL_GET_SRV_TYPE_SUPPORT(ros2_android_vhal, srv, VhalSetFloatProperty),
                                   "/VhalSetFloatProperty"));
  RCCHECK(rclc_client_init_default(&m_setUint8PropertyClient, &m_node,
                                   ROSIDL_GET_SRV_TYPE_SUPPORT(ros2_android_vhal, srv, VhalSetUint8Property),
                                   "/VhalSetUint8Property"));
  RCCHECK(rclc_client_init_default(&m_setStringPropertyClient, &m_node,
                                   ROSIDL_GET_SRV_TYPE_SUPPORT(ros2_android_vhal, srv, VhalSetStringProperty),
                                   "/VhalSetStringProperty"));

  RCCHECK(rclc_executor_init(&m_executor, &m_support.context, 1, &m_allocator));

  RCCHECK(rclc_executor_add_client(&m_executor, &m_setInt64PropertyClient, &m_setInt64Resp,
                                   setInt64PropertyClient_callback));
  RCCHECK(rclc_executor_add_client(&m_executor, &m_setInt32PropertyClient, &m_setInt32Resp,
                                   setInt32PropertyClient_callback));
  RCCHECK(rclc_executor_add_client(&m_executor, &m_setFloatPropertyClient, &m_setFloatResp,
                                   setFloatPropertyClient_callback));
  RCCHECK(rclc_executor_add_client(&m_executor, &m_setUint8PropertyClient, &m_setUint8Resp,
                                   setUint8PropertyClient_callback));
  RCCHECK(rclc_executor_add_client(&m_executor, &m_setStringPropertyClient, &m_setStringResp,
                                   setStringPropertyClient_callback));
}

void ROS2Bridge::destroyEntities()
{
  ALOGI("ROS2Bridge - destroying node entities");

  RCSOFTCHECK(rclc_executor_fini(&m_executor));
  RCSOFTCHECK(rcl_client_fini(&m_setInt64PropertyClient, &m_node));
  RCSOFTCHECK(rcl_client_fini(&m_setInt32PropertyClient, &m_node));
  RCSOFTCHECK(rcl_client_fini(&m_setFloatPropertyClient, &m_node));
  RCSOFTCHECK(rcl_client_fini(&m_setUint8PropertyClient, &m_node));
  RCSOFTCHECK(rcl_client_fini(&m_setStringPropertyClient, &m_node));
  RCSOFTCHECK(rcl_node_fini(&m_node));
  RCSOFTCHECK(rclc_support_fini(&m_support));
}

bool ROS2Bridge::pingAgent()
{
  if (m_rmw_options != nullptr) {
    ALOGD("ROS2Bridge - ping discovered agent");
    return (rmw_uros_ping_agent_options(200, 5, m_rmw_options) == RMW_RET_OK);
  }
  else {
    ALOGD("ROS2Bridge - ping default agent");
    return (rmw_uros_ping_agent(200, 5) == RMW_RET_OK);
  }
}

void ROS2Bridge::start()
{
  m_thread = std::thread([this]() {
    switch (m_AgentState) {
      case AgentConnectionState::DISCONNECTED:
        m_rmw_options = rcl_init_options_get_rmw_init_options(&m_init_options);
        ALOGD("ROS2Bridge - discovery agent");
        if (rmw_uros_discover_agent(m_rmw_options) == RMW_RET_OK) {
          ALOGD("ROS2Bridge - agent discovered");
        }
        else {
          ALOGD("ROS2Bridge - agent discovery failed");
          m_rmw_options = nullptr;
        }

        if (pingAgent()) {
          ALOGD("ROS2Bridge - agent found");
          createEntities();
          m_AgentState = AgentConnectionState::CONNECTED;
        }
        break;

      case AgentConnectionState::CONNECTED:
        if (pingAgent()) {
          rclc_executor_spin_some(&m_executor, RCL_MS_TO_NS(100));
        }
        else {
          destroyEntities();
          m_AgentState = AgentConnectionState::DISCONNECTED;
        }
        break;
    }
  });
}

}  // namespace vendor::spyrosoft::vehicle::ros2
