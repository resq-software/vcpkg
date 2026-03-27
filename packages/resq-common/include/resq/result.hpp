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

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace resq {

/**
 * @brief Result type representing either success (Ok) or failure (Err)
 *
 * Example usage:
 * @code
 * Result<int> parse_port(const std::string& s) {
 *     if (s.empty()) {
 *         return Result<int>::Err(400, "Port cannot be empty");
 *     }
 *     int port = std::stoi(s);
 *     if (port < 1 || port > 65535) {
 *         return Result<int>::Err(400, "Port must be in range [1, 65535]");
 *     }
 *     return Result<int>::Ok(port);
 * }
 *
 * auto result = parse_port("8080");
 * if (result.is_ok()) {
 *     int port = result.unwrap();
 *     // Use port...
 * } else {
 *     spdlog::error("Parse failed: {} (code={})", result.error(), result.code());
 * }
 * @endcode
 */
template <typename T>
class Result {
private:
    T value_;
    bool is_ok_;
    std::string error_msg_;
    uint32_t error_code_;

public:
    // Delete default constructor - must be Ok or Err
    Result() = delete;

    // Copy constructor
    Result(const Result& other)
        : value_(other.value_),
          is_ok_(other.is_ok_),
          error_msg_(other.error_msg_),
          error_code_(other.error_code_) {}

    // Move constructor
    Result(Result&& other) noexcept
        : value_(std::move(other.value_)),
          is_ok_(other.is_ok_),
          error_msg_(std::move(other.error_msg_)),
          error_code_(other.error_code_) {}

    // Copy assignment
    Result& operator=(const Result& other) {
        if (this != &other) {
            value_ = other.value_;
            is_ok_ = other.is_ok_;
            error_msg_ = other.error_msg_;
            error_code_ = other.error_code_;
        }
        return *this;
    }

    // Move assignment
    Result& operator=(Result&& other) noexcept {
        if (this != &other) {
            value_ = std::move(other.value_);
            is_ok_ = other.is_ok_;
            error_msg_ = std::move(other.error_msg_);
            error_code_ = other.error_code_;
        }
        return *this;
    }

    /**
     * @brief Create a successful result
     */
    [[nodiscard]] static Result Ok(const T& value) { return Result(value, true, "", 0); }

    [[nodiscard]] static Result Ok(T&& value) { return Result(std::move(value), true, "", 0); }

    /**
     * @brief Create an error result
     * @param code Error code (use HTTP-style: 400=bad request, 404=not found, 500=internal error)
     * @param msg Human-readable error message
     */
    [[nodiscard]] static Result Err(uint32_t code, std::string_view msg) {
        return Result(T{}, false, std::string(msg), code);
    }

    /**
     * @brief Check if result is successful
     */
    [[nodiscard]] bool is_ok() const noexcept { return is_ok_; }

    /**
     * @brief Check if result is an error
     */
    [[nodiscard]] bool is_err() const noexcept { return !is_ok_; }

    /**
     * @brief Get the value (throws if error)
     * Use is_ok() to check first, or use unwrap_or()
     */
    [[nodiscard]] const T& unwrap() const {
        if (!is_ok_) {
            throw std::runtime_error("Called unwrap() on error: " + error_msg_);
        }
        return value_;
    }

    [[nodiscard]] T& unwrap() {
        if (!is_ok_) {
            throw std::runtime_error("Called unwrap() on error: " + error_msg_);
        }
        return value_;
    }

    /**
     * @brief Get the value or a default
     */
    [[nodiscard]] T unwrap_or(const T& default_value) const {
        return is_ok_ ? value_ : default_value;
    }

    /**
     * @brief Get the value or compute from error
     */
    template <typename F>
    [[nodiscard]] T unwrap_or_else(F&& op) const {
        return is_ok_ ? value_ : op(error_msg_, error_code_);
    }

    /**
     * @brief Get error message
     */
    [[nodiscard]] const std::string& error() const noexcept { return error_msg_; }

    /**
     * @brief Get error code
     */
    [[nodiscard]] uint32_t code() const noexcept { return error_code_; }

    /**
     * @brief Map the value if Ok, preserve error if Err
     */
    template <typename F>
    [[nodiscard]] auto map(F&& func) const -> Result<decltype(func(value_))> {
        using U = decltype(func(value_));
        if (is_ok_) {
            return Result<U>::Ok(func(value_));
        } else {
            return Result<U>::Err(error_code_, error_msg_);
        }
    }

    /**
     * @brief Convert to bool (true if Ok)
     */
    explicit operator bool() const noexcept { return is_ok_; }

private:
    // Private constructors - use Ok() or Err() static methods
    Result(const T& value, bool ok, std::string msg, uint32_t code)
        : value_(value), is_ok_(ok), error_msg_(std::move(msg)), error_code_(code) {}

    Result(T&& value, bool ok, std::string msg, uint32_t code)
        : value_(std::move(value)), is_ok_(ok), error_msg_(std::move(msg)), error_code_(code) {}
};

/**
 * @brief Specialization for void (operation succeeded with no return value)
 */
template <>
class Result<void> {
private:
    bool is_ok_;
    std::string error_msg_;
    uint32_t error_code_;

public:
    Result() = delete;

    [[nodiscard]] static Result Ok() { return Result(true, "", 0); }

    [[nodiscard]] static Result Err(uint32_t code, std::string_view msg) {
        return Result(false, std::string(msg), code);
    }

    [[nodiscard]] bool is_ok() const noexcept { return is_ok_; }
    [[nodiscard]] bool is_err() const noexcept { return !is_ok_; }
    [[nodiscard]] const std::string& error() const noexcept { return error_msg_; }
    [[nodiscard]] uint32_t code() const noexcept { return error_code_; }

    [[nodiscard]] void unwrap() const {
        if (!is_ok_) {
            throw std::runtime_error("Called unwrap() on error: " + error_msg_);
        }
    }

    explicit operator bool() const noexcept { return is_ok_; }

private:
    Result(bool ok, std::string msg, uint32_t code)
        : is_ok_(ok), error_msg_(std::move(msg)), error_code_(code) {}
};

}  // namespace resq
