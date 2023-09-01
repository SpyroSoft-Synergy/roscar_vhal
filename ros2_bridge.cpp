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

#include "ros2_bridge.hpp"

#include <rcl/error_handling.h>
#include <rmw_microros/error_handling.h>
#include <rmw_microros/discovery.h>
#include <rcl/rcl.h>
#include <rclc/executor.h>
#include <rmw_microros/ping.h>
#include <std_msgs/msg/header.h>
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
void ping_timer_callback(rcl_timer_t* timer, int64_t last_call_time)
{
  (void)timer;
  (void)last_call_time;
}
}  // namespace

namespace vendor::spyrosoft::vehicle {

ROS2Bridge::~ROS2Bridge()
{
  RCSOFTCHECK(rcl_publisher_fini(&heartbeat_publisher, &node));
  RCSOFTCHECK(rcl_node_fini(&node));
}

void ROS2Bridge::run()
{
  mThread = std::thread([this]() {
    rclc_support_t support;
    rcl_allocator_t allocator = rcutils_get_default_allocator();

    ALOGI("ROS2Bridge - rcl_init_options_init");
    auto rcl_init_options = rcl_get_zero_initialized_init_options();
    RCCHECK(rcl_init_options_init(&rcl_init_options, allocator));
    auto* rmw_options = rcl_init_options_get_rmw_init_options(&rcl_init_options);

    auto discover_result = RMW_RET_OK;
    int discovery_retries = 20;
    do {
      ALOGI("ROS2Bridge - rmw_uros_discover_agent");
      discover_result = rmw_uros_discover_agent(rmw_options);
      discovery_retries--;
      usleep(1000000);
    } while (discover_result == RMW_RET_TIMEOUT && discovery_retries > 0);

    if (RMW_RET_OK != discover_result) {
      ALOGE("ROS2Bridge - rmw_uros_discover_agent failed: %d", (int)discover_result);
      auto ping_status = RMW_RET_OK;
      do {
        ping_status = rmw_uros_ping_agent(200, 10);
        ALOGI("ROS2Bridge - ping_agent_status: %d", (int)ping_status);
        usleep(1000000);
      } while (ping_status != RMW_RET_OK);

      ALOGI("ROS2Bridge - rclc_support_init");
      RCCHECK(rclc_support_init(&support, 0, nullptr, &allocator));
    }
    else {
      auto ping_status = RMW_RET_OK;
      do {
        ping_status = rmw_uros_ping_agent_options(200, 10, rmw_options);
        ALOGI("ROS2Bridge - ping_agent_options_status: %d", (int)ping_status);
        usleep(1000000);
      } while (ping_status != RMW_RET_OK);

      ALOGI("ROS2Bridge - rclc_support_init_with_options");
      RCCHECK(rclc_support_init_with_options(&support, 0, nullptr, &rcl_init_options, &allocator));
    }

    // create node
    rcl_node_t node;
    ALOGI("ROS2Bridge - initializing node");
    RCCHECK(rclc_node_init_default(&node, "vhal_node", "", &support));

    // Create a reliable publisher
    rcl_publisher_t heartbeat_publisher;
    RCCHECK(rclc_publisher_init_default(&heartbeat_publisher, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Header),
                                        "/vhal/heartbeat"));

    // Create a 2 seconds heartbeat timer,
    rcl_timer_t timer;
    RCCHECK(rclc_timer_init_default(&timer, &support, RCL_MS_TO_NS(2000), ping_timer_callback));

    // Create executor
    rclc_executor_t executor;
    RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
    RCCHECK(rclc_executor_add_timer(&executor, &timer));

    while (mRunning) {
      rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
      usleep(10000);
    }
  });
}

}  // namespace vendor::spyrosoft::vehicle
