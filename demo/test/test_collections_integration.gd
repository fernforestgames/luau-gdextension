extends GutTest
# Integration tests for Array and Dictionary bridging with Luau

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
# Array Tests
# ============================================================================

func test_simple_array_round_trip() -> void:
	var original = [1, 2, 3, 4]
	L.push_variant(original)
	L.set_global("arr")

	L.get_global("arr")
	var retrieved = L.to_array(-1)
	L.pop(1)

	assert_typeof(retrieved, TYPE_ARRAY)
	assert_eq(retrieved.size(), 4)
	assert_eq(retrieved[0], 1)
	assert_eq(retrieved[1], 2)
	assert_eq(retrieved[2], 3)
	assert_eq(retrieved[3], 4)

func test_mixed_type_array() -> void:
	var original = [42, "hello", true, 3.14]
	L.push_variant(original)
	L.set_global("mixed")

	L.get_global("mixed")
	var retrieved = L.to_array(-1)
	L.pop(1)

	assert_eq(retrieved.size(), 4)
	assert_eq(retrieved[0], 42)
	assert_eq(retrieved[1], "hello")
	assert_eq(retrieved[2], true)
	assert_almost_eq(float(retrieved[3]), 3.14, 0.001)

func test_empty_array() -> void:
	var empty = []
	L.push_variant(empty)

	var retrieved = L.to_array(-1)
	L.pop(1)

	assert_typeof(retrieved, TYPE_ARRAY)
	assert_eq(retrieved.size(), 0)

func test_nested_arrays() -> void:
	var nested = [[1, 2], [3, 4], [5, 6]]
	L.push_variant(nested)
	L.set_global("nested")

	L.get_global("nested")
	var retrieved = L.to_array(-1)
	L.pop(1)

	assert_eq(retrieved.size(), 3)

	var first = retrieved[0]
	assert_typeof(first, TYPE_ARRAY)
	assert_eq(first.size(), 2)
	assert_eq(first[0], 1)
	assert_eq(first[1], 2)

	var third = retrieved[2]
	assert_eq(third[0], 5)
	assert_eq(third[1], 6)

func test_array_manipulation_in_lua() -> void:
	var original = [10, 20, 30]
	L.push_variant(original)
	L.set_global("arr")

	var code: String = """
	table.insert(arr, 40)
	table.insert(arr, 50)
	return arr
	"""

	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result = L.to_array(-1)
	assert_eq(result.size(), 5)
	assert_eq(result[3], 40)
	assert_eq(result[4], 50)

func test_array_creation_in_lua() -> void:
	var code: String = """
	return {100, 200, 300, 400}
	"""

	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var result = L.to_array(-1)
	assert_eq(result.size(), 4)
	assert_eq(result[0], 100)
	assert_eq(result[3], 400)

func test_array_vs_dictionary_detection() -> void:
	# Consecutive integer keys starting at 1 = array
	var code1 = "return {10, 20, 30}"
	var bytecode1 = Luau.compile(code1)
	L.load_bytecode(bytecode1, "test1")
	L.resume()

	assert_true(L.is_array(-1), "Should detect as array")
	assert_false(L.is_dictionary(-1), "Should not be dictionary")
	L.pop(1)

	# Non-consecutive keys = dictionary
	var code2 = "return {[1] = 'a', [3] = 'c'}"
	var bytecode2 = Luau.compile(code2)
	L.load_bytecode(bytecode2, "test2")
	L.resume()

	assert_false(L.is_array(-1), "Missing index 2, should be dictionary")
	assert_true(L.is_dictionary(-1), "Should detect as dictionary")
	L.pop(1)

	# String keys = dictionary
	var code3 = "return {name = 'test', value = 42}"
	var bytecode3 = Luau.compile(code3)
	L.load_bytecode(bytecode3, "test3")
	L.resume()

	assert_false(L.is_array(-1), "String keys = dictionary")
	assert_true(L.is_dictionary(-1))

func test_large_array() -> void:
	var large = []
	for i in range(1000):
		large.append(i)

	L.push_variant(large)
	var retrieved = L.to_array(-1)
	L.pop(1)

	assert_eq(retrieved.size(), 1000)
	assert_eq(retrieved[0], 0)
	assert_eq(retrieved[500], 500)
	assert_eq(retrieved[999], 999)

# ============================================================================
# Dictionary Tests
# ============================================================================

func test_simple_dictionary_round_trip() -> void:
	var original = {"name": "test", "value": 42, "active": true}
	L.push_variant(original)
	L.set_global("dict")

	L.get_global("dict")
	var retrieved = L.to_dictionary(-1)
	L.pop(1)

	assert_typeof(retrieved, TYPE_DICTIONARY)
	assert_true(retrieved.has("name"))
	assert_true(retrieved.has("value"))
	assert_true(retrieved.has("active"))

	assert_eq(retrieved["name"], "test")
	assert_eq(retrieved["value"], 42)
	assert_eq(retrieved["active"], true)

func test_empty_dictionary() -> void:
	var empty = {}
	L.push_variant(empty)

	var retrieved = L.to_dictionary(-1)
	L.pop(1)

	assert_typeof(retrieved, TYPE_DICTIONARY)
	assert_eq(retrieved.size(), 0)

func test_integer_keyed_dictionary() -> void:
	var dict = {1: "one", 2: "two", 100: "hundred"}
	L.push_variant(dict)
	L.set_global("dict")

	L.get_global("dict")
	var retrieved = L.to_dictionary(-1)
	L.pop(1)

	assert_eq(retrieved[1], "one")
	assert_eq(retrieved[2], "two")
	assert_eq(retrieved[100], "hundred")

func test_nested_dictionaries() -> void:
	var nested = {
		"user": {"name": "Bob", "age": 25},
		"settings": {"volume": 80, "fullscreen": true}
	}

	L.push_variant(nested)
	L.set_global("data")

	L.get_global("data")
	var retrieved = L.to_dictionary(-1)
	L.pop(1)

	assert_true(retrieved.has("user"))
	assert_true(retrieved.has("settings"))

	var user = retrieved["user"]
	assert_typeof(user, TYPE_DICTIONARY)
	assert_eq(user["name"], "Bob")
	assert_eq(user["age"], 25)

	var settings = retrieved["settings"]
	assert_eq(settings["volume"], 80)
	assert_eq(settings["fullscreen"], true)

func test_dictionary_from_lua() -> void:
	var code: String = """
	return {
		name = "Alice",
		age = 30,
		active = true
	}
	"""

	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var dict = L.to_dictionary(-1)

	assert_typeof(dict, TYPE_DICTIONARY)
	assert_eq(dict["name"], "Alice")
	assert_eq(dict["age"], 30)
	assert_eq(dict["active"], true)

func test_mixed_key_types_from_lua() -> void:
	var code: String = """
	return {
		name = "test",
		[1] = "first",
		[2] = "second",
		count = 99
	}
	"""

	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var dict = L.to_dictionary(-1)

	assert_true(dict.has("name"))
	assert_true(dict.has(1))
	assert_true(dict.has(2))
	assert_true(dict.has("count"))

func test_large_dictionary() -> void:
	var large = {}
	for i in range(100):
		large["key" + str(i)] = i

	L.push_variant(large)
	var retrieved = L.to_dictionary(-1)
	L.pop(1)

	assert_eq(retrieved.size(), 100)
	assert_eq(retrieved["key0"], 0)
	assert_eq(retrieved["key50"], 50)
	assert_eq(retrieved["key99"], 99)

# ============================================================================
# Mixed Collections Tests
# ============================================================================

func test_dictionary_containing_arrays() -> void:
	var data = {
		"items": [1, 2, 3],
		"count": 3
	}

	L.push_variant(data)
	L.set_global("data")

	var code: String = """
	items = data.items
	first = items[1]
	count = data.count
	return first, count
	"""

	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var count = L.to_integer(-1)
	var first = L.to_integer(-2)

	assert_eq(first, 1)
	assert_eq(count, 3)

func test_array_containing_dictionaries() -> void:
	var items = [
		{"id": 1, "name": "Item A"},
		{"id": 2, "name": "Item B"}
	]

	L.push_variant(items)
	L.set_global("items")

	var code: String = """
	first_item = items[1]
	first_name = first_item.name
	second_id = items[2].id
	return first_name, second_id
	"""

	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	var second_id = L.to_integer(-1)
	var first_name = L.to_string(-2)

	assert_eq(first_name, "Item A")
	assert_eq(second_id, 2)

func test_deeply_nested_structures() -> void:
	var complex = {
		"level1": {
			"level2": {
				"level3": [1, 2, 3]
			}
		}
	}

	L.push_variant(complex)
	var retrieved = L.to_dictionary(-1)
	L.pop(1)

	var level1 = retrieved["level1"]
	var level2 = level1["level2"]
	var level3 = level2["level3"]

	assert_typeof(level3, TYPE_ARRAY)
	assert_eq(level3.size(), 3)
	assert_eq(level3[0], 1)

func test_complex_structure_from_lua() -> void:
	var code: String = """
	return {
		position = Vector2(100, 200),
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

	var result = L.to_dictionary(-1)

	assert_true(result.has("position"))
	assert_true(result.has("items"))
	assert_true(result.has("metadata"))

	var pos = result["position"]
	assert_typeof(pos, TYPE_VECTOR2)
	assert_almost_eq(pos.x, 100.0, 0.001)

	var items = result["items"]
	assert_typeof(items, TYPE_ARRAY)
	assert_eq(items.size(), 3)

	var meta = result["metadata"]
	assert_typeof(meta, TYPE_DICTIONARY)
	assert_eq(meta["name"], "entity")
	assert_eq(meta["active"], true)

# ============================================================================
# Edge Cases
# ============================================================================

func test_special_string_keys() -> void:
	var dict = {
		"with space": 1,
		"with.dot": 2,
		"with-dash": 3,
		"": 4  # Empty string key
	}

	L.push_variant(dict)
	var retrieved = L.to_dictionary(-1)
	L.pop(1)

	assert_eq(retrieved["with space"], 1)
	assert_eq(retrieved["with.dot"], 2)
	assert_eq(retrieved["with-dash"], 3)
	assert_eq(retrieved[""], 4)

func test_table_with_zero_index() -> void:
	var code: String = "return {[0] = 'zero', [1] = 'one'}"

	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	# Should be dictionary (arrays start at 1 in Lua)
	assert_true(L.is_dictionary(-1))
	assert_false(L.is_array(-1))

	var dict = L.to_dictionary(-1)
	assert_eq(dict[0], "zero")
	assert_eq(dict[1], "one")

func test_table_with_negative_indices() -> void:
	var code: String = "return {[-1] = 'neg', [1] = 'pos'}"

	var bytecode: PackedByteArray = Luau.compile(code)
	L.load_bytecode(bytecode, "test")
	L.resume()

	assert_true(L.is_dictionary(-1))

	var dict = L.to_dictionary(-1)
	assert_true(dict.has(-1))
	assert_true(dict.has(1))
	assert_eq(dict[-1], "neg")
	assert_eq(dict[1], "pos")
