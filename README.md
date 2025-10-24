# Luau GDExtension

Godot 4.5+ GDExtension to integrate the [Luau](https://luau.org/) (Lua
derivative) scripting language.

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
ctest --preset windows-x86_64
ctest --preset linux-x86_64
ctest --preset macos-arm64
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

## Demo Project

Run the demo project to see Luau integration in action:

```bash
godot --editor --path demo/
```

The demo showcases:

- Compiling and executing Luau scripts
- Single-step debugging with breakpoints
- Math type usage (Vector2, Vector3, Color)
- Array and Dictionary conversions
- Sandboxing for safe script execution
