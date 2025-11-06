// Tests for Luau class (compiler wrapper and utilities)

#include "doctest.h"
#include "test_fixtures.h"
#include "luau.h"
#include "lua_compileoptions.h"

using namespace godot;

TEST_SUITE("Luau")
{
    TEST_CASE("compile - basic code compilation")
    {
        String code = "return 1 + 2";
        PackedByteArray bytecode = Luau::compile(code);

        CHECK(bytecode.size() > 0);

        // Verify bytecode is executable
        RawLuaStateFixture f;
        int result = luau_load(f.L, "test", (const char *)bytecode.ptr(), bytecode.size(), 0);
        CHECK(result == 0);

        result = lua_resume(f.L, nullptr, 0);
        CHECK(result == LUA_OK);
        CHECK(lua_isnumber(f.L, -1));
        CHECK(lua_tonumber(f.L, -1) == 3.0);
        lua_pop(f.L, 1);
    }

    TEST_CASE("compile - function definition")
    {
        String code = "function add(a, b) return a + b end";
        PackedByteArray bytecode = Luau::compile(code);

        CHECK(bytecode.size() > 0);

        // Verify bytecode is executable
        RawLuaStateFixture f;
        int result = luau_load(f.L, "test", (const char *)bytecode.ptr(), bytecode.size(), 0);
        CHECK(result == 0);

        result = lua_resume(f.L, nullptr, 0);
        CHECK(result == LUA_OK);
        lua_pop(f.L, lua_gettop(f.L)); // Clear stack
    }

    TEST_CASE("compile - syntax error")
    {
        String code = "return 1 +"; // Incomplete expression
        PackedByteArray bytecode = Luau::compile(code);

        // Luau compiler returns bytecode with error message embedded
        // When loaded, it will error
        CHECK(bytecode.size() > 0);

        RawLuaStateFixture f;
        int result = luau_load(f.L, "test", (const char *)bytecode.ptr(), bytecode.size(), 0);
        // Load might succeed, but execution will fail
        if (result == 0)
        {
            result = lua_resume(f.L, nullptr, 0);
            CHECK(result != LUA_OK);
        }
        lua_pop(f.L, lua_gettop(f.L));
    }

    TEST_CASE("compile - empty string")
    {
        String code = "";
        PackedByteArray bytecode = Luau::compile(code);

        CHECK(bytecode.size() > 0);

        RawLuaStateFixture f;
        int result = luau_load(f.L, "test", (const char *)bytecode.ptr(), bytecode.size(), 0);
        CHECK(result == 0);

        result = lua_resume(f.L, nullptr, 0);
        CHECK(result == LUA_OK);
        lua_pop(f.L, lua_gettop(f.L));
    }

    TEST_CASE("compile - complex multi-line code")
    {
        String code = R"(
            local function factorial(n)
                if n <= 1 then
                    return 1
                else
                    return n * factorial(n - 1)
                end
            end
            return factorial(5)
        )";
        PackedByteArray bytecode = Luau::compile(code);

        CHECK(bytecode.size() > 0);

        RawLuaStateFixture f;
        int result = luau_load(f.L, "test", (const char *)bytecode.ptr(), bytecode.size(), 0);
        CHECK(result == 0);

        result = lua_resume(f.L, nullptr, 0);
        CHECK(result == LUA_OK);
        CHECK(lua_isnumber(f.L, -1));
        CHECK(lua_tonumber(f.L, -1) == 120.0);
        lua_pop(f.L, 1);
    }

    TEST_CASE("compile - with compile options")
    {
        LuaCompileOptions *options = memnew(LuaCompileOptions);
        options->set_optimization_level(2);
        options->set_debug_level(2);

        String code = "return 42";
        PackedByteArray bytecode = Luau::compile(code, options);

        CHECK(bytecode.size() > 0);

        memdelete(options);

        // Verify bytecode is executable
        RawLuaStateFixture f;
        int result = luau_load(f.L, "test", (const char *)bytecode.ptr(), bytecode.size(), 0);
        CHECK(result == 0);

        result = lua_resume(f.L, nullptr, 0);
        CHECK(result == LUA_OK);
        CHECK(lua_tonumber(f.L, -1) == 42.0);
        lua_pop(f.L, 1);
    }

    TEST_CASE("compile - null options uses defaults")
    {
        String code = "return 99";
        PackedByteArray bytecode = Luau::compile(code, nullptr);

        CHECK(bytecode.size() > 0);

        RawLuaStateFixture f;
        int result = luau_load(f.L, "test", (const char *)bytecode.ptr(), bytecode.size(), 0);
        CHECK(result == 0);

        result = lua_resume(f.L, nullptr, 0);
        CHECK(result == LUA_OK);
        CHECK(lua_tonumber(f.L, -1) == 99.0);
        lua_pop(f.L, 1);
    }

    TEST_CASE("upvalue_index - converts upvalue number to pseudo-index")
    {
        int idx1 = Luau::upvalue_index(1);
        int idx2 = Luau::upvalue_index(2);
        int idx3 = Luau::upvalue_index(10);

        // Upvalue indices should be negative pseudo-indices
        CHECK(idx1 < 0);
        CHECK(idx2 < 0);
        CHECK(idx3 < 0);

        // Different upvalues should have different indices
        CHECK(idx1 != idx2);
        CHECK(idx2 != idx3);

        // Should match lua_upvalueindex macro
        CHECK(idx1 == lua_upvalueindex(1));
        CHECK(idx2 == lua_upvalueindex(2));
        CHECK(idx3 == lua_upvalueindex(10));
    }

    TEST_CASE("is_pseudo - identifies pseudo-indices")
    {
        // Standard stack indices are not pseudo-indices
        CHECK_FALSE(Luau::is_pseudo(1));
        CHECK_FALSE(Luau::is_pseudo(5));
        CHECK_FALSE(Luau::is_pseudo(-1));

        // Registry, environment, and globals are pseudo-indices
        CHECK(Luau::is_pseudo(LUA_REGISTRYINDEX));
        CHECK(Luau::is_pseudo(LUA_ENVIRONINDEX));
        CHECK(Luau::is_pseudo(LUA_GLOBALSINDEX));

        // Upvalue indices are pseudo-indices
        CHECK(Luau::is_pseudo(Luau::upvalue_index(1)));
        CHECK(Luau::is_pseudo(Luau::upvalue_index(5)));
    }

    TEST_CASE("clock - returns monotonic time")
    {
        double t1 = Luau::clock();
        CHECK(t1 >= 0.0);

        // Sleep briefly to ensure time advances
        // (use busy wait to avoid platform-specific sleep functions)
        volatile int dummy = 0;
        for (int i = 0; i < 1000000; i++)
        {
            dummy += i;
        }

        double t2 = Luau::clock();
        CHECK(t2 >= t1); // Time should advance or stay the same

        // Both times should be reasonable (not huge values)
        CHECK(t1 < 1e10);
        CHECK(t2 < 1e10);
    }

    TEST_CASE("clock - multiple calls")
    {
        double times[5];
        for (int i = 0; i < 5; i++)
        {
            times[i] = Luau::clock();
            CHECK(times[i] >= 0.0);
        }

        // Times should be non-decreasing
        for (int i = 1; i < 5; i++)
        {
            CHECK(times[i] >= times[i - 1]);
        }
    }
}
