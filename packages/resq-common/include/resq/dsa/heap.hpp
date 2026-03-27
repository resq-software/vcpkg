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
/**
 * @brief Fixed-size priority queue with bounded capacity
 *
 * A bounded min-heap that maintains at most N elements, automatically
 * evicting the element with the highest distance value when full.
 * Useful for K-nearest-neighbors, top-K selection, and beam search.
 *
 * @note Uses max-heap internally (highest distance at root), but exposes
 *       the element with minimum distance value as the "best"
 * @warning The comparison function must be consistent - if f(a) < f(b)
 *          and f(b) < f(c), then f(a) < f(c)
 *
 * @par Example:
 * @code
 * // Keep top 5 closest drones to target
 * BoundedHeap<Drone> closest(5, [](const Drone& d) {
 *     return distance(d.position, target);
 * });
 *
 * for (const auto& drone : all_drones) {
 *     closest.insert(drone);
 * }
 *
 * // Get sorted results
 * auto results = closest.to_sorted();
 * @endcode
 */

#include <algorithm>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <vector>

namespace resq::dsa {

/**
 * @brief Bounded heap for top-K element selection
 *
 * A priority queue that maintains a fixed maximum size. When capacity
 * is reached, new elements only replace the worst element if they are
 * better (have lower distance according to the provided function).
 *
 * @tparam T Type of elements stored in the heap
 *
 * @invariant limit_ >= 1
 * @invariant dist_ must not throw or mutate state
 *
 * @note Provides O(log n) insert and O(1) peek
 * @note Memory: O(limit_) elements maximum
 */
template <typename T>
class BoundedHeap {
public:
    /**
     * @brief Distance function type
     *
     * Takes a reference to an element and returns a distance value.
     * Lower values are considered "better" (closer to target).
     */
    using DistFn = std::function<double(const T&)>;

    /**
     * @brief Construct a bounded heap
     *
     * @param limit Maximum number of elements to retain
     * @param dist Distance function - lower values are preferred
     *
     * @throws std::invalid_argument If limit < 1
     *
     * @post Heap is empty and ready for inserts
     */
    BoundedHeap(std::size_t limit, DistFn dist) : limit_(limit), dist_(std::move(dist)) {
        if (limit < 1) throw std::invalid_argument("limit must be >= 1");
    }

    /**
     * @brief Insert an element into the heap
     *
     * If heap is not full, adds element. If heap is full, only adds
     * element if it is better than the current worst (root).
     *
     * @param entry Element to insert
     *
     * @post If heap was not full, element is in heap
     * @post If heap was full and entry is better than root, root is replaced
     * @post Heap property is maintained after insert
     */
    void insert(T entry) {
        if (data_.size() < limit_) {
            data_.push_back(std::move(entry));
            sift_up(data_.size() - 1);
        } else if (!data_.empty() && dist_(entry) < dist_(data_[0])) {
            data_[0] = std::move(entry);
            sift_down(0);
        }
    }

    /**
     * @brief Peek at the worst (highest-distance) element in the heap
     *
     * @return Pointer to root element (max distance), or nullptr if empty
     *
     * @note The returned pointer is invalid after insert() or destruction
     */
    [[nodiscard]] const T* peek() const { return data_.empty() ? nullptr : &data_[0]; }

    /**
     * @brief Extract all elements sorted by distance
     *
     * @return Vector of elements sorted from best to worst
     *
     * @note Returns copy of elements; original heap is unchanged
     */
    [[nodiscard]] std::vector<T> to_sorted() const {
        auto copy = data_;
        std::sort(copy.begin(), copy.end(),
                  [&](const T& a, const T& b) { return dist_(a) < dist_(b); });
        return copy;
    }

    /**
     * @brief Get current number of elements in heap
     *
     * @return Number of elements (0 to limit_)
     */
    [[nodiscard]] std::size_t size() const { return data_.size(); }

    /**
     * @brief Check if heap is empty
     */
    [[nodiscard]] bool empty() const { return data_.empty(); }

private:
    /// Internal heap storage
    std::vector<T> data_;
    /// Maximum capacity
    std::size_t limit_;
    /// Distance evaluation function
    DistFn dist_;

    /**
     * @brief Restore heap property by moving element up
     */
    void sift_up(std::size_t i) {
        while (i > 0) {
            auto p = (i - 1) / 2;
            if (dist_(data_[p]) >= dist_(data_[i])) break;
            std::swap(data_[p], data_[i]);
            i = p;
        }
    }

    /**
     * @brief Restore heap property by moving element down
     */
    void sift_down(std::size_t i) {
        auto n = data_.size();
        while (true) {
            auto x = i, l = 2 * i + 1, r = 2 * i + 2;
            if (l < n && dist_(data_[l]) > dist_(data_[x])) x = l;
            if (r < n && dist_(data_[r]) > dist_(data_[x])) x = r;
            if (x == i) break;
            std::swap(data_[i], data_[x]);
            i = x;
        }
    }
};

}  // namespace resq::dsa
