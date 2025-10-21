extends GutTest
# Integration tests for generic Variant conversions between Godot and Luau

var L: LuaState

func before_each():
	L = LuaState.new()
	L.openlibs()

func after_each():
	if L:
		L.close()
		L = null

# ============================================================================
# Primitive Type Tests
# ============================================================================

func test_nil_variant():
	var nil_var = null
	L.pushvariant(nil_var)

	assert_true(L.isnil(-1), "Should be nil in Lua")

	var retrieved = L.tovariant(-1)
	assert_null(retrieved, "Should retrieve as null")

func test_boolean_true():
	var bool_var = true
	L.pushvariant(bool_var)

	assert_true(L.isboolean(-1))
	assert_true(L.toboolean(-1))

	var retrieved = L.tovariant(-1)
	assert_typeof(retrieved, TYPE_BOOL)
	assert_true(retrieved)

func test_boolean_false():
	var bool_var = false
	L.pushvariant(bool_var)

	assert_true(L.isboolean(-1))
	assert_false(L.toboolean(-1))

	var retrieved = L.tovariant(-1)
	assert_false(retrieved)

func test_integer():
	var int_var = 42
	L.pushvariant(int_var)

	assert_true(L.isnumber(-1))
	assert_eq(L.tointeger(-1), 42)

	var retrieved = L.tovariant(-1)
	assert_eq(int(retrieved), 42)

func test_float():
	var float_var = 3.14159
	L.pushvariant(float_var)

	assert_true(L.isnumber(-1))
	assert_almost_eq(L.tonumber(-1), 3.14159, 0.00001)

	var retrieved = L.tovariant(-1)
	assert_almost_eq(float(retrieved), 3.14159, 0.00001)

func test_negative_numbers():
	var neg_int = -100
	L.pushvariant(neg_int)

	var retrieved = L.tovariant(-1)
	assert_eq(int(retrieved), -100)

func test_zero():
	var zero = 0
	L.pushvariant(zero)

	var retrieved = L.tovariant(-1)
	assert_eq(int(retrieved), 0)

func test_very_large_integer():
	var large = 2147483647  # Max 32-bit int
	L.pushvariant(large)

	var retrieved = L.tovariant(-1)
	assert_eq(int(retrieved), 2147483647)

func test_very_small_float():
	var small = 0.000001
	L.pushvariant(small)

	var retrieved = L.tovariant(-1)
	assert_almost_eq(float(retrieved), 0.000001, 0.0000001)

# ============================================================================
# String Tests
# ============================================================================

func test_simple_string():
	var str_var = "hello world"
	L.pushvariant(str_var)

	assert_true(L.isstring(-1))
	assert_eq(L.tostring(-1), "hello world")

	var retrieved = L.tovariant(-1)
	assert_typeof(retrieved, TYPE_STRING)
	assert_eq(retrieved, "hello world")

func test_empty_string():
	var empty = ""
	L.pushvariant(empty)

	assert_true(L.isstring(-1))
	assert_false(L.isnil(-1), "Empty string should not be nil")

	var retrieved = L.tovariant(-1)
	assert_eq(retrieved, "")

func test_string_with_special_characters():
	var special = "Hello\nWorld\t!"
	L.pushvariant(special)

	var retrieved = L.tovariant(-1)
	assert_eq(retrieved, "Hello\nWorld\t!")

func test_unicode_string():
	var unicode = "こんにちは 世界 🌍"
	L.pushvariant(unicode)

	var retrieved = L.tovariant(-1)
	assert_eq(retrieved, "こんにちは 世界 🌍")

func test_very_long_string():
	var long_str = ""
	for i in range(1000):
		long_str += "x"

	L.pushvariant(long_str)
	var retrieved = L.tovariant(-1)

	assert_eq(retrieved.length(), 1000)

# ============================================================================
# Math Type Variant Tests
# ============================================================================

func test_vector2_variant():
	var vec = Vector2(3.5, 4.5)
	var variant = Variant(vec)

	L.pushvariant(variant)
	var retrieved = L.tovariant(-1)

	assert_typeof(retrieved, TYPE_VECTOR2)
	assert_almost_eq(retrieved.x, 3.5, 0.001)
	assert_almost_eq(retrieved.y, 4.5, 0.001)

func test_vector2i_variant():
	var vec = Vector2i(10, 20)
	var variant = Variant(vec)

	L.pushvariant(variant)
	var retrieved = L.tovariant(-1)

	assert_typeof(retrieved, TYPE_VECTOR2I)
	assert_eq(retrieved.x, 10)
	assert_eq(retrieved.y, 20)

func test_vector3_variant():
	var vec = Vector3(1.0, 2.0, 3.0)
	var variant = Variant(vec)

	L.pushvariant(variant)
	var retrieved = L.tovariant(-1)

	assert_typeof(retrieved, TYPE_VECTOR3)
	assert_almost_eq(retrieved.x, 1.0, 0.001)
	assert_almost_eq(retrieved.y, 2.0, 0.001)
	assert_almost_eq(retrieved.z, 3.0, 0.001)

func test_vector3i_variant():
	var vec = Vector3i(100, 200, 300)
	var variant = Variant(vec)

	L.pushvariant(variant)
	var retrieved = L.tovariant(-1)

	assert_typeof(retrieved, TYPE_VECTOR3I)
	assert_eq(retrieved.x, 100)
	assert_eq(retrieved.y, 200)
	assert_eq(retrieved.z, 300)

func test_color_variant():
	var col = Color(1.0, 0.5, 0.0, 0.8)
	var variant = Variant(col)

	L.pushvariant(variant)
	var retrieved = L.tovariant(-1)

	assert_typeof(retrieved, TYPE_COLOR)
	assert_almost_eq(retrieved.r, 1.0, 0.001)
	assert_almost_eq(retrieved.g, 0.5, 0.001)
	assert_almost_eq(retrieved.b, 0.0, 0.001)
	assert_almost_eq(retrieved.a, 0.8, 0.001)

# ============================================================================
# Collection Variant Tests
# ============================================================================

func test_array_variant():
	var arr = [1, "two", 3.0]
	var variant = Variant(arr)

	L.pushvariant(variant)
	assert_true(L.istable(-1))

	var retrieved = L.tovariant(-1)
	assert_typeof(retrieved, TYPE_ARRAY)

	var arr_result = Array(retrieved)
	assert_eq(arr_result.size(), 3)
	assert_eq(arr_result[0], 1)
	assert_eq(arr_result[1], "two")
	assert_almost_eq(float(arr_result[2]), 3.0, 0.001)

func test_dictionary_variant():
	var dict = {"name": "test", "value": 42}
	var variant = Variant(dict)

	L.pushvariant(variant)
	assert_true(L.istable(-1))

	var retrieved = L.tovariant(-1)
	assert_typeof(retrieved, TYPE_DICTIONARY)

	var dict_result = Dictionary(retrieved)
	assert_eq(dict_result["name"], "test")
	assert_eq(dict_result["value"], 42)

func test_empty_array_variant():
	var empty = []
	L.pushvariant(empty)

	var retrieved = L.tovariant(-1)
	var arr = Array(retrieved)
	assert_eq(arr.size(), 0)

func test_empty_dictionary_variant():
	var empty = {}
	L.pushvariant(empty)

	var retrieved = L.tovariant(-1)
	var dict = Dictionary(retrieved)
	assert_eq(dict.size(), 0)

# ============================================================================
# Nested Structure Variant Tests
# ============================================================================

func test_nested_array_variant():
	var nested = [[1, 2], [3, 4]]
	L.pushvariant(nested)

	var retrieved = L.tovariant(-1)
	var outer = Array(retrieved)

	assert_eq(outer.size(), 2)

	var inner1 = Array(outer[0])
	assert_eq(inner1[0], 1)
	assert_eq(inner1[1], 2)

	var inner2 = Array(outer[1])
	assert_eq(inner2[0], 3)
	assert_eq(inner2[1], 4)

func test_complex_nested_variant():
	var complex = {
		"items": [10, 20, 30],
		"meta": {"count": 3},
		"name": "complex"
	}

	L.pushvariant(complex)
	var retrieved = L.tovariant(-1)

	var dict = Dictionary(retrieved)
	assert_eq(dict["name"], "complex")

	var items = Array(dict["items"])
	assert_eq(items.size(), 3)
	assert_eq(items[0], 10)

	var meta = Dictionary(dict["meta"])
	assert_eq(meta["count"], 3)

func test_array_with_mixed_types_including_math():
	var arr = [42, Vector2(1, 2), "text", Color(1, 0, 0, 1)]
	L.pushvariant(arr)

	var retrieved = L.tovariant(-1)
	var result = Array(retrieved)

	assert_eq(result.size(), 4)
	assert_eq(result[0], 42)

	var vec = Vector2(result[1])
	assert_almost_eq(vec.x, 1.0, 0.001)
	assert_almost_eq(vec.y, 2.0, 0.001)

	assert_eq(result[2], "text")

	var col = Color(result[3])
	assert_almost_eq(col.r, 1.0, 0.001)
	assert_almost_eq(col.a, 1.0, 0.001)

# ============================================================================
# Round-trip Through Lua Execution
# ============================================================================

func test_modify_variant_in_lua():
	var original = [1, 2]
	L.pushvariant(original)
	L.setglobal("arr")

	var code = """
	table.insert(arr, 3)
	table.insert(arr, 4)
	return arr
	"""

	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result = L.tovariant(-1)
	var result_arr = Array(result)

	assert_eq(result_arr.size(), 4)
	assert_eq(result_arr[2], 3)
	assert_eq(result_arr[3], 4)

func test_create_complex_structure_in_lua():
	var code = """
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

	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result = L.tovariant(-1)
	var dict = Dictionary(result)

	assert_true(dict.has("position"))
	assert_true(dict.has("color"))
	assert_true(dict.has("items"))
	assert_true(dict.has("metadata"))

	var pos = Vector2(dict["position"])
	assert_almost_eq(pos.x, 100.0, 0.001)
	assert_almost_eq(pos.y, 200.0, 0.001)

	var items = Array(dict["items"])
	assert_eq(items.size(), 3)
	assert_eq(items[0], 10)

	var meta = Dictionary(dict["metadata"])
	assert_eq(meta["name"], "entity")
	assert_true(meta["active"])

# ============================================================================
# Type Edge Cases
# ============================================================================

func test_multiple_nil_variants():
	L.pushvariant(null)
	L.pushvariant(null)
	L.pushvariant(null)

	assert_eq(L.gettop(), 3)
	assert_true(L.isnil(-1))
	assert_true(L.isnil(-2))
	assert_true(L.isnil(-3))

func test_boolean_vs_nil():
	L.pushvariant(false)
	assert_true(L.isboolean(-1), "False should be boolean")
	assert_false(L.isnil(-1), "False should not be nil")
	L.pop(1)

	L.pushvariant(null)
	assert_false(L.isboolean(-1), "Nil should not be boolean")
	assert_true(L.isnil(-1), "Null should be nil")

func test_zero_vs_nil():
	L.pushvariant(0)
	assert_true(L.isnumber(-1), "Zero should be number")
	assert_false(L.isnil(-1), "Zero should not be nil")
	assert_eq(L.tointeger(-1), 0)

func test_empty_string_vs_nil():
	L.pushvariant("")
	assert_true(L.isstring(-1), "Empty string should be string")
	assert_false(L.isnil(-1), "Empty string should not be nil")

	var retrieved = L.tovariant(-1)
	assert_eq(retrieved, "")
	assert_ne(retrieved, null)
