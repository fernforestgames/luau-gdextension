// Tests for LuaState class - Thread/coroutine support
// Thread creation, coroutine operations, thread lifecycle

#include "doctest.h"
#include "test_fixtures.h"
#include "lua_state.h"

using namespace godot;

TEST_SUITE("LuaState - Threads")
{
    TEST_CASE_FIXTURE(LuaStateFixture, "new_thread - creates thread")
    {
        Ref<LuaState> thread = state->new_thread();

        CHECK(thread.is_valid());
        CHECK(thread->is_valid());
        CHECK_FALSE(thread->is_main_thread());
        CHECK(thread->get_main_thread() == state);

        // Thread should be on stack
        CHECK(state->is_thread(-1));

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "new_thread - multiple threads")
    {
        Ref<LuaState> thread1 = state->new_thread();
        Ref<LuaState> thread2 = state->new_thread();
        Ref<LuaState> thread3 = state->new_thread();

        CHECK(thread1 != thread2);
        CHECK(thread2 != thread3);
        CHECK(thread1 != thread3);

        CHECK(thread1->get_main_thread() == state);
        CHECK(thread2->get_main_thread() == state);
        CHECK(thread3->get_main_thread() == state);

        state->pop(3);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_thread - converts stack value to thread")
    {
        Ref<LuaState> thread1 = state->new_thread();
        Ref<LuaState> thread2 = state->to_thread(-1);

        // Both should wrap the same lua_State*
        CHECK(thread1->get_lua_state() == thread2->get_lua_state());

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "thread - shared globals")
    {
        // Set global in main thread
        state->push_number(42.0);
        state->set_global("shared_value");

        // Create thread
        Ref<LuaState> thread = state->new_thread();

        // Thread should see the global
        thread->get_global("shared_value");
        CHECK(thread->to_number(-1) == 42.0);
        thread->pop(1);

        // Set global in thread
        thread->push_number(99.0);
        thread->set_global("thread_set");

        // Main thread should see it
        state->get_global("thread_set");
        CHECK(state->to_number(-1) == 99.0);
        state->pop(1);

        state->pop(1); // Pop thread from main stack
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "thread - independent stacks")
    {
        state->push_number(1.0);
        state->push_number(2.0);
        CHECK(state->get_top() == 2);

        Ref<LuaState> thread = state->new_thread();
        CHECK(thread->get_top() == 0); // Thread has empty stack

        thread->push_number(99.0);
        CHECK(thread->get_top() == 1);
        CHECK(state->get_top() == 3); // Main has 2 numbers + thread

        thread->pop(1);
        state->pop(3);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "thread - coroutine execution")
    {
        // Create coroutine function
        exec_lua_ok(R"(
            function coro_func()
                coroutine.yield(1)
                coroutine.yield(2)
                return 3
            end
        )");

        // Create thread
        Ref<LuaState> thread = state->new_thread();

        // Load coroutine function into thread
        thread->get_global("coro_func");

        // First resume - yields 1
        lua_Status status = thread->resume();
        CHECK(status == LUA_YIELD);
        CHECK(thread->to_number(-1) == 1.0);
        thread->pop(1);

        // Second resume - yields 2
        status = thread->resume();
        CHECK(status == LUA_YIELD);
        CHECK(thread->to_number(-1) == 2.0);
        thread->pop(1);

        // Third resume - returns 3
        status = thread->resume();
        CHECK(status == LUA_OK);
        CHECK(thread->to_number(-1) == 3.0);
        thread->pop(1);

        state->pop(1); // Pop thread
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "thread - passing arguments to resume")
    {
        exec_lua_ok(R"(
            function add_values(a, b)
                return a + b
            end
        )");

        Ref<LuaState> thread = state->new_thread();
        thread->get_global("add_values");
        thread->push_number(10.0);
        thread->push_number(20.0);

        lua_Status status = thread->resume(2); // 2 arguments
        CHECK(status == LUA_OK);
        CHECK(thread->to_number(-1) == 30.0);
        thread->pop(1);

        state->pop(1); // Pop thread
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "co_status - returns thread status")
    {
        Ref<LuaState> thread = state->new_thread();

        lua_CoStatus co_status = state->co_status(thread.ptr());
        CHECK(co_status == LUA_COFIN);

        // Load a yielding function
        thread->load_string("coroutine.yield(1)", "test");

        // Start execution
        lua_Status status = thread->resume();
        CHECK(status == LUA_YIELD);

        // Now suspended
        co_status = state->co_status(thread.ptr());
        CHECK(co_status == LUA_COSUS);

        thread->pop(1); // Pop yield value
        state->pop(1);  // Pop thread
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "reset_thread - resets thread state")
    {
        Ref<LuaState> thread = state->new_thread();

        thread->push_number(1.0);
        thread->push_number(2.0);
        CHECK(thread->get_top() == 2);

        thread->reset_thread();
        CHECK(thread->get_top() == 0);
        CHECK(thread->is_thread_reset());

        state->pop(1); // Pop thread
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "xmove - moves values between states")
    {
        Ref<LuaState> thread = state->new_thread();

        // Push values in main thread
        state->push_number(1.0);
        state->push_number(2.0);
        state->push_string("test");

        // Move 2 values to thread
        state->xmove(thread.ptr(), 2);

        // Main thread should have 1 value + thread
        CHECK(state->get_top() == 2);
        CHECK(state->to_number(-1) == 1.0);

        // Thread should have 2 values
        CHECK(thread->get_top() == 2);
        CHECK(thread->to_number(-2) == 2.0);
        CHECK(thread->to_string_inplace(-1) == "test");

        thread->pop(2);
        state->pop(2);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "xpush - copies value to another state")
    {
        Ref<LuaState> thread = state->new_thread();

        state->push_number(42.0);

        // Copy value to thread (doesn't remove from main)
        state->xpush(thread.ptr(), -1);

        // Both should have the value
        CHECK(state->to_number(-1) == 42.0);
        CHECK(thread->to_number(-1) == 42.0);

        thread->pop(1);
        state->pop(2); // Pop number and thread
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_thread - pushes thread onto its own stack")
    {
        Ref<LuaState> thread = state->new_thread();

        thread->push_thread();
        CHECK(thread->is_thread(-1));

        thread->pop(1);
        state->pop(1); // Pop thread from main
    }

    TEST_CASE("thread - survives main thread close (reference counting)")
    {
        Ref<LuaState> main = memnew(LuaState);
        main->open_libs();

        main->push_number(42.0);
        main->set_global("test_value");

        Ref<LuaState> thread = main->new_thread();

        // Thread can access global
        thread->get_global("test_value");
        CHECK(thread->to_number(-1) == 42.0);
        thread->pop(1);

        main->pop(1); // Pop thread from stack

        // Close main thread
        main->close();

        // Thread should now be invalid (main closed)
        CHECK_FALSE(thread->is_valid());
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "thread - multiple wrappers for same lua_State*")
    {
        Ref<LuaState> thread1 = state->new_thread();
        lua_State *L = thread1->get_lua_state();

        // Create another wrapper for the same lua_State*
        Ref<LuaState> thread2 = state->to_thread(-1);

        CHECK(thread1->get_lua_state() == thread2->get_lua_state());

        // Operations on either wrapper affect the same state
        thread1->push_number(99.0);
        CHECK(thread2->get_top() == 1);
        CHECK(thread2->to_number(-1) == 99.0);

        thread1->pop(1);
        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "thread - error handling in coroutine")
    {
        exec_lua_ok(R"(
            function error_coro()
                coroutine.yield(1)
                error("intentional error")
            end
        )");

        Ref<LuaState> thread = state->new_thread();
        thread->get_global("error_coro");

        // First resume succeeds
        lua_Status status = thread->resume();
        CHECK(status == LUA_YIELD);
        thread->pop(1);

        // Second resume errors
        status = thread->resume();
        CHECK(status == LUA_ERRRUN);
        CHECK(thread->is_string(-1)); // Error message

        thread->pop(1);
        state->pop(1); // Pop thread
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "thread - nested coroutines")
    {
        exec_lua_ok(R"(
            function outer()
                coroutine.yield(1)
                local inner = coroutine.create(function()
                    coroutine.yield(2)
                    return 3
                end)
                local status, val = coroutine.resume(inner)
                coroutine.yield(val)
                status, val = coroutine.resume(inner)
                return val
            end
        )");

        Ref<LuaState> thread = state->new_thread();
        state->pop(1); // Pop thread
        thread->get_global("outer");

        // First yield from outer
        lua_Status status = thread->resume();
        CHECK(status == LUA_YIELD);
        CHECK(thread->to_number(-1) == 1.0);
        thread->pop(1);

        // Second yield from inner (via outer)
        status = thread->resume();
        CHECK(status == LUA_YIELD);
        CHECK(thread->to_number(-1) == 2.0);
        thread->pop(1);

        // Final return
        status = thread->resume();
        CHECK(status == LUA_OK);
        CHECK(thread->to_number(-1) == 3.0);
        thread->pop(1);
    }
}
