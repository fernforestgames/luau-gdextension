// Tests for bridging/array - Array bridging between Godot and Lua
// is_array, to_array, push_array

#include "doctest.h"
#include "test_fixtures.h"
#include "lua_state.h"

using namespace godot;

TEST_SUITE("Bridging - Array")
{
    TEST_CASE("push_array - empty array")
    {
        LuaStateFixture f;

        Array arr;
        f.state->push_array(arr);

        CHECK(f.state->is_table(-1));
        CHECK(f.state->obj_len(-1) == 0);

        f.state->pop(1);
    }

    TEST_CASE("push_array - simple values")
    {
        LuaStateFixture f;

        Array arr;
        arr.push_back(1);
        arr.push_back(2);
        arr.push_back(3);

        f.state->push_array(arr);

        CHECK(f.state->is_table(-1));
        CHECK(f.state->obj_len(-1) == 3);

        // Check values (Lua arrays are 1-based)
        f.state->raw_geti(-1, 1);
        CHECK(f.state->to_number(-1) == 1.0);
        f.state->pop(1);

        f.state->raw_geti(-1, 2);
        CHECK(f.state->to_number(-1) == 2.0);
        f.state->pop(1);

        f.state->raw_geti(-1, 3);
        CHECK(f.state->to_number(-1) == 3.0);
        f.state->pop(1);

        f.state->pop(1); // Pop table
    }

    TEST_CASE("push_array - mixed types")
    {
        LuaStateFixture f;

        Array arr;
        arr.push_back(42);
        arr.push_back("hello");
        arr.push_back(true);
        arr.push_back(3.14);

        f.state->push_array(arr);

        f.state->raw_geti(-1, 1);
        CHECK(f.state->to_number(-1) == 42.0);
        f.state->pop(1);

        f.state->raw_geti(-1, 2);
        CHECK(f.state->to_string_inplace(-1) == "hello");
        f.state->pop(1);

        f.state->raw_geti(-1, 3);
        CHECK(f.state->to_boolean(-1) == true);
        f.state->pop(1);

        f.state->raw_geti(-1, 4);
        CHECK(f.state->to_number(-1) == doctest::Approx(3.14));
        f.state->pop(1);

        f.state->pop(1); // Pop table
    }

    TEST_CASE("push_array - nested arrays")
    {
        LuaStateFixture f;

        Array inner;
        inner.push_back(1);
        inner.push_back(2);

        Array outer;
        outer.push_back(inner);
        outer.push_back("test");

        f.state->push_array(outer);

        // Check outer array
        CHECK(f.state->obj_len(-1) == 2);

        // Check inner array
        f.state->raw_geti(-1, 1);
        CHECK(f.state->is_table(-1));
        CHECK(f.state->obj_len(-1) == 2);

        f.state->raw_geti(-1, 1);
        CHECK(f.state->to_number(-1) == 1.0);
        f.state->pop(1);

        f.state->raw_geti(-1, 2);
        CHECK(f.state->to_number(-1) == 2.0);
        f.state->pop(1);

        f.state->pop(1); // Pop inner array

        // Check second element
        f.state->raw_geti(-1, 2);
        CHECK(f.state->to_string_inplace(-1) == "test");
        f.state->pop(1);

        f.state->pop(1); // Pop outer array
    }

    TEST_CASE("is_array - empty table")
    {
        LuaStateFixture f;

        f.state->create_table();
        CHECK(f.state->is_array(-1));

        f.state->pop(1);
    }

    TEST_CASE("is_array - sequential integer keys")
    {
        LuaStateFixture f;

        f.state->create_table();

        for (int i = 1; i <= 5; i++)
        {
            f.state->push_number(i * 10.0);
            f.state->raw_seti(-2, i);
        }

        CHECK(f.state->is_array(-1));

        f.state->pop(1);
    }

    TEST_CASE("is_array - non-sequential keys")
    {
        LuaStateFixture f;

        f.state->create_table();

        f.state->push_number(1.0);
        f.state->raw_seti(-2, 1);

        f.state->push_number(2.0);
        f.state->raw_seti(-2, 3); // Gap at index 2

        CHECK_FALSE(f.state->is_array(-1));

        f.state->pop(1);
    }

    TEST_CASE("is_array - starts at wrong index")
    {
        LuaStateFixture f;

        f.state->create_table();

        f.state->push_number(1.0);
        f.state->raw_seti(-2, 0); // Starts at 0 instead of 1

        f.state->push_number(2.0);
        f.state->raw_seti(-2, 1);

        CHECK_FALSE(f.state->is_array(-1));

        f.state->pop(1);
    }

    TEST_CASE("is_array - has string keys")
    {
        LuaStateFixture f;

        f.state->create_table();

        f.state->push_number(1.0);
        f.state->raw_seti(-2, 1);

        f.state->push_string("value");
        f.state->raw_set_field(-2, "key"); // String key

        CHECK_FALSE(f.state->is_array(-1));

        f.state->pop(1);
    }

    TEST_CASE("is_array - has non-integer numeric keys")
    {
        LuaStateFixture f;

        f.state->create_table();

        f.state->push_number(1.0);
        f.state->raw_seti(-2, 1);

        f.state->push_number(1.5); // key
        f.state->push_string("value");
        f.state->raw_set(-3);

        CHECK_FALSE(f.state->is_array(-1));

        f.state->pop(1);
    }

    TEST_CASE("is_array - not a table")
    {
        LuaStateFixture f;

        f.state->push_number(42.0);
        CHECK_FALSE(f.state->is_array(-1));

        f.state->pop(1);

        f.state->push_string("test");
        CHECK_FALSE(f.state->is_array(-1));

        f.state->pop(1);
    }

    TEST_CASE("to_array - simple array")
    {
        LuaStateFixture f;

        f.state->create_table();
        f.state->push_number(10.0);
        f.state->raw_seti(-2, 1);
        f.state->push_number(20.0);
        f.state->raw_seti(-2, 2);
        f.state->push_number(30.0);
        f.state->raw_seti(-2, 3);

        Array result = f.state->to_array(-1);

        CHECK(result.size() == 3);
        CHECK(static_cast<int>(result[0]) == 10);
        CHECK(static_cast<int>(result[1]) == 20);
        CHECK(static_cast<int>(result[2]) == 30);

        f.state->pop(1);
    }

    TEST_CASE("to_array - mixed types")
    {
        LuaStateFixture f;

        f.state->create_table();
        f.state->push_number(42.0);
        f.state->raw_seti(-2, 1);
        f.state->push_string("test");
        f.state->raw_seti(-2, 2);
        f.state->push_boolean(true);
        f.state->raw_seti(-2, 3);

        Array result = f.state->to_array(-1);

        CHECK(result.size() == 3);
        CHECK(static_cast<int>(result[0]) == 42);
        CHECK(static_cast<String>(result[1]) == "test");
        CHECK(static_cast<bool>(result[2]) == true);

        f.state->pop(1);
    }

    TEST_CASE("to_array - empty table")
    {
        LuaStateFixture f;

        f.state->create_table();

        Array result = f.state->to_array(-1);

        CHECK(result.size() == 0);

        f.state->pop(1);
    }

    TEST_CASE("to_array - nested arrays")
    {
        LuaStateFixture f;

        // Create nested structure
        f.state->create_table(); // outer

        f.state->create_table(); // inner
        f.state->push_number(1.0);
        f.state->raw_seti(-2, 1);
        f.state->push_number(2.0);
        f.state->raw_seti(-2, 2);
        f.state->raw_seti(-2, 1); // outer[1] = inner

        f.state->push_string("test");
        f.state->raw_seti(-2, 2); // outer[2] = "test"

        Array result = f.state->to_array(-1);

        CHECK(result.size() == 2);
        CHECK(result[0].get_type() == Variant::ARRAY);
        CHECK(static_cast<String>(result[1]) == "test");

        Array inner_result = result[0];
        CHECK(inner_result.size() == 2);
        CHECK(static_cast<int>(inner_result[0]) == 1);
        CHECK(static_cast<int>(inner_result[1]) == 2);

        f.state->pop(1);
    }

    // Note: to_array in LuaState doesn't have an is_array flag parameter
    // It's only in the bridging layer function. Use is_array() separately.

    TEST_CASE("to_array - not a table")
    {
        LuaStateFixture f;

        f.state->push_number(42.0);

        Array result = f.state->to_array(-1);

        CHECK(result.size() == 0);

        f.state->pop(1);
    }

    TEST_CASE("round-trip - array to Lua and back")
    {
        LuaStateFixture f;

        Array original;
        original.push_back(1);
        original.push_back("two");
        original.push_back(3.2);
        original.push_back(true);

        f.state->push_array(original);
        Array result = f.state->to_array(-1);

        CHECK(result.size() == original.size());
        for (int i = 0; i < original.size(); i++)
        {
            CHECK(result[i] == original[i]);
        }

        f.state->pop(1);
    }

    TEST_CASE("round-trip - nested arrays")
    {
        LuaStateFixture f;

        Array inner1;
        inner1.push_back(1);
        inner1.push_back(2);

        Array inner2;
        inner2.push_back(3);
        inner2.push_back(4);

        Array outer;
        outer.push_back(inner1);
        outer.push_back(inner2);
        outer.push_back("test");

        f.state->push_array(outer);
        Array result = f.state->to_array(-1);

        CHECK(result.size() == 3);

        Array result_inner1 = result[0];
        CHECK(result_inner1.size() == 2);
        CHECK(static_cast<int>(result_inner1[0]) == 1);
        CHECK(static_cast<int>(result_inner1[1]) == 2);

        Array result_inner2 = result[1];
        CHECK(result_inner2.size() == 2);
        CHECK(static_cast<int>(result_inner2[0]) == 3);
        CHECK(static_cast<int>(result_inner2[1]) == 4);

        CHECK(static_cast<String>(result[2]) == "test");

        f.state->pop(1);
    }

    TEST_CASE("integration - Lua script manipulation")
    {
        LuaStateFixture f;

        f.exec_lua_ok(R"(
            test_array = {10, 20, 30, 40, 50}
            return test_array
        )");

        Array result = f.state->to_array(-1);

        CHECK(result.size() == 5);
        CHECK(static_cast<int>(result[0]) == 10);
        CHECK(static_cast<int>(result[1]) == 20);
        CHECK(static_cast<int>(result[2]) == 30);
        CHECK(static_cast<int>(result[3]) == 40);
        CHECK(static_cast<int>(result[4]) == 50);

        f.state->pop(1);
    }

    TEST_CASE("integration - passing array to Lua function")
    {
        LuaStateFixture f;

        f.exec_lua_ok(R"(
            function sum_array(arr)
                local total = 0
                for i = 1, #arr do
                    total = total + arr[i]
                end
                return total
            end
        )");

        Array arr;
        arr.push_back(1);
        arr.push_back(2);
        arr.push_back(3);
        arr.push_back(4);
        arr.push_back(5);

        f.state->get_global("sum_array");
        f.state->push_array(arr);
        f.state->call(1, 1);

        CHECK(f.state->to_number(-1) == 15.0);

        f.state->pop(1);
    }
}
