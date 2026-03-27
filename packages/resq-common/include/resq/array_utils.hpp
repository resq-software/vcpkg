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

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace resq {

/**
 * @brief Fast operations on sorted uint32_t arrays
 *
 * All input arrays MUST be sorted in ascending order.
 * Output arrays are dynamically allocated - caller must free with delete[].
 *
 * Example:
 * @code
 * uint32_t available[] = {1, 3, 5, 7, 9};
 * uint32_t assigned[] = {2, 3, 4, 5};
 *
 * uint32_t* free_drones = nullptr;
 * size_t count = ArrayUtils::exclude_scalar(available, 5, assigned, 4, &free_drones);
 * // free_drones = {1, 7, 9}, count = 3
 *
 * delete[] free_drones;
 * @endcode
 */
class ArrayUtils {
public:
    /**
     * @brief Fast intersection (AND) of two sorted arrays
     *
     * @param A First sorted array
     * @param lenA Length of array A
     * @param B Second sorted array
     * @param lenB Length of array B
     * @param out Output array (allocated by function, caller must delete[])
     * @return Size of intersection (elements in out)
     *
     * Time complexity: O(lenA + lenB)
     * Space complexity: O(min(lenA, lenB))
     */
    [[nodiscard]] static size_t and_scalar(const uint32_t* A, size_t lenA, const uint32_t* B,
                                           size_t lenB, uint32_t** out) {
        // Allocate worst-case size
        size_t max_size = std::min(lenA, lenB);
        *out = new uint32_t[max_size];

        size_t i = 0, j = 0, k = 0;

        // Fast scalar merge
        while (i < lenA && j < lenB) {
            uint32_t a = A[i];
            uint32_t b = B[j];

            if (a == b) {
                (*out)[k++] = a;
                ++i;
                ++j;
            } else if (a < b) {
                ++i;
            } else {
                ++j;
            }
        }

        return k;
    }

    /**
     * @brief Fast union (OR) of two sorted arrays
     *
     * @return Size of union (elements in out)
     *
     * Time complexity: O(lenA + lenB)
     * Space complexity: O(lenA + lenB)
     */
    [[nodiscard]] static size_t or_scalar(const uint32_t* A, size_t lenA, const uint32_t* B,
                                          size_t lenB, uint32_t** out) {
        // Allocate worst-case size
        size_t max_size = lenA + lenB;
        *out = new uint32_t[max_size];

        size_t i = 0, j = 0, k = 0;

        // Merge with deduplication
        while (i < lenA && j < lenB) {
            uint32_t a = A[i];
            uint32_t b = B[j];

            if (a == b) {
                (*out)[k++] = a;
                ++i;
                ++j;
            } else if (a < b) {
                (*out)[k++] = a;
                ++i;
            } else {
                (*out)[k++] = b;
                ++j;
            }
        }

        // Add remaining elements
        while (i < lenA) {
            (*out)[k++] = A[i++];
        }

        while (j < lenB) {
            (*out)[k++] = B[j++];
        }

        return k;
    }

    /**
     * @brief Exclude elements (A - B): elements in A but not in B
     *
     * @param src Source array
     * @param lenSrc Length of source
     * @param filter Elements to exclude
     * @param lenFilter Length of filter
     * @param out Output array (allocated by function)
     * @return Size of result
     *
     * Time complexity: O(lenSrc + lenFilter)
     */
    [[nodiscard]] static size_t exclude_scalar(const uint32_t* src, size_t lenSrc,
                                               const uint32_t* filter, size_t lenFilter,
                                               uint32_t** out) {
        // Allocate worst-case size
        *out = new uint32_t[lenSrc];

        size_t i = 0, j = 0, k = 0;

        while (i < lenSrc) {
            uint32_t s = src[i];

            // Skip all filter elements less than current source element
            while (j < lenFilter && filter[j] < s) {
                ++j;
            }

            // If current source element is not in filter, add it
            if (j >= lenFilter || filter[j] != s) {
                (*out)[k++] = s;
            }

            ++i;
        }

        return k;
    }

    /**
     * @brief Binary search with skip-ahead
     *
     * Searches for 'id' in sorted array. If found, sets curr_index to that index.
     * If not found, sets curr_index to the index of the next larger element.
     *
     * @param curr_index Current index (input/output)
     * @param array Sorted array to search
     * @param array_len Length of array
     * @param id Value to search for
     * @return true if id was found, false otherwise
     */
    [[nodiscard]] static bool skip_index_to_id(uint32_t& curr_index, const uint32_t* array,
                                               uint32_t array_len, uint32_t id) {
        if (curr_index >= array_len) {
            return false;
        }

        // Binary search from curr_index onwards
        uint32_t left = curr_index;
        uint32_t right = array_len;

        while (left < right) {
            uint32_t mid = left + (right - left) / 2;

            if (array[mid] == id) {
                curr_index = mid;
                return true;
            } else if (array[mid] < id) {
                left = mid + 1;
            } else {
                right = mid;
            }
        }

        // Not found - curr_index points to next larger element
        curr_index = left;
        return false;
    }

    /**
     * @brief Check if arrays are equal
     */
    [[nodiscard]] static bool arrays_equal(const uint32_t* a, size_t len_a, const uint32_t* b,
                                           size_t len_b) {
        if (len_a != len_b) return false;
        return std::equal(a, a + len_a, b);
    }

    /**
     * @brief Check if array is sorted
     */
    [[nodiscard]] static bool is_sorted(const uint32_t* array, size_t len) {
        for (size_t i = 1; i < len; ++i) {
            if (array[i] < array[i - 1]) {
                return false;
            }
        }
        return true;
    }

    // ========================================================================
    // Convenience wrappers for std::vector
    // ========================================================================

    /**
     * @brief Intersection of two vectors (returns new vector)
     */
    [[nodiscard]] static std::vector<uint32_t> intersect(const std::vector<uint32_t>& a,
                                                         const std::vector<uint32_t>& b) {
        std::vector<uint32_t> result;
        result.reserve(std::min(a.size(), b.size()));
        size_t i = 0, j = 0;
        while (i < a.size() && j < b.size()) {
            if (a[i] == b[j]) {
                result.push_back(a[i]);
                ++i;
                ++j;
            } else if (a[i] < b[j]) {
                ++i;
            } else {
                ++j;
            }
        }
        return result;
    }

    /**
     * @brief Union of two vectors (returns new vector)
     */
    [[nodiscard]] static std::vector<uint32_t> union_of(const std::vector<uint32_t>& a,
                                                        const std::vector<uint32_t>& b) {
        std::vector<uint32_t> result;
        result.reserve(a.size() + b.size());
        size_t i = 0, j = 0;
        while (i < a.size() && j < b.size()) {
            if (a[i] == b[j]) {
                result.push_back(a[i]);
                ++i;
                ++j;
            } else if (a[i] < b[j]) {
                result.push_back(a[i]);
                ++i;
            } else {
                result.push_back(b[j]);
                ++j;
            }
        }
        while (i < a.size()) result.push_back(a[i++]);
        while (j < b.size()) result.push_back(b[j++]);
        return result;
    }

    /**
     * @brief Exclude elements (a - b) (returns new vector)
     */
    [[nodiscard]] static std::vector<uint32_t> exclude(const std::vector<uint32_t>& src,
                                                       const std::vector<uint32_t>& filter) {
        std::vector<uint32_t> result;
        result.reserve(src.size());
        size_t j = 0;
        for (size_t i = 0; i < src.size(); ++i) {
            while (j < filter.size() && filter[j] < src[i]) ++j;
            if (j >= filter.size() || filter[j] != src[i]) {
                result.push_back(src[i]);
            }
        }
        return result;
    }

    /**
     * @brief Check if element exists in sorted array
     */
    [[nodiscard]] static bool contains(const uint32_t* array, size_t len, uint32_t value) {
        return std::binary_search(array, array + len, value);
    }

    [[nodiscard]] static bool contains(const std::vector<uint32_t>& vec, uint32_t value) {
        return std::binary_search(vec.begin(), vec.end(), value);
    }
};

}  // namespace resq
