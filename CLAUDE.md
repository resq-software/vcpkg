# ResQ vcpkg Packages -- Agent Guide

## Mission
Registry repository for ResQ C++ libraries distributed via vcpkg. Contains resq-common: a zero-dependency, header-only C++17 utility library.

## Stack
- Language: C++17 (stdlib only -- zero external dependencies)
- Build: CMake 3.16+
- Testing: Raw assertions (no framework dependency)
- CI: GitHub Actions (gcc, clang, MSVC matrix)

## Repo Map
- `packages/resq-common/include/resq/dsa/` -- Bloom filter, Count-Min, Graph, Heap, Trie
- `packages/resq-common/include/resq/` -- Result, file_utils, string_utils, array_utils, env_utils
- `packages/resq-common/include/resq/common/` -- geo, time, nfz domain helpers
- `packages/resq-common/tests/` -- test_dsa.cpp, test_utilities.cpp

## Commands
```bash
cmake -B build -DRESQ_COMMON_BUILD_TESTS=ON packages/resq-common
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

## Rules
- Zero external dependencies -- stdlib only
- Header-only INTERFACE library
- CMake target: `resq::common`
- All new utilities require tests
- All headers use `#pragma once` include guards

## Safety
- Don't add external dependencies
- Don't break the public API without versioning
- Don't remove or rename public headers without a major version bump
