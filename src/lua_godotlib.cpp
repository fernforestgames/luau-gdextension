#include "lua_godotlib.h"
#include <godot_cpp/core/error_macros.hpp>
#include <lualib.h>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include <godot_cpp/variant/vector4i.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/rect2i.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/plane.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/projection.hpp>
#include <godot_cpp/variant/callable.hpp>
#include "lua_callable.h"
#include <godot_cpp/variant/array.hpp>
#include <cstdio>

using namespace godot;

// =============================================================================
// Vector2
// =============================================================================

static int vector2_constructor(lua_State *L)
{
    double x = luaL_optnumber(L, 1, 0.0);
    double y = luaL_optnumber(L, 2, 0.0);
    push_vector2(L, Vector2(x, y));
    return 1;
}

static int vector2_index(lua_State *L)
{
    Vector2 *v = static_cast<Vector2 *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "x") == 0)
    {
        lua_pushnumber(L, v->x);
        return 1;
    }
    else if (strcmp(key, "y") == 0)
    {
        lua_pushnumber(L, v->y);
        return 1;
    }

    return 0;
}

static int vector2_newindex(lua_State *L)
{
    Vector2 *v = static_cast<Vector2 *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);
    double value = luaL_checknumber(L, 3);

    if (strcmp(key, "x") == 0)
    {
        v->x = value;
    }
    else if (strcmp(key, "y") == 0)
    {
        v->y = value;
    }

    return 0;
}

static int vector2_add(lua_State *L)
{
    Vector2 *a = static_cast<Vector2 *>(lua_touserdata(L, 1));
    Vector2 *b = static_cast<Vector2 *>(lua_touserdata(L, 2));
    push_vector2(L, *a + *b);
    return 1;
}

static int vector2_sub(lua_State *L)
{
    Vector2 *a = static_cast<Vector2 *>(lua_touserdata(L, 1));
    Vector2 *b = static_cast<Vector2 *>(lua_touserdata(L, 2));
    push_vector2(L, *a - *b);
    return 1;
}

static int vector2_mul(lua_State *L)
{
    Vector2 *a = static_cast<Vector2 *>(lua_touserdata(L, 1));

    if (lua_isnumber(L, 2))
    {
        double scalar = lua_tonumber(L, 2);
        push_vector2(L, *a * scalar);
    }
    else
    {
        Vector2 *b = static_cast<Vector2 *>(lua_touserdata(L, 2));
        push_vector2(L, *a * *b);
    }

    return 1;
}

static int vector2_div(lua_State *L)
{
    Vector2 *a = static_cast<Vector2 *>(lua_touserdata(L, 1));

    if (lua_isnumber(L, 2))
    {
        double scalar = lua_tonumber(L, 2);
        push_vector2(L, *a / scalar);
    }
    else
    {
        Vector2 *b = static_cast<Vector2 *>(lua_touserdata(L, 2));
        push_vector2(L, *a / *b);
    }

    return 1;
}

static int vector2_unm(lua_State *L)
{
    Vector2 *v = static_cast<Vector2 *>(lua_touserdata(L, 1));
    push_vector2(L, -*v);
    return 1;
}

static int vector2_eq(lua_State *L)
{
    Vector2 *a = static_cast<Vector2 *>(lua_touserdata(L, 1));
    Vector2 *b = static_cast<Vector2 *>(lua_touserdata(L, 2));
    lua_pushboolean(L, *a == *b);
    return 1;
}

static int vector2_tostring(lua_State *L)
{
    Vector2 *v = static_cast<Vector2 *>(lua_touserdata(L, 1));
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Vector2(%g, %g)", v->x, v->y);
    lua_pushstring(L, buffer);
    return 1;
}

static void register_vector2_metatable(lua_State *L)
{
    luaL_newmetatable(L, "Vector2");

    lua_pushcfunction(L, vector2_index, "__index");
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, vector2_newindex, "__newindex");
    lua_setfield(L, -2, "__newindex");

    lua_pushcfunction(L, vector2_add, "__add");
    lua_setfield(L, -2, "__add");

    lua_pushcfunction(L, vector2_sub, "__sub");
    lua_setfield(L, -2, "__sub");

    lua_pushcfunction(L, vector2_mul, "__mul");
    lua_setfield(L, -2, "__mul");

    lua_pushcfunction(L, vector2_div, "__div");
    lua_setfield(L, -2, "__div");

    lua_pushcfunction(L, vector2_unm, "__unm");
    lua_setfield(L, -2, "__unm");

    lua_pushcfunction(L, vector2_eq, "__eq");
    lua_setfield(L, -2, "__eq");

    lua_pushcfunction(L, vector2_tostring, "__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_setuserdatametatable(L, LUA_TAG_VECTOR2);

    // Register constructor
    lua_pushcfunction(L, vector2_constructor, "Vector2");
    lua_setglobal(L, "Vector2");
}

// =============================================================================
// Vector2i
// =============================================================================

static int vector2i_constructor(lua_State *L)
{
    int x = luaL_optinteger(L, 1, 0);
    int y = luaL_optinteger(L, 2, 0);
    push_vector2i(L, Vector2i(x, y));
    return 1;
}

static int vector2i_index(lua_State *L)
{
    Vector2i *v = static_cast<Vector2i *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "x") == 0)
    {
        lua_pushinteger(L, v->x);
        return 1;
    }
    else if (strcmp(key, "y") == 0)
    {
        lua_pushinteger(L, v->y);
        return 1;
    }

    return 0;
}

static int vector2i_newindex(lua_State *L)
{
    Vector2i *v = static_cast<Vector2i *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);
    int value = luaL_checkinteger(L, 3);

    if (strcmp(key, "x") == 0)
    {
        v->x = value;
    }
    else if (strcmp(key, "y") == 0)
    {
        v->y = value;
    }

    return 0;
}

static int vector2i_add(lua_State *L)
{
    Vector2i *a = static_cast<Vector2i *>(lua_touserdata(L, 1));
    Vector2i *b = static_cast<Vector2i *>(lua_touserdata(L, 2));
    push_vector2i(L, *a + *b);
    return 1;
}

static int vector2i_sub(lua_State *L)
{
    Vector2i *a = static_cast<Vector2i *>(lua_touserdata(L, 1));
    Vector2i *b = static_cast<Vector2i *>(lua_touserdata(L, 2));
    push_vector2i(L, *a - *b);
    return 1;
}

static int vector2i_mul(lua_State *L)
{
    Vector2i *a = static_cast<Vector2i *>(lua_touserdata(L, 1));

    if (lua_isnumber(L, 2))
    {
        int scalar = lua_tointeger(L, 2);
        push_vector2i(L, *a * scalar);
    }
    else
    {
        Vector2i *b = static_cast<Vector2i *>(lua_touserdata(L, 2));
        push_vector2i(L, *a * *b);
    }

    return 1;
}

static int vector2i_div(lua_State *L)
{
    Vector2i *a = static_cast<Vector2i *>(lua_touserdata(L, 1));

    if (lua_isnumber(L, 2))
    {
        int scalar = lua_tointeger(L, 2);
        push_vector2i(L, *a / scalar);
    }
    else
    {
        Vector2i *b = static_cast<Vector2i *>(lua_touserdata(L, 2));
        push_vector2i(L, *a / *b);
    }

    return 1;
}

static int vector2i_unm(lua_State *L)
{
    Vector2i *v = static_cast<Vector2i *>(lua_touserdata(L, 1));
    push_vector2i(L, -*v);
    return 1;
}

static int vector2i_eq(lua_State *L)
{
    Vector2i *a = static_cast<Vector2i *>(lua_touserdata(L, 1));
    Vector2i *b = static_cast<Vector2i *>(lua_touserdata(L, 2));
    lua_pushboolean(L, *a == *b);
    return 1;
}

static int vector2i_tostring(lua_State *L)
{
    Vector2i *v = static_cast<Vector2i *>(lua_touserdata(L, 1));
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Vector2i(%d, %d)", v->x, v->y);
    lua_pushstring(L, buffer);
    return 1;
}

static void register_vector2i_metatable(lua_State *L)
{
    luaL_newmetatable(L, "Vector2i");

    lua_pushcfunction(L, vector2i_index, "__index");
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, vector2i_newindex, "__newindex");
    lua_setfield(L, -2, "__newindex");

    lua_pushcfunction(L, vector2i_add, "__add");
    lua_setfield(L, -2, "__add");

    lua_pushcfunction(L, vector2i_sub, "__sub");
    lua_setfield(L, -2, "__sub");

    lua_pushcfunction(L, vector2i_mul, "__mul");
    lua_setfield(L, -2, "__mul");

    lua_pushcfunction(L, vector2i_div, "__div");
    lua_setfield(L, -2, "__div");

    lua_pushcfunction(L, vector2i_unm, "__unm");
    lua_setfield(L, -2, "__unm");

    lua_pushcfunction(L, vector2i_eq, "__eq");
    lua_setfield(L, -2, "__eq");

    lua_pushcfunction(L, vector2i_tostring, "__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_setuserdatametatable(L, LUA_TAG_VECTOR2I);

    // Register constructor
    lua_pushcfunction(L, vector2i_constructor, "Vector2i");
    lua_setglobal(L, "Vector2i");
}

// =============================================================================
// Vector3
// =============================================================================

static int vector3_constructor(lua_State *L)
{
    double x = luaL_optnumber(L, 1, 0.0);
    double y = luaL_optnumber(L, 2, 0.0);
    double z = luaL_optnumber(L, 3, 0.0);
    lua_pushvector(L, static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
    return 1;
}

// Vector3 uses Luau's native vector type - no metatable needed
// Operators (+, -, *, /, unary -, ==) are handled natively by the VM
// Property access (x, y, z) is handled by Luau's built-in vector.__index

// =============================================================================
// Vector3i
// =============================================================================

static int vector3i_constructor(lua_State *L)
{
    int x = luaL_optinteger(L, 1, 0);
    int y = luaL_optinteger(L, 2, 0);
    int z = luaL_optinteger(L, 3, 0);
    push_vector3i(L, Vector3i(x, y, z));
    return 1;
}

static int vector3i_index(lua_State *L)
{
    Vector3i *v = static_cast<Vector3i *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "x") == 0)
    {
        lua_pushinteger(L, v->x);
        return 1;
    }
    else if (strcmp(key, "y") == 0)
    {
        lua_pushinteger(L, v->y);
        return 1;
    }
    else if (strcmp(key, "z") == 0)
    {
        lua_pushinteger(L, v->z);
        return 1;
    }

    return 0;
}

static int vector3i_newindex(lua_State *L)
{
    Vector3i *v = static_cast<Vector3i *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);
    int value = luaL_checkinteger(L, 3);

    if (strcmp(key, "x") == 0)
    {
        v->x = value;
    }
    else if (strcmp(key, "y") == 0)
    {
        v->y = value;
    }
    else if (strcmp(key, "z") == 0)
    {
        v->z = value;
    }

    return 0;
}

static int vector3i_add(lua_State *L)
{
    Vector3i *a = static_cast<Vector3i *>(lua_touserdata(L, 1));
    Vector3i *b = static_cast<Vector3i *>(lua_touserdata(L, 2));
    push_vector3i(L, *a + *b);
    return 1;
}

static int vector3i_sub(lua_State *L)
{
    Vector3i *a = static_cast<Vector3i *>(lua_touserdata(L, 1));
    Vector3i *b = static_cast<Vector3i *>(lua_touserdata(L, 2));
    push_vector3i(L, *a - *b);
    return 1;
}

static int vector3i_mul(lua_State *L)
{
    Vector3i *a = static_cast<Vector3i *>(lua_touserdata(L, 1));

    if (lua_isnumber(L, 2))
    {
        int scalar = lua_tointeger(L, 2);
        push_vector3i(L, *a * scalar);
    }
    else
    {
        Vector3i *b = static_cast<Vector3i *>(lua_touserdata(L, 2));
        push_vector3i(L, *a * *b);
    }

    return 1;
}

static int vector3i_div(lua_State *L)
{
    Vector3i *a = static_cast<Vector3i *>(lua_touserdata(L, 1));

    if (lua_isnumber(L, 2))
    {
        int scalar = lua_tointeger(L, 2);
        push_vector3i(L, *a / scalar);
    }
    else
    {
        Vector3i *b = static_cast<Vector3i *>(lua_touserdata(L, 2));
        push_vector3i(L, *a / *b);
    }

    return 1;
}

static int vector3i_unm(lua_State *L)
{
    Vector3i *v = static_cast<Vector3i *>(lua_touserdata(L, 1));
    push_vector3i(L, -*v);
    return 1;
}

static int vector3i_eq(lua_State *L)
{
    Vector3i *a = static_cast<Vector3i *>(lua_touserdata(L, 1));
    Vector3i *b = static_cast<Vector3i *>(lua_touserdata(L, 2));
    lua_pushboolean(L, *a == *b);
    return 1;
}

static int vector3i_tostring(lua_State *L)
{
    Vector3i *v = static_cast<Vector3i *>(lua_touserdata(L, 1));
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Vector3i(%d, %d, %d)", v->x, v->y, v->z);
    lua_pushstring(L, buffer);
    return 1;
}

static void register_vector3i_metatable(lua_State *L)
{
    luaL_newmetatable(L, "Vector3i");

    lua_pushcfunction(L, vector3i_index, "__index");
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, vector3i_newindex, "__newindex");
    lua_setfield(L, -2, "__newindex");

    lua_pushcfunction(L, vector3i_add, "__add");
    lua_setfield(L, -2, "__add");

    lua_pushcfunction(L, vector3i_sub, "__sub");
    lua_setfield(L, -2, "__sub");

    lua_pushcfunction(L, vector3i_mul, "__mul");
    lua_setfield(L, -2, "__mul");

    lua_pushcfunction(L, vector3i_div, "__div");
    lua_setfield(L, -2, "__div");

    lua_pushcfunction(L, vector3i_unm, "__unm");
    lua_setfield(L, -2, "__unm");

    lua_pushcfunction(L, vector3i_eq, "__eq");
    lua_setfield(L, -2, "__eq");

    lua_pushcfunction(L, vector3i_tostring, "__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_setuserdatametatable(L, LUA_TAG_VECTOR3I);

    // Register constructor
    lua_pushcfunction(L, vector3i_constructor, "Vector3i");
    lua_setglobal(L, "Vector3i");
}

// =============================================================================
// Vector4
// =============================================================================

static int vector4_constructor(lua_State *L)
{
    double x = luaL_optnumber(L, 1, 0.0);
    double y = luaL_optnumber(L, 2, 0.0);
    double z = luaL_optnumber(L, 3, 0.0);
    double w = luaL_optnumber(L, 4, 0.0);
    push_vector4(L, Vector4(x, y, z, w));
    return 1;
}

static int vector4_index(lua_State *L)
{
    Vector4 *v = static_cast<Vector4 *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "x") == 0)
    {
        lua_pushnumber(L, v->x);
        return 1;
    }
    else if (strcmp(key, "y") == 0)
    {
        lua_pushnumber(L, v->y);
        return 1;
    }
    else if (strcmp(key, "z") == 0)
    {
        lua_pushnumber(L, v->z);
        return 1;
    }
    else if (strcmp(key, "w") == 0)
    {
        lua_pushnumber(L, v->w);
        return 1;
    }

    return 0;
}

static int vector4_newindex(lua_State *L)
{
    Vector4 *v = static_cast<Vector4 *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);
    double value = luaL_checknumber(L, 3);

    if (strcmp(key, "x") == 0)
    {
        v->x = value;
    }
    else if (strcmp(key, "y") == 0)
    {
        v->y = value;
    }
    else if (strcmp(key, "z") == 0)
    {
        v->z = value;
    }
    else if (strcmp(key, "w") == 0)
    {
        v->w = value;
    }

    return 0;
}

static int vector4_add(lua_State *L)
{
    Vector4 *a = static_cast<Vector4 *>(lua_touserdata(L, 1));
    Vector4 *b = static_cast<Vector4 *>(lua_touserdata(L, 2));
    push_vector4(L, *a + *b);
    return 1;
}

static int vector4_sub(lua_State *L)
{
    Vector4 *a = static_cast<Vector4 *>(lua_touserdata(L, 1));
    Vector4 *b = static_cast<Vector4 *>(lua_touserdata(L, 2));
    push_vector4(L, *a - *b);
    return 1;
}

static int vector4_mul(lua_State *L)
{
    Vector4 *a = static_cast<Vector4 *>(lua_touserdata(L, 1));

    if (lua_isnumber(L, 2))
    {
        double scalar = lua_tonumber(L, 2);
        push_vector4(L, *a * scalar);
    }
    else
    {
        Vector4 *b = static_cast<Vector4 *>(lua_touserdata(L, 2));
        push_vector4(L, *a * *b);
    }

    return 1;
}

static int vector4_div(lua_State *L)
{
    Vector4 *a = static_cast<Vector4 *>(lua_touserdata(L, 1));

    if (lua_isnumber(L, 2))
    {
        double scalar = lua_tonumber(L, 2);
        push_vector4(L, *a / scalar);
    }
    else
    {
        Vector4 *b = static_cast<Vector4 *>(lua_touserdata(L, 2));
        push_vector4(L, *a / *b);
    }

    return 1;
}

static int vector4_unm(lua_State *L)
{
    Vector4 *v = static_cast<Vector4 *>(lua_touserdata(L, 1));
    push_vector4(L, -*v);
    return 1;
}

static int vector4_eq(lua_State *L)
{
    Vector4 *a = static_cast<Vector4 *>(lua_touserdata(L, 1));
    Vector4 *b = static_cast<Vector4 *>(lua_touserdata(L, 2));
    lua_pushboolean(L, *a == *b);
    return 1;
}

static int vector4_tostring(lua_State *L)
{
    Vector4 *v = static_cast<Vector4 *>(lua_touserdata(L, 1));
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Vector4(%g, %g, %g, %g)", v->x, v->y, v->z, v->w);
    lua_pushstring(L, buffer);
    return 1;
}

static void register_vector4_metatable(lua_State *L)
{
    luaL_newmetatable(L, "Vector4");

    lua_pushcfunction(L, vector4_index, "__index");
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, vector4_newindex, "__newindex");
    lua_setfield(L, -2, "__newindex");

    lua_pushcfunction(L, vector4_add, "__add");
    lua_setfield(L, -2, "__add");

    lua_pushcfunction(L, vector4_sub, "__sub");
    lua_setfield(L, -2, "__sub");

    lua_pushcfunction(L, vector4_mul, "__mul");
    lua_setfield(L, -2, "__mul");

    lua_pushcfunction(L, vector4_div, "__div");
    lua_setfield(L, -2, "__div");

    lua_pushcfunction(L, vector4_unm, "__unm");
    lua_setfield(L, -2, "__unm");

    lua_pushcfunction(L, vector4_eq, "__eq");
    lua_setfield(L, -2, "__eq");

    lua_pushcfunction(L, vector4_tostring, "__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_setuserdatametatable(L, LUA_TAG_VECTOR4);

    // Register constructor
    lua_pushcfunction(L, vector4_constructor, "Vector4");
    lua_setglobal(L, "Vector4");
}

// =============================================================================
// Vector4i
// =============================================================================

static int vector4i_constructor(lua_State *L)
{
    int x = luaL_optinteger(L, 1, 0);
    int y = luaL_optinteger(L, 2, 0);
    int z = luaL_optinteger(L, 3, 0);
    int w = luaL_optinteger(L, 4, 0);
    push_vector4i(L, Vector4i(x, y, z, w));
    return 1;
}

static int vector4i_index(lua_State *L)
{
    Vector4i *v = static_cast<Vector4i *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "x") == 0)
    {
        lua_pushinteger(L, v->x);
        return 1;
    }
    else if (strcmp(key, "y") == 0)
    {
        lua_pushinteger(L, v->y);
        return 1;
    }
    else if (strcmp(key, "z") == 0)
    {
        lua_pushinteger(L, v->z);
        return 1;
    }
    else if (strcmp(key, "w") == 0)
    {
        lua_pushinteger(L, v->w);
        return 1;
    }

    return 0;
}

static int vector4i_newindex(lua_State *L)
{
    Vector4i *v = static_cast<Vector4i *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);
    int value = luaL_checkinteger(L, 3);

    if (strcmp(key, "x") == 0)
    {
        v->x = value;
    }
    else if (strcmp(key, "y") == 0)
    {
        v->y = value;
    }
    else if (strcmp(key, "z") == 0)
    {
        v->z = value;
    }
    else if (strcmp(key, "w") == 0)
    {
        v->w = value;
    }

    return 0;
}

static int vector4i_add(lua_State *L)
{
    Vector4i *a = static_cast<Vector4i *>(lua_touserdata(L, 1));
    Vector4i *b = static_cast<Vector4i *>(lua_touserdata(L, 2));
    push_vector4i(L, *a + *b);
    return 1;
}

static int vector4i_sub(lua_State *L)
{
    Vector4i *a = static_cast<Vector4i *>(lua_touserdata(L, 1));
    Vector4i *b = static_cast<Vector4i *>(lua_touserdata(L, 2));
    push_vector4i(L, *a - *b);
    return 1;
}

static int vector4i_mul(lua_State *L)
{
    Vector4i *a = static_cast<Vector4i *>(lua_touserdata(L, 1));

    if (lua_isnumber(L, 2))
    {
        int scalar = lua_tointeger(L, 2);
        push_vector4i(L, *a * scalar);
    }
    else
    {
        Vector4i *b = static_cast<Vector4i *>(lua_touserdata(L, 2));
        push_vector4i(L, *a * *b);
    }

    return 1;
}

static int vector4i_div(lua_State *L)
{
    Vector4i *a = static_cast<Vector4i *>(lua_touserdata(L, 1));

    if (lua_isnumber(L, 2))
    {
        int scalar = lua_tointeger(L, 2);
        push_vector4i(L, *a / scalar);
    }
    else
    {
        Vector4i *b = static_cast<Vector4i *>(lua_touserdata(L, 2));
        push_vector4i(L, *a / *b);
    }

    return 1;
}

static int vector4i_unm(lua_State *L)
{
    Vector4i *v = static_cast<Vector4i *>(lua_touserdata(L, 1));
    push_vector4i(L, -*v);
    return 1;
}

static int vector4i_eq(lua_State *L)
{
    Vector4i *a = static_cast<Vector4i *>(lua_touserdata(L, 1));
    Vector4i *b = static_cast<Vector4i *>(lua_touserdata(L, 2));
    lua_pushboolean(L, *a == *b);
    return 1;
}

static int vector4i_tostring(lua_State *L)
{
    Vector4i *v = static_cast<Vector4i *>(lua_touserdata(L, 1));
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Vector4i(%d, %d, %d, %d)", v->x, v->y, v->z, v->w);
    lua_pushstring(L, buffer);
    return 1;
}

static void register_vector4i_metatable(lua_State *L)
{
    luaL_newmetatable(L, "Vector4i");

    lua_pushcfunction(L, vector4i_index, "__index");
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, vector4i_newindex, "__newindex");
    lua_setfield(L, -2, "__newindex");

    lua_pushcfunction(L, vector4i_add, "__add");
    lua_setfield(L, -2, "__add");

    lua_pushcfunction(L, vector4i_sub, "__sub");
    lua_setfield(L, -2, "__sub");

    lua_pushcfunction(L, vector4i_mul, "__mul");
    lua_setfield(L, -2, "__mul");

    lua_pushcfunction(L, vector4i_div, "__div");
    lua_setfield(L, -2, "__div");

    lua_pushcfunction(L, vector4i_unm, "__unm");
    lua_setfield(L, -2, "__unm");

    lua_pushcfunction(L, vector4i_eq, "__eq");
    lua_setfield(L, -2, "__eq");

    lua_pushcfunction(L, vector4i_tostring, "__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_setuserdatametatable(L, LUA_TAG_VECTOR4I);

    // Register constructor
    lua_pushcfunction(L, vector4i_constructor, "Vector4i");
    lua_setglobal(L, "Vector4i");
}

// =============================================================================
// Color
// =============================================================================

static int color_constructor(lua_State *L)
{
    double r = luaL_optnumber(L, 1, 0.0);
    double g = luaL_optnumber(L, 2, 0.0);
    double b = luaL_optnumber(L, 3, 0.0);
    double a = luaL_optnumber(L, 4, 1.0);
    push_color(L, Color(r, g, b, a));
    return 1;
}

static int color_index(lua_State *L)
{
    Color *c = static_cast<Color *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "r") == 0)
    {
        lua_pushnumber(L, c->r);
        return 1;
    }
    else if (strcmp(key, "g") == 0)
    {
        lua_pushnumber(L, c->g);
        return 1;
    }
    else if (strcmp(key, "b") == 0)
    {
        lua_pushnumber(L, c->b);
        return 1;
    }
    else if (strcmp(key, "a") == 0)
    {
        lua_pushnumber(L, c->a);
        return 1;
    }

    return 0;
}

static int color_newindex(lua_State *L)
{
    Color *c = static_cast<Color *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);
    double value = luaL_checknumber(L, 3);

    if (strcmp(key, "r") == 0)
    {
        c->r = value;
    }
    else if (strcmp(key, "g") == 0)
    {
        c->g = value;
    }
    else if (strcmp(key, "b") == 0)
    {
        c->b = value;
    }
    else if (strcmp(key, "a") == 0)
    {
        c->a = value;
    }

    return 0;
}

static int color_add(lua_State *L)
{
    Color *a = static_cast<Color *>(lua_touserdata(L, 1));
    Color *b = static_cast<Color *>(lua_touserdata(L, 2));
    push_color(L, *a + *b);
    return 1;
}

static int color_sub(lua_State *L)
{
    Color *a = static_cast<Color *>(lua_touserdata(L, 1));
    Color *b = static_cast<Color *>(lua_touserdata(L, 2));
    push_color(L, *a - *b);
    return 1;
}

static int color_mul(lua_State *L)
{
    Color *a = static_cast<Color *>(lua_touserdata(L, 1));

    if (lua_isnumber(L, 2))
    {
        double scalar = lua_tonumber(L, 2);
        push_color(L, *a * scalar);
    }
    else
    {
        Color *b = static_cast<Color *>(lua_touserdata(L, 2));
        push_color(L, *a * *b);
    }

    return 1;
}

static int color_div(lua_State *L)
{
    Color *a = static_cast<Color *>(lua_touserdata(L, 1));

    if (lua_isnumber(L, 2))
    {
        double scalar = lua_tonumber(L, 2);
        push_color(L, *a / scalar);
    }
    else
    {
        Color *b = static_cast<Color *>(lua_touserdata(L, 2));
        push_color(L, *a / *b);
    }

    return 1;
}

static int color_unm(lua_State *L)
{
    Color *c = static_cast<Color *>(lua_touserdata(L, 1));
    push_color(L, -*c);
    return 1;
}

static int color_eq(lua_State *L)
{
    Color *a = static_cast<Color *>(lua_touserdata(L, 1));
    Color *b = static_cast<Color *>(lua_touserdata(L, 2));
    lua_pushboolean(L, *a == *b);
    return 1;
}

static int color_tostring(lua_State *L)
{
    Color *c = static_cast<Color *>(lua_touserdata(L, 1));
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Color(%g, %g, %g, %g)", c->r, c->g, c->b, c->a);
    lua_pushstring(L, buffer);
    return 1;
}

static void register_color_metatable(lua_State *L)
{
    luaL_newmetatable(L, "Color");

    lua_pushcfunction(L, color_index, "__index");
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, color_newindex, "__newindex");
    lua_setfield(L, -2, "__newindex");

    lua_pushcfunction(L, color_add, "__add");
    lua_setfield(L, -2, "__add");

    lua_pushcfunction(L, color_sub, "__sub");
    lua_setfield(L, -2, "__sub");

    lua_pushcfunction(L, color_mul, "__mul");
    lua_setfield(L, -2, "__mul");

    lua_pushcfunction(L, color_div, "__div");
    lua_setfield(L, -2, "__div");

    lua_pushcfunction(L, color_unm, "__unm");
    lua_setfield(L, -2, "__unm");

    lua_pushcfunction(L, color_eq, "__eq");
    lua_setfield(L, -2, "__eq");

    lua_pushcfunction(L, color_tostring, "__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_setuserdatametatable(L, LUA_TAG_COLOR);

    // Register constructor
    lua_pushcfunction(L, color_constructor, "Color");
    lua_setglobal(L, "Color");
}

// =============================================================================
// Rect2
// =============================================================================

static int rect2_constructor(lua_State *L)
{
    double x = luaL_optnumber(L, 1, 0.0);
    double y = luaL_optnumber(L, 2, 0.0);
    double width = luaL_optnumber(L, 3, 0.0);
    double height = luaL_optnumber(L, 4, 0.0);
    push_rect2(L, Rect2(x, y, width, height));
    return 1;
}

static int rect2_index(lua_State *L)
{
    Rect2 *r = static_cast<Rect2 *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "x") == 0 || strcmp(key, "position_x") == 0)
    {
        lua_pushnumber(L, r->position.x);
        return 1;
    }
    else if (strcmp(key, "y") == 0 || strcmp(key, "position_y") == 0)
    {
        lua_pushnumber(L, r->position.y);
        return 1;
    }
    else if (strcmp(key, "width") == 0 || strcmp(key, "size_x") == 0)
    {
        lua_pushnumber(L, r->size.x);
        return 1;
    }
    else if (strcmp(key, "height") == 0 || strcmp(key, "size_y") == 0)
    {
        lua_pushnumber(L, r->size.y);
        return 1;
    }
    else if (strcmp(key, "position") == 0)
    {
        push_vector2(L, r->position);
        return 1;
    }
    else if (strcmp(key, "size") == 0)
    {
        push_vector2(L, r->size);
        return 1;
    }

    return 0;
}

static int rect2_newindex(lua_State *L)
{
    Rect2 *r = static_cast<Rect2 *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "x") == 0 || strcmp(key, "position_x") == 0)
    {
        r->position.x = luaL_checknumber(L, 3);
    }
    else if (strcmp(key, "y") == 0 || strcmp(key, "position_y") == 0)
    {
        r->position.y = luaL_checknumber(L, 3);
    }
    else if (strcmp(key, "width") == 0 || strcmp(key, "size_x") == 0)
    {
        r->size.x = luaL_checknumber(L, 3);
    }
    else if (strcmp(key, "height") == 0 || strcmp(key, "size_y") == 0)
    {
        r->size.y = luaL_checknumber(L, 3);
    }

    return 0;
}

static int rect2_eq(lua_State *L)
{
    Rect2 *a = static_cast<Rect2 *>(lua_touserdata(L, 1));
    Rect2 *b = static_cast<Rect2 *>(lua_touserdata(L, 2));
    lua_pushboolean(L, *a == *b);
    return 1;
}

static int rect2_tostring(lua_State *L)
{
    Rect2 *r = static_cast<Rect2 *>(lua_touserdata(L, 1));
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Rect2(%g, %g, %g, %g)", r->position.x, r->position.y, r->size.x, r->size.y);
    lua_pushstring(L, buffer);
    return 1;
}

static void register_rect2_metatable(lua_State *L)
{
    luaL_newmetatable(L, "Rect2");

    lua_pushcfunction(L, rect2_index, "__index");
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, rect2_newindex, "__newindex");
    lua_setfield(L, -2, "__newindex");

    lua_pushcfunction(L, rect2_eq, "__eq");
    lua_setfield(L, -2, "__eq");

    lua_pushcfunction(L, rect2_tostring, "__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_setuserdatametatable(L, LUA_TAG_RECT2);

    // Register constructor
    lua_pushcfunction(L, rect2_constructor, "Rect2");
    lua_setglobal(L, "Rect2");
}

// =============================================================================
// Rect2i
// =============================================================================

static int rect2i_constructor(lua_State *L)
{
    int x = luaL_optinteger(L, 1, 0);
    int y = luaL_optinteger(L, 2, 0);
    int width = luaL_optinteger(L, 3, 0);
    int height = luaL_optinteger(L, 4, 0);
    push_rect2i(L, Rect2i(x, y, width, height));
    return 1;
}

static int rect2i_index(lua_State *L)
{
    Rect2i *r = static_cast<Rect2i *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "x") == 0 || strcmp(key, "position_x") == 0)
    {
        lua_pushinteger(L, r->position.x);
        return 1;
    }
    else if (strcmp(key, "y") == 0 || strcmp(key, "position_y") == 0)
    {
        lua_pushinteger(L, r->position.y);
        return 1;
    }
    else if (strcmp(key, "width") == 0 || strcmp(key, "size_x") == 0)
    {
        lua_pushinteger(L, r->size.x);
        return 1;
    }
    else if (strcmp(key, "height") == 0 || strcmp(key, "size_y") == 0)
    {
        lua_pushinteger(L, r->size.y);
        return 1;
    }
    else if (strcmp(key, "position") == 0)
    {
        push_vector2i(L, r->position);
        return 1;
    }
    else if (strcmp(key, "size") == 0)
    {
        push_vector2i(L, r->size);
        return 1;
    }

    return 0;
}

static int rect2i_newindex(lua_State *L)
{
    Rect2i *r = static_cast<Rect2i *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "x") == 0 || strcmp(key, "position_x") == 0)
    {
        r->position.x = luaL_checkinteger(L, 3);
    }
    else if (strcmp(key, "y") == 0 || strcmp(key, "position_y") == 0)
    {
        r->position.y = luaL_checkinteger(L, 3);
    }
    else if (strcmp(key, "width") == 0 || strcmp(key, "size_x") == 0)
    {
        r->size.x = luaL_checkinteger(L, 3);
    }
    else if (strcmp(key, "height") == 0 || strcmp(key, "size_y") == 0)
    {
        r->size.y = luaL_checkinteger(L, 3);
    }

    return 0;
}

static int rect2i_eq(lua_State *L)
{
    Rect2i *a = static_cast<Rect2i *>(lua_touserdata(L, 1));
    Rect2i *b = static_cast<Rect2i *>(lua_touserdata(L, 2));
    lua_pushboolean(L, *a == *b);
    return 1;
}

static int rect2i_tostring(lua_State *L)
{
    Rect2i *r = static_cast<Rect2i *>(lua_touserdata(L, 1));
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Rect2i(%d, %d, %d, %d)", r->position.x, r->position.y, r->size.x, r->size.y);
    lua_pushstring(L, buffer);
    return 1;
}

static void register_rect2i_metatable(lua_State *L)
{
    luaL_newmetatable(L, "Rect2i");

    lua_pushcfunction(L, rect2i_index, "__index");
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, rect2i_newindex, "__newindex");
    lua_setfield(L, -2, "__newindex");

    lua_pushcfunction(L, rect2i_eq, "__eq");
    lua_setfield(L, -2, "__eq");

    lua_pushcfunction(L, rect2i_tostring, "__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_setuserdatametatable(L, LUA_TAG_RECT2I);

    // Register constructor
    lua_pushcfunction(L, rect2i_constructor, "Rect2i");
    lua_setglobal(L, "Rect2i");
}

// =============================================================================
// AABB
// =============================================================================

static int aabb_constructor(lua_State *L)
{
    double px = luaL_optnumber(L, 1, 0.0);
    double py = luaL_optnumber(L, 2, 0.0);
    double pz = luaL_optnumber(L, 3, 0.0);
    double sx = luaL_optnumber(L, 4, 0.0);
    double sy = luaL_optnumber(L, 5, 0.0);
    double sz = luaL_optnumber(L, 6, 0.0);
    push_aabb(L, AABB(Vector3(px, py, pz), Vector3(sx, sy, sz)));
    return 1;
}

static int aabb_index(lua_State *L)
{
    AABB *aabb = static_cast<AABB *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "position") == 0)
    {
        push_vector3(L, aabb->position);
        return 1;
    }
    else if (strcmp(key, "size") == 0)
    {
        push_vector3(L, aabb->size);
        return 1;
    }

    return 0;
}

static int aabb_newindex(lua_State *L)
{
    AABB *aabb = static_cast<AABB *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "position") == 0)
    {
        aabb->position = to_vector3(L, 3);
    }
    else if (strcmp(key, "size") == 0)
    {
        aabb->size = to_vector3(L, 3);
    }

    return 0;
}

static int aabb_eq(lua_State *L)
{
    AABB *a = static_cast<AABB *>(lua_touserdata(L, 1));
    AABB *b = static_cast<AABB *>(lua_touserdata(L, 2));
    lua_pushboolean(L, *a == *b);
    return 1;
}

static int aabb_tostring(lua_State *L)
{
    AABB *aabb = static_cast<AABB *>(lua_touserdata(L, 1));
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "AABB((%g, %g, %g), (%g, %g, %g))",
             aabb->position.x, aabb->position.y, aabb->position.z,
             aabb->size.x, aabb->size.y, aabb->size.z);
    lua_pushstring(L, buffer);
    return 1;
}

static void register_aabb_metatable(lua_State *L)
{
    luaL_newmetatable(L, "AABB");

    lua_pushcfunction(L, aabb_index, "__index");
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, aabb_newindex, "__newindex");
    lua_setfield(L, -2, "__newindex");

    lua_pushcfunction(L, aabb_eq, "__eq");
    lua_setfield(L, -2, "__eq");

    lua_pushcfunction(L, aabb_tostring, "__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_setuserdatametatable(L, LUA_TAG_AABB);

    // Register constructor
    lua_pushcfunction(L, aabb_constructor, "AABB");
    lua_setglobal(L, "AABB");
}

// =============================================================================
// Plane
// =============================================================================

static int plane_constructor(lua_State *L)
{
    double a = luaL_optnumber(L, 1, 0.0);
    double b = luaL_optnumber(L, 2, 0.0);
    double c = luaL_optnumber(L, 3, 0.0);
    double d = luaL_optnumber(L, 4, 0.0);
    push_plane(L, Plane(a, b, c, d));
    return 1;
}

static int plane_index(lua_State *L)
{
    Plane *p = static_cast<Plane *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "normal") == 0)
    {
        push_vector3(L, p->normal);
        return 1;
    }
    else if (strcmp(key, "d") == 0)
    {
        lua_pushnumber(L, p->d);
        return 1;
    }

    return 0;
}

static int plane_newindex(lua_State *L)
{
    Plane *p = static_cast<Plane *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "normal") == 0)
    {
        p->normal = to_vector3(L, 3);
    }
    else if (strcmp(key, "d") == 0)
    {
        p->d = luaL_checknumber(L, 3);
    }

    return 0;
}

static int plane_unm(lua_State *L)
{
    Plane *p = static_cast<Plane *>(lua_touserdata(L, 1));
    push_plane(L, -*p);
    return 1;
}

static int plane_eq(lua_State *L)
{
    Plane *a = static_cast<Plane *>(lua_touserdata(L, 1));
    Plane *b = static_cast<Plane *>(lua_touserdata(L, 2));
    lua_pushboolean(L, *a == *b);
    return 1;
}

static int plane_tostring(lua_State *L)
{
    Plane *p = static_cast<Plane *>(lua_touserdata(L, 1));
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Plane(%g, %g, %g, %g)", p->normal.x, p->normal.y, p->normal.z, p->d);
    lua_pushstring(L, buffer);
    return 1;
}

static void register_plane_metatable(lua_State *L)
{
    luaL_newmetatable(L, "Plane");

    lua_pushcfunction(L, plane_index, "__index");
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, plane_newindex, "__newindex");
    lua_setfield(L, -2, "__newindex");

    lua_pushcfunction(L, plane_unm, "__unm");
    lua_setfield(L, -2, "__unm");

    lua_pushcfunction(L, plane_eq, "__eq");
    lua_setfield(L, -2, "__eq");

    lua_pushcfunction(L, plane_tostring, "__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_setuserdatametatable(L, LUA_TAG_PLANE);

    // Register constructor
    lua_pushcfunction(L, plane_constructor, "Plane");
    lua_setglobal(L, "Plane");
}

// =============================================================================
// Quaternion
// =============================================================================

static int quaternion_constructor(lua_State *L)
{
    double x = luaL_optnumber(L, 1, 0.0);
    double y = luaL_optnumber(L, 2, 0.0);
    double z = luaL_optnumber(L, 3, 0.0);
    double w = luaL_optnumber(L, 4, 1.0);
    push_quaternion(L, Quaternion(x, y, z, w));
    return 1;
}

static int quaternion_index(lua_State *L)
{
    Quaternion *q = static_cast<Quaternion *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "x") == 0)
    {
        lua_pushnumber(L, q->x);
        return 1;
    }
    else if (strcmp(key, "y") == 0)
    {
        lua_pushnumber(L, q->y);
        return 1;
    }
    else if (strcmp(key, "z") == 0)
    {
        lua_pushnumber(L, q->z);
        return 1;
    }
    else if (strcmp(key, "w") == 0)
    {
        lua_pushnumber(L, q->w);
        return 1;
    }

    return 0;
}

static int quaternion_newindex(lua_State *L)
{
    Quaternion *q = static_cast<Quaternion *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);
    double value = luaL_checknumber(L, 3);

    if (strcmp(key, "x") == 0)
    {
        q->x = value;
    }
    else if (strcmp(key, "y") == 0)
    {
        q->y = value;
    }
    else if (strcmp(key, "z") == 0)
    {
        q->z = value;
    }
    else if (strcmp(key, "w") == 0)
    {
        q->w = value;
    }

    return 0;
}

static int quaternion_add(lua_State *L)
{
    Quaternion *a = static_cast<Quaternion *>(lua_touserdata(L, 1));
    Quaternion *b = static_cast<Quaternion *>(lua_touserdata(L, 2));
    push_quaternion(L, *a + *b);
    return 1;
}

static int quaternion_sub(lua_State *L)
{
    Quaternion *a = static_cast<Quaternion *>(lua_touserdata(L, 1));
    Quaternion *b = static_cast<Quaternion *>(lua_touserdata(L, 2));
    push_quaternion(L, *a - *b);
    return 1;
}

static int quaternion_mul(lua_State *L)
{
    Quaternion *a = static_cast<Quaternion *>(lua_touserdata(L, 1));

    if (lua_isnumber(L, 2))
    {
        double scalar = lua_tonumber(L, 2);
        push_quaternion(L, *a * scalar);
    }
    else
    {
        Quaternion *b = static_cast<Quaternion *>(lua_touserdata(L, 2));
        push_quaternion(L, *a * *b);
    }

    return 1;
}

static int quaternion_div(lua_State *L)
{
    Quaternion *a = static_cast<Quaternion *>(lua_touserdata(L, 1));
    double scalar = luaL_checknumber(L, 2);
    push_quaternion(L, *a / scalar);
    return 1;
}

static int quaternion_unm(lua_State *L)
{
    Quaternion *q = static_cast<Quaternion *>(lua_touserdata(L, 1));
    push_quaternion(L, -*q);
    return 1;
}

static int quaternion_eq(lua_State *L)
{
    Quaternion *a = static_cast<Quaternion *>(lua_touserdata(L, 1));
    Quaternion *b = static_cast<Quaternion *>(lua_touserdata(L, 2));
    lua_pushboolean(L, *a == *b);
    return 1;
}

static int quaternion_tostring(lua_State *L)
{
    Quaternion *q = static_cast<Quaternion *>(lua_touserdata(L, 1));
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Quaternion(%g, %g, %g, %g)", q->x, q->y, q->z, q->w);
    lua_pushstring(L, buffer);
    return 1;
}

static void register_quaternion_metatable(lua_State *L)
{
    luaL_newmetatable(L, "Quaternion");

    lua_pushcfunction(L, quaternion_index, "__index");
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, quaternion_newindex, "__newindex");
    lua_setfield(L, -2, "__newindex");

    lua_pushcfunction(L, quaternion_add, "__add");
    lua_setfield(L, -2, "__add");

    lua_pushcfunction(L, quaternion_sub, "__sub");
    lua_setfield(L, -2, "__sub");

    lua_pushcfunction(L, quaternion_mul, "__mul");
    lua_setfield(L, -2, "__mul");

    lua_pushcfunction(L, quaternion_div, "__div");
    lua_setfield(L, -2, "__div");

    lua_pushcfunction(L, quaternion_unm, "__unm");
    lua_setfield(L, -2, "__unm");

    lua_pushcfunction(L, quaternion_eq, "__eq");
    lua_setfield(L, -2, "__eq");

    lua_pushcfunction(L, quaternion_tostring, "__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_setuserdatametatable(L, LUA_TAG_QUATERNION);

    // Register constructor
    lua_pushcfunction(L, quaternion_constructor, "Quaternion");
    lua_setglobal(L, "Quaternion");
}

// =============================================================================
// Basis
// =============================================================================

static int basis_constructor(lua_State *L)
{
    // Default constructor creates identity basis
    push_basis(L, Basis());
    return 1;
}

static int basis_index(lua_State *L)
{
    Basis *b = static_cast<Basis *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "x") == 0)
    {
        push_vector3(L, b->get_column(0));
        return 1;
    }
    else if (strcmp(key, "y") == 0)
    {
        push_vector3(L, b->get_column(1));
        return 1;
    }
    else if (strcmp(key, "z") == 0)
    {
        push_vector3(L, b->get_column(2));
        return 1;
    }

    return 0;
}

static int basis_mul(lua_State *L)
{
    Basis *a = static_cast<Basis *>(lua_touserdata(L, 1));

    if (lua_isnumber(L, 2))
    {
        double scalar = lua_tonumber(L, 2);
        push_basis(L, *a * scalar);
    }
    else
    {
        Basis *b = static_cast<Basis *>(lua_touserdata(L, 2));
        push_basis(L, *a * *b);
    }

    return 1;
}

static int basis_eq(lua_State *L)
{
    Basis *a = static_cast<Basis *>(lua_touserdata(L, 1));
    Basis *b = static_cast<Basis *>(lua_touserdata(L, 2));
    lua_pushboolean(L, *a == *b);
    return 1;
}

static int basis_tostring(lua_State *L)
{
    Basis *b = static_cast<Basis *>(lua_touserdata(L, 1));
    char buffer[512];
    Vector3 x = b->get_column(0);
    Vector3 y = b->get_column(1);
    Vector3 z = b->get_column(2);
    snprintf(buffer, sizeof(buffer), "Basis((%g, %g, %g), (%g, %g, %g), (%g, %g, %g))",
             x.x, x.y, x.z, y.x, y.y, y.z, z.x, z.y, z.z);
    lua_pushstring(L, buffer);
    return 1;
}

static void register_basis_metatable(lua_State *L)
{
    luaL_newmetatable(L, "Basis");

    lua_pushcfunction(L, basis_index, "__index");
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, basis_mul, "__mul");
    lua_setfield(L, -2, "__mul");

    lua_pushcfunction(L, basis_eq, "__eq");
    lua_setfield(L, -2, "__eq");

    lua_pushcfunction(L, basis_tostring, "__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_setuserdatametatable(L, LUA_TAG_BASIS);

    // Register constructor
    lua_pushcfunction(L, basis_constructor, "Basis");
    lua_setglobal(L, "Basis");
}

// =============================================================================
// Transform2D
// =============================================================================

static int transform2d_constructor(lua_State *L)
{
    // Default constructor creates identity transform
    push_transform2d(L, Transform2D());
    return 1;
}

static int transform2d_index(lua_State *L)
{
    Transform2D *t = static_cast<Transform2D *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "x") == 0)
    {
        push_vector2(L, (*t)[0]);
        return 1;
    }
    else if (strcmp(key, "y") == 0)
    {
        push_vector2(L, (*t)[1]);
        return 1;
    }
    else if (strcmp(key, "origin") == 0)
    {
        push_vector2(L, t->get_origin());
        return 1;
    }

    return 0;
}

static int transform2d_mul(lua_State *L)
{
    Transform2D *a = static_cast<Transform2D *>(lua_touserdata(L, 1));
    Transform2D *b = static_cast<Transform2D *>(lua_touserdata(L, 2));
    push_transform2d(L, *a * *b);
    return 1;
}

static int transform2d_eq(lua_State *L)
{
    Transform2D *a = static_cast<Transform2D *>(lua_touserdata(L, 1));
    Transform2D *b = static_cast<Transform2D *>(lua_touserdata(L, 2));
    lua_pushboolean(L, *a == *b);
    return 1;
}

static int transform2d_tostring(lua_State *L)
{
    Transform2D *t = static_cast<Transform2D *>(lua_touserdata(L, 1));
    char buffer[256];
    Vector2 x = (*t)[0];
    Vector2 y = (*t)[1];
    Vector2 o = t->get_origin();
    snprintf(buffer, sizeof(buffer), "Transform2D((%g, %g), (%g, %g), (%g, %g))",
             x.x, x.y, y.x, y.y, o.x, o.y);
    lua_pushstring(L, buffer);
    return 1;
}

static void register_transform2d_metatable(lua_State *L)
{
    luaL_newmetatable(L, "Transform2D");

    lua_pushcfunction(L, transform2d_index, "__index");
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, transform2d_mul, "__mul");
    lua_setfield(L, -2, "__mul");

    lua_pushcfunction(L, transform2d_eq, "__eq");
    lua_setfield(L, -2, "__eq");

    lua_pushcfunction(L, transform2d_tostring, "__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_setuserdatametatable(L, LUA_TAG_TRANSFORM2D);

    // Register constructor
    lua_pushcfunction(L, transform2d_constructor, "Transform2D");
    lua_setglobal(L, "Transform2D");
}

// =============================================================================
// Transform3D
// =============================================================================

static int transform3d_constructor(lua_State *L)
{
    // Default constructor creates identity transform
    push_transform3d(L, Transform3D());
    return 1;
}

static int transform3d_index(lua_State *L)
{
    Transform3D *t = static_cast<Transform3D *>(lua_touserdata(L, 1));
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "basis") == 0)
    {
        push_basis(L, t->basis);
        return 1;
    }
    else if (strcmp(key, "origin") == 0)
    {
        push_vector3(L, t->origin);
        return 1;
    }

    return 0;
}

static int transform3d_mul(lua_State *L)
{
    Transform3D *a = static_cast<Transform3D *>(lua_touserdata(L, 1));
    Transform3D *b = static_cast<Transform3D *>(lua_touserdata(L, 2));
    push_transform3d(L, *a * *b);
    return 1;
}

static int transform3d_eq(lua_State *L)
{
    Transform3D *a = static_cast<Transform3D *>(lua_touserdata(L, 1));
    Transform3D *b = static_cast<Transform3D *>(lua_touserdata(L, 2));
    lua_pushboolean(L, *a == *b);
    return 1;
}

static int transform3d_tostring(lua_State *L)
{
    Transform3D *t = static_cast<Transform3D *>(lua_touserdata(L, 1));
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "Transform3D(Basis(...), Origin(%g, %g, %g))",
             t->origin.x, t->origin.y, t->origin.z);
    lua_pushstring(L, buffer);
    return 1;
}

static void register_transform3d_metatable(lua_State *L)
{
    luaL_newmetatable(L, "Transform3D");

    lua_pushcfunction(L, transform3d_index, "__index");
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, transform3d_mul, "__mul");
    lua_setfield(L, -2, "__mul");

    lua_pushcfunction(L, transform3d_eq, "__eq");
    lua_setfield(L, -2, "__eq");

    lua_pushcfunction(L, transform3d_tostring, "__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_setuserdatametatable(L, LUA_TAG_TRANSFORM3D);

    // Register constructor
    lua_pushcfunction(L, transform3d_constructor, "Transform3D");
    lua_setglobal(L, "Transform3D");
}

// =============================================================================
// Projection
// =============================================================================

static int projection_constructor(lua_State *L)
{
    // Default constructor creates identity projection
    push_projection(L, Projection());
    return 1;
}

static int projection_mul(lua_State *L)
{
    Projection *a = static_cast<Projection *>(lua_touserdata(L, 1));
    Projection *b = static_cast<Projection *>(lua_touserdata(L, 2));
    push_projection(L, *a * *b);
    return 1;
}

static int projection_eq(lua_State *L)
{
    Projection *a = static_cast<Projection *>(lua_touserdata(L, 1));
    Projection *b = static_cast<Projection *>(lua_touserdata(L, 2));
    lua_pushboolean(L, *a == *b);
    return 1;
}

static int projection_tostring(lua_State *L)
{
    lua_pushstring(L, "Projection(...)");
    return 1;
}

static void register_projection_metatable(lua_State *L)
{
    luaL_newmetatable(L, "Projection");

    lua_pushcfunction(L, projection_mul, "__mul");
    lua_setfield(L, -2, "__mul");

    lua_pushcfunction(L, projection_eq, "__eq");
    lua_setfield(L, -2, "__eq");

    lua_pushcfunction(L, projection_tostring, "__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_setuserdatametatable(L, LUA_TAG_PROJECTION);

    // Register constructor
    lua_pushcfunction(L, projection_constructor, "Projection");
    lua_setglobal(L, "Projection");
}

// =============================================================================
// Callable
// =============================================================================

// Callable __call metamethod - allows calling from Lua
static int callable_call(lua_State *L)
{
    // Get the callable userdata (first argument is self)
    Callable *callable = static_cast<Callable *>(lua_touserdatatagged(L, 1, LUA_TAG_CALLABLE));
    if (!callable)
    {
        luaL_error(L, "Invalid Callable userdata");
        return 0;
    }

    // Check if callable is valid
    if (!callable->is_valid())
    {
        luaL_error(L, "Callable is not valid");
        return 0;
    }

    // Validate argument count if possible
    int arg_count = lua_gettop(L) - 1; // Subtract 1 for self
    int expected_args = callable->get_argument_count();

    if (expected_args >= 0 && arg_count != expected_args)
    {
        luaL_error(L, "Callable expects %d arguments, got %d", expected_args, arg_count);
        return 0;
    }

    // Convert Lua arguments to Godot Variant array
    Array args;
    args.resize(arg_count);

    // Need access to LuaState to convert arguments
    // We can't easily get the LuaState from here, so we'll use a workaround
    // by manually converting basic types
    for (int i = 0; i < arg_count; i++)
    {
        int idx = i + 2; // Skip self (1) and start from argument 1 (2)
        int type = lua_type(L, idx);

        switch (type)
        {
        case LUA_TNIL:
            args[i] = Variant();
            break;
        case LUA_TBOOLEAN:
            args[i] = Variant(lua_toboolean(L, idx) != 0);
            break;
        case LUA_TNUMBER:
            args[i] = Variant(lua_tonumber(L, idx));
            break;
        case LUA_TSTRING:
        {
            size_t len;
            const char *str = lua_tolstring(L, idx, &len);
            args[i] = Variant(String::utf8(str, len));
            break;
        }
        case LUA_TVECTOR:
        {
            const float *v = lua_tovector(L, idx);
            args[i] = Variant(Vector3(v[0], v[1], v[2]));
            break;
        }
        case LUA_TFUNCTION:
        {
            // Convert Lua function to LuaCallable
            // Get the LuaState pointer from the registry
            lua_getfield(L, LUA_REGISTRYINDEX, GDLUAU_STATE_REGISTRY_KEY);
            LuaState *state = static_cast<LuaState *>(lua_tolightuserdata(L, -1));
            lua_pop(L, 1);

            if (!state)
            {
                luaL_error(L, "Failed to get LuaState from registry");
                return 0;
            }

            // Validate the function before storing
            if (!lua_isfunction(L, idx))
            {
                luaL_error(L, "Argument %d is not a valid function", i + 1);
                return 0;
            }

            String funcname;
            lua_Debug ar;
            if (lua_getinfo(L, idx, "n", &ar))
            {
                funcname = ar.name;
            }
            else
            {
                funcname = "<unknown>";
            }

            // Store function in registry and create LuaCallable
            // In Luau, lua_ref takes a stack index and stores that value in the registry
            int func_ref = lua_ref(L, idx);

            if (func_ref == LUA_NOREF || func_ref == LUA_REFNIL)
            {
                luaL_error(L, "Failed to create reference for Lua function");
                return 0;
            }

            // Create LuaCallable wrapper
            Callable lua_callable = Callable(memnew(LuaCallable(state, funcname, func_ref)));
            args[i] = lua_callable;
            break;
        }
        case LUA_TUSERDATA:
        {
            // Check for math types
            int tag = lua_userdatatag(L, idx);
            switch (tag)
            {
            case LUA_TAG_VECTOR2:
                args[i] = Variant(to_vector2(L, idx));
                break;
            case LUA_TAG_VECTOR2I:
                args[i] = Variant(to_vector2i(L, idx));
                break;
            case LUA_TAG_VECTOR3I:
                args[i] = Variant(to_vector3i(L, idx));
                break;
            case LUA_TAG_VECTOR4:
                args[i] = Variant(to_vector4(L, idx));
                break;
            case LUA_TAG_VECTOR4I:
                args[i] = Variant(to_vector4i(L, idx));
                break;
            case LUA_TAG_COLOR:
                args[i] = Variant(to_color(L, idx));
                break;
            case LUA_TAG_RECT2:
                args[i] = Variant(to_rect2(L, idx));
                break;
            case LUA_TAG_RECT2I:
                args[i] = Variant(to_rect2i(L, idx));
                break;
            case LUA_TAG_AABB:
                args[i] = Variant(to_aabb(L, idx));
                break;
            case LUA_TAG_PLANE:
                args[i] = Variant(to_plane(L, idx));
                break;
            case LUA_TAG_QUATERNION:
                args[i] = Variant(to_quaternion(L, idx));
                break;
            case LUA_TAG_BASIS:
                args[i] = Variant(to_basis(L, idx));
                break;
            case LUA_TAG_TRANSFORM2D:
                args[i] = Variant(to_transform2d(L, idx));
                break;
            case LUA_TAG_TRANSFORM3D:
                args[i] = Variant(to_transform3d(L, idx));
                break;
            case LUA_TAG_PROJECTION:
                args[i] = Variant(to_projection(L, idx));
                break;
            case LUA_TAG_CALLABLE:
                args[i] = Variant(to_callable(L, idx));
                break;
            default:
                luaL_error(L, "Unsupported argument type: userdata with tag %d", tag);
                return 0;
            }
            break;
        }
        default:
            luaL_error(L, "Unsupported argument type: %s", lua_typename(L, type));
            return 0;
        }
    }

    // Call the Callable with array of arguments
    Variant result = callable->callv(args);

    // Convert result back to Lua using LuaState::pushvariant for full type support
    // Get the LuaState pointer from the registry
    lua_getfield(L, LUA_REGISTRYINDEX, GDLUAU_STATE_REGISTRY_KEY);
    LuaState *state = static_cast<LuaState *>(lua_tolightuserdata(L, -1));
    lua_pop(L, 1);

    if (!state)
    {
        luaL_error(L, "Failed to get LuaState from registry for result conversion");
        return 0;
    }

    // Use LuaState's pushvariant which handles all types correctly
    state->pushvariant(result);

    return 1;
}

// Callable __gc metamethod - destructor
static int callable_gc(lua_State *L)
{
    Callable *callable = static_cast<Callable *>(lua_touserdata(L, 1));
    if (callable)
    {
        callable->~Callable(); // Explicit destructor call
    }
    return 0;
}

// Callable __tostring metamethod
static int callable_tostring(lua_State *L)
{
    Callable *callable = static_cast<Callable *>(lua_touserdatatagged(L, 1, LUA_TAG_CALLABLE));
    if (callable && callable->is_valid())
    {
        String str = vformat("Callable(%s)", callable->get_method());
        CharString utf8 = str.utf8();
        lua_pushlstring(L, utf8.get_data(), utf8.length());
    }
    else
    {
        lua_pushstring(L, "Callable(invalid)");
    }
    return 1;
}

static void register_callable_metatable(lua_State *L)
{
    luaL_newmetatable(L, "Callable");

    lua_pushcfunction(L, callable_call, "__call");
    lua_setfield(L, -2, "__call");

    lua_pushcfunction(L, callable_gc, "__gc");
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, callable_tostring, "__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_setuserdatametatable(L, LUA_TAG_CALLABLE);
}

// =============================================================================
// Public API - Push functions
// =============================================================================

void godot::push_vector2(lua_State *L, const Vector2 &value)
{
    Vector2 *v = static_cast<Vector2 *>(lua_newuserdatataggedwithmetatable(L, sizeof(Vector2), LUA_TAG_VECTOR2));
    *v = value;
}

void godot::push_vector2i(lua_State *L, const Vector2i &value)
{
    Vector2i *v = static_cast<Vector2i *>(lua_newuserdatataggedwithmetatable(L, sizeof(Vector2i), LUA_TAG_VECTOR2I));
    *v = value;
}

void godot::push_vector3(lua_State *L, const Vector3 &value)
{
    lua_pushvector(L, static_cast<float>(value.x), static_cast<float>(value.y), static_cast<float>(value.z));
}

void godot::push_vector3i(lua_State *L, const Vector3i &value)
{
    Vector3i *v = static_cast<Vector3i *>(lua_newuserdatataggedwithmetatable(L, sizeof(Vector3i), LUA_TAG_VECTOR3I));
    *v = value;
}

void godot::push_color(lua_State *L, const Color &value)
{
    Color *c = static_cast<Color *>(lua_newuserdatataggedwithmetatable(L, sizeof(Color), LUA_TAG_COLOR));
    *c = value;
}

void godot::push_vector4(lua_State *L, const Vector4 &value)
{
    Vector4 *v = static_cast<Vector4 *>(lua_newuserdatataggedwithmetatable(L, sizeof(Vector4), LUA_TAG_VECTOR4));
    *v = value;
}

void godot::push_vector4i(lua_State *L, const Vector4i &value)
{
    Vector4i *v = static_cast<Vector4i *>(lua_newuserdatataggedwithmetatable(L, sizeof(Vector4i), LUA_TAG_VECTOR4I));
    *v = value;
}

void godot::push_rect2(lua_State *L, const Rect2 &value)
{
    Rect2 *r = static_cast<Rect2 *>(lua_newuserdatataggedwithmetatable(L, sizeof(Rect2), LUA_TAG_RECT2));
    *r = value;
}

void godot::push_rect2i(lua_State *L, const Rect2i &value)
{
    Rect2i *r = static_cast<Rect2i *>(lua_newuserdatataggedwithmetatable(L, sizeof(Rect2i), LUA_TAG_RECT2I));
    *r = value;
}

void godot::push_aabb(lua_State *L, const AABB &value)
{
    AABB *aabb = static_cast<AABB *>(lua_newuserdatataggedwithmetatable(L, sizeof(AABB), LUA_TAG_AABB));
    *aabb = value;
}

void godot::push_plane(lua_State *L, const Plane &value)
{
    Plane *p = static_cast<Plane *>(lua_newuserdatataggedwithmetatable(L, sizeof(Plane), LUA_TAG_PLANE));
    *p = value;
}

void godot::push_quaternion(lua_State *L, const Quaternion &value)
{
    Quaternion *q = static_cast<Quaternion *>(lua_newuserdatataggedwithmetatable(L, sizeof(Quaternion), LUA_TAG_QUATERNION));
    *q = value;
}

void godot::push_basis(lua_State *L, const Basis &value)
{
    Basis *b = static_cast<Basis *>(lua_newuserdatataggedwithmetatable(L, sizeof(Basis), LUA_TAG_BASIS));
    *b = value;
}

void godot::push_transform2d(lua_State *L, const Transform2D &value)
{
    Transform2D *t = static_cast<Transform2D *>(lua_newuserdatataggedwithmetatable(L, sizeof(Transform2D), LUA_TAG_TRANSFORM2D));
    *t = value;
}

void godot::push_transform3d(lua_State *L, const Transform3D &value)
{
    Transform3D *t = static_cast<Transform3D *>(lua_newuserdatataggedwithmetatable(L, sizeof(Transform3D), LUA_TAG_TRANSFORM3D));
    *t = value;
}

void godot::push_projection(lua_State *L, const Projection &value)
{
    Projection *p = static_cast<Projection *>(lua_newuserdatataggedwithmetatable(L, sizeof(Projection), LUA_TAG_PROJECTION));
    *p = value;
}

void godot::push_callable(lua_State *L, const Callable &value)
{
    // Check if this is a LuaCallable wrapping a Lua function
    // If so, push the original Lua function instead of wrapping it again
    CallableCustom *custom = value.get_custom();
    if (custom)
    {
        LuaCallable *lua_callable = dynamic_cast<LuaCallable *>(custom);
        if (lua_callable && lua_callable->get_lua_state() == L)
        {
            // This is a LuaCallable from the same Lua state
            // Push the original function from the registry
            lua_rawgeti(L, LUA_REGISTRYINDEX, lua_callable->get_func_ref());
            return;
        }
    }

    // Not a LuaCallable or from a different state - wrap as userdata
    // Use placement new since Callable has a destructor
    void *userdata = lua_newuserdatataggedwithmetatable(L, sizeof(Callable), LUA_TAG_CALLABLE);
    memnew_placement(userdata, Callable(value)); // Placement new
}

// =============================================================================
// Public API - Type checking functions
// =============================================================================

bool godot::is_vector2(lua_State *L, int index)
{
    return lua_userdatatag(L, index) == LUA_TAG_VECTOR2;
}

bool godot::is_vector2i(lua_State *L, int index)
{
    return lua_userdatatag(L, index) == LUA_TAG_VECTOR2I;
}

bool godot::is_vector3(lua_State *L, int index)
{
    return lua_isvector(L, index);
}

bool godot::is_vector3i(lua_State *L, int index)
{
    return lua_userdatatag(L, index) == LUA_TAG_VECTOR3I;
}

bool godot::is_color(lua_State *L, int index)
{
    return lua_userdatatag(L, index) == LUA_TAG_COLOR;
}

bool godot::is_vector4(lua_State *L, int index)
{
    return lua_userdatatag(L, index) == LUA_TAG_VECTOR4;
}

bool godot::is_vector4i(lua_State *L, int index)
{
    return lua_userdatatag(L, index) == LUA_TAG_VECTOR4I;
}

bool godot::is_rect2(lua_State *L, int index)
{
    return lua_userdatatag(L, index) == LUA_TAG_RECT2;
}

bool godot::is_rect2i(lua_State *L, int index)
{
    return lua_userdatatag(L, index) == LUA_TAG_RECT2I;
}

bool godot::is_aabb(lua_State *L, int index)
{
    return lua_userdatatag(L, index) == LUA_TAG_AABB;
}

bool godot::is_plane(lua_State *L, int index)
{
    return lua_userdatatag(L, index) == LUA_TAG_PLANE;
}

bool godot::is_quaternion(lua_State *L, int index)
{
    return lua_userdatatag(L, index) == LUA_TAG_QUATERNION;
}

bool godot::is_basis(lua_State *L, int index)
{
    return lua_userdatatag(L, index) == LUA_TAG_BASIS;
}

bool godot::is_transform2d(lua_State *L, int index)
{
    return lua_userdatatag(L, index) == LUA_TAG_TRANSFORM2D;
}

bool godot::is_transform3d(lua_State *L, int index)
{
    return lua_userdatatag(L, index) == LUA_TAG_TRANSFORM3D;
}

bool godot::is_projection(lua_State *L, int index)
{
    return lua_userdatatag(L, index) == LUA_TAG_PROJECTION;
}

bool godot::is_callable(lua_State *L, int index)
{
    return lua_userdatatag(L, index) == LUA_TAG_CALLABLE;
}

// =============================================================================
// Public API - Extract functions
// =============================================================================

Vector2 godot::to_vector2(lua_State *L, int index)
{
    Vector2 *v = static_cast<Vector2 *>(lua_touserdatatagged(L, index, LUA_TAG_VECTOR2));
    ERR_FAIL_NULL_V_MSG(v, Vector2(), "Invalid Vector2 userdata");
    return *v;
}

Vector2i godot::to_vector2i(lua_State *L, int index)
{
    Vector2i *v = static_cast<Vector2i *>(lua_touserdatatagged(L, index, LUA_TAG_VECTOR2I));
    ERR_FAIL_NULL_V_MSG(v, Vector2i(), "Invalid Vector2i userdata");
    return *v;
}

Vector3 godot::to_vector3(lua_State *L, int index)
{
    const float *v = lua_tovector(L, index);
    ERR_FAIL_NULL_V_MSG(v, Vector3(), "Invalid Vector3 (expected native vector)");
    return Vector3(static_cast<real_t>(v[0]), static_cast<real_t>(v[1]), static_cast<real_t>(v[2]));
}

Vector3i godot::to_vector3i(lua_State *L, int index)
{
    Vector3i *v = static_cast<Vector3i *>(lua_touserdatatagged(L, index, LUA_TAG_VECTOR3I));
    ERR_FAIL_NULL_V_MSG(v, Vector3i(), "Invalid Vector3i userdata");
    return *v;
}

Color godot::to_color(lua_State *L, int index)
{
    Color *c = static_cast<Color *>(lua_touserdatatagged(L, index, LUA_TAG_COLOR));
    ERR_FAIL_NULL_V_MSG(c, Color(), "Invalid Color userdata");
    return *c;
}

Vector4 godot::to_vector4(lua_State *L, int index)
{
    Vector4 *v = static_cast<Vector4 *>(lua_touserdatatagged(L, index, LUA_TAG_VECTOR4));
    ERR_FAIL_NULL_V_MSG(v, Vector4(), "Invalid Vector4 userdata");
    return *v;
}

Vector4i godot::to_vector4i(lua_State *L, int index)
{
    Vector4i *v = static_cast<Vector4i *>(lua_touserdatatagged(L, index, LUA_TAG_VECTOR4I));
    ERR_FAIL_NULL_V_MSG(v, Vector4i(), "Invalid Vector4i userdata");
    return *v;
}

Rect2 godot::to_rect2(lua_State *L, int index)
{
    Rect2 *r = static_cast<Rect2 *>(lua_touserdatatagged(L, index, LUA_TAG_RECT2));
    ERR_FAIL_NULL_V_MSG(r, Rect2(), "Invalid Rect2 userdata");
    return *r;
}

Rect2i godot::to_rect2i(lua_State *L, int index)
{
    Rect2i *r = static_cast<Rect2i *>(lua_touserdatatagged(L, index, LUA_TAG_RECT2I));
    ERR_FAIL_NULL_V_MSG(r, Rect2i(), "Invalid Rect2i userdata");
    return *r;
}

AABB godot::to_aabb(lua_State *L, int index)
{
    AABB *aabb = static_cast<AABB *>(lua_touserdatatagged(L, index, LUA_TAG_AABB));
    ERR_FAIL_NULL_V_MSG(aabb, AABB(), "Invalid AABB userdata");
    return *aabb;
}

Plane godot::to_plane(lua_State *L, int index)
{
    Plane *p = static_cast<Plane *>(lua_touserdatatagged(L, index, LUA_TAG_PLANE));
    ERR_FAIL_NULL_V_MSG(p, Plane(), "Invalid Plane userdata");
    return *p;
}

Quaternion godot::to_quaternion(lua_State *L, int index)
{
    Quaternion *q = static_cast<Quaternion *>(lua_touserdatatagged(L, index, LUA_TAG_QUATERNION));
    ERR_FAIL_NULL_V_MSG(q, Quaternion(), "Invalid Quaternion userdata");
    return *q;
}

Basis godot::to_basis(lua_State *L, int index)
{
    Basis *b = static_cast<Basis *>(lua_touserdatatagged(L, index, LUA_TAG_BASIS));
    ERR_FAIL_NULL_V_MSG(b, Basis(), "Invalid Basis userdata");
    return *b;
}

Transform2D godot::to_transform2d(lua_State *L, int index)
{
    Transform2D *t = static_cast<Transform2D *>(lua_touserdatatagged(L, index, LUA_TAG_TRANSFORM2D));
    ERR_FAIL_NULL_V_MSG(t, Transform2D(), "Invalid Transform2D userdata");
    return *t;
}

Transform3D godot::to_transform3d(lua_State *L, int index)
{
    Transform3D *t = static_cast<Transform3D *>(lua_touserdatatagged(L, index, LUA_TAG_TRANSFORM3D));
    ERR_FAIL_NULL_V_MSG(t, Transform3D(), "Invalid Transform3D userdata");
    return *t;
}

Projection godot::to_projection(lua_State *L, int index)
{
    Projection *p = static_cast<Projection *>(lua_touserdatatagged(L, index, LUA_TAG_PROJECTION));
    ERR_FAIL_NULL_V_MSG(p, Projection(), "Invalid Projection userdata");
    return *p;
}

Callable godot::to_callable(lua_State *L, int index)
{
    Callable *c = static_cast<Callable *>(lua_touserdatatagged(L, index, LUA_TAG_CALLABLE));
    ERR_FAIL_NULL_V_MSG(c, Callable(), "Invalid Callable userdata");
    return *c;
}

// =============================================================================
// Library Entry Point
// =============================================================================

int luaopen_godot(lua_State *L)
{
    // Register all math type metatables
    register_vector2_metatable(L);
    register_vector2i_metatable(L);

    // Vector3 uses native vector type - just register the constructor
    lua_pushcfunction(L, vector3_constructor, "Vector3");
    lua_setglobal(L, "Vector3");

    register_vector3i_metatable(L);
    register_vector4_metatable(L);
    register_vector4i_metatable(L);
    register_color_metatable(L);
    register_rect2_metatable(L);
    register_rect2i_metatable(L);
    register_aabb_metatable(L);
    register_plane_metatable(L);
    register_quaternion_metatable(L);
    register_basis_metatable(L);
    register_transform2d_metatable(L);
    register_transform3d_metatable(L);
    register_projection_metatable(L);
    register_callable_metatable(L);

    return 1;
}
