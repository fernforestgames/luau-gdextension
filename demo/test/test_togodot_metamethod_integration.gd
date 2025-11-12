extends GutTest
# Integration tests for __togodot metamethod functionality
# Tests custom conversion of Lua values to Godot Variants and Objects

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
# Basic __togodot Metamethod Tests (to_variant)
# ============================================================================

func test_togodot_basic_conversion() -> void:
	# Create a table with __togodot metamethod that returns a custom value
	var code: String = """
	local t = {value = 42}
	local mt = {}
	setmetatable(t, mt)
	return t
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	# Set __togodot metamethod from Godot side
	assert_true(L.get_metatable(-1), "Table should have metatable")
	var converter = func(lua_state: LuaState, index: int, _tag: int) -> Variant:
		# Read the table's value field and return it multiplied by 2
		lua_state.get_field(index, "value")
		var val = lua_state.to_number(-1)
		lua_state.pop(1)
		return int(val * 2)
	L.push_callable(converter)
	L.set_field(-2, "__togodot")
	L.pop(1) # Pop metatable

	# Convert to variant - should call __togodot
	var result = L.to_variant(-1)
	assert_eq(result, 84, "__togodot should have been called and returned 42 * 2")

	L.pop(1)
	assert_stack_balanced()

func test_togodot_returns_string() -> void:
	# Test __togodot returning a string
	var code: String = """
	local t = {name = "test"}
	local mt = {}
	setmetatable(t, mt)
	return t
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	# Set __togodot to return a custom string
	assert_true(L.get_metatable(-1), "Table should have metatable")
	var converter = func(_lua_state: LuaState, _index: int, _tag: int) -> Variant:
		return "Custom Godot String"
	L.push_callable(converter)
	L.set_field(-2, "__togodot")
	L.pop(1)

	var result = L.to_variant(-1)
	assert_eq(result, "Custom Godot String", "__togodot should return custom string")

	L.pop(1)
	assert_stack_balanced()

func test_togodot_returns_dictionary() -> void:
	# Test __togodot returning a Dictionary
	var code: String = """
	local t = {x = 10, y = 20}
	local mt = {}
	setmetatable(t, mt)
	return t
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	assert_true(L.get_metatable(-1), "Table should have metatable")
	var converter = func(lua_state: LuaState, index: int, _tag: int) -> Variant:
		# Convert to a custom dictionary with different structure
		lua_state.get_field(index, "x")
		var x = lua_state.to_number(-1)
		lua_state.pop(1)
		lua_state.get_field(index, "y")
		var y = lua_state.to_number(-1)
		lua_state.pop(1)
		return {"sum": int(x + y), "product": int(x * y)}
	L.push_callable(converter)
	L.set_field(-2, "__togodot")
	L.pop(1)

	var result = L.to_variant(-1)
	assert_typeof(result, TYPE_DICTIONARY, "Should be a Dictionary")
	assert_eq(result["sum"], 30, "Sum should be correct")
	assert_eq(result["product"], 200, "Product should be correct")

	L.pop(1)
	assert_stack_balanced()

func test_togodot_returns_array() -> void:
	# Test __togodot returning an Array
	var code: String = """
	local t = {1, 2, 3}
	local mt = {}
	setmetatable(t, mt)
	return t
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	assert_true(L.get_metatable(-1), "Table should have metatable")
	var converter = func(lua_state: LuaState, index: int, _tag: int) -> Variant:
		# Convert to an array with doubled values
		var arr: Array = []
		var original = lua_state.to_array(index)
		for val in original:
			arr.append(val * 2)
		return arr
	L.push_callable(converter)
	L.set_field(-2, "__togodot")
	L.pop(1)

	var result = L.to_variant(-1)
	assert_typeof(result, TYPE_ARRAY, "Should be an Array")
	assert_eq(result, [2, 4, 6], "Array values should be doubled")

	L.pop(1)
	assert_stack_balanced()

func test_togodot_returns_vector() -> void:
	# Test __togodot returning a Vector3
	var code: String = """
	local t = {x = 1, y = 2, z = 3}
	local mt = {}
	setmetatable(t, mt)
	return t
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	assert_true(L.get_metatable(-1), "Table should have metatable")
	var converter = func(lua_state: LuaState, index: int, _tag: int) -> Variant:
		lua_state.get_field(index, "x")
		var x = lua_state.to_number(-1)
		lua_state.pop(1)
		lua_state.get_field(index, "y")
		var y = lua_state.to_number(-1)
		lua_state.pop(1)
		lua_state.get_field(index, "z")
		var z = lua_state.to_number(-1)
		lua_state.pop(1)
		return Vector3(x, y, z)
	L.push_callable(converter)
	L.set_field(-2, "__togodot")
	L.pop(1)

	var result = L.to_variant(-1)
	assert_typeof(result, TYPE_VECTOR3, "Should be a Vector3")
	assert_eq(result, Vector3(1, 2, 3), "Vector3 should have correct values")

	L.pop(1)
	assert_stack_balanced()

# ============================================================================
# __togodot with to_object Tests
# ============================================================================

func test_togodot_returns_object() -> void:
	# Test __togodot returning an Object via to_object
	var code: String = """
	local t = {type = "custom"}
	local mt = {}
	setmetatable(t, mt)
	return t
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var test_obj = RefCounted.new()

	assert_true(L.get_metatable(-1), "Table should have metatable")
	var converter = func(_lua_state: LuaState, _index: int, _tag: int) -> Variant:
		return test_obj
	L.push_callable(converter)
	L.set_field(-2, "__togodot")
	L.pop(1)

	# to_object should use __togodot
	var result = L.to_object(-1)
	assert_eq(result, test_obj, "to_object should return the object from __togodot")

	L.pop(1)
	assert_stack_balanced()

func test_togodot_returns_null_object() -> void:
	# Test __togodot returning null for to_object
	var code: String = """
	local t = {}
	local mt = {}
	setmetatable(t, mt)
	return t
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	assert_true(L.get_metatable(-1), "Table should have metatable")
	var converter = func(_lua_state: LuaState, _index: int, _tag: int) -> Variant:
		return null
	L.push_callable(converter)
	L.set_field(-2, "__togodot")
	L.pop(1)

	var result = L.to_object(-1)
	assert_eq(result, null, "to_object should accept null from __togodot")

	L.pop(1)
	assert_stack_balanced()

# ============================================================================
# __togodot with Tag Parameter Tests
# ============================================================================

func test_togodot_receives_tag_parameter() -> void:
	# Test that __togodot receives the tag parameter correctly via to_variant
	# Note: to_variant doesn't take a tag parameter, so it passes 0
	var code: String = """
	local t = {}
	local mt = {}
	setmetatable(t, mt)
	return t
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	assert_true(L.get_metatable(-1), "Table should have metatable")
	var converter = func(_lua_state: LuaState, _index: int, tag: int) -> Variant:
		return tag
	L.push_callable(converter)
	L.set_field(-2, "__togodot")
	L.pop(1)

	# to_variant doesn't take a tag parameter, so it passes 0 by default
	var result_variant = L.to_variant(-1)
	assert_eq(result_variant, 0, "__togodot should receive tag 0 from to_variant")

	L.pop(1)
	assert_stack_balanced()

func test_togodot_with_to_object_custom_tag() -> void:
	# Test __togodot with to_object and custom tag
	var code: String = """
	local t = {}
	local mt = {}
	setmetatable(t, mt)
	return t
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var obj1 = RefCounted.new()
	var obj2 = RefCounted.new()

	assert_true(L.get_metatable(-1), "Table should have metatable")
	var converter = func(_lua_state: LuaState, _index: int, tag: int) -> Variant:
		if tag == 100:
			return obj1
		else:
			return obj2
	L.push_callable(converter)
	L.set_field(-2, "__togodot")
	L.pop(1)

	# Test with tag 100
	var result1 = L.to_object(-1, 100)
	assert_eq(result1, obj1, "Should return obj1 for tag 100")

	# Test with tag -1 (default)
	var result2 = L.to_object(-1)
	assert_eq(result2, obj2, "Should return obj2 for default tag")

	L.pop(1)
	assert_stack_balanced()

# ============================================================================
# __togodot Inheritance Tests
# ============================================================================

func test_togodot_inherited_from_parent_metatable() -> void:
	# Test that __togodot can be inherited through metatable chain
	# Note: Metamethods are looked up via __index on the metatable itself
	var code: String = """
	-- Create base metatable
	local base_mt = {}

	-- Create derived metatable that has base_mt as its __index
	local derived_mt = {}
	-- Give derived_mt a metatable so it can inherit from base_mt
	setmetatable(derived_mt, {__index = base_mt})

	-- Create table with derived metatable
	local t = {value = 100}
	setmetatable(t, derived_mt)

	return t, base_mt
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	# Stack: [table, base_mt]
	# Set __togodot on base_mt
	var converter = func(lua_state: LuaState, index: int, _tag: int) -> Variant:
		lua_state.get_field(index, "value")
		var val = lua_state.to_number(-1)
		lua_state.pop(1)
		return int(val + 1000)
	L.push_callable(converter)
	L.set_field(-2, "__togodot") # Set in base_mt
	L.pop(1) # Pop base_mt

	# Convert table - should find __togodot through metatable inheritance
	var result = L.to_variant(-1)
	assert_eq(result, 1100, "__togodot should be inherited from base metatable")

	L.pop(1)
	assert_stack_balanced()

# ============================================================================
# __togodot with Different Value Types Tests
# ============================================================================

func test_togodot_on_userdata() -> void:
	# Test __togodot on userdata
	var test_obj = RefCounted.new()
	L.push_object(test_obj)

	# Add a custom metatable with __togodot
	L.create_table()
	L.push_default_object_metatable()
	L.set_field(-2, "__index")

	var converter = func(_lua_state: LuaState, _index: int, _tag: int) -> Variant:
		return "converted userdata"
	L.push_callable(converter)
	L.set_field(-2, "__togodot")

	L.set_metatable(-2)

	var result = L.to_variant(-1)
	assert_eq(result, "converted userdata", "__togodot should work on userdata")

	L.pop(1)
	assert_stack_balanced()

func test_togodot_on_number() -> void:
	# Test that numbers without metatables don't try to call __togodot
	L.push_number(42)

	var result = L.to_variant(-1)
	assert_eq(result, 42, "Number without metatable should convert normally")

	L.pop(1)
	assert_stack_balanced()

func test_togodot_on_string() -> void:
	# Test strings convert normally without __togodot
	L.push_string("test string")

	var result = L.to_variant(-1)
	assert_eq(result, "test string", "String should convert normally")

	L.pop(1)
	assert_stack_balanced()

# ============================================================================
# Edge Cases and Error Handling Tests
# ============================================================================

func test_togodot_not_present_uses_default_conversion() -> void:
	# Test that tables without __togodot use default conversion
	var code: String = """
	return {a = 1, b = 2, c = 3}
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result = L.to_variant(-1)
	assert_typeof(result, TYPE_DICTIONARY, "Should use default Dictionary conversion")
	assert_eq(result["a"], 1, "Should have correct values")

	L.pop(1)
	assert_stack_balanced()

func test_togodot_returns_complex_nested_structure() -> void:
	# Test __togodot returning complex nested structures
	var code: String = """
	local t = {data = "test"}
	local mt = {}
	setmetatable(t, mt)
	return t
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	assert_true(L.get_metatable(-1), "Table should have metatable")
	var converter = func(_lua_state: LuaState, _index: int, _tag: int) -> Variant:
		return {
			"level1": {
				"level2": {
					"level3": [1, 2, 3]
				}
			}
		}
	L.push_callable(converter)
	L.set_field(-2, "__togodot")
	L.pop(1)

	var result = L.to_variant(-1)
	assert_typeof(result, TYPE_DICTIONARY, "Should be a Dictionary")
	assert_true("level1" in result, "Should have level1 key")
	assert_eq(result["level1"]["level2"]["level3"], [1, 2, 3], "Nested structure should be correct")

	L.pop(1)
	assert_stack_balanced()

func test_togodot_can_read_lua_state() -> void:
	# Test that __togodot can use the LuaState to read other values
	var code: String = """
	local t = {multiplier = 5}
	local mt = {}
	setmetatable(t, mt)
	return t
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	assert_true(L.get_metatable(-1), "Table should have metatable")
	var converter = func(lua_state: LuaState, index: int, _tag: int) -> Variant:
		# Read the multiplier from the table
		lua_state.get_field(index, "multiplier")
		var mult = lua_state.to_number(-1)
		lua_state.pop(1)

		# Do some computation
		var result_array: Array = []
		for i in range(5):
			result_array.append(int(i * mult))
		return result_array
	L.push_callable(converter)
	L.set_field(-2, "__togodot")
	L.pop(1)

	var result = L.to_variant(-1)
	assert_typeof(result, TYPE_ARRAY, "Should be an Array")
	assert_eq(result, [0, 5, 10, 15, 20], "Should compute correctly using multiplier")

	L.pop(1)
	assert_stack_balanced()

func test_togodot_multiple_values_same_metatable() -> void:
	# Test multiple values sharing the same metatable with __togodot
	var code: String = """
	local mt = {}

	local t1 = {id = 1}
	setmetatable(t1, mt)

	local t2 = {id = 2}
	setmetatable(t2, mt)

	return t1, t2, mt
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	# Stack: [t1, t2, mt]
	# Set __togodot on shared metatable
	var converter = func(lua_state: LuaState, index: int, _tag: int) -> Variant:
		lua_state.get_field(index, "id")
		var id = lua_state.to_number(-1)
		lua_state.pop(1)
		return "Object_%d" % id
	L.push_callable(converter)
	L.set_field(-2, "__togodot")
	L.pop(1) # Pop mt

	# Convert both tables
	var result2 = L.to_variant(-1)
	L.pop(1)
	var result1 = L.to_variant(-1)
	L.pop(1)

	assert_eq(result1, "Object_1", "First table should convert correctly")
	assert_eq(result2, "Object_2", "Second table should convert correctly")

	assert_stack_balanced()

func test_togodot_returns_callable() -> void:
	# Test __togodot returning a Callable
	var code: String = """
	local t = {}
	local mt = {}
	setmetatable(t, mt)
	return t
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var test_callable = func(x: int) -> int:
		return x * 2

	assert_true(L.get_metatable(-1), "Table should have metatable")
	var converter = func(_lua_state: LuaState, _index: int, _tag: int) -> Variant:
		return test_callable
	L.push_callable(converter)
	L.set_field(-2, "__togodot")
	L.pop(1)

	var result = L.to_variant(-1)
	assert_typeof(result, TYPE_CALLABLE, "Should be a Callable")
	assert_eq(result.call(5), 10, "Callable should work correctly")

	L.pop(1)
	assert_stack_balanced()
