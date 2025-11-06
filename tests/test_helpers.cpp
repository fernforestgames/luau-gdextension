// Tests for helpers - Helper functions and utilities

#include "doctest.h"
#include "test_fixtures.h"
#include "helpers.h"

using namespace godot;

TEST_SUITE("Helpers")
{
    TEST_CASE_FIXTURE(RawLuaStateFixture, "metatable_matches - checks metatable name")
    {
        // Create a table with a specific metatable
        lua_createtable(L, 0, 0);

        luaL_newmetatable(L, "TestMetatable");
        lua_setmetatable(L, -2);

        CHECK(metatable_matches(L, -1, "TestMetatable"));
        CHECK_FALSE(metatable_matches(L, -1, "OtherMetatable"));

        lua_pop(L, 1);
    }

    TEST_CASE_FIXTURE(RawLuaStateFixture, "metatable_matches - no metatable")
    {
        lua_createtable(L, 0, 0);

        CHECK_FALSE(metatable_matches(L, -1, "AnyMetatable"));

        lua_pop(L, 1);
    }

    TEST_CASE_FIXTURE(RawLuaStateFixture, "generic_lua_concat - concatenates values")
    {
        lua_pushcfunction(L, generic_lua_concat, "test_concat");
        lua_pushstring(L, "Hello");
        lua_pushstring(L, " World");
        lua_call(L, 2, 1);

        CHECK(strcmp(lua_tostring(L, -1), "Hello World") == 0);

        lua_pop(L, 1);
    }

    TEST_CASE("STRING_NAME_TO_UTF8 macro")
    {
        StringName test_name("test_string");
        const char *utf8 = STRING_NAME_TO_UTF8(test_name);

        CHECK(strcmp(utf8, "test_string") == 0);
    }
}
