/**
 * Copyright 2026 ResQ Software
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <chrono>

#include <google/protobuf/timestamp.pb.h>

namespace resq {
namespace common {
namespace time {

/**
 * @brief Convert std::chrono::system_clock::time_point to google::protobuf::Timestamp
 */
inline void to_proto_timestamp(const std::chrono::system_clock::time_point& from,
                               google::protobuf::Timestamp* to) {
    auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(from);
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(from - seconds);
    to->set_seconds(seconds.time_since_epoch().count());
    to->set_nanos(nanoseconds.count());
}

/**
 * @brief Convert google::protobuf::Timestamp to std::chrono::system_clock::time_point
 */
[[nodiscard]] inline std::chrono::system_clock::time_point from_proto_timestamp(
    const google::protobuf::Timestamp& from) {
    auto duration = std::chrono::seconds(from.seconds()) + std::chrono::nanoseconds(from.nanos());
    return std::chrono::system_clock::time_point(duration);
}

}  // namespace time
}  // namespace common
}  // namespace resq
