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
 * @brief Count-Min sketch for frequency estimation
 *
 * Probabilistic data structure for estimating frequencies of events.
 * Provides space-efficient counting with guaranteed error bounds.
 *
 * Use cases:
 * - Network traffic analysis
 * - Heavy hitter detection
 * - Approximate frequency queries
 *
 * @note Guarantees estimate >= true count (upper bound)
 * @note Actual error is at most eps * N with probability delta
 * @warning Not suitable for exact counting
 *
 * @par Example:
 * @code
 * // Create sketch with 1% error bound, 99% confidence
 * CountMinSketch sketch(0.01, 0.01);
 *
 * for (const auto& req : requests) {
 *     sketch.increment(req.ip_address);
 * }
 *
 * // Estimate frequency (may be overestimate)
 * uint64_t freq = sketch.estimate("192.168.1.1");
 * @endcode
 */

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace resq::dsa {

/**
 * @brief Count-Min sketch for frequency estimation
 *
 * Probabilistic counter that over-estimates frequencies. Uses d hash
 * functions and w counters per row to provide (epsilon, delta) guarantees.
 *
 * @invariant eps in (0, 1) - error parameter
 * @invariant delta in (0, 1) - confidence parameter
 *
 * @note Memory: O(1/eps * log(1/delta)) counters
 * @note Update: O(log(1/delta)) operations
 * @note Query: O(log(1/delta)) operations
 *
 * @warning Returned estimate is always >= true count (upper bound)
 */
class CountMinSketch {
    /// Table of counters [hash_function][bucket]
    std::vector<std::vector<uint64_t>> table_;
    /// Number of buckets (w)
    std::size_t w_;
    /// Number of hash functions (d)
    std::size_t d_;

    /**
     * @brief Compute hash using double hashing technique
     */
    std::size_t hash_fn(std::string_view key, uint32_t seed) const {
        uint32_t h = 2166136261u ^ seed;
        for (unsigned char c : key) {
            h ^= c;
            h *= 16777619u;
        }
        return h % w_;
    }

public:
    /**
     * @brief Construct a Count-Min sketch
     *
     * @param eps Error parameter (epsilon). Error <= eps * N with probability delta
     *            Smaller eps = more memory, more accuracy
     * @param delta Confidence parameter. Error guarantee holds with probability (1 - delta)
     *              Smaller delta = more memory, higher confidence
     *
     * @throws std::invalid_argument If eps <= 0 or eps >= 1
     * @throws std::invalid_argument If delta <= 0 or delta >= 1
     *
     * @par Example:
     * @code
     * // 1% error, 99% confidence
     * CountMinSketch cms(0.01, 0.01);
     *
     * // 5% error, 95% confidence (smaller)
     * CountMinSketch cms_small(0.05, 0.05);
     * @endcode
     */
    CountMinSketch(double eps, double delta) {
        if (eps <= 0.0 || eps >= 1.0) throw std::invalid_argument("eps must be in (0, 1)");
        if (delta <= 0.0 || delta >= 1.0) throw std::invalid_argument("delta must be in (0, 1)");
        w_ = static_cast<std::size_t>(std::ceil(std::exp(1.0) / eps));
        d_ = static_cast<std::size_t>(std::ceil(std::log(1.0 / delta)));
        table_.assign(d_, std::vector<uint64_t>(w_, 0));
    }

    /**
     * @brief Increment count for a key
     *
     * @param key String identifier to count
     * @param count Amount to add (default: 1)
     *
     * @post Count for key is incremented by count
     * @note Updates all d hash function buckets
     */
    void increment(std::string_view key, uint64_t count = 1) {
        for (std::size_t i = 0; i < d_; ++i)
            table_[i][hash_fn(key, static_cast<uint32_t>(i) * 0x9e3779b9u)] += count;
    }

    /**
     * @brief Estimate frequency of a key
     *
     * @param key Key to estimate count for
     * @return Estimated count (guaranteed >= true count)
     *
     * @retval >= Actual frequency (upper bound)
     * @retval <= eps * N (theoretical upper bound)
     *
     * @note Returns minimum across all hash function buckets
     * @note Always returns estimate >= true count
     */
    [[nodiscard]] uint64_t estimate(std::string_view key) const {
        uint64_t mn = std::numeric_limits<uint64_t>::max();
        for (std::size_t i = 0; i < d_; ++i)
            mn = std::min(mn, table_[i][hash_fn(key, static_cast<uint32_t>(i) * 0x9e3779b9u)]);
        return mn;
    }

    /**
     * @brief Reset all counters to zero
     */
    void clear() {
        for (auto& row : table_) std::fill(row.begin(), row.end(), 0);
    }
};

}  // namespace resq::dsa
