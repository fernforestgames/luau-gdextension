// Tests for LuaState class - Code execution
// load_bytecode, load_string, do_string, call, pcall, resume

#include "doctest.h"
#include "test_fixtures.h"
#include "lua_state.h"
#include "luau.h"

using namespace godot;

TEST_SUITE("LuaState - Code Loading")
{
    TEST_CASE_FIXTURE(LuaStateFixture, "load_bytecode - loads and executes bytecode")
    {
        PackedByteArray bytecode = Luau::compile("return 42");
        bool loaded = state->load_bytecode(bytecode, "test_chunk");
        CHECK(loaded);
        CHECK(state->is_function(-1));

        // Execute the loaded chunk
        lua_Status status = state->resume();
        CHECK(status == LUA_OK);
        CHECK(state->to_number(-1) == 42.0);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "load_bytecode - with chunk name")
    {
        PackedByteArray bytecode = Luau::compile("error('test error')");
        bool loaded = state->load_bytecode(bytecode, "my_chunk_name");
        CHECK(loaded);

        // Execute and expect error with chunk name
        lua_Status status = state->resume();
        CHECK(status != LUA_OK);

        // Error message should contain chunk name
        String error_msg = state->to_string_inplace(-1);
        CHECK(error_msg.contains("my_chunk_name"));

        // Resume does not unwind the stack on error
        state->set_top(0);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "load_string - compiles and loads code")
    {
        bool loaded = state->load_string("return 1 + 2", "test");
        CHECK(loaded);
        CHECK(state->is_function(-1));

        lua_Status status = state->resume();
        CHECK(status == LUA_OK);
        CHECK(state->to_number(-1) == 3.0);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "load_string - syntax error")
    {
        bool loaded = state->load_string("return 1 +", "test");

        // May succeed at load, but will fail at execution
        if (loaded)
        {
            lua_Status status = state->resume();
            CHECK(status != LUA_OK);
        }

        state->pop(state->get_top());
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "do_string - compiles, loads, and executes")
    {
        lua_Status status = state->do_string("return 5 * 8", "test");

        CHECK(status == LUA_OK);
        CHECK(state->to_number(-1) == 40.0);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "do_string - with side effects")
    {
        lua_Status status = state->do_string("global_var = 999", "test");
        CHECK(status == LUA_OK);

        // Verify global was set
        state->get_global("global_var");
        CHECK(state->to_number(-1) == 999.0);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "do_string - runtime error")
    {
        lua_Status status = state->do_string("error('runtime error')", "test");

        CHECK(status != LUA_OK);
        CHECK(state->is_string(-1)); // Error message on stack

        state->pop(1);
    }
}

TEST_SUITE("LuaState - Function Calls")
{
    TEST_CASE_FIXTURE(LuaStateFixture, "call - basic function call")
    {
        exec_lua_ok("function add(a, b) return a + b end");

        state->get_global("add");
        state->push_number(10.0);
        state->push_number(20.0);
        state->call(2, 1); // 2 args, 1 result

        CHECK(state->to_number(-1) == 30.0);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "call - multiple return values")
    {
        exec_lua_ok("function multi() return 1, 2, 3 end");

        state->get_global("multi");
        state->call(0, 3); // 0 args, 3 results

        CHECK(state->to_number(-3) == 1.0);
        CHECK(state->to_number(-2) == 2.0);
        CHECK(state->to_number(-1) == 3.0);

        state->pop(3);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "call - LUA_MULTRET")
    {
        exec_lua_ok("function multi() return 1, 2, 3 end");

        state->get_global("multi");
        state->call(0, LUA_MULTRET); // Accept all results

        CHECK(state->get_top() == 3);
        CHECK(state->to_number(-3) == 1.0);
        CHECK(state->to_number(-2) == 2.0);
        CHECK(state->to_number(-1) == 3.0);

        state->pop(3);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "pcall - protected call success")
    {
        exec_lua_ok("function safe(x) return x * 2 end");

        state->get_global("safe");
        state->push_number(21.0);

        lua_Status status = state->pcall(1, 1, 0);

        CHECK(status == LUA_OK);
        CHECK(state->to_number(-1) == 42.0);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "pcall - protected call with error")
    {
        exec_lua_ok("function failing() error('intentional error') end");

        state->get_global("failing");

        lua_Status status = state->pcall(0, 0, 0);

        CHECK(status != LUA_OK);
        CHECK(state->is_string(-1)); // Error message

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "pcall - with error handler")
    {
        // Create error handler
        exec_lua_ok("function errhandler(msg) return 'HANDLED: ' .. msg end");
        state->get_global("errhandler");
        int errfunc_idx = state->get_top();

        // Create failing function
        exec_lua_ok("function failing() error('test error') end");
        state->get_global("failing");

        lua_Status status = state->pcall(0, 0, errfunc_idx);

        CHECK(status != LUA_OK);
        String error_msg = state->to_string_inplace(-1);
        CHECK(error_msg.contains("HANDLED:"));

        state->pop(2); // Pop error message and error handler
    }
}

TEST_SUITE("LuaState - Coroutines")
{
    TEST_CASE_FIXTURE(LuaStateFixture, "resume - basic execution")
    {
        PackedByteArray bytecode = Luau::compile("return 42");
        state->load_bytecode(bytecode, "test");

        lua_Status status = state->resume();
        CHECK(status == LUA_OK);
        CHECK(state->to_number(-1) == 42.0);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "status - returns coroutine status")
    {
        CHECK(state->status() == LUA_OK);

        state->load_string("return 1", "test");
        CHECK(state->status() == LUA_OK);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "is_yieldable - main thread")
    {
        // Main thread may or may not be yieldable depending on context
        // Just verify the function works
        bool yieldable = state->is_yieldable();
        (void)yieldable; // Suppress unused warning
    }
}

TEST_SUITE("LuaState - String Operations")
{
    TEST_CASE_FIXTURE(LuaStateFixture, "concat - concatenates stack values")
    {
        state->push_string("Hello");
        state->push_string(" ");
        state->push_string("World");

        state->concat(3);

        CHECK(state->to_string_inplace(-1) == "Hello World");

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "concat - with numbers")
    {
        state->push_string("Answer: ");
        state->push_number(42.0);

        state->concat(2);

        CHECK(state->to_string_inplace(-1) == "Answer: 42");

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_as_string - converts to string representation")
    {
        state->push_number(42.0);
        String result = state->push_as_string(-1);

        CHECK(result == "42");
        CHECK(state->to_string_inplace(-1) == "42");

        state->pop(2); // Pop number and string
    }
}

TEST_SUITE("LuaState - lualib Functions")
{
    TEST_CASE_FIXTURE(LuaStateFixture, "get_meta_field - retrieves metatable field")
    {
        // Create table with metatable
        state->create_table();
        state->create_table(); // Metatable
        state->push_string("test_value");
        state->set_field(-2, "__name");
        state->set_metatable(-2);

        // Get metafield
        bool found = state->get_meta_field(-1, "__name");
        CHECK(found);
        CHECK(state->to_string_inplace(-1) == "test_value");
        state->pop(1);

        // Non-existent metafield
        found = state->get_meta_field(-1, "__nonexistent");
        CHECK_FALSE(found);

        state->pop(1); // Pop table
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "enforce_type - validates type")
    {
        state->push_number(42.0);
        state->enforce_type(-1, LUA_TNUMBER); // Should not error

        state->pop(1);

        // Wrong type should error - we can't easily test this without catching errors
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "enforce_any - ensures value exists")
    {
        state->push_number(42.0);
        state->enforce_any(-1); // Should not error

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "enforce_string_inplace and opt_string")
    {
        state->push_string("test");
        String result = state->enforce_string_inplace(-1);
        CHECK(result == "test");
        state->pop(1);

        // opt_string with default
        state->push_nil();
        result = state->opt_string(-1, "default");
        CHECK(result == "default");
        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "enforce_number and opt_number")
    {
        state->push_number(42.5);
        double result = state->enforce_number(-1);
        CHECK(result == 42.5);
        state->pop(1);

        // opt_number with default
        state->push_nil();
        result = state->opt_number(-1, 99.0);
        CHECK(result == 99.0);
        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "enforce_integer and opt_integer")
    {
        state->push_integer(42);
        int result = state->enforce_integer(-1);
        CHECK(result == 42);
        state->pop(1);

        // opt_integer with default
        state->push_nil();
        result = state->opt_integer(-1, 99);
        CHECK(result == 99);
        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "enforce_boolean and opt_boolean")
    {
        state->push_boolean(true);
        bool result = state->enforce_boolean(-1);
        CHECK(result == true);
        state->pop(1);

        // opt_boolean with default
        state->push_nil();
        result = state->opt_boolean(-1, false);
        CHECK(result == false);
        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "enforce_vector3 and opt_vector3")
    {
        Vector3 v(1.5, 2.5, 3.5);
        state->push_vector3(v);
        Vector3 result = state->enforce_vector3(-1);
        CHECK(result.x == 1.5);
        CHECK(result.y == 2.5);
        CHECK(result.z == 3.5);
        state->pop(1);

        // opt_vector3 with default
        state->push_nil();
        result = state->opt_vector3(-1, Vector3(9, 9, 9));
        CHECK(result.x == 9.0);
        CHECK(result.y == 9.0);
        CHECK(result.z == 9.0);
        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "enforce_option - validates enum choice")
    {
        PackedStringArray options;
        options.push_back("option1");
        options.push_back("option2");
        options.push_back("option3");

        state->push_string("option2");
        int result = state->enforce_option(-1, options);
        CHECK(result == 1); // Index of "option2"
        state->pop(1);
    }
}
