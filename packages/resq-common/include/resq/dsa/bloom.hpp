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
 * @brief Probabilistic set membership data structure
 *
 * Space-efficient probabilistic data structure that tests whether
 * an element is a member of a set. May produce false positives
 * but never false negatives.
 *
 * Use cases:
 * - Quick membership checks for large sets
 * - Spam filtering
 * - Distributed cache lookup
 * - Network packet deduplication
 *
 * @note False positive rate is configurable at construction
 * @warning Not suitable for security-critical membership tests
 * @warning Cannot remove elements (use CountingBloomFilter instead)
 *
 * @par Example:
 * @code
 * BloomFilter filter(10000, 0.01); // 10k elements, 1% false positive rate
 * filter.add("drone_001");
 * filter.add("drone_002");
 *
 * if (filter.has("drone_001")) {
 *     // Likely in set (may be false positive)
 * }
 * @endcode
 */

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace resq::dsa {

/**
 * @brief Bloom filter for probabilistic set membership
 *
 * Implements a space-efficient probabilistic set data structure.
 * Supports insert and membership test operations.
 *
 * @tparam N/A Uses internal bit array (not template parameter)
 *
 * @invariant Filter capacity must be >= 1
 * @invariant False positive rate must be in (0, 1)
 *
 * @note Memory usage: approximately 1.2 bytes per expected element at 1% FPR
 * @note Uses FNV-1a hash with double hashing for k independent hash functions
 */
class BloomFilter {
    /// Bit array storage
    std::vector<uint8_t> bits_;
    /// Number of hash functions
    std::size_t k_;
    /// Number of bits in filter
    std::size_t m_;

    /**
     * @brief Compute hash at index i using double hashing technique
     *
     * Uses FNV-1a as base hash, then applies secondary hash for
     * generating k independent hash functions via h(i) = h1 + i*h2
     *
     * @param s String to hash
     * @param seed Secondary hash seed
     * @return Hash value modulo m_
     */
    std::size_t hash_fn(std::string_view s, uint32_t seed) const {
        uint32_t h = 2166136261u ^ seed;
        for (unsigned char c : s) {
            h ^= c;
            h *= 16777619u;
        }
        return h % m_;
    }

public:
    /**
     * @brief Construct a Bloom filter with specified capacity and false positive rate
     *
     * @param expected_elements Maximum number of elements expected to be inserted
     * @param false_positive_rate Desired false positive rate (0.0 to 1.0)
     *
     * @throws std::invalid_argument If capacity < 1
     * @throws std::invalid_argument If false_positive_rate <= 0.0 or >= 1.0
     *
     * @post Filter is initialized with all bits set to 0
     *
     * @par Example:
     * @code
     * // Create filter for 10000 elements with 1% false positive rate
     * BloomFilter filter(10000, 0.01);
     * @endcode
     */
    BloomFilter(std::size_t cap, double err = 0.01) {
        if (cap < 1) throw std::invalid_argument("cap must be >= 1");
        if (err <= 0.0 || err >= 1.0) throw std::invalid_argument("err must be in (0, 1)");
        double m =
            std::ceil(-static_cast<double>(cap) * std::log(err) / (std::log(2.0) * std::log(2.0)));
        m_ = static_cast<std::size_t>(m);
        k_ = std::max(1u, static_cast<unsigned>(std::round(m / cap * std::log(2.0))));
        bits_.assign((m_ + 7) / 8, 0);
    }

    /**
     * @brief Insert an element into the filter
     *
     * @param item String to insert
     *
     * @post Element will return true for subsequent has() calls
     * @note May set additional bits (affects other elements' FPR)
     *
     * @note Thread-safe: caller must ensure no concurrent access
     */
    void add(std::string_view item) {
        for (std::size_t i = 0; i < k_; ++i) {
            auto idx = hash_fn(item, static_cast<uint32_t>(i) * 0x9e3779b9u);
            bits_[idx >> 3] |= 1 << (idx & 7);
        }
    }

    /**
     * @brief Check if element may be in the set
     *
     * @param item String to check
     * @return true if element possibly exists, false if definitely not exists
     *
     * @retval true Element may be in set (or false positive)
     * @retval false Element is definitely not in set
     *
     * @note False positives are possible (returns true when element wasn't added)
     * @note False negatives are impossible (returns false only for non-members)
     *
     * @par Example:
     * @code
     * if (filter.has("drone_001")) {
     *     // High confidence element was added
     *     // But could be false positive
     * } else {
     *     // Definitely not in set
     * }
     * @endcode
     */
    [[nodiscard]] bool has(std::string_view item) const {
        for (std::size_t i = 0; i < k_; ++i) {
            auto idx = hash_fn(item, static_cast<uint32_t>(i) * 0x9e3779b9u);
            if (!(bits_[idx >> 3] & (1 << (idx & 7)))) return false;
        }
        return true;
    }

    /**
     * @brief Reset all bits to zero
     */
    void clear() { std::fill(bits_.begin(), bits_.end(), 0); }
};

}  // namespace resq::dsa
