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
#include <cctype>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace resq {

struct StringUtils {
    /**
     * @brief Split string by delimiter with optional trimming
     *
     * @param s Input string
     * @param result Output vector (appends to existing elements)
     * @param delim Delimiter string
     * @param keep_empty Keep empty tokens
     * @param trim_space Trim whitespace from each token
     * @return Number of characters processed
     *
     * Example:
     * @code
     * std::vector<std::string> parts;
     * StringUtils::split("drone1,drone2, drone3", parts, ",");
     * // parts = {"drone1", "drone2", "drone3"}
     * @endcode
     */
    [[nodiscard]] static size_t split(const std::string& s, std::vector<std::string>& result,
                                      const std::string& delim, bool keep_empty = false,
                                      bool trim_space = true) {
        if (delim.empty()) {
            result.push_back(s);
            return s.size();
        }

        size_t start = 0;
        size_t end = s.find(delim);

        while (end != std::string::npos) {
            std::string token = s.substr(start, end - start);

            if (trim_space) {
                token = trim(token);
            }

            if (keep_empty || !token.empty()) {
                result.push_back(token);
            }

            start = end + delim.length();
            end = s.find(delim, start);
        }

        // Add remaining part
        std::string token = s.substr(start);
        if (trim_space) {
            token = trim(token);
        }
        if (keep_empty || !token.empty()) {
            result.push_back(token);
        }

        return s.size();
    }

    /**
     * @brief Split string by single character delimiter (faster)
     */
    [[nodiscard]] static size_t split(const std::string& s, std::vector<std::string>& result,
                                      char delim, bool keep_empty = false, bool trim_space = true) {
        std::stringstream ss(s);
        std::string token;

        while (std::getline(ss, token, delim)) {
            if (trim_space) {
                token = trim(token);
            }
            if (keep_empty || !token.empty()) {
                result.push_back(token);
            }
        }

        return s.size();
    }

    /**
     * @brief Trim whitespace from both ends
     */
    [[nodiscard]] static std::string trim(const std::string& s) {
        auto start =
            std::find_if_not(s.begin(), s.end(), [](unsigned char ch) { return std::isspace(ch); });

        auto end = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char ch) {
                       return std::isspace(ch);
                   }).base();

        return (start < end) ? std::string(start, end) : std::string();
    }

    /**
     * @brief Trim whitespace from left
     */
    [[nodiscard]] static std::string ltrim(const std::string& s) {
        auto start =
            std::find_if_not(s.begin(), s.end(), [](unsigned char ch) { return std::isspace(ch); });
        return std::string(start, s.end());
    }

    /**
     * @brief Trim whitespace from right
     */
    [[nodiscard]] static std::string rtrim(const std::string& s) {
        auto end = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char ch) {
                       return std::isspace(ch);
                   }).base();
        return std::string(s.begin(), end);
    }

    /**
     * @brief Join strings with delimiter
     *
     * Example:
     * @code
     * std::vector<std::string> ids = {"DRONE-1", "DRONE-2", "DRONE-3"};
     * std::string csv = StringUtils::join(ids, ", ");
     * // csv = "DRONE-1, DRONE-2, DRONE-3"
     * @endcode
     */
    [[nodiscard]] static std::string join(const std::vector<std::string>& parts,
                                          const std::string& delimiter) {
        if (parts.empty()) return "";

        std::ostringstream result;
        result << parts[0];

        for (size_t i = 1; i < parts.size(); ++i) {
            result << delimiter << parts[i];
        }

        return result.str();
    }

    /**
     * @brief Convert string to lowercase
     */
    [[nodiscard]] static std::string to_lower(const std::string& s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    /**
     * @brief Convert string to uppercase
     */
    [[nodiscard]] static std::string to_upper(const std::string& s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        return result;
    }

    /**
     * @brief Check if string starts with prefix
     */
    [[nodiscard]] static bool starts_with(std::string_view s, std::string_view prefix) {
        return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
    }

    /**
     * @brief Check if string ends with suffix
     */
    [[nodiscard]] static bool ends_with(std::string_view s, std::string_view suffix) {
        return s.size() >= suffix.size() && s.substr(s.size() - suffix.size()) == suffix;
    }

    /**
     * @brief Check if string contains substring
     */
    [[nodiscard]] static bool contains(std::string_view s, std::string_view substr) {
        return s.find(substr) != std::string_view::npos;
    }

    /**
     * @brief Replace all occurrences of 'from' with 'to'
     */
    [[nodiscard]] static std::string replace_all(std::string s, const std::string& from,
                                                 const std::string& to) {
        if (from.empty()) return s;

        size_t start_pos = 0;
        while ((start_pos = s.find(from, start_pos)) != std::string::npos) {
            s.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
        return s;
    }

    /**
     * @brief Split keeping quoted sections intact
     *
     * Example:
     * @code
     * std::vector<std::string> parts;
     * StringUtils::split_respecting_quotes("field1,\"value,with,commas\",field3", parts, ',', '"');
     * // parts = {"field1", "value,with,commas", "field3"}
     * @endcode
     */
    static void split_respecting_quotes(const std::string& s, std::vector<std::string>& result,
                                        char delimiter, char quote_char = '"') {
        bool in_quotes = false;
        std::string current;

        for (char c : s) {
            if (c == quote_char) {
                in_quotes = !in_quotes;
            } else if (c == delimiter && !in_quotes) {
                result.push_back(trim(current));
                current.clear();
            } else {
                current += c;
            }
        }

        if (!current.empty()) {
            result.push_back(trim(current));
        }
    }

    /**
     * @brief Remove all whitespace from string
     */
    [[nodiscard]] static std::string remove_whitespace(const std::string& s) {
        std::string result;
        result.reserve(s.size());
        std::copy_if(s.begin(), s.end(), std::back_inserter(result),
                     [](unsigned char c) { return !std::isspace(c); });
        return result;
    }

    /**
     * @brief Pad string to width with fill character
     */
    [[nodiscard]] static std::string pad_left(const std::string& s, size_t width, char fill = ' ') {
        if (s.size() >= width) return s;
        return std::string(width - s.size(), fill) + s;
    }

    [[nodiscard]] static std::string pad_right(const std::string& s, size_t width,
                                               char fill = ' ') {
        if (s.size() >= width) return s;
        return s + std::string(width - s.size(), fill);
    }

    /**
     * @brief Escape special characters for JSON
     */
    [[nodiscard]] static std::string escape_json(const std::string& s) {
        std::string result;
        result.reserve(s.size());

        for (char c : s) {
            switch (c) {
                case '"':
                    result += "\\\"";
                    break;
                case '\\':
                    result += "\\\\";
                    break;
                case '\b':
                    result += "\\b";
                    break;
                case '\f':
                    result += "\\f";
                    break;
                case '\n':
                    result += "\\n";
                    break;
                case '\r':
                    result += "\\r";
                    break;
                case '\t':
                    result += "\\t";
                    break;
                default:
                    result += c;
                    break;
            }
        }

        return result;
    }
};

}  // namespace resq
