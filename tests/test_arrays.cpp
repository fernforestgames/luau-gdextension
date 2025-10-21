// Tests for Godot Array <-> Lua table bridging
// Tests array conversions, nested arrays, and type handling

#include "doctest.h"
#include "../src/lua_state.h"
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <lua.h>
#include <lualib.h>

using namespace godot;

// Helper to create a LuaState with all libs
static lua_State* create_test_state() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    return L;
}

static void close_test_state(lua_State* L) {
    lua_close(L);
}

TEST_CASE("Array: Simple array conversion Godot -> Lua") {
    lua_State* L = create_test_state();
    LuaState wrapper;
    wrapper.openlibs(LuaState::LIB_ALL);

    SUBCASE("Integer array") {
        Array arr;
        arr.push_back(1);
        arr.push_back(2);
        arr.push_back(3);

        // Use the LuaState wrapper to push
        wrapper.pusharray(arr);
        lua_State* internal_L = (lua_State*)wrapper.getbindproperty("L");

        // Should be a table
        CHECK(lua_istable(internal_L, -1));

        // Check length (Lua arrays are 1-indexed)
        int len = lua_objlen(internal_L, -1);
        CHECK(len == 3);

        // Check values
        lua_rawgeti(internal_L, -1, 1);
        CHECK(lua_tonumber(internal_L, -1) == 1);
        lua_pop(internal_L, 1);

        lua_rawgeti(internal_L, -1, 2);
        CHECK(lua_tonumber(internal_L, -1) == 2);
        lua_pop(internal_L, 1);

        lua_rawgeti(internal_L, -1, 3);
        CHECK(lua_tonumber(internal_L, -1) == 3);
    }

    SUBCASE("Mixed type array") {
        Array arr;
        arr.push_back(42);
        arr.push_back("hello");
        arr.push_back(true);
        arr.push_back(3.14);

        wrapper.pusharray(arr);
        lua_State* internal_L = (lua_State*)wrapper.getbindproperty("L");

        CHECK(lua_objlen(internal_L, -1) == 4);

        lua_rawgeti(internal_L, -1, 1);
        CHECK(lua_isnumber(internal_L, -1));
        lua_pop(internal_L, 1);

        lua_rawgeti(internal_L, -1, 2);
        CHECK(lua_isstring(internal_L, -1));
        lua_pop(internal_L, 1);

        lua_rawgeti(internal_L, -1, 3);
        CHECK(lua_isboolean(internal_L, -1));
        lua_pop(internal_L, 1);

        lua_rawgeti(internal_L, -1, 4);
        CHECK(lua_isnumber(internal_L, -1));
    }

    SUBCASE("Empty array") {
        Array arr;

        wrapper.pusharray(arr);
        lua_State* internal_L = (lua_State*)wrapper.getbindproperty("L");

        CHECK(lua_istable(internal_L, -1));
        CHECK(lua_objlen(internal_L, -1) == 0);
    }

    close_test_state(L);
}

TEST_CASE("Array: Lua table -> Godot Array conversion") {
    lua_State* L = create_test_state();
    LuaState wrapper;
    wrapper.openlibs(LuaState::LIB_ALL);

    SUBCASE("Simple numeric table") {
        const char* code = "return {10, 20, 30, 40}";
        luaL_loadstring(L, code);
        lua_call(L, 0, 1);

        // Manually convert using internal API (would need access to LuaState internals)
        // This test assumes we have access to toarray method
        // For now, we'll test the high-level wrapper behavior
    }

    SUBCASE("Array detection via isarray") {
        // Test that consecutive integer keys are detected as array
        luaL_dostring(L, "t = {10, 20, 30}");
        lua_getglobal(L, "t");

        // Would use wrapper.isarray(-1) if we had access
        // For direct testing, check table structure
        CHECK(lua_istable(L, -1));
        CHECK(lua_objlen(L, -1) == 3);
    }

    close_test_state(L);
}

TEST_CASE("Array: Nested arrays") {
    lua_State* L = create_test_state();
    LuaState wrapper;
    wrapper.openlibs(LuaState::LIB_ALL);

    SUBCASE("2D array") {
        Array inner1;
        inner1.push_back(1);
        inner1.push_back(2);

        Array inner2;
        inner2.push_back(3);
        inner2.push_back(4);

        Array outer;
        outer.push_back(inner1);
        outer.push_back(inner2);

        wrapper.pusharray(outer);
        lua_State* internal_L = (lua_State*)wrapper.getbindproperty("L");

        // Check outer table
        CHECK(lua_istable(internal_L, -1));
        CHECK(lua_objlen(internal_L, -1) == 2);

        // Check first inner table
        lua_rawgeti(internal_L, -1, 1);
        CHECK(lua_istable(internal_L, -1));
        CHECK(lua_objlen(internal_L, -1) == 2);

        lua_rawgeti(internal_L, -1, 1);
        CHECK(lua_tonumber(internal_L, -1) == 1);
        lua_pop(internal_L, 2); // Pop value and inner table

        // Check second inner table
        lua_rawgeti(internal_L, -1, 2);
        CHECK(lua_istable(internal_L, -1));

        lua_rawgeti(internal_L, -1, 2);
        CHECK(lua_tonumber(internal_L, -1) == 4);
    }

    SUBCASE("Deeply nested arrays") {
        Array level3;
        level3.push_back(100);

        Array level2;
        level2.push_back(level3);

        Array level1;
        level1.push_back(level2);

        wrapper.pusharray(level1);
        lua_State* internal_L = (lua_State*)wrapper.getbindproperty("L");

        // Navigate down the nested structure
        lua_rawgeti(internal_L, -1, 1);
        CHECK(lua_istable(internal_L, -1));

        lua_rawgeti(internal_L, -1, 1);
        CHECK(lua_istable(internal_L, -1));

        lua_rawgeti(internal_L, -1, 1);
        CHECK(lua_tonumber(internal_L, -1) == 100);
    }

    close_test_state(L);
}

TEST_CASE("Array: Round-trip conversion") {
    LuaState L;
    L.openlibs(LuaState::LIB_ALL);

    SUBCASE("Simple array round-trip") {
        Array original;
        original.push_back(1);
        original.push_back(2);
        original.push_back(3);

        // Push to Lua
        L.pusharray(original);
        L.setglobal("test_array");

        // Retrieve from Lua
        L.getglobal("test_array");
        Array retrieved = L.toarray(-1);

        CHECK(retrieved.size() == 3);
        CHECK((int)retrieved[0] == 1);
        CHECK((int)retrieved[1] == 2);
        CHECK((int)retrieved[2] == 3);
    }

    SUBCASE("Mixed types round-trip") {
        Array original;
        original.push_back(42);
        original.push_back("test");
        original.push_back(3.14);
        original.push_back(true);

        L.pusharray(original);
        L.setglobal("mixed");

        L.getglobal("mixed");
        Array retrieved = L.toarray(-1);

        CHECK(retrieved.size() == 4);
        CHECK((int)retrieved[0] == 42);
        CHECK((String)retrieved[1] == "test");
        CHECK((double)retrieved[2] == doctest::Approx(3.14));
        CHECK((bool)retrieved[3] == true);
    }

    SUBCASE("Nested array round-trip") {
        Array inner;
        inner.push_back(10);
        inner.push_back(20);

        Array outer;
        outer.push_back(inner);
        outer.push_back(99);

        L.pusharray(outer);
        L.setglobal("nested");

        L.getglobal("nested");
        Array retrieved = L.toarray(-1);

        CHECK(retrieved.size() == 2);

        Array retrieved_inner = retrieved[0];
        CHECK(retrieved_inner.size() == 2);
        CHECK((int)retrieved_inner[0] == 10);
        CHECK((int)retrieved_inner[1] == 20);

        CHECK((int)retrieved[1] == 99);
    }
}

TEST_CASE("Array: Array vs Dictionary detection") {
    LuaState L;
    L.openlibs(LuaState::LIB_ALL);

    SUBCASE("Consecutive integer keys = array") {
        const char* code = R"(
            t = {10, 20, 30}
            return t
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        // Result should be on stack
        CHECK(L.isarray(-1));
        CHECK_FALSE(L.isdictionary(-1));

        Array arr = L.toarray(-1);
        CHECK(arr.size() == 3);
    }

    SUBCASE("Non-consecutive keys = dictionary") {
        const char* code = R"(
            t = {[1] = "a", [3] = "c"}
            return t
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        // Should be detected as dictionary due to missing index 2
        CHECK_FALSE(L.isarray(-1));
        CHECK(L.isdictionary(-1));
    }

    SUBCASE("String keys = dictionary") {
        const char* code = R"(
            t = {name = "test", value = 42}
            return t
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        CHECK_FALSE(L.isarray(-1));
        CHECK(L.isdictionary(-1));
    }

    SUBCASE("Mixed keys = dictionary") {
        const char* code = R"(
            t = {10, 20, name = "test"}
            return t
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        // Has both numeric and string keys -> dictionary
        CHECK_FALSE(L.isarray(-1));
        CHECK(L.isdictionary(-1));
    }
}

TEST_CASE("Array: Edge cases") {
    LuaState L;
    L.openlibs(LuaState::LIB_ALL);

    SUBCASE("Array with nil values") {
        const char* code = R"(
            t = {10, nil, 30}
            return t
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        // Lua arrays with nil values have shorter length
        // lua_objlen stops at first nil
        Array arr = L.toarray(-1);
        // The actual behavior depends on implementation
        // Typically, nil terminates the array in Lua's perspective
    }

    SUBCASE("Large array") {
        Array large;
        for (int i = 0; i < 1000; i++) {
            large.push_back(i);
        }

        L.pusharray(large);
        L.setglobal("large");

        L.getglobal("large");
        Array retrieved = L.toarray(-1);

        CHECK(retrieved.size() == 1000);
        CHECK((int)retrieved[0] == 0);
        CHECK((int)retrieved[500] == 500);
        CHECK((int)retrieved[999] == 999);
    }
}
