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

    TEST_CASE_FIXTURE(LuaStateFixture, "open_libs - opens standard libraries")
    {
        // Verify standard library is available (already opened in fixture)
        state->get_global("print");
        CHECK(state->is_function(-1));
        state->pop(1);

        state->get_global("table");
        CHECK(state->is_table(-1));
        state->pop(1);

        state->get_global("math");
        CHECK(state->is_table(-1));
        state->pop(1);
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

    TEST_CASE_FIXTURE(LuaStateFixture, "is_main_thread - identifies main thread")
    {
        MESSAGE("Checking is_main_thread on original state");
        CHECK(state->is_main_thread());

        MESSAGE("Creating new thread");
        Ref<LuaState> thread = state->new_thread();
        MESSAGE("Checking is_main_thread on new state");
        CHECK_FALSE(thread->is_main_thread());
        MESSAGE("Verifying main thread reference on new state");
        CHECK(thread->get_main_thread() == state);

        MESSAGE("Popping thread from stack");
        state->pop(1); // Pop thread from stack
    }
}

TEST_SUITE("LuaState - Stack Manipulation")
{
    TEST_CASE_FIXTURE(LuaStateFixture, "get_top - returns stack size")
    {
        CHECK(state->get_top() == 0);

        state->push_number(1.0);
        CHECK(state->get_top() == 1);

        state->push_number(2.0);
        CHECK(state->get_top() == 2);

        state->push_string("test");
        CHECK(state->get_top() == 3);

        state->pop(3);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "set_top - sets stack size")
    {
        state->push_number(1.0);
        state->push_number(2.0);
        state->push_number(3.0);
        CHECK(state->get_top() == 3);

        state->set_top(1);
        CHECK(state->get_top() == 1);
        CHECK(state->to_number(-1) == 1.0);

        state->set_top(0);
        CHECK(state->get_top() == 0);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "pop - removes elements from stack")
    {
        state->push_number(1.0);
        state->push_number(2.0);
        state->push_number(3.0);
        CHECK(state->get_top() == 3);

        state->pop(1);
        CHECK(state->get_top() == 2);
        CHECK(state->to_number(-1) == 2.0);

        state->pop(2);
        CHECK(state->get_top() == 0);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_value - duplicates stack value")
    {
        state->push_number(42.0);
        CHECK(state->get_top() == 1);

        state->push_value(-1); // Duplicate top
        CHECK(state->get_top() == 2);
        CHECK(state->to_number(-1) == 42.0);
        CHECK(state->to_number(-2) == 42.0);

        state->pop(2);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "remove - removes element from middle of stack")
    {
        state->push_number(1.0);
        state->push_number(2.0);
        state->push_number(3.0);

        state->remove(2); // Remove middle element
        CHECK(state->get_top() == 2);
        CHECK(state->to_number(1) == 1.0);
        CHECK(state->to_number(2) == 3.0);

        state->pop(2);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "insert - inserts element into stack")
    {
        state->push_number(1.0);
        state->push_number(2.0);
        state->push_number(99.0); // Value to insert

        state->insert(2); // Insert before index 2
        CHECK(state->get_top() == 3);
        CHECK(state->to_number(1) == 1.0);
        CHECK(state->to_number(2) == 99.0);
        CHECK(state->to_number(3) == 2.0);

        state->pop(3);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "replace - replaces element in stack")
    {
        state->push_number(1.0);
        state->push_number(2.0);
        state->push_number(99.0); // Replacement value

        state->replace(1); // Replace index 1
        CHECK(state->get_top() == 2);
        CHECK(state->to_number(1) == 99.0);
        CHECK(state->to_number(2) == 2.0);

        state->pop(2);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "abs_index - converts relative to absolute index")
    {
        state->push_number(1.0);
        state->push_number(2.0);
        state->push_number(3.0);

        CHECK(state->abs_index(-1) == 3);
        CHECK(state->abs_index(-2) == 2);
        CHECK(state->abs_index(-3) == 1);
        CHECK(state->abs_index(1) == 1);
        CHECK(state->abs_index(2) == 2);

        state->pop(3);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "check_stack - ensures stack space")
    {
        CHECK(state->check_stack(10));
        CHECK(state->check_stack(100));
        CHECK(state->check_stack(1000));
    }
}

TEST_SUITE("LuaState - Push Functions")
{
    TEST_CASE_FIXTURE(LuaStateFixture, "push_nil")
    {
        state->push_nil();
        CHECK(state->is_nil(-1));
        CHECK(state->type(-1) == LUA_TNIL);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_number")
    {
        state->push_number(42.5);
        CHECK(state->is_number(-1));
        CHECK(state->to_number(-1) == 42.5);

        state->pop(1);

        state->push_number(-123.456);
        CHECK(state->to_number(-1) == -123.456);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_integer")
    {
        state->push_integer(42);
        CHECK(state->is_number(-1));
        CHECK(state->to_integer(-1) == 42);
        CHECK(state->to_number(-1) == 42.0);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_vector3")
    {
        Vector3 v(1.5, 2.5, 3.5);
        state->push_vector3(v);
        CHECK(state->is_vector(-1));

        Vector3 result = state->to_vector(-1);
        CHECK(result.x == 1.5);
        CHECK(result.y == 2.5);
        CHECK(result.z == 3.5);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_string")
    {
        state->push_string("Hello, World!");
        CHECK(state->is_string(-1));
        CHECK(state->to_string_inplace(-1) == "Hello, World!");

        state->pop(1);

        // Empty string
        state->push_string("");
        CHECK(state->is_string(-1));
        CHECK(state->to_string_inplace(-1) == "");

        state->pop(1);

        // String with special characters
        state->push_string("Line 1\nLine 2\tTab");
        CHECK(state->to_string_inplace(-1) == "Line 1\nLine 2\tTab");

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_string_name")
    {
        state->push_string_name(StringName("test_name"));
        CHECK(state->is_string(-1));
        CHECK(state->to_stringname(-1) == StringName("test_name"));

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_boolean")
    {
        state->push_boolean(true);
        CHECK(state->is_boolean(-1));
        CHECK(state->to_boolean(-1) == true);

        state->pop(1);

        state->push_boolean(false);
        CHECK(state->is_boolean(-1));
        CHECK(state->to_boolean(-1) == false);

        state->pop(1);
    }
}

TEST_SUITE("LuaState - Type Checking")
{
    TEST_CASE_FIXTURE(LuaStateFixture, "type - returns lua_Type")
    {
        state->push_nil();
        CHECK(state->type(-1) == LUA_TNIL);
        state->pop(1);

        state->push_number(42.0);
        CHECK(state->type(-1) == LUA_TNUMBER);
        state->pop(1);

        state->push_string("test");
        CHECK(state->type(-1) == LUA_TSTRING);
        state->pop(1);

        state->push_boolean(true);
        CHECK(state->type(-1) == LUA_TBOOLEAN);
        state->pop(1);

        state->create_table();
        CHECK(state->type(-1) == LUA_TTABLE);
        state->pop(1);

        state->push_vector3(Vector3(1, 2, 3));
        CHECK(state->type(-1) == LUA_TVECTOR);
        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "is_* functions")
    {
        state->push_number(42.0);
        CHECK(state->is_number(-1));
        CHECK(state->is_string(-1)); // numbers are always convertible to strings in Lua
        CHECK_FALSE(state->is_boolean(-1));
        CHECK_FALSE(state->is_nil(-1));
        state->pop(1);

        state->push_string("test");
        CHECK(state->is_string(-1));
        CHECK_FALSE(state->is_number(-1));
        state->pop(1);

        state->push_boolean(true);
        CHECK(state->is_boolean(-1));
        CHECK_FALSE(state->is_number(-1));
        state->pop(1);

        state->push_nil();
        CHECK(state->is_nil(-1));
        CHECK_FALSE(state->is_number(-1));
        state->pop(1);

        state->create_table();
        CHECK(state->is_table(-1));
        CHECK_FALSE(state->is_function(-1));
        state->pop(1);

        state->push_vector3(Vector3(1, 2, 3));
        CHECK(state->is_vector(-1));
        CHECK_FALSE(state->is_number(-1));
        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "is_none and is_none_or_nil")
    {
        // Valid index with nil
        state->push_nil();
        CHECK_FALSE(state->is_none(-1));
        CHECK(state->is_none_or_nil(-1));
        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "type_name - returns type name")
    {
        // type_name returns StringName, compare as String
        CHECK(String(state->type_name(LUA_TNIL)) == "nil");
        CHECK(String(state->type_name(LUA_TNUMBER)) == "number");
        CHECK(String(state->type_name(LUA_TBOOLEAN)) == "boolean");
        CHECK(String(state->type_name(LUA_TSTRING)) == "string");
        CHECK(String(state->type_name(LUA_TTABLE)) == "table");
        CHECK(String(state->type_name(LUA_TFUNCTION)) == "function");
        CHECK(String(state->type_name(LUA_TVECTOR)) == "vector");
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "type_name_for_value - returns type name for stack value")
    {
        state->push_nil();
        CHECK(String(state->type_name_for_value(-1)) == "nil");
        state->pop(1);

        state->push_number(42.0);
        CHECK(String(state->type_name_for_value(-1)) == "number");
        state->pop(1);

        state->push_string("test");
        CHECK(String(state->type_name_for_value(-1)) == "string");
        state->pop(1);
    }
}

TEST_SUITE("LuaState - Comparison")
{
    TEST_CASE_FIXTURE(LuaStateFixture, "equal - compares values with metamethods")
    {
        state->push_number(42.0);
        state->push_number(42.0);
        CHECK(state->equal(-1, -2));
        state->pop(2);

        state->push_number(42.0);
        state->push_number(43.0);
        CHECK_FALSE(state->equal(-1, -2));
        state->pop(2);

        state->push_string("test");
        state->push_string("test");
        CHECK(state->equal(-1, -2));
        state->pop(2);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "raw_equal - compares without metamethods")
    {
        state->push_number(42.0);
        state->push_number(42.0);
        CHECK(state->raw_equal(-1, -2));
        state->pop(2);

        state->push_string("test");
        state->push_string("test");
        CHECK(state->raw_equal(-1, -2));
        state->pop(2);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "less_than - compares values")
    {
        state->push_number(1.0);
        state->push_number(2.0);
        CHECK(state->less_than(-2, -1));       // 1 < 2
        CHECK_FALSE(state->less_than(-1, -2)); // 2 < 1
        state->pop(2);
    }
}
