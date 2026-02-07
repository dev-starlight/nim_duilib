# Lua 5.4.7

This directory contains the Lua 5.4.7 source code.

## Source

- Version: 5.4.7
- Downloaded from: https://www.lua.org/ftp/lua-5.4.7.tar.gz
- Release Date: June 2024
- License: MIT License (see lua.h or https://www.lua.org/license.html)

## Purpose

Lua is embedded into nim_duilib as an optional scripting engine for UI logic.
When DUILIB_ENABLE_LUA CMake option is enabled, the Lua VM will be compiled
into the duilib library.

## Integration Notes

- Only the src/ directory is included (core Lua implementation)
- lua.c and luac.c (standalone interpreters) are excluded from build
- Used with sol3 (C++ binding library) for seamless C++ integration

## Files

All .c and .h files from the official Lua 5.4.7 source distribution are
included in the src/ subdirectory.
