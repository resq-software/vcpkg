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
 * Tests for ResQ Common Utilities
 * Compile: g++ -std=c++17 -I../include test_utilities.cpp -o test_utilities
 * Run: ./test_utilities
 */

#include <resq/resq_common.hpp>
#include <iostream>
#include <cassert>

using namespace resq;

void test_result_type() {
    std::cout << "=== Testing Result<T> ===" << std::endl;

    // Success case
    auto ok_result = Result<int>::Ok(42);
    assert(ok_result.is_ok());
    assert(ok_result.unwrap() == 42);
    assert(ok_result.unwrap_or(0) == 42);
    std::cout << "✓ Result::Ok works" << std::endl;

    // Error case
    auto err_result = Result<int>::Err(404, "Not found");
    assert(err_result.is_err());
    assert(err_result.code() == 404);
    assert(err_result.error() == "Not found");
    assert(err_result.unwrap_or(100) == 100);
    std::cout << "✓ Result::Err works" << std::endl;

    // Map operation
    auto mapped = ok_result.map([](int x) { return x * 2; });
    assert(mapped.is_ok());
    assert(mapped.unwrap() == 84);
    std::cout << "✓ Result::map works" << std::endl;

    // Result<void>
    auto void_ok = Result<void>::Ok();
    assert(void_ok.is_ok());
    auto void_err = Result<void>::Err(500, "Internal error");
    assert(void_err.is_err());
    std::cout << "✓ Result<void> works" << std::endl;

    std::cout << std::endl;
}

void test_string_utils() {
    std::cout << "=== Testing StringUtils ===" << std::endl;

    // Split
    std::vector<std::string> parts;
    StringUtils::split("drone1,drone2,drone3", parts, ",");
    assert(parts.size() == 3);
    assert(parts[0] == "drone1");
    std::cout << "✓ StringUtils::split works" << std::endl;

    // Join
    std::string joined = StringUtils::join(parts, " | ");
    assert(joined == "drone1 | drone2 | drone3");
    std::cout << "✓ StringUtils::join works" << std::endl;

    // Trim
    std::string trimmed = StringUtils::trim("  hello  ");
    assert(trimmed == "hello");
    std::cout << "✓ StringUtils::trim works" << std::endl;

    // Case conversion
    assert(StringUtils::to_upper("hello") == "HELLO");
    assert(StringUtils::to_lower("WORLD") == "world");
    std::cout << "✓ StringUtils case conversion works" << std::endl;

    // Contains/starts_with/ends_with
    assert(StringUtils::contains("hello world", "world"));
    assert(StringUtils::starts_with("hello", "hel"));
    assert(StringUtils::ends_with("hello", "llo"));
    std::cout << "✓ StringUtils predicates work" << std::endl;

    // Replace all
    std::string replaced = StringUtils::replace_all("foo bar foo", "foo", "baz");
    assert(replaced == "baz bar baz");
    std::cout << "✓ StringUtils::replace_all works" << std::endl;

    // Quote-respecting split
    std::vector<std::string> quoted;
    StringUtils::split_respecting_quotes("field1,\"value,with,comma\",field3", quoted, ',');
    assert(quoted.size() == 3);
    assert(quoted[1] == "value,with,comma");
    std::cout << "✓ StringUtils::split_respecting_quotes works" << std::endl;

    std::cout << std::endl;
}

void test_file_utils() {
    std::cout << "=== Testing FileUtils ===" << std::endl;

    const std::string test_file = "/tmp/resq_test.txt";
    const std::string test_dir = "/tmp/resq_test_dir";

    // Write file
    auto write_result = FileUtils::write_file(test_file, "Hello, ResQ!");
    assert(write_result.is_ok());
    std::cout << "✓ FileUtils::write_file works" << std::endl;

    // File exists
    assert(FileUtils::file_exists(test_file));
    std::cout << "✓ FileUtils::file_exists works" << std::endl;

    // Read file
    auto read_result = FileUtils::read_file(test_file);
    assert(read_result.is_ok());
    assert(read_result.unwrap() == "Hello, ResQ!");
    std::cout << "✓ FileUtils::read_file works" << std::endl;

    // Append file
    auto append_result = FileUtils::append_file(test_file, "\nLine 2");
    assert(append_result.is_ok());
    std::cout << "✓ FileUtils::append_file works" << std::endl;

    // Read lines
    auto lines_result = FileUtils::read_lines(test_file);
    assert(lines_result.is_ok());
    assert(lines_result.unwrap().size() == 2);
    std::cout << "✓ FileUtils::read_lines works" << std::endl;

    // File size
    auto size_result = FileUtils::file_size(test_file);
    assert(size_result.is_ok());
    assert(size_result.unwrap() > 0);
    std::cout << "✓ FileUtils::file_size works" << std::endl;

    // Create directory
    auto mkdir_result = FileUtils::create_directory(test_dir);
    assert(mkdir_result.is_ok());
    assert(FileUtils::directory_exists(test_dir));
    std::cout << "✓ FileUtils::create_directory works" << std::endl;

    // Get path components
    assert(FileUtils::get_extension("test.txt") == "txt");
    assert(FileUtils::get_filename("/path/to/file.txt") == "file.txt");
    std::cout << "✓ FileUtils path utilities work" << std::endl;

    // Cleanup
    FileUtils::delete_path(test_file);
    FileUtils::delete_path(test_dir);
    std::cout << "✓ FileUtils cleanup works" << std::endl;

    std::cout << std::endl;
}

void test_array_utils() {
    std::cout << "=== Testing ArrayUtils ===" << std::endl;

    // Test with vectors (easier syntax)
    std::vector<uint32_t> a = {1, 3, 5, 7, 9};
    std::vector<uint32_t> b = {3, 5, 11, 13};

    // Intersection
    auto intersection = ArrayUtils::intersect(a, b);
    assert(intersection.size() == 2);
    assert(intersection[0] == 3);
    assert(intersection[1] == 5);
    std::cout << "✓ ArrayUtils::intersect works: {3, 5}" << std::endl;

    // Union
    auto union_result = ArrayUtils::union_of(a, b);
    assert(union_result.size() == 7);  // {1, 3, 5, 7, 9, 11, 13}
    std::cout << "✓ ArrayUtils::union_of works: {1,3,5,7,9,11,13}" << std::endl;

    // Exclude (a - b)
    auto excluded = ArrayUtils::exclude(a, b);
    assert(excluded.size() == 3);  // {1, 7, 9}
    assert(excluded[0] == 1);
    assert(excluded[1] == 7);
    assert(excluded[2] == 9);
    std::cout << "✓ ArrayUtils::exclude works: {1, 7, 9}" << std::endl;

    // Contains
    assert(ArrayUtils::contains(a, 5));
    assert(!ArrayUtils::contains(a, 4));
    std::cout << "✓ ArrayUtils::contains works" << std::endl;

    // Is sorted
    assert(ArrayUtils::is_sorted(a.data(), a.size()));
    std::cout << "✓ ArrayUtils::is_sorted works" << std::endl;

    // Binary search with skip
    uint32_t idx = 0;
    bool found = ArrayUtils::skip_index_to_id(idx, a.data(), a.size(), 5);
    assert(found);
    assert(a[idx] == 5);
    std::cout << "✓ ArrayUtils::skip_index_to_id works" << std::endl;

    std::cout << std::endl;
}

void test_integration() {
    std::cout << "=== Integration Test ===" << std::endl;

    // Simulate drone filtering workflow
    std::cout << "Scenario: Filter available drones" << std::endl;

    // All drones in fleet
    std::vector<uint32_t> all_drones = {1, 2, 3, 4, 5, 6, 7, 8};

    // Offline drones
    std::vector<uint32_t> offline = {2, 5, 8};

    // Currently assigned
    std::vector<uint32_t> assigned = {3, 7};

    // Find available = all - offline - assigned
    auto not_offline = ArrayUtils::exclude(all_drones, offline);
    auto available = ArrayUtils::exclude(not_offline, assigned);

    assert(available.size() == 3);  // {1, 4, 6}
    std::cout << "Available drones: ";
    for (auto id : available) {
        std::cout << id << " ";
    }
    std::cout << std::endl;

    // Write to file using Result pattern
    std::vector<std::string> drone_ids;
    for (auto id : available) {
        drone_ids.push_back("DRONE-" + std::to_string(id));
    }

    std::string csv = StringUtils::join(drone_ids, ", ");
    auto result = FileUtils::write_file("/tmp/available_drones.txt", csv);

    if (result.is_ok()) {
        std::cout << "✓ Wrote available drones to file" << std::endl;
    } else {
        std::cout << "✗ Failed: " << result.error() << std::endl;
    }

    // Cleanup
    FileUtils::delete_path("/tmp/available_drones.txt");

    std::cout << std::endl;
}

int main() {
    std::cout << "╔════════════════════════════════════════╗" << std::endl;
    std::cout << "║  ResQ Common Utilities Test Suite     ║" << std::endl;
    std::cout << "║  Version: " << RESQ_COMMON_VERSION << "                           ║" << std::endl;
    std::cout << "╚════════════════════════════════════════╝" << std::endl;
    std::cout << std::endl;

    try {
        test_result_type();
        test_string_utils();
        test_file_utils();
        test_array_utils();
        test_integration();

        std::cout << "╔════════════════════════════════════════╗" << std::endl;
        std::cout << "║  ✓ All Tests Passed!                   ║" << std::endl;
        std::cout << "╚════════════════════════════════════════╝" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "✗ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
