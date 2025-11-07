# Luau GDExtension

Godot 4.5+ GDExtension to integrate the [Luau](https://luau.org/) scripting
language (a high-performance Lua derivative) into Godot Engine.

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
var L = LuaState.new()
L.open_libs()

# Compile and execute Luau code
var bytecode = Luau.compile("print('Hello from Luau!')")
L.load_bytecode(bytecode, "hello")
L.resume()
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
var player_data = {"name": "Player", "health": 100.0, "level": 5.0}
L.push_variant(player_data)
L.set_global("player")

# Execute Luau code that modifies the data
L.do_string("""
    player.health = player.health - 10
    player.level = player.level + 1
""")

# Retrieve modified data back to Godot
L.get_global("player")
var updated_data = L.to_dictionary(-1)
print(updated_data)  # {"name": "Player", "health": 90.0, "level": 6.0}
```

### Callable Bridging

```gdscript
# Pass Godot Callables to Luau
L.push_variant(func(x): return x * 2)
L.set_global("double")

L.do_string("print(double(21))")  # Prints: 42

# Get Lua functions as Godot Callables
L.do_string("function add(a, b) return a + b end")
L.get_global("add")
var lua_add = L.to_variant(-1)  # Returns a Callable
print(lua_add.call(10, 32))  # Prints: 42
```

### Coroutines

```gdscript
# Create a Lua thread (coroutine)
L.do_string("""
    function counter()
        for i = 1, 3 do
            coroutine.yield(i)
        end
    end
""")

var thread = L.new_thread()
thread.get_global("counter")
thread.resume()

# Resume the coroutine multiple times
while thread.resume() == Luau.LUA_YIELD:
    print(thread.to_number(-1))  # Prints: 1, 2, 3
    thread.pop(1)
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
