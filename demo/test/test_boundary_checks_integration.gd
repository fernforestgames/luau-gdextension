extends GutTest
# Integration tests for boundary checking and crash prevention
# Tests that invalid operations print errors instead of crashing

func test_pop_from_empty_stack():
	var state = LuaState.new()

	# Empty stack - should not crash
	assert_eq(state.get_top(), 0, "Stack should be empty initially")

	# Try to pop from empty stack - should fail gracefully
	state.pop(1)
	assert_engine_error("Stack underflow")

	# Stack should still be empty
	assert_eq(state.get_top(), 0, "Stack should still be empty after invalid pop")

func test_pop_more_than_available():
	var state = LuaState.new()
	state.push_integer(1)
	state.push_integer(2)

	assert_eq(state.get_top(), 2, "Should have 2 items")

	# Try to pop 10 items when only 2 exist
	state.pop(10)
	assert_engine_error("Stack underflow")

	# Operation should be rejected, stack unchanged
	assert_eq(state.get_top(), 2, "Stack should be unchanged after invalid pop")

func test_pushvalue_with_invalid_index():
	var state = LuaState.new()
	state.push_integer(42)

	var top = state.get_top()

	# Try to push invalid index
	state.push_value(100)
	assert_engine_error("Invalid stack index")

	# Stack should be unchanged
	assert_eq(state.get_top(), top, "Stack should be unchanged after invalid pushvalue")

func test_pushvalue_with_zero_index():
	var state = LuaState.new()
	state.push_integer(1)

	var top = state.get_top()

	# Index 0 is never valid in Lua
	state.push_value(0)
	assert_engine_error("Invalid stack index")

	# Stack should be unchanged
	assert_eq(state.get_top(), top, "Stack should be unchanged with index 0")

func test_remove_invalid_index():
	var state = LuaState.new()
	state.push_integer(1)
	state.push_integer(2)

	var top = state.get_top()

	# Try to remove invalid index
	state.remove(10)
	assert_engine_error("Invalid stack index")

	# Stack should be unchanged
	assert_eq(state.get_top(), top, "Stack should be unchanged after invalid remove")

func test_replace_on_empty_stack():
	var state = LuaState.new()

	# Try to replace when there's nothing to replace with
	state.replace(1)
	assert_engine_error("has_n_items")

	# Should not crash, stack still empty
	assert_eq(state.get_top(), 0, "Stack should still be empty")

func test_insert_on_empty_stack():
	var state = LuaState.new()

	# Try to insert when there's nothing to insert
	state.insert(1)
	assert_engine_error("has_n_items")

	# Should not crash, stack still empty
	assert_eq(state.get_top(), 0, "Stack should still be empty")

func test_settop_with_invalid_negative():
	var state = LuaState.new()
	state.push_integer(1)
	state.push_integer(2)

	assert_eq(state.get_top(), 2, "Should have 2 items")

	# Try invalid negative index
	state.set_top(-10)
	assert_engine_error("index")

	# Stack should be unchanged
	assert_eq(state.get_top(), 2, "Stack should be unchanged after invalid settop")

func test_gettable_without_key():
	var state = LuaState.new()
	state.open_libs()
	state.do_string("t = {a = 1}", "test")
	state.get_global("t")
	state.pop(1) # Remove table, now stack is empty

	# Try gettable without a key
	state.get_table(1)
	assert_engine_error("top")

	# Should not crash
	assert_eq(state.get_top(), 0, "Stack should be empty")

func test_settable_without_prerequisites():
	var state = LuaState.new()
	state.open_libs()
	state.do_string("t = {}", "test")
	state.get_global("t")

	# settable needs key and value, we only have table
	var top = state.get_top()
	state.set_table(1)
	assert_engine_error("top")

	# Should not crash, stack unchanged
	assert_eq(state.get_top(), top, "Stack should be unchanged")

func test_setfield_without_value():
	var state = LuaState.new()
	state.open_libs()
	state.do_string("t = {}", "test")
	state.get_global("t")
	state.pop(1) # Empty stack

	# setfield needs a value on stack
	state.set_field(1, "key")
	assert_engine_error("top")

	# Should not crash
	assert_eq(state.get_top(), 0, "Stack should still be empty")

func test_rawget_without_key():
	var state = LuaState.new()
	state.open_libs()
	state.do_string("t = {a = 1}", "test")
	state.get_global("t")
	state.pop(1)

	# Try rawget without key
	state.raw_get(1)
	assert_engine_error("top")

	# Should not crash
	assert_eq(state.get_top(), 0, "Stack should be empty")

func test_rawset_without_prerequisites():
	var state = LuaState.new()
	state.open_libs()
	state.do_string("t = {}", "test")
	state.get_global("t")
	state.pop(1)

	# rawset needs key and value
	state.raw_set(1)
	assert_engine_error("top")

	# Should not crash
	assert_eq(state.get_top(), 0, "Stack should be empty")

func test_rawseti_without_value():
	var state = LuaState.new()
	state.open_libs()
	state.do_string("t = {}", "test")
	state.get_global("t")
	state.pop(1)

	# rawseti needs a value
	state.raw_seti(1, 1)
	assert_engine_error("top")

	# Should not crash
	assert_eq(state.get_top(), 0, "Stack should be empty")

func test_getmetatable_invalid_index():
	var state = LuaState.new()
	state.push_integer(42)

	# Try to get metatable of non-existent index
	var result = state.get_metatable(10)
	assert_engine_error("Invalid stack index")

	# Should return false, not crash
	assert_false(result, "getmetatable should return false for invalid index")

func test_setmetatable_without_metatable():
	var state = LuaState.new()
	state.push_integer(42)
	state.pop(1) # Empty stack

	# Try to setmetatable without metatable on stack
	var result = state.set_metatable(1)
	assert_engine_error("top")

	# Should return false, not crash
	assert_false(result, "setmetatable should return false without metatable")
	assert_eq(state.get_top(), 0, "Stack should still be empty")

func test_call_without_function():
	var state = LuaState.new()

	# Empty stack, try to call
	state.call(0, 0)
	assert_engine_error("has_n_items")

	# Should not crash
	assert_eq(state.get_top(), 0, "Stack should still be empty")

func test_call_without_enough_args():
	var state = LuaState.new()
	state.open_libs()
	state.do_string("function f(a, b, c) return a + b + c end", "test")
	state.get_global("f")

	var top = state.get_top()

	# Function expects 3 args, but we claim to pass 5
	state.call(5, 1)
	assert_engine_error("has_n_items")

	# Should not crash, operation rejected
	assert_eq(state.get_top(), top, "Stack should be unchanged")

func test_call_with_negative_nargs():
	var state = LuaState.new()
	state.open_libs()
	state.do_string("function f() return 42 end", "test")
	state.get_global("f")

	var top = state.get_top()

	# Negative nargs is invalid
	state.call(-1, 1)
	assert_engine_error("nargs")

	# Should not crash
	assert_eq(state.get_top(), top, "Stack should be unchanged")

func test_pcall_without_function():
	var state = LuaState.new()

	# Empty stack, try to pcall
	var status = state.pcall(0, 0, 0)
	assert_engine_error("has_n_items")

	# Should return error status, not crash
	assert_ne(status, 0, "Should return error status")
	assert_eq(state.get_top(), 0, "Stack should still be empty")

func test_pcall_with_invalid_errfunc():
	var state = LuaState.new()
	state.open_libs()
	state.do_string("function f() return 42 end", "test")
	state.get_global("f")

	# Invalid errfunc index
	var status = state.pcall(0, 1, 10)
	assert_engine_error("errfunc")

	# Should return error status, not crash
	assert_ne(status, 0, "Should return error status")

func test_valid_operations_still_work():
	var state = LuaState.new()
	state.open_libs()

	# Test that valid operations still work correctly
	state.push_integer(1)
	state.push_integer(2)
	state.push_integer(3)
	assert_eq(state.get_top(), 3, "Should have 3 items")

	state.pop(2)
	assert_eq(state.get_top(), 1, "Should have 1 item after pop")
	assert_eq(state.to_integer(-1), 1, "Should have correct value")

	# Test valid table operations
	state.new_table()
	state.push_string("key")
	state.push_integer(123)
	state.set_table(-3)

	state.push_string("key")
	state.get_table(-2)
	assert_eq(state.to_integer(-1), 123, "Table value should be set correctly")

	# Test valid function call
	state.do_string("function add(a, b) return a + b end", "test")
	state.get_global("add")
	state.push_integer(10)
	state.push_integer(20)
	state.call(2, 1)
	assert_eq(state.to_integer(-1), 30, "Function should return correct result")

func test_complex_scenario_no_crash():
	# Test a complex scenario with multiple boundary violations
	var state = LuaState.new()
	state.open_libs()

	# Perform a series of invalid operations
	state.pop(100) # Pop from empty stack
	assert_engine_error("Stack underflow")
	state.push_value(50) # Invalid index
	assert_engine_error("LuaState.pushvalue(50): Invalid stack index")
	state.remove(25) # Invalid index
	assert_engine_error("LuaState.remove(25): Invalid stack index")
	state.replace(30) # Invalid index
	assert_engine_error("Stack is empty")

	# Stack should still be valid
	assert_eq(state.get_top(), 0, "Stack should still be empty")

	# Should still be able to use the state
	state.push_integer(42)
	assert_eq(state.to_integer(-1), 42, "State should still work after invalid operations")
