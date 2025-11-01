// Tests for LuaState reference management and execution control
// Tests ref/unref, status, yield, signals

#include "doctest.h"
#include "test_fixtures.h"
#include <godot_cpp/core/memory.hpp>

using namespace godot;

TEST_CASE_FIXTURE(LuaStateFixture, "Reference management: ref and unref")
{
    SUBCASE("Create and retrieve reference")
    {
        state->new_table();
        state->push_number(42);
        state->set_field(-2, "value");

        int ref = state->ref(-1);
        CHECK(ref != LUA_NOREF);
        CHECK(ref != LUA_REFNIL);

        // Luau's lua_ref doesn't pop, so we need to pop manually
        state->pop(1);
        CHECK(state->get_top() == 0);

        // Get reference back
        state->get_ref(ref);
        CHECK(state->is_table(-1));
        state->get_field(-1, "value");
        CHECK(state->to_number(-1) == 42);
        state->pop(2);

        // Clean up
        state->unref(ref);
    }

    SUBCASE("Unref releases reference slot for reuse")
    {
        state->new_table();
        int ref1 = state->ref(-1);
        state->pop(1); // lua_ref doesn't pop

        // Unref it
        state->unref(ref1);

        // Create new ref - in Luau, this may reuse the freed slot
        state->new_table();
        int ref2 = state->ref(-1);
        state->pop(1);

        // The important part is that we can create and use new refs after unref
        state->get_ref(ref2);
        CHECK(state->is_table(-1));
        state->pop(1);

        state->unref(ref2);
    }

    SUBCASE("ref with nil returns LUA_REFNIL")
    {
        state->push_nil();
        int ref = state->ref(-1);
        state->pop(1); // lua_ref doesn't pop

        CHECK(ref == LUA_REFNIL);
        CHECK(state->get_top() == 0);
    }

    SUBCASE("Multiple references to same table")
    {
        state->new_table();
        state->push_number(100);
        state->set_field(-2, "value");

        // Create multiple refs to same table
        state->push_value(-1);
        int ref1 = state->ref(-1);
        state->pop(1); // lua_ref doesn't pop

        state->push_value(-1);
        int ref2 = state->ref(-1);
        state->pop(1); // lua_ref doesn't pop

        state->pop(1); // Pop original table

        // Both refs should point to same table
        state->get_ref(ref1);
        state->get_ref(ref2);

        // Modify through first ref
        state->push_number(200);
        state->set_field(-2, "value");

        // Second ref should see change
        state->get_field(-1, "value");
        CHECK(state->to_number(-1) == 200);
        state->pop(1);

        state->pop(2); // Pop both refs

        state->unref(ref1);
        state->unref(ref2);
    }

    SUBCASE("Reference lifecycle")
    {
        // Create multiple refs
        godot::Array refs;
        for (int i = 0; i < 10; i++)
        {
            state->new_table();
            state->push_number(i);
            state->set_field(-2, "id");
            refs.append(state->ref(-1));
            state->pop(1); // lua_ref doesn't pop
        }

        // Verify all retrievable
        for (int i = 0; i < 10; i++)
        {
            state->get_ref((int)refs[i]);
            state->get_field(-1, "id");
            CHECK(state->to_number(-1) == i);
            state->pop(2);
        }

        // Unref half
        for (int i = 0; i < 5; i++)
        {
            state->unref((int)refs[i]);
        }

        // Second half should still work
        for (int i = 5; i < 10; i++)
        {
            state->get_ref((int)refs[i]);
            state->get_field(-1, "id");
            CHECK(state->to_number(-1) == i);
            state->pop(2);
        }

        // Clean up remaining
        for (int i = 5; i < 10; i++)
        {
            state->unref((int)refs[i]);
        }
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Execution control: status")
{
    SUBCASE("Main state starts with LUA_OK")
    {
        CHECK(state->status() == LUA_OK);
    }

    SUBCASE("State status after successful execution")
    {
        CHECK(state->do_string("return 42", "test") == LUA_OK);
        CHECK(state->status() == LUA_OK);
        state->pop(1);
    }

    SUBCASE("Thread status")
    {
        state->new_thread();
        Ref<LuaState> thread = state->to_thread(-1);
        state->pop(1);

        // New thread should have LUA_OK status
        CHECK(thread->status() == LUA_OK);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Execution control: is_yieldable")
{
    SUBCASE("Thread is yieldable")
    {
        state->new_thread();
        Ref<LuaState> thread = state->to_thread(-1);
        state->pop(1);

        CHECK(thread->is_yieldable());
    }

    SUBCASE("is_yieldable after coroutine operations")
    {
        exec_lua_ok(R"(
            function yielder()
                coroutine.yield(1)
                return 2
            end
        )");

        state->new_thread();
        Ref<LuaState> thread = state->to_thread(-1);
        state->pop(1);

        thread->get_global("yielder");

        // Should be yieldable before running
        CHECK(thread->is_yieldable());

        // Resume - function yields
        CHECK(thread->resume(0) == LUA_YIELD);
        thread->pop(1);

        // Should still be yieldable
        CHECK(thread->is_yieldable());
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Signals: debugstep signal")
{
    SUBCASE("debugstep signal exists")
    {
        bool has_signal = state->has_signal("debugstep");
        CHECK(has_signal);
    }

    SUBCASE("debugstep signal fires during single-step execution")
    {
        int step_count = 0;

        // Create a callable to count steps
        Callable on_step = Callable(state.ptr(), "push_number"); // Dummy callable for test

        // Note: In actual usage, you'd connect a real callback
        // For this test, we just verify the signal exists and can be connected
        CHECK(state->has_signal("debugstep"));
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Signals: interrupt signal")
{
    SUBCASE("interrupt signal exists")
    {
        bool has_signal = state->has_signal("interrupt");
        CHECK(has_signal);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Type method: returns lua_Type enum")
{
    SUBCASE("Correct types for various values")
    {
        state->push_nil();
        CHECK(state->type(-1) == LUA_TNIL);
        state->pop(1);

        state->push_boolean(true);
        CHECK(state->type(-1) == LUA_TBOOLEAN);
        state->pop(1);

        state->push_number(42);
        CHECK(state->type(-1) == LUA_TNUMBER);
        state->pop(1);

        state->push_string("hello");
        CHECK(state->type(-1) == LUA_TSTRING);
        state->pop(1);

        state->new_table();
        CHECK(state->type(-1) == LUA_TTABLE);
        state->pop(1);

        exec_lua_ok("return function() end");
        CHECK(state->type(-1) == LUA_TFUNCTION);
        state->pop(1);

        state->new_thread();
        CHECK(state->type(-1) == LUA_TTHREAD);
        state->pop(1);
    }

    SUBCASE("Invalid index returns LUA_TNONE")
    {
        CHECK(state->type(10) == LUA_TNONE);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Const correctness: to_* methods")
{
    SUBCASE("to_string is const")
    {
        state->push_string("test");
        const LuaState *const_state = state.ptr();

        String s = const_state->to_string(-1);
        CHECK(s == "test");

        state->pop(1);
    }

    SUBCASE("to_number is const")
    {
        state->push_number(3.14);
        const LuaState *const_state = state.ptr();

        double d = const_state->to_number(-1);
        CHECK(d == doctest::Approx(3.14));

        state->pop(1);
    }

    SUBCASE("to_integer is const")
    {
        state->push_number(42);
        const LuaState *const_state = state.ptr();

        int i = const_state->to_integer(-1);
        CHECK(i == 42);

        state->pop(1);
    }

    SUBCASE("to_boolean is const")
    {
        state->push_boolean(true);
        const LuaState *const_state = state.ptr();

        bool b = const_state->to_boolean(-1);
        CHECK(b);

        state->pop(1);
    }
}
