extends GutTest
# Integration tests for generic Variant conversions between Godot and Luau

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
# Primitive Type Tests
# ============================================================================

func test_nil_variant() -> void:
	var nil_var = null
	L.push_variant(nil_var)

	assert_true(L.is_nil(-1), "Should be nil in Lua")

	var retrieved = L.to_variant(-1)
	assert_null(retrieved, "Should retrieve as null")

func test_boolean_true() -> void:
	var bool_var = true
	L.push_variant(bool_var)

	assert_true(L.is_boolean(-1))
	assert_true(L.to_boolean(-1))

	var retrieved = L.to_variant(-1)
	assert_typeof(retrieved, TYPE_BOOL)
	assert_true(retrieved)

func test_boolean_false() -> void:
	var bool_var = false
	L.push_variant(bool_var)

	assert_true(L.is_boolean(-1))
	assert_false(L.to_boolean(-1))

	var retrieved = L.to_variant(-1)
	assert_false(retrieved)

func test_integer() -> void:
	var int_var = 42
	L.push_variant(int_var)

	assert_true(L.is_number(-1))
	assert_eq(L.to_integer(-1), 42)

	var retrieved = L.to_variant(-1)
	assert_eq(int(retrieved), 42)

func test_float() -> void:
	var float_var = 3.14159
	L.push_variant(float_var)

	assert_true(L.is_number(-1))
	assert_almost_eq(L.to_number(-1), 3.14159, 0.00001)

	var retrieved = L.to_variant(-1)
	assert_almost_eq(float(retrieved), 3.14159, 0.00001)

func test_negative_numbers() -> void:
	var neg_int = -100
	L.push_variant(neg_int)

	var retrieved = L.to_variant(-1)
	assert_eq(int(retrieved), -100)

func test_zero() -> void:
	var zero = 0
	L.push_variant(zero)

	var retrieved = L.to_variant(-1)
	assert_eq(int(retrieved), 0)

func test_very_large_integer() -> void:
	var large = 2147483647  # Max 32-bit int
	L.push_variant(large)

	var retrieved = L.to_variant(-1)
	assert_eq(int(retrieved), 2147483647)

func test_very_small_float() -> void:
	var small = 0.000001
	L.push_variant(small)

	var retrieved = L.to_variant(-1)
	assert_almost_eq(float(retrieved), 0.000001, 0.0000001)

# ============================================================================
# String Tests
# ============================================================================

func test_simple_string() -> void:
	var str_var = "hello world"
	L.push_variant(str_var)

	assert_true(L.is_string(-1))
	assert_eq(L.to_string(-1), "hello world")

	var retrieved = L.to_variant(-1)
	assert_typeof(retrieved, TYPE_STRING)
	assert_eq(retrieved, "hello world")

func test_empty_string() -> void:
	var empty = ""
	L.push_variant(empty)

	assert_true(L.is_string(-1))
	assert_false(L.is_nil(-1), "Empty string should not be nil")

	var retrieved = L.to_variant(-1)
	assert_eq(retrieved, "")

func test_string_with_special_characters() -> void:
	var special = "Hello\nWorld\t!"
	L.push_variant(special)

	var retrieved = L.to_variant(-1)
	assert_eq(retrieved, "Hello\nWorld\t!")

func test_unicode_string() -> void:
	var unicode = "こんにちは 世界 🌍"
	L.push_variant(unicode)

	var retrieved = L.to_variant(-1)
	assert_eq(retrieved, "こんにちは 世界 🌍")

func test_very_long_string() -> void:
	var long_str = ""
	for i in range(1000):
		long_str += "x"

	L.push_variant(long_str)
	var retrieved = L.to_variant(-1)

	assert_eq(retrieved.length(), 1000)

# ============================================================================
# Math Type Variant Tests
# ============================================================================

func test_vector2_variant() -> void:
	var vec = Vector2(3.5, 4.5)

	L.push_variant(vec)
	var retrieved = L.to_variant(-1)

	assert_typeof(retrieved, TYPE_VECTOR2)
	assert_almost_eq(retrieved.x, 3.5, 0.001)
	assert_almost_eq(retrieved.y, 4.5, 0.001)

func test_vector2i_variant() -> void:
	var vec = Vector2i(10, 20)

	L.push_variant(vec)
	var retrieved = L.to_variant(-1)

	assert_typeof(retrieved, TYPE_VECTOR2I)
	assert_eq(retrieved.x, 10)
	assert_eq(retrieved.y, 20)

func test_vector3_variant() -> void:
	var vec = Vector3(1.0, 2.0, 3.0)

	L.push_variant(vec)
	var retrieved = L.to_variant(-1)

	assert_typeof(retrieved, TYPE_VECTOR3)
	assert_almost_eq(retrieved.x, 1.0, 0.001)
	assert_almost_eq(retrieved.y, 2.0, 0.001)
	assert_almost_eq(retrieved.z, 3.0, 0.001)

func test_vector3i_variant() -> void:
	var vec = Vector3i(100, 200, 300)

	L.push_variant(vec)
	var retrieved = L.to_variant(-1)

	assert_typeof(retrieved, TYPE_VECTOR3I)
	assert_eq(retrieved.x, 100)
	assert_eq(retrieved.y, 200)
	assert_eq(retrieved.z, 300)

func test_color_variant() -> void:
	var col = Color(1.0, 0.5, 0.0, 0.8)

	L.push_variant(col)
	var retrieved = L.to_variant(-1)

	assert_typeof(retrieved, TYPE_COLOR)
	assert_almost_eq(retrieved.r, 1.0, 0.001)
	assert_almost_eq(retrieved.g, 0.5, 0.001)
	assert_almost_eq(retrieved.b, 0.0, 0.001)
	assert_almost_eq(retrieved.a, 0.8, 0.001)

func test_vector4_variant() -> void:
	var vec = Vector4(1.5, 2.5, 3.5, 4.5)

	L.push_variant(vec)
	var retrieved = L.to_variant(-1)

	assert_typeof(retrieved, TYPE_VECTOR4)
	assert_almost_eq(retrieved.x, 1.5, 0.001)
	assert_almost_eq(retrieved.y, 2.5, 0.001)
	assert_almost_eq(retrieved.z, 3.5, 0.001)
	assert_almost_eq(retrieved.w, 4.5, 0.001)

func test_vector4i_variant() -> void:
	var vec = Vector4i(10, 20, 30, 40)

	L.push_variant(vec)
	var retrieved = L.to_variant(-1)

	assert_typeof(retrieved, TYPE_VECTOR4I)
	assert_eq(retrieved.x, 10)
	assert_eq(retrieved.y, 20)
	assert_eq(retrieved.z, 30)
	assert_eq(retrieved.w, 40)

func test_rect2_variant() -> void:
	var rect = Rect2(10.0, 20.0, 100.0, 200.0)

	L.push_variant(rect)
	var retrieved = L.to_variant(-1)

	assert_typeof(retrieved, TYPE_RECT2)
	assert_almost_eq(retrieved.position.x, 10.0, 0.001)
	assert_almost_eq(retrieved.position.y, 20.0, 0.001)
	assert_almost_eq(retrieved.size.x, 100.0, 0.001)
	assert_almost_eq(retrieved.size.y, 200.0, 0.001)

func test_rect2i_variant() -> void:
	var rect = Rect2i(5, 10, 50, 100)

	L.push_variant(rect)
	var retrieved = L.to_variant(-1)

	assert_typeof(retrieved, TYPE_RECT2I)
	assert_eq(retrieved.position.x, 5)
	assert_eq(retrieved.position.y, 10)
	assert_eq(retrieved.size.x, 50)
	assert_eq(retrieved.size.y, 100)

func test_aabb_variant() -> void:
	var box = AABB(Vector3(1, 2, 3), Vector3(10, 20, 30))

	L.push_variant(box)
	var retrieved = L.to_variant(-1)

	assert_typeof(retrieved, TYPE_AABB)
	assert_almost_eq(retrieved.position.x, 1.0, 0.001)
	assert_almost_eq(retrieved.position.y, 2.0, 0.001)
	assert_almost_eq(retrieved.position.z, 3.0, 0.001)
	assert_almost_eq(retrieved.size.x, 10.0, 0.001)
	assert_almost_eq(retrieved.size.y, 20.0, 0.001)
	assert_almost_eq(retrieved.size.z, 30.0, 0.001)

func test_plane_variant() -> void:
	var plane = Plane(1.0, 0.0, 0.0, 5.0)

	L.push_variant(plane)
	var retrieved = L.to_variant(-1)

	assert_typeof(retrieved, TYPE_PLANE)
	assert_almost_eq(retrieved.normal.x, 1.0, 0.001)
	assert_almost_eq(retrieved.normal.y, 0.0, 0.001)
	assert_almost_eq(retrieved.normal.z, 0.0, 0.001)
	assert_almost_eq(retrieved.d, 5.0, 0.001)

func test_quaternion_variant() -> void:
	var quat = Quaternion(0.0, 0.0, 0.0, 1.0)

	L.push_variant(quat)
	var retrieved = L.to_variant(-1)

	assert_typeof(retrieved, TYPE_QUATERNION)
	assert_almost_eq(retrieved.x, 0.0, 0.001)
	assert_almost_eq(retrieved.y, 0.0, 0.001)
	assert_almost_eq(retrieved.z, 0.0, 0.001)
	assert_almost_eq(retrieved.w, 1.0, 0.001)

func test_basis_variant() -> void:
	var basis = Basis()  # Identity

	L.push_variant(basis)
	var retrieved = L.to_variant(-1)

	assert_typeof(retrieved, TYPE_BASIS)
	assert_eq(retrieved, Basis())

func test_transform2d_variant() -> void:
	var transform = Transform2D()  # Identity

	L.push_variant(transform)
	var retrieved = L.to_variant(-1)

	assert_typeof(retrieved, TYPE_TRANSFORM2D)
	assert_eq(retrieved, Transform2D())

func test_transform3d_variant() -> void:
	var transform = Transform3D()  # Identity

	L.push_variant(transform)
	var retrieved = L.to_variant(-1)

	assert_typeof(retrieved, TYPE_TRANSFORM3D)
	assert_eq(retrieved, Transform3D())

func test_projection_variant() -> void:
	var projection = Projection()  # Identity

	L.push_variant(projection)
	var retrieved = L.to_variant(-1)

	assert_typeof(retrieved, TYPE_PROJECTION)
	assert_eq(retrieved, Projection())

# ============================================================================
# Collection Variant Tests
# ============================================================================

func test_array_variant() -> void:
	var arr = [1, "two", 3.0]

	L.push_variant(arr)
	assert_true(L.is_table(-1))

	var retrieved = L.to_variant(-1)
	assert_typeof(retrieved, TYPE_ARRAY)

	var arr_result = retrieved as Array
	assert_eq(arr_result.size(), 3)
	assert_eq(arr_result[0], 1)
	assert_eq(arr_result[1], "two")
	assert_almost_eq(float(arr_result[2]), 3.0, 0.001)

func test_dictionary_variant() -> void:
	var dict = {"name": "test", "value": 42}

	L.push_variant(dict)
	assert_true(L.is_table(-1))

	var retrieved = L.to_variant(-1)
	assert_typeof(retrieved, TYPE_DICTIONARY)

	var dict_result = retrieved as Dictionary
	assert_eq(dict_result["name"], "test")
	assert_eq(dict_result["value"], 42)

func test_empty_array_variant() -> void:
	var empty = []
	L.push_variant(empty)

	var retrieved = L.to_variant(-1)
	var arr = retrieved as Array
	assert_eq(arr.size(), 0)

func test_empty_dictionary_variant() -> void:
	var empty = {}
	L.push_variant(empty)

	var retrieved = L.to_variant(-1)
	var dict = retrieved as Dictionary
	assert_eq(dict.size(), 0)

# ============================================================================
# Nested Structure Variant Tests
# ============================================================================

func test_nested_array_variant() -> void:
	var nested = [[1, 2], [3, 4]]
	L.push_variant(nested)

	var retrieved = L.to_variant(-1)
	var outer = retrieved as Array

	assert_eq(outer.size(), 2)

	var inner1 = outer[0] as Array
	assert_eq(inner1[0], 1)
	assert_eq(inner1[1], 2)

	var inner2 = outer[1] as Array
	assert_eq(inner2[0], 3)
	assert_eq(inner2[1], 4)

func test_complex_nested_variant() -> void:
	var complex = {
		"items": [10, 20, 30],
		"meta": {"count": 3},
		"name": "complex"
	}

	L.push_variant(complex)
	var retrieved = L.to_variant(-1)

	var dict = retrieved as Dictionary
	assert_eq(dict["name"], "complex")

	var items = dict["items"] as Array
	assert_eq(items.size(), 3)
	assert_eq(items[0], 10)

	var meta = dict["meta"] as Dictionary
	assert_eq(meta["count"], 3)

func test_array_with_mixed_types_including_math() -> void:
	var arr = [42, Vector2(1, 2), "text", Color(1, 0, 0, 1)]
	L.push_variant(arr)

	var retrieved = L.to_variant(-1)
	var result = retrieved as Array

	assert_eq(result.size(), 4)
	assert_eq(result[0], 42)

	var vec = result[1] as Vector2
	assert_almost_eq(vec.x, 1.0, 0.001)
	assert_almost_eq(vec.y, 2.0, 0.001)

	assert_eq(result[2], "text")

	var col = result[3] as Color
	assert_almost_eq(col.r, 1.0, 0.001)
	assert_almost_eq(col.a, 1.0, 0.001)

# ============================================================================
# Round-trip Through Lua Execution
# ============================================================================

func test_modify_variant_in_lua() -> void:
	var original = [1, 2]
	L.push_variant(original)
	L.set_global("arr")

	var code: String = """
	table.insert(arr, 3)
	table.insert(arr, 4)
	return arr
	"""

	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result = L.to_variant(-1)
	var result_arr = result as Array

	assert_eq(result_arr.size(), 4)
	assert_eq(result_arr[2], 3)
	assert_eq(result_arr[3], 4)

func test_create_complex_structure_in_lua() -> void:
	var code: String = """
	return {
		position = Vector2(100, 200),
		color = Color(1, 0, 0, 1),
		items = {10, 20, 30},
		metadata = {
			name = "entity",
			active = true
		}
	}
	"""

	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result = L.to_variant(-1)
	var dict = result as Dictionary

	assert_true(dict.has("position"))
	assert_true(dict.has("color"))
	assert_true(dict.has("items"))
	assert_true(dict.has("metadata"))

	var pos = dict["position"] as Vector2
	assert_almost_eq(pos.x, 100.0, 0.001)
	assert_almost_eq(pos.y, 200.0, 0.001)

	var items = dict["items"] as Array
	assert_eq(items.size(), 3)
	assert_eq(items[0], 10)

	var meta = dict["metadata"] as Dictionary
	assert_eq(meta["name"], "entity")
	assert_true(meta["active"])

# ============================================================================
# Type Edge Cases
# ============================================================================

func test_multiple_nil_variants() -> void:
	L.push_variant(null)
	L.push_variant(null)
	L.push_variant(null)

	assert_eq(L.get_top(), 3)
	assert_true(L.is_nil(-1))
	assert_true(L.is_nil(-2))
	assert_true(L.is_nil(-3))

func test_boolean_vs_nil() -> void:
	L.push_variant(false)
	assert_true(L.is_boolean(-1), "False should be boolean")
	assert_false(L.is_nil(-1), "False should not be nil")
	L.pop(1)

	L.push_variant(null)
	assert_false(L.is_boolean(-1), "Nil should not be boolean")
	assert_true(L.is_nil(-1), "Null should be nil")

func test_zero_vs_nil() -> void:
	L.push_variant(0)
	assert_true(L.is_number(-1), "Zero should be number")
	assert_false(L.is_nil(-1), "Zero should not be nil")
	assert_eq(L.to_integer(-1), 0)

func test_empty_string_vs_nil() -> void:
	L.push_variant("")
	assert_true(L.is_string(-1), "Empty string should be string")
	assert_false(L.is_nil(-1), "Empty string should not be nil")

	var retrieved = L.to_variant(-1)
	assert_eq(retrieved, "")
	assert_ne(retrieved, null)
