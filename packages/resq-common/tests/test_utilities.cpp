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

#include <gtest/gtest.h>
#include <resq/resq_common.hpp>

using namespace resq;

// --- Result<T> ---

TEST(Result, OkValue) {
    auto ok_result = Result<int>::Ok(42);
    EXPECT_TRUE(ok_result.is_ok());
    EXPECT_EQ(ok_result.unwrap(), 42);
    EXPECT_EQ(ok_result.unwrap_or(0), 42);
}

TEST(Result, ErrValue) {
    auto err_result = Result<int>::Err(404, "Not found");
    EXPECT_TRUE(err_result.is_err());
    EXPECT_EQ(err_result.code(), 404);
    EXPECT_EQ(err_result.error(), "Not found");
    EXPECT_EQ(err_result.unwrap_or(100), 100);
}

TEST(Result, Map) {
    auto ok_result = Result<int>::Ok(42);
    auto mapped = ok_result.map([](int x) { return x * 2; });
    EXPECT_TRUE(mapped.is_ok());
    EXPECT_EQ(mapped.unwrap(), 84);
}

TEST(Result, VoidOkAndErr) {
    auto void_ok = Result<void>::Ok();
    EXPECT_TRUE(void_ok.is_ok());
    auto void_err = Result<void>::Err(500, "Internal error");
    EXPECT_TRUE(void_err.is_err());
}

// --- StringUtils ---

TEST(StringUtils, Split) {
    std::vector<std::string> parts;
    StringUtils::split("drone1,drone2,drone3", parts, ",");
    ASSERT_EQ(parts.size(), 3);
    EXPECT_EQ(parts[0], "drone1");
    EXPECT_EQ(parts[1], "drone2");
    EXPECT_EQ(parts[2], "drone3");
}

TEST(StringUtils, Join) {
    std::vector<std::string> parts = {"drone1", "drone2", "drone3"};
    std::string joined = StringUtils::join(parts, " | ");
    EXPECT_EQ(joined, "drone1 | drone2 | drone3");
}

TEST(StringUtils, Trim) {
    EXPECT_EQ(StringUtils::trim("  hello  "), "hello");
}

TEST(StringUtils, CaseConversion) {
    EXPECT_EQ(StringUtils::to_upper("hello"), "HELLO");
    EXPECT_EQ(StringUtils::to_lower("WORLD"), "world");
}

TEST(StringUtils, Predicates) {
    EXPECT_TRUE(StringUtils::contains("hello world", "world"));
    EXPECT_TRUE(StringUtils::starts_with("hello", "hel"));
    EXPECT_TRUE(StringUtils::ends_with("hello", "llo"));
}

TEST(StringUtils, ReplaceAll) {
    std::string replaced = StringUtils::replace_all("foo bar foo", "foo", "baz");
    EXPECT_EQ(replaced, "baz bar baz");
}

TEST(StringUtils, SplitRespectingQuotes) {
    std::vector<std::string> quoted;
    StringUtils::split_respecting_quotes("field1,\"value,with,comma\",field3", quoted, ',');
    ASSERT_EQ(quoted.size(), 3);
    EXPECT_EQ(quoted[1], "value,with,comma");
}

// --- FileUtils ---

class FileUtilsTest : public ::testing::Test {
protected:
    const std::string test_file = "/tmp/resq_gtest.txt";
    const std::string test_dir = "/tmp/resq_gtest_dir";

    void TearDown() override {
        FileUtils::delete_path(test_file);
        FileUtils::delete_path(test_dir);
    }
};

TEST_F(FileUtilsTest, WriteAndRead) {
    auto write_result = FileUtils::write_file(test_file, "Hello, ResQ!");
    ASSERT_TRUE(write_result.is_ok());
    EXPECT_TRUE(FileUtils::file_exists(test_file));

    auto read_result = FileUtils::read_file(test_file);
    ASSERT_TRUE(read_result.is_ok());
    EXPECT_EQ(read_result.unwrap(), "Hello, ResQ!");
}

TEST_F(FileUtilsTest, AppendAndReadLines) {
    FileUtils::write_file(test_file, "Hello, ResQ!");
    auto append_result = FileUtils::append_file(test_file, "\nLine 2");
    ASSERT_TRUE(append_result.is_ok());

    auto lines_result = FileUtils::read_lines(test_file);
    ASSERT_TRUE(lines_result.is_ok());
    EXPECT_EQ(lines_result.unwrap().size(), 2);
}

TEST_F(FileUtilsTest, FileSize) {
    FileUtils::write_file(test_file, "Hello, ResQ!");
    auto size_result = FileUtils::file_size(test_file);
    ASSERT_TRUE(size_result.is_ok());
    EXPECT_GT(size_result.unwrap(), 0);
}

TEST_F(FileUtilsTest, CreateDirectory) {
    auto mkdir_result = FileUtils::create_directory(test_dir);
    ASSERT_TRUE(mkdir_result.is_ok());
    EXPECT_TRUE(FileUtils::directory_exists(test_dir));
}

TEST_F(FileUtilsTest, PathComponents) {
    EXPECT_EQ(FileUtils::get_extension("test.txt"), "txt");
    EXPECT_EQ(FileUtils::get_filename("/path/to/file.txt"), "file.txt");
}

// --- ArrayUtils ---

TEST(ArrayUtils, Intersect) {
    std::vector<uint32_t> a = {1, 3, 5, 7, 9};
    std::vector<uint32_t> b = {3, 5, 11, 13};
    auto intersection = ArrayUtils::intersect(a, b);
    ASSERT_EQ(intersection.size(), 2);
    EXPECT_EQ(intersection[0], 3);
    EXPECT_EQ(intersection[1], 5);
}

TEST(ArrayUtils, UnionOf) {
    std::vector<uint32_t> a = {1, 3, 5, 7, 9};
    std::vector<uint32_t> b = {3, 5, 11, 13};
    auto union_result = ArrayUtils::union_of(a, b);
    EXPECT_EQ(union_result.size(), 7);
}

TEST(ArrayUtils, Exclude) {
    std::vector<uint32_t> a = {1, 3, 5, 7, 9};
    std::vector<uint32_t> b = {3, 5, 11, 13};
    auto excluded = ArrayUtils::exclude(a, b);
    ASSERT_EQ(excluded.size(), 3);
    EXPECT_EQ(excluded[0], 1);
    EXPECT_EQ(excluded[1], 7);
    EXPECT_EQ(excluded[2], 9);
}

TEST(ArrayUtils, Contains) {
    std::vector<uint32_t> a = {1, 3, 5, 7, 9};
    EXPECT_TRUE(ArrayUtils::contains(a, 5));
    EXPECT_FALSE(ArrayUtils::contains(a, 4));
}

TEST(ArrayUtils, IsSorted) {
    std::vector<uint32_t> a = {1, 3, 5, 7, 9};
    EXPECT_TRUE(ArrayUtils::is_sorted(a.data(), a.size()));
}

TEST(ArrayUtils, SkipIndexToId) {
    std::vector<uint32_t> a = {1, 3, 5, 7, 9};
    uint32_t idx = 0;
    bool found = ArrayUtils::skip_index_to_id(idx, a.data(), a.size(), 5);
    EXPECT_TRUE(found);
    EXPECT_EQ(a[idx], 5);
}

// --- Integration ---

TEST(Integration, DroneFilteringWorkflow) {
    std::vector<uint32_t> all_drones = {1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<uint32_t> offline = {2, 5, 8};
    std::vector<uint32_t> assigned = {3, 7};

    auto not_offline = ArrayUtils::exclude(all_drones, offline);
    auto available = ArrayUtils::exclude(not_offline, assigned);

    ASSERT_EQ(available.size(), 3);  // {1, 4, 6}

    std::vector<std::string> drone_ids;
    for (auto id : available) {
        drone_ids.push_back("DRONE-" + std::to_string(id));
    }

    std::string csv = StringUtils::join(drone_ids, ", ");
    auto result = FileUtils::write_file("/tmp/available_drones_gtest.txt", csv);
    EXPECT_TRUE(result.is_ok());

    FileUtils::delete_path("/tmp/available_drones_gtest.txt");
}
