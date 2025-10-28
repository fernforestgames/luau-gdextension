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
        int initial_top = state->gettop();
        CHECK(initial_top == 0);

        // Try to pop 1 element from empty stack
        state->pop(1);

        // Should still be empty (operation was rejected)
        CHECK(state->gettop() == 0);
    }

    SUBCASE("Pop more items than available")
    {
        state->pushinteger(42);
        state->pushinteger(100);
        CHECK(state->gettop() == 2);

        // Try to pop 5 items when only 2 exist
        state->pop(5);

        // Stack should be unchanged (operation was rejected)
        CHECK(state->gettop() == 2);
    }

    SUBCASE("Pop negative count")
    {
        state->pushinteger(1);
        state->pushinteger(2);
        int top = state->gettop();

        // Try to pop negative number of items
        state->pop(-1);

        // Stack should be unchanged
        CHECK(state->gettop() == top);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Boundary checks: Invalid index protection")
{

    SUBCASE("Pushvalue with invalid positive index")
    {
        state->pushinteger(1);
        state->pushinteger(2);
        int top = state->gettop();

        // Try to push index 10 when only 2 items exist
        state->pushvalue(10);

        // Stack should be unchanged
        CHECK(state->gettop() == top);
    }

    SUBCASE("Pushvalue with invalid negative index")
    {
        state->pushinteger(1);
        int top = state->gettop();

        // Try to push index -10 when only 1 item exists
        state->pushvalue(-10);

        // Stack should be unchanged
        CHECK(state->gettop() == top);
    }

    SUBCASE("Pushvalue with index 0")
    {
        state->pushinteger(42);
        int top = state->gettop();

        // Index 0 is never valid in Lua
        state->pushvalue(0);

        // Stack should be unchanged
        CHECK(state->gettop() == top);
    }

    SUBCASE("Remove with invalid index")
    {
        state->pushinteger(1);
        state->pushinteger(2);
        int top = state->gettop();

        // Try to remove index 5
        state->remove(5);

        // Stack should be unchanged
        CHECK(state->gettop() == top);
    }

    SUBCASE("Replace on empty stack")
    {
        CHECK(state->gettop() == 0);

        // Try to replace when there's nothing to replace with
        state->replace(1);

        // Should still be empty
        CHECK(state->gettop() == 0);
    }

    SUBCASE("Insert on empty stack")
    {
        CHECK(state->gettop() == 0);

        // Try to insert when there's nothing to insert
        state->insert(1);

        // Should still be empty
        CHECK(state->gettop() == 0);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Boundary checks: Settop with invalid indices")
{

    SUBCASE("Settop with invalid negative index")
    {
        state->pushinteger(1);
        state->pushinteger(2);
        CHECK(state->gettop() == 2);

        // Try to set top to -10 (invalid: only 2 items on stack)
        state->settop(-10);

        // Stack should be unchanged
        CHECK(state->gettop() == 2);
    }

    SUBCASE("Settop to zero clears stack")
    {
        state->pushinteger(1);
        state->pushinteger(2);
        state->pushinteger(3);

        state->settop(0);

        // Stack should be empty
        CHECK(state->gettop() == 0);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Boundary checks: Table operations without prerequisites")
{

    SUBCASE("gettable without key on stack")
    {
        exec_lua("t = {a = 1}");
        state->getglobal("t");
        int top_with_table = state->gettop();

        // Try gettable without pushing a key first
        state->pop(1); // Remove the table
        CHECK(state->gettop() == 0);

        state->gettable(1); // No key, no table

        // Should not crash, stack unchanged
        CHECK(state->gettop() == 0);
    }

    SUBCASE("settable without key and value on stack")
    {
        exec_lua("t = {}");
        state->getglobal("t");

        // settable needs key and value, we only have table
        state->settable(1);

        // Should not crash
        CHECK(state->gettop() == 1);
    }

    SUBCASE("settable with only key, no value")
    {
        exec_lua("t = {}");
        state->getglobal("t");
        state->pushstring("key");

        // settable needs both key and value
        state->settable(-2);

        // Should not crash, stack should still have table and key
        CHECK(state->gettop() == 2);
    }

    SUBCASE("setfield without value on stack")
    {
        exec_lua("t = {}");
        state->getglobal("t");
        state->pop(1); // Remove table

        // No value on stack
        state->setfield(1, "key");

        // Should not crash
        CHECK(state->gettop() == 0);
    }

    SUBCASE("rawget without key")
    {
        exec_lua("t = {a = 1}");
        state->getglobal("t");
        state->pop(1);

        // No key on stack
        state->rawget(1);

        // Should not crash
        CHECK(state->gettop() == 0);
    }

    SUBCASE("rawset without key and value")
    {
        exec_lua("t = {}");
        state->getglobal("t");
        state->pop(1);

        // No key or value on stack
        state->rawset(1);

        // Should not crash
        CHECK(state->gettop() == 0);
    }

    SUBCASE("rawseti without value")
    {
        exec_lua("t = {}");
        state->getglobal("t");
        state->pop(1);

        // No value on stack
        state->rawseti(1, 1);

        // Should not crash
        CHECK(state->gettop() == 0);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Boundary checks: Metatable operations")
{

    SUBCASE("getmetatable with invalid index")
    {
        state->pushinteger(42);

        // Try to get metatable of non-existent index
        bool result = state->getmetatable(5);

        // Should return false and not crash
        CHECK_FALSE(result);
    }

    SUBCASE("setmetatable without metatable on stack")
    {
        state->pushinteger(42);

        // Try to setmetatable without pushing a metatable first
        state->pop(1); // Empty stack

        bool result = state->setmetatable(1);

        // Should return false and not crash
        CHECK_FALSE(result);
        CHECK(state->gettop() == 0);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Boundary checks: Function call operations")
{

    SUBCASE("call without function on stack")
    {
        // Empty stack, try to call
        state->call(0, 0);

        // Should not crash
        CHECK(state->gettop() == 0);
    }

    SUBCASE("call without enough arguments")
    {
        exec_lua("function f(a, b, c) return a + b + c end");
        state->getglobal("f");

        // Function expects 3 args, but we claim to pass 5
        state->call(5, 1);

        // Should not crash (rejected due to insufficient items)
        // Stack should still have the function
        CHECK(state->gettop() == 1);
    }

    SUBCASE("call with negative nargs")
    {
        exec_lua("function f() return 42 end");
        state->getglobal("f");

        // Negative nargs is invalid
        state->call(-1, 1);

        // Should not crash
        CHECK(state->gettop() == 1);
    }

    SUBCASE("pcall without function on stack")
    {
        // Empty stack, try to pcall
        lua_Status status = state->pcall(0, 0, 0);

        // Should return error status, not crash
        CHECK(status == LUA_ERRMEM);
        CHECK(state->gettop() == 0);
    }

    SUBCASE("pcall with invalid error function index")
    {
        exec_lua("function f() return 42 end");
        state->getglobal("f");

        // Invalid errfunc index
        lua_Status status = state->pcall(0, 1, 10);

        // Should return error status, not crash
        CHECK(status == LUA_ERRMEM);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Boundary checks: Operations work correctly when valid")
{

    SUBCASE("Valid pop operations")
    {
        state->pushinteger(1);
        state->pushinteger(2);
        state->pushinteger(3);
        CHECK(state->gettop() == 3);

        state->pop(2);
        CHECK(state->gettop() == 1);
        CHECK(state->tointeger(-1) == 1);
    }

    SUBCASE("Valid pushvalue")
    {
        state->pushinteger(42);
        state->pushvalue(-1); // Duplicate top

        CHECK(state->gettop() == 2);
        CHECK(state->tointeger(-1) == 42);
        CHECK(state->tointeger(-2) == 42);
    }

    SUBCASE("Valid table operations")
    {
        state->newtable();
        state->pushstring("key");
        state->pushinteger(123);
        state->settable(-3);

        // Verify the value was set
        state->pushstring("key");
        state->gettable(-2);
        CHECK(state->tointeger(-1) == 123);
    }

    SUBCASE("Valid function call")
    {
        exec_lua("function add(a, b) return a + b end");
        state->getglobal("add");
        state->pushinteger(10);
        state->pushinteger(20);

        state->call(2, 1);

        CHECK(state->tointeger(-1) == 30);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Boundary checks: Edge cases in index validation")
{

    SUBCASE("Pseudo-indices should work")
    {
        // LUA_GLOBALSINDEX and other pseudo-indices are valid
        state->pushvalue(LUA_GLOBALSINDEX);

        // Should push the globals table
        CHECK(state->istable(-1));
    }

    SUBCASE("Valid negative indices")
    {
        state->pushinteger(1);
        state->pushinteger(2);
        state->pushinteger(3);

        // -1 is top (3), -2 is second from top (2), -3 is third (1)
        CHECK(state->tointeger(-1) == 3);
        CHECK(state->tointeger(-2) == 2);
        CHECK(state->tointeger(-3) == 1);

        // -4 should be invalid (only 3 items)
        state->pushvalue(-4);
        CHECK(state->gettop() == 3); // Unchanged
    }
}
