// Tests for Lua thread support in LuaState
// Tests thread creation, conversion, reference counting, and lifecycle

#include "doctest.h"
#include "test_fixtures.h"
#include <godot_cpp/core/memory.hpp>

using namespace godot;

TEST_CASE_FIXTURE(LuaStateFixture, "Thread: Basic creation and conversion")
{
    SUBCASE("Create thread and convert to LuaState")
    {
        state->new_thread();
        CHECK(state->is_thread(-1));

        Ref<LuaState> thread = state->to_thread(-1);
        CHECK(thread.is_valid());
        CHECK(thread->get_main_thread() == state);
        state->pop(1); // Clean up thread from parent stack
    }

    SUBCASE("Thread has valid lua_State*")
    {
        state->new_thread();
        Ref<LuaState> thread = state->to_thread(-1);

        CHECK(thread->get_lua_state() != nullptr);
        CHECK(thread->get_lua_state() != L); // Different from parent
        state->pop(1); // Clean up thread from parent stack
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Thread: Shared globals, separate stacks")
{
    SUBCASE("Threads share global environment")
    {
        // Set a global in parent
        exec_lua_ok("shared_value = 42");

        // Create thread
        state->new_thread();
        Ref<LuaState> thread = state->to_thread(-1);
        state->pop(1);

        // Access global from thread
        thread->get_global("shared_value");
        CHECK(thread->to_number(-1) == 42);
        thread->pop(1);

        // Modify global from thread
        thread->push_number(100);
        thread->set_global("shared_value");

        // Verify change in parent
        state->get_global("shared_value");
        CHECK(state->to_number(-1) == 100);
        state->pop(1); // Clean up global from parent stack
    }

    SUBCASE("Threads have separate stacks")
    {
        // Push value to parent stack
        state->push_number(123);
        int parent_top = state->get_top();

        // Create thread
        state->new_thread();
        Ref<LuaState> thread = state->to_thread(-1);
        state->pop(1); // Remove thread from parent stack

        // Thread should have empty stack
        CHECK(thread->get_top() == 0);

        // Push to thread stack
        thread->push_number(456);
        CHECK(thread->get_top() == 1);

        // Parent stack unaffected
        CHECK(state->get_top() == parent_top);
        CHECK(state->to_number(-1) == 123);
        state->pop(1); // Clean up the 123 from parent stack
        thread->pop(1); // Clean up the 456 from thread stack
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Thread: Execute code in thread")
{
    SUBCASE("Load and execute function in thread")
    {
        // Define function in parent
        exec_lua_ok("function test_func() return 'from_thread' end");

        // Create thread
        state->new_thread();
        Ref<LuaState> thread = state->to_thread(-1);
        state->pop(1);

        // Get function and call it from thread
        thread->get_global("test_func");
        CHECK(thread->is_function(-1));

        lua_Status status = thread->pcall(0, 1, 0);
        CHECK(status == LUA_OK);

        String result = thread->to_string(-1);
        CHECK(result == "from_thread");
        thread->pop(1); // Clean up result from thread stack
    }

    SUBCASE("Coroutine-style execution")
    {
        // Load a yielding function
        exec_lua_ok(R"(
            function counter()
                for i = 1, 3 do
                    coroutine.yield(i)
                end
                return "done"
            end
        )");

        // Create thread and load function
        state->new_thread();
        Ref<LuaState> thread = state->to_thread(-1);
        state->pop(1);

        thread->get_global("counter");

        // Resume multiple times
        CHECK(thread->resume(0) == LUA_YIELD);
        CHECK(thread->to_number(-1) == 1);
        thread->pop(1);

        CHECK(thread->resume(0) == LUA_YIELD);
        CHECK(thread->to_number(-1) == 2);
        thread->pop(1);

        CHECK(thread->resume(0) == LUA_YIELD);
        CHECK(thread->to_number(-1) == 3);
        thread->pop(1);

        CHECK(thread->resume(0) == LUA_OK);
        CHECK(thread->to_string(-1) == "done");
        thread->pop(1); // Clean up final result from thread stack
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Thread: Reference counting")
{
    SUBCASE("Thread keeps parent alive")
    {
        Ref<LuaState> thread;

        {
            // Create thread in scope
            state->new_thread();
            thread = state->to_thread(-1);
            state->pop(1);

            // Thread should be valid
            CHECK(thread.is_valid());
            CHECK(thread->get_lua_state() != nullptr);
        }

        // Thread should still be valid after parent goes out of scope
        // (because state is a member and won't be deleted yet)
        CHECK(thread.is_valid());
        CHECK(thread->get_lua_state() != nullptr);
    }

    SUBCASE("Multiple threads reference same parent")
    {
        Ref<LuaState> thread1, thread2;

        state->new_thread();
        thread1 = state->to_thread(-1);
        state->pop(1);

        state->new_thread();
        thread2 = state->to_thread(-1);
        state->pop(1);

        CHECK(thread1->get_main_thread() == state);
        CHECK(thread2->get_main_thread() == state);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Thread: Close behavior")
{
    SUBCASE("Closing parent invalidates threads")
    {
        state->new_thread();
        Ref<LuaState> thread = state->to_thread(-1);
        state->pop(1);

        CHECK(thread->get_lua_state() != nullptr);

        // Close parent
        state->close();

        // Thread's lua_State* should also be invalidated
        // (because parent's close() calls lua_close which invalidates all threads)
        CHECK(state->get_lua_state() == nullptr);

        // Thread operations should fail gracefully
        thread->push_number(123); // Should print error but not crash
    }

    SUBCASE("Calling close() on thread warns but doesn't crash")
    {
        state->new_thread();
        Ref<LuaState> thread = state->to_thread(-1);
        state->pop(1);

        // Closing thread should just invalidate the pointer, not call lua_close
        thread->close(); // Should print warning

        // Parent should still be valid
        CHECK(state->get_lua_state() != nullptr);
        state->push_number(456);
        CHECK(state->to_number(-1) == 456);
        state->pop(1); // Clean up the 456 from parent stack
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Thread: Bidirectional bridging via push_variant")
{
    SUBCASE("Push thread as Variant")
    {
        state->new_thread();
        Ref<LuaState> thread = state->to_thread(-1);
        CHECK(thread->get_main_thread() == state);
        state->pop(1);

        // Push thread back to Lua as Variant
        Variant v = thread;
        thread->push_variant(v);

        // Should be a thread on the stack
        CHECK(thread->is_thread(-1));

        // Convert again - should create new wrapper
        Ref<LuaState> thread2 = thread->to_thread(-1);
        CHECK(thread2.is_valid());
        CHECK(thread2->get_main_thread() == state);
        thread->pop(1); // Clean up thread from thread stack
    }

    SUBCASE("Store thread in table")
    {
        state->new_thread();
        Ref<LuaState> thread = state->to_thread(-1);
        state->pop(1);

        // Store in table
        thread->new_table();
        thread->push_variant(Variant(thread));
        thread->set_field(-2, "thread_ref");
        thread->set_global("my_table");

        // Retrieve from table
        thread->get_global("my_table");
        thread->get_field(-1, "thread_ref");

        CHECK(thread->is_thread(-1));

        Ref<LuaState> retrieved = thread->to_thread(-1);
        CHECK(retrieved->get_main_thread() == state);
        thread->pop(2); // Clean up thread_ref and my_table from thread stack
    }
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "Thread: Nested thread (thread from thread)")
{
    SUBCASE("Create thread from thread")
    {
        // Create parent thread
        lua_State *parent_thread = lua_newthread(L);
        CHECK(parent_thread != nullptr);

        // Create thread from the thread
        lua_State *nested_thread = lua_newthread(parent_thread);
        CHECK(nested_thread != nullptr);
        CHECK(nested_thread != parent_thread);
        CHECK(nested_thread != L);

        // Both should share globals with main state
        lua_pushinteger(L, 999);
        lua_setglobal(L, "shared_nested");

        lua_getglobal(nested_thread, "shared_nested");
        CHECK(lua_tointeger(nested_thread, -1) == 999);
        lua_pop(nested_thread, 1); // Clean up global from nested_thread stack
        lua_pop(parent_thread, 1); // Clean up nested_thread from parent_thread stack
        lua_pop(L, 1); // Clean up parent_thread from main stack
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Thread: Error handling")
{
    SUBCASE("tothread on non-thread value fails gracefully")
    {
        state->push_number(123);

        Ref<LuaState> thread = state->to_thread(-1);
        CHECK(!thread.is_valid()); // Should fail and return null
        state->pop(1); // Clean up the 123 from parent stack
    }

    SUBCASE("isthread returns false for non-threads")
    {
        state->push_number(123);
        CHECK_FALSE(state->is_thread(-1));

        state->push_string("test");
        CHECK_FALSE(state->is_thread(-1));

        state->new_table();
        CHECK_FALSE(state->is_thread(-1));
        state->pop(3); // Clean up table, string, and number from parent stack
    }
}
