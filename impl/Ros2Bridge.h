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

#pragma once

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <ros2_android_vhal/srv/vhal_set_int64_property.h>
#include <ros2_android_vhal/srv/vhal_set_int32_property.h>
#include <ros2_android_vhal/srv/vhal_set_float_property.h>
#include <ros2_android_vhal/srv/vhal_set_uint8_property.h>
#include <ros2_android_vhal/srv/vhal_set_string_property.h>

#include <atomic>
#include <mutex>
#include <thread>
#include <variant>

namespace vendor::spyrosoft::vehicle::ros2 {

enum class AgentConnectionState { CONNECTED, DISCONNECTED };

/**
 * @brief
 *
 */
class ROS2Bridge {
 public:
  ROS2Bridge();
  virtual ~ROS2Bridge();

  void start();
  void stop() { m_running = false; }
  bool is_connected() const { return m_running; }

  template <typename ValueT>
  ValueT setProperty(int64_t timestamp, int32_t areaId, int32_t prop, ValueT value)
  {
    ros2_android_vhal__srv__VhalSetInt64Property_Request req;
    ros2_android_vhal__srv__VhalSetInt64Property_Request__init(&req);
    req.timestamp = timestamp;
    req.area_id = areaId;
    req.prop = prop;
    req.new_value = value;

    int64_t seq;
    (void)rcl_send_request(&m_setInt64PropertyClient, &req, &seq);

    return value;
  }

 protected:
  void createEntities();
  void destroyEntities();
  bool pingAgent();

 private:
  std::thread m_thread;
  std::atomic_bool m_running{true};
  std::atomic<AgentConnectionState> m_AgentState = AgentConnectionState::DISCONNECTED;

  rcl_init_options_t m_init_options;
  rmw_init_options_t* m_rmw_options = nullptr;
  rclc_support_t m_support;
  rcl_allocator_t m_allocator;
  rcl_node_t m_node;
  rclc_executor_t m_executor;

  rcl_client_t m_setInt64PropertyClient;
  rcl_client_t m_setInt32PropertyClient;
  rcl_client_t m_setUint8PropertyClient;
  rcl_client_t m_setFloatPropertyClient;
  rcl_client_t m_setStringPropertyClient;

  ros2_android_vhal__srv__VhalSetInt64Property_Response m_setInt64Resp;
  ros2_android_vhal__srv__VhalSetInt32Property_Response m_setInt32Resp;
  ros2_android_vhal__srv__VhalSetFloatProperty_Response m_setFloatResp;
  ros2_android_vhal__srv__VhalSetUint8Property_Response m_setUint8Resp;
  ros2_android_vhal__srv__VhalSetStringProperty_Response m_setStringResp;
};

}  // namespace vendor::spyrosoft::vehicle::ros2
