# sol3 (sol2 v3.x)

This directory contains the sol3 header-only C++ binding library for Lua.

## Source

- Version: v3.5.0
- Repository: https://github.com/ThePhD/sol2
- License: MIT License

## Purpose

sol3 is a modern C++17/20 binding library for Lua, providing:
- Type-safe Lua bindings with template metaprogramming
- Zero-overhead abstraction (performance close to raw Lua C API)
- Full support for classes, inheritance, properties, enums, and containers
- Exception-safe operation with configurable error handling
- Header-only library (no compilation required)

## Integration

sol3 is used by nim_duilib's Lua binding layer to expose C++ UI classes to Lua scripts.

## Usage

```cpp
#include "duilib/third_party/sol/include/sol/sol.hpp"

sol::state lua;
lua.open_libraries(sol::lib::base);

// Bind C++ class
lua.new_usertype<MyClass>("MyClass",
    "method", &MyClass::method,
    "property", sol::property(&MyClass::get, &MyClass::set)
);
```

## Files

All header files from sol3 v3.5.0 release are included in the `include/sol/`
subdirectory. The main header is `sol.hpp`.

## Dependencies

- Requires Lua 5.4.x (provided in `../lua/`)
- Requires C++17 or later (nim_duilib uses C++20)
