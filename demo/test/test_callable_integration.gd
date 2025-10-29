extends GutTest
# Integration tests for Callable bridging between Godot and Luau
# Tests bidirectional conversion of functions and callables
#
# NOTE: Some tests intentionally trigger errors/warnings to verify error handling.
# These are expected and part of the test suite:
# - test_lua_function_multiple_return_values: Expects warning about multiple returns
# - test_lua_function_error_handling: Expects error from Lua error() function

var L: LuaState

func before_each() -> void:
	L = LuaState.new()
	L.open_libs() # Load all libraries including Godot types

func after_each() -> void:
	if L:
		L.close()
		L = null


# Helper to verify Lua stack is balanced
func assert_stack_balanced(expected_top: int = 0) -> void:
	assert_eq(L.get_top(), expected_top, "Lua stack should be balanced at %d, but is %d" % [expected_top, L.get_top()])

# ============================================================================
# Lua Function → Godot Callable Tests
# ============================================================================

func test_lua_function_to_callable_conversion() -> void:
	var code: String = "function add(a, b) return a + b end"
	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	# Get the function as a Callable
	L.get_global("add")
	var lua_func: Variant = L.to_variant(-1)
	L.pop(1)

	assert_typeof(lua_func, TYPE_CALLABLE, "Lua function should convert to Callable")
	assert_stack_balanced()

func test_lua_function_callable_invocation() -> void:
	var code: String = "function multiply(a, b) return a * b end"
	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	L.get_global("multiply")
	var lua_func: Callable = L.to_variant(-1)
	L.pop(1)

	var result: Variant = lua_func.call(5, 7)
	assert_eq(result, 35, "Lua function should be callable from Godot and return correct result")
	assert_stack_balanced()

func test_lua_function_with_string_args() -> void:
	var code: String = """
	function greet(name)
		return "Hello, " .. name .. "!"
	end
	"""
	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	L.get_global("greet")
	var lua_func: Callable = L.to_variant(-1)
	L.pop(1)

	var result: String = lua_func.call("World")
	assert_eq(result, "Hello, World!", "Lua function should handle string arguments and concatenation")
	assert_stack_balanced()

func test_lua_function_multiple_return_values() -> void:
	var code: String = """
	function multi()
		return 1, 2, 3
	end
	"""
	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	L.get_global("multi")
	var lua_func: Callable = L.to_variant(-1)
	L.pop(1)

	var result: Variant = lua_func.call()
	assert_engine_error("LuaCallable: Lua function returned 3 values, returning only the first.")
	assert_eq(result, 1, "Should return first value when function returns multiple")
	assert_stack_balanced()

func test_lua_function_with_godot_types() -> void:
	var code: String = """
	function double_vector(v)
		return v * 2
	end
	"""
	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	L.get_global("double_vector")
	var lua_func: Callable = L.to_variant(-1)
	L.pop(1)

	var input: Vector2 = Vector2(3.0, 4.0)
	var result: Vector2 = lua_func.call(input)

	assert_typeof(result, TYPE_VECTOR2, "Lua function should return Vector2 type")
	assert_almost_eq(result.x, 6.0, 0.001, "Vector2.x should be doubled")
	assert_almost_eq(result.y, 8.0, 0.001, "Vector2.y should be doubled")
	assert_stack_balanced()

func test_lua_function_error_handling() -> void:
	var code: String = """
	function error_func()
		error("Test error")
	end
	"""
	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	L.get_global("error_func")
	var lua_func: Callable = L.to_variant(-1)
	L.pop(1)

	var result: Variant = lua_func.call()
	assert_engine_error("LuaCallable: Unhandled error during call to Lua function")
	assert_null(result, "Erroring Lua function should return null on error")
	assert_stack_balanced()

# ============================================================================
# Godot Callable → Lua Function Tests
# ============================================================================

func test_godot_callable_to_lua_conversion() -> void:
	var callable: Callable = func(x): return x * 2

	L.push_variant(callable)
	L.set_global("godot_func")

	L.get_global("godot_func")
	assert_true(L.is_userdata(-1), "Callable should be pushed as userdata to Lua")
	L.pop(1)
	assert_stack_balanced()

func test_godot_callable_invocation_from_lua() -> void:
	var callable: Callable = func(a, b): return a + b

	L.push_variant(callable)
	L.set_global("add")

	var code: String = "return add(10, 20)"
	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result: float = L.to_number(-1)
	assert_eq(result, 30, "Callable should be invokable from Lua and return correct sum")
	L.pop(1)
	assert_stack_balanced()

func test_godot_callable_with_string_result() -> void:
	var callable: Callable = func(name): return "Hello, " + name

	L.push_variant(callable)
	L.set_global("greet")

	var code: String = 'return greet("Lua")'
	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result: String = L.to_string(-1)
	assert_eq(result, "Hello, Lua", "Callable should return concatenated string to Lua")
	L.pop(1)
	assert_stack_balanced()

func test_godot_callable_with_vector_args() -> void:
	var callable: Callable = func(v: Vector2): return v.length()

	L.push_variant(callable)
	L.set_global("get_length")

	var code: String = """
	v = Vector2(3, 4)
	return get_length(v)
	"""
	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result: float = L.to_number(-1)
	assert_almost_eq(result, 5.0, 0.001, "Callable should accept Vector2 from Lua and compute correct length")
	L.pop(1)
	assert_stack_balanced()

func test_godot_callable_returning_vector() -> void:
	var callable: Callable = func(x, y): return Vector2(x, y)

	L.push_variant(callable)
	L.set_global("make_vector")

	var code: String = "return make_vector(7, 9)"
	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result: Vector2 = L.to_variant(-1)
	assert_typeof(result, TYPE_VECTOR2, "Callable should return Vector2 type to Lua")
	assert_almost_eq(result.x, 7.0, 0.001, "Vector2.x should match passed argument")
	assert_almost_eq(result.y, 9.0, 0.001, "Vector2.y should match passed argument")
	L.pop(1)
	assert_stack_balanced()

# ============================================================================
# Round-Trip Tests (Lua → Godot → Lua)
# ============================================================================

func test_round_trip_lua_to_godot_to_lua() -> void:
	var code: String = "function double(x) return x * 2 end"
	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	# Get Lua function as Callable
	L.get_global("double")
	var callable: Callable = L.to_variant(-1)
	L.pop(1)

	# Push it back to Lua
	L.push_variant(callable)
	L.set_global("godot_double")

	# Call it from Lua
	var test_code: String = "return godot_double(21)"
	var test_bytecode: PackedByteArray = Luau.compile(test_code)
	L.load_bytecode(test_bytecode, "test2")
	L.resume()

	var result: float = L.to_number(-1)
	assert_eq(result, 42, "Lua function should work correctly after round-trip through Godot Callable")
	L.pop(1)
	assert_stack_balanced()

func test_nested_callable_calls() -> void:
	# Godot function that takes a callback
	var apply_twice: Callable = func(callback, value):
		return callback.call(callback.call(value))

	L.push_variant(apply_twice)
	L.set_global("apply_twice")

	var code: String = """
	function increment(x)
		return x + 1
	end

	result = apply_twice(increment, 10)
	return result
	"""

	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result: float = L.to_number(-1)
	assert_eq(result, 12, "Should apply Lua function twice through Godot callable: 10 + 1 + 1 = 12")
	L.pop(1)
	assert_stack_balanced()

# ============================================================================
# Edge Cases and Error Handling
# ============================================================================

func test_callable_argument_count_validation() -> void:
	# Callable with fixed argument count
	var callable: Callable = func(a, b): return a + b

	L.push_variant(callable)
	L.set_global("add")

	# Try calling with wrong number of arguments
	var code: String = "return add(1)" # Missing one argument
	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	var status: int = L.resume()

	# Should error due to argument count mismatch
	assert_ne(status, Luau.LUA_OK, "Should error when calling callable with wrong argument count")

func test_callable_with_no_arguments() -> void:
	var callable: Callable = func(): return 42

	L.push_variant(callable)
	L.set_global("get_answer")

	var code: String = "return get_answer()"
	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result: float = L.to_number(-1)
	assert_eq(result, 42, "Callable with no arguments should execute and return correct value")
	L.pop(1)
	assert_stack_balanced()

func test_callable_with_nil_result() -> void:
	var callable: Callable = func(): return null

	L.push_variant(callable)
	L.set_global("get_nil")

	var code: String = "return get_nil()"
	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	assert_true(L.is_nil(-1), "Callable returning null should push nil value to Lua")
	L.pop(1)
	assert_stack_balanced()
