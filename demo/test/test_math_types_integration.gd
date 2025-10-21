extends GutTest
# Integration tests for Godot math type bridging with Luau
# Tests Vector2, Vector2i, Vector3, Vector3i, Color from GDScript perspective

var L: LuaState

func before_each():
	L = LuaState.new()
	L.openlibs()  # Load all libraries including Godot math types

func after_each():
	if L:
		L.close()
		L = null

# ============================================================================
# Vector2 Tests
# ============================================================================

func test_vector2_round_trip():
	var original = Vector2(3.5, 4.2)
	L.pushvariant(original)
	L.setglobal("v")

	L.getglobal("v")
	var retrieved = L.tovariant(-1)
	L.pop(1)

	assert_typeof(retrieved, TYPE_VECTOR2, "Should be Vector2 type")
	assert_almost_eq(retrieved.x, 3.5, 0.001, "X component should match")
	assert_almost_eq(retrieved.y, 4.2, 0.001, "Y component should match")

func test_vector2_construction_in_lua():
	var code = "return Vector2(10.5, 20.3)"
	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	var status = L.resume()

	assert_eq(status, Luau.LUA_OK, "Script should execute successfully")

	var result = L.tovariant(-1)
	assert_typeof(result, TYPE_VECTOR2)
	assert_almost_eq(result.x, 10.5, 0.001)
	assert_almost_eq(result.y, 20.3, 0.001)

func test_vector2_property_access():
	var v = Vector2(7.5, 8.5)
	L.pushvariant(v)
	L.setglobal("v")

	var code = """
	x_val = v.x
	y_val = v.y
	return x_val, y_val
	"""

	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	# Results: x_val, y_val
	var y = L.tonumber(-1)
	var x = L.tonumber(-2)

	assert_almost_eq(x, 7.5, 0.001)
	assert_almost_eq(y, 8.5, 0.001)

func test_vector2_property_modification():
	var v = Vector2(1.0, 2.0)
	L.pushvariant(v)
	L.setglobal("v")

	var code = """
	v.x = 99.5
	v.y = 88.5
	return v
	"""

	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var modified = L.tovariant(-1)
	assert_almost_eq(modified.x, 99.5, 0.001)
	assert_almost_eq(modified.y, 88.5, 0.001)

func test_vector2_addition():
	var code = """
	v1 = Vector2(1.0, 2.0)
	v2 = Vector2(3.0, 4.0)
	return v1 + v2
	"""

	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result = L.tovariant(-1)
	assert_almost_eq(result.x, 4.0, 0.001)
	assert_almost_eq(result.y, 6.0, 0.001)

func test_vector2_scalar_multiplication():
	var code = """
	v = Vector2(2.0, 3.0)
	return v * 2.5
	"""

	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result = L.tovariant(-1)
	assert_almost_eq(result.x, 5.0, 0.001)
	assert_almost_eq(result.y, 7.5, 0.001)

func test_vector2_equality():
	var code = """
	v1 = Vector2(1.0, 2.0)
	v2 = Vector2(1.0, 2.0)
	v3 = Vector2(3.0, 4.0)
	equal = (v1 == v2)
	not_equal = (v1 == v3)
	return equal, not_equal
	"""

	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var not_equal = L.toboolean(-1)
	var equal = L.toboolean(-2)

	assert_true(equal, "Equal vectors should compare true")
	assert_false(not_equal, "Different vectors should compare false")

# ============================================================================
# Vector2i Tests
# ============================================================================

func test_vector2i_integer_operations():
	var code = """
	v1 = Vector2i(10, 20)
	v2 = Vector2i(3, 7)
	sum = v1 + v2
	diff = v1 - v2
	return sum, diff
	"""

	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var diff = L.tovariant(-1)
	var sum = L.tovariant(-2)

	assert_typeof(sum, TYPE_VECTOR2I)
	assert_eq(sum.x, 13)
	assert_eq(sum.y, 27)

	assert_typeof(diff, TYPE_VECTOR2I)
	assert_eq(diff.x, 7)
	assert_eq(diff.y, 13)

# ============================================================================
# Vector3 Tests
# ============================================================================

func test_vector3_native_operations():
	var original = Vector3(1.5, 2.5, 3.5)
	L.pushvariant(original)
	L.setglobal("v")

	L.getglobal("v")
	var retrieved = L.tovariant(-1)
	L.pop(1)

	assert_typeof(retrieved, TYPE_VECTOR3)
	assert_almost_eq(retrieved.x, 1.5, 0.001)
	assert_almost_eq(retrieved.y, 2.5, 0.001)
	assert_almost_eq(retrieved.z, 3.5, 0.001)

func test_vector3_arithmetic():
	var code = """
	v1 = Vector3(1, 2, 3)
	v2 = Vector3(4, 5, 6)
	return v1 + v2
	"""

	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var sum = L.tovariant(-1)
	assert_typeof(sum, TYPE_VECTOR3)
	assert_almost_eq(sum.x, 5.0, 0.001)
	assert_almost_eq(sum.y, 7.0, 0.001)
	assert_almost_eq(sum.z, 9.0, 0.001)

func test_vector3_property_access():
	var code = """
	v = Vector3(7, 8, 9)
	return v.x, v.y, v.z
	"""

	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var z = L.tonumber(-1)
	var y = L.tonumber(-2)
	var x = L.tonumber(-3)

	assert_almost_eq(x, 7.0, 0.001)
	assert_almost_eq(y, 8.0, 0.001)
	assert_almost_eq(z, 9.0, 0.001)

# ============================================================================
# Vector3i Tests
# ============================================================================

func test_vector3i_construction():
	var original = Vector3i(100, 200, 300)
	L.pushvariant(original)

	var retrieved = L.tovariant(-1)
	L.pop(1)

	assert_typeof(retrieved, TYPE_VECTOR3I)
	assert_eq(retrieved.x, 100)
	assert_eq(retrieved.y, 200)
	assert_eq(retrieved.z, 300)

func test_vector3i_scalar_multiplication():
	var code = """
	v = Vector3i(5, 10, 15)
	return v * 2
	"""

	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result = L.tovariant(-1)
	assert_eq(result.x, 10)
	assert_eq(result.y, 20)
	assert_eq(result.z, 30)

# ============================================================================
# Color Tests
# ============================================================================

func test_color_rgb_construction():
	var code = "return Color(1.0, 0.5, 0.0)"

	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var color = L.tovariant(-1)
	assert_typeof(color, TYPE_COLOR)
	assert_almost_eq(color.r, 1.0, 0.001)
	assert_almost_eq(color.g, 0.5, 0.001)
	assert_almost_eq(color.b, 0.0, 0.001)
	assert_almost_eq(color.a, 1.0, 0.001, "Alpha should default to 1.0")

func test_color_rgba_construction():
	var code = "return Color(0.2, 0.4, 0.6, 0.8)"

	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var color = L.tovariant(-1)
	assert_almost_eq(color.r, 0.2, 0.001)
	assert_almost_eq(color.g, 0.4, 0.001)
	assert_almost_eq(color.b, 0.6, 0.001)
	assert_almost_eq(color.a, 0.8, 0.001)

func test_color_property_modification():
	var code = """
	c = Color(1, 0, 0, 1)
	c.g = 0.5
	c.a = 0.7
	return c
	"""

	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var color = L.tovariant(-1)
	assert_almost_eq(color.r, 1.0, 0.001)
	assert_almost_eq(color.g, 0.5, 0.001)
	assert_almost_eq(color.b, 0.0, 0.001)
	assert_almost_eq(color.a, 0.7, 0.001)

func test_color_addition():
	var code = """
	c1 = Color(0.2, 0.3, 0.4, 1.0)
	c2 = Color(0.1, 0.2, 0.1, 0.0)
	return c1 + c2
	"""

	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result = L.tovariant(-1)
	assert_almost_eq(result.r, 0.3, 0.001)
	assert_almost_eq(result.g, 0.5, 0.001)
	assert_almost_eq(result.b, 0.5, 0.001)
	assert_almost_eq(result.a, 1.0, 0.001)

func test_color_scalar_multiplication():
	var code = """
	c = Color(0.5, 0.5, 0.5, 1.0)
	return c * 2.0
	"""

	var bytecode = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result = L.tovariant(-1)
	assert_almost_eq(result.r, 1.0, 0.001)
	assert_almost_eq(result.g, 1.0, 0.001)
	assert_almost_eq(result.b, 1.0, 0.001)
	# Alpha is also scaled
	assert_almost_eq(result.a, 2.0, 0.001)

func test_color_round_trip():
	var original = Color(0.1, 0.2, 0.3, 0.4)
	L.pushvariant(original)

	var retrieved = L.tovariant(-1)
	L.pop(1)

	assert_typeof(retrieved, TYPE_COLOR)
	assert_almost_eq(retrieved.r, 0.1, 0.001)
	assert_almost_eq(retrieved.g, 0.2, 0.001)
	assert_almost_eq(retrieved.b, 0.3, 0.001)
	assert_almost_eq(retrieved.a, 0.4, 0.001)
