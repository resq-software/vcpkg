/**
 * Copyright (c) 2026 ResQ Software
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file env_utils.hpp
 * @brief Shared environment variable utilities for ResQ C++ services.
 *
 * Provides type-safe environment variable loading with defaults,
 * and URL scheme validation. Used by edge-aeai and strategic-dtsop.
 */

#ifndef RESQ_ENV_UTILS_HPP
#define RESQ_ENV_UTILS_HPP

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

namespace resq {
namespace env {

/**
 * @brief Get environment variable with default value.
 *
 * @param name Environment variable name
 * @param default_value Default value if not set or empty
 * @return The environment variable value or default
 */
inline std::string get_env_or(const char* name, const char* default_value) {
    const char* value = std::getenv(name);
    if (value != nullptr && strlen(value) > 0) {
        return std::string(value);
    }
    return std::string(default_value);
}

/**
 * @brief Get environment variable as double with default.
 *
 * @param name Environment variable name
 * @param default_value Default value if not set, empty, or unparseable
 * @return Parsed double or default
 */
inline double get_env_double(const char* name, double default_value) {
    const char* value = std::getenv(name);
    if (value != nullptr && strlen(value) > 0) {
        try {
            return std::stod(value);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Invalid value for " << name
                      << ", using default: " << default_value << std::endl;
        }
    }
    return default_value;
}

/**
 * @brief Get environment variable as int with default.
 *
 * @param name Environment variable name
 * @param default_value Default value if not set, empty, or unparseable
 * @return Parsed int or default
 */
inline int get_env_int(const char* name, int default_value) {
    const char* value = std::getenv(name);
    if (value != nullptr && strlen(value) > 0) {
        try {
            return std::stoi(value);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Invalid value for " << name
                      << ", using default: " << default_value << std::endl;
        }
    }
    return default_value;
}

/**
 * @brief Get environment variable as bool with default.
 *
 * Accepts: "true", "1", "yes" (case-insensitive) as true.
 * Accepts: "false", "0", "no" (case-insensitive) as false.
 *
 * @param name Environment variable name
 * @param default_value Default value if not set, empty, or unrecognized
 * @return Parsed bool or default
 */
inline bool get_env_bool(const char* name, bool default_value) {
    const char* value = std::getenv(name);
    if (value != nullptr && strlen(value) > 0) {
        std::string val_lower;
        for (char c : std::string(value)) {
            val_lower += static_cast<char>(std::tolower(c));
        }
        if (val_lower == "true" || val_lower == "1" || val_lower == "yes") {
            return true;
        } else if (val_lower == "false" || val_lower == "0" || val_lower == "no") {
            return false;
        }
        std::cerr << "Warning: Invalid value for " << name
                  << ", using default: " << (default_value ? "true" : "false") << std::endl;
    }
    return default_value;
}

/**
 * @brief Validate that a URL environment variable has a valid HTTP(S) scheme.
 *
 * @param env_name Name of the environment variable
 * @param required If true, the variable must be set and non-empty
 * @return true if validation passes (or variable is optional and unset)
 */
inline bool validate_url_env(const char* env_name, bool required = false) {
    const char* value = std::getenv(env_name);
    if (value == nullptr || strlen(value) == 0) {
        if (required) {
            std::cerr << "Error: " << env_name << " is required but not set" << std::endl;
            return false;
        }
        return true;
    }

    std::string url(value);
    if (url.find("http://") != 0 && url.find("https://") != 0) {
        std::cerr << "Error: " << env_name << " must start with http:// or https://" << std::endl;
        return false;
    }

    return true;
}

}  // namespace env
}  // namespace resq

#endif  // RESQ_ENV_UTILS_HPP
