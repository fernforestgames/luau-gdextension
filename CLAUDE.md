# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with
code in this repository.

## Project Overview

A Godot 4.5+ GDExtension that integrates the Luau scripting language (Lua
derivative) into Godot Engine.

**Core Classes:**

- `Luau`: Static class that compiles Luau source code to bytecode
- `LuaState`: Manages a Lua VM instance, executes bytecode, and provides
  debugging support via single-step execution
- Math type bindings in `lua_math_types.h/cpp`: Bridge between Godot math types
  and Luau. Vector3 uses Luau's native vector type for high performance. Other
  types (Vector2, Vector4, Color, etc.) use userdata with metatables for
  operators and property access.

## Project Structure

```
src/
  luau.h/cpp          - Luau compiler wrapper (static methods)
  lua_state.h/cpp     - Lua VM state management and execution
  lua_math_types.h/cpp- Godot math type bindings for Luau
  register_types.h/cpp- GDExtension registration
demo/
  main.gd             - Example integration code
  test_script.luau    - Sample Luau script with math types
  addons/luau_gdextension/
    gdextension.json  - GDExtension manifest
    build/            - Built binaries copied here by CMake
```

## Building

```bash
# Debug build
cmake --preset default
cmake --build --preset default -j

# Release build
cmake --preset release
cmake --build --preset release -j
```

Built binaries output to `bin/<platform>/` and are automatically copied to
`demo/addons/luau_gdextension/build/`

Dependencies (auto-fetched via CMake, stored in `build/_deps/`):

- Luau from https://github.com/luau-lang/luau.git
- godot-cpp from https://github.com/godotengine/godot-cpp.git

## Demo Project

The `demo/` directory contains a working Godot project demonstrating usage. See
`demo/main.gd` for example integration code and `demo/test_script.luau` for a
sample Luau script.

## Architecture

**Key Design Patterns:**

- LuaState wraps a Luau VM and manages its lifecycle
- **Vector3 performance optimization:** Vector3 is bridged to Luau's native
  `vector` type, which provides:
  - Inline arithmetic operations in the VM (no C function calls for +, -, *, /)
  - JIT compilation support for vector operations
  - Better memory layout (stored inline in stack, not as heap allocations)
  - Lower GC pressure
- Other math types are implemented as userdata with metatables
- All Lua execution happens on the main thread

## Important Notes

**Critical: Always use `call_deferred()` when resuming from signal handlers to
prevent reentrancy crashes:**

```gdscript
func _on_step(state: LuaState) -> void:
    # Do work...
    state.resume.call_deferred()  # Correct
    # state.resume()  # WRONG - will crash
```

## Documentation Resources

Read these if you ever need more information:

- The `lua.h` and `lualib.h` headers in the Luau source code, at
  `build/_deps/luau-src/VM/include`, which document the C API functions
  available.
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
  - [Vector library](https://luau.org/library#vector-library) (used for Vector3)
- The
  [documentation for godot-cpp](https://docs.godotengine.org/en/stable/tutorials/scripting/cpp/gdextension_cpp_example.html),
  used to create GDExtensions.
- The headers for godot-cpp, located in
  `build/_deps/godot-cpp-src/include/godot_cpp`, documenting the classes and
  methods available for GDExtensions.
- Godot Engine core documentation, which describes features that also exist for
  GDExtensions in godot-cpp:
  - [Common engine methods and macros](https://docs.godotengine.org/en/stable/engine_details/architecture/common_engine_methods_and_macros.html)
  - [Core types](https://docs.godotengine.org/en/stable/engine_details/architecture/core_types.html)
  - The
    [Object](https://docs.godotengine.org/en/stable/engine_details/architecture/object_class.html)
    class
