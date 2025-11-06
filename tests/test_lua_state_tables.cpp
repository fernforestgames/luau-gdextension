// Tests for LuaState class - Table operations
// Table creation, get/set operations, iteration

#include "doctest.h"
#include "test_fixtures.h"
#include "lua_state.h"

using namespace godot;

TEST_SUITE("LuaState - Tables")
{
    TEST_CASE_FIXTURE(LuaStateFixture, "create_table - creates empty table")
    {
        state->create_table();
        CHECK(state->is_table(-1));
        CHECK(state->obj_len(-1) == 0);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "create_table - with size hints")
    {
        // Create table with expected array and hash sizes
        state->create_table(10, 5);
        CHECK(state->is_table(-1));

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "set_field and get_field - basic operations")
    {
        state->create_table();

        // Set field
        state->push_number(42.0);
        state->set_field(-2, "test_key");

        // Get field
        lua_Type type = state->get_field(-1, "test_key");
        CHECK(type == LUA_TNUMBER);
        CHECK(state->to_number(-1) == 42.0);
        state->pop(1);

        state->pop(1); // Pop table
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "set_field and get_field - multiple fields")
    {
        state->create_table();

        // Set multiple fields
        state->push_number(1.0);
        state->set_field(-2, "a");

        state->push_string("hello");
        state->set_field(-2, "b");

        state->push_boolean(true);
        state->set_field(-2, "c");

        // Retrieve fields
        state->get_field(-1, "a");
        CHECK(state->to_number(-1) == 1.0);
        state->pop(1);

        state->get_field(-1, "b");
        CHECK(state->to_string_inplace(-1) == "hello");
        state->pop(1);

        state->get_field(-1, "c");
        CHECK(state->to_boolean(-1) == true);
        state->pop(1);

        state->pop(1); // Pop table
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "raw_set_field and raw_get_field - bypass metamethods")
    {
        state->create_table();

        state->push_number(99.0);
        state->raw_set_field(-2, "raw_key");

        lua_Type type = state->raw_get_field(-1, "raw_key");
        CHECK(type == LUA_TNUMBER);
        CHECK(state->to_number(-1) == 99.0);
        state->pop(1);

        state->pop(1); // Pop table
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "set_table and get_table - stack-based operations")
    {
        state->create_table();

        // Set using set_table: table[key] = value
        state->push_string("my_key"); // key
        state->push_number(123.0);    // value
        state->set_table(-3);         // table[key] = value

        // Get using get_table: value = table[key]
        state->push_string("my_key"); // key
        lua_Type type = state->get_table(-2);
        CHECK(type == LUA_TNUMBER);
        CHECK(state->to_number(-1) == 123.0);
        state->pop(1);

        state->pop(1); // Pop table
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "raw_set and raw_get - stack-based without metamethods")
    {
        state->create_table();

        state->push_string("raw_key");
        state->push_number(456.0);
        state->raw_set(-3);

        state->push_string("raw_key");
        lua_Type type = state->raw_get(-2);
        CHECK(type == LUA_TNUMBER);
        CHECK(state->to_number(-1) == 456.0);
        state->pop(1);

        state->pop(1); // Pop table
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "raw_seti and raw_geti - integer index operations")
    {
        state->create_table();

        // Set array elements
        state->push_string("first");
        state->raw_seti(-2, 1);

        state->push_string("second");
        state->raw_seti(-2, 2);

        state->push_string("third");
        state->raw_seti(-2, 3);

        // Get array elements
        lua_Type type = state->raw_geti(-1, 1);
        CHECK(type == LUA_TSTRING);
        CHECK(state->to_string_inplace(-1) == "first");
        state->pop(1);

        type = state->raw_geti(-1, 2);
        CHECK(state->to_string_inplace(-1) == "second");
        state->pop(1);

        type = state->raw_geti(-1, 3);
        CHECK(state->to_string_inplace(-1) == "third");
        state->pop(1);

        state->pop(1); // Pop table
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "obj_len - returns table length")
    {
        state->create_table();

        // Empty table
        CHECK(state->obj_len(-1) == 0);

        // Add array elements
        for (int i = 1; i <= 5; i++)
        {
            state->push_number(i * 10.0);
            state->raw_seti(-2, i);
        }

        CHECK(state->obj_len(-1) == 5);

        state->pop(1); // Pop table
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "next - iterates over table")
    {
        state->create_table();

        // Add some entries
        state->push_number(1.0);
        state->set_field(-2, "a");
        state->push_number(2.0);
        state->set_field(-2, "b");
        state->push_number(3.0);
        state->set_field(-2, "c");

        // Iterate
        int count = 0;
        state->push_nil(); // Initial key

        while (state->next(-2))
        {
            // Stack: table, key, value
            count++;
            state->pop(1); // Pop value, keep key for next iteration
        }

        CHECK(count == 3);

        state->pop(1); // Pop table
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "raw_iter - raw iteration")
    {
        state->create_table();

        // Add entries
        state->push_string("value1");
        state->set_field(-2, "key1");
        state->push_string("value2");
        state->set_field(-2, "key2");

        // Raw iteration
        int count = 0;
        int iter = 0;

        while ((iter = state->raw_iter(-1, iter)) >= 0)
        {
            // Key at -2, value at -1
            count++;
            state->pop(2); // Pop key and value
        }

        CHECK(count == 2);

        state->pop(1); // Pop table
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "clear_table - removes all entries")
    {
        state->create_table();

        // Add entries
        for (int i = 1; i <= 5; i++)
        {
            state->push_number(i * 10.0);
            state->raw_seti(-2, i);
        }

        CHECK(state->obj_len(-1) == 5);

        state->clear_table(-1);
        CHECK(state->obj_len(-1) == 0);

        state->pop(1); // Pop table
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "clone_table - creates shallow copy")
    {
        // Create original table
        state->create_table();
        state->push_number(42.0);
        state->set_field(-2, "test");

        // Clone it
        state->clone_table(-1);

        // Verify clone has same data
        CHECK(state->is_table(-1));
        state->get_field(-1, "test");
        CHECK(state->to_number(-1) == 42.0);
        state->pop(1);

        // Verify they are different tables
        CHECK_FALSE(state->raw_equal(-1, -2));

        state->pop(2); // Pop both tables
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "set_read_only and get_read_only")
    {
        state->create_table();

        CHECK_FALSE(state->get_read_only(-1));

        state->set_read_only(-1, true);
        CHECK(state->get_read_only(-1));

        state->set_read_only(-1, false);
        CHECK_FALSE(state->get_read_only(-1));

        state->pop(1); // Pop table
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "get_metatable and set_metatable")
    {
        state->create_table(); // Main table

        // No metatable initially
        CHECK_FALSE(state->get_metatable(-1));

        // Create and set metatable
        state->create_table(); // Metatable
        state->push_string("test_meta");
        state->set_field(-2, "__name");
        state->set_metatable(-2);

        // Retrieve metatable
        CHECK(state->get_metatable(-1));
        state->get_field(-1, "__name");
        CHECK(state->to_string_inplace(-1) == "test_meta");
        state->pop(1);
        state->pop(1); // Pop metatable

        state->pop(1); // Pop main table
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "new_metatable_named and get_metatable_named")
    {
        // Create new named metatable
        bool is_new = state->new_metatable_named("TestMetatable");
        CHECK(is_new);

        // Add something to it
        state->push_string("test_value");
        state->set_field(-2, "__index");
        state->pop(1); // Pop metatable

        // Trying to create same metatable again returns existing one
        is_new = state->new_metatable_named("TestMetatable");
        CHECK_FALSE(is_new);

        // Verify it's the same metatable
        state->get_field(-1, "__index");
        CHECK(state->to_string_inplace(-1) == "test_value");
        state->pop(1);
        state->pop(1); // Pop metatable

        // Get metatable by name
        lua_Type type = state->get_metatable_named("TestMetatable");
        CHECK(type == LUA_TTABLE);
        state->pop(1);

        // Non-existent metatable
        type = state->get_metatable_named("NonExistent");
        CHECK(type == LUA_TNIL);
        state->pop(1);
    }
}

TEST_SUITE("LuaState - Globals")
{
    TEST_CASE_FIXTURE(LuaStateFixture, "set_global and get_global")
    {
        state->push_number(123.0);
        state->set_global("my_global");

        lua_Type type = state->get_global("my_global");
        CHECK(type == LUA_TNUMBER);
        CHECK(state->to_number(-1) == 123.0);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "get_global - standard library")
    {
        lua_Type type = state->get_global("print");
        CHECK(type == LUA_TFUNCTION);
        state->pop(1);

        type = state->get_global("math");
        CHECK(type == LUA_TTABLE);
        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "get_global - non-existent")
    {
        lua_Type type = state->get_global("nonexistent_global");
        CHECK(type == LUA_TNIL);
        state->pop(1);
    }
}
