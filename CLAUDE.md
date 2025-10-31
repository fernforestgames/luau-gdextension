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
- `LuauScript`: Resource type for managing Luau scripts as Godot resources
- Math type bindings in `lua_godotlib.h/cpp`: Bridge between Godot math types
  and Luau. Vector3 uses Luau's native vector type for high performance. Other
  types (Vector2, Vector4, Color, etc.) use userdata with metatables for
  operators and property access.
- `LuaCallable` in `lua_callable.h/cpp`: Wraps Lua functions as Godot Callables,
  enabling bidirectional callable bridging between Godot and Luau. Uses manual
  reference counting to manage LuaState lifetime.
- **Lua threads**: LuaState supports Lua threads (coroutines) via `new_thread()`
  and `to_thread()` methods. Thread LuaState instances share globals with the
  parent but have independent stacks. Thread lifecycles are managed via Godot's
  reference counting.

## Project Structure

```
src/
  luau.h/cpp          - Luau compiler wrapper (static methods)
  lua_state.h/cpp     - Lua VM state management and execution
  lua_callable.h/cpp  - Callable bridging (Lua functions ↔ Godot Callables)
  lua_godotlib.h/cpp  - Godot type userdata and metatables for Luau
  luau_script.h/cpp   - LuauScript resource type for managing Luau scripts
  register_types.h/cpp- GDExtension registration
demo/
  main.gd             - Example integration code
  test_script.luau    - Sample Luau script with math types
  test_runner.gd      - Entry point that runs tests or demo
  test_loader.gd      - Loads and runs GDScript integration tests
  addons/
    luau_gdextension/
      luau_gdextension.gdextension - GDExtension manifest
      bin/              - Built binaries copied here by CMake
    luau_gdextension_tests/
      luau_gdextension_tests.gdextension - Test library manifest
      bin/              - Test library binaries (Debug builds only)
    gut/                - GUT testing framework for GDScript tests
  test/                 - GDScript integration tests
tests/
  test_*.cpp          - C++ test cases (math types, arrays, etc.)
  luau_gdextension_tests.cpp - Main test runner with doctest
  register_types.h/cpp- Test library registration
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
- **Thread support:** Lua threads (coroutines) for cooperative multitasking:
  - `new_thread()` creates a thread and pushes it to the stack
  - `to_thread(index)` converts stack value to a LuaState wrapper
  - Threads keep parent alive via reference counting
  - Explicit `parent.close()` invalidates all child threads
  - Threads share global state but have independent execution stacks
  - Creating new wrapper from same thread is safe (independent ref counting)
- All Lua execution happens on a single OS thread (Lua threads are cooperative, not OS threads)

## Testing

This project has comprehensive automated tests using CMake's CTest framework.

**Test Infrastructure:**

- **C++ Tests (doctest):** Embedded in Debug builds of the GDExtension library
  - Located in `tests/test_*.cpp` (math types, arrays, dictionaries, variants,
    threads, etc.)
  - Tests both POD types (Vector2, Color) and runtime types (Array, Dictionary,
    Variant)
  - Built as separate library `gdluau_tests` when `BUILD_TESTING=ON` (enabled
    in Debug presets)
  - Has its own `.gdextension` file for independent loading
  - Run inside Godot runtime for full engine integration
- **GDScript Integration Tests:** High-level tests from Godot's perspective
  - Located in `demo/test/test_*_integration.gd`
  - Tests full round-trip conversions and Lua script execution
  - Run via `test_loader.gd` script

**Building with Tests:**

```bash
# Debug build includes test library (BUILD_TESTING=ON)
cmake --preset windows-x86_64-debug   # or linux-x86_64-debug, macos-arm64-debug
cmake --build --preset windows-x86_64-debug -j

# Release build excludes tests (BUILD_TESTING=OFF)
cmake --preset windows-x86_64-release
cmake --build --preset windows-x86_64-release -j
```

**Running Tests:**

```bash
# Using CTest (recommended)
ctest --preset windows-x86_64   # or linux-x86_64, macos-arm64

# Or directly via Godot
godot --headless --path demo/ -- --run-tests
```

**Test Architecture:** Main library (`gdluau`) never contains test code, even
in Debug builds. Test library (`gdluau_tests`) is a separate GDExtension with
its own `.gdextension` file, only built when `BUILD_TESTING=ON`. Test library
is completely optional and can be excluded from distributions. CTest drives test
execution via Godot with `--run-tests` flag.

**Coverage:**

- ✅ All math types (Vector2, Vector2i, Vector3, Vector3i, Color, Vector4, etc.)
- ✅ Array/Dictionary bridging (including nested structures)
- ✅ Variant conversions (all supported types)
- ✅ Array vs Dictionary detection logic
- ✅ Callable bridging (Lua functions ↔ Godot Callables)
- ✅ Lua threads (coroutine support, lifecycle management)
- ✅ Edge cases and error handling

See `tests/README.md` for detailed testing documentation and best practices.

**When Adding New Features:**

1. Write C++ tests in `tests/test_*.cpp` (create new test file or add to
   existing)
2. Write GDScript integration tests in `demo/test/`
3. Run `ctest --preset <platform>` to verify all tests pass
4. CI automatically runs tests on push/PR (via CTest)

**Distribution:** When packaging for distribution, simply exclude
`*_tests*` files from `demo/addons/luau_gdextension/`. This removes both the
test library binaries and the `luau_tests.gdextension` file.

## Important Notes

**Critical: Always use `call_deferred()` when resuming from signal handlers to
prevent reentrancy crashes:**

```gdscript
func _on_step(state: LuaState) -> void:
    # Do work...
    state.resume.call_deferred()  # Correct
    # state.resume()  # WRONG - will crash
```

### Lua Thread Usage

**Creating and using threads:**

```gdscript
var state = LuaState.new()
state.open_libs()
state.do_string("function coro() coroutine.yield(1); return 2 end")

# Create thread
var thread = state.new_thread()  # Pushes thread to stack
state.pop(1)  # Clean up stack

# Execute coroutine in thread
thread.get_global("coro")
assert(thread.resume(0) == LUA_YIELD)  # Yields 1
print(thread.to_number(-1))  # Prints 1
thread.pop(1)

assert(thread.resume(0) == LUA_OK)  # Returns 2
print(thread.to_number(-1))  # Prints 2
```

**Important thread lifecycle rules:**

1. **Shared globals, separate stacks**: Threads share the global environment but
   have independent execution stacks
2. **Reference counting**: Threads keep the parent state alive via reference
   counting, preventing premature GC
3. **Explicit close**: Calling `parent.close()` explicitly invalidates ALL child
   threads, even if they're still referenced
4. **Don't close threads**: Calling `thread.close()` warns and invalidates the
   pointer but doesn't affect the parent
5. **Multiple wrappers OK**: Creating multiple LuaState wrappers for the same
   thread is safe - they each independently ref-count the parent

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
- Don't use `tail`, `grep`, etc. over `ctest` output. Just run the tests and see the output in full each time.
- Don't manually invoke Godot to run tests. Use `ctest` as the test runner.