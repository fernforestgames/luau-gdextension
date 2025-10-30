// Tests for Callable stack manipulation
// Verifies that calling Godot Callables from Lua doesn't leak stack space

#include "doctest.h"
#include "test_fixtures.h"
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/array.hpp>

using namespace godot;

// Helper functions for creating test Callables
namespace {

// Helper callable that returns nil
Variant callable_returns_nil() {
    return Variant();
}

// Helper callable that returns an integer
Variant callable_returns_int() {
    return Variant(42);
}

// Helper callable that returns a string
Variant callable_returns_string() {
    return Variant(String("hello"));
}

// Helper callable that echoes its first argument
Variant callable_echo(Variant arg) {
    return arg;
}

// Helper callable that sums two numbers
Variant callable_add(int a, int b) {
    return Variant(a + b);
}

} // anonymous namespace

TEST_CASE_FIXTURE(LuaStateFixture, "Callable stack: Basic invocation with no return")
{
    SUBCASE("Single call with nil return")
    {
        Callable c = callable_mp_static(&callable_returns_nil);
        state->push_variant(Variant(c));
        state->set_global("my_callable");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            local result = my_callable()
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("Multiple calls with nil return")
    {
        Callable c = callable_mp_static(&callable_returns_nil);
        state->push_variant(Variant(c));
        state->set_global("my_callable");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            my_callable()
            my_callable()
            my_callable()
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("Call in loop (100 iterations)")
    {
        Callable c = callable_mp_static(&callable_returns_nil);
        state->push_variant(Variant(c));
        state->set_global("my_callable");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            for i = 1, 100 do
                my_callable()
            end
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Callable stack: Basic invocation with return value")
{
    SUBCASE("Single call returning integer")
    {
        Callable c = callable_mp_static(&callable_returns_int);
        state->push_variant(Variant(c));
        state->set_global("my_callable");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            local result = my_callable()
            assert(result == 42)
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("Multiple sequential calls returning integer")
    {
        Callable c = callable_mp_static(&callable_returns_int);
        state->push_variant(Variant(c));
        state->set_global("my_callable");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            local r1 = my_callable()
            local r2 = my_callable()
            local r3 = my_callable()
            assert(r1 == 42 and r2 == 42 and r3 == 42)
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("Call in loop with result capture (100 iterations)")
    {
        Callable c = callable_mp_static(&callable_returns_int);
        state->push_variant(Variant(c));
        state->set_global("my_callable");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            for i = 1, 100 do
                local result = my_callable()
                assert(result == 42)
            end
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("Call in loop without capturing result")
    {
        Callable c = callable_mp_static(&callable_returns_int);
        state->push_variant(Variant(c));
        state->set_global("my_callable");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            for i = 1, 100 do
                my_callable()  -- Result not captured
            end
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("Call returning string")
    {
        Callable c = callable_mp_static(&callable_returns_string);
        state->push_variant(Variant(c));
        state->set_global("my_callable");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            local result = my_callable()
            assert(result == "hello")
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Callable stack: Calls with arguments")
{
    SUBCASE("Echo callable with single argument")
    {
        Callable c = callable_mp_static(&callable_echo);
        state->push_variant(Variant(c));
        state->set_global("echo");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            local r1 = echo(10)
            assert(r1 == 10)
            local r2 = echo("test")
            assert(r2 == "test")
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("Echo in loop with varying arguments")
    {
        Callable c = callable_mp_static(&callable_echo);
        state->push_variant(Variant(c));
        state->set_global("echo");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            for i = 1, 50 do
                local result = echo(i)
                assert(result == i)
            end
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("Add callable with two arguments")
    {
        Callable c = callable_mp_static(&callable_add);
        state->push_variant(Variant(c));
        state->set_global("add");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            local r1 = add(5, 10)
            assert(r1 == 15)
            local r2 = add(100, 200)
            assert(r2 == 300)
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("Add in loop with varying arguments")
    {
        Callable c = callable_mp_static(&callable_add);
        state->push_variant(Variant(c));
        state->set_global("add");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            for i = 1, 50 do
                local result = add(i, i)
                assert(result == i * 2)
            end
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Callable stack: Nested and complex calls")
{
    SUBCASE("Nested callable invocations")
    {
        Callable echo = callable_mp_static(&callable_echo);
        Callable add = callable_mp_static(&callable_add);
        state->push_variant(Variant(echo));
        state->set_global("echo");
        state->push_variant(Variant(add));
        state->set_global("add");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            local result = echo(add(5, 10))
            assert(result == 15)
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("Multiple nested calls in loop")
    {
        Callable echo = callable_mp_static(&callable_echo);
        Callable add = callable_mp_static(&callable_add);
        state->push_variant(Variant(echo));
        state->set_global("echo");
        state->push_variant(Variant(add));
        state->set_global("add");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            for i = 1, 50 do
                local result = echo(add(i, i))
                assert(result == i * 2)
            end
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("Callable called from table")
    {
        Callable c = callable_mp_static(&callable_returns_int);
        state->push_variant(Variant(c));
        state->set_global("my_callable");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            local t = { func = my_callable }
            local result = t.func()
            assert(result == 42)
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("Callable stored in array and called")
    {
        Callable c1 = callable_mp_static(&callable_returns_int);
        Callable c2 = callable_mp_static(&callable_returns_string);

        Array arr;
        arr.push_back(c1);
        arr.push_back(c2);

        state->push_variant(Variant(arr));
        state->set_global("callables");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            local r1 = callables[1]()
            assert(r1 == 42)
            local r2 = callables[2]()
            assert(r2 == "hello")
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Callable stack: Expression contexts")
{
    SUBCASE("Callable result used directly in expression")
    {
        Callable add = callable_mp_static(&callable_add);
        state->push_variant(Variant(add));
        state->set_global("add");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            local result = add(5, 10) + add(3, 7)
            assert(result == 25)
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("Callable result used in condition")
    {
        Callable returns_int = callable_mp_static(&callable_returns_int);
        state->push_variant(Variant(returns_int));
        state->set_global("get_value");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            if get_value() == 42 then
                -- Expected
            else
                error("Unexpected value")
            end
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("Callable result passed to another function")
    {
        Callable echo = callable_mp_static(&callable_echo);
        Callable returns_int = callable_mp_static(&callable_returns_int);
        state->push_variant(Variant(echo));
        state->set_global("echo");
        state->push_variant(Variant(returns_int));
        state->set_global("get_value");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            local result = echo(get_value())
            assert(result == 42)
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("Callable result in table constructor")
    {
        Callable returns_int = callable_mp_static(&callable_returns_int);
        Callable returns_string = callable_mp_static(&callable_returns_string);
        state->push_variant(Variant(returns_int));
        state->set_global("get_int");
        state->push_variant(Variant(returns_string));
        state->set_global("get_string");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            local t = {
                num = get_int(),
                str = get_string()
            }
            assert(t.num == 42)
            assert(t.str == "hello")
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Callable stack: High iteration stress test")
{
    SUBCASE("1000 iterations with result capture")
    {
        Callable c = callable_mp_static(&callable_returns_int);
        state->push_variant(Variant(c));
        state->set_global("my_callable");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            for i = 1, 1000 do
                local result = my_callable()
                assert(result == 42)
            end
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("1000 iterations with arguments")
    {
        Callable echo = callable_mp_static(&callable_echo);
        state->push_variant(Variant(echo));
        state->set_global("echo");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            for i = 1, 1000 do
                local result = echo(i)
                assert(result == i)
            end
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("1000 iterations with nested calls")
    {
        Callable echo = callable_mp_static(&callable_echo);
        Callable add = callable_mp_static(&callable_add);
        state->push_variant(Variant(echo));
        state->set_global("echo");
        state->push_variant(Variant(add));
        state->set_global("add");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            for i = 1, 1000 do
                local result = echo(add(i, i))
                assert(result == i * 2)
            end
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Callable stack: Multiple callables interleaved")
{
    SUBCASE("Alternating between two callables")
    {
        Callable c1 = callable_mp_static(&callable_returns_int);
        Callable c2 = callable_mp_static(&callable_returns_string);
        state->push_variant(Variant(c1));
        state->set_global("get_int");
        state->push_variant(Variant(c2));
        state->set_global("get_string");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            for i = 1, 100 do
                local n = get_int()
                assert(n == 42)
                local s = get_string()
                assert(s == "hello")
            end
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("Three callables in sequence")
    {
        Callable c1 = callable_mp_static(&callable_returns_int);
        Callable c2 = callable_mp_static(&callable_returns_string);
        Callable c3 = callable_mp_static(&callable_returns_nil);
        state->push_variant(Variant(c1));
        state->set_global("get_int");
        state->push_variant(Variant(c2));
        state->set_global("get_string");
        state->push_variant(Variant(c3));
        state->set_global("get_nil");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            for i = 1, 50 do
                local n = get_int()
                local s = get_string()
                local v = get_nil()
                assert(n == 42)
                assert(s == "hello")
                assert(v == nil)
            end
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Callable stack: Discarded results")
{
    SUBCASE("Result explicitly discarded")
    {
        Callable c = callable_mp_static(&callable_returns_int);
        state->push_variant(Variant(c));
        state->set_global("my_callable");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            for i = 1, 100 do
                my_callable()  -- Result not assigned to any variable
            end
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("Result in void context statement")
    {
        Callable c = callable_mp_static(&callable_returns_int);
        state->push_variant(Variant(c));
        state->set_global("my_callable");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            my_callable()
            my_callable()
            my_callable()
            my_callable()
            my_callable()
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Callable stack: Lua function calling callable")
{
    SUBCASE("Callable invoked from Lua function")
    {
        Callable c = callable_mp_static(&callable_returns_int);
        state->push_variant(Variant(c));
        state->set_global("get_value");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            function call_it()
                return get_value()
            end

            for i = 1, 100 do
                local result = call_it()
                assert(result == 42)
            end
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("Callable invoked in recursive Lua function")
    {
        Callable add = callable_mp_static(&callable_add);
        state->push_variant(Variant(add));
        state->set_global("add");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            function sum_to_n(n)
                if n <= 0 then
                    return 0
                end
                return add(n, sum_to_n(n - 1))
            end

            local result = sum_to_n(10)
            assert(result == 55)  -- 1+2+3+...+10 = 55
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Callable stack: Complex argument scenarios")
{
    SUBCASE("Callable with complex type argument (Array)")
    {
        Callable echo = callable_mp_static(&callable_echo);
        state->push_variant(Variant(echo));
        state->set_global("echo");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            local arr = {1, 2, 3, 4, 5}
            local result = echo(arr)
            assert(#result == 5)
            assert(result[1] == 1)
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }

    SUBCASE("Callable with complex type argument in loop")
    {
        Callable echo = callable_mp_static(&callable_echo);
        state->push_variant(Variant(echo));
        state->set_global("echo");

        int stack_before = state->get_top();

        exec_lua_ok(R"(
            for i = 1, 50 do
                local arr = {i, i*2, i*3}
                local result = echo(arr)
                assert(#result == 3)
                assert(result[1] == i)
            end
        )");

        int stack_after = state->get_top();
        CHECK(stack_before == stack_after);
    }
}
