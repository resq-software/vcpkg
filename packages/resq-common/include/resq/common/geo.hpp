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

/**
 * @brief Geographic utilities for drone navigation
 *
 * Provides geospatial calculations for drone swarm coordination,
 * including distance calculations and coordinate validation.
 *
 * @note All coordinates use WGS84 coordinate system
 * @note Distances are calculated in meters
 */

#pragma once

#include <cmath>

namespace resq {
namespace common {
namespace geo {

inline constexpr double kPi = 3.14159265358979323846;

/**
 * @brief Geographic point with latitude, longitude, and altitude
 *
 * Represents a 3D position on Earth's surface using the WGS84 ellipsoid.
 * Latitude and longitude are in degrees, altitude in meters above sea level.
 *
 * @invariant latitude must be in range [-90, 90]
 * @invariant longitude must be in range [-180, 180]
 */
struct GeoPoint {
    /// Latitude in degrees (-90 to 90)
    double latitude;
    /// Longitude in degrees (-180 to 180)
    double longitude;
    /// Altitude in meters above sea level
    double altitude;
};

/**
 * @brief Calculate Haversine distance between two geographic points
 *
 * Computes the great-circle distance between two points on Earth's surface
 * using the Haversine formula. This provides accurate results for most
 * navigation purposes, though for survey-grade precision consider
 * Vincenty's formulae.
 *
 * @param p1 First geographic point
 * @param p2 Second geographic point
 * @return Distance in meters between the two points
 *
 * @post Returns 0.0 if both points are identical
 * @post Returns approximately 20,000,000 meters for antipodal points
 *
 * @note Uses Earth radius of 6,371,000 meters (mean radius)
 * @note Accuracy degrades for very large distances due to Earth's ellipsoidal shape
 *
 * @par Example:
 * @code
 * GeoPoint base{40.7128, -74.0060, 0.0};  // New York
 * GeoPoint target{34.0522, -118.2437, 0.0}; // Los Angeles
 * double distance = haversine_distance_meters(base, target);
 * // distance ≈ 3,944,000 meters
 * @endcode
 */
[[nodiscard]] inline double haversine_distance_meters(const GeoPoint& p1, const GeoPoint& p2) {
    const double R = 6371000.0;  // Earth radius in meters
    double lat1_rad = p1.latitude * kPi / 180.0;
    double lat2_rad = p2.latitude * kPi / 180.0;
    double delta_lat = (p2.latitude - p1.latitude) * kPi / 180.0;
    double delta_lon = (p2.longitude - p1.longitude) * kPi / 180.0;

    double a =
        std::sin(delta_lat / 2) * std::sin(delta_lat / 2) +
        std::cos(lat1_rad) * std::cos(lat2_rad) * std::sin(delta_lon / 2) * std::sin(delta_lon / 2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

    return R * c;
}

}  // namespace geo
}  // namespace common
}  // namespace resq
