# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with
code in this repository.

## Project Overview

A Godot 4.5+ GDExtension that integrates the Luau scripting language (Lua
derivative) into Godot Engine.

**Core Classes:**

- `Luau`: Compiles Luau source code to bytecode
- `LuaState`: Executes Luau bytecode with step-by-step debugging support

## Building

```bash
# Debug build
cmake --preset default
cmake --build --preset default -j

# Release build
cmake --preset release
cmake --build --preset release -j
```

Built binaries output to `addons/luau_gdextension/build/`

Dependencies (auto-fetched via CMake, stored in
`addons/luau_gdextension/build/_deps/`):

- Luau from https://github.com/luau-lang/luau.git
- godot-cpp from https://github.com/godotengine/godot-cpp.git

## Demo Project

The root directory contains a working Godot project demonstrating usage. See
`demo/main.gd` for example integration code and `demo/test_script.luau` for a
sample Luau script.

## Important Notes

When resuming execution from a breakpoint handler, always use `call_deferred()`
to avoid crashes:

```gdscript
func _on_break(state: LuaState) -> void:
    L.resume.call_deferred()  # Correct - prevents crashes
```

## Documentation Resources

Read these if you ever need more information:

- The `lua.h` and `lualib.h` headers in the Luau source code, at
  `addons/luau_gdextension/build/_deps/luau-src/VM/include`, which document the
  C API functions available.
- [The Lua 5.1 manual](https://www.lua.org/manual/5.1/manual.html), as Luau is
  based on Lua 5.1, including the C API.
- The Luau manual pages:
  - [Syntax](https://luau.org/syntax)
  - [Linting](https://luau.org/lint)
  - [Performance](https://luau.org/performance)
  - [Sandboxing](https://luau.org/sandbox)
  - [Compatibility](https://luau.org/compatibility) (with regard to Lua)
  - [Typechecking](https://luau.org/typecheck)
  - [Standard library](https://luau.org/library)
- The
  [documentation for godot-cpp](https://docs.godotengine.org/en/stable/tutorials/scripting/cpp/gdextension_cpp_example.html),
  used to create GDExtensions.
- The headers for godot-cpp, located in
  `addons/luau_gdextension/build/_deps/godot-cpp-src/include/godot_cpp`,
  documenting the classes and methods available for GDExtensions.
- Godot Engine core documentation, which describes features that also exist for
  GDExtensions in godot-cpp:
  - [Common engine methods and macros](https://docs.godotengine.org/en/stable/engine_details/architecture/common_engine_methods_and_macros.html)
  - [Core types](https://docs.godotengine.org/en/stable/engine_details/architecture/core_types.html)
  - The
    [Object](https://docs.godotengine.org/en/stable/engine_details/architecture/object_class.html)
    class
