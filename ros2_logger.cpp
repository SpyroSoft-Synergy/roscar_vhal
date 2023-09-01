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

#include "ros2_logger.hpp"

#include <rcl/rcl.h>
#include <rcutils/logging.h>
#include <rmw_microxrcedds_c/config.h>
#include <rmw_microros/error_handling.h>

#include "common/logging.hpp"

namespace {

void rcutils_logcat_handler(const rcutils_log_location_t *,  // location
                            int severity,
                            const char *,                // name
                            rcutils_time_point_value_t,  // timestamp
                            const char *format, va_list *args)
{
  switch (severity) {
    case RCUTILS_LOG_SEVERITY_DEBUG:
      (void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, format, args);
      break;
    case RCUTILS_LOG_SEVERITY_INFO:
      (void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, format, args);
      break;
    case RCUTILS_LOG_SEVERITY_WARN:
      (void)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, format, args);
      break;
    case RCUTILS_LOG_SEVERITY_ERROR:
      (void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, format, args);
      break;
    case RCUTILS_LOG_SEVERITY_FATAL:
      (void)__android_log_print(ANDROID_LOG_FATAL, LOG_TAG, format, args);
      break;
    default:
      break;
  }
}

const char *rmw_uros_error_type_to_str(const rmw_uros_error_entity_type_t type)
{
  switch (type) {
    case RMW_UROS_ERROR_ON_UNKNOWN:
      return "RMW_UROS_ERROR_ON_UNKNOWN";
    case RMW_UROS_ERROR_ON_NODE:
      return "RMW_UROS_ERROR_ON_NODE";
    case RMW_UROS_ERROR_ON_SERVICE:
      return "RMW_UROS_ERROR_ON_SERVICE";
    case RMW_UROS_ERROR_ON_CLIENT:
      return "RMW_UROS_ERROR_ON_CLIENT";
    case RMW_UROS_ERROR_ON_SUBSCRIPTION:
      return "RMW_UROS_ERROR_ON_SUBSCRIPTION";
    case RMW_UROS_ERROR_ON_PUBLISHER:
      return "RMW_UROS_ERROR_ON_PUBLISHER";
    case RMW_UROS_ERROR_ON_GRAPH:
      return "RMW_UROS_ERROR_ON_GRAPH";
    case RMW_UROS_ERROR_ON_GUARD_CONDITION:
      return "RMW_UROS_ERROR_ON_GUARD_CONDITION";
    case RMW_UROS_ERROR_ON_TOPIC:
      return "RMW_UROS_ERROR_ON_TOPIC";
    default:
      return "RMW_UROS_ERROR_UNKNOWN";
  }
}

const char *rmw_uros_error_source_to_str(const rmw_uros_error_source_t source)
{
  switch (source) {
    case RMW_UROS_ERROR_ENTITY_CREATION:
      return "RMW_UROS_ERROR_ENTITY_CREATION";
    case RMW_UROS_ERROR_ENTITY_DESTRUCTION:
      return "RMW_UROS_ERROR_ENTITY_DESTRUCTION";
    case RMW_UROS_ERROR_CHECK:
      return "RMW_UROS_ERROR_CHECK";
    case RMW_UROS_ERROR_NOT_IMPLEMENTED:
      return "RMW_UROS_ERROR_NOT_IMPLEMENTED";
    case RMW_UROS_ERROR_MIDDLEWARE_ALLOCATION:
      return "RMW_UROS_ERROR_MIDDLEWARE_ALLOCATION";
    default:
      return "RMW_UROS_ERROR_UNKNOWN";
  }
}

void rmw_uros_logcat_handler(const rmw_uros_error_entity_type_t entity, const rmw_uros_error_source_t source,
                             const rmw_uros_error_context_t context, const char *file, const int line)
{
  ALOGE("rmw_uros error in file %s on line %d, entity: %s, src: %s, description: %s", file, line,
        rmw_uros_error_type_to_str(entity), rmw_uros_error_source_to_str(source), context.description);
}

}  // namespace

namespace vendor::spyrosoft::vehicle {

Logger::Logger()
{
  ALOGI("Initializing ROS 2 logger");
  rcutils_logging_set_output_handler(rcutils_logcat_handler);
  rcutils_logging_set_default_logger_level(RCUTILS_LOG_SEVERITY_DEBUG);

  ALOGI("Initializing rmw_uros error handling");
  rmw_uros_set_error_handling_callback(rmw_uros_logcat_handler);
}

}  // namespace vendor::spyrosoft::vehicle