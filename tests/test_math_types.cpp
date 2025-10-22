// Tests for Godot math type bridging with Luau
// Tests Vector2, Vector2i, Vector3, Vector3i, and Color conversions and operations

#include "doctest.h"
#include "lua_state.h"
#include "lua_godotlib.h"
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/color.hpp>
#include <lua.h>
#include <lualib.h>
#include <luacode.h>
#include <cstring>

using namespace godot;

// Helper to create a LuaState with Godot libs
static lua_State *create_test_state()
{
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaopen_godot(L); // Open Godot math types library
    return L;
}

// Helper to clean up
static void close_test_state(lua_State *L)
{
    lua_close(L);
}

// Helper to execute Luau code (replacement for exec_lua which doesn't exist in Luau)
static int exec_lua(lua_State *L, const char *code)
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

TEST_CASE("Vector2: Construction and round-trip conversion")
{
    lua_State *L = create_test_state();

    SUBCASE("Push Vector2 to Lua and retrieve")
    {
        Vector2 original(3.5, 4.2);
        push_vector2(L, original);

        CHECK(is_vector2(L, -1));

        Vector2 retrieved = to_vector2(L, -1);
        CHECK(retrieved.x == doctest::Approx(3.5));
        CHECK(retrieved.y == doctest::Approx(4.2));
    }

    SUBCASE("Create Vector2 in Lua via constructor")
    {
        exec_lua(L, "v = Vector2(10.5, 20.3)");
        lua_getglobal(L, "v");

        CHECK(is_vector2(L, -1));

        Vector2 result = to_vector2(L, -1);
        CHECK(result.x == doctest::Approx(10.5));
        CHECK(result.y == doctest::Approx(20.3));
    }

    close_test_state(L);
}

TEST_CASE("Vector2: Property access")
{
    lua_State *L = create_test_state();

    SUBCASE("Read properties")
    {
        Vector2 v(7.5, 8.5);
        push_vector2(L, v);
        lua_setglobal(L, "v");

        exec_lua(L, "x_val = v.x");
        exec_lua(L, "y_val = v.y");

        lua_getglobal(L, "x_val");
        CHECK(lua_tonumber(L, -1) == doctest::Approx(7.5));
        lua_pop(L, 1);

        lua_getglobal(L, "y_val");
        CHECK(lua_tonumber(L, -1) == doctest::Approx(8.5));
    }

    SUBCASE("Write properties")
    {
        Vector2 v(1.0, 2.0);
        push_vector2(L, v);
        lua_setglobal(L, "v");

        exec_lua(L, "v.x = 99.5");
        exec_lua(L, "v.y = 88.5");

        lua_getglobal(L, "v");
        Vector2 modified = to_vector2(L, -1);
        CHECK(modified.x == doctest::Approx(99.5));
        CHECK(modified.y == doctest::Approx(88.5));
    }

    close_test_state(L);
}

TEST_CASE("Vector2: Arithmetic operators")
{
    lua_State *L = create_test_state();

    SUBCASE("Addition")
    {
        exec_lua(L, "v1 = Vector2(1.0, 2.0)");
        exec_lua(L, "v2 = Vector2(3.0, 4.0)");
        exec_lua(L, "result = v1 + v2");

        lua_getglobal(L, "result");
        Vector2 result = to_vector2(L, -1);
        CHECK(result.x == doctest::Approx(4.0));
        CHECK(result.y == doctest::Approx(6.0));
    }

    SUBCASE("Subtraction")
    {
        exec_lua(L, "v1 = Vector2(10.0, 20.0)");
        exec_lua(L, "v2 = Vector2(3.0, 7.0)");
        exec_lua(L, "result = v1 - v2");

        lua_getglobal(L, "result");
        Vector2 result = to_vector2(L, -1);
        CHECK(result.x == doctest::Approx(7.0));
        CHECK(result.y == doctest::Approx(13.0));
    }

    SUBCASE("Scalar multiplication")
    {
        exec_lua(L, "v = Vector2(2.0, 3.0)");
        exec_lua(L, "result = v * 2.5");

        lua_getglobal(L, "result");
        Vector2 result = to_vector2(L, -1);
        CHECK(result.x == doctest::Approx(5.0));
        CHECK(result.y == doctest::Approx(7.5));
    }

    SUBCASE("Component-wise multiplication")
    {
        exec_lua(L, "v1 = Vector2(2.0, 3.0)");
        exec_lua(L, "v2 = Vector2(4.0, 5.0)");
        exec_lua(L, "result = v1 * v2");

        lua_getglobal(L, "result");
        Vector2 result = to_vector2(L, -1);
        CHECK(result.x == doctest::Approx(8.0));
        CHECK(result.y == doctest::Approx(15.0));
    }

    SUBCASE("Scalar division")
    {
        exec_lua(L, "v = Vector2(10.0, 20.0)");
        exec_lua(L, "result = v / 2.0");

        lua_getglobal(L, "result");
        Vector2 result = to_vector2(L, -1);
        CHECK(result.x == doctest::Approx(5.0));
        CHECK(result.y == doctest::Approx(10.0));
    }

    SUBCASE("Component-wise division")
    {
        exec_lua(L, "v1 = Vector2(12.0, 20.0)");
        exec_lua(L, "v2 = Vector2(4.0, 5.0)");
        exec_lua(L, "result = v1 / v2");

        lua_getglobal(L, "result");
        Vector2 result = to_vector2(L, -1);
        CHECK(result.x == doctest::Approx(3.0));
        CHECK(result.y == doctest::Approx(4.0));
    }

    SUBCASE("Unary negation")
    {
        exec_lua(L, "v = Vector2(5.0, -3.0)");
        exec_lua(L, "result = -v");

        lua_getglobal(L, "result");
        Vector2 result = to_vector2(L, -1);
        CHECK(result.x == doctest::Approx(-5.0));
        CHECK(result.y == doctest::Approx(3.0));
    }

    close_test_state(L);
}

TEST_CASE("Vector2: Equality and tostring")
{
    lua_State *L = create_test_state();

    SUBCASE("Equality comparison")
    {
        exec_lua(L, "v1 = Vector2(1.0, 2.0)");
        exec_lua(L, "v2 = Vector2(1.0, 2.0)");
        exec_lua(L, "v3 = Vector2(3.0, 4.0)");
        exec_lua(L, "equal = (v1 == v2)");
        exec_lua(L, "not_equal = (v1 == v3)");

        lua_getglobal(L, "equal");
        CHECK(lua_toboolean(L, -1) == true);
        lua_pop(L, 1);

        lua_getglobal(L, "not_equal");
        CHECK(lua_toboolean(L, -1) == false);
    }

    SUBCASE("String conversion")
    {
        exec_lua(L, "v = Vector2(3.5, 4.2)");
        exec_lua(L, "str = tostring(v)");

        lua_getglobal(L, "str");
        const char *str = lua_tostring(L, -1);
        CHECK(str != nullptr);
        // String should contain the coordinates in some form
        // Expected format: "Vector2(3.5, 4.2)" or similar
    }

    close_test_state(L);
}

TEST_CASE("Vector2i: Integer vector operations")
{
    lua_State *L = create_test_state();

    SUBCASE("Construction with integers")
    {
        exec_lua(L, "v = Vector2i(10, 20)");
        lua_getglobal(L, "v");

        CHECK(is_vector2i(L, -1));

        Vector2i result = to_vector2i(L, -1);
        CHECK(result.x == 10);
        CHECK(result.y == 20);
    }

    SUBCASE("Integer arithmetic")
    {
        exec_lua(L, "v1 = Vector2i(10, 20)");
        exec_lua(L, "v2 = Vector2i(3, 7)");
        exec_lua(L, "sum = v1 + v2");
        exec_lua(L, "diff = v1 - v2");

        lua_getglobal(L, "sum");
        Vector2i sum = to_vector2i(L, -1);
        CHECK(sum.x == 13);
        CHECK(sum.y == 27);
        lua_pop(L, 1);

        lua_getglobal(L, "diff");
        Vector2i diff = to_vector2i(L, -1);
        CHECK(diff.x == 7);
        CHECK(diff.y == 13);
    }

    close_test_state(L);
}

TEST_CASE("Vector3: Native vector type operations")
{
    lua_State *L = create_test_state();

    SUBCASE("Construction and retrieval")
    {
        Vector3 original(1.5, 2.5, 3.5);
        push_vector3(L, original);

        CHECK(is_vector3(L, -1));

        Vector3 retrieved = to_vector3(L, -1);
        CHECK(retrieved.x == doctest::Approx(1.5));
        CHECK(retrieved.y == doctest::Approx(2.5));
        CHECK(retrieved.z == doctest::Approx(3.5));
    }

    SUBCASE("Lua constructor")
    {
        exec_lua(L, "v = Vector3(10, 20, 30)");
        lua_getglobal(L, "v");

        Vector3 result = to_vector3(L, -1);
        CHECK(result.x == doctest::Approx(10));
        CHECK(result.y == doctest::Approx(20));
        CHECK(result.z == doctest::Approx(30));
    }

    SUBCASE("Native vector arithmetic")
    {
        // Vector3 uses Luau's native vector type, so arithmetic is built-in
        exec_lua(L, "v1 = Vector3(1, 2, 3)");
        exec_lua(L, "v2 = Vector3(4, 5, 6)");
        exec_lua(L, "sum = v1 + v2");

        lua_getglobal(L, "sum");
        Vector3 sum = to_vector3(L, -1);
        CHECK(sum.x == doctest::Approx(5));
        CHECK(sum.y == doctest::Approx(7));
        CHECK(sum.z == doctest::Approx(9));
    }

    SUBCASE("Property access")
    {
        exec_lua(L, "v = Vector3(7, 8, 9)");
        exec_lua(L, "x = v.x");
        exec_lua(L, "y = v.y");
        exec_lua(L, "z = v.z");

        lua_getglobal(L, "x");
        CHECK(lua_tonumber(L, -1) == doctest::Approx(7));
        lua_pop(L, 1);

        lua_getglobal(L, "y");
        CHECK(lua_tonumber(L, -1) == doctest::Approx(8));
        lua_pop(L, 1);

        lua_getglobal(L, "z");
        CHECK(lua_tonumber(L, -1) == doctest::Approx(9));
    }

    close_test_state(L);
}

TEST_CASE("Vector3i: Integer 3D vector")
{
    lua_State *L = create_test_state();

    SUBCASE("Construction and round-trip")
    {
        Vector3i original(100, 200, 300);
        push_vector3i(L, original);

        CHECK(is_vector3i(L, -1));

        Vector3i retrieved = to_vector3i(L, -1);
        CHECK(retrieved.x == 100);
        CHECK(retrieved.y == 200);
        CHECK(retrieved.z == 300);
    }

    SUBCASE("Operations")
    {
        exec_lua(L, "v = Vector3i(5, 10, 15)");
        exec_lua(L, "doubled = v * 2");

        lua_getglobal(L, "doubled");
        Vector3i result = to_vector3i(L, -1);
        CHECK(result.x == 10);
        CHECK(result.y == 20);
        CHECK(result.z == 30);
    }

    close_test_state(L);
}

TEST_CASE("Color: RGBA color operations")
{
    lua_State *L = create_test_state();

    SUBCASE("Construction with RGB")
    {
        exec_lua(L, "c = Color(1.0, 0.5, 0.0)");
        lua_getglobal(L, "c");

        CHECK(is_color(L, -1));

        Color result = to_color(L, -1);
        CHECK(result.r == doctest::Approx(1.0));
        CHECK(result.g == doctest::Approx(0.5));
        CHECK(result.b == doctest::Approx(0.0));
        CHECK(result.a == doctest::Approx(1.0)); // Default alpha
    }

    SUBCASE("Construction with RGBA")
    {
        exec_lua(L, "c = Color(0.2, 0.4, 0.6, 0.8)");
        lua_getglobal(L, "c");

        Color result = to_color(L, -1);
        CHECK(result.r == doctest::Approx(0.2));
        CHECK(result.g == doctest::Approx(0.4));
        CHECK(result.b == doctest::Approx(0.6));
        CHECK(result.a == doctest::Approx(0.8));
    }

    SUBCASE("Property access and modification")
    {
        exec_lua(L, "c = Color(1, 0, 0, 1)");
        exec_lua(L, "c.g = 0.5");
        exec_lua(L, "c.a = 0.7");

        lua_getglobal(L, "c");
        Color result = to_color(L, -1);
        CHECK(result.r == doctest::Approx(1.0));
        CHECK(result.g == doctest::Approx(0.5));
        CHECK(result.b == doctest::Approx(0.0));
        CHECK(result.a == doctest::Approx(0.7));
    }

    SUBCASE("Color arithmetic")
    {
        exec_lua(L, "c1 = Color(0.2, 0.3, 0.4, 1.0)");
        exec_lua(L, "c2 = Color(0.1, 0.2, 0.1, 0.0)");
        exec_lua(L, "sum = c1 + c2");

        lua_getglobal(L, "sum");
        Color sum = to_color(L, -1);
        CHECK(sum.r == doctest::Approx(0.3));
        CHECK(sum.g == doctest::Approx(0.5));
        CHECK(sum.b == doctest::Approx(0.5));
        CHECK(sum.a == doctest::Approx(1.0));
    }

    SUBCASE("Scalar multiplication")
    {
        exec_lua(L, "c = Color(0.5, 0.5, 0.5, 1.0)");
        exec_lua(L, "brighter = c * 2.0");

        lua_getglobal(L, "brighter");
        Color result = to_color(L, -1);
        CHECK(result.r == doctest::Approx(1.0));
        CHECK(result.g == doctest::Approx(1.0));
        CHECK(result.b == doctest::Approx(1.0));
        CHECK(result.a == doctest::Approx(2.0)); // Alpha is also scaled
    }

    SUBCASE("Round-trip conversion")
    {
        Color original(0.1, 0.2, 0.3, 0.4);
        push_color(L, original);

        Color retrieved = to_color(L, -1);
        CHECK(retrieved.r == doctest::Approx(0.1));
        CHECK(retrieved.g == doctest::Approx(0.2));
        CHECK(retrieved.b == doctest::Approx(0.3));
        CHECK(retrieved.a == doctest::Approx(0.4));
    }

    close_test_state(L);
}

TEST_CASE("Math types: Type checking")
{
    lua_State *L = create_test_state();

    SUBCASE("Correctly identify types")
    {
        push_vector2(L, Vector2(1, 2));
        CHECK(is_vector2(L, -1));
        CHECK_FALSE(is_vector3(L, -1));
        CHECK_FALSE(is_color(L, -1));
        lua_pop(L, 1);

        push_vector3(L, Vector3(1, 2, 3));
        CHECK(is_vector3(L, -1));
        CHECK_FALSE(is_vector2(L, -1));
        CHECK_FALSE(is_color(L, -1));
        lua_pop(L, 1);

        push_color(L, Color(1, 0, 0, 1));
        CHECK(is_color(L, -1));
        CHECK_FALSE(is_vector2(L, -1));
        CHECK_FALSE(is_vector3(L, -1));
    }

    close_test_state(L);
}
