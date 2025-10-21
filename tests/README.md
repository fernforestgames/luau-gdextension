# Testing Documentation

This directory contains automated tests for the Godot-Luau bridging functionality.

## Overview

The test suite consists of two complementary approaches:

1. **C++ Unit Tests (doctest)** - Low-level tests of the bridging layer
2. **GDScript Integration Tests (GUT)** - High-level tests from Godot's perspective

## C++ Unit Tests (doctest)

### Structure

```
tests/
├── doctest.h              # doctest header (single-file framework)
├── test_main.cpp          # Main entry point with doctest runner
├── test_math_types.cpp    # Tests for Vector2, Vector3, Color, etc.
├── test_arrays.cpp        # Tests for Array ↔ Lua table bridging
├── test_dictionaries.cpp  # Tests for Dictionary ↔ Lua table bridging
├── test_variants.cpp      # Tests for generic Variant conversions
└── test_edge_cases.cpp    # Edge cases and error handling tests
```

### What's Tested

**Math Types:**
- Vector2, Vector2i, Vector3, Vector3i, Color
- Construction and round-trip conversion (Godot → Lua → Godot)
- Property access and modification
- All arithmetic operators (+, -, *, /, unary -)
- Equality comparison
- String conversion

**Array/Table Bridging:**
- Simple arrays: `[1, 2, 3]` ↔ Lua table `{1, 2, 3}`
- Mixed-type arrays
- Nested arrays
- Empty arrays
- Array vs Dictionary detection logic
- Large arrays (performance)

**Dictionary/Table Bridging:**
- String-keyed dictionaries
- Integer-keyed dictionaries
- Mixed key types
- Nested dictionaries
- Empty dictionaries
- Special string keys (with spaces, dots, etc.)

**Variant Conversions:**
- All primitive types (nil, bool, int, float, string)
- Math types as variants
- Collections as variants
- Nested structures
- Round-trip through Lua execution

**Edge Cases:**
- Division by zero
- NaN and infinity handling
- Nil values in structures
- Type mismatches
- Stack management
- Unicode and special strings
- Boundary values for integers
- Large collections

### Building and Running

**Build tests:**
```bash
cmake --preset default
cmake --build --preset default
```

**Run tests:**
```bash
# Windows
./bin/gdluau_tests.exe

# Linux/macOS
./bin/gdluau_tests
```

**Run with CTest:**
```bash
cd build
ctest
```

### Output

doctest provides detailed output for failures:
```
test_math_types.cpp:45: FAILED:
  CHECK( result.x == doctest::Approx(4.0) )
with expansion:
  CHECK( 3.9999 == Approx( 4.0 ) )
```

Successful runs show:
```
[doctest] test cases:  50 |  50 passed | 0 failed | 0 skipped
[doctest] assertions: 250 | 250 passed | 0 failed |
[doctest] Status: SUCCESS!
```

## GDScript Integration Tests (GUT)

### Structure

```
demo/
├── addons/gut/            # GUT framework (installed via script)
└── test/
    ├── test_math_types_integration.gd   # Math type tests from GDScript
    ├── test_collections_integration.gd  # Array/Dictionary tests
    └── test_variants_integration.gd     # Variant conversion tests
```

### What's Tested

The GUT tests provide integration testing from the GDScript/Godot Engine perspective:

**Math Types Integration:**
- All Vector and Color types
- Full round-trip from GDScript → Lua → GDScript
- Lua script execution that creates and manipulates math types
- Property access and modification through Lua

**Collections Integration:**
- Creating collections in GDScript, passing to Lua
- Creating collections in Lua, returning to GDScript
- Nested structures
- Table manipulation in Lua
- Array vs Dictionary detection

**Variant Integration:**
- All Variant types
- Complex nested structures
- Creating complex data in Lua scripts
- Type preservation through conversions

### Running GUT Tests

**Option 1: Godot Editor GUI**
1. Open the demo project in Godot
2. Click on "GUT" tab in the bottom panel
3. Click "Run All" to execute all tests
4. View results in the GUI

**Option 2: Command Line (Headless)**
```bash
# Run all tests in demo/test/ directory
godot --headless -s --path demo/ addons/gut/gut_cmdln.gd -gdir=res://test -gprefix=test_ -gexit

# Run specific test file
godot --headless -s --path demo/ addons/gut/gut_cmdln.gd -gtest=res://test/test_math_types_integration.gd -gexit

# Run with more verbose output
godot --headless -s --path demo/ addons/gut/gut_cmdln.gd -gdir=res://test -gprefix=test_ -glog=2 -gexit

# Export results to JUnit XML (for CI)
godot --headless -s --path demo/ addons/gut/gut_cmdln.gd -gdir=res://test -gprefix=test_ -gjunit_xml_file=test_results.xml -gexit
```

**Command Line Options:**
- `-gdir=<path>`: Directory to search for test files (default: `res://test`)
- `-gprefix=<string>`: Test file prefix to match (default: `test_`)
- `-gtest=<path>`: Run specific test file
- `-glog=<0-3>`: Log verbosity level (0=quiet, 3=verbose)
- `-gexit`: Exit after tests complete (required for CI)
- `-gjunit_xml_file=<path>`: Export results to JUnit XML format
- `-gdisable_colors`: Disable colored output (useful for CI logs)

**Option 3: From Demo Scene**
The GUT panel can also be added to any scene for interactive testing.

### Test Output

GUT provides detailed assertion output:
```
✓ test_vector2_round_trip - PASSED
✗ test_array_manipulation_in_lua - FAILED
  Expected: [10, 20, 30, 40]
  Actual:   [10, 20, 30]
  Line: 45

Tests: 48 | Passed: 47 | Failed: 1
```

## Test Coverage

### Currently Implemented (Tested)

- ✅ Vector2 (all features)
- ✅ Vector2i (all features)
- ✅ Vector3 (all features, native vector type)
- ✅ Vector3i (all features)
- ✅ Color (all features)
- ✅ Array ↔ Lua table conversion
- ✅ Dictionary ↔ Lua table conversion
- ✅ Variant system (all supported types)
- ✅ Nested structures (arrays in dicts, dicts in arrays, etc.)
- ✅ Type detection (isarray, isdictionary)
- ✅ Edge cases and error handling

### Not Yet Implemented (Stubs Only)

- ⏸ Vector4, Vector4i (stubs exist, not fully implemented)
- ⏸ Rect2, Rect2i, AABB, Plane
- ⏸ Quaternion, Basis
- ⏸ Transform2D, Transform3D, Projection

These will be tested once implemented.

## Continuous Integration

Tests run automatically on every push and pull request via GitHub Actions:

```yaml
- name: Run C++ tests
  run: ./bin/gdluau_tests
```

The CI runs tests on:
- Windows (x86_64, Debug & Release)
- Linux (x86_64, Debug & Release)
- macOS (arm64, Debug & Release)

All tests must pass for the build to succeed.

## Adding New Tests

### Adding C++ Tests

1. **Choose the appropriate file** based on what you're testing
   - Math types → `test_math_types.cpp`
   - Arrays → `test_arrays.cpp`
   - Dictionaries → `test_dictionaries.cpp`
   - Variants → `test_variants.cpp`
   - Edge cases → `test_edge_cases.cpp`

2. **Write a test case:**
   ```cpp
   TEST_CASE("Description of what this tests") {
       lua_State* L = create_test_state();

       // Test code here
       CHECK(condition);
       REQUIRE(critical_condition);

       close_test_state(L);
   }
   ```

3. **Use SUBCASEs for related tests:**
   ```cpp
   TEST_CASE("Vector2 operations") {
       SUBCASE("Addition") {
           // Test addition
       }

       SUBCASE("Subtraction") {
           // Test subtraction
       }
   }
   ```

### Adding GDScript Tests

1. **Choose the appropriate file** or create a new one following the naming convention `test_*_integration.gd`

2. **Extend GutTest:**
   ```gdscript
   extends GutTest

   var L: LuaState

   func before_each():
       L = LuaState.new()
       L.openlibs()

   func after_each():
       if L:
           L.close()
           L = null

   func test_my_feature():
       # Test code here
       assert_eq(actual, expected, "Error message")
       assert_true(condition)
       assert_typeof(value, TYPE_VECTOR2)
   ```

3. **Use descriptive test names** starting with `test_`

## Best Practices

1. **Test both directions:** Always test Godot → Lua AND Lua → Godot conversions
2. **Test edge cases:** Empty collections, nil values, boundary values
3. **Use appropriate assertions:**
   - `CHECK` for non-critical assertions (test continues)
   - `REQUIRE` for critical assertions (test stops)
   - `assert_almost_eq` for floating-point comparisons
4. **Clean up resources:** Close LuaState in `after_each()` or after test
5. **Keep tests focused:** Each test should verify one specific behavior
6. **Use meaningful names:** Test names should describe what they verify

## Troubleshooting

### C++ Tests Fail to Compile

- Ensure godot-cpp and Luau are properly fetched by CMake
- Check that `CMAKE_CXX_STANDARD` is set to 17
- Verify include paths in CMakeLists.txt

### C++ Tests Crash

- Check for memory leaks (unclosed lua_State)
- Verify stack is balanced (pushes match pops)
- Look for null pointer dereferences

### GUT Tests Not Found

- Ensure GUT is installed in `demo/addons/gut/`
- Enable the GUT plugin in Project Settings
- Restart Godot editor
- Verify test files are in `demo/test/` and start with `test_`

### GUT Tests Fail

- Check that the demo project has the built GDExtension library
- Rebuild the project if you changed C++ code
- Ensure Godot version is 4.5+
- Check test output for assertion details

## Documentation

For more information:
- doctest documentation: https://github.com/doctest/doctest
- GUT documentation: https://gut.readthedocs.io/
- Godot GDExtension docs: https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/
- Project architecture: See `CLAUDE.md` in project root
