// Tests for LuaState class - Code execution
// load_bytecode, load_string, do_string, call, pcall, resume

#include "doctest.h"
#include "test_fixtures.h"
#include "lua_state.h"
#include "luau.h"

using namespace godot;

TEST_SUITE("LuaState - Code Loading")
{
    TEST_CASE("load_bytecode - loads and executes bytecode")
    {
        LuaStateFixture f;

        PackedByteArray bytecode = Luau::compile("return 42");
        bool loaded = f.state->load_bytecode(bytecode, "test_chunk");
        CHECK(loaded);
        CHECK(f.state->is_function(-1));

        // Execute the loaded chunk
        lua_Status status = f.state->resume();
        CHECK(status == LUA_OK);
        CHECK(f.state->to_number(-1) == 42.0);

        f.state->pop(1);
    }

    TEST_CASE("load_bytecode - with chunk name")
    {
        LuaStateFixture f;

        PackedByteArray bytecode = Luau::compile("error('test error')");
        bool loaded = f.state->load_bytecode(bytecode, "my_chunk_name");
        CHECK(loaded);

        // Execute and expect error with chunk name
        lua_Status status = f.state->resume();
        CHECK(status != LUA_OK);

        // Error message should contain chunk name
        String error_msg = f.state->to_string_inplace(-1);
        CHECK(error_msg.contains("my_chunk_name"));

        f.state->pop(1);
    }

    TEST_CASE("load_string - compiles and loads code")
    {
        LuaStateFixture f;

        bool loaded = f.state->load_string("return 1 + 2", "test");
        CHECK(loaded);
        CHECK(f.state->is_function(-1));

        lua_Status status = f.state->resume();
        CHECK(status == LUA_OK);
        CHECK(f.state->to_number(-1) == 3.0);

        f.state->pop(1);
    }

    TEST_CASE("load_string - syntax error")
    {
        LuaStateFixture f;

        bool loaded = f.state->load_string("return 1 +", "test");

        // May succeed at load, but will fail at execution
        if (loaded)
        {
            lua_Status status = f.state->resume();
            CHECK(status != LUA_OK);
        }

        f.state->pop(f.state->get_top());
    }

    TEST_CASE("do_string - compiles, loads, and executes")
    {
        LuaStateFixture f;

        lua_Status status = f.state->do_string("return 5 * 8", "test");

        CHECK(status == LUA_OK);
        CHECK(f.state->to_number(-1) == 40.0);

        f.state->pop(1);
    }

    TEST_CASE("do_string - with side effects")
    {
        LuaStateFixture f;

        lua_Status status = f.state->do_string("global_var = 999", "test");
        CHECK(status == LUA_OK);

        // Verify global was set
        f.state->get_global("global_var");
        CHECK(f.state->to_number(-1) == 999.0);

        f.state->pop(1);
    }

    TEST_CASE("do_string - runtime error")
    {
        LuaStateFixture f;

        lua_Status status = f.state->do_string("error('runtime error')", "test");

        CHECK(status != LUA_OK);
        CHECK(f.state->is_string(-1)); // Error message on stack

        f.state->pop(1);
    }
}

TEST_SUITE("LuaState - Function Calls")
{
    TEST_CASE("call - basic function call")
    {
        LuaStateFixture f;

        f.exec_lua_ok("function add(a, b) return a + b end");

        f.state->get_global("add");
        f.state->push_number(10.0);
        f.state->push_number(20.0);
        f.state->call(2, 1); // 2 args, 1 result

        CHECK(f.state->to_number(-1) == 30.0);

        f.state->pop(1);
    }

    TEST_CASE("call - multiple return values")
    {
        LuaStateFixture f;

        f.exec_lua_ok("function multi() return 1, 2, 3 end");

        f.state->get_global("multi");
        f.state->call(0, 3); // 0 args, 3 results

        CHECK(f.state->to_number(-3) == 1.0);
        CHECK(f.state->to_number(-2) == 2.0);
        CHECK(f.state->to_number(-1) == 3.0);

        f.state->pop(3);
    }

    TEST_CASE("call - LUA_MULTRET")
    {
        LuaStateFixture f;

        f.exec_lua_ok("function multi() return 1, 2, 3 end");

        f.state->get_global("multi");
        f.state->call(0, LUA_MULTRET); // Accept all results

        CHECK(f.state->get_top() == 3);
        CHECK(f.state->to_number(-3) == 1.0);
        CHECK(f.state->to_number(-2) == 2.0);
        CHECK(f.state->to_number(-1) == 3.0);

        f.state->pop(3);
    }

    TEST_CASE("pcall - protected call success")
    {
        LuaStateFixture f;

        f.exec_lua_ok("function safe(x) return x * 2 end");

        f.state->get_global("safe");
        f.state->push_number(21.0);

        lua_Status status = f.state->pcall(1, 1, 0);

        CHECK(status == LUA_OK);
        CHECK(f.state->to_number(-1) == 42.0);

        f.state->pop(1);
    }

    TEST_CASE("pcall - protected call with error")
    {
        LuaStateFixture f;

        f.exec_lua_ok("function failing() error('intentional error') end");

        f.state->get_global("failing");

        lua_Status status = f.state->pcall(0, 0, 0);

        CHECK(status != LUA_OK);
        CHECK(f.state->is_string(-1)); // Error message

        f.state->pop(1);
    }

    TEST_CASE("pcall - with error handler")
    {
        LuaStateFixture f;

        // Create error handler
        f.exec_lua_ok("function errhandler(msg) return 'HANDLED: ' .. msg end");
        f.state->get_global("errhandler");
        int errfunc_idx = f.state->get_top();

        // Create failing function
        f.exec_lua_ok("function failing() error('test error') end");
        f.state->get_global("failing");

        lua_Status status = f.state->pcall(0, 0, errfunc_idx);

        CHECK(status != LUA_OK);
        String error_msg = f.state->to_string_inplace(-1);
        CHECK(error_msg.contains("HANDLED:"));

        f.state->pop(2); // Pop error message and error handler
    }
}

TEST_SUITE("LuaState - Coroutines")
{
    TEST_CASE("resume - basic execution")
    {
        LuaStateFixture f;

        PackedByteArray bytecode = Luau::compile("return 42");
        f.state->load_bytecode(bytecode, "test");

        lua_Status status = f.state->resume();
        CHECK(status == LUA_OK);
        CHECK(f.state->to_number(-1) == 42.0);

        f.state->pop(1);
    }

    TEST_CASE("status - returns coroutine status")
    {
        LuaStateFixture f;

        CHECK(f.state->status() == LUA_OK);

        f.state->load_string("return 1", "test");
        CHECK(f.state->status() == LUA_OK);

        f.state->pop(1);
    }

    TEST_CASE("is_yieldable - main thread")
    {
        LuaStateFixture f;

        // Main thread may or may not be yieldable depending on context
        // Just verify the function works
        bool yieldable = f.state->is_yieldable();
        (void)yieldable; // Suppress unused warning
    }
}

TEST_SUITE("LuaState - String Operations")
{
    TEST_CASE("concat - concatenates stack values")
    {
        LuaStateFixture f;

        f.state->push_string("Hello");
        f.state->push_string(" ");
        f.state->push_string("World");

        f.state->concat(3);

        CHECK(f.state->to_string_inplace(-1) == "Hello World");

        f.state->pop(1);
    }

    TEST_CASE("concat - with numbers")
    {
        LuaStateFixture f;

        f.state->push_string("Answer: ");
        f.state->push_number(42.0);

        f.state->concat(2);

        CHECK(f.state->to_string_inplace(-1) == "Answer: 42");

        f.state->pop(1);
    }

    TEST_CASE("push_as_string - converts to string representation")
    {
        LuaStateFixture f;

        f.state->push_number(42.0);
        String result = f.state->push_as_string(-1);

        CHECK(result == "42");
        CHECK(f.state->to_string_inplace(-1) == "42");

        f.state->pop(2); // Pop number and string
    }
}

TEST_SUITE("LuaState - lualib Functions")
{
    TEST_CASE("get_meta_field - retrieves metatable field")
    {
        LuaStateFixture f;

        // Create table with metatable
        f.state->create_table();
        f.state->create_table(); // Metatable
        f.state->push_string("test_value");
        f.state->set_field(-2, "__name");
        f.state->set_metatable(-2);

        // Get metafield
        bool found = f.state->get_meta_field(-1, "__name");
        CHECK(found);
        CHECK(f.state->to_string_inplace(-1) == "test_value");
        f.state->pop(1);

        // Non-existent metafield
        found = f.state->get_meta_field(-1, "__nonexistent");
        CHECK_FALSE(found);

        f.state->pop(1); // Pop table
    }

    TEST_CASE("enforce_type - validates type")
    {
        LuaStateFixture f;

        f.state->push_number(42.0);
        f.state->enforce_type(-1, LUA_TNUMBER); // Should not error

        f.state->pop(1);

        // Wrong type should error - we can't easily test this without catching errors
    }

    TEST_CASE("enforce_any - ensures value exists")
    {
        LuaStateFixture f;

        f.state->push_number(42.0);
        f.state->enforce_any(-1); // Should not error

        f.state->pop(1);
    }

    TEST_CASE("enforce_string_inplace and opt_string")
    {
        LuaStateFixture f;

        f.state->push_string("test");
        String result = f.state->enforce_string_inplace(-1);
        CHECK(result == "test");
        f.state->pop(1);

        // opt_string with default
        f.state->push_nil();
        result = f.state->opt_string(-1, "default");
        CHECK(result == "default");
        f.state->pop(1);
    }

    TEST_CASE("enforce_number and opt_number")
    {
        LuaStateFixture f;

        f.state->push_number(42.5);
        double result = f.state->enforce_number(-1);
        CHECK(result == 42.5);
        f.state->pop(1);

        // opt_number with default
        f.state->push_nil();
        result = f.state->opt_number(-1, 99.0);
        CHECK(result == 99.0);
        f.state->pop(1);
    }

    TEST_CASE("enforce_integer and opt_integer")
    {
        LuaStateFixture f;

        f.state->push_integer(42);
        int result = f.state->enforce_integer(-1);
        CHECK(result == 42);
        f.state->pop(1);

        // opt_integer with default
        f.state->push_nil();
        result = f.state->opt_integer(-1, 99);
        CHECK(result == 99);
        f.state->pop(1);
    }

    TEST_CASE("enforce_boolean and opt_boolean")
    {
        LuaStateFixture f;

        f.state->push_boolean(true);
        bool result = f.state->enforce_boolean(-1);
        CHECK(result == true);
        f.state->pop(1);

        // opt_boolean with default
        f.state->push_nil();
        result = f.state->opt_boolean(-1, false);
        CHECK(result == false);
        f.state->pop(1);
    }

    TEST_CASE("enforce_vector3 and opt_vector3")
    {
        LuaStateFixture f;

        Vector3 v(1.5, 2.5, 3.5);
        f.state->push_vector3(v);
        Vector3 result = f.state->enforce_vector3(-1);
        CHECK(result.x == 1.5);
        CHECK(result.y == 2.5);
        CHECK(result.z == 3.5);
        f.state->pop(1);

        // opt_vector3 with default
        f.state->push_nil();
        result = f.state->opt_vector3(-1, Vector3(9, 9, 9));
        CHECK(result.x == 9.0);
        CHECK(result.y == 9.0);
        CHECK(result.z == 9.0);
        f.state->pop(1);
    }

    TEST_CASE("enforce_option - validates enum choice")
    {
        LuaStateFixture f;

        PackedStringArray options;
        options.push_back("option1");
        options.push_back("option2");
        options.push_back("option3");

        f.state->push_string("option2");
        int result = f.state->enforce_option(-1, options);
        CHECK(result == 1); // Index of "option2"
        f.state->pop(1);
    }
}
