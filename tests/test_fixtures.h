// Common test fixtures for doctest-based tests
// Reduces duplication by providing shared setup/teardown logic

#ifndef TEST_FIXTURES_H
#define TEST_FIXTURES_H

#include "lua_state.h"
#include "lua_godotlib.h"
#include "luau.h"
#include <godot_cpp/core/memory.hpp>
#include <lua.h>
#include <lualib.h>
#include <luacode.h>
#include <cstring>

using namespace godot;

// Fixture for tests that need raw lua_State* (e.g., math type bridge tests)
// Provides direct access to Lua C API
struct RawLuaStateFixture
{
    lua_State *L;

    RawLuaStateFixture()
    {
        L = luaL_newstate();
        luaL_openlibs(L);
        luaopen_godot(L); // Open Godot math types library
    }

    ~RawLuaStateFixture()
    {
        lua_close(L);
    }

    // Helper to execute Luau code
    int exec_lua(const char *code)
    {
        size_t bytecode_size = 0;
        char *bytecode = luau_compile(code, strlen(code), nullptr, &bytecode_size);
        if (!bytecode)
        {
            return -1;
        }

        int result = luau_load(L, "test", bytecode, bytecode_size, 0);
        free(bytecode);

        if (result == 0)
        {
            result = lua_resume(L, nullptr, 0);
        }

        return result;
    }
};

// Fixture for tests that use LuaState wrapper
// Provides higher-level GDExtension API
struct LuaStateFixture
{
    Ref<LuaState> state;
    lua_State *L; // Convenience pointer to underlying lua_State

    LuaStateFixture()
    {
        state = Ref<LuaState>(memnew(LuaState));
        state->openlibs(LuaState::LIB_ALL);
        L = state->get_lua_state();
    }

    // Helper to execute Luau code and return status
    lua_Status exec_lua(const char *code, const char *chunk_name = "test")
    {
        PackedByteArray bytecode = Luau::compile(code);
        state->load_bytecode(bytecode, chunk_name);
        return state->resume();
    }

    // Helper to execute and check for success
    bool exec_lua_ok(const char *code, const char *chunk_name = "test")
    {
        return exec_lua(code, chunk_name) == LUA_OK;
    }
};

#endif // TEST_FIXTURES_H
