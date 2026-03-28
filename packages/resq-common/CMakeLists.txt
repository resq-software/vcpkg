cmake_minimum_required(VERSION 3.16)
project(resq-common VERSION 0.1.0 LANGUAGES CXX)

add_library(resq-common INTERFACE)
add_library(resq::common ALIAS resq-common)

target_include_directories(resq-common
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(resq-common INTERFACE cxx_std_17)

# Tests
option(RESQ_COMMON_BUILD_TESTS "Build tests" ON)
if(RESQ_COMMON_BUILD_TESTS)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.16.0
  )
  # Prevent GoogleTest from overriding parent project settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing()
  include(GoogleTest)

  add_executable(test_dsa tests/test_dsa.cpp)
  target_link_libraries(test_dsa PRIVATE resq::common GTest::gtest_main)
  gtest_discover_tests(test_dsa)

  add_executable(test_utilities tests/test_utilities.cpp)
  target_link_libraries(test_utilities PRIVATE resq::common GTest::gtest_main)
  gtest_discover_tests(test_utilities)
endif()

# Install
install(TARGETS resq-common EXPORT resq-common-targets)
install(DIRECTORY include/ DESTINATION include)
install(EXPORT resq-common-targets
  FILE resq-common-config.cmake
  NAMESPACE resq::
  DESTINATION lib/cmake/resq-common
)
