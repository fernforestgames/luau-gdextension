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
# Run C++ unit tests
./bin/gdluau_tests          # Linux/macOS
./bin/gdluau_tests.exe      # Windows

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
