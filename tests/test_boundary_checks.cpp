// Tests for boundary checking and stack safety in LuaState
// Verifies that the GDExtension never crashes from invalid stack operations

#include "doctest.h"
#include "test_fixtures.h"
#include <godot_cpp/variant/variant.hpp>

using namespace godot;

TEST_CASE_FIXTURE(LuaStateFixture, "Boundary checks: Stack underflow protection")
{

    SUBCASE("Pop from empty stack")
    {
        // Empty stack - should not crash
        int initial_top = state->get_top();
        CHECK(initial_top == 0);

        // Try to pop 1 element from empty stack
        state->pop(1);

        // Should still be empty (operation was rejected)
        CHECK(state->get_top() == 0);
    }

    SUBCASE("Pop more items than available")
    {
        state->push_integer(42);
        state->push_integer(100);
        CHECK(state->get_top() == 2);

        // Try to pop 5 items when only 2 exist
        state->pop(5);

        // Stack should be unchanged (operation was rejected)
        CHECK(state->get_top() == 2);

        // Clean up the 2 items that are still on the stack
        state->pop(2);
    }

    SUBCASE("Pop negative count")
    {
        state->push_integer(1);
        state->push_integer(2);
        int top = state->get_top();

        // Try to pop negative number of items
        state->pop(-1);

        // Stack should be unchanged
        CHECK(state->get_top() == top);

        // Clean up the 2 items that are still on the stack
        state->pop(2);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Boundary checks: Invalid index protection")
{

    SUBCASE("Pushvalue with invalid positive index")
    {
        state->push_integer(1);
        state->push_integer(2);
        int top = state->get_top();

        // Try to push index 10 when only 2 items exist
        state->push_value(10);

        // Stack should be unchanged
        CHECK(state->get_top() == top);

        // Clean up the 2 items that are still on the stack
        state->pop(2);
    }

    SUBCASE("Pushvalue with invalid negative index")
    {
        state->push_integer(1);
        int top = state->get_top();

        // Try to push index -10 when only 1 item exists
        state->push_value(-10);

        // Stack should be unchanged
        CHECK(state->get_top() == top);

        // Clean up the item that is still on the stack
        state->pop(1);
    }

    SUBCASE("Pushvalue with index 0")
    {
        state->push_integer(42);
        int top = state->get_top();

        // Index 0 is never valid in Lua
        state->push_value(0);

        // Stack should be unchanged
        CHECK(state->get_top() == top);

        // Clean up the item that is still on the stack
        state->pop(1);
    }

    SUBCASE("Remove with invalid index")
    {
        state->push_integer(1);
        state->push_integer(2);
        int top = state->get_top();

        // Try to remove index 5
        state->remove(5);

        // Stack should be unchanged
        CHECK(state->get_top() == top);

        // Clean up the 2 items that are still on the stack
        state->pop(2);
    }

    SUBCASE("Replace on empty stack")
    {
        CHECK(state->get_top() == 0);

        // Try to replace when there's nothing to replace with
        state->replace(1);

        // Should still be empty
        CHECK(state->get_top() == 0);
    }

    SUBCASE("Insert on empty stack")
    {
        CHECK(state->get_top() == 0);

        // Try to insert when there's nothing to insert
        state->insert(1);

        // Should still be empty
        CHECK(state->get_top() == 0);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Boundary checks: Settop with invalid indices")
{

    SUBCASE("Settop with invalid negative index")
    {
        state->push_integer(1);
        state->push_integer(2);
        CHECK(state->get_top() == 2);

        // Try to set top to -10 (invalid: only 2 items on stack)
        state->set_top(-10);

        // Stack should be unchanged
        CHECK(state->get_top() == 2);

        // Clean up the 2 items that are still on the stack
        state->pop(2);
    }

    SUBCASE("Settop to zero clears stack")
    {
        state->push_integer(1);
        state->push_integer(2);
        state->push_integer(3);

        state->set_top(0);

        // Stack should be empty
        CHECK(state->get_top() == 0);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Boundary checks: Table operations without prerequisites")
{

    SUBCASE("gettable without key on stack")
    {
        exec_lua("t = {a = 1}");
        state->get_global("t");
        int top_with_table = state->get_top();

        // Try gettable without pushing a key first
        state->pop(1); // Remove the table
        CHECK(state->get_top() == 0);

        state->get_table(1); // No key, no table

        // Should not crash, stack unchanged
        CHECK(state->get_top() == 0);
    }

    SUBCASE("settable without key and value on stack")
    {
        exec_lua("t = {}");
        state->get_global("t");

        // settable needs key and value, we only have table
        state->set_table(1);

        // Should not crash
        CHECK(state->get_top() == 1);

        // Clean up the table that is still on the stack
        state->pop(1);
    }

    SUBCASE("settable with only key, no value")
    {
        exec_lua("t = {}");
        state->get_global("t");
        state->push_string("key");

        // settable needs both key and value
        state->set_table(-2);

        // Should not crash, stack should still have table and key
        CHECK(state->get_top() == 2);

        // Clean up the table and key that are still on the stack
        state->pop(2);
    }

    SUBCASE("setfield without value on stack")
    {
        exec_lua("t = {}");
        state->get_global("t");
        state->pop(1); // Remove table

        // No value on stack
        state->set_field(1, "key");

        // Should not crash
        CHECK(state->get_top() == 0);
    }

    SUBCASE("rawget without key")
    {
        exec_lua("t = {a = 1}");
        state->get_global("t");
        state->pop(1);

        // No key on stack
        state->raw_get(1);

        // Should not crash
        CHECK(state->get_top() == 0);
    }

    SUBCASE("rawset without key and value")
    {
        exec_lua("t = {}");
        state->get_global("t");
        state->pop(1);

        // No key or value on stack
        state->raw_set(1);

        // Should not crash
        CHECK(state->get_top() == 0);
    }

    SUBCASE("rawseti without value")
    {
        exec_lua("t = {}");
        state->get_global("t");
        state->pop(1);

        // No value on stack
        state->raw_seti(1, 1);

        // Should not crash
        CHECK(state->get_top() == 0);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Boundary checks: Metatable operations")
{

    SUBCASE("getmetatable with invalid index")
    {
        state->push_integer(42);

        // Try to get metatable of non-existent index
        bool result = state->get_metatable(5);

        // Should return false and not crash
        CHECK_FALSE(result);

        // Clean up the integer that is still on the stack
        state->pop(1);
    }

    SUBCASE("setmetatable without metatable on stack")
    {
        state->push_integer(42);

        // Try to setmetatable without pushing a metatable first
        state->pop(1); // Empty stack

        bool result = state->set_metatable(1);

        // Should return false and not crash
        CHECK_FALSE(result);
        CHECK(state->get_top() == 0);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Boundary checks: Function call operations")
{

    SUBCASE("call without function on stack")
    {
        // Empty stack, try to call
        state->call(0, 0);

        // Should not crash
        CHECK(state->get_top() == 0);
    }

    SUBCASE("call without enough arguments")
    {
        exec_lua("function f(a, b, c) return a + b + c end");
        state->get_global("f");

        // Function expects 3 args, but we claim to pass 5
        state->call(5, 1);

        // Should not crash (rejected due to insufficient items)
        // Stack should still have the function
        CHECK(state->get_top() == 1);

        // Clean up the function that is still on the stack
        state->pop(1);
    }

    SUBCASE("call with negative nargs")
    {
        exec_lua("function f() return 42 end");
        state->get_global("f");

        // Negative nargs is invalid
        state->call(-1, 1);

        // Should not crash
        CHECK(state->get_top() == 1);

        // Clean up the function that is still on the stack
        state->pop(1);
    }

    SUBCASE("pcall without function on stack")
    {
        // Empty stack, try to pcall
        lua_Status status = state->pcall(0, 0, 0);

        // Should return error status, not crash
        CHECK(status == LUA_ERRMEM);
        CHECK(state->get_top() == 0);
    }

    SUBCASE("pcall with invalid error function index")
    {
        exec_lua("function f() return 42 end");
        state->get_global("f");

        // Invalid errfunc index
        lua_Status status = state->pcall(0, 1, 10);

        // Should return error status, not crash
        CHECK(status == LUA_ERRMEM);

        // Clean up whatever is left on the stack after failed pcall
        state->set_top(0);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Boundary checks: Operations work correctly when valid")
{

    SUBCASE("Valid pop operations")
    {
        state->push_integer(1);
        state->push_integer(2);
        state->push_integer(3);
        CHECK(state->get_top() == 3);

        state->pop(2);
        CHECK(state->get_top() == 1);
        CHECK(state->to_integer(-1) == 1);

        // Clean up the remaining item on the stack
        state->pop(1);
    }

    SUBCASE("Valid pushvalue")
    {
        state->push_integer(42);
        state->push_value(-1); // Duplicate top

        CHECK(state->get_top() == 2);
        CHECK(state->to_integer(-1) == 42);
        CHECK(state->to_integer(-2) == 42);

        // Clean up both values (original and duplicate)
        state->pop(2);
    }

    SUBCASE("Valid table operations")
    {
        state->new_table();
        state->push_string("key");
        state->push_integer(123);
        state->set_table(-3);

        // Verify the value was set
        state->push_string("key");
        state->get_table(-2);
        CHECK(state->to_integer(-1) == 123);

        // Clean up the retrieved value and the table
        state->pop(2);
    }

    SUBCASE("Valid function call")
    {
        exec_lua("function add(a, b) return a + b end");
        state->get_global("add");
        state->push_integer(10);
        state->push_integer(20);

        state->call(2, 1);

        CHECK(state->to_integer(-1) == 30);

        // Clean up the return value
        state->pop(1);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Boundary checks: Edge cases in index validation")
{

    SUBCASE("Pseudo-indices should work")
    {
        // LUA_GLOBALSINDEX and other pseudo-indices are valid
        state->push_value(LUA_GLOBALSINDEX);

        // Should push the globals table
        CHECK(state->is_table(-1));

        // Clean up the globals table
        state->pop(1);
    }

    SUBCASE("Valid negative indices")
    {
        state->push_integer(1);
        state->push_integer(2);
        state->push_integer(3);

        // -1 is top (3), -2 is second from top (2), -3 is third (1)
        CHECK(state->to_integer(-1) == 3);
        CHECK(state->to_integer(-2) == 2);
        CHECK(state->to_integer(-3) == 1);

        // -4 should be invalid (only 3 items)
        state->push_value(-4);
        CHECK(state->get_top() == 3); // Unchanged

        // Clean up all 3 items that are still on the stack
        state->pop(3);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Stack manipulation: abs_index")
{
    SUBCASE("Converts negative indices to absolute")
    {
        state->push_number(1.0);
        state->push_number(2.0);
        state->push_number(3.0);

        // Positive indices are already absolute
        CHECK(state->abs_index(1) == 1);
        CHECK(state->abs_index(2) == 2);
        CHECK(state->abs_index(3) == 3);

        // Negative indices get converted
        CHECK(state->abs_index(-1) == 3);
        CHECK(state->abs_index(-2) == 2);
        CHECK(state->abs_index(-3) == 1);

        state->pop(3);
    }

    SUBCASE("Handles pseudo-indices correctly")
    {
        // Pseudo-indices should be returned as-is
        CHECK(state->abs_index(LUA_GLOBALSINDEX) == LUA_GLOBALSINDEX);
        CHECK(state->abs_index(LUA_REGISTRYINDEX) == LUA_REGISTRYINDEX);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Stack manipulation: obj_len")
{
    SUBCASE("Returns correct length for strings")
    {
        state->push_string("hello");
        CHECK(state->obj_len(-1) == 5);
        state->pop(1);

        state->push_string("");
        CHECK(state->obj_len(-1) == 0);
        state->pop(1);
    }

    SUBCASE("Returns correct length for tables")
    {
        exec_lua("return {1, 2, 3, 4, 5}");
        CHECK(state->obj_len(-1) == 5);
        state->pop(1);

        state->new_table();
        CHECK(state->obj_len(-1) == 0);
        state->pop(1);
    }

    SUBCASE("Handles invalid indices safely")
    {
        // Invalid index should return 0 and not crash
        int len = state->obj_len(10);
        CHECK(len == 0);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Comparisons: equal")
{
    SUBCASE("Compares equal numbers")
    {
        state->push_number(42);
        state->push_number(42);
        CHECK(state->equal(-1, -2));
        state->pop(2);
    }

    SUBCASE("Compares unequal numbers")
    {
        state->push_number(1);
        state->push_number(2);
        CHECK_FALSE(state->equal(-1, -2));
        state->pop(2);
    }

    SUBCASE("Compares equal strings")
    {
        state->push_string("test");
        state->push_string("test");
        CHECK(state->equal(-1, -2));
        state->pop(2);
    }

    SUBCASE("Handles invalid indices safely")
    {
        state->push_number(1);
        // One valid, one invalid index
        CHECK_FALSE(state->equal(-1, 10));
        state->pop(1);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Comparisons: raw_equal")
{
    SUBCASE("Compares equal numbers")
    {
        state->push_number(42);
        state->push_number(42);
        CHECK(state->raw_equal(-1, -2));
        state->pop(2);
    }

    SUBCASE("Different tables are not equal")
    {
        state->new_table();
        state->new_table();
        CHECK_FALSE(state->raw_equal(-1, -2));
        state->pop(2);
    }

    SUBCASE("Same table reference is equal")
    {
        state->new_table();
        state->push_value(-1);
        CHECK(state->raw_equal(-1, -2));
        state->pop(2);
    }

    SUBCASE("Handles invalid indices safely")
    {
        state->push_number(1);
        CHECK_FALSE(state->raw_equal(-1, 10));
        state->pop(1);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Comparisons: less_than")
{
    SUBCASE("Compares numbers")
    {
        state->push_number(1);
        state->push_number(2);
        CHECK(state->less_than(-2, -1));
        CHECK_FALSE(state->less_than(-1, -2));
        state->pop(2);
    }

    SUBCASE("Compares strings")
    {
        state->push_string("abc");
        state->push_string("def");
        CHECK(state->less_than(-2, -1));
        CHECK_FALSE(state->less_than(-1, -2));
        state->pop(2);
    }

    SUBCASE("Handles invalid indices safely")
    {
        state->push_number(1);
        CHECK_FALSE(state->less_than(-1, 10));
        state->pop(1);
    }
}
