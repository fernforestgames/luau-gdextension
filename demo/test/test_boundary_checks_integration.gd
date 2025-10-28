extends GutTest
# Integration tests for boundary checking and crash prevention
# Tests that invalid operations print errors instead of crashing

func test_pop_from_empty_stack():
	var state = LuaState.new()

	# Empty stack - should not crash
	assert_eq(state.gettop(), 0, "Stack should be empty initially")

	# Try to pop from empty stack - should fail gracefully
	state.pop(1)

	# Stack should still be empty
	assert_eq(state.gettop(), 0, "Stack should still be empty after invalid pop")

func test_pop_more_than_available():
	var state = LuaState.new()
	state.pushinteger(1)
	state.pushinteger(2)

	assert_eq(state.gettop(), 2, "Should have 2 items")

	# Try to pop 10 items when only 2 exist
	state.pop(10)

	# Operation should be rejected, stack unchanged
	assert_eq(state.gettop(), 2, "Stack should be unchanged after invalid pop")

func test_pushvalue_with_invalid_index():
	var state = LuaState.new()
	state.pushinteger(42)

	var top = state.gettop()

	# Try to push invalid index
	state.pushvalue(100)

	# Stack should be unchanged
	assert_eq(state.gettop(), top, "Stack should be unchanged after invalid pushvalue")

func test_pushvalue_with_zero_index():
	var state = LuaState.new()
	state.pushinteger(1)

	var top = state.gettop()

	# Index 0 is never valid in Lua
	state.pushvalue(0)

	# Stack should be unchanged
	assert_eq(state.gettop(), top, "Stack should be unchanged with index 0")

func test_remove_invalid_index():
	var state = LuaState.new()
	state.pushinteger(1)
	state.pushinteger(2)

	var top = state.gettop()

	# Try to remove invalid index
	state.remove(10)

	# Stack should be unchanged
	assert_eq(state.gettop(), top, "Stack should be unchanged after invalid remove")

func test_replace_on_empty_stack():
	var state = LuaState.new()

	# Try to replace when there's nothing to replace with
	state.replace(1)

	# Should not crash, stack still empty
	assert_eq(state.gettop(), 0, "Stack should still be empty")

func test_insert_on_empty_stack():
	var state = LuaState.new()

	# Try to insert when there's nothing to insert
	state.insert(1)

	# Should not crash, stack still empty
	assert_eq(state.gettop(), 0, "Stack should still be empty")

func test_settop_with_invalid_negative():
	var state = LuaState.new()
	state.pushinteger(1)
	state.pushinteger(2)

	assert_eq(state.gettop(), 2, "Should have 2 items")

	# Try invalid negative index
	state.settop(-10)

	# Stack should be unchanged
	assert_eq(state.gettop(), 2, "Stack should be unchanged after invalid settop")

func test_gettable_without_key():
	var state = LuaState.new()
	state.openlibs()
	state.dostring("t = {a = 1}")
	state.getglobal("t")
	state.pop(1)  # Remove table, now stack is empty

	# Try gettable without a key
	state.gettable(1)

	# Should not crash
	assert_eq(state.gettop(), 0, "Stack should be empty")

func test_settable_without_prerequisites():
	var state = LuaState.new()
	state.openlibs()
	state.dostring("t = {}")
	state.getglobal("t")

	# settable needs key and value, we only have table
	var top = state.gettop()
	state.settable(1)

	# Should not crash, stack unchanged
	assert_eq(state.gettop(), top, "Stack should be unchanged")

func test_setfield_without_value():
	var state = LuaState.new()
	state.openlibs()
	state.dostring("t = {}")
	state.getglobal("t")
	state.pop(1)  # Empty stack

	# setfield needs a value on stack
	state.setfield(1, "key")

	# Should not crash
	assert_eq(state.gettop(), 0, "Stack should still be empty")

func test_rawget_without_key():
	var state = LuaState.new()
	state.openlibs()
	state.dostring("t = {a = 1}")
	state.getglobal("t")
	state.pop(1)

	# Try rawget without key
	state.rawget(1)

	# Should not crash
	assert_eq(state.gettop(), 0, "Stack should be empty")

func test_rawset_without_prerequisites():
	var state = LuaState.new()
	state.openlibs()
	state.dostring("t = {}")
	state.getglobal("t")
	state.pop(1)

	# rawset needs key and value
	state.rawset(1)

	# Should not crash
	assert_eq(state.gettop(), 0, "Stack should be empty")

func test_rawseti_without_value():
	var state = LuaState.new()
	state.openlibs()
	state.dostring("t = {}")
	state.getglobal("t")
	state.pop(1)

	# rawseti needs a value
	state.rawseti(1, 1)

	# Should not crash
	assert_eq(state.gettop(), 0, "Stack should be empty")

func test_getmetatable_invalid_index():
	var state = LuaState.new()
	state.pushinteger(42)

	# Try to get metatable of non-existent index
	var result = state.getmetatable(10)

	# Should return false, not crash
	assert_false(result, "getmetatable should return false for invalid index")

func test_setmetatable_without_metatable():
	var state = LuaState.new()
	state.pushinteger(42)
	state.pop(1)  # Empty stack

	# Try to setmetatable without metatable on stack
	var result = state.setmetatable(1)

	# Should return false, not crash
	assert_false(result, "setmetatable should return false without metatable")
	assert_eq(state.gettop(), 0, "Stack should still be empty")

func test_call_without_function():
	var state = LuaState.new()

	# Empty stack, try to call
	state.call(0, 0)

	# Should not crash
	assert_eq(state.gettop(), 0, "Stack should still be empty")

func test_call_without_enough_args():
	var state = LuaState.new()
	state.openlibs()
	state.dostring("function f(a, b, c) return a + b + c end")
	state.getglobal("f")

	var top = state.gettop()

	# Function expects 3 args, but we claim to pass 5
	state.call(5, 1)

	# Should not crash, operation rejected
	assert_eq(state.gettop(), top, "Stack should be unchanged")

func test_call_with_negative_nargs():
	var state = LuaState.new()
	state.openlibs()
	state.dostring("function f() return 42 end")
	state.getglobal("f")

	var top = state.gettop()

	# Negative nargs is invalid
	state.call(-1, 1)

	# Should not crash
	assert_eq(state.gettop(), top, "Stack should be unchanged")

func test_pcall_without_function():
	var state = LuaState.new()

	# Empty stack, try to pcall
	var status = state.pcall(0, 0, 0)

	# Should return error status, not crash
	assert_eq(status, LUA_ERRMEM, "Should return error status")
	assert_eq(state.gettop(), 0, "Stack should still be empty")

func test_pcall_with_invalid_errfunc():
	var state = LuaState.new()
	state.openlibs()
	state.dostring("function f() return 42 end")
	state.getglobal("f")

	# Invalid errfunc index
	var status = state.pcall(0, 1, 10)

	# Should return error status, not crash
	assert_eq(status, LUA_ERRMEM, "Should return error status")

func test_valid_operations_still_work():
	var state = LuaState.new()
	state.openlibs()

	# Test that valid operations still work correctly
	state.pushinteger(1)
	state.pushinteger(2)
	state.pushinteger(3)
	assert_eq(state.gettop(), 3, "Should have 3 items")

	state.pop(2)
	assert_eq(state.gettop(), 1, "Should have 1 item after pop")
	assert_eq(state.tointeger(-1), 1, "Should have correct value")

	# Test valid table operations
	state.newtable()
	state.pushstring("key")
	state.pushinteger(123)
	state.settable(-3)

	state.pushstring("key")
	state.gettable(-2)
	assert_eq(state.tointeger(-1), 123, "Table value should be set correctly")

	# Test valid function call
	state.dostring("function add(a, b) return a + b end")
	state.getglobal("add")
	state.pushinteger(10)
	state.pushinteger(20)
	state.call(2, 1)
	assert_eq(state.tointeger(-1), 30, "Function should return correct result")

func test_complex_scenario_no_crash():
	# Test a complex scenario with multiple boundary violations
	var state = LuaState.new()
	state.openlibs()

	# Perform a series of invalid operations
	state.pop(100)  # Pop from empty stack
	state.pushvalue(50)  # Invalid index
	state.remove(25)  # Invalid index
	state.replace(30)  # Invalid index

	# Stack should still be valid
	assert_eq(state.gettop(), 0, "Stack should still be empty")

	# Should still be able to use the state
	state.pushinteger(42)
	assert_eq(state.tointeger(-1), 42, "State should still work after invalid operations")
