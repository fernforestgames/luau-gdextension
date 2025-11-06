// Tests for bridging/dictionary - Dictionary bridging between Godot and Lua
// to_dictionary, push_dictionary

#include "doctest.h"
#include "test_fixtures.h"
#include "lua_state.h"

using namespace godot;

TEST_SUITE("Bridging - Dictionary")
{
    TEST_CASE("push_dictionary - empty dictionary")
    {
        LuaStateFixture f;

        Dictionary dict;
        f.state->push_dictionary(dict);

        CHECK(f.state->is_table(-1));
        CHECK(f.state->obj_len(-1) == 0);

        f.state->pop(1);
    }

    TEST_CASE("push_dictionary - string keys")
    {
        LuaStateFixture f;

        Dictionary dict;
        dict["name"] = "Alice";
        dict["age"] = 30;
        dict["active"] = true;

        f.state->push_dictionary(dict);

        CHECK(f.state->is_table(-1));

        // Check values
        f.state->get_field(-1, "name");
        CHECK(f.state->to_string_inplace(-1) == "Alice");
        f.state->pop(1);

        f.state->get_field(-1, "age");
        CHECK(f.state->to_number(-1) == 30.0);
        f.state->pop(1);

        f.state->get_field(-1, "active");
        CHECK(f.state->to_boolean(-1) == true);
        f.state->pop(1);

        f.state->pop(1); // Pop table
    }

    TEST_CASE("push_dictionary - numeric keys")
    {
        LuaStateFixture f;

        Dictionary dict;
        dict[1] = "one";
        dict[2] = "two";
        dict[10] = "ten";

        f.state->push_dictionary(dict);

        // Check values
        f.state->raw_geti(-1, 1);
        CHECK(f.state->to_string_inplace(-1) == "one");
        f.state->pop(1);

        f.state->raw_geti(-1, 2);
        CHECK(f.state->to_string_inplace(-1) == "two");
        f.state->pop(1);

        f.state->raw_geti(-1, 10);
        CHECK(f.state->to_string_inplace(-1) == "ten");
        f.state->pop(1);

        f.state->pop(1); // Pop table
    }

    TEST_CASE("push_dictionary - mixed key types")
    {
        LuaStateFixture f;

        Dictionary dict;
        dict["string_key"] = 42;
        dict[123] = "numeric_key";
        dict[true] = "bool_key";

        f.state->push_dictionary(dict);

        f.state->get_field(-1, "string_key");
        CHECK(f.state->to_number(-1) == 42.0);
        f.state->pop(1);

        f.state->raw_geti(-1, 123);
        CHECK(f.state->to_string_inplace(-1) == "numeric_key");
        f.state->pop(1);

        f.state->push_boolean(true);
        f.state->get_table(-2);
        CHECK(f.state->to_string_inplace(-1) == "bool_key");
        f.state->pop(1);

        f.state->pop(1); // Pop table
    }

    TEST_CASE("push_dictionary - nested dictionaries")
    {
        LuaStateFixture f;

        Dictionary inner;
        inner["x"] = 10;
        inner["y"] = 20;

        Dictionary outer;
        outer["position"] = inner;
        outer["name"] = "test";

        f.state->push_dictionary(outer);

        // Check outer
        f.state->get_field(-1, "name");
        CHECK(f.state->to_string_inplace(-1) == "test");
        f.state->pop(1);

        // Check inner
        f.state->get_field(-1, "position");
        CHECK(f.state->type(-1) == LUA_TTABLE);

        f.state->get_field(-1, "x");
        CHECK(f.state->to_number(-1) == 10.0);
        f.state->pop(1);

        f.state->get_field(-1, "y");
        CHECK(f.state->to_number(-1) == 20.0);
        f.state->pop(1);

        f.state->pop(1); // Pop inner table

        f.state->pop(1); // Pop outer table
    }

    TEST_CASE("to_dictionary - empty table")
    {
        LuaStateFixture f;

        f.state->create_table();

        Dictionary result = f.state->to_dictionary(-1);

        CHECK(result.size() == 0);

        f.state->pop(1);
    }

    TEST_CASE("to_dictionary - string keys")
    {
        LuaStateFixture f;

        f.state->create_table();

        f.state->push_string("Alice");
        f.state->set_field(-2, "name");

        f.state->push_number(30.0);
        f.state->set_field(-2, "age");

        Dictionary result = f.state->to_dictionary(-1);

        CHECK(result.size() == 2);
        CHECK(static_cast<String>(result["name"]) == "Alice");
        CHECK(static_cast<int>(result["age"]) == 30);

        f.state->pop(1);
    }

    TEST_CASE("to_dictionary - numeric keys")
    {
        LuaStateFixture f;

        f.state->create_table();

        f.state->push_string("one");
        f.state->raw_seti(-2, 1);

        f.state->push_string("five");
        f.state->raw_seti(-2, 5);

        Dictionary result = f.state->to_dictionary(-1);

        CHECK(result.size() == 2);
        CHECK(static_cast<String>(result[1]) == "one");
        CHECK(static_cast<String>(result[5]) == "five");

        f.state->pop(1);
    }

    TEST_CASE("to_dictionary - mixed key types")
    {
        LuaStateFixture f;

        f.state->create_table();

        f.state->push_number(42.0);
        f.state->set_field(-2, "str_key");

        f.state->push_string("test");
        f.state->raw_seti(-2, 99);

        f.state->push_boolean(true);
        f.state->push_string("bool_val");
        f.state->set_table(-3);

        Dictionary result = f.state->to_dictionary(-1);

        CHECK(result.size() == 3);
        CHECK(static_cast<int>(result["str_key"]) == 42);
        CHECK(static_cast<String>(result[99]) == "test");
        CHECK(static_cast<String>(result[true]) == "bool_val");

        f.state->pop(1);
    }

    TEST_CASE("to_dictionary - nested tables")
    {
        LuaStateFixture f;

        // Create nested structure
        f.state->create_table(); // outer

        f.state->create_table(); // inner
        f.state->push_number(10.0);
        f.state->set_field(-2, "x");
        f.state->push_number(20.0);
        f.state->set_field(-2, "y");
        f.state->set_field(-2, "position"); // outer.position = inner

        f.state->push_string("test");
        f.state->set_field(-2, "name"); // outer.name = "test"

        Dictionary result = f.state->to_dictionary(-1);

        CHECK(result.size() == 2);
        CHECK(static_cast<String>(result["name"]) == "test");
        CHECK(result["position"].get_type() == Variant::DICTIONARY);

        Dictionary inner = result["position"];
        CHECK(inner.size() == 2);
        CHECK(static_cast<int>(inner["x"]) == 10);
        CHECK(static_cast<int>(inner["y"]) == 20);

        f.state->pop(1);
    }

    TEST_CASE("to_dictionary - array vs dictionary distinction")
    {
        LuaStateFixture f;

        // Pure array (sequential integer keys from 1)
        f.state->create_table();
        f.state->push_number(1.0);
        f.state->raw_seti(-2, 1);
        f.state->push_number(2.0);
        f.state->raw_seti(-2, 2);

        Dictionary dict_result = f.state->to_dictionary(-1);
        // Dictionary conversion always works, even for arrays
        CHECK(dict_result.size() == 2);

        f.state->pop(1);

        // Table with gap (not a pure array)
        f.state->create_table();
        f.state->push_number(1.0);
        f.state->raw_seti(-2, 1);
        f.state->push_number(3.0);
        f.state->raw_seti(-2, 3); // Gap at 2

        dict_result = f.state->to_dictionary(-1);
        CHECK(dict_result.size() == 2);
        CHECK(static_cast<int>(dict_result[1]) == 1);
        CHECK(static_cast<int>(dict_result[3]) == 3);

        f.state->pop(1);
    }

    // Note: to_dictionary in LuaState doesn't have a success flag parameter
    // It's only in the bridging layer function. Skipping this test for now.

    TEST_CASE("to_dictionary - not a table")
    {
        LuaStateFixture f;

        f.state->push_number(42.0);

        Dictionary result = f.state->to_dictionary(-1);

        CHECK(result.size() == 0);

        f.state->pop(1);
    }

    TEST_CASE("round-trip - dictionary to Lua and back")
    {
        LuaStateFixture f;

        Dictionary original;
        original["name"] = "Bob";
        original["age"] = 25;
        original["score"] = 98.5;
        original["active"] = true;

        f.state->push_dictionary(original);
        Dictionary result = f.state->to_dictionary(-1);

        CHECK(result.size() == original.size());
        CHECK(result["name"] == original["name"]);
        CHECK(result["age"] == original["age"]);
        CHECK(result["score"] == original["score"]);
        CHECK(result["active"] == original["active"]);

        f.state->pop(1);
    }

    TEST_CASE("round-trip - nested dictionaries")
    {
        LuaStateFixture f;

        Dictionary inner;
        inner["x"] = 100;
        inner["y"] = 200;

        Dictionary outer;
        outer["position"] = inner;
        outer["name"] = "player";

        f.state->push_dictionary(outer);
        Dictionary result = f.state->to_dictionary(-1);

        CHECK(result.size() == 2);
        CHECK(static_cast<String>(result["name"]) == "player");

        Dictionary result_inner = result["position"];
        CHECK(result_inner.size() == 2);
        CHECK(static_cast<int>(result_inner["x"]) == 100);
        CHECK(static_cast<int>(result_inner["y"]) == 200);

        f.state->pop(1);
    }

    TEST_CASE("integration - Lua script manipulation")
    {
        LuaStateFixture f;

        f.exec_lua_ok(R"(
            test_dict = {
                name = "Test",
                count = 42,
                enabled = true
            }
            return test_dict
        )");

        Dictionary result = f.state->to_dictionary(-1);

        CHECK(result.size() == 3);
        CHECK(static_cast<String>(result["name"]) == "Test");
        CHECK(static_cast<int>(result["count"]) == 42);
        CHECK(static_cast<bool>(result["enabled"]) == true);

        f.state->pop(1);
    }

    TEST_CASE("integration - passing dictionary to Lua function")
    {
        LuaStateFixture f;

        f.exec_lua_ok(R"(
            function process_dict(dict)
                local result = {}
                for k, v in pairs(dict) do
                    result[k] = v
                end
                result.processed = true
                return result
            end
        )");

        Dictionary input;
        input["a"] = 1;
        input["b"] = 2;

        f.state->get_global("process_dict");
        f.state->push_dictionary(input);
        f.state->call(1, 1);

        Dictionary result = f.state->to_dictionary(-1);

        CHECK(result.size() == 3);
        CHECK(static_cast<int>(result["a"]) == 1);
        CHECK(static_cast<int>(result["b"]) == 2);
        CHECK(static_cast<bool>(result["processed"]) == true);

        f.state->pop(1);
    }

    TEST_CASE("integration - table with both array and hash parts")
    {
        LuaStateFixture f;

        f.exec_lua_ok(R"(
            mixed_table = {
                1, 2, 3,  -- Array part
                key = "value"  -- Hash part
            }
            return mixed_table
        )");

        // Convert to dictionary (gets everything)
        Dictionary dict = f.state->to_dictionary(-1);
        CHECK(dict.size() == 4);
        CHECK(static_cast<int>(dict[1]) == 1);
        CHECK(static_cast<int>(dict[2]) == 2);
        CHECK(static_cast<int>(dict[3]) == 3);
        CHECK(static_cast<String>(dict["key"]) == "value");

        f.state->pop(1);
    }
}
