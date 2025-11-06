// Tests for bridging/variant - Variant bridging between Godot and Lua
// to_variant, push_variant, variant type conversions

#include "doctest.h"
#include "test_fixtures.h"
#include "lua_state.h"

using namespace godot;

TEST_SUITE("Bridging - Variant - Basic Types")
{
    TEST_CASE_FIXTURE(LuaStateFixture, "push_variant and to_variant - nil")
    {
        state->push_variant(Variant());
        CHECK(state->is_nil(-1));

        Variant result = state->to_variant(-1);
        CHECK(result.get_type() == Variant::NIL);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_variant and to_variant - bool")
    {
        state->push_variant(true);
        CHECK(state->is_boolean(-1));
        CHECK(state->to_boolean(-1) == true);

        Variant result = state->to_variant(-1);
        CHECK(result.get_type() == Variant::BOOL);
        CHECK(static_cast<bool>(result) == true);

        state->pop(1);

        state->push_variant(false);
        result = state->to_variant(-1);
        CHECK(static_cast<bool>(result) == false);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_variant and to_variant - int")
    {
        state->push_variant(42);
        CHECK(state->is_number(-1));

        Variant result = state->to_variant(-1);
        CHECK(result.get_type() == Variant::INT);
        CHECK(static_cast<int>(result) == 42);

        state->pop(1);

        state->push_variant(-99);
        result = state->to_variant(-1);
        CHECK(static_cast<int>(result) == -99);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_variant and to_variant - float")
    {
        state->push_variant(3.14159);
        CHECK(state->is_number(-1));

        Variant result = state->to_variant(-1);
        CHECK(result.get_type() == Variant::FLOAT);
        CHECK(static_cast<double>(result) == doctest::Approx(3.14159));

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_variant and to_variant - string")
    {
        state->push_variant("Hello, World!");
        CHECK(state->is_string(-1));

        Variant result = state->to_variant(-1);
        CHECK(result.get_type() == Variant::STRING);
        CHECK(static_cast<String>(result) == "Hello, World!");

        state->pop(1);

        // Empty string
        state->push_variant("");
        result = state->to_variant(-1);
        CHECK(static_cast<String>(result) == "");

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_variant and to_variant - Vector3")
    {
        Vector3 v(1.5, 2.5, 3.5);
        state->push_variant(v);
        CHECK(state->is_vector(-1));

        Variant result = state->to_variant(-1);
        CHECK(result.get_type() == Variant::VECTOR3);
        Vector3 result_v = result;
        CHECK(result_v.x == 1.5);
        CHECK(result_v.y == 2.5);
        CHECK(result_v.z == 3.5);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_variant and to_variant - Array")
    {
        Array arr;
        arr.push_back(1);
        arr.push_back("test");
        arr.push_back(true);

        state->push_variant(arr);
        CHECK(state->is_table(-1));

        Variant result = state->to_variant(-1);
        CHECK(result.get_type() == Variant::ARRAY);
        Array result_arr = result;
        CHECK(result_arr.size() == 3);
        CHECK(static_cast<int>(result_arr[0]) == 1);
        CHECK(static_cast<String>(result_arr[1]) == "test");
        CHECK(static_cast<bool>(result_arr[2]) == true);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_variant and to_variant - Dictionary")
    {
        Dictionary dict;
        dict["name"] = "Alice";
        dict["age"] = 30;

        state->push_variant(dict);
        CHECK(state->is_table(-1));

        Variant result = state->to_variant(-1);
        CHECK(result.get_type() == Variant::DICTIONARY);
        Dictionary result_dict = result;
        CHECK(result_dict.size() == 2);
        CHECK(static_cast<String>(result_dict["name"]) == "Alice");
        CHECK(static_cast<int>(result_dict["age"]) == 30);

        state->pop(1);
    }
}

TEST_SUITE("Bridging - Variant - Type Conversions")
{
    TEST_CASE_FIXTURE(LuaStateFixture, "to_variant - from Lua number")
    {
        state->push_number(42.5);
        Variant result = state->to_variant(-1);

        CHECK(result.get_type() == Variant::FLOAT);
        CHECK(static_cast<double>(result) == 42.5);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_variant - from Lua integer")
    {
        state->push_integer(42);
        Variant result = state->to_variant(-1);

        CHECK(result.get_type() == Variant::INT);
        CHECK(static_cast<int>(result) == 42);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_variant - from Lua string")
    {
        state->push_string("test");
        Variant result = state->to_variant(-1);

        CHECK(result.get_type() == Variant::STRING);
        CHECK(static_cast<String>(result) == "test");

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_variant - from Lua boolean")
    {
        state->push_boolean(true);
        Variant result = state->to_variant(-1);

        CHECK(result.get_type() == Variant::BOOL);
        CHECK(static_cast<bool>(result) == true);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_variant - from Lua nil")
    {
        state->push_nil();
        Variant result = state->to_variant(-1);

        CHECK(result.get_type() == Variant::NIL);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_variant - from Lua vector")
    {
        state->push_vector3(Vector3(1, 2, 3));
        Variant result = state->to_variant(-1);

        CHECK(result.get_type() == Variant::VECTOR3);
        Vector3 v = result;
        CHECK(v.x == 1.0);
        CHECK(v.y == 2.0);
        CHECK(v.z == 3.0);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_variant - from Lua table (array)")
    {
        state->create_table();
        state->push_number(1.0);
        state->raw_seti(-2, 1);
        state->push_number(2.0);
        state->raw_seti(-2, 2);

        Variant result = state->to_variant(-1);

        CHECK(result.get_type() == Variant::ARRAY);
        Array arr = result;
        CHECK(arr.size() == 2);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_variant - from Lua table (dictionary)")
    {
        state->create_table();
        state->push_string("value");
        state->set_field(-2, "key");

        Variant result = state->to_variant(-1);

        CHECK(result.get_type() == Variant::DICTIONARY);
        Dictionary dict = result;
        CHECK(dict.size() == 1);

        state->pop(1);
    }
}

TEST_SUITE("Bridging - Variant - Complex Types")
{
    TEST_CASE_FIXTURE(LuaStateFixture, "round-trip - nested arrays")
    {
        Array inner;
        inner.push_back(1);
        inner.push_back(2);

        Array outer;
        outer.push_back(inner);
        outer.push_back("test");

        Variant original(outer);
        state->push_variant(original);
        Variant result = state->to_variant(-1);

        CHECK(result.get_type() == Variant::ARRAY);
        Array result_arr = result;
        CHECK(result_arr.size() == 2);

        Array result_inner = result_arr[0];
        CHECK(result_inner.size() == 2);
        CHECK(static_cast<int>(result_inner[0]) == 1);
        CHECK(static_cast<int>(result_inner[1]) == 2);

        CHECK(static_cast<String>(result_arr[1]) == "test");

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "round-trip - nested dictionaries")
    {
        Dictionary inner;
        inner["x"] = 10;
        inner["y"] = 20;

        Dictionary outer;
        outer["position"] = inner;
        outer["name"] = "test";

        Variant original(outer);
        state->push_variant(original);
        Variant result = state->to_variant(-1);

        CHECK(result.get_type() == Variant::DICTIONARY);
        Dictionary result_dict = result;
        CHECK(result_dict.size() == 2);

        Dictionary result_inner = result_dict["position"];
        CHECK(static_cast<int>(result_inner["x"]) == 10);
        CHECK(static_cast<int>(result_inner["y"]) == 20);

        CHECK(static_cast<String>(result_dict["name"]) == "test");

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "round-trip - mixed array and dictionary")
    {
        Dictionary dict;
        dict["key"] = "value";

        Array arr;
        arr.push_back(dict);
        arr.push_back(42);

        Variant original(arr);
        state->push_variant(original);
        Variant result = state->to_variant(-1);

        CHECK(result.get_type() == Variant::ARRAY);
        Array result_arr = result;
        CHECK(result_arr.size() == 2);

        Dictionary result_dict = result_arr[0];
        CHECK(static_cast<String>(result_dict["key"]) == "value");

        CHECK(static_cast<int>(result_arr[1]) == 42);

        state->pop(1);
    }
}

TEST_SUITE("Bridging - Variant - Integration")
{
    TEST_CASE_FIXTURE(LuaStateFixture, "integration - passing variants to Lua")
    {
        exec_lua_ok(R"(
            function process_variant(v)
                return type(v)
            end
        )");

        // Test different variant types
        state->get_global("process_variant");
        state->push_variant(42);
        state->call(1, 1);
        CHECK(state->to_string_inplace(-1) == "number");
        state->pop(1);

        state->get_global("process_variant");
        state->push_variant("test");
        state->call(1, 1);
        CHECK(state->to_string_inplace(-1) == "string");
        state->pop(1);

        state->get_global("process_variant");
        Array arr;
        arr.push_back(1);
        state->push_variant(arr);
        state->call(1, 1);
        CHECK(state->to_string_inplace(-1) == "table");
        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "integration - receiving variants from Lua")
    {
        exec_lua_ok("return 42");
        Variant result = state->to_variant(-1);
        CHECK(static_cast<double>(result) == 42.0);
        state->pop(1);

        exec_lua_ok("return 'hello'");
        result = state->to_variant(-1);
        CHECK(static_cast<String>(result) == "hello");
        state->pop(1);

        exec_lua_ok("return {1, 2, 3}");
        result = state->to_variant(-1);
        CHECK(result.get_type() == Variant::ARRAY);
        state->pop(1);

        exec_lua_ok("return {key = 'value'}");
        result = state->to_variant(-1);
        CHECK(result.get_type() == Variant::DICTIONARY);
        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "integration - variant operations in Lua")
    {
        // Create variant and manipulate in Lua
        exec_lua_ok(R"(
            function double_value(v)
                return v * 2
            end
        )");

        state->get_global("double_value");
        state->push_variant(21);
        state->call(1, 1);

        Variant result = state->to_variant(-1);
        CHECK(static_cast<double>(result) == 42.0);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "integration - table to array vs dictionary heuristic")
    {
        // Sequential integer keys from 1 -> Array
        exec_lua_ok("return {10, 20, 30}");
        Variant result = state->to_variant(-1);
        CHECK(result.get_type() == Variant::ARRAY);
        state->pop(1);

        // String keys -> Dictionary
        exec_lua_ok("return {a = 1, b = 2}");
        result = state->to_variant(-1);
        CHECK(result.get_type() == Variant::DICTIONARY);
        state->pop(1);

        // Mixed keys -> Dictionary
        exec_lua_ok("return {1, 2, key = 'value'}");
        result = state->to_variant(-1);
        CHECK(result.get_type() == Variant::DICTIONARY);
        state->pop(1);

        // Non-sequential integer keys -> Dictionary
        exec_lua_ok("return {[1] = 'a', [3] = 'c'}");
        result = state->to_variant(-1);
        CHECK(result.get_type() == Variant::DICTIONARY);
        state->pop(1);

        // Empty table -> Array (by convention)
        exec_lua_ok("return {}");
        result = state->to_variant(-1);
        CHECK(result.get_type() == Variant::ARRAY);
        state->pop(1);
    }
}

TEST_SUITE("Bridging - Variant - Edge Cases")
{
    TEST_CASE_FIXTURE(LuaStateFixture, "variant - large numbers")
    {
        double large = 9007199254740992.0; // 2^53, max safe integer in double
        state->push_variant(large);
        Variant result = state->to_variant(-1);

        CHECK(static_cast<double>(result) == large);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "variant - special float values")
    {
        // Note: NaN and Inf may have special handling in Luau
        // Test what actually happens

        state->push_variant(0.0);
        Variant result = state->to_variant(-1);
        CHECK(static_cast<double>(result) == 0.0);
        state->pop(1);

        state->push_variant(-0.0);
        result = state->to_variant(-1);
        // -0.0 should be preserved
        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "variant - empty containers")
    {
        Array empty_array;
        state->push_variant(empty_array);
        Variant result = state->to_variant(-1);
        Array arr = result;
        CHECK(arr.size() == 0);
        state->pop(1);

        Dictionary empty_dict;
        state->push_variant(empty_dict);
        result = state->to_variant(-1);
        Dictionary dict = result;
        CHECK(dict.size() == 0);
        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "variant - deeply nested structures")
    {
        // Create a deeply nested array
        Array level3;
        level3.push_back(42);

        Array level2;
        level2.push_back(level3);

        Array level1;
        level1.push_back(level2);

        state->push_variant(level1);
        Variant result = state->to_variant(-1);

        CHECK(result.get_type() == Variant::ARRAY);
        Array r1 = result;
        Array r2 = r1[0];
        Array r3 = r2[0];
        CHECK(static_cast<int>(r3[0]) == 42);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "variant - unicode strings")
    {
        String unicode = "Hello 世界 🌍";
        state->push_variant(unicode);
        Variant result = state->to_variant(-1);

        CHECK(result.get_type() == Variant::STRING);
        CHECK(static_cast<String>(result) == unicode);

        state->pop(1);
    }
}
