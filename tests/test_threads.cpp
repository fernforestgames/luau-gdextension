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
        state->newthread();
        CHECK(state->isthread(-1));

        Ref<LuaState> thread = state->tothread(-1);
        CHECK(thread.is_valid());
        CHECK(thread->mainthread() == state);
    }

    SUBCASE("Thread has valid lua_State*")
    {
        state->newthread();
        Ref<LuaState> thread = state->tothread(-1);

        CHECK(thread->get_lua_state() != nullptr);
        CHECK(thread->get_lua_state() != L); // Different from parent
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Thread: Shared globals, separate stacks")
{
    SUBCASE("Threads share global environment")
    {
        // Set a global in parent
        exec_lua_ok("shared_value = 42");

        // Create thread
        state->newthread();
        Ref<LuaState> thread = state->tothread(-1);
        state->pop(1);

        // Access global from thread
        thread->getglobal("shared_value");
        CHECK(thread->tonumber(-1) == 42);
        thread->pop(1);

        // Modify global from thread
        thread->pushnumber(100);
        thread->setglobal("shared_value");

        // Verify change in parent
        state->getglobal("shared_value");
        CHECK(state->tonumber(-1) == 100);
    }

    SUBCASE("Threads have separate stacks")
    {
        // Push value to parent stack
        state->pushnumber(123);
        int parent_top = state->gettop();

        // Create thread
        state->newthread();
        Ref<LuaState> thread = state->tothread(-1);
        state->pop(1); // Remove thread from parent stack

        // Thread should have empty stack
        CHECK(thread->gettop() == 0);

        // Push to thread stack
        thread->pushnumber(456);
        CHECK(thread->gettop() == 1);

        // Parent stack unaffected
        CHECK(state->gettop() == parent_top);
        CHECK(state->tonumber(-1) == 123);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Thread: Execute code in thread")
{
    SUBCASE("Load and execute function in thread")
    {
        // Define function in parent
        exec_lua_ok("function test_func() return 'from_thread' end");

        // Create thread
        state->newthread();
        Ref<LuaState> thread = state->tothread(-1);
        state->pop(1);

        // Get function and call it from thread
        thread->getglobal("test_func");
        CHECK(thread->isfunction(-1));

        lua_Status status = thread->pcall(0, 1, 0);
        CHECK(status == LUA_OK);

        String result = thread->tostring(-1);
        CHECK(result == "from_thread");
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
        state->newthread();
        Ref<LuaState> thread = state->tothread(-1);
        state->pop(1);

        thread->getglobal("counter");

        // Resume multiple times
        CHECK(thread->resume(0) == LUA_YIELD);
        CHECK(thread->tonumber(-1) == 1);
        thread->pop(1);

        CHECK(thread->resume(0) == LUA_YIELD);
        CHECK(thread->tonumber(-1) == 2);
        thread->pop(1);

        CHECK(thread->resume(0) == LUA_YIELD);
        CHECK(thread->tonumber(-1) == 3);
        thread->pop(1);

        CHECK(thread->resume(0) == LUA_OK);
        CHECK(thread->tostring(-1) == "done");
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Thread: Reference counting")
{
    SUBCASE("Thread keeps parent alive")
    {
        Ref<LuaState> thread;

        {
            // Create thread in scope
            state->newthread();
            thread = state->tothread(-1);
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

        state->newthread();
        thread1 = state->tothread(-1);
        state->pop(1);

        state->newthread();
        thread2 = state->tothread(-1);
        state->pop(1);

        CHECK(thread1->mainthread() == state);
        CHECK(thread2->mainthread() == state);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Thread: Close behavior")
{
    SUBCASE("Closing parent invalidates threads")
    {
        state->newthread();
        Ref<LuaState> thread = state->tothread(-1);
        state->pop(1);

        CHECK(thread->get_lua_state() != nullptr);

        // Close parent
        state->close();

        // Thread's lua_State* should also be invalidated
        // (because parent's close() calls lua_close which invalidates all threads)
        CHECK(state->get_lua_state() == nullptr);

        // Thread operations should fail gracefully
        thread->pushnumber(123); // Should print error but not crash
    }

    SUBCASE("Calling close() on thread warns but doesn't crash")
    {
        state->newthread();
        Ref<LuaState> thread = state->tothread(-1);
        state->pop(1);

        // Closing thread should just invalidate the pointer, not call lua_close
        thread->close(); // Should print warning

        // Parent should still be valid
        CHECK(state->get_lua_state() != nullptr);
        state->pushnumber(456);
        CHECK(state->tonumber(-1) == 456);
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Thread: Bidirectional bridging via push_variant")
{
    SUBCASE("Push thread as Variant")
    {
        state->newthread();
        Ref<LuaState> thread = state->tothread(-1);
        CHECK(thread->mainthread() == state);
        state->pop(1);

        // Push thread back to Lua as Variant
        Variant v = thread;
        thread->pushvariant(v);

        // Should be a thread on the stack
        CHECK(thread->isthread(-1));

        // Convert again - should create new wrapper
        Ref<LuaState> thread2 = thread->tothread(-1);
        CHECK(thread2.is_valid());
        CHECK(thread2->mainthread() == state);
    }

    SUBCASE("Store thread in table")
    {
        state->newthread();
        Ref<LuaState> thread = state->tothread(-1);
        state->pop(1);

        // Store in table
        thread->newtable();
        thread->pushvariant(Variant(thread));
        thread->setfield(-2, "thread_ref");
        thread->setglobal("my_table");

        // Retrieve from table
        thread->getglobal("my_table");
        thread->getfield(-1, "thread_ref");

        CHECK(thread->isthread(-1));

        Ref<LuaState> retrieved = thread->tothread(-1);
        CHECK(retrieved->mainthread() == state);
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
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Thread: Error handling")
{
    SUBCASE("tothread on non-thread value fails gracefully")
    {
        state->pushnumber(123);

        Ref<LuaState> thread = state->tothread(-1);
        CHECK(!thread.is_valid()); // Should fail and return null
    }

    SUBCASE("isthread returns false for non-threads")
    {
        state->pushnumber(123);
        CHECK_FALSE(state->isthread(-1));

        state->pushstring("test");
        CHECK_FALSE(state->isthread(-1));

        state->newtable();
        CHECK_FALSE(state->isthread(-1));
    }
}
