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
	L.push_userdata(test_obj)

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
	var result_obj = L.to_userdata(-1)
	assert_eq(result_obj, test_obj, "Object with inherited metatable should be convertible")

	L.pop(1)
	assert_stack_balanced()

func test_multiple_levels_of_inheritance() -> void:
	# Create an object with multiple levels of metatable inheritance
	var test_obj = RefCounted.new()
	L.push_userdata(test_obj)

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
	var result_obj = L.to_userdata(-1)
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
	L.push_userdata(test_obj, tag)

	# Should be able to convert with tag
	var result_obj = L.to_userdata(-1, tag)
	assert_eq(result_obj, test_obj, "Tagged object should be convertible with correct tag")

	# Should also work without specifying tag
	result_obj = L.to_userdata(-1)
	assert_eq(result_obj, test_obj, "Tagged object should be convertible without specifying tag")

	L.pop(1)
	assert_stack_balanced()

func test_tagged_userdata_tag_mismatch() -> void:
	var tag1: int = 70
	var tag2: int = 71
	var test_obj = RefCounted.new()

	# Push with tag1
	L.push_userdata(test_obj, tag1)

	# Try to read with tag2 (should fail)
	var result_obj = L.to_userdata(-1, tag2)
	assert_eq(result_obj, null, "Tag mismatch should return null")

	# Reading with tag1 should work
	result_obj = L.to_userdata(-1, tag1)
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
	L.push_userdata(test_obj)
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
	L.push_userdata(test_obj)
	L.push_userdata(test_obj)
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
	L.push_userdata(obj1)
	L.push_userdata(obj2)
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
	L.push_userdata(obj1)
	L.push_userdata(obj2)
	L.resume(2)

	var result = L.to_variant(-1)
	assert_typeof(result, TYPE_BOOL, "__le should return a boolean")

	L.pop(1)
	assert_stack_balanced()
