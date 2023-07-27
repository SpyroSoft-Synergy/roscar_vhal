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
#include <rcl/rcl.h>
#include <rclc/executor.h>
#include <rclc/rclc.h>
#include <std_msgs/msg/header.h>
#include <unistd.h>

#define RCCHECK(fn)                                                                      \
    {                                                                                    \
        rcl_ret_t temp_rc = fn;                                                          \
        if ((temp_rc != RCL_RET_OK)) {                                                   \
            printf("Failed status on line %d: %d. Aborting.\n", __LINE__, (int)temp_rc); \
            abort();                                                                     \
        }                                                                                \
    }
#define RCSOFTCHECK(fn)                                                                    \
    {                                                                                      \
        rcl_ret_t temp_rc = fn;                                                            \
        if ((temp_rc != RCL_RET_OK)) {                                                     \
            printf("Failed status on line %d: %d. Continuing.\n", __LINE__, (int)temp_rc); \
        }                                                                                  \
    }

namespace {
void ping_timer_callback(rcl_timer_t* timer, int64_t last_call_time) {
    (void)timer;
    (void)last_call_time;
}
}  // namespace

namespace vendor::spyrosoft::vehicle {

ROS2Bridge::~ROS2Bridge() {
    // RCCHECK(rcl_publisher_fini(&heartbeat_publisher, &node));
    // RCCHECK(rcl_node_fini(&node));
}

void ROS2Bridge::run() {
    mThread = std::thread([this]() {
        rcl_allocator_t allocator = rcl_get_default_allocator();
        rclc_support_t support;

        // create init_options
        RCCHECK(rclc_support_init(&support, 0, nullptr, &allocator));

        // create node
        rcl_node_t node;
        RCCHECK(rclc_node_init_default(&node, "vhal_node", "", &support));

        // Create a reliable publisher
        rcl_publisher_t heartbeat_publisher;
        RCCHECK(rclc_publisher_init_default(&heartbeat_publisher, &node,
                                            ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Header), "/vhal/heartbeat"));

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

std::vector<android::hardware::automotive::vehicle::V2_0::VehiclePropConfig> ROS2Bridge::getAllPropertyConfig() const {
    return {};
}

android::hardware::automotive::vehicle::V2_0::StatusCode ROS2Bridge::setProperty(
    const android::hardware::automotive::vehicle::V2_0::VehiclePropValue& value, bool updateStatus) {
    (void)value;
    (void)updateStatus;
    return android::hardware::automotive::vehicle::V2_0::StatusCode::OK;
}

void ROS2Bridge::onPropertyValue(const android::hardware::automotive::vehicle::V2_0::VehiclePropValue& value, bool updateStatus) {
    (void)value;
    (void)updateStatus;
}

}  // namespace vendor::spyrosoft::vehicle