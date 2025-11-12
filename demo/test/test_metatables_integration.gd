extends GutTest
# Integration tests for userdata metatable functionality
# Tests read-only metatables, metatable inheritance, and custom metatables

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
# Metatable Inheritance Tests
# ============================================================================

func test_push_default_object_metatable() -> void:
	# Test that we can access the default object metatable
	L.push_default_object_metatable()
	assert_true(L.is_table(-1), "push_default_object_metatable should push a table")

	# Verify it has expected metamethods
	L.get_field(-1, "__tostring")
	assert_true(L.is_function(-1), "Default object metatable should have __tostring")
	L.pop(1)

	L.get_field(-1, "__eq")
	assert_true(L.is_function(-1), "Default object metatable should have __eq")
	L.pop(1)

	L.pop(1) # Pop metatable
	assert_stack_balanced()

func test_custom_metatable_with_inheritance() -> void:
	# Create an object and give it a custom metatable that inherits from default
	var test_obj = RefCounted.new()
	L.push_object(test_obj)

	# Create custom metatable
	var is_new = L.new_metatable_named("CustomObject")
	if is_new:
		L.push_default_object_metatable()
		L.set_field(-2, "__index")

		# Add a custom method
		var custom_method_code: String = """
		return function(obj)
			return "custom method called"
		end
		"""
		var bytecode: PackedByteArray = Luau.compile(custom_method_code, null)
		L.load_bytecode(bytecode, "custom_method")
		L.resume()
		L.set_field(-2, "custom_method")

	# Set the custom metatable
	L.set_metatable(-2)

	# Should still be able to convert to Object (inherits from Object metatable)
	var result_obj = L.to_full_userdata(-1)
	assert_eq(result_obj, test_obj, "Object with inherited metatable should be convertible")

	L.pop(1)
	assert_stack_balanced()

func test_multiple_levels_of_inheritance() -> void:
	# Create an object with multiple levels of metatable inheritance
	var test_obj = RefCounted.new()
	L.push_object(test_obj)

	# Create first level: inherits from default
	var is_new = L.new_metatable_named("Level1")
	if is_new:
		L.push_default_object_metatable()
		L.set_field(-2, "__index")
	L.set_metatable(-2)

	# Replace with second level: inherits from Level1
	L.get_metatable(-1)
	L.create_table() # Create Level2 metatable
	L.push_value(-2) # Push Level1 metatable
	L.set_field(-2, "__index")
	L.set_metatable(-3)
	L.pop(1) # Pop Level1 metatable

	# Should still be able to convert (walks inheritance chain)
	var result_obj = L.to_full_userdata(-1)
	assert_eq(result_obj, test_obj, "Object with multi-level inherited metatable should be convertible")

	L.pop(1)
	assert_stack_balanced()

# ============================================================================
# Tagged Userdata Tests
# ============================================================================

func test_tagged_userdata_basic() -> void:
	var tag: int = 50
	var test_obj = RefCounted.new()

	# Push tagged object
	L.push_object(test_obj, tag)

	# Should be able to convert with tag
	var result_obj = L.to_full_userdata(-1, tag)
	assert_eq(result_obj, test_obj, "Tagged object should be convertible with correct tag")

	# Should also work without specifying tag
	result_obj = L.to_full_userdata(-1)
	assert_eq(result_obj, test_obj, "Tagged object should be convertible without specifying tag")

	L.pop(1)
	assert_stack_balanced()

func test_tagged_userdata_tag_mismatch() -> void:
	var tag1: int = 70
	var tag2: int = 71
	var test_obj = RefCounted.new()

	# Push with tag1
	L.push_object(test_obj, tag1)

	# Try to read with tag2 (should fail)
	var result_obj = L.to_full_userdata(-1, tag2)
	assert_eq(result_obj, null, "Tag mismatch should return null")

	# Reading with tag1 should work
	result_obj = L.to_full_userdata(-1, tag1)
	assert_eq(result_obj, test_obj, "Correct tag should return object")

	L.pop(1)
	assert_stack_balanced()

# ============================================================================
# Default Metatable Metamethods Tests
# ============================================================================

func test_object_tostring_metamethod() -> void:
	var code: String = """
	local obj = ...
	return tostring(obj)
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")

	var test_obj = RefCounted.new()
	L.push_object(test_obj)
	L.resume(1)

	var result = L.to_variant(-1)
	assert_typeof(result, TYPE_STRING, "__tostring should return a string")
	assert_true(result is String and result.length() > 0, "__tostring should return non-empty string")

	L.pop(1)
	assert_stack_balanced()

func test_object_eq_metamethod() -> void:
	var code: String = """
	local obj1 = ...
	local obj2 = select(2, ...)
	return obj1 == obj2
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")

	var test_obj = RefCounted.new()
	L.push_object(test_obj)
	L.push_object(test_obj)
	L.resume(2)

	var result = L.to_variant(-1)
	assert_eq(result, true, "__eq should return true for same object")

	L.pop(1)
	assert_stack_balanced()

func test_object_lt_metamethod() -> void:
	var code: String = """
	local obj1 = ...
	local obj2 = select(2, ...)
	return obj1 < obj2
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")

	var obj1 = RefCounted.new()
	var obj2 = RefCounted.new()
	L.push_object(obj1)
	L.push_object(obj2)
	L.resume(2)

	var result = L.to_variant(-1)
	assert_typeof(result, TYPE_BOOL, "__lt should return a boolean")

	L.pop(1)
	assert_stack_balanced()

func test_object_le_metamethod() -> void:
	var code: String = """
	local obj1 = ...
	local obj2 = select(2, ...)
	return obj1 <= obj2
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")

	var obj1 = RefCounted.new()
	var obj2 = RefCounted.new()
	L.push_object(obj1)
	L.push_object(obj2)
	L.resume(2)

	var result = L.to_variant(-1)
	assert_typeof(result, TYPE_BOOL, "__le should return a boolean")

	L.pop(1)
	assert_stack_balanced()

# ============================================================================
# Custom push_to_lua Method Tests
# ============================================================================

# Helper class that implements push_to_lua to return a custom table
class CustomPushObject extends RefCounted:
	var value: int

	func _init(val: int) -> void:
		value = val

	func push_to_lua(lua_state: LuaState, tag: int) -> void:
		# Push a custom table representation instead of default userdata
		var custom_table: Dictionary = {
			"type": "CustomPushObject",
			"value": value,
			"tag": tag
		}
		lua_state.push_variant(custom_table)

func test_push_to_lua_basic() -> void:
	var obj = CustomPushObject.new(42)

	# Push the object - should call push_to_lua
	L.push_object(obj)

	# Should have pushed a table, not userdata
	assert_true(L.is_table(-1), "push_to_lua should have pushed a table")

	# Verify the table structure
	var result = L.to_variant(-1)
	assert_typeof(result, TYPE_DICTIONARY, "Should be a Dictionary")
	assert_eq(result["type"], "CustomPushObject", "Table should have correct type")
	assert_eq(result["value"], 42, "Table should have correct value")
	assert_eq(result["tag"], -1, "Table should have default tag")

	L.pop(1)
	assert_stack_balanced()

func test_push_to_lua_with_tag() -> void:
	var obj = CustomPushObject.new(100)
	var custom_tag: int = 999

	# Push with custom tag
	L.push_object(obj, custom_tag)

	assert_true(L.is_table(-1), "push_to_lua should have pushed a table")

	# Verify the tag was passed through
	var result = L.to_variant(-1)
	assert_eq(result["tag"], 999, "push_to_lua should receive the custom tag")
	assert_eq(result["value"], 100, "Table should have correct value")

	L.pop(1)
	assert_stack_balanced()

func test_push_to_lua_multiple_times() -> void:
	var obj = CustomPushObject.new(7)

	# Push the same object multiple times
	L.push_object(obj)
	L.push_object(obj)
	L.push_object(obj)

	# Each push should call push_to_lua and create a new table
	assert_eq(L.get_top(), 3, "Should have 3 items on stack")

	# Verify all three are tables with the same value
	for i in range(3):
		var result = L.to_variant(- (3 - i))
		assert_typeof(result, TYPE_DICTIONARY, "All items should be dictionaries")
		assert_eq(result["value"], 7, "All tables should have value 7")

	L.pop(3)
	assert_stack_balanced()

# Helper class that uses push_to_lua to push a simple number
class NumberPushObject extends RefCounted:
	var number: float

	func _init(n: float) -> void:
		number = n

	func push_to_lua(lua_state: LuaState, _tag: int) -> void:
		# Push a simple number instead of userdata or table
		lua_state.push_number(number)

func test_push_to_lua_can_push_any_type() -> void:
	var obj = NumberPushObject.new(3.14159)

	# Push the object
	L.push_object(obj)

	# Should have pushed a number, not userdata
	assert_true(L.is_number(-1), "push_to_lua can push any Lua type")
	assert_almost_eq(L.to_number(-1), 3.14159, 0.00001, "Number should be correct")

	L.pop(1)
	assert_stack_balanced()

# Helper class that uses push_to_lua for stack manipulation
class StackManipulationObject extends RefCounted:
	func push_to_lua(lua_state: LuaState, _tag: int) -> void:
		# Push multiple values and manipulate stack
		lua_state.push_string("first")
		lua_state.push_string("second")
		lua_state.push_string("final")
		# Pop the first two, leaving only "final"
		lua_state.remove(-3) # Remove "first"
		lua_state.remove(-2) # Remove "second"

func test_push_to_lua_stack_manipulation() -> void:
	var obj = StackManipulationObject.new()

	var initial_top = L.get_top()

	# Push the object
	L.push_object(obj)

	# Should have exactly one item on stack (net +1)
	assert_eq(L.get_top(), initial_top + 1, "push_to_lua must leave exactly one value on stack")

	# Verify it's the correct value
	assert_true(L.is_string(-1), "Should be a string")
	assert_eq(L.to_string_inplace(-1), "final", "Should be the final string")

	L.pop(1)
	assert_stack_balanced()

func test_push_to_lua_vs_normal_object() -> void:
	# Create two objects: one with push_to_lua, one without
	var custom_obj = CustomPushObject.new(50)
	var normal_obj = RefCounted.new()

	# Push both
	L.push_object(custom_obj)
	L.push_object(normal_obj)

	# Custom object should be a table
	assert_true(L.is_userdata(-1), "Normal object should be userdata")
	assert_false(L.is_table(-1), "Normal object should NOT be a table")

	assert_true(L.is_table(-2), "Custom object should be a table")
	assert_false(L.is_userdata(-2), "Custom object should NOT be userdata")

	L.pop(2)
	assert_stack_balanced()

# Helper class that uses push_to_lua to create a complex nested structure
class ComplexPushObject extends RefCounted:
	var name: String
	var nested_data: Dictionary

	func _init(obj_name: String, data: Dictionary) -> void:
		name = obj_name
		nested_data = data

	func push_to_lua(lua_state: LuaState, tag: int) -> void:
		# Create a complex nested table structure
		lua_state.create_table()

		# Add name
		lua_state.push_string("name")
		lua_state.push_string(name)
		lua_state.set_table(-3)

		# Add nested data
		lua_state.push_string("data")
		lua_state.push_variant(nested_data)
		lua_state.set_table(-3)

		# Add metadata
		lua_state.push_string("_meta")
		lua_state.create_table()
		lua_state.push_string("tag")
		lua_state.push_number(tag)
		lua_state.set_table(-3)
		lua_state.set_table(-3)

func test_push_to_lua_complex_structure() -> void:
	var nested: Dictionary = {
		"level": 1,
		"items": [1, 2, 3],
		"flags": {"enabled": true, "priority": 5}
	}
	var obj = ComplexPushObject.new("TestObject", nested)

	L.push_object(obj, 777)

	assert_true(L.is_table(-1), "Should have pushed a table")

	# Verify the complex structure
	var result = L.to_variant(-1)
	assert_typeof(result, TYPE_DICTIONARY, "Should be a dictionary")
	assert_eq(result["name"], "TestObject", "Should have name field")
	assert_true("data" in result, "Should have data field")
	assert_typeof(result["data"], TYPE_DICTIONARY, "data should be a dictionary")
	assert_eq(result["data"]["level"], 1, "Nested data should be correct")
	assert_true("_meta" in result, "Should have _meta field")
	assert_eq(result["_meta"]["tag"], 777, "Meta tag should be correct")

	L.pop(1)
	assert_stack_balanced()

func test_push_to_lua_from_lua_code() -> void:
	# Test that push_to_lua works when objects are passed back to Lua from GDScript
	var obj = CustomPushObject.new(999)

	var code: String = """
	local obj = ...
	-- The object should already be a table (from push_to_lua)
	return type(obj) == "table" and obj.value
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")

	# Push the object as argument - should trigger push_to_lua
	L.push_object(obj)
	L.resume(1)

	# Should return the value from the table
	var result = L.to_variant(-1)
	assert_eq(result, 999, "Lua should see the custom table representation")

	L.pop(1)
	assert_stack_balanced()
