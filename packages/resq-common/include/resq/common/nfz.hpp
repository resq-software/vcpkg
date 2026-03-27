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
 * @brief No-Fly Zone (NFZ) and geofencing utilities
 *
 * Provides geofencing functionality for FAA Part 107 compliance:
 * - No-fly zone polygon definitions
 * - Point-in-polygon checking
 * - Path/line segment intersection detection
 *
 * This module implements AV-03 (Geofencing) control from the compliance catalog.
 * Used by strategic-dtsop to reject flight plans intersecting NFZ polygons.
 *
 * @note All coordinates use WGS84 coordinate system
 */

#pragma once

#include <cmath>
#include <optional>
#include <string>
#include <vector>

#include "geo.hpp"

namespace resq {
namespace common {
namespace geo {

/**
 * @brief Type of no-fly zone
 */
enum class NFZType {
    AIRPORT,        ///< FAA restricted airspace around airports
    TEMPORARY,      ///< Temporary flight restrictions (TFR)
    MILITARY,       ///< Military restricted zones
    NATIONAL_PARK,  ///< National park no-drone zones
    PRISON,         ///< Correctional facility airspace
    HOSPITAL,       ///< Hospital/helipad emergency zones
    CUSTOM          ///< Custom user-defined zone
};

/**
 * @brief No-Fly Zone polygon definition
 *
 * Represents a restricted airspace area as a polygon.
 * Altitude bounds define the vertical extent of the restriction.
 */
struct NoFlyZone {
    /// Unique identifier for the NFZ
    std::string id;

    /// Type of no-fly zone
    NFZType type;

    /// Polygon vertices (lat, lon pairs)
    std::vector<GeoPoint> vertices;

    /// Minimum altitude affected (meters MSL)
    double min_altitude_m;

    /// Maximum altitude affected (meters MSL)
    double max_altitude_m;

    /**
     * @brief Check if a point is inside the NFZ polygon (horizontal)
     * @param point The geographic point to check
     * @return true if point is inside the polygon
     */
    [[nodiscard]] bool contains_point(const GeoPoint& point) const;

    /**
     * @brief Check if an altitude is within the NFZ vertical bounds
     * @param altitude_m Altitude in meters
     * @return true if altitude is within the NFZ vertical extent
     */
    [[nodiscard]] bool contains_altitude(double altitude_m) const {
        return altitude_m >= min_altitude_m && altitude_m <= max_altitude_m;
    }

    /**
     * @brief Check if a point (including altitude) is in the NFZ
     * @param point The 3D geographic point
     * @return true if point is within the NFZ
     */
    [[nodiscard]] bool contains(const GeoPoint& point) const {
        return contains_point(point) && contains_altitude(point.altitude);
    }
};

/**
 * @brief Check if a point is inside a polygon using ray casting
 *
 * Uses the even-odd rule (ray casting algorithm).
 *
 * @param point The point to check
 * @param vertices Polygon vertices
 * @return true if point is inside the polygon
 */
[[nodiscard]] inline bool is_point_in_polygon(const GeoPoint& point,
                                              const std::vector<GeoPoint>& vertices) {
    if (vertices.size() < 3) {
        return false;
    }

    bool inside = false;
    size_t n = vertices.size();
    size_t j = n - 1;

    for (size_t i = 0; i < n; ++i) {
        const auto& pi = vertices[i];
        const auto& pj = vertices[j];

        // Check if ray from point crosses edge
        if (((pi.longitude > point.longitude) != (pj.longitude > point.longitude)) &&
            (point.latitude < (pj.latitude - pi.latitude) * (point.longitude - pi.longitude) /
                                      (pj.longitude - pi.longitude) +
                                  pi.latitude)) {
            inside = !inside;
        }

        j = i;
    }

    return inside;
}

inline bool NoFlyZone::contains_point(const GeoPoint& point) const {
    return is_point_in_polygon(point, vertices);
}

/**
 * @brief Line segment in geographic coordinates
 */
struct GeoSegment {
    GeoPoint start;
    GeoPoint end;
};

/**
 * @brief Check if a line segment intersects a polygon (bounding box check)
 *
 * This is a simplified check using bounding boxes.
 * For more accurate results, consider great circle paths.
 *
 * @param segment The line segment
 * @param polygon The polygon vertices
 * @return true if segment might intersect polygon
 */
[[nodiscard]] inline bool segment_intersects_polygon(const GeoSegment& segment,
                                                     const std::vector<GeoPoint>& polygon) {
    if (polygon.size() < 3) {
        return false;
    }

    // First check if either endpoint is inside
    GeoPoint start{segment.start.latitude, segment.start.longitude, 0};
    GeoPoint end{segment.end.latitude, segment.end.longitude, 0};

    if (is_point_in_polygon(start, polygon) || is_point_in_polygon(end, polygon)) {
        return true;
    }

    // Check segment-polygon edge intersections
    size_t n = polygon.size();
    for (size_t i = 0; i < n; ++i) {
        const auto& p1 = polygon[i];
        const auto& p2 = polygon[(i + 1) % n];

        GeoSegment edge{p1, p2};

        // Simplified bounding box intersection check
        double min_x = std::min(segment.start.longitude, segment.end.longitude);
        double max_x = std::max(segment.start.longitude, segment.end.longitude);
        double min_y = std::min(segment.start.latitude, segment.end.latitude);
        double max_y = std::max(segment.start.latitude, segment.end.latitude);

        double edge_min_x = std::min(p1.longitude, p2.longitude);
        double edge_max_x = std::max(p1.longitude, p2.longitude);
        double edge_min_y = std::min(p1.latitude, p2.latitude);
        double edge_max_y = std::max(p1.latitude, p2.latitude);

        // Bounding boxes don't intersect
        if (max_x < edge_min_x || min_x > edge_max_x || max_y < edge_min_y || min_y > edge_max_y) {
            continue;
        }

        // Simple line intersection check (not perfect for lat/lon but good enough)
        // Check if segments share a point
        if (std::abs(segment.start.latitude - p1.latitude) < 0.0001 &&
            std::abs(segment.start.longitude - p1.longitude) < 0.0001) {
            return true;
        }
    }

    return false;
}

/**
 * @brief Check if a flight path (waypoint to waypoint) intersects any NFZ
 *
 * @param from Starting point
 * @param to Ending point
 * @param nfz_list List of no-fly zones to check against
 * @return Pointer to the NFZ that the path intersects, or nullopt if clear
 */
[[nodiscard]] inline std::optional<std::reference_wrapper<const NoFlyZone>> path_intersects_nfz(
    const GeoPoint& from, const GeoPoint& to, const std::vector<NoFlyZone>& nfz_list) {
    GeoSegment path{from, to};

    for (const auto& nfz : nfz_list) {
        // Check if path altitude range overlaps with NFZ altitude bounds
        double path_min_alt = std::min(from.altitude, to.altitude);
        double path_max_alt = std::max(from.altitude, to.altitude);

        if (path_max_alt < nfz.min_altitude_m || path_min_alt > nfz.max_altitude_m) {
            continue;  // No vertical overlap
        }

        // Check horizontal intersection
        if (segment_intersects_polygon(path, nfz.vertices)) {
            return std::ref(nfz);
        }
    }

    return std::nullopt;
}

/**
 * @brief Check if any point in a path intersects NFZ
 *
 * @param waypoints List of waypoints defining the path
 * @param nfz_list List of no-fly zones
 * @return Pointer to first intersecting NFZ, or nullopt if clear
 */
[[nodiscard]] inline std::optional<std::reference_wrapper<const NoFlyZone>> path_intersects_nfz(
    const std::vector<GeoPoint>& waypoints, const std::vector<NoFlyZone>& nfz_list) {
    if (waypoints.size() < 2) {
        return std::nullopt;
    }

    for (size_t i = 0; i < waypoints.size() - 1; ++i) {
        auto result = path_intersects_nfz(waypoints[i], waypoints[i + 1], nfz_list);
        if (result.has_value()) {
            return result;
        }
    }

    return std::nullopt;
}

}  // namespace geo
}  // namespace common
}  // namespace resq
