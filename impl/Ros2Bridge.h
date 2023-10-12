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
#include <ros2_android_vhal/msg/vehicle_property.h>
#include <ros2_android_vhal/srv/set_vehicle_property.h>

#include <atomic>
#include <mutex>
#include <string>
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
  using PropertyValue_t = std::variant<int64_t, int32_t, uint8_t, float, std::string>;

 public:
  ROS2Bridge();
  virtual ~ROS2Bridge();

  void start(std::chrono::seconds timeout = std::chrono::seconds(0));
  void stop() { m_running = false; }
  bool is_connected() const { return (m_AgentState == AgentConnectionState::CONNECTED); }

  bool setProperty(int64_t timestamp, int32_t areaId, int32_t propId, PropertyValue_t value);

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

  rcl_client_t m_vehiclePropertyClient;
  std::mutex m_clientMutex;
};

}  // namespace vendor::spyrosoft::vehicle::ros2
