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
 * @brief Trie (prefix tree) and string matching algorithms
 *
 * Provides efficient string data structures for:
 * - Prefix-based autocomplete
 * - String storage and lookup
 * - Pattern matching
 *
 * @note Uses case-sensitive ASCII character comparison
 * @warning Not suitable for Unicode strings without modification
 *
 * @par Example:
 * @code
 * Trie trie;
 * trie.insert("drone");
 * trie.insert("drone_001");
 * trie.insert("drone_002");
 *
 * auto suggestions = trie.starts_with("drone_");
 * // suggestions = {"drone_001", "drone_002"}
 * @endcode
 */

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace resq::dsa {

/**
 * @brief Trie (prefix tree) for efficient string prefix operations
 *
 * Space-optimized prefix tree supporting insert, exact search,
 * and prefix-based autocomplete operations.
 *
 * @note Memory: O(total characters stored) - shared prefixes
 * @note Operations: O(length of string) for insert/search
 *
 * @par Example:
 * @code
 * Trie t;
 * t.insert("api");
 * t.insert("app");
 * t.insert("apple");
 *
 * t.search("api");              // true
 * t.search("ap");               // false
 * t.starts_with("ap");          // {"api", "app", "apple"}
 * @endcode
 */
class Trie {
    /**
     * @brief Internal trie node
     */
    struct Node {
        /// Children nodes keyed by character
        std::unordered_map<char, std::unique_ptr<Node>> ch;
        /// Whether this node marks end of a word
        bool is_end = false;
    };
    /// Root node
    Node root_;

    /**
     * @brief Collect all words with given prefix
     */
    void collect(const Node& n, std::string& acc, std::vector<std::string>& out) const {
        if (n.is_end) out.push_back(acc);
        for (auto& [c, child] : n.ch) {
            acc.push_back(c);
            collect(*child, acc, out);
            acc.pop_back();
        }
    }

public:
    /**
     * @brief Insert a word into the trie
     *
     * @param w Word to insert (ASCII characters)
     *
     * @post Word can be found via search()
     * @note Duplicate inserts are idempotent
     */
    void insert(std::string_view w) {
        Node* n = &root_;
        for (char c : w) {
            if (!n->ch.count(c)) n->ch[c] = std::make_unique<Node>();
            n = n->ch[c].get();
        }
        n->is_end = true;
    }

    /**
     * @brief Search for exact word
     *
     * @param w Word to search for
     * @return true if word exists in trie
     */
    [[nodiscard]] bool search(std::string_view w) const {
        const Node* n = &root_;
        for (char c : w) {
            auto it = n->ch.find(c);
            if (it == n->ch.end()) return false;
            n = it->second.get();
        }
        return n->is_end;
    }

    /**
     * @brief Find all words starting with prefix
     *
     * @param prefix Prefix to search for
     * @return Vector of all words starting with prefix
     *
     * @post Returns empty vector if prefix not found
     */
    [[nodiscard]] std::vector<std::string> starts_with(std::string_view prefix) const {
        const Node* n = &root_;
        for (char c : prefix) {
            auto it = n->ch.find(c);
            if (it == n->ch.end()) return {};
            n = it->second.get();
        }
        std::vector<std::string> r;
        std::string acc(prefix);
        collect(*n, acc, r);
        return r;
    }
};

/**
 * @brief Rabin-Karp string matching algorithm
 *
 * Uses rolling hash for O(n + m) average-case string matching.
 * Useful for finding all occurrences of a pattern in text.
 *
 * @param text Text to search in
 * @param pat Pattern to search for
 * @return Vector of starting positions where pattern matches
 *
 * @note Uses base 31 with modulo 1,000,000,007
 * @note False matches are possible due to hash collisions
 *
 * @par Example:
 * @code
 * auto matches = rabin_karp("ABABDABACDABABCABAB", "ABABCABAB");
 * // matches = {10}
 * @endcode
 */
[[nodiscard]] inline std::vector<std::size_t> rabin_karp(const std::string& text,
                                                         const std::string& pat) {
    std::size_t n = text.size(), m = pat.size();
    std::vector<std::size_t> matches;
    if (m > n || !m) return matches;
    const uint64_t B = 31, M = 1'000'000'007;
    std::vector<uint64_t> pw(m, 1);
    for (std::size_t i = 1; i < m; ++i) pw[i] = pw[i - 1] * B % M;
    auto cv = [](char c) -> uint64_t {
        return static_cast<uint64_t>(static_cast<unsigned char>(c)) + 1;
    };
    uint64_t ph = 0, wh = 0;
    for (std::size_t i = 0; i < m; ++i) {
        ph = (ph + cv(pat[i]) * pw[m - 1 - i]) % M;
        wh = (wh + cv(text[i]) * pw[m - 1 - i]) % M;
    }
    if (wh == ph && text.substr(0, m) == pat) matches.push_back(0);
    for (std::size_t i = 1; i + m <= n; ++i) {
        wh = (wh + M - cv(text[i - 1]) * pw[m - 1] % M) % M;
        wh = (wh * B + cv(text[i + m - 1])) % M;
        if (wh == ph && text.substr(i, m) == pat) matches.push_back(i);
    }
    return matches;
}

}  // namespace resq::dsa
