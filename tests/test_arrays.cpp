// Tests for Godot Array <-> Lua table bridging
// Tests array conversions, nested arrays, and type handling

#include "doctest.h"
#include "test_fixtures.h"
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/variant.hpp>

using namespace godot;

TEST_CASE_FIXTURE(LuaStateFixture, "Array: Simple array conversion Godot -> Lua")
{
    SUBCASE("Integer array")
    {
        Array arr;
        arr.push_back(1);
        arr.push_back(2);
        arr.push_back(3);

        // Use the LuaState wrapper to push
        state->push_array(arr);

        // Should be a table
        CHECK(lua_istable(L, -1));

        // Check length (Lua arrays are 1-indexed)
        int len = lua_objlen(L, -1);
        CHECK(len == 3);

        // Check values
        lua_rawgeti(L, -1, 1);
        CHECK(lua_tonumber(L, -1) == 1);
        lua_pop(L, 1);

        lua_rawgeti(L, -1, 2);
        CHECK(lua_tonumber(L, -1) == 2);
        lua_pop(L, 1);

        lua_rawgeti(L, -1, 3);
        CHECK(lua_tonumber(L, -1) == 3);
        lua_pop(L, 2); // Pop value and table
    }

    SUBCASE("Mixed type array")
    {
        Array arr;
        arr.push_back(42);
        arr.push_back("hello");
        arr.push_back(true);
        arr.push_back(3.14);

        state->push_array(arr);
        CHECK(lua_objlen(L, -1) == 4);

        lua_rawgeti(L, -1, 1);
        CHECK(lua_isnumber(L, -1));
        lua_pop(L, 1);

        lua_rawgeti(L, -1, 2);
        CHECK(lua_isstring(L, -1));
        lua_pop(L, 1);

        lua_rawgeti(L, -1, 3);
        CHECK(lua_isboolean(L, -1));
        lua_pop(L, 1);

        lua_rawgeti(L, -1, 4);
        CHECK(lua_isnumber(L, -1));
        lua_pop(L, 2); // Pop value and table
    }

    SUBCASE("Empty array")
    {
        Array arr;

        state->push_array(arr);
        CHECK(lua_istable(L, -1));
        CHECK(lua_objlen(L, -1) == 0);
        state->pop(1);
    }

}

TEST_CASE_FIXTURE(LuaStateFixture, "Array: Lua table -> Godot Array conversion")
{

    SUBCASE("Simple numeric table")
    {
        const char *code = "return {10, 20, 30, 40}";
        state->load_string(code, "test");
        state->call(0, 1);

        // Convert to Array
        Array result = state->to_array(-1);
        CHECK(result.size() == 4);
        CHECK((int)result[0] == 10);
        CHECK((int)result[1] == 20);
        CHECK((int)result[2] == 30);
        CHECK((int)result[3] == 40);
        state->pop(1);
    }

    SUBCASE("Array detection via isarray")
    {
        // Test that consecutive integer keys are detected as array
        state->do_string("t = {10, 20, 30}", "test");
        state->get_global("t");

        CHECK(state->is_array(-1));
        CHECK(lua_objlen(L, -1) == 3);
        state->pop(1);
    }

}

TEST_CASE_FIXTURE(LuaStateFixture, "Array: Nested arrays")
{

    SUBCASE("2D array")
    {
        Array inner1;
        inner1.push_back(1);
        inner1.push_back(2);

        Array inner2;
        inner2.push_back(3);
        inner2.push_back(4);

        Array outer;
        outer.push_back(inner1);
        outer.push_back(inner2);

        state->push_array(outer);

        // Check outer table
        CHECK(lua_istable(L, -1));
        CHECK(lua_objlen(L, -1) == 2);

        // Check first inner table
        lua_rawgeti(L, -1, 1);
        CHECK(lua_istable(L, -1));
        CHECK(lua_objlen(L, -1) == 2);

        lua_rawgeti(L, -1, 1);
        CHECK(lua_tonumber(L, -1) == 1);
        lua_pop(L, 2); // Pop value and inner table

        // Check second inner table
        lua_rawgeti(L, -1, 2);
        CHECK(lua_istable(L, -1));

        lua_rawgeti(L, -1, 2);
        CHECK(lua_tonumber(L, -1) == 4);
        lua_pop(L, 3); // Pop value, inner table, and outer table
    }

    SUBCASE("Deeply nested arrays")
    {
        Array level3;
        level3.push_back(100);

        Array level2;
        level2.push_back(level3);

        Array level1;
        level1.push_back(level2);

        state->push_array(level1);

        // Navigate down the nested structure
        lua_rawgeti(L, -1, 1);
        CHECK(lua_istable(L, -1));

        lua_rawgeti(L, -1, 1);
        CHECK(lua_istable(L, -1));

        lua_rawgeti(L, -1, 1);
        CHECK(lua_tonumber(L, -1) == 100);
        lua_pop(L, 4); // Pop value and 3 nested tables
    }

}

TEST_CASE_FIXTURE(LuaStateFixture, "Array: Round-trip conversion")
{

    SUBCASE("Simple array round-trip")
    {
        Array original;
        original.push_back(1);
        original.push_back(2);
        original.push_back(3);

        // Push to Lua
        state->push_array(original);
        state->set_global("test_array");

        // Retrieve from Lua
        state->get_global("test_array");
        Array retrieved = state->to_array(-1);

        CHECK(retrieved.size() == 3);
        CHECK((int)retrieved[0] == 1);
        CHECK((int)retrieved[1] == 2);
        CHECK((int)retrieved[2] == 3);
        state->pop(1);
    }

    SUBCASE("Mixed types round-trip")
    {
        Array original;
        original.push_back(42);
        original.push_back("test");
        original.push_back(3.14);
        original.push_back(true);

        state->push_array(original);
        state->set_global("mixed");

        state->get_global("mixed");
        Array retrieved = state->to_array(-1);

        CHECK(retrieved.size() == 4);
        CHECK((int)retrieved[0] == 42);
        CHECK((String)retrieved[1] == "test");
        CHECK((double)retrieved[2] == doctest::Approx(3.14));
        CHECK((bool)retrieved[3] == true);
        state->pop(1);
    }

    SUBCASE("Nested array round-trip")
    {
        Array inner;
        inner.push_back(10);
        inner.push_back(20);

        Array outer;
        outer.push_back(inner);
        outer.push_back(99);

        state->push_array(outer);
        state->set_global("nested");

        state->get_global("nested");
        Array retrieved = state->to_array(-1);

        CHECK(retrieved.size() == 2);

        Array retrieved_inner = retrieved[0];
        CHECK(retrieved_inner.size() == 2);
        CHECK((int)retrieved_inner[0] == 10);
        CHECK((int)retrieved_inner[1] == 20);

        CHECK((int)retrieved[1] == 99);
        state->pop(1);
    }

}

TEST_CASE_FIXTURE(LuaStateFixture, "Array: Array vs Dictionary detection")
{

    SUBCASE("Consecutive integer keys = array")
    {
        const char *code = R"(
            t = {10, 20, 30}
            return t
        )";

        PackedByteArray bytecode = Luau::compile(code);
        state->load_bytecode(bytecode, "test");
        state->resume();

        // Result should be on stack
        CHECK(state->is_array(-1));
        CHECK_FALSE(state->is_dictionary(-1));

        Array arr = state->to_array(-1);
        CHECK(arr.size() == 3);
        state->pop(1);
    }

    SUBCASE("Non-consecutive keys = dictionary")
    {
        const char *code = R"(
            t = {[1] = "a", [3] = "c"}
            return t
        )";

        PackedByteArray bytecode = Luau::compile(code);
        state->load_bytecode(bytecode, "test");
        state->resume();

        // Should be detected as dictionary due to missing index 2
        CHECK_FALSE(state->is_array(-1));
        CHECK(state->is_dictionary(-1));
        state->pop(1);
    }

    SUBCASE("String keys = dictionary")
    {
        const char *code = R"(
            t = {name = "test", value = 42}
            return t
        )";

        PackedByteArray bytecode = Luau::compile(code);
        state->load_bytecode(bytecode, "test");
        state->resume();

        CHECK_FALSE(state->is_array(-1));
        CHECK(state->is_dictionary(-1));
        state->pop(1);
    }

    SUBCASE("Mixed keys = dictionary")
    {
        const char *code = R"(
            t = {10, 20, name = "test"}
            return t
        )";

        PackedByteArray bytecode = Luau::compile(code);
        state->load_bytecode(bytecode, "test");
        state->resume();

        // Has both numeric and string keys -> dictionary
        CHECK_FALSE(state->is_array(-1));
        CHECK(state->is_dictionary(-1));
        state->pop(1);
    }

}

TEST_CASE_FIXTURE(LuaStateFixture, "Array: Edge cases")
{

    SUBCASE("Array with nil values")
    {
        const char *code = R"(
            t = {10, nil, 30}
            return t
        )";

        PackedByteArray bytecode = Luau::compile(code);
        state->load_bytecode(bytecode, "test");
        state->resume();

        // Lua arrays with nil values have shorter length
        // lua_objlen stops at first nil
        Array arr = state->to_array(-1);
        // The actual behavior depends on implementation
        // Typically, nil terminates the array in Lua's perspective
        state->pop(1);
    }

    SUBCASE("Large array")
    {
        Array large;
        for (int i = 0; i < 1000; i++)
        {
            large.push_back(i);
        }

        state->push_array(large);
        state->set_global("large");

        state->get_global("large");
        Array retrieved = state->to_array(-1);

        CHECK(retrieved.size() == 1000);
        CHECK((int)retrieved[0] == 0);
        CHECK((int)retrieved[500] == 500);
        CHECK((int)retrieved[999] == 999);
        state->pop(1);
    }

}
