// Tests for bridging/callable - Callable bridging between Godot and Lua
// to_callable, push_callable, LuaCallable

#include "doctest.h"
#include "test_fixtures.h"
#include "lua_state.h"

using namespace gdluau;
using namespace godot;

TEST_SUITE("Bridging - Callable")
{
    TEST_CASE_FIXTURE(LuaStateFixture, "push_callable and to_callable - basic round-trip")
    {
        // Create a simple callable (using a Lua function)
        exec_lua_ok(R"(
            function test_func()
                return 42
            end
        )");

        state->get_global("test_func");
        CHECK(state->type(-1) == LUA_TFUNCTION);

        Callable callable = state->to_callable(-1);
        state->pop(1);
        CHECK(callable.is_valid());

        // Push it back
        state->push_callable(callable);
        bool is_func_or_userdata = state->is_function(-1) || state->is_userdata(-1);
        CHECK(is_func_or_userdata);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_callable - from Lua function")
    {
        exec_lua_ok(R"(
            function add(a, b)
                return a + b
            end
        )");

        state->get_global("add");
        Callable callable = state->to_callable(-1);
        state->pop(1);

        CHECK(callable.is_valid());

        // Call the callable
        Array args;
        args.push_back(10);
        args.push_back(20);

        Variant result = callable.callv(args);
        CHECK(static_cast<int>(result) == 30);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_callable - with captured state")
    {
        exec_lua_ok(R"(
            local counter = 0
            function increment()
                counter = counter + 1
                return counter
            end
        )");

        state->get_global("increment");
        Callable callable = state->to_callable(-1);
        state->pop(1);

        // Call multiple times
        Variant result1 = callable.call();
        Variant result2 = callable.call();
        Variant result3 = callable.call();

        CHECK(static_cast<int>(result1) == 1);
        CHECK(static_cast<int>(result2) == 2);
        CHECK(static_cast<int>(result3) == 3);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_callable - multiple return values")
    {
        exec_lua_ok(R"(
            function multi_return()
                return 1, 2, 3
            end
        )");

        state->get_global("multi_return");
        Callable callable = state->to_callable(-1);
        state->pop(1);

        // Callables return only the first value
        Variant result = callable.call();
        CHECK(static_cast<int>(result) == 1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_callable - with no return value")
    {
        exec_lua_ok(R"(
            function no_return()
                -- Does nothing
            end
        )");

        state->get_global("no_return");
        Callable callable = state->to_callable(-1);
        state->pop(1);

        Variant result = callable.call();
        CHECK(result.get_type() == Variant::NIL);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_callable - error handling")
    {
        exec_lua_ok(R"(
            function error_func()
                error("intentional error")
            end
        )");

        state->get_global("error_func");
        Callable callable = state->to_callable(-1);
        state->pop(1);

        // Calling should not crash, returns nil on error
        Variant result = callable.call();
        // Error should be printed, result should be nil
        CHECK(result.get_type() == Variant::NIL);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_callable - Godot callable to Lua")
    {
        // Create a mock callable (we can't easily create a real one without an Object)
        // Test that pushing doesn't crash
        Callable empty_callable;
        state->push_callable(empty_callable);

        // Should push something to stack
        CHECK(state->get_top() == 1);

        state->pop(1);
    }

    TEST_CASE("callable - persists across LuaState lifetime")
    {
        Callable callable;

        {
            LuaStateFixture f;

            f.exec_lua_ok(R"(
                function persistent_func()
                    return 99
                end
            )");

            f.state->get_global("persistent_func");
            callable = f.state->to_callable(-1);
            f.state->pop(1);
        }

        // LuaState is now destroyed
        CHECK(!callable.is_valid());

        // Calling should return nil (state closed)
        Variant result = callable.call();
        CHECK(result.get_type() == Variant::NIL);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "integration - passing Lua function to Lua")
    {
        exec_lua_ok(R"(
            function apply(func, value)
                return func(value)
            end

            function double(x)
                return x * 2
            end
        )");

        // Get the callable
        state->get_global("double");
        Callable double_callable = state->to_callable(-1);
        state->pop(1);

        // Call apply with the callable
        state->get_global("apply");
        state->push_callable(double_callable);
        state->push_number(21.0);
        state->pcall(2, 1);

        CHECK(state->to_number(-1) == 42.0);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_callable - Lua function with varargs")
    {
        exec_lua_ok(R"(
            function sum(...)
                local total = 0
                local args = {...}
                for i = 1, #args do
                    total = total + args[i]
                end
                return total
            end
        )");

        state->get_global("sum");
        Callable callable = state->to_callable(-1);
        state->pop(1);

        CHECK(callable.is_valid());

        // Call with varying numbers of arguments
        Array args0;
        Variant result0 = callable.callv(args0);
        CHECK(static_cast<int>(result0) == 0);

        Array args1;
        args1.push_back(5);
        Variant result1 = callable.callv(args1);
        CHECK(static_cast<int>(result1) == 5);

        Array args3;
        args3.push_back(10);
        args3.push_back(20);
        args3.push_back(30);
        Variant result3 = callable.callv(args3);
        CHECK(static_cast<int>(result3) == 60);

        Array args5;
        args5.push_back(1);
        args5.push_back(2);
        args5.push_back(3);
        args5.push_back(4);
        args5.push_back(5);
        Variant result5 = callable.callv(args5);
        CHECK(static_cast<int>(result5) == 15);
    }
}
