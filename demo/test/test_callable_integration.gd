extends GutTest
# Integration tests for Callable bridging between Godot and Luau
# Tests bidirectional conversion of functions and callables

var L: LuaState

func before_each():
	L = LuaState.new()
	L.openlibs()  # Load all libraries including Godot types

func after_each():
	if L:
		L.close()
		L = null

# ============================================================================
# Lua Function → Godot Callable Tests
# ============================================================================

func test_lua_function_to_callable_conversion():
	var code = "function add(a, b) return a + b end"
	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	# Get the function as a Callable
	L.getglobal("add")
	var lua_func = L.tovariant(-1)
	L.pop(1)

	assert_typeof(lua_func, TYPE_CALLABLE, "Lua function should convert to Callable")

func test_lua_function_callable_invocation():
	var code = "function multiply(a, b) return a * b end"
	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	L.getglobal("multiply")
	var lua_func: Callable = L.tovariant(-1)
	L.pop(1)

	var result = lua_func.call(5, 7)
	assert_eq(result, 35, "Lua function should be callable from Godot and return correct result")

func test_lua_function_with_string_args():
	var code = """
	function greet(name)
		return "Hello, " .. name .. "!"
	end
	"""
	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	L.getglobal("greet")
	var lua_func: Callable = L.tovariant(-1)
	L.pop(1)

	var result = lua_func.call("World")
	assert_eq(result, "Hello, World!", "Lua function should handle string arguments")

func test_lua_function_multiple_return_values():
	var code = """
	function multi()
		return 1, 2, 3
	end
	"""
	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	L.getglobal("multi")
	var lua_func: Callable = L.tovariant(-1)
	L.pop(1)

	# Should return first value and warn about multiple returns
	# Ignore the expected warning
	ignore_errors = true
	var result = lua_func.call()
	ignore_errors = false
	assert_eq(result, 1, "Should return first value when function returns multiple")

func test_lua_function_with_godot_types():
	var code = """
	function double_vector(v)
		return v * 2
	end
	"""
	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	L.getglobal("double_vector")
	var lua_func: Callable = L.tovariant(-1)
	L.pop(1)

	var input = Vector2(3.0, 4.0)
	var result = lua_func.call(input)

	assert_typeof(result, TYPE_VECTOR2)
	assert_almost_eq(result.x, 6.0, 0.001)
	assert_almost_eq(result.y, 8.0, 0.001)

func test_lua_function_error_handling():
	var code = """
	function error_func()
		error("Test error")
	end
	"""
	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	L.getglobal("error_func")
	var lua_func: Callable = L.tovariant(-1)
	L.pop(1)

	# Should print error and return nil
	# Ignore the expected error
	ignore_errors = true
	var result = lua_func.call()
	ignore_errors = false
	assert_null(result, "Erroring Lua function should return null")

# ============================================================================
# Godot Callable → Lua Function Tests
# ============================================================================

func test_godot_callable_to_lua_conversion():
	var callable = func(x): return x * 2

	L.pushvariant(callable)
	L.setglobal("godot_func")

	L.getglobal("godot_func")
	assert_true(L.isuserdata(-1), "Callable should be pushed as userdata")
	L.pop(1)

func test_godot_callable_invocation_from_lua():
	var callable = func(a, b): return a + b

	L.pushvariant(callable)
	L.setglobal("add")

	var code = "return add(10, 20)"
	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result = L.tonumber(-1)
	assert_eq(result, 30, "Callable should be invokable from Lua")

func test_godot_callable_with_string_result():
	var callable = func(name): return "Hello, " + name

	L.pushvariant(callable)
	L.setglobal("greet")

	var code = 'return greet("Lua")'
	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result = L.tostring(-1)
	assert_eq(result, "Hello, Lua", "Callable should return string to Lua")

func test_godot_callable_with_vector_args():
	var callable = func(v: Vector2): return v.length()

	L.pushvariant(callable)
	L.setglobal("get_length")

	var code = """
	v = Vector2(3, 4)
	return get_length(v)
	"""
	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result = L.tonumber(-1)
	assert_almost_eq(result, 5.0, 0.001, "Callable should accept Vector2 from Lua")

func test_godot_callable_returning_vector():
	var callable = func(x, y): return Vector2(x, y)

	L.pushvariant(callable)
	L.setglobal("make_vector")

	var code = "return make_vector(7, 9)"
	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result = L.tovariant(-1)
	assert_typeof(result, TYPE_VECTOR2)
	assert_almost_eq(result.x, 7.0, 0.001)
	assert_almost_eq(result.y, 9.0, 0.001)

# ============================================================================
# Round-Trip Tests (Lua → Godot → Lua)
# ============================================================================

func test_round_trip_lua_to_godot_to_lua():
	var code = "function double(x) return x * 2 end"
	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	# Get Lua function as Callable
	L.getglobal("double")
	var callable: Callable = L.tovariant(-1)
	L.pop(1)

	# Push it back to Lua
	L.pushvariant(callable)
	L.setglobal("godot_double")

	# Call it from Lua
	var test_code = "return godot_double(21)"
	var test_bytecode = Luau.compile(test_code)
	L.load_bytecode(test_bytecode, "test2")
	L.resume()

	var result = L.tonumber(-1)
	assert_eq(result, 42, "Lua function should work after round-trip through Godot")

func test_nested_callable_calls():
	# Godot function that takes a callback
	var apply_twice = func(callback, value):
		return callback.call(callback.call(value))

	L.pushvariant(apply_twice)
	L.setglobal("apply_twice")

	var code = """
	function increment(x)
		return x + 1
	end

	result = apply_twice(increment, 10)
	return result
	"""

	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result = L.tonumber(-1)
	assert_eq(result, 12, "Should apply Lua function twice: 10 + 1 + 1 = 12")

# ============================================================================
# Edge Cases and Error Handling
# ============================================================================

func test_callable_argument_count_validation():
	# Callable with fixed argument count
	var callable = func(a, b): return a + b

	L.pushvariant(callable)
	L.setglobal("add")

	# Try calling with wrong number of arguments
	var code = "return add(1)"  # Missing one argument
	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	var status = L.resume()

	# Should error due to argument count mismatch
	assert_ne(status, Luau.LUA_OK, "Should error with wrong argument count")

func test_callable_with_no_arguments():
	var callable = func(): return 42

	L.pushvariant(callable)
	L.setglobal("get_answer")

	var code = "return get_answer()"
	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result = L.tonumber(-1)
	assert_eq(result, 42, "Callable with no arguments should work")

func test_callable_with_nil_result():
	var callable = func(): return null

	L.pushvariant(callable)
	L.setglobal("get_nil")

	var code = "return get_nil()"
	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	assert_true(L.isnil(-1), "Callable returning null should push nil to Lua")
