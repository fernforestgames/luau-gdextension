// Tests for bridging/variant - Variant bridging between Godot and Lua
// to_variant, push_variant, variant type conversions

#include "doctest.h"
#include "test_fixtures.h"
#include "lua_state.h"

using namespace godot;

TEST_SUITE("Bridging - Variant - Basic Types")
{
    TEST_CASE("push_variant and to_variant - nil")
    {
        LuaStateFixture f;

        f.state->push_variant(Variant());
        CHECK(f.state->is_nil(-1));

        Variant result = f.state->to_variant(-1);
        CHECK(result.get_type() == Variant::NIL);

        f.state->pop(1);
    }

    TEST_CASE("push_variant and to_variant - bool")
    {
        LuaStateFixture f;

        f.state->push_variant(true);
        CHECK(f.state->is_boolean(-1));
        CHECK(f.state->to_boolean(-1) == true);

        Variant result = f.state->to_variant(-1);
        CHECK(result.get_type() == Variant::BOOL);
        CHECK(static_cast<bool>(result) == true);

        f.state->pop(1);

        f.state->push_variant(false);
        result = f.state->to_variant(-1);
        CHECK(static_cast<bool>(result) == false);

        f.state->pop(1);
    }

    TEST_CASE("push_variant and to_variant - int")
    {
        LuaStateFixture f;

        f.state->push_variant(42);
        CHECK(f.state->is_number(-1));

        Variant result = f.state->to_variant(-1);
        CHECK(result.get_type() == Variant::INT);
        CHECK(static_cast<int>(result) == 42);

        f.state->pop(1);

        f.state->push_variant(-99);
        result = f.state->to_variant(-1);
        CHECK(static_cast<int>(result) == -99);

        f.state->pop(1);
    }

    TEST_CASE("push_variant and to_variant - float")
    {
        LuaStateFixture f;

        f.state->push_variant(3.14159);
        CHECK(f.state->is_number(-1));

        Variant result = f.state->to_variant(-1);
        CHECK(result.get_type() == Variant::FLOAT);
        CHECK(static_cast<double>(result) == doctest::Approx(3.14159));

        f.state->pop(1);
    }

    TEST_CASE("push_variant and to_variant - string")
    {
        LuaStateFixture f;

        f.state->push_variant("Hello, World!");
        CHECK(f.state->is_string(-1));

        Variant result = f.state->to_variant(-1);
        CHECK(result.get_type() == Variant::STRING);
        CHECK(static_cast<String>(result) == "Hello, World!");

        f.state->pop(1);

        // Empty string
        f.state->push_variant("");
        result = f.state->to_variant(-1);
        CHECK(static_cast<String>(result) == "");

        f.state->pop(1);
    }

    TEST_CASE("push_variant and to_variant - Vector3")
    {
        LuaStateFixture f;

        Vector3 v(1.5, 2.5, 3.5);
        f.state->push_variant(v);
        CHECK(f.state->is_vector(-1));

        Variant result = f.state->to_variant(-1);
        CHECK(result.get_type() == Variant::VECTOR3);
        Vector3 result_v = result;
        CHECK(result_v.x == 1.5);
        CHECK(result_v.y == 2.5);
        CHECK(result_v.z == 3.5);

        f.state->pop(1);
    }

    TEST_CASE("push_variant and to_variant - Array")
    {
        LuaStateFixture f;

        Array arr;
        arr.push_back(1);
        arr.push_back("test");
        arr.push_back(true);

        f.state->push_variant(arr);
        CHECK(f.state->is_table(-1));

        Variant result = f.state->to_variant(-1);
        CHECK(result.get_type() == Variant::ARRAY);
        Array result_arr = result;
        CHECK(result_arr.size() == 3);
        CHECK(static_cast<int>(result_arr[0]) == 1);
        CHECK(static_cast<String>(result_arr[1]) == "test");
        CHECK(static_cast<bool>(result_arr[2]) == true);

        f.state->pop(1);
    }

    TEST_CASE("push_variant and to_variant - Dictionary")
    {
        LuaStateFixture f;

        Dictionary dict;
        dict["name"] = "Alice";
        dict["age"] = 30;

        f.state->push_variant(dict);
        CHECK(f.state->is_table(-1));

        Variant result = f.state->to_variant(-1);
        CHECK(result.get_type() == Variant::DICTIONARY);
        Dictionary result_dict = result;
        CHECK(result_dict.size() == 2);
        CHECK(static_cast<String>(result_dict["name"]) == "Alice");
        CHECK(static_cast<int>(result_dict["age"]) == 30);

        f.state->pop(1);
    }
}

TEST_SUITE("Bridging - Variant - Type Conversions")
{
    TEST_CASE("to_variant - from Lua number")
    {
        LuaStateFixture f;

        f.state->push_number(42.0);
        Variant result = f.state->to_variant(-1);

        // Lua numbers become floats in Variant
        CHECK(result.get_type() == Variant::FLOAT);
        CHECK(static_cast<double>(result) == 42.0);

        f.state->pop(1);
    }

    TEST_CASE("to_variant - from Lua string")
    {
        LuaStateFixture f;

        f.state->push_string("test");
        Variant result = f.state->to_variant(-1);

        CHECK(result.get_type() == Variant::STRING);
        CHECK(static_cast<String>(result) == "test");

        f.state->pop(1);
    }

    TEST_CASE("to_variant - from Lua boolean")
    {
        LuaStateFixture f;

        f.state->push_boolean(true);
        Variant result = f.state->to_variant(-1);

        CHECK(result.get_type() == Variant::BOOL);
        CHECK(static_cast<bool>(result) == true);

        f.state->pop(1);
    }

    TEST_CASE("to_variant - from Lua nil")
    {
        LuaStateFixture f;

        f.state->push_nil();
        Variant result = f.state->to_variant(-1);

        CHECK(result.get_type() == Variant::NIL);

        f.state->pop(1);
    }

    TEST_CASE("to_variant - from Lua vector")
    {
        LuaStateFixture f;

        f.state->push_vector3(Vector3(1, 2, 3));
        Variant result = f.state->to_variant(-1);

        CHECK(result.get_type() == Variant::VECTOR3);
        Vector3 v = result;
        CHECK(v.x == 1.0);
        CHECK(v.y == 2.0);
        CHECK(v.z == 3.0);

        f.state->pop(1);
    }

    TEST_CASE("to_variant - from Lua table (array)")
    {
        LuaStateFixture f;

        f.state->create_table();
        f.state->push_number(1.0);
        f.state->raw_seti(-2, 1);
        f.state->push_number(2.0);
        f.state->raw_seti(-2, 2);

        Variant result = f.state->to_variant(-1);

        CHECK(result.get_type() == Variant::ARRAY);
        Array arr = result;
        CHECK(arr.size() == 2);

        f.state->pop(1);
    }

    TEST_CASE("to_variant - from Lua table (dictionary)")
    {
        LuaStateFixture f;

        f.state->create_table();
        f.state->push_string("value");
        f.state->set_field(-2, "key");

        Variant result = f.state->to_variant(-1);

        CHECK(result.get_type() == Variant::DICTIONARY);
        Dictionary dict = result;
        CHECK(dict.size() == 1);

        f.state->pop(1);
    }
}

TEST_SUITE("Bridging - Variant - Complex Types")
{
    TEST_CASE("round-trip - nested arrays")
    {
        LuaStateFixture f;

        Array inner;
        inner.push_back(1);
        inner.push_back(2);

        Array outer;
        outer.push_back(inner);
        outer.push_back("test");

        Variant original(outer);
        f.state->push_variant(original);
        Variant result = f.state->to_variant(-1);

        CHECK(result.get_type() == Variant::ARRAY);
        Array result_arr = result;
        CHECK(result_arr.size() == 2);

        Array result_inner = result_arr[0];
        CHECK(result_inner.size() == 2);
        CHECK(static_cast<int>(result_inner[0]) == 1);
        CHECK(static_cast<int>(result_inner[1]) == 2);

        CHECK(static_cast<String>(result_arr[1]) == "test");

        f.state->pop(1);
    }

    TEST_CASE("round-trip - nested dictionaries")
    {
        LuaStateFixture f;

        Dictionary inner;
        inner["x"] = 10;
        inner["y"] = 20;

        Dictionary outer;
        outer["position"] = inner;
        outer["name"] = "test";

        Variant original(outer);
        f.state->push_variant(original);
        Variant result = f.state->to_variant(-1);

        CHECK(result.get_type() == Variant::DICTIONARY);
        Dictionary result_dict = result;
        CHECK(result_dict.size() == 2);

        Dictionary result_inner = result_dict["position"];
        CHECK(static_cast<int>(result_inner["x"]) == 10);
        CHECK(static_cast<int>(result_inner["y"]) == 20);

        CHECK(static_cast<String>(result_dict["name"]) == "test");

        f.state->pop(1);
    }

    TEST_CASE("round-trip - mixed array and dictionary")
    {
        LuaStateFixture f;

        Dictionary dict;
        dict["key"] = "value";

        Array arr;
        arr.push_back(dict);
        arr.push_back(42);

        Variant original(arr);
        f.state->push_variant(original);
        Variant result = f.state->to_variant(-1);

        CHECK(result.get_type() == Variant::ARRAY);
        Array result_arr = result;
        CHECK(result_arr.size() == 2);

        Dictionary result_dict = result_arr[0];
        CHECK(static_cast<String>(result_dict["key"]) == "value");

        CHECK(static_cast<int>(result_arr[1]) == 42);

        f.state->pop(1);
    }
}

TEST_SUITE("Bridging - Variant - Integration")
{
    TEST_CASE("integration - passing variants to Lua")
    {
        LuaStateFixture f;

        f.exec_lua_ok(R"(
            function process_variant(v)
                return type(v)
            end
        )");

        // Test different variant types
        f.state->get_global("process_variant");
        f.state->push_variant(42);
        f.state->call(1, 1);
        CHECK(f.state->to_string_inplace(-1) == "number");
        f.state->pop(1);

        f.state->get_global("process_variant");
        f.state->push_variant("test");
        f.state->call(1, 1);
        CHECK(f.state->to_string_inplace(-1) == "string");
        f.state->pop(1);

        f.state->get_global("process_variant");
        Array arr;
        arr.push_back(1);
        f.state->push_variant(arr);
        f.state->call(1, 1);
        CHECK(f.state->to_string_inplace(-1) == "table");
        f.state->pop(1);
    }

    TEST_CASE("integration - receiving variants from Lua")
    {
        LuaStateFixture f;

        f.exec_lua_ok("return 42");
        Variant result = f.state->to_variant(-1);
        CHECK(static_cast<double>(result) == 42.0);
        f.state->pop(1);

        f.exec_lua_ok("return 'hello'");
        result = f.state->to_variant(-1);
        CHECK(static_cast<String>(result) == "hello");
        f.state->pop(1);

        f.exec_lua_ok("return {1, 2, 3}");
        result = f.state->to_variant(-1);
        CHECK(result.get_type() == Variant::ARRAY);
        f.state->pop(1);

        f.exec_lua_ok("return {key = 'value'}");
        result = f.state->to_variant(-1);
        CHECK(result.get_type() == Variant::DICTIONARY);
        f.state->pop(1);
    }

    TEST_CASE("integration - variant operations in Lua")
    {
        LuaStateFixture f;

        // Create variant and manipulate in Lua
        f.exec_lua_ok(R"(
            function double_value(v)
                return v * 2
            end
        )");

        f.state->get_global("double_value");
        f.state->push_variant(21);
        f.state->call(1, 1);

        Variant result = f.state->to_variant(-1);
        CHECK(static_cast<double>(result) == 42.0);

        f.state->pop(1);
    }

    TEST_CASE("integration - table to array vs dictionary heuristic")
    {
        LuaStateFixture f;

        // Sequential integer keys from 1 -> Array
        f.exec_lua_ok("return {10, 20, 30}");
        Variant result = f.state->to_variant(-1);
        CHECK(result.get_type() == Variant::ARRAY);
        f.state->pop(1);

        // String keys -> Dictionary
        f.exec_lua_ok("return {a = 1, b = 2}");
        result = f.state->to_variant(-1);
        CHECK(result.get_type() == Variant::DICTIONARY);
        f.state->pop(1);

        // Mixed keys -> Dictionary
        f.exec_lua_ok("return {1, 2, key = 'value'}");
        result = f.state->to_variant(-1);
        CHECK(result.get_type() == Variant::DICTIONARY);
        f.state->pop(1);

        // Non-sequential integer keys -> Dictionary
        f.exec_lua_ok("return {[1] = 'a', [3] = 'c'}");
        result = f.state->to_variant(-1);
        CHECK(result.get_type() == Variant::DICTIONARY);
        f.state->pop(1);

        // Empty table -> Array (by convention)
        f.exec_lua_ok("return {}");
        result = f.state->to_variant(-1);
        CHECK(result.get_type() == Variant::ARRAY);
        f.state->pop(1);
    }
}

TEST_SUITE("Bridging - Variant - Edge Cases")
{
    TEST_CASE("variant - large numbers")
    {
        LuaStateFixture f;

        double large = 9007199254740992.0; // 2^53, max safe integer in double
        f.state->push_variant(large);
        Variant result = f.state->to_variant(-1);

        CHECK(static_cast<double>(result) == large);

        f.state->pop(1);
    }

    TEST_CASE("variant - special float values")
    {
        LuaStateFixture f;

        // Note: NaN and Inf may have special handling in Luau
        // Test what actually happens

        f.state->push_variant(0.0);
        Variant result = f.state->to_variant(-1);
        CHECK(static_cast<double>(result) == 0.0);
        f.state->pop(1);

        f.state->push_variant(-0.0);
        result = f.state->to_variant(-1);
        // -0.0 should be preserved
        f.state->pop(1);
    }

    TEST_CASE("variant - empty containers")
    {
        LuaStateFixture f;

        Array empty_array;
        f.state->push_variant(empty_array);
        Variant result = f.state->to_variant(-1);
        CHECK(result.get_type() == Variant::ARRAY);
        Array arr = result;
        CHECK(arr.size() == 0);
        f.state->pop(1);

        Dictionary empty_dict;
        f.state->push_variant(empty_dict);
        result = f.state->to_variant(-1);
        CHECK(result.get_type() == Variant::DICTIONARY);
        Dictionary dict = result;
        CHECK(dict.size() == 0);
        f.state->pop(1);
    }

    TEST_CASE("variant - deeply nested structures")
    {
        LuaStateFixture f;

        // Create a deeply nested array
        Array level3;
        level3.push_back(42);

        Array level2;
        level2.push_back(level3);

        Array level1;
        level1.push_back(level2);

        f.state->push_variant(level1);
        Variant result = f.state->to_variant(-1);

        CHECK(result.get_type() == Variant::ARRAY);
        Array r1 = result;
        Array r2 = r1[0];
        Array r3 = r2[0];
        CHECK(static_cast<int>(r3[0]) == 42);

        f.state->pop(1);
    }

    TEST_CASE("variant - unicode strings")
    {
        LuaStateFixture f;

        String unicode = "Hello 世界 🌍";
        f.state->push_variant(unicode);
        Variant result = f.state->to_variant(-1);

        CHECK(result.get_type() == Variant::STRING);
        CHECK(static_cast<String>(result) == unicode);

        f.state->pop(1);
    }
}
