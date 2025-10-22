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
- `LuaCallable` in `lua_callable.h/cpp`: Wraps Lua functions as Godot Callables,
  enabling bidirectional callable bridging between Godot and Luau. Uses manual
  reference counting to manage LuaState lifetime.

## Project Structure

```
src/
  luau.h/cpp          - Luau compiler wrapper (static methods)
  lua_state.h/cpp     - Lua VM state management and execution
  lua_math_types.h/cpp- Godot math type bindings for Luau
  lua_callable.h/cpp  - Callable bridging (Lua functions ↔ Godot Callables)
  lua_godotlib.h/cpp  - Godot type userdata and metatables for Luau
  register_types.h/cpp- GDExtension registration
demo/
  main.gd             - Example integration code
  test_script.luau    - Sample Luau script with math types
  addons/
    luau_gdextension/
      gdextension.json  - GDExtension manifest
      bin/              - Built binaries copied here by CMake
    gut/                - GUT testing framework for GDScript tests
  test/                 - GDScript integration tests
tests/
  test_*.cpp          - C++ unit tests using doctest
  doctest.h           - doctest testing framework header
  README.md           - Detailed testing documentation
```

## Building

Build the preset appropriate for the current platform—for example:

```bash
cmake --preset windows-x86_64-debug
cmake --build --preset windows-x86_64-debug -j
```

```bash
cmake --preset linux-x86_64-debug
cmake --build --preset linux-x86_64-debug -j
```

```bash
cmake --preset macos-arm64-debug
cmake --build --preset macos-arm64-debug -j
```

Built binaries output to `build/bin/` and are automatically copied to
`demo/addons/luau_gdextension/bin/`

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
- **Callable bridging:** Bidirectional conversion between Lua functions and
  Godot Callables:
  - Lua functions → Godot Callables: Wrapped in `LuaCallable` class using manual
    reference counting to keep LuaState alive
  - Godot Callables → Lua: Stored as userdata with `__call` metamethod
  - Error handling: Prints errors and returns nil on failure
  - Multiple returns: Returns first value with warning
- All Lua execution happens on the main thread

## Testing

This project has comprehensive automated tests for all Godot-Luau bridging
functionality.

**Test Infrastructure:**

- **Runtime-Embedded C++ Tests (doctest):** All bridging layer tests running
  inside Godot runtime
  - Located in `src/tests_runtime.cpp`
  - Tests both POD types (Vector2, Color) and runtime types (Array, Dictionary,
    Variant)
  - Embedded in Debug builds only, exposed via `LuauGDExtensionTests` class
  - Run via `godot --headless --path demo/ -- --run-tests`
- **GDScript Integration Tests (GUT):** High-level tests from Godot's
  perspective
  - Located in `demo/test/test_*_integration.gd`
  - Tests full round-trip conversions and Lua script execution
  - Run via GUT panel in Godot editor or via `--run-tests` flag

**Running Tests:**

Tests are available in Debug builds, and can be run via Godot:

```bash
godot --headless --path demo/ -- --run-tests
```

**Why Runtime-Embedded Tests?** Godot types like Array, Dictionary, and Variant
require full Godot runtime initialization. By embedding tests in the GDExtension
and running them inside Godot, we can test everything in one place with a
realistic environment.

**Coverage:**

- ✅ All math types (Vector2, Vector2i, Vector3, Vector3i, Color, Vector4, etc.)
- ✅ Array/Dictionary bridging (including nested structures)
- ✅ Variant conversions (all supported types)
- ✅ Array vs Dictionary detection logic
- ✅ Callable bridging (Lua functions ↔ Godot Callables)
- ✅ Edge cases and error handling

See `tests/README.md` for detailed testing documentation and best practices.

**When Adding New Features:**

1. Write C++ tests in `src/tests_runtime.cpp`
2. Write GDScript integration tests in `demo/test/`
3. Ensure all tests pass before committing
4. CI automatically runs all tests on push/PR

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
