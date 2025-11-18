extends GutTest
# Integration tests for lua_userdata_tag class constant feature
# Tests that classes can define a constant to specify their default tag

var L: LuaState

func before_each() -> void:
	L = LuaState.new()
	L.open_libs()

func after_each() -> void:
	if L:
		L.close()
		L = null

# Helper to verify Lua stack is balanced
func assert_stack_balanced(expected_top: int = 0) -> void:
	assert_eq(L.get_top(), expected_top, "Lua stack should be balanced at %d, but is %d" % [expected_top, L.get_top()])

# ============================================================================
# Class Constant Tag Tests
# ============================================================================

# Test class with lua_userdata_tag constant
class TaggedObject extends RefCounted:
	const lua_userdata_tag: int = 42

	var data: String

	func _init(val: String = "test") -> void:
		data = val

func test_class_with_tag_constant_uses_default_tag() -> void:
	var obj = TaggedObject.new("hello")

	# Push without explicit tag - should use class constant
	L.push_object(obj)

	# Should be able to read with the class's tag
	var result_obj = L.to_full_userdata(-1, TaggedObject.lua_userdata_tag)
	assert_eq(result_obj, obj, "Object should be retrievable with class constant tag")

	# Should also work without specifying tag
	result_obj = L.to_full_userdata(-1)
	assert_eq(result_obj, obj, "Object should be retrievable without specifying tag")

	L.pop(1)
	assert_stack_balanced()

func test_class_tag_constant_with_wrong_tag_returns_null() -> void:
	var obj = TaggedObject.new("test")

	# Push object (uses class constant tag 42)
	L.push_object(obj)

	# Try to read with wrong tag
	var result_obj = L.to_full_userdata(-1, 99)
	assert_eq(result_obj, null, "Wrong tag should return null")

	# Correct tag should work
	result_obj = L.to_full_userdata(-1, TaggedObject.lua_userdata_tag)
	assert_eq(result_obj, obj, "Correct tag should return object")

	L.pop(1)
	assert_stack_balanced()

func test_multiple_objects_same_class_tag() -> void:
	var obj1 = TaggedObject.new("first")
	var obj2 = TaggedObject.new("second")
	var obj3 = TaggedObject.new("third")

	# Push all three objects - should all use tag 42
	L.push_object(obj1)
	L.push_object(obj2)
	L.push_object(obj3)

	# All should be retrievable with the class tag
	var result3 = L.to_full_userdata(-1, TaggedObject.lua_userdata_tag)
	var result2 = L.to_full_userdata(-2, TaggedObject.lua_userdata_tag)
	var result1 = L.to_full_userdata(-3, TaggedObject.lua_userdata_tag)

	assert_eq(result1, obj1, "First object should be retrievable")
	assert_eq(result2, obj2, "Second object should be retrievable")
	assert_eq(result3, obj3, "Third object should be retrievable")

	L.pop(3)
	assert_stack_balanced()

# ============================================================================
# Explicit Tag Override Tests
# ============================================================================

func test_explicit_tag_overrides_class_constant() -> void:
	var obj = TaggedObject.new()
	var explicit_tag: int = 100

	# Push with explicit tag different from class constant (42)
	# This should generate a warning but use the explicit tag
	L.push_object(obj, explicit_tag)
	assert_engine_error("has lua_userdata_tag set to 42, but a different tag 100 was given")

	# Should NOT be readable with class constant tag
	var result_obj = L.to_full_userdata(-1, TaggedObject.lua_userdata_tag)
	assert_eq(result_obj, null, "Class constant tag should not work when explicit tag given")

	# Should be readable with explicit tag
	result_obj = L.to_full_userdata(-1, explicit_tag)
	assert_eq(result_obj, obj, "Explicit tag should override class constant")

	L.pop(1)
	assert_stack_balanced()

func test_explicit_tag_same_as_class_constant_no_warning() -> void:
	var obj = TaggedObject.new()

	# Push with explicit tag same as class constant
	# Should not generate warning
	L.push_object(obj, TaggedObject.lua_userdata_tag)

	# Should be readable with the tag
	var result_obj = L.to_full_userdata(-1, TaggedObject.lua_userdata_tag)
	assert_eq(result_obj, obj, "Same tag should work normally")

	L.pop(1)
	assert_stack_balanced()

# ============================================================================
# Different Tags for Different Classes
# ============================================================================

class AnotherTaggedObject extends RefCounted:
	const lua_userdata_tag: int = 75

	var value: int

	func _init(val: int = 0) -> void:
		value = val

func test_different_classes_different_tags() -> void:
	var obj1 = TaggedObject.new("tagged_42")
	var obj2 = AnotherTaggedObject.new(123)

	# Push both objects
	L.push_object(obj1) # Uses tag 42
	L.push_object(obj2) # Uses tag 75

	# Should not be cross-retrievable
	var wrong_result = L.to_full_userdata(-1, TaggedObject.lua_userdata_tag)
	assert_eq(wrong_result, null, "obj2 should not be retrievable with tag 42")

	wrong_result = L.to_full_userdata(-2, AnotherTaggedObject.lua_userdata_tag)
	assert_eq(wrong_result, null, "obj1 should not be retrievable with tag 75")

	# Should be retrievable with correct tags
	var result2 = L.to_full_userdata(-1, AnotherTaggedObject.lua_userdata_tag)
	var result1 = L.to_full_userdata(-2, TaggedObject.lua_userdata_tag)

	assert_eq(result1, obj1, "obj1 should be retrievable with tag 42")
	assert_eq(result2, obj2, "obj2 should be retrievable with tag 75")

	L.pop(2)
	assert_stack_balanced()

# ============================================================================
# Class Without Tag Constant
# ============================================================================

class UntaggedObject extends RefCounted:
	var name: String

	func _init(n: String = "untagged") -> void:
		name = n

func test_class_without_tag_constant_uses_default() -> void:
	var obj = UntaggedObject.new("test")

	# Push without explicit tag - should use LUA_NOTAG (default behavior)
	L.push_object(obj)

	# Should be retrievable without tag
	var result_obj = L.to_full_userdata(-1)
	assert_eq(result_obj, obj, "Untagged object should be retrievable")

	L.pop(1)
	assert_stack_balanced()

func test_class_without_tag_can_still_use_explicit_tag() -> void:
	var obj = UntaggedObject.new("explicit")
	var explicit_tag: int = 50

	# Push with explicit tag
	L.push_object(obj, explicit_tag)

	# Should be retrievable with that tag
	var result_obj = L.to_full_userdata(-1, explicit_tag)
	assert_eq(result_obj, obj, "Explicit tag should work on untagged class")

	L.pop(1)
	assert_stack_balanced()

# ============================================================================
# Tag Boundary Tests
# ============================================================================

class MinTagObject extends RefCounted:
	const lua_userdata_tag: int = 0 # Minimum valid tag

class MaxTagObject extends RefCounted:
	const lua_userdata_tag: int = 127 # Maximum valid tag (LUA_UTAG_LIMIT - 1)

func test_minimum_valid_tag() -> void:
	var obj = MinTagObject.new()

	L.push_object(obj)

	var result_obj = L.to_full_userdata(-1, 0)
	assert_eq(result_obj, obj, "Minimum tag (0) should work")

	L.pop(1)
	assert_stack_balanced()

func test_maximum_valid_tag() -> void:
	var obj = MaxTagObject.new()

	L.push_object(obj)

	var result_obj = L.to_full_userdata(-1, Luau.LUA_UTAG_LIMIT - 1)
	assert_eq(result_obj, obj, "Maximum tag (127) should work")

	L.pop(1)
	assert_stack_balanced()

# ============================================================================
# Integration with push_full_userdata
# ============================================================================

func test_push_full_userdata_respects_class_tag() -> void:
	var obj = TaggedObject.new("full_userdata")

	# push_full_userdata should also respect class constant
	L.push_full_userdata(obj)

	var result_obj = L.to_full_userdata(-1, TaggedObject.lua_userdata_tag)
	assert_eq(result_obj, obj, "push_full_userdata should use class constant tag")

	L.pop(1)
	assert_stack_balanced()

func test_push_full_userdata_with_explicit_tag_override() -> void:
	var obj = TaggedObject.new("override")
	var explicit_tag: int = 88

	# Explicit tag should override class constant
	L.push_full_userdata(obj, explicit_tag)
	assert_engine_error("has lua_userdata_tag set to 42, but a different tag 88 was given")

	# Should NOT work with class constant tag
	var wrong_result = L.to_full_userdata(-1, TaggedObject.lua_userdata_tag)
	assert_eq(wrong_result, null, "Class tag should not work when explicit tag given")

	# Should work with explicit tag
	var result_obj = L.to_full_userdata(-1, explicit_tag)
	assert_eq(result_obj, obj, "Explicit tag should override class constant")

	L.pop(1)
	assert_stack_balanced()

# ============================================================================
# Lua Code Integration Tests
# ============================================================================

func test_tagged_objects_from_lua_perspective() -> void:
	var obj1 = TaggedObject.new("lua_test_1")
	var obj2 = AnotherTaggedObject.new(456)

	var code: String = """
	local obj1, obj2 = ...
	-- Both should be userdata from Lua's perspective
	return type(obj1) == "userdata" and type(obj2) == "userdata"
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")

	L.push_object(obj1)
	L.push_object(obj2)
	L.resume(2)

	var result = L.to_variant(-1)
	assert_eq(result, true, "Tagged objects should appear as userdata in Lua")

	L.pop(1)
	assert_stack_balanced()

func test_tagged_objects_round_trip_through_lua() -> void:
	var obj = TaggedObject.new("round_trip")

	var code: String = """
	local obj = ...
	-- Just return the object back
	return obj
	"""
	var bytecode: PackedByteArray = Luau.compile(code, null)
	L.load_bytecode(bytecode, "test")

	L.push_object(obj)
	L.resume(1)

	# Should get the same object back
	var result_obj = L.to_full_userdata(-1, TaggedObject.lua_userdata_tag)
	assert_eq(result_obj, obj, "Object should survive round trip through Lua")

	L.pop(1)
	assert_stack_balanced()

# ============================================================================
# Null Object Tests
# ============================================================================

func test_null_object_still_pushes_nil() -> void:
	var obj: TaggedObject = null

	# Pushing null should still push nil, regardless of class tag
	L.push_object(obj)

	assert_true(L.is_nil(-1), "Null object should push nil")

	L.pop(1)
	assert_stack_balanced()

func test_null_object_with_explicit_tag_still_pushes_nil() -> void:
	var obj: TaggedObject = null

	# Even with explicit tag, null should push nil
	L.push_object(obj, 99)

	assert_true(L.is_nil(-1), "Null object should push nil even with explicit tag")

	L.pop(1)
	assert_stack_balanced()
