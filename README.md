# Luau GDExtension

Godot 4.5+ GDExtension to integrate the [Luau](https://luau.org/) scripting
language (a high-performance Lua derivative) into Godot Engine.

**📖 Documentation:** Full API reference is available in Godot's built-in help system (Godot 4.3+). Search for `LuaState`, `Luau`, or `LuauScript` in the editor, or browse the XML files in [`doc_classes/`](doc_classes/).

## Why Luau?

[Luau](https://luau.org/) is a high-performance scripting language derived from
Lua 5.1, originally created by Roblox. It offers several advantages for
integration into games:

- **Performance**: Up to 2x faster than standard Lua 5.1 with optimized bytecode
- **Type Checking**: Optional gradual typing catches errors before runtime
- **Safety**: Built-in sandboxing capabilities for running untrusted code
- **Modern Features**: Native vector type, improved string library, buffer API

## Quick Start

### Basic Usage

```gdscript
# Create a Lua state and load libraries
var state := LuaState.new()
state.open_libs()

# Compile and execute Luau code
var bytecode := Luau.compile("print('Hello from Luau!')")
if state.load_bytecode(bytecode, "hello"):
    state.pcall(0, 0)  # Execute with 0 arguments, 0 return values
```

### Math Type Integration

Godot math types work seamlessly in Luau with operator support:

```lua
-- Vector3 uses Luau's native vector type for maximum performance
local pos = Vector3(10, 20, 30)
local dir = Vector3(1, 0, 0)
local new_pos = pos + dir * 5.0  -- Inline VM operations, JIT-friendly

-- Built-in vector library functions
local length = vector.magnitude(pos)
local normalized = vector.normalize(dir)

-- Other math types (Vector2, Vector4, Color, etc.)
local color = Color(1.0, 0.5, 0.0, 1.0)
local tinted = color * 0.8
```

### Data Exchange

```gdscript
# Pass Godot data to Luau
var player_data := {"name": "Player", "health": 100.0, "level": 5.0}
var state := LuaState.new()
state.open_libs()
state.push_variant(player_data)
state.set_global("player")

# Execute Luau code that modifies the data
state.do_string("""
    player.health = player.health - 10
    player.level = player.level + 1
""", "modify_player")

# Retrieve modified data back to Godot
state.get_global("player")
var updated_data := state.to_dictionary(-1)
print(updated_data)  # {"name": "Player", "health": 90.0, "level": 6.0}
```

### Callable Bridging

```gdscript
# Pass Godot Callables to Luau
var state := LuaState.new()
state.open_libs()
state.push_variant(func(x): return x * 2)
state.set_global("double")

state.do_string("print(double(21))", "test_callable")  # Prints: 42

# Get Lua functions as Godot Callables
state.do_string("function add(a, b) return a + b end", "define_add")
state.get_global("add")
var lua_add := state.to_callable(-1)  # Returns a Callable
print(lua_add.call(10, 32))  # Prints: 42
```

### Sandboxing

Sandboxing protects builtin libraries and enables runtime optimizations:

```gdscript
var state := LuaState.new()
state.open_libs()

# Set up any globals you want to expose
state.push_variant({"max_health": 100})
state.set_global("config")

# Load untrusted code
var untrusted_code := """
    -- This code cannot modify builtin libraries
    -- string.byte = nil  -- This would fail
    print("Player max health: " .. config.max_health)
"""
var bytecode := Luau.compile(untrusted_code)
state.load_bytecode(bytecode, "untrusted")

# Apply sandbox AFTER loading code, BEFORE executing
state.sandbox()

# Now safe to execute
state.pcall(0, 0)
```

For threads, use `sandbox_thread()` after the main state is sandboxed:

```gdscript
# Main state already sandboxed
var thread := state.new_thread()
state.pop(1)

# Load code into thread
thread.load_bytecode(bytecode, "thread_func")

# Sandbox the thread before execution
thread.sandbox_thread()

# Now safe to execute
thread.pcall(0, 0)
```

### Coroutines

```gdscript
# Create a Lua thread (coroutine)
var state := LuaState.new()
state.open_libs()
state.do_string("""
    function counter()
        for i = 1, 3 do
            coroutine.yield(i)
        end
    end
""", "define_counter")

var thread := state.new_thread()
state.pop(1)  # Clean up the thread from main stack
thread.get_global("counter")
var status := thread.resume(0)  # Call the function with 0 arguments

# Resume the coroutine multiple times
while status == Luau.LUA_YIELD:
    print(thread.to_number(-1))  # Prints: 1, 2, 3
    thread.pop(1)
    status = thread.resume(0)
```

## Building

Prerequisites:

- [CMake](https://cmake.org/) 3.28+
- [Godot](https://godotengine.org) 4.5+

Build the preset appropriate for your platform—for example:

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

## Testing

This project has comprehensive automated tests for all Godot-Luau bridging
functionality:

```bash
ctest --preset windows-x86_64-debug
ctest --preset linux-x86_64-debug
ctest --preset macos-arm64-debug
```

**📖 For detailed testing documentation, see
[`tests/README.md`](tests/README.md)**

Test coverage includes:

- ✅ Math types (Vector2/3, Color, etc.)
- ✅ Array/Dictionary bridging
- ✅ Variant conversions
- ✅ Edge cases and error handling

**Note:** C++ tests are embedded in the Debug build of the GDExtension library
and run inside the Godot runtime. This allows testing of both POD types
(Vector2, Color) and runtime-dependent types (Array, Dictionary, Variant).

## Supported Types

### Math Types

All Godot math types with full operator support:

- **Vector2, Vector2i** - 2D vectors (integer and float)
- **Vector3, Vector3i** - 3D vectors with native Luau vector optimization
- **Vector4, Vector4i** - 4D vectors
- **Color** - RGBA colors with arithmetic operations
- **Rect2, Rect2i** - 2D rectangles
- **Plane, AABB** - Geometric primitives
- **Quaternion, Basis, Transform2D, Transform3D, Projection** - Transforms

### Data Types

- **Array** - Godot Arrays (bidirectional conversion to/from Lua tables)
- **Dictionary** - Godot Dictionaries (bidirectional conversion to/from Lua tables)
- **String** - Transparent string bridging
- **Callable** - First-class functions crossing language boundaries
- **Variant** - Generic type that handles all Godot types

## Demo Project

Run the demo project to see Luau integration in action:

```bash
godot --editor --path demo/
```

The demo showcases:

- Compiling and executing Luau scripts
- Single-step debugging with breakpoints and step signals
- Math type usage with operators (Vector2, Vector3, Color, etc.)
- Array and Dictionary conversions (including nested structures)
- Sandboxing for safe script execution
- Vector library functions (magnitude, normalize, dot, cross, etc.)

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
