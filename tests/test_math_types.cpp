// Tests for Godot math type bridging with Luau
// Tests all Godot math types: vectors, colors, rectangles, transforms, etc.

#include "doctest.h"
#include "test_fixtures.h"
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include <godot_cpp/variant/vector4i.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/rect2i.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/plane.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/projection.hpp>

using namespace godot;

TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector2: Construction and round-trip conversion")
{
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
        exec_lua("v = Vector2(10.5, 20.3)");
        lua_getglobal(L, "v");

        CHECK(is_vector2(L, -1));

        Vector2 result = to_vector2(L, -1);
        CHECK(result.x == doctest::Approx(10.5));
        CHECK(result.y == doctest::Approx(20.3));
    }
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector2: Property access")
{
    SUBCASE("Read properties")
    {
        Vector2 v(7.5, 8.5);
        push_vector2(L, v);
        lua_setglobal(L, "v");

        exec_lua("x_val = v.x");
        exec_lua("y_val = v.y");

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

        exec_lua("v.x = 99.5");
        exec_lua("v.y = 88.5");

        lua_getglobal(L, "v");
        Vector2 modified = to_vector2(L, -1);
        CHECK(modified.x == doctest::Approx(99.5));
        CHECK(modified.y == doctest::Approx(88.5));
    }
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector2: Arithmetic operators")
{
    SUBCASE("Addition")
    {
        exec_lua("v1 = Vector2(1.0, 2.0)");
        exec_lua("v2 = Vector2(3.0, 4.0)");
        exec_lua("result = v1 + v2");

        lua_getglobal(L, "result");
        Vector2 result = to_vector2(L, -1);
        CHECK(result.x == doctest::Approx(4.0));
        CHECK(result.y == doctest::Approx(6.0));
    }

    SUBCASE("Subtraction")
    {
        exec_lua("v1 = Vector2(10.0, 20.0)");
        exec_lua("v2 = Vector2(3.0, 7.0)");
        exec_lua("result = v1 - v2");

        lua_getglobal(L, "result");
        Vector2 result = to_vector2(L, -1);
        CHECK(result.x == doctest::Approx(7.0));
        CHECK(result.y == doctest::Approx(13.0));
    }

    SUBCASE("Scalar multiplication")
    {
        exec_lua("v = Vector2(2.0, 3.0)");
        exec_lua("result = v * 2.5");

        lua_getglobal(L, "result");
        Vector2 result = to_vector2(L, -1);
        CHECK(result.x == doctest::Approx(5.0));
        CHECK(result.y == doctest::Approx(7.5));
    }

    SUBCASE("Component-wise multiplication")
    {
        exec_lua("v1 = Vector2(2.0, 3.0)");
        exec_lua("v2 = Vector2(4.0, 5.0)");
        exec_lua("result = v1 * v2");

        lua_getglobal(L, "result");
        Vector2 result = to_vector2(L, -1);
        CHECK(result.x == doctest::Approx(8.0));
        CHECK(result.y == doctest::Approx(15.0));
    }

    SUBCASE("Scalar division")
    {
        exec_lua("v = Vector2(10.0, 20.0)");
        exec_lua("result = v / 2.0");

        lua_getglobal(L, "result");
        Vector2 result = to_vector2(L, -1);
        CHECK(result.x == doctest::Approx(5.0));
        CHECK(result.y == doctest::Approx(10.0));
    }

    SUBCASE("Component-wise division")
    {
        exec_lua("v1 = Vector2(12.0, 20.0)");
        exec_lua("v2 = Vector2(4.0, 5.0)");
        exec_lua("result = v1 / v2");

        lua_getglobal(L, "result");
        Vector2 result = to_vector2(L, -1);
        CHECK(result.x == doctest::Approx(3.0));
        CHECK(result.y == doctest::Approx(4.0));
    }

    SUBCASE("Unary negation")
    {
        exec_lua("v = Vector2(5.0, -3.0)");
        exec_lua("result = -v");

        lua_getglobal(L, "result");
        Vector2 result = to_vector2(L, -1);
        CHECK(result.x == doctest::Approx(-5.0));
        CHECK(result.y == doctest::Approx(3.0));
    }
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector2: Equality and tostring")
{

    SUBCASE("Equality comparison")
    {
        exec_lua("v1 = Vector2(1.0, 2.0)");
        exec_lua("v2 = Vector2(1.0, 2.0)");
        exec_lua("v3 = Vector2(3.0, 4.0)");
        exec_lua("equal = (v1 == v2)");
        exec_lua("not_equal = (v1 == v3)");

        lua_getglobal(L, "equal");
        CHECK(lua_toboolean(L, -1));
        lua_pop(L, 1);

        lua_getglobal(L, "not_equal");
        CHECK(!lua_toboolean(L, -1));
    }

    SUBCASE("String conversion")
    {
        exec_lua("v = Vector2(3.5, 4.2)");
        exec_lua("str = tostring(v)");

        lua_getglobal(L, "str");
        const char *str = lua_tostring(L, -1);
        CHECK(str != nullptr);
        // String should contain the coordinates in some form
        // Expected format: "Vector2(3.5, 4.2)" or similar
    }
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector2i: Integer vector operations")
{

    SUBCASE("Construction with integers")
    {
        exec_lua("v = Vector2i(10, 20)");
        lua_getglobal(L, "v");

        CHECK(is_vector2i(L, -1));

        Vector2i result = to_vector2i(L, -1);
        CHECK(result.x == 10);
        CHECK(result.y == 20);
    }

    SUBCASE("Integer arithmetic")
    {
        exec_lua("v1 = Vector2i(10, 20)");
        exec_lua("v2 = Vector2i(3, 7)");
        exec_lua("sum = v1 + v2");
        exec_lua("diff = v1 - v2");

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
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector3: Native vector type operations")
{

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
        exec_lua("v = Vector3(10, 20, 30)");
        lua_getglobal(L, "v");

        Vector3 result = to_vector3(L, -1);
        CHECK(result.x == doctest::Approx(10));
        CHECK(result.y == doctest::Approx(20));
        CHECK(result.z == doctest::Approx(30));
    }

    SUBCASE("Native vector arithmetic")
    {
        // Vector3 uses Luau's native vector type, so arithmetic is built-in
        exec_lua("v1 = Vector3(1, 2, 3)");
        exec_lua("v2 = Vector3(4, 5, 6)");
        exec_lua("sum = v1 + v2");

        lua_getglobal(L, "sum");
        Vector3 sum = to_vector3(L, -1);
        CHECK(sum.x == doctest::Approx(5));
        CHECK(sum.y == doctest::Approx(7));
        CHECK(sum.z == doctest::Approx(9));
    }

    SUBCASE("Property access")
    {
        exec_lua("v = Vector3(7, 8, 9)");
        exec_lua("x = v.x");
        exec_lua("y = v.y");
        exec_lua("z = v.z");

        lua_getglobal(L, "x");
        CHECK(lua_tonumber(L, -1) == doctest::Approx(7));
        lua_pop(L, 1);

        lua_getglobal(L, "y");
        CHECK(lua_tonumber(L, -1) == doctest::Approx(8));
        lua_pop(L, 1);

        lua_getglobal(L, "z");
        CHECK(lua_tonumber(L, -1) == doctest::Approx(9));
    }
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector3i: Integer 3D vector")
{

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
        exec_lua("v = Vector3i(5, 10, 15)");
        exec_lua("doubled = v * 2");

        lua_getglobal(L, "doubled");
        Vector3i result = to_vector3i(L, -1);
        CHECK(result.x == 10);
        CHECK(result.y == 20);
        CHECK(result.z == 30);
    }
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "Color: RGBA color operations")
{

    SUBCASE("Construction with RGB")
    {
        exec_lua("c = Color(1.0, 0.5, 0.0)");
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
        exec_lua("c = Color(0.2, 0.4, 0.6, 0.8)");
        lua_getglobal(L, "c");

        Color result = to_color(L, -1);
        CHECK(result.r == doctest::Approx(0.2));
        CHECK(result.g == doctest::Approx(0.4));
        CHECK(result.b == doctest::Approx(0.6));
        CHECK(result.a == doctest::Approx(0.8));
    }

    SUBCASE("Property access and modification")
    {
        exec_lua("c = Color(1, 0, 0, 1)");
        exec_lua("c.g = 0.5");
        exec_lua("c.a = 0.7");

        lua_getglobal(L, "c");
        Color result = to_color(L, -1);
        CHECK(result.r == doctest::Approx(1.0));
        CHECK(result.g == doctest::Approx(0.5));
        CHECK(result.b == doctest::Approx(0.0));
        CHECK(result.a == doctest::Approx(0.7));
    }

    SUBCASE("Color arithmetic")
    {
        exec_lua("c1 = Color(0.2, 0.3, 0.4, 1.0)");
        exec_lua("c2 = Color(0.1, 0.2, 0.1, 0.0)");
        exec_lua("sum = c1 + c2");

        lua_getglobal(L, "sum");
        Color sum = to_color(L, -1);
        CHECK(sum.r == doctest::Approx(0.3));
        CHECK(sum.g == doctest::Approx(0.5));
        CHECK(sum.b == doctest::Approx(0.5));
        CHECK(sum.a == doctest::Approx(1.0));
    }

    SUBCASE("Scalar multiplication")
    {
        exec_lua("c = Color(0.5, 0.5, 0.5, 1.0)");
        exec_lua("brighter = c * 2.0");

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
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "Math types: Type checking")
{

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
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector4: Construction and round-trip conversion")
{
    SUBCASE("Push Vector4 to Lua and retrieve")
    {
        Vector4 original(1.5, 2.5, 3.5, 4.5);
        push_vector4(L, original);

        CHECK(is_vector4(L, -1));

        Vector4 retrieved = to_vector4(L, -1);
        CHECK(retrieved.x == doctest::Approx(1.5));
        CHECK(retrieved.y == doctest::Approx(2.5));
        CHECK(retrieved.z == doctest::Approx(3.5));
        CHECK(retrieved.w == doctest::Approx(4.5));
    }

    SUBCASE("Create Vector4 in Lua via constructor")
    {
        exec_lua("v = Vector4(10.5, 20.3, 30.1, 40.9)");
        lua_getglobal(L, "v");

        CHECK(is_vector4(L, -1));

        Vector4 result = to_vector4(L, -1);
        CHECK(result.x == doctest::Approx(10.5));
        CHECK(result.y == doctest::Approx(20.3));
        CHECK(result.z == doctest::Approx(30.1));
        CHECK(result.w == doctest::Approx(40.9));
    }
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector4: Arithmetic operators")
{
    SUBCASE("Addition")
    {
        exec_lua("result = Vector4(1, 2, 3, 4) + Vector4(5, 6, 7, 8)");
        lua_getglobal(L, "result");
        Vector4 result = to_vector4(L, -1);
        CHECK(result.x == doctest::Approx(6.0));
        CHECK(result.y == doctest::Approx(8.0));
        CHECK(result.z == doctest::Approx(10.0));
        CHECK(result.w == doctest::Approx(12.0));
    }

    SUBCASE("Scalar multiplication")
    {
        exec_lua("result = Vector4(2, 3, 4, 5) * 2.5");
        lua_getglobal(L, "result");
        Vector4 result = to_vector4(L, -1);
        CHECK(result.x == doctest::Approx(5.0));
        CHECK(result.y == doctest::Approx(7.5));
        CHECK(result.z == doctest::Approx(10.0));
        CHECK(result.w == doctest::Approx(12.5));
    }

    SUBCASE("Unary negation")
    {
        exec_lua("result = -Vector4(5, -3, 2, -1)");
        lua_getglobal(L, "result");
        Vector4 result = to_vector4(L, -1);
        CHECK(result.x == doctest::Approx(-5.0));
        CHECK(result.y == doctest::Approx(3.0));
        CHECK(result.z == doctest::Approx(-2.0));
        CHECK(result.w == doctest::Approx(1.0));
    }
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector4i: Construction and operators")
{
    SUBCASE("Constructor and property access")
    {
        exec_lua("v = Vector4i(10, 20, 30, 40)");
        lua_getglobal(L, "v");

        CHECK(is_vector4i(L, -1));

        Vector4i result = to_vector4i(L, -1);
        CHECK(result.x == 10);
        CHECK(result.y == 20);
        CHECK(result.z == 30);
        CHECK(result.w == 40);
    }

    SUBCASE("Addition")
    {
        exec_lua("result = Vector4i(1, 2, 3, 4) + Vector4i(5, 6, 7, 8)");
        lua_getglobal(L, "result");
        Vector4i result = to_vector4i(L, -1);
        CHECK(result.x == 6);
        CHECK(result.y == 8);
        CHECK(result.z == 10);
        CHECK(result.w == 12);
    }
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "Rect2: Construction and properties")
{
    SUBCASE("Push Rect2 to Lua and retrieve")
    {
        Rect2 original(10.0, 20.0, 100.0, 200.0);
        push_rect2(L, original);

        CHECK(is_rect2(L, -1));

        Rect2 retrieved = to_rect2(L, -1);
        CHECK(retrieved.position.x == doctest::Approx(10.0));
        CHECK(retrieved.position.y == doctest::Approx(20.0));
        CHECK(retrieved.size.x == doctest::Approx(100.0));
        CHECK(retrieved.size.y == doctest::Approx(200.0));
    }

    SUBCASE("Create Rect2 in Lua via constructor")
    {
        exec_lua("r = Rect2(5, 10, 50, 100)");
        lua_getglobal(L, "r");

        CHECK(is_rect2(L, -1));

        Rect2 result = to_rect2(L, -1);
        CHECK(result.position.x == doctest::Approx(5.0));
        CHECK(result.position.y == doctest::Approx(10.0));
        CHECK(result.size.x == doctest::Approx(50.0));
        CHECK(result.size.y == doctest::Approx(100.0));
    }

    SUBCASE("Property access")
    {
        exec_lua("r = Rect2(1, 2, 3, 4)");
        exec_lua("x = r.x; y = r.y; w = r.width; h = r.height");

        lua_getglobal(L, "x");
        CHECK(lua_tonumber(L, -1) == doctest::Approx(1.0));
        lua_pop(L, 1);

        lua_getglobal(L, "w");
        CHECK(lua_tonumber(L, -1) == doctest::Approx(3.0));
    }
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "Rect2i: Construction and properties")
{
    SUBCASE("Constructor")
    {
        exec_lua("r = Rect2i(10, 20, 100, 200)");
        lua_getglobal(L, "r");

        CHECK(is_rect2i(L, -1));

        Rect2i result = to_rect2i(L, -1);
        CHECK(result.position.x == 10);
        CHECK(result.position.y == 20);
        CHECK(result.size.x == 100);
        CHECK(result.size.y == 200);
    }
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "AABB: Construction and properties")
{
    SUBCASE("Push AABB to Lua and retrieve")
    {
        AABB original(Vector3(1, 2, 3), Vector3(10, 20, 30));
        push_aabb(L, original);

        CHECK(is_aabb(L, -1));

        AABB retrieved = to_aabb(L, -1);
        CHECK(retrieved.position.x == doctest::Approx(1.0));
        CHECK(retrieved.position.y == doctest::Approx(2.0));
        CHECK(retrieved.position.z == doctest::Approx(3.0));
        CHECK(retrieved.size.x == doctest::Approx(10.0));
        CHECK(retrieved.size.y == doctest::Approx(20.0));
        CHECK(retrieved.size.z == doctest::Approx(30.0));
    }

    SUBCASE("Create AABB in Lua via constructor")
    {
        exec_lua("aabb = AABB(5, 10, 15, 50, 100, 150)");
        lua_getglobal(L, "aabb");

        CHECK(is_aabb(L, -1));

        AABB result = to_aabb(L, -1);
        CHECK(result.position.x == doctest::Approx(5.0));
        CHECK(result.position.y == doctest::Approx(10.0));
        CHECK(result.position.z == doctest::Approx(15.0));
        CHECK(result.size.x == doctest::Approx(50.0));
        CHECK(result.size.y == doctest::Approx(100.0));
        CHECK(result.size.z == doctest::Approx(150.0));
    }
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "Plane: Construction and properties")
{
    SUBCASE("Push Plane to Lua and retrieve")
    {
        Plane original(1.0, 0.0, 0.0, 5.0);
        push_plane(L, original);

        CHECK(is_plane(L, -1));

        Plane retrieved = to_plane(L, -1);
        CHECK(retrieved.normal.x == doctest::Approx(1.0));
        CHECK(retrieved.normal.y == doctest::Approx(0.0));
        CHECK(retrieved.normal.z == doctest::Approx(0.0));
        CHECK(retrieved.d == doctest::Approx(5.0));
    }

    SUBCASE("Create Plane in Lua via constructor")
    {
        exec_lua("p = Plane(0, 1, 0, 10)");
        lua_getglobal(L, "p");

        CHECK(is_plane(L, -1));

        Plane result = to_plane(L, -1);
        CHECK(result.normal.x == doctest::Approx(0.0));
        CHECK(result.normal.y == doctest::Approx(1.0));
        CHECK(result.normal.z == doctest::Approx(0.0));
        CHECK(result.d == doctest::Approx(10.0));
    }

    SUBCASE("Unary negation")
    {
        exec_lua("result = -Plane(1, 0, 0, 5)");
        lua_getglobal(L, "result");
        Plane result = to_plane(L, -1);
        CHECK(result.normal.x == doctest::Approx(-1.0));
        CHECK(result.d == doctest::Approx(-5.0));
    }
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "Quaternion: Construction and operators")
{
    SUBCASE("Push Quaternion to Lua and retrieve")
    {
        Quaternion original(0.0, 0.0, 0.0, 1.0);
        push_quaternion(L, original);

        CHECK(is_quaternion(L, -1));

        Quaternion retrieved = to_quaternion(L, -1);
        CHECK(retrieved.x == doctest::Approx(0.0));
        CHECK(retrieved.y == doctest::Approx(0.0));
        CHECK(retrieved.z == doctest::Approx(0.0));
        CHECK(retrieved.w == doctest::Approx(1.0));
    }

    SUBCASE("Create Quaternion in Lua via constructor")
    {
        exec_lua("q = Quaternion(1, 2, 3, 4)");
        lua_getglobal(L, "q");

        CHECK(is_quaternion(L, -1));

        Quaternion result = to_quaternion(L, -1);
        CHECK(result.x == doctest::Approx(1.0));
        CHECK(result.y == doctest::Approx(2.0));
        CHECK(result.z == doctest::Approx(3.0));
        CHECK(result.w == doctest::Approx(4.0));
    }

    SUBCASE("Addition")
    {
        exec_lua("result = Quaternion(1, 2, 3, 4) + Quaternion(5, 6, 7, 8)");
        lua_getglobal(L, "result");
        Quaternion result = to_quaternion(L, -1);
        CHECK(result.x == doctest::Approx(6.0));
        CHECK(result.y == doctest::Approx(8.0));
        CHECK(result.z == doctest::Approx(10.0));
        CHECK(result.w == doctest::Approx(12.0));
    }

    SUBCASE("Scalar multiplication")
    {
        exec_lua("result = Quaternion(1, 2, 3, 4) * 2.0");
        lua_getglobal(L, "result");
        Quaternion result = to_quaternion(L, -1);
        CHECK(result.x == doctest::Approx(2.0));
        CHECK(result.y == doctest::Approx(4.0));
        CHECK(result.z == doctest::Approx(6.0));
        CHECK(result.w == doctest::Approx(8.0));
    }
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "Basis: Construction")
{
    SUBCASE("Push Basis to Lua and retrieve")
    {
        Basis original; // Identity
        push_basis(L, original);

        CHECK(is_basis(L, -1));

        Basis retrieved = to_basis(L, -1);
        CHECK(retrieved == Basis()); // Should be identity
    }

    SUBCASE("Create Basis in Lua via constructor")
    {
        exec_lua("b = Basis()");
        lua_getglobal(L, "b");

        CHECK(is_basis(L, -1));

        Basis result = to_basis(L, -1);
        CHECK(result == Basis()); // Should be identity
    }
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "Transform2D: Construction")
{
    SUBCASE("Push Transform2D to Lua and retrieve")
    {
        Transform2D original; // Identity
        push_transform2d(L, original);

        CHECK(is_transform2d(L, -1));

        Transform2D retrieved = to_transform2d(L, -1);
        CHECK(retrieved == Transform2D()); // Should be identity
    }

    SUBCASE("Create Transform2D in Lua via constructor")
    {
        exec_lua("t = Transform2D()");
        lua_getglobal(L, "t");

        CHECK(is_transform2d(L, -1));

        Transform2D result = to_transform2d(L, -1);
        CHECK(result == Transform2D()); // Should be identity
    }
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "Transform3D: Construction")
{
    SUBCASE("Push Transform3D to Lua and retrieve")
    {
        Transform3D original; // Identity
        push_transform3d(L, original);

        CHECK(is_transform3d(L, -1));

        Transform3D retrieved = to_transform3d(L, -1);
        CHECK(retrieved == Transform3D()); // Should be identity
    }

    SUBCASE("Create Transform3D in Lua via constructor")
    {
        exec_lua("t = Transform3D()");
        lua_getglobal(L, "t");

        CHECK(is_transform3d(L, -1));

        Transform3D result = to_transform3d(L, -1);
        CHECK(result == Transform3D()); // Should be identity
    }
}

TEST_CASE_FIXTURE(RawLuaStateFixture, "Projection: Construction")
{
    SUBCASE("Push Projection to Lua and retrieve")
    {
        Projection original; // Identity
        push_projection(L, original);

        CHECK(is_projection(L, -1));

        Projection retrieved = to_projection(L, -1);
        CHECK(retrieved == Projection()); // Should be identity
    }

    SUBCASE("Create Projection in Lua via constructor")
    {
        exec_lua("p = Projection()");
        lua_getglobal(L, "p");

        CHECK(is_projection(L, -1));

        Projection result = to_projection(L, -1);
        CHECK(result == Projection()); // Should be identity
    }
}
