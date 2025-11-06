// Tests for bridging/dictionary - Dictionary bridging between Godot and Lua
// to_dictionary, push_dictionary

#include "doctest.h"
#include "test_fixtures.h"
#include "lua_state.h"

using namespace godot;

TEST_SUITE("Bridging - Dictionary")
{
    TEST_CASE_FIXTURE(LuaStateFixture, "push_dictionary - empty dictionary")
    {
        Dictionary dict;
        state->push_dictionary(dict);

        CHECK(state->is_table(-1));
        CHECK(state->obj_len(-1) == 0);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_dictionary - string keys")
    {
        Dictionary dict;
        dict["name"] = "Alice";
        dict["age"] = 30;
        dict["active"] = true;

        state->push_dictionary(dict);

        CHECK(state->is_table(-1));

        // Check values
        state->get_field(-1, "name");
        CHECK(state->to_string_inplace(-1) == "Alice");
        state->pop(1);

        state->get_field(-1, "age");
        CHECK(state->to_number(-1) == 30.0);
        state->pop(1);

        state->get_field(-1, "active");
        CHECK(state->to_boolean(-1) == true);
        state->pop(1);

        state->pop(1); // Pop table
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_dictionary - numeric keys")
    {
        Dictionary dict;
        dict[1] = "one";
        dict[2] = "two";
        dict[10] = "ten";

        state->push_dictionary(dict);

        // Check values
        state->raw_geti(-1, 1);
        CHECK(state->to_string_inplace(-1) == "one");
        state->pop(1);

        state->raw_geti(-1, 2);
        CHECK(state->to_string_inplace(-1) == "two");
        state->pop(1);

        state->raw_geti(-1, 10);
        CHECK(state->to_string_inplace(-1) == "ten");
        state->pop(1);

        state->pop(1); // Pop table
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_dictionary - mixed key types")
    {
        Dictionary dict;
        dict["string_key"] = 42.5;
        dict[123] = "numeric_key";
        dict[true] = "bool_key";

        state->push_dictionary(dict);

        state->get_field(-1, "string_key");
        CHECK(state->to_number(-1) == 42.5);
        state->pop(1);

        state->raw_geti(-1, 123);
        CHECK(state->to_string_inplace(-1) == "numeric_key");
        state->pop(1);

        state->push_boolean(true);
        state->get_table(-2);
        CHECK(state->to_string_inplace(-1) == "bool_key");
        state->pop(1);

        state->pop(1); // Pop table
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_dictionary - nested dictionaries")
    {
        Dictionary inner;
        inner["x"] = 10;
        inner["y"] = 20;

        Dictionary outer;
        outer["position"] = inner;
        outer["name"] = "test";

        state->push_dictionary(outer);

        // Check outer
        state->get_field(-1, "name");
        CHECK(state->to_string_inplace(-1) == "test");
        state->pop(1);

        // Check inner
        state->get_field(-1, "position");
        CHECK(state->type(-1) == LUA_TTABLE);

        state->get_field(-1, "x");
        CHECK(state->to_number(-1) == 10.0);
        state->pop(1);

        state->get_field(-1, "y");
        CHECK(state->to_number(-1) == 20.0);
        state->pop(1);

        state->pop(1); // Pop inner table

        state->pop(1); // Pop outer table
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_dictionary - empty table")
    {
        state->create_table();

        Dictionary result = state->to_dictionary(-1);

        CHECK(result.size() == 0);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_dictionary - string keys")
    {
        state->create_table();

        state->push_string("Alice");
        state->set_field(-2, "name");

        state->push_number(30.0);
        state->set_field(-2, "age");

        Dictionary result = state->to_dictionary(-1);

        CHECK(result.size() == 2);
        CHECK(static_cast<String>(result["name"]) == "Alice");
        CHECK(static_cast<int>(result["age"]) == 30);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_dictionary - numeric keys")
    {
        state->create_table();

        state->push_string("one");
        state->raw_seti(-2, 1);

        state->push_string("five");
        state->raw_seti(-2, 5);

        Dictionary result = state->to_dictionary(-1);

        CHECK(result.size() == 2);
        CHECK(static_cast<String>(result[1]) == "one");
        CHECK(static_cast<String>(result[5]) == "five");

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_dictionary - mixed key types")
    {
        state->create_table();

        state->push_number(42.0);
        state->set_field(-2, "str_key");

        state->push_string("test");
        state->raw_seti(-2, 99);

        state->push_boolean(true);
        state->push_string("bool_val");
        state->set_table(-3);

        Dictionary result = state->to_dictionary(-1);

        CHECK(result.size() == 3);
        CHECK(static_cast<int>(result["str_key"]) == 42);
        CHECK(static_cast<String>(result[99]) == "test");
        CHECK(static_cast<String>(result[true]) == "bool_val");

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_dictionary - nested tables")
    {
        // Create nested structure
        state->create_table(); // outer

        state->create_table(); // inner
        state->push_number(10.0);
        state->set_field(-2, "x");
        state->push_number(20.0);
        state->set_field(-2, "y");
        state->set_field(-2, "position"); // outer.position = inner

        state->push_string("test");
        state->set_field(-2, "name"); // outer.name = "test"

        Dictionary result = state->to_dictionary(-1);

        CHECK(result.size() == 2);
        CHECK(static_cast<String>(result["name"]) == "test");
        CHECK(result["position"].get_type() == Variant::DICTIONARY);

        Dictionary inner = result["position"];
        CHECK(inner.size() == 2);
        CHECK(static_cast<int>(inner["x"]) == 10);
        CHECK(static_cast<int>(inner["y"]) == 20);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_dictionary - array vs dictionary distinction")
    {
        // Pure array (sequential integer keys from 1)
        state->create_table();
        state->push_number(1.0);
        state->raw_seti(-2, 1);
        state->push_number(2.0);
        state->raw_seti(-2, 2);

        Dictionary dict_result = state->to_dictionary(-1);
        // Dictionary conversion always works, even for arrays
        CHECK(dict_result.size() == 2);

        state->pop(1);

        // Table with gap (not a pure array)
        state->create_table();
        state->push_number(1.0);
        state->raw_seti(-2, 1);
        state->push_number(3.0);
        state->raw_seti(-2, 3); // Gap at 2

        dict_result = state->to_dictionary(-1);
        CHECK(dict_result.size() == 2);
        CHECK(static_cast<int>(dict_result[1]) == 1);
        CHECK(static_cast<int>(dict_result[3]) == 3);

        state->pop(1);
    }

    // Note: to_dictionary in LuaState doesn't have a success flag parameter
    // It's only in the bridging layer function. Skipping this test for now.

    TEST_CASE_FIXTURE(LuaStateFixture, "to_dictionary - not a table")
    {
        state->push_number(42.0);

        Dictionary result = state->to_dictionary(-1);

        CHECK(result.size() == 0);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "round-trip - dictionary to Lua and back")
    {
        Dictionary original;
        original["name"] = "Bob";
        original["age"] = 25;
        original["score"] = 98.5;
        original["active"] = true;

        state->push_dictionary(original);
        Dictionary result = state->to_dictionary(-1);

        CHECK(result.size() == original.size());
        CHECK(result["name"] == original["name"]);
        CHECK(result["age"] == original["age"]);
        CHECK(result["score"] == original["score"]);
        CHECK(result["active"] == original["active"]);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "round-trip - nested dictionaries")
    {
        Dictionary inner;
        inner["x"] = 100;
        inner["y"] = 200;

        Dictionary outer;
        outer["position"] = inner;
        outer["name"] = "player";

        state->push_dictionary(outer);
        Dictionary result = state->to_dictionary(-1);

        CHECK(result.size() == 2);
        CHECK(static_cast<String>(result["name"]) == "player");

        Dictionary result_inner = result["position"];
        CHECK(result_inner.size() == 2);
        CHECK(static_cast<int>(result_inner["x"]) == 100);
        CHECK(static_cast<int>(result_inner["y"]) == 200);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "integration - Lua script manipulation")
    {
        exec_lua_ok(R"(
            test_dict = {
                name = "Test",
                count = 42,
                enabled = true
            }
            return test_dict
        )");

        Dictionary result = state->to_dictionary(-1);

        CHECK(result.size() == 3);
        CHECK(static_cast<String>(result["name"]) == "Test");
        CHECK(static_cast<int>(result["count"]) == 42);
        CHECK(static_cast<bool>(result["enabled"]) == true);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "integration - passing dictionary to Lua function")
    {
        exec_lua_ok(R"(
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

        state->get_global("process_dict");
        state->push_dictionary(input);
        state->call(1, 1);

        Dictionary result = state->to_dictionary(-1);

        CHECK(result.size() == 3);
        CHECK(static_cast<int>(result["a"]) == 1);
        CHECK(static_cast<int>(result["b"]) == 2);
        CHECK(static_cast<bool>(result["processed"]) == true);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "integration - table with both array and hash parts")
    {
        exec_lua_ok(R"(
            mixed_table = {
                1, 2, 3,  -- Array part
                key = "value"  -- Hash part
            }
            return mixed_table
        )");

        // Convert to dictionary (gets everything)
        Dictionary dict = state->to_dictionary(-1);
        CHECK(dict.size() == 4);
        CHECK(static_cast<int>(dict[1]) == 1);
        CHECK(static_cast<int>(dict[2]) == 2);
        CHECK(static_cast<int>(dict[3]) == 3);
        CHECK(static_cast<String>(dict["key"]) == "value");

        state->pop(1);
    }
}
