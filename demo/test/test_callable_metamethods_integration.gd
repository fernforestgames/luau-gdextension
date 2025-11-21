extends GutTest

# Test using Godot Callables as Lua metamethods (__index, __newindex, etc.)
# This tests the reverse direction of callable bridging: passing Godot functions into Lua

func test_callable_as_index_metamethod():
	var state = LuaState.new()
	state.open_libs()

	# Track if callable was invoked
	var index_calls = []

	# Create a Godot callable for __index
	var index_callable = func(_table, key):
		index_calls.append(key)
		return "indexed_" + str(key)

	# Create a table
	state.create_table()
	var table_idx = state.get_top()

	# Create a metatable
	state.create_table()

	# Set __index metamethod to our Godot Callable
	state.set_index_metamethod(-1, index_callable)

	# Set the metatable on the table
	state.set_metatable(table_idx)

	# Test __index - access a non-existent key
	state.push_value(table_idx)
	state.push_string("test_key")
	state.get_table(-2)
	var result = state.to_string_inplace(-1)
	state.pop(2)

	assert_eq(index_calls.size(), 1, "__index callable should have been called once")
	assert_eq(index_calls[0], "test_key", "__index should receive correct key")
	assert_eq(result, "indexed_test_key", "__index should return correct value")

	state.close()

func test_callable_as_newindex_metamethod():
	var state = LuaState.new()
	state.open_libs()

	# Track if callable was invoked
	var newindex_calls = []

	# Create a Godot callable for __newindex
	var newindex_callable = func(_table, key, value):
		newindex_calls.append([key, value])

	# Create a table
	state.create_table()
	var table_idx = state.get_top()

	# Create a metatable
	state.create_table()

	# Set __newindex metamethod to our Godot Callable
	state.set_newindex_metamethod(-1, newindex_callable)

	# Set the metatable on the table
	state.set_metatable(table_idx)

	# Test __newindex - set a non-existent key
	state.push_value(table_idx)
	state.push_string("new_key")
	state.push_string("new_value")
	state.set_table(-3)
	state.pop(1)

	assert_eq(newindex_calls.size(), 1, "__newindex callable should have been called once")
	assert_eq(newindex_calls[0][0], "new_key", "__newindex should receive correct key")
	assert_eq(newindex_calls[0][1], "new_value", "__newindex should receive correct value")

	state.close()

func test_callable_as_call_metamethod():
	var state = LuaState.new()
	state.open_libs()

	# Track if callable was invoked
	var call_invocations = []

	# Create a Godot callable for __call
	var call_callable = func(_table, arg1, arg2):
		call_invocations.append([arg1, arg2])
		return arg1 + arg2

	# Create a table with __call metamethod
	state.create_table()
	state.create_table()
	state.set_call_metamethod(-1, call_callable)
	state.set_metatable(-2)

	# Set as global
	state.set_global("callable_table")

	# Test __call - invoke the table as a function via Lua code
	var lua_code = """
	return callable_table(10, 20)
	"""
	var status = state.do_string(lua_code, "test_call")

	assert_eq(status, Luau.LUA_OK, "__call should execute successfully")
	assert_eq(call_invocations.size(), 1, "__call callable should have been invoked once")
	assert_eq(call_invocations[0][0], 10, "__call should receive first arg")
	assert_eq(call_invocations[0][1], 20, "__call should receive second arg")

	var result = state.to_number(-1)
	assert_eq(result, 30, "__call should return the sum")

	state.close()

func test_callable_metamethods_combined():
	var state = LuaState.new()
	state.open_libs()

	# Track invocations
	var index_calls = []
	var newindex_calls = []

	# Create Godot callables
	var index_callable = func(_table, key):
		index_calls.append(key)
		return "value_for_" + str(key)

	var newindex_callable = func(_table, key, value):
		newindex_calls.append([key, value])

	# Create table and metatable
	state.create_table()
	var table_idx = state.get_top()

	# Set both metamethods
	state.create_table()
	state.set_index_metamethod(-1, index_callable)
	state.set_newindex_metamethod(-1, newindex_callable)
	state.set_metatable(table_idx)

	# Test via Lua code
	state.push_value(table_idx)
	state.set_global("my_table")

	var lua_code = """
	local val = my_table.foo  -- Should trigger __index
	my_table.bar = 42        -- Should trigger __newindex
	return val
	"""

	var status = state.do_string(lua_code, "test_metamethods")

	assert_eq(status, Luau.LUA_OK, "Lua code should execute successfully")
	assert_eq(index_calls.size(), 1, "__index should be called once")
	assert_eq(index_calls[0], "foo", "__index should receive 'foo' key")
	assert_eq(newindex_calls.size(), 1, "__newindex should be called once")
	assert_eq(newindex_calls[0][0], "bar", "__newindex should receive 'bar' key")
	assert_eq(newindex_calls[0][1], 42, "__newindex should receive value 42")

	var result = state.to_string_inplace(-1)
	assert_eq(result, "value_for_foo", "Should return value from __index")

	state.close()
