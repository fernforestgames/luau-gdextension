// Tests for helpers - Helper functions and utilities

#include "doctest.h"
#include "test_fixtures.h"
#include "helpers.h"

using namespace gdluau;
using namespace godot;

TEST_SUITE("Helpers")
{
    TEST_CASE_FIXTURE(RawLuaStateFixture, "generic_lua_concat - concatenates values")
    {
        lua_pushcfunction(L, generic_lua_concat, "test_concat");
        lua_pushstring(L, "Hello");
        lua_pushstring(L, " World");
        lua_call(L, 2, 1);

        CHECK(strcmp(lua_tostring(L, -1), "Hello World") == 0);

        lua_pop(L, 1);
    }
}
