// Tests for LuaState class - Basic functionality
// Stack manipulation, type checking, basic operations

#include "doctest.h"
#include "test_fixtures.h"
#include "lua_state.h"
#include "luau.h"

using namespace godot;

TEST_SUITE("LuaState - Basic Operations")
{
    TEST_CASE("constructor - creates valid state")
    {
        Ref<LuaState> state = memnew(LuaState);

        CHECK(state.is_valid());
        CHECK(state->is_valid());
        CHECK(state->is_main_thread());
        CHECK(state->get_top() == 0);
        CHECK(state->get_lua_state() != nullptr);

        state->close();
    }

    TEST_CASE("open_libs - opens standard libraries")
    {
        LuaStateFixture f;

        // Verify standard library is available (already opened in fixture)
        f.state->get_global("print");
        CHECK(f.state->is_function(-1));
        f.state->pop(1);

        f.state->get_global("table");
        CHECK(f.state->is_table(-1));
        f.state->pop(1);

        f.state->get_global("math");
        CHECK(f.state->is_table(-1));
        f.state->pop(1);
    }

    TEST_CASE("open_libs - selective libraries")
    {
        Ref<LuaState> state = memnew(LuaState);

        // Open only base and math libraries
        state->open_libs(LuaState::LIB_BASE | LuaState::LIB_MATH);

        // Base library should be available
        state->get_global("print");
        CHECK(state->is_function(-1));
        state->pop(1);

        // Math library should be available
        state->get_global("math");
        CHECK(state->is_table(-1));
        state->pop(1);

        // String library should NOT be available
        state->get_global("string");
        CHECK(state->is_nil(-1));
        state->pop(1);

        state->close();
    }

    TEST_CASE("close - invalidates state")
    {
        Ref<LuaState> state = memnew(LuaState);
        CHECK(state->is_valid());

        state->close();
        CHECK_FALSE(state->is_valid());

        // Operations on closed state should be safe (no crash)
        state->close(); // Double close should be safe
    }

    TEST_CASE("is_main_thread - identifies main thread")
    {
        LuaStateFixture f;
        CHECK(f.state->is_main_thread());

        Ref<LuaState> thread = f.state->new_thread();
        CHECK_FALSE(thread->is_main_thread());
        CHECK(thread->get_main_thread() == f.state);

        f.state->pop(1); // Pop thread from stack
    }
}

TEST_SUITE("LuaState - Stack Manipulation")
{
    TEST_CASE("get_top - returns stack size")
    {
        LuaStateFixture f;

        CHECK(f.state->get_top() == 0);

        f.state->push_number(1.0);
        CHECK(f.state->get_top() == 1);

        f.state->push_number(2.0);
        CHECK(f.state->get_top() == 2);

        f.state->push_string("test");
        CHECK(f.state->get_top() == 3);

        f.state->pop(3);
    }

    TEST_CASE("set_top - sets stack size")
    {
        LuaStateFixture f;

        f.state->push_number(1.0);
        f.state->push_number(2.0);
        f.state->push_number(3.0);
        CHECK(f.state->get_top() == 3);

        f.state->set_top(1);
        CHECK(f.state->get_top() == 1);
        CHECK(f.state->to_number(-1) == 1.0);

        f.state->set_top(0);
        CHECK(f.state->get_top() == 0);
    }

    TEST_CASE("pop - removes elements from stack")
    {
        LuaStateFixture f;

        f.state->push_number(1.0);
        f.state->push_number(2.0);
        f.state->push_number(3.0);
        CHECK(f.state->get_top() == 3);

        f.state->pop(1);
        CHECK(f.state->get_top() == 2);
        CHECK(f.state->to_number(-1) == 2.0);

        f.state->pop(2);
        CHECK(f.state->get_top() == 0);
    }

    TEST_CASE("push_value - duplicates stack value")
    {
        LuaStateFixture f;

        f.state->push_number(42.0);
        CHECK(f.state->get_top() == 1);

        f.state->push_value(-1); // Duplicate top
        CHECK(f.state->get_top() == 2);
        CHECK(f.state->to_number(-1) == 42.0);
        CHECK(f.state->to_number(-2) == 42.0);

        f.state->pop(2);
    }

    TEST_CASE("remove - removes element from middle of stack")
    {
        LuaStateFixture f;

        f.state->push_number(1.0);
        f.state->push_number(2.0);
        f.state->push_number(3.0);

        f.state->remove(2); // Remove middle element
        CHECK(f.state->get_top() == 2);
        CHECK(f.state->to_number(1) == 1.0);
        CHECK(f.state->to_number(2) == 3.0);

        f.state->pop(2);
    }

    TEST_CASE("insert - inserts element into stack")
    {
        LuaStateFixture f;

        f.state->push_number(1.0);
        f.state->push_number(2.0);
        f.state->push_number(99.0); // Value to insert

        f.state->insert(2); // Insert before index 2
        CHECK(f.state->get_top() == 3);
        CHECK(f.state->to_number(1) == 1.0);
        CHECK(f.state->to_number(2) == 99.0);
        CHECK(f.state->to_number(3) == 2.0);

        f.state->pop(3);
    }

    TEST_CASE("replace - replaces element in stack")
    {
        LuaStateFixture f;

        f.state->push_number(1.0);
        f.state->push_number(2.0);
        f.state->push_number(99.0); // Replacement value

        f.state->replace(1); // Replace index 1
        CHECK(f.state->get_top() == 2);
        CHECK(f.state->to_number(1) == 99.0);
        CHECK(f.state->to_number(2) == 2.0);

        f.state->pop(2);
    }

    TEST_CASE("abs_index - converts relative to absolute index")
    {
        LuaStateFixture f;

        f.state->push_number(1.0);
        f.state->push_number(2.0);
        f.state->push_number(3.0);

        CHECK(f.state->abs_index(-1) == 3);
        CHECK(f.state->abs_index(-2) == 2);
        CHECK(f.state->abs_index(-3) == 1);
        CHECK(f.state->abs_index(1) == 1);
        CHECK(f.state->abs_index(2) == 2);

        f.state->pop(3);
    }

    TEST_CASE("check_stack - ensures stack space")
    {
        LuaStateFixture f;

        CHECK(f.state->check_stack(10));
        CHECK(f.state->check_stack(100));
        CHECK(f.state->check_stack(1000));
    }
}

TEST_SUITE("LuaState - Push Functions")
{
    TEST_CASE("push_nil")
    {
        LuaStateFixture f;

        f.state->push_nil();
        CHECK(f.state->is_nil(-1));
        CHECK(f.state->type(-1) == LUA_TNIL);

        f.state->pop(1);
    }

    TEST_CASE("push_number")
    {
        LuaStateFixture f;

        f.state->push_number(42.5);
        CHECK(f.state->is_number(-1));
        CHECK(f.state->to_number(-1) == 42.5);

        f.state->pop(1);

        f.state->push_number(-123.456);
        CHECK(f.state->to_number(-1) == -123.456);

        f.state->pop(1);
    }

    TEST_CASE("push_integer")
    {
        LuaStateFixture f;

        f.state->push_integer(42);
        CHECK(f.state->is_number(-1));
        CHECK(f.state->to_integer(-1) == 42);
        CHECK(f.state->to_number(-1) == 42.0);

        f.state->pop(1);
    }

    TEST_CASE("push_vector3")
    {
        LuaStateFixture f;

        Vector3 v(1.5, 2.5, 3.5);
        f.state->push_vector3(v);
        CHECK(f.state->is_vector(-1));

        Vector3 result = f.state->to_vector(-1);
        CHECK(result.x == 1.5);
        CHECK(result.y == 2.5);
        CHECK(result.z == 3.5);

        f.state->pop(1);
    }

    TEST_CASE("push_string")
    {
        LuaStateFixture f;

        f.state->push_string("Hello, World!");
        CHECK(f.state->is_string(-1));
        CHECK(f.state->to_string_inplace(-1) == "Hello, World!");

        f.state->pop(1);

        // Empty string
        f.state->push_string("");
        CHECK(f.state->is_string(-1));
        CHECK(f.state->to_string_inplace(-1) == "");

        f.state->pop(1);

        // String with special characters
        f.state->push_string("Line 1\nLine 2\tTab");
        CHECK(f.state->to_string_inplace(-1) == "Line 1\nLine 2\tTab");

        f.state->pop(1);
    }

    TEST_CASE("push_string_name")
    {
        LuaStateFixture f;

        f.state->push_string_name(StringName("test_name"));
        CHECK(f.state->is_string(-1));
        CHECK(f.state->to_stringname(-1) == StringName("test_name"));

        f.state->pop(1);
    }

    TEST_CASE("push_boolean")
    {
        LuaStateFixture f;

        f.state->push_boolean(true);
        CHECK(f.state->is_boolean(-1));
        CHECK(f.state->to_boolean(-1) == true);

        f.state->pop(1);

        f.state->push_boolean(false);
        CHECK(f.state->is_boolean(-1));
        CHECK(f.state->to_boolean(-1) == false);

        f.state->pop(1);
    }
}

TEST_SUITE("LuaState - Type Checking")
{
    TEST_CASE("type - returns lua_Type")
    {
        LuaStateFixture f;

        f.state->push_nil();
        CHECK(f.state->type(-1) == LUA_TNIL);
        f.state->pop(1);

        f.state->push_number(42.0);
        CHECK(f.state->type(-1) == LUA_TNUMBER);
        f.state->pop(1);

        f.state->push_string("test");
        CHECK(f.state->type(-1) == LUA_TSTRING);
        f.state->pop(1);

        f.state->push_boolean(true);
        CHECK(f.state->type(-1) == LUA_TBOOLEAN);
        f.state->pop(1);

        f.state->create_table();
        CHECK(f.state->type(-1) == LUA_TTABLE);
        f.state->pop(1);

        f.state->push_vector3(Vector3(1, 2, 3));
        CHECK(f.state->type(-1) == LUA_TVECTOR);
        f.state->pop(1);
    }

    TEST_CASE("is_* functions")
    {
        LuaStateFixture f;

        f.state->push_number(42.0);
        CHECK(f.state->is_number(-1));
        CHECK(f.state->is_string(-1)); // numbers are always convertible to strings in Lua
        CHECK_FALSE(f.state->is_boolean(-1));
        CHECK_FALSE(f.state->is_nil(-1));
        f.state->pop(1);

        f.state->push_string("test");
        CHECK(f.state->is_string(-1));
        CHECK_FALSE(f.state->is_number(-1));
        f.state->pop(1);

        f.state->push_boolean(true);
        CHECK(f.state->is_boolean(-1));
        CHECK_FALSE(f.state->is_number(-1));
        f.state->pop(1);

        f.state->push_nil();
        CHECK(f.state->is_nil(-1));
        CHECK_FALSE(f.state->is_number(-1));
        f.state->pop(1);

        f.state->create_table();
        CHECK(f.state->is_table(-1));
        CHECK_FALSE(f.state->is_function(-1));
        f.state->pop(1);

        f.state->push_vector3(Vector3(1, 2, 3));
        CHECK(f.state->is_vector(-1));
        CHECK_FALSE(f.state->is_number(-1));
        f.state->pop(1);
    }

    TEST_CASE("is_none and is_none_or_nil")
    {
        LuaStateFixture f;

        // Valid index with nil
        f.state->push_nil();
        CHECK_FALSE(f.state->is_none(-1));
        CHECK(f.state->is_none_or_nil(-1));
        f.state->pop(1);

        // Invalid index (beyond stack)
        CHECK(f.state->is_none(10));
        CHECK(f.state->is_none_or_nil(10));
    }

    TEST_CASE("type_name - returns type name")
    {
        LuaStateFixture f;

        // type_name returns StringName, compare as String
        CHECK(String(f.state->type_name(LUA_TNIL)) == "nil");
        CHECK(String(f.state->type_name(LUA_TNUMBER)) == "number");
        CHECK(String(f.state->type_name(LUA_TBOOLEAN)) == "boolean");
        CHECK(String(f.state->type_name(LUA_TSTRING)) == "string");
        CHECK(String(f.state->type_name(LUA_TTABLE)) == "table");
        CHECK(String(f.state->type_name(LUA_TFUNCTION)) == "function");
        CHECK(String(f.state->type_name(LUA_TVECTOR)) == "vector");
    }

    TEST_CASE("type_name_for_value - returns type name for stack value")
    {
        LuaStateFixture f;

        f.state->push_nil();
        CHECK(String(f.state->type_name_for_value(-1)) == "nil");
        f.state->pop(1);

        f.state->push_number(42.0);
        CHECK(String(f.state->type_name_for_value(-1)) == "number");
        f.state->pop(1);

        f.state->push_string("test");
        CHECK(String(f.state->type_name_for_value(-1)) == "string");
        f.state->pop(1);
    }
}

TEST_SUITE("LuaState - Comparison")
{
    TEST_CASE("equal - compares values with metamethods")
    {
        LuaStateFixture f;

        f.state->push_number(42.0);
        f.state->push_number(42.0);
        CHECK(f.state->equal(-1, -2));
        f.state->pop(2);

        f.state->push_number(42.0);
        f.state->push_number(43.0);
        CHECK_FALSE(f.state->equal(-1, -2));
        f.state->pop(2);

        f.state->push_string("test");
        f.state->push_string("test");
        CHECK(f.state->equal(-1, -2));
        f.state->pop(2);
    }

    TEST_CASE("raw_equal - compares without metamethods")
    {
        LuaStateFixture f;

        f.state->push_number(42.0);
        f.state->push_number(42.0);
        CHECK(f.state->raw_equal(-1, -2));
        f.state->pop(2);

        f.state->push_string("test");
        f.state->push_string("test");
        CHECK(f.state->raw_equal(-1, -2));
        f.state->pop(2);
    }

    TEST_CASE("less_than - compares values")
    {
        LuaStateFixture f;

        f.state->push_number(1.0);
        f.state->push_number(2.0);
        CHECK(f.state->less_than(-2, -1));       // 1 < 2
        CHECK_FALSE(f.state->less_than(-1, -2)); // 2 < 1
        f.state->pop(2);
    }
}
