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

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "result.hpp"

namespace resq {
namespace fs = std::filesystem;

struct FileUtils {
    /**
     * @brief Check if directory exists
     */
    [[nodiscard]] static bool directory_exists(const std::string& dir_path) noexcept {
        std::error_code ec;
        return fs::is_directory(dir_path, ec);
    }

    /**
     * @brief Check if file exists
     */
    [[nodiscard]] static bool file_exists(const std::string& file_path) noexcept {
        std::error_code ec;
        return fs::is_regular_file(file_path, ec);
    }

    /**
     * @brief Check if path exists (file or directory)
     */
    [[nodiscard]] static bool path_exists(const std::string& path) noexcept {
        std::error_code ec;
        return fs::exists(path, ec);
    }

    /**
     * @brief Create directory (and parent directories if needed)
     */
    [[nodiscard]] static Result<void> create_directory(const std::string& dir_path) {
        std::error_code ec;
        if (fs::create_directories(dir_path, ec)) {
            return Result<void>::Ok();
        }
        if (fs::exists(dir_path)) {
            return Result<void>::Ok();  // Already exists
        }
        return Result<void>::Err(500, "Failed to create directory: " + ec.message());
    }

    /**
     * @brief Copy directory (recursive)
     * Uses hard links when possible for efficiency (like Typesense)
     */
    [[nodiscard]] static Result<void> copy_directory(const std::string& from_path,
                                                     const std::string& to_path) {
        std::error_code ec;

        // Try to create destination
        fs::create_directories(to_path, ec);
        if (ec) {
            return Result<void>::Err(500, "Failed to create destination: " + ec.message());
        }

        // Copy recursively
        fs::copy(from_path, to_path,
                 fs::copy_options::recursive | fs::copy_options::create_hard_links |
                     fs::copy_options::skip_existing,
                 ec);

        if (ec) {
            // Fallback to regular copy if hard links fail
            fs::copy(from_path, to_path,
                     fs::copy_options::recursive | fs::copy_options::skip_existing, ec);

            if (ec) {
                return Result<void>::Err(500, "Failed to copy directory: " + ec.message());
            }
        }

        return Result<void>::Ok();
    }

    /**
     * @brief Move/rename path (works for files and directories)
     */
    [[nodiscard]] static Result<void> move_path(const std::string& from_path,
                                                const std::string& to_path) {
        std::error_code ec;
        fs::rename(from_path, to_path, ec);

        if (ec) {
            return Result<void>::Err(500, "Failed to move: " + ec.message());
        }

        return Result<void>::Ok();
    }

    /**
     * @brief Delete path (file or directory)
     * @param recursive If true, delete directory contents recursively
     */
    [[nodiscard]] static Result<void> delete_path(const std::string& path, bool recursive = true) {
        std::error_code ec;

        if (recursive) {
            fs::remove_all(path, ec);
        } else {
            fs::remove(path, ec);
        }

        if (ec && fs::exists(path)) {
            return Result<void>::Err(500, "Failed to delete: " + ec.message());
        }

        return Result<void>::Ok();
    }

    /**
     * @brief Get file size in bytes
     */
    [[nodiscard]] static Result<size_t> file_size(const std::string& file_path) {
        std::error_code ec;
        auto size = fs::file_size(file_path, ec);

        if (ec) {
            return Result<size_t>::Err(404, "Failed to get file size: " + ec.message());
        }

        return Result<size_t>::Ok(size);
    }

    /**
     * @brief List files in directory
     * @param pattern Optional glob pattern (e.g., "*.txt")
     */
    [[nodiscard]] static Result<std::vector<std::string>> list_files(
        const std::string& dir_path, const std::string& pattern = "") {
        std::vector<std::string> files;
        std::error_code ec;

        if (!fs::is_directory(dir_path, ec)) {
            return Result<std::vector<std::string>>::Err(404, "Not a directory");
        }

        for (const auto& entry : fs::directory_iterator(dir_path, ec)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();

                // Simple glob pattern matching (just suffix for now)
                if (pattern.empty() || filename.find(pattern) != std::string::npos) {
                    files.push_back(entry.path().string());
                }
            }
        }

        if (ec) {
            return Result<std::vector<std::string>>::Err(
                500, "Error reading directory: " + ec.message());
        }

        return Result<std::vector<std::string>>::Ok(std::move(files));
    }

    /**
     * @brief Read entire file into string
     */
    [[nodiscard]] static Result<std::string> read_file(const std::string& file_path) {
        std::ifstream file(file_path, std::ios::binary);

        if (!file) {
            return Result<std::string>::Err(404, "Failed to open file: " + file_path);
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

        return Result<std::string>::Ok(std::move(content));
    }

    /**
     * @brief Read file into vector of lines
     */
    [[nodiscard]] static Result<std::vector<std::string>> read_lines(const std::string& file_path) {
        std::ifstream file(file_path);

        if (!file) {
            return Result<std::vector<std::string>>::Err(404, "Failed to open file: " + file_path);
        }

        std::vector<std::string> lines;
        std::string line;

        while (std::getline(file, line)) {
            lines.push_back(std::move(line));
        }

        return Result<std::vector<std::string>>::Ok(std::move(lines));
    }

    /**
     * @brief Write string to file (overwrites existing)
     */
    [[nodiscard]] static Result<void> write_file(const std::string& file_path,
                                                 const std::string& content) {
        std::ofstream file(file_path, std::ios::binary);

        if (!file) {
            return Result<void>::Err(500, "Failed to open file for writing: " + file_path);
        }

        file << content;

        if (!file) {
            return Result<void>::Err(500, "Failed to write to file: " + file_path);
        }

        return Result<void>::Ok();
    }

    /**
     * @brief Append string to file
     */
    [[nodiscard]] static Result<void> append_file(const std::string& file_path,
                                                  const std::string& content) {
        std::ofstream file(file_path, std::ios::binary | std::ios::app);

        if (!file) {
            return Result<void>::Err(500, "Failed to open file for appending: " + file_path);
        }

        file << content;

        if (!file) {
            return Result<void>::Err(500, "Failed to append to file: " + file_path);
        }

        return Result<void>::Ok();
    }

    /**
     * @brief Get file extension (without dot)
     */
    [[nodiscard]] static std::string get_extension(const std::string& file_path) {
        fs::path p(file_path);
        std::string ext = p.extension().string();
        if (!ext.empty() && ext[0] == '.') {
            ext = ext.substr(1);
        }
        return ext;
    }

    /**
     * @brief Get filename without directory
     */
    [[nodiscard]] static std::string get_filename(const std::string& file_path) {
        return fs::path(file_path).filename().string();
    }

    /**
     * @brief Get directory path
     */
    [[nodiscard]] static std::string get_directory(const std::string& file_path) {
        return fs::path(file_path).parent_path().string();
    }

    /**
     * @brief Get absolute path
     */
    [[nodiscard]] static Result<std::string> absolute_path(const std::string& path) {
        std::error_code ec;
        auto abs = fs::absolute(path, ec);

        if (ec) {
            return Result<std::string>::Err(500, "Failed to get absolute path: " + ec.message());
        }

        return Result<std::string>::Ok(abs.string());
    }

    /**
     * @brief Get current working directory
     */
    [[nodiscard]] static Result<std::string> current_directory() {
        std::error_code ec;
        auto cwd = fs::current_path(ec);

        if (ec) {
            return Result<std::string>::Err(500,
                                            "Failed to get current directory: " + ec.message());
        }

        return Result<std::string>::Ok(cwd.string());
    }
};

/**
 * @brief RAII file handle that ensures closure
 *
 * Example:
 * @code
 * {
 *     auto file = FileHandle::open("data.txt", std::ios::out);
 *     if (file.is_open()) {
 *         file.stream() << "Hello, World!\n";
 *     }
 * }  // File automatically closed here
 * @endcode
 */
class FileHandle {
private:
    std::fstream stream_;

public:
    FileHandle() = default;

    [[nodiscard]] static FileHandle open(const std::string& path,
                                         std::ios::openmode mode = std::ios::in | std::ios::out) {
        FileHandle handle;
        handle.stream_.open(path, mode);
        return handle;
    }

    ~FileHandle() {
        if (stream_.is_open()) {
            stream_.close();
        }
    }

    // Delete copy, allow move
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    FileHandle(FileHandle&& other) noexcept : stream_(std::move(other.stream_)) {}

    FileHandle& operator=(FileHandle&& other) noexcept {
        if (this != &other) {
            if (stream_.is_open()) {
                stream_.close();
            }
            stream_ = std::move(other.stream_);
        }
        return *this;
    }

    [[nodiscard]] bool is_open() const { return stream_.is_open(); }
    [[nodiscard]] std::fstream& stream() { return stream_; }
};

}  // namespace resq
