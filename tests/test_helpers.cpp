// Tests for helpers - Helper functions and utilities

#include "doctest.h"
#include "test_fixtures.h"
#include "helpers.h"

using namespace godot;

TEST_SUITE("Helpers")
{
    TEST_CASE("metatable_matches - checks metatable name")
    {
        RawLuaStateFixture f;

        // Create a table with a specific metatable
        lua_createtable(f.L, 0, 0);

        luaL_newmetatable(f.L, "TestMetatable");
        lua_setmetatable(f.L, -2);

        CHECK(metatable_matches(f.L, -1, "TestMetatable"));
        CHECK_FALSE(metatable_matches(f.L, -1, "OtherMetatable"));

        lua_pop(f.L, 1);
    }

    TEST_CASE("metatable_matches - no metatable")
    {
        RawLuaStateFixture f;

        lua_createtable(f.L, 0, 0);

        CHECK_FALSE(metatable_matches(f.L, -1, "AnyMetatable"));

        lua_pop(f.L, 1);
    }

    TEST_CASE("generic_lua_concat - concatenates values")
    {
        RawLuaStateFixture f;

        lua_pushcfunction(f.L, generic_lua_concat, "test_concat");
        lua_pushstring(f.L, "Hello");
        lua_pushstring(f.L, " World");
        lua_call(f.L, 2, 1);

        CHECK(strcmp(lua_tostring(f.L, -1), "Hello World") == 0);

        lua_pop(f.L, 1);
    }

    TEST_CASE("STRING_NAME_TO_UTF8 macro")
    {
        StringName test_name("test_string");
        const char *utf8 = STRING_NAME_TO_UTF8(test_name);

        CHECK(strcmp(utf8, "test_string") == 0);
    }
}
