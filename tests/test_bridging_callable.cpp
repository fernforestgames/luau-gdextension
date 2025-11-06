// Tests for bridging/callable - Callable bridging between Godot and Lua
// to_callable, push_callable, LuaCallable

#include "doctest.h"
#include "test_fixtures.h"
#include "lua_state.h"

using namespace godot;

TEST_SUITE("Bridging - Callable")
{
    TEST_CASE("push_callable and to_callable - basic round-trip")
    {
        LuaStateFixture f;

        // Create a simple callable (using a Lua function)
        f.exec_lua_ok(R"(
            function test_func()
                return 42
            end
        )");

        f.state->get_global("test_func");
        CHECK(f.state->type(-1) == LUA_TFUNCTION);

        Callable callable = f.state->to_callable(-1);
        f.state->pop(1);
        CHECK(callable.is_valid());

        // Push it back
        f.state->push_callable(callable);
        bool is_func_or_userdata = f.state->is_function(-1) || f.state->is_userdata(-1);
        CHECK(is_func_or_userdata);

        f.state->pop(1);
    }

    TEST_CASE("to_callable - from Lua function")
    {
        LuaStateFixture f;

        f.exec_lua_ok(R"(
            function add(a, b)
                return a + b
            end
        )");

        f.state->get_global("add");
        Callable callable = f.state->to_callable(-1);
        f.state->pop(1);

        CHECK(callable.is_valid());

        // Call the callable
        Array args;
        args.push_back(10);
        args.push_back(20);

        Variant result = callable.callv(args);
        CHECK(static_cast<int>(result) == 30);
    }

    TEST_CASE("to_callable - with captured state")
    {
        LuaStateFixture f;

        f.exec_lua_ok(R"(
            local counter = 0
            function increment()
                counter = counter + 1
                return counter
            end
        )");

        f.state->get_global("increment");
        Callable callable = f.state->to_callable(-1);
        f.state->pop(1);

        // Call multiple times
        Variant result1 = callable.call();
        Variant result2 = callable.call();
        Variant result3 = callable.call();

        CHECK(static_cast<int>(result1) == 1);
        CHECK(static_cast<int>(result2) == 2);
        CHECK(static_cast<int>(result3) == 3);
    }

    TEST_CASE("to_callable - multiple return values")
    {
        LuaStateFixture f;

        f.exec_lua_ok(R"(
            function multi_return()
                return 1, 2, 3
            end
        )");

        f.state->get_global("multi_return");
        Callable callable = f.state->to_callable(-1);
        f.state->pop(1);

        // Callables return only the first value
        Variant result = callable.call();
        CHECK(static_cast<int>(result) == 1);
    }

    TEST_CASE("to_callable - with no return value")
    {
        LuaStateFixture f;

        f.exec_lua_ok(R"(
            function no_return()
                -- Does nothing
            end
        )");

        f.state->get_global("no_return");
        Callable callable = f.state->to_callable(-1);
        f.state->pop(1);

        Variant result = callable.call();
        CHECK(result.get_type() == Variant::NIL);
    }

    TEST_CASE("to_callable - error handling")
    {
        LuaStateFixture f;

        f.exec_lua_ok(R"(
            function error_func()
                error("intentional error")
            end
        )");

        f.state->get_global("error_func");
        Callable callable = f.state->to_callable(-1);
        f.state->pop(1);

        // Calling should not crash, returns nil on error
        Variant result = callable.call();
        // Error should be printed, result should be nil
        CHECK(result.get_type() == Variant::NIL);
    }

    TEST_CASE("push_callable - Godot callable to Lua")
    {
        LuaStateFixture f;

        // Create a mock callable (we can't easily create a real one without an Object)
        // Test that pushing doesn't crash
        Callable empty_callable;
        f.state->push_callable(empty_callable);

        // Should push something to stack
        CHECK(f.state->get_top() == 1);

        f.state->pop(1);
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

    TEST_CASE("integration - passing Lua function to Lua")
    {
        LuaStateFixture f;

        f.exec_lua_ok(R"(
            function apply(func, value)
                return func(value)
            end

            function double(x)
                return x * 2
            end
        )");

        // Get the callable
        f.state->get_global("double");
        Callable double_callable = f.state->to_callable(-1);
        f.state->pop(1);

        // Call apply with the callable
        f.state->get_global("apply");
        f.state->push_callable(double_callable);
        f.state->push_number(21.0);
        f.state->call(2, 1);

        CHECK(f.state->to_number(-1) == 42.0);

        f.state->pop(1);
    }
}
