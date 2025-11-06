// Tests for LuaState class - Table operations
// Table creation, get/set operations, iteration

#include "doctest.h"
#include "test_fixtures.h"
#include "lua_state.h"

using namespace godot;

TEST_SUITE("LuaState - Tables")
{
    TEST_CASE("create_table - creates empty table")
    {
        LuaStateFixture f;

        f.state->create_table();
        CHECK(f.state->is_table(-1));
        CHECK(f.state->obj_len(-1) == 0);

        f.state->pop(1);
    }

    TEST_CASE("create_table - with size hints")
    {
        LuaStateFixture f;

        // Create table with expected array and hash sizes
        f.state->create_table(10, 5);
        CHECK(f.state->is_table(-1));

        f.state->pop(1);
    }

    TEST_CASE("set_field and get_field - basic operations")
    {
        LuaStateFixture f;

        f.state->create_table();

        // Set field
        f.state->push_number(42.0);
        f.state->set_field(-2, "test_key");

        // Get field
        lua_Type type = f.state->get_field(-1, "test_key");
        CHECK(type == LUA_TNUMBER);
        CHECK(f.state->to_number(-1) == 42.0);
        f.state->pop(1);

        f.state->pop(1); // Pop table
    }

    TEST_CASE("set_field and get_field - multiple fields")
    {
        LuaStateFixture f;

        f.state->create_table();

        // Set multiple fields
        f.state->push_number(1.0);
        f.state->set_field(-2, "a");

        f.state->push_string("hello");
        f.state->set_field(-2, "b");

        f.state->push_boolean(true);
        f.state->set_field(-2, "c");

        // Retrieve fields
        f.state->get_field(-1, "a");
        CHECK(f.state->to_number(-1) == 1.0);
        f.state->pop(1);

        f.state->get_field(-1, "b");
        CHECK(f.state->to_string_inplace(-1) == "hello");
        f.state->pop(1);

        f.state->get_field(-1, "c");
        CHECK(f.state->to_boolean(-1) == true);
        f.state->pop(1);

        f.state->pop(1); // Pop table
    }

    TEST_CASE("raw_set_field and raw_get_field - bypass metamethods")
    {
        LuaStateFixture f;

        f.state->create_table();

        f.state->push_number(99.0);
        f.state->raw_set_field(-2, "raw_key");

        lua_Type type = f.state->raw_get_field(-1, "raw_key");
        CHECK(type == LUA_TNUMBER);
        CHECK(f.state->to_number(-1) == 99.0);
        f.state->pop(1);

        f.state->pop(1); // Pop table
    }

    TEST_CASE("set_table and get_table - stack-based operations")
    {
        LuaStateFixture f;

        f.state->create_table();

        // Set using set_table: table[key] = value
        f.state->push_string("my_key"); // key
        f.state->push_number(123.0);    // value
        f.state->set_table(-3);         // table[key] = value

        // Get using get_table: value = table[key]
        f.state->push_string("my_key"); // key
        lua_Type type = f.state->get_table(-2);
        CHECK(type == LUA_TNUMBER);
        CHECK(f.state->to_number(-1) == 123.0);
        f.state->pop(1);

        f.state->pop(1); // Pop table
    }

    TEST_CASE("raw_set and raw_get - stack-based without metamethods")
    {
        LuaStateFixture f;

        f.state->create_table();

        f.state->push_string("raw_key");
        f.state->push_number(456.0);
        f.state->raw_set(-3);

        f.state->push_string("raw_key");
        lua_Type type = f.state->raw_get(-2);
        CHECK(type == LUA_TNUMBER);
        CHECK(f.state->to_number(-1) == 456.0);
        f.state->pop(1);

        f.state->pop(1); // Pop table
    }

    TEST_CASE("raw_seti and raw_geti - integer index operations")
    {
        LuaStateFixture f;

        f.state->create_table();

        // Set array elements
        f.state->push_string("first");
        f.state->raw_seti(-2, 1);

        f.state->push_string("second");
        f.state->raw_seti(-2, 2);

        f.state->push_string("third");
        f.state->raw_seti(-2, 3);

        // Get array elements
        lua_Type type = f.state->raw_geti(-1, 1);
        CHECK(type == LUA_TSTRING);
        CHECK(f.state->to_string_inplace(-1) == "first");
        f.state->pop(1);

        type = f.state->raw_geti(-1, 2);
        CHECK(f.state->to_string_inplace(-1) == "second");
        f.state->pop(1);

        type = f.state->raw_geti(-1, 3);
        CHECK(f.state->to_string_inplace(-1) == "third");
        f.state->pop(1);

        f.state->pop(1); // Pop table
    }

    TEST_CASE("obj_len - returns table length")
    {
        LuaStateFixture f;

        f.state->create_table();

        // Empty table
        CHECK(f.state->obj_len(-1) == 0);

        // Add array elements
        for (int i = 1; i <= 5; i++)
        {
            f.state->push_number(i * 10.0);
            f.state->raw_seti(-2, i);
        }

        CHECK(f.state->obj_len(-1) == 5);

        f.state->pop(1); // Pop table
    }

    TEST_CASE("next - iterates over table")
    {
        LuaStateFixture f;

        f.state->create_table();

        // Add some entries
        f.state->push_number(1.0);
        f.state->set_field(-2, "a");
        f.state->push_number(2.0);
        f.state->set_field(-2, "b");
        f.state->push_number(3.0);
        f.state->set_field(-2, "c");

        // Iterate
        int count = 0;
        f.state->push_nil(); // Initial key

        while (f.state->next(-2))
        {
            // Stack: table, key, value
            count++;
            f.state->pop(1); // Pop value, keep key for next iteration
        }

        CHECK(count == 3);

        f.state->pop(1); // Pop table
    }

    TEST_CASE("raw_iter - raw iteration")
    {
        LuaStateFixture f;

        f.state->create_table();

        // Add entries
        f.state->push_string("value1");
        f.state->set_field(-2, "key1");
        f.state->push_string("value2");
        f.state->set_field(-2, "key2");

        // Raw iteration
        int count = 0;
        int iter = -1;

        while ((iter = f.state->raw_iter(-1, iter)) != -1)
        {
            // Key at -2, value at -1
            count++;
            f.state->pop(2); // Pop key and value
        }

        CHECK(count == 2);

        f.state->pop(1); // Pop table
    }

    TEST_CASE("clear_table - removes all entries")
    {
        LuaStateFixture f;

        f.state->create_table();

        // Add entries
        for (int i = 1; i <= 5; i++)
        {
            f.state->push_number(i * 10.0);
            f.state->raw_seti(-2, i);
        }

        CHECK(f.state->obj_len(-1) == 5);

        f.state->clear_table(-1);
        CHECK(f.state->obj_len(-1) == 0);

        f.state->pop(1); // Pop table
    }

    TEST_CASE("clone_table - creates shallow copy")
    {
        LuaStateFixture f;

        // Create original table
        f.state->create_table();
        f.state->push_number(42.0);
        f.state->set_field(-2, "test");

        // Clone it
        f.state->clone_table(-1);

        // Verify clone has same data
        CHECK(f.state->is_table(-1));
        f.state->get_field(-1, "test");
        CHECK(f.state->to_number(-1) == 42.0);
        f.state->pop(1);

        // Verify they are different tables
        CHECK_FALSE(f.state->raw_equal(-1, -2));

        f.state->pop(2); // Pop both tables
    }

    TEST_CASE("set_read_only and get_read_only")
    {
        LuaStateFixture f;

        f.state->create_table();

        CHECK_FALSE(f.state->get_read_only(-1));

        f.state->set_read_only(-1, true);
        CHECK(f.state->get_read_only(-1));

        f.state->set_read_only(-1, false);
        CHECK_FALSE(f.state->get_read_only(-1));

        f.state->pop(1); // Pop table
    }

    TEST_CASE("get_metatable and set_metatable")
    {
        LuaStateFixture f;

        f.state->create_table(); // Main table

        // No metatable initially
        CHECK_FALSE(f.state->get_metatable(-1));

        // Create and set metatable
        f.state->create_table(); // Metatable
        f.state->push_string("test_meta");
        f.state->set_field(-2, "__name");
        f.state->set_metatable(-2);

        // Retrieve metatable
        CHECK(f.state->get_metatable(-1));
        f.state->get_field(-1, "__name");
        CHECK(f.state->to_string_inplace(-1) == "test_meta");
        f.state->pop(1);
        f.state->pop(1); // Pop metatable

        f.state->pop(1); // Pop main table
    }

    TEST_CASE("new_metatable_named and get_metatable_named")
    {
        LuaStateFixture f;

        // Create new named metatable
        bool is_new = f.state->new_metatable_named("TestMetatable");
        CHECK(is_new);

        // Add something to it
        f.state->push_string("test_value");
        f.state->set_field(-2, "__index");
        f.state->pop(1); // Pop metatable

        // Trying to create same metatable again returns existing one
        is_new = f.state->new_metatable_named("TestMetatable");
        CHECK_FALSE(is_new);

        // Verify it's the same metatable
        f.state->get_field(-1, "__index");
        CHECK(f.state->to_string_inplace(-1) == "test_value");
        f.state->pop(1);
        f.state->pop(1); // Pop metatable

        // Get metatable by name
        lua_Type type = f.state->get_metatable_named("TestMetatable");
        CHECK(type == LUA_TTABLE);
        f.state->pop(1);

        // Non-existent metatable
        type = f.state->get_metatable_named("NonExistent");
        CHECK(type == LUA_TNIL);
        f.state->pop(1);
    }
}

TEST_SUITE("LuaState - Globals")
{
    TEST_CASE("set_global and get_global")
    {
        LuaStateFixture f;

        f.state->push_number(123.0);
        f.state->set_global("my_global");

        lua_Type type = f.state->get_global("my_global");
        CHECK(type == LUA_TNUMBER);
        CHECK(f.state->to_number(-1) == 123.0);

        f.state->pop(1);
    }

    TEST_CASE("get_global - standard library")
    {
        LuaStateFixture f;

        lua_Type type = f.state->get_global("print");
        CHECK(type == LUA_TFUNCTION);
        f.state->pop(1);

        type = f.state->get_global("math");
        CHECK(type == LUA_TTABLE);
        f.state->pop(1);
    }

    TEST_CASE("get_global - non-existent")
    {
        LuaStateFixture f;

        lua_Type type = f.state->get_global("nonexistent_global");
        CHECK(type == LUA_TNIL);
        f.state->pop(1);
    }
}
