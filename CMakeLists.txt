cmake_minimum_required(VERSION 3.16)
project(resq-vcpkg LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(packages/resq-common)
