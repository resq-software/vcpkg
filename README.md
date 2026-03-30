# ResQ vcpkg Packages

[![CI](https://img.shields.io/github/actions/workflow/status/resq-software/vcpkg/ci.yml?branch=main&label=ci&style=flat-square)](https://github.com/resq-software/vcpkg/actions)
[![C++](https://img.shields.io/badge/C++-17-00599C?style=flat-square)](#)
[![CMake](https://img.shields.io/badge/CMake-3.20+-064F8C?style=flat-square)](#)
[![License](https://img.shields.io/badge/license-Apache--2.0-blue?style=flat-square)](LICENSE)

> C++ vcpkg libraries for the ResQ autonomous drone platform.

## Packages

| Port | Description |
|------|-------------|
| `resq-common` | Shared DSA, geo utilities, result types, and string/array/file helpers |

## Installation

**Via CMake FetchContent** (recommended):

```cmake
include(FetchContent)
FetchContent_Declare(
  resq-common
  GIT_REPOSITORY https://github.com/resq-software/vcpkg
  GIT_TAG        resq-common@v0.1.0
)
FetchContent_MakeAvailable(resq-common)
target_link_libraries(your_target PRIVATE resq::common)
```

**Via vcpkg overlay:**
```bash
vcpkg install resq-common
```

**Direct include path:**
```cmake
target_include_directories(your_target PRIVATE path/to/resq-common/include)
```

## Modules

### DSA

Probabilistic and graph data structures optimized for drone swarm coordination.

**BloomFilter** -- Probabilistic set membership with configurable false-positive rate.

```cpp
#include <resq/dsa/bloom.hpp>

resq::BloomFilter filter(10000, 0.01); // 10k elements, 1% FP rate
filter.add("drone_001");
assert(filter.possibly_contains("drone_001")); // true
```

**CountMinSketch** -- Frequency estimation for streaming data.

```cpp
#include <resq/dsa/count_min.hpp>
```

**Graph** -- Adjacency-list graph with Dijkstra and A* pathfinding.

```cpp
#include <resq/dsa/graph.hpp>

resq::Graph<std::string> g;
g.add_edge("A", "B", 1.0);
g.add_edge("B", "C", 2.0);
auto result = g.dijkstra("A", "C");
```

**BoundedHeap** -- Fixed-capacity min/max heap for top-K selection.

```cpp
#include <resq/dsa/heap.hpp>
```

**Trie** -- Prefix tree for string-based lookups.

```cpp
#include <resq/dsa/trie.hpp>
```

### Result

Rust-inspired `Result<T>` type for explicit error handling without exceptions.

```cpp
#include <resq/result.hpp>

resq::Result<int> parse_port(const std::string& s) {
    if (s.empty()) return resq::Result<int>::Err(400, "Port cannot be empty");
    int port = std::stoi(s);
    return resq::Result<int>::Ok(port);
}

auto result = parse_port("8080");
if (result.is_ok()) {
    int port = result.unwrap();
}
```

### File Utils

Safe filesystem operations with `Result`-based error handling.

```cpp
#include <resq/file_utils.hpp>
```

### String Utils

Text processing helpers (trim, split, join, case conversion).

```cpp
#include <resq/string_utils.hpp>
```

### Array Utils

SIMD-optimized operations on sorted `uint32_t` arrays (intersection, union, difference).

```cpp
#include <resq/array_utils.hpp>
```

### Geo

Geographic primitives for drone navigation using the WGS84 coordinate system. Includes distance calculations and coordinate validation.

```cpp
#include <resq/common/geo.hpp>
```

### Additional Utilities

- **`resq/env_utils.hpp`** -- Environment variable access
- **`resq/common/time.hpp`** -- Time utilities
- **`resq/common/nfz.hpp`** -- No-fly zone domain helpers
- **`resq/resq_common.hpp`** -- Convenience header (includes everything)

## Building from source

```bash
cmake -B build -DRESQ_COMMON_BUILD_TESTS=ON packages/resq-common
cmake --build build --config Debug
```

## Testing

```bash
ctest --test-dir build -C Debug --output-on-failure
```

## License

[Apache-2.0](LICENSE)
