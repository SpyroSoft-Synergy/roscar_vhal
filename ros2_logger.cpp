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

#include <android/log.h>
#include <log/log.h>
#include <rcl/rcl.h>
#include <rcutils/logging.h>

namespace {

const char *rcutils_log_tag = "";

void rcutils_logcat_handler(const rcutils_log_location_t *,  // location
                            int severity,
                            const char *,                // name
                            rcutils_time_point_value_t,  // timestamp
                            const char *format, va_list *args) {
    switch (severity) {
        case RCUTILS_LOG_SEVERITY_DEBUG:
            (void)__android_log_print(ANDROID_LOG_DEBUG, rcutils_log_tag, format, args);
            break;
        case RCUTILS_LOG_SEVERITY_INFO:
            (void)__android_log_print(ANDROID_LOG_INFO, rcutils_log_tag, format, args);
            break;
        case RCUTILS_LOG_SEVERITY_WARN:
            (void)__android_log_print(ANDROID_LOG_WARN, rcutils_log_tag, format, args);
            break;
        case RCUTILS_LOG_SEVERITY_ERROR:
            (void)__android_log_print(ANDROID_LOG_ERROR, rcutils_log_tag, format, args);
            break;
        case RCUTILS_LOG_SEVERITY_FATAL:
            (void)__android_log_print(ANDROID_LOG_FATAL, rcutils_log_tag, format, args);
            break;
        default:
            break;
    }
}

}  // namespace

namespace vendor::spyrosoft::vehicle {

Logger::Logger(const char *log_tag) {
    if (log_tag != nullptr) {
        rcutils_log_tag = log_tag;
    }
    rcutils_logging_set_output_handler(rcutils_logcat_handler);
    rcutils_logging_set_default_logger_level(RCUTILS_LOG_SEVERITY_DEBUG);
}

}  // namespace vendor::spyrosoft::vehicle