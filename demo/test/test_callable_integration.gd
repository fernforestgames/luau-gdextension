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
	var bytecode: PackedByteArray = Luau.compile(code, null)
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
	var bytecode: PackedByteArray = Luau.compile(code, null)
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
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	L.get_global("multi")
	var lua_func: Callable = L.to_variant(-1)
	L.pop(1)

	var result: Variant = lua_func.call()
	# Note: Multiple return values are automatically adjusted to 1, no warning
	assert_eq(result, 1, "Should return first value when function returns multiple")
	assert_stack_balanced()

func test_lua_function_with_godot_types() -> void:
	var code: String = """
	function double_vector(v)
		return v * 2
	end
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
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
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	L.get_global("error_func")
	var lua_func: Callable = L.to_variant(-1)
	L.pop(1)

	var result: Variant = lua_func.call()
	assert_engine_error("error during call to error_func")
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
	var bytecode: PackedByteArray = Luau.compile(code, null)
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
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result: String = L.to_string_inplace(-1)
	assert_eq(result, "Hello, Lua", "Callable should return concatenated string to Lua")
	L.pop(1)
	assert_stack_balanced()

# DISABLED: lua_godotlib not yet refactored
func skip_test_godot_callable_with_vector_args() -> void:
	var callable: Callable = func(v: Vector2): return v.length()

	L.push_variant(callable)
	L.set_global("get_length")

	var code: String = """
	v = Vector2(3, 4)
	return get_length(v)
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result: float = L.to_number(-1)
	assert_almost_eq(result, 5.0, 0.001, "Callable should accept Vector2 from Lua and compute correct length")
	L.pop(1)
	assert_stack_balanced()

# DISABLED: lua_godotlib not yet refactored
func skip_test_godot_callable_returning_vector() -> void:
	var callable: Callable = func(x, y): return Vector2(x, y)

	L.push_variant(callable)
	L.set_global("make_vector")

	var code: String = "return make_vector(7, 9)"
	var bytecode: PackedByteArray = Luau.compile(code, null)
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
	var bytecode: PackedByteArray = Luau.compile(code, null)
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
	var test_bytecode: PackedByteArray = Luau.compile(test_code, null)
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

	var bytecode: PackedByteArray = Luau.compile(code, null)
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
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	var status: int = L.resume()

	# Should error due to argument count mismatch
	assert_ne(status, Luau.LUA_OK, "Should error when calling callable with wrong argument count")

func test_callable_with_no_arguments() -> void:
	var callable: Callable = func(): return 42

	L.push_variant(callable)
	L.set_global("get_answer")

	var code: String = "return get_answer()"
	var bytecode: PackedByteArray = Luau.compile(code, null)
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
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	assert_true(L.is_nil(-1), "Callable returning null should push nil value to Lua")
	L.pop(1)
	assert_stack_balanced()

# ============================================================================
# Stack Stress Tests for Callables
# ============================================================================

func test_callable_heavy_loop_stress() -> void:
	var counter: Array = [0] # Use array to make it mutable in lambda
	var callable: Callable = func():
		counter[0] += 1
		return counter[0]

	L.push_variant(callable)
	L.set_global("increment")

	var code: String = """
	local sum = 0
	for i = 1, 1000 do
		sum = sum + increment()
	end
	return sum
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result: int = L.to_integer(-1)
	assert_eq(result, 500500, "Should correctly accumulate 1000 calls: 1+2+...+1000 = 500500")
	assert_eq(counter[0], 1000, "Counter should have been incremented 1000 times")
	L.pop(1)
	assert_stack_balanced()

func test_callable_nested_calls_stress() -> void:
	var add: Callable = func(a, b): return a + b
	var multiply: Callable = func(a, b): return a * b

	L.push_variant(add)
	L.set_global("add")
	L.push_variant(multiply)
	L.set_global("mul")

	var code: String = """
	local result = 0
	for i = 1, 100 do
		-- Nested: result = result + (i * 2)
		result = add(result, mul(i, 2))
	end
	return result
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result: int = L.to_integer(-1)
	assert_eq(result, 10100, "Nested callable calls: 2+4+6+...+200 = 10100")
	L.pop(1)
	assert_stack_balanced()

func test_callable_with_complex_returns_in_loop() -> void:
	var create_vector: Callable = func(x, y): return Vector2(x, y)

	L.push_variant(create_vector)
	L.set_global("vec")

	var code: String = """
	local vectors = {}
	for i = 1, 100 do
		local v = vec(i, i * 2)
		table.insert(vectors, v)
	end
	return #vectors
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var count: int = L.to_integer(-1)
	assert_eq(count, 100, "Should create 100 Vector2 objects via callable")
	L.pop(1)
	assert_stack_balanced()

func test_callable_discarded_results_stress() -> void:
	var side_effect_counter: Array = [0] # Use array to make it mutable in lambda
	var callable: Callable = func():
		side_effect_counter[0] += 1
		return 999 # Return value is discarded

	L.push_variant(callable)
	L.set_global("do_work")

	var code: String = """
	-- Call 500 times without capturing result
	for i = 1, 500 do
		do_work()  -- Result discarded
	end
	return true
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	assert_eq(side_effect_counter[0], 500, "Should have called callable 500 times despite discarded results")
	L.pop(1)
	assert_stack_balanced()

func test_callable_alternating_stress() -> void:
	var get_int: Callable = func(): return 42
	var get_string: Callable = func(): return "test"
	var get_vector: Callable = func(): return Vector2(1, 2)

	L.push_variant(get_int)
	L.set_global("get_int")
	L.push_variant(get_string)
	L.set_global("get_str")
	L.push_variant(get_vector)
	L.set_global("get_vec")

	var code: String = """
	for i = 1, 200 do
		local a = get_int()
		local b = get_str()
		local c = get_vec()
		-- Just ensure they're called, don't accumulate
	end
	return true
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	assert_true(L.to_boolean(-1), "Should complete 200 iterations alternating between different callable types")
	L.pop(1)
	assert_stack_balanced()

func test_callable_recursive_stress() -> void:
	var adder: Callable = func(a, b): return a + b

	L.push_variant(adder)
	L.set_global("add")

	var code: String = """
	function recursive_sum(n)
		if n <= 0 then return 0 end
		return add(n, recursive_sum(n - 1))
	end
	return recursive_sum(50)
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result: int = L.to_integer(-1)
	assert_eq(result, 1275, "Recursive callable invocation: 1+2+...+50 = 1275")
	L.pop(1)
	assert_stack_balanced()

func test_callable_in_table_operations_stress() -> void:
	var process: Callable = func(x): return x * 2

	L.push_variant(process)
	L.set_global("process")

	var code: String = """
	local results = {}
	for i = 1, 100 do
		results[i] = process(i)
	end
	-- Verify all results
	local sum = 0
	for i = 1, 100 do
		sum = sum + results[i]
	end
	return sum
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result: int = L.to_integer(-1)
	# Sum of 2+4+6+...+200 = 2*(1+2+3+...+100) = 2*5050 = 10100
	assert_eq(result, 10100, "Callable results stored in table should maintain integrity")
	L.pop(1)
	assert_stack_balanced()

# ============================================================================
# Varargs Tests
# ============================================================================

func test_godot_callable_with_varargs_from_lua() -> void:
	# Create a Callable to a GDScript method with optional parameters
	# This simulates varargs behavior by accepting 0-5 string arguments
	L.push_callable(self._concat_varargs)
	L.set_global("concat")

	# Test with 0 arguments
	L.do_string('return concat()', "test0")
	assert_eq(L.to_string_inplace(-1), "", "Varargs callable with 0 args should return empty string")
	L.pop(1)

	# Test with 1 argument
	L.do_string('return concat("A")', "test1")
	assert_eq(L.to_string_inplace(-1), "A", "Varargs callable with 1 arg should return single value")
	L.pop(1)

	# Test with 3 arguments
	L.do_string('return concat("Hello", " ", "World")', "test3")
	assert_eq(L.to_string_inplace(-1), "Hello World", "Varargs callable with 3 args should concatenate all")
	L.pop(1)

	# Test with 5 arguments
	L.do_string('return concat("A", "B", "C", "D", "E")', "test5")
	assert_eq(L.to_string_inplace(-1), "ABCDE", "Varargs callable with 5 args should concatenate all")
	L.pop(1)

	assert_stack_balanced()

# Helper function that accepts varargs using Godot's ... syntax
# When called from C++ via Callable, Godot passes all arguments into the Array
func _concat_varargs(...values: Array) -> String:
	var result: String = ""
	for val in values:
		result += val
	return result
