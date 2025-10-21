# Testing Documentation

This directory contains automated tests for the Godot-Luau bridging functionality.

## Overview

The test suite consists of two complementary approaches:

1. **Runtime-Embedded C++ Tests (doctest)** - All bridging layer tests running inside Godot runtime
2. **GDScript Integration Tests (GUT)** - High-level tests from Godot's perspective

## Runtime-Embedded C++ Tests (doctest)

### Architecture

The C++ tests are embedded directly into the GDExtension library (Debug builds only):

```
src/
├── runtime_tests.h        # RuntimeTests singleton class definition
├── runtime_tests.cpp      # Test runner implementation using doctest
└── tests_runtime.cpp      # All test cases (POD types + runtime types)

tests/
└── doctest.h              # doctest header (single-file framework)
```

Tests are compiled into the library with `ENABLE_RUNTIME_TESTS` defined (Debug only) and exposed to GDScript via the `RuntimeTests` class.

### Why Runtime-Embedded Tests?

**Key Insight:** Godot types like `Array`, `Dictionary`, and `Variant` require the full Godot runtime to be initialized. POD types like `Vector2` and `Color` can work without it, but for consistency and simplicity, **all C++ tests run inside the Godot runtime**.

This approach:
- ✅ Allows testing both POD and runtime-dependent types in one place
- ✅ Simpler to maintain (one test framework, one way to run tests)
- ✅ More realistic testing environment (how the code actually runs)
- ✅ No need for separate test executables or complex build configurations

### What's Tested

**POD Math Types:**
- Vector2/Vector2i: Construction, property access, arithmetic operators, equality, tostring
- Vector3/Vector3i: Native vector type operations, property access
- Color: RGBA operations, arithmetic, property access
- Type checking: Verify correct type identification

**Runtime-Dependent Types:**
- **Array Bridging:** Simple arrays, mixed types, nested arrays, empty arrays, round-trip conversions
- **Dictionary Bridging:** String/integer keys, mixed keys, nested dictionaries, round-trip conversions
- **Variant Conversions:** All supported Variant types, nested structures, type preservation
- **Array vs Dictionary Detection:** Proper detection based on Lua table structure

### Building and Running

**Build with tests (Debug only):**
```bash
cmake --preset default
cmake --build --preset default -j
```

**Run tests:**
```bash
# The test runner is integrated into the demo project
godot --headless --path demo/ -- --run-runtime-tests
```

**How it works:**
1. `demo/test_runner.gd` is the main scene
2. It checks for `--run-runtime-tests` command-line flag
3. If present: calls `RuntimeTests.run()` and exits with pass/fail status
4. If not present: loads normal demo scene (`main.tscn`)

### Output

doctest provides detailed output for failures:
```
tests_runtime.cpp:45: FAILED:
  CHECK( result.x == doctest::Approx(4.0) )
with expansion:
  CHECK( 3.9999 == Approx( 4.0 ) )
```

Successful runs show:
```
=== Running Runtime-Embedded C++ Tests ===

[doctest] test cases:  15 |  15 passed | 0 failed | 0 skipped
[doctest] assertions: 120 | 120 passed | 0 failed |

=== Test Results ===
Test cases: 15 passed, 0 failed
Assertions: 120 passed, 0 failed

✓ All tests passed!
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
  run: godot --headless --path demo/ -- --run-runtime-tests

- name: Run GDScript tests
  run: godot --headless -s --path demo/ addons/gut/gut_cmdln.gd -gdir=res://test -gexit
```

The CI runs tests on:
- Windows (x86_64, Debug & Release)
- Linux (x86_64, Debug & Release)
- macOS (arm64, Debug & Release)

All tests must pass for the build to succeed.

## Adding New Tests

### Adding C++ Tests

1. **Edit `src/tests_runtime.cpp`** to add new test cases

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

4. **For runtime types, use the LuaState wrapper:**
   ```cpp
   TEST_CASE("Array round-trip") {
       LuaState* wrapper = create_wrapper_state();

       Array arr;
       arr.push_back(1);
       arr.push_back(2);

       wrapper->pusharray(arr);
       wrapper->setglobal("test");

       wrapper->getglobal("test");
       Array result = wrapper->toarray(-1);

       CHECK(result.size() == 2);

       close_wrapper_state(wrapper);
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
       L.openlibs(LuaState.LIB_ALL)

   func after_each():
       if L:
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
   - `doctest::Approx()` for floating-point comparisons
4. **Clean up resources:** Close LuaState after tests
5. **Keep tests focused:** Each test should verify one specific behavior
6. **Use meaningful names:** Test names should describe what they verify

## Troubleshooting

### RuntimeTests Class Not Found

- Ensure you built in Debug mode: `cmake --preset default`
- Runtime tests are only available in Debug builds
- Check that `ENABLE_RUNTIME_TESTS` is defined during compilation

### C++ Tests Crash

- Check for memory leaks (unclosed lua_State)
- Verify stack is balanced (pushes match pops)
- Look for null pointer dereferences
- Ensure Godot runtime types are created after runtime initialization

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
