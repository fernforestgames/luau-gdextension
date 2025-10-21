# Luau GDExtension

Godot 4.5+ GDExtension to integrate the [Luau](https://luau.org/) (Lua
derivative) scripting language.

## Building

Prerequisites:

- [CMake](https://cmake.org/) 3.28+
- [Godot](https://godotengine.org) 4.5+

```bash
cmake --preset default
cmake --build --preset default -j
```

## Testing

This project has comprehensive automated tests for all Godot-Luau bridging functionality.

**Quick Start:**
```bash
# Run all C++ tests (embedded in GDExtension, requires Godot runtime)
godot --headless --path demo/ -- --run-runtime-tests

# Run GDScript integration tests (in Godot editor)
# Open demo/ project → GUT tab → Run All

# Or run GDScript tests headlessly
godot --headless -s --path demo/ addons/gut/gut_cmdln.gd -gdir=res://test -gprefix=test_ -gexit
```

**📖 For detailed testing documentation, see [`tests/README.md`](tests/README.md)**

Test coverage includes:
- ✅ Math types (Vector2/3, Color, etc.)
- ✅ Array/Dictionary bridging
- ✅ Variant conversions
- ✅ Edge cases and error handling

**Note:** C++ tests are embedded in the Debug build of the GDExtension library and run inside the Godot runtime. This allows testing of both POD types (Vector2, Color) and runtime-dependent types (Array, Dictionary, Variant).

## Demo Project

Run the demo project to see Luau integration in action:

```bash
# Open in Godot Editor
godot --editor demo/project.godot

# Or run directly
godot demo/project.godot
```

The demo showcases:
- Compiling and executing Luau scripts
- Single-step debugging with breakpoints
- Math type usage (Vector2, Vector3, Color)
- Array and Dictionary conversions
- Sandboxing for safe script execution
