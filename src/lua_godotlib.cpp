#include "lua_godotlib.h"
#include "lua_state.h"
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

// Metatable name for marking tables that come from Godot Dictionaries
static const char *GODOT_DICTIONARY_MT = "__godot_dictionary_mt";

static Ref<LuaState> get_godot_lua_state(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, GDLUAU_STATE_REGISTRY_KEY);
    LuaState *state = static_cast<LuaState *>(lua_tolightuserdata(L, -1));
    lua_pop(L, 1);

    return Ref(state);
}

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

    // Convert Lua arguments to Godot Variant array using to_variant
    Array args;
    args.resize(arg_count);

    for (int i = 0; i < arg_count; i++)
    {
        int idx = i + 2; // Skip self (1) and start from argument 1 (2)
        args[i] = to_variant(L, idx);
    }

    // Call the Callable with array of arguments
    Variant result = callable->callv(args);

    // Convert result back to Lua using push_variant
    push_variant(L, result);

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
// Array/Dictionary/Variant conversion helpers
// =============================================================================

bool godot::is_array(lua_State *L, int index)
{
    if (!lua_istable(L, index))
        return false;

    // Normalize negative indices
    int abs_index = index < 0 ? lua_gettop(L) + index + 1 : index;

    // Check if table has the dictionary marker metatable
    if (lua_getmetatable(L, abs_index))
    {
        luaL_getmetatable(L, GODOT_DICTIONARY_MT);
        bool is_dict = lua_rawequal(L, -1, -2);
        lua_pop(L, 2); // Pop both metatables
        if (is_dict)
            return false; // Has dictionary marker
    }

    // Check if table has consecutive integer keys starting from 1
    int n = 0;
    lua_pushnil(L);
    while (lua_next(L, abs_index) != 0)
    {
        // Key is at -2, value is at -1
        if (!lua_isnumber(L, -2))
        {
            // Non-numeric key, not an array
            lua_pop(L, 2); // Pop key and value
            return false;
        }

        double key_num = lua_tonumber(L, -2);
        int key_int = static_cast<int>(key_num);

        // Check if key is an integer
        if (key_num != static_cast<double>(key_int))
        {
            // Key is not an integer, not an array
            lua_pop(L, 2); // Pop key and value
            return false;
        }

        // Arrays must start at 1 in Lua, so any key < 1 means it's a dictionary
        if (key_int < 1)
        {
            lua_pop(L, 2); // Pop key and value
            return false;
        }

        // Keep track of max key
        if (key_int > n)
            n = key_int;

        // Pop value, keep key for next iteration
        lua_pop(L, 1);
    }

    // Check if all keys from 1 to n are present
    for (int i = 1; i <= n; i++)
    {
        lua_rawgeti(L, abs_index, i);
        if (lua_isnil(L, -1))
        {
            // Gap in sequence, not an array
            lua_pop(L, 1);
            return false;
        }
        lua_pop(L, 1);
    }

    return true;
}

bool godot::is_dictionary(lua_State *L, int index)
{
    if (!lua_istable(L, index))
        return false;

    // Normalize negative indices
    int abs_index = index < 0 ? lua_gettop(L) + index + 1 : index;

    // Check if this table has the dictionary marker metatable
    if (lua_getmetatable(L, abs_index))
    {
        luaL_getmetatable(L, GODOT_DICTIONARY_MT);
        bool is_dict = lua_rawequal(L, -1, -2);
        lua_pop(L, 2); // Pop both metatables
        if (is_dict)
            return true; // Has dictionary metatable
    }

    return !is_array(L, index);
}

Array godot::to_array(lua_State *L, int index)
{
    ERR_FAIL_COND_V_MSG(!lua_istable(L, index), Array(), "Value at index is not a table.");

    Array arr;

    // Normalize negative indices
    int abs_index = index < 0 ? lua_gettop(L) + index + 1 : index;

    // Get array length
    int len = lua_objlen(L, abs_index);

    // Convert each element
    for (int i = 1; i <= len; i++)
    {
        lua_rawgeti(L, abs_index, i);
        arr.append(to_variant(L, -1));
        lua_pop(L, 1);
    }

    return arr;
}

Dictionary godot::to_dictionary(lua_State *L, int index)
{
    ERR_FAIL_COND_V_MSG(!lua_istable(L, index), Dictionary(), "Value at index is not a table.");

    Dictionary dict;

    // Normalize negative indices
    int abs_index = index < 0 ? lua_gettop(L) + index + 1 : index;

    // Push nil as the first key for lua_next
    lua_pushnil(L);

    // Iterate over the table
    while (lua_next(L, abs_index) != 0)
    {
        Variant key = to_variant(L, -2);
        Variant value = to_variant(L, -1);

        dict[key] = value;

        // Remove value, keep key for next iteration
        lua_pop(L, 1);
    }

    return dict;
}

String godot::to_string(lua_State *L, int index)
{
    size_t len;
    const char *str = lua_tolstring(L, index, &len);
    return String::utf8(str, len);
}

Variant godot::to_variant(lua_State *L, int index)
{
    int type_id = lua_type(L, index);

    switch (type_id)
    {
    case LUA_TNIL:
        return Variant();

    case LUA_TBOOLEAN:
        return Variant(lua_toboolean(L, index) != 0);

    case LUA_TNUMBER:
    {
        // Check if the number is actually an integer
        double num = lua_tonumber(L, index);
        int int_val = static_cast<int>(num);
        if (num == static_cast<double>(int_val))
            return Variant(int_val); // Return as integer
        else
            return Variant(num); // Return as float
    }

    case LUA_TSTRING:
    {
        size_t len;
        const char *str = lua_tolstring(L, index, &len);
        return Variant(String::utf8(str, len));
    }

    case LUA_TTABLE:
    {
        // Check if the table is array-like
        if (is_array(L, index))
            return Variant(to_array(L, index));
        else
            return Variant(to_dictionary(L, index));
    }

    case LUA_TVECTOR:
        // Luau's native vector type (used for Vector3)
        return Variant(to_vector3(L, index));

    case LUA_TFUNCTION:
    {
        // Create a reference to the function in the registry
        int func_ref = lua_ref(L, index);

        String funcname;
        lua_Debug ar;
        if (lua_getinfo(L, index, "n", &ar))
        {
            funcname = ar.name;
        }
        else
        {
            funcname = "<unknown>";
        }

        Ref<LuaState> state = get_godot_lua_state(L);
        ERR_FAIL_NULL_V_MSG(state, Variant(), "Failed to get LuaState from registry");

        // Create LuaCallable wrapper
        Callable lua_callable = Callable(memnew(LuaCallable(*state, funcname, func_ref)));
        return Variant(lua_callable);
    }

    case LUA_TUSERDATA:
    {
        // Check for math types
        if (is_vector2(L, index))
            return Variant(to_vector2(L, index));
        else if (is_vector2i(L, index))
            return Variant(to_vector2i(L, index));
        else if (is_vector3(L, index))
            return Variant(to_vector3(L, index));
        else if (is_vector3i(L, index))
            return Variant(to_vector3i(L, index));
        else if (is_vector4(L, index))
            return Variant(to_vector4(L, index));
        else if (is_vector4i(L, index))
            return Variant(to_vector4i(L, index));
        else if (is_color(L, index))
            return Variant(to_color(L, index));
        else if (is_rect2(L, index))
            return Variant(to_rect2(L, index));
        else if (is_rect2i(L, index))
            return Variant(to_rect2i(L, index));
        else if (is_aabb(L, index))
            return Variant(to_aabb(L, index));
        else if (is_plane(L, index))
            return Variant(to_plane(L, index));
        else if (is_quaternion(L, index))
            return Variant(to_quaternion(L, index));
        else if (is_basis(L, index))
            return Variant(to_basis(L, index));
        else if (is_transform2d(L, index))
            return Variant(to_transform2d(L, index));
        else if (is_transform3d(L, index))
            return Variant(to_transform3d(L, index));
        else if (is_projection(L, index))
            return Variant(to_projection(L, index));
        else if (is_callable(L, index))
            return Variant(to_callable(L, index));

        ERR_PRINT("Cannot convert Lua userdata to Variant (unknown type).");
        return Variant();
    }

    case LUA_TTHREAD:
    {
        Ref<LuaState> state = get_godot_lua_state(L);
        ERR_FAIL_NULL_V_MSG(state, Variant(), "Failed to get LuaState from registry");
        return Variant(state);
    }

    default:
    {
        ERR_PRINT(vformat("Unsupported Lua type %s at index %d.", lua_typename(L, type_id), index));
        return Variant();
    }
    }
}

void godot::push_array(lua_State *L, const Array &arr)
{
    lua_createtable(L, arr.size(), 0);

    // Lua arrays are 1-indexed
    for (int i = 0; i < arr.size(); i++)
    {
        push_variant(L, arr[i]);
        lua_rawseti(L, -2, i + 1);
    }
}

void godot::push_dictionary(lua_State *L, const Dictionary &dict)
{
    lua_createtable(L, 0, dict.size());

    Array keys = dict.keys();
    for (int i = 0; i < keys.size(); i++)
    {
        Variant key = keys[i];
        Variant val = dict[key];

        // Push key and value
        push_variant(L, key);
        push_variant(L, val);

        // Set table[key] = value
        lua_settable(L, -3);
    }

    // Set the dictionary marker metatable
    luaL_getmetatable(L, GODOT_DICTIONARY_MT);
    lua_setmetatable(L, -2);
}

void godot::push_string(lua_State *L, const String &value)
{
    CharString utf8 = value.utf8();
    lua_pushlstring(L, utf8.get_data(), utf8.length());
}

void godot::push_variant(lua_State *L, const Variant &value)
{
    Variant::Type variant_type = value.get_type();

    switch (variant_type)
    {
    case Variant::NIL:
        lua_pushnil(L);
        break;

    case Variant::BOOL:
        lua_pushboolean(L, static_cast<bool>(value));
        break;

    case Variant::INT:
        lua_pushinteger(L, static_cast<int>(value));
        break;

    case Variant::FLOAT:
        lua_pushnumber(L, static_cast<double>(value));
        break;

    case Variant::STRING:
    case Variant::STRING_NAME:
    {
        String str = value;
        push_string(L, str);
        break;
    }

    case Variant::ARRAY:
        push_array(L, value);
        break;

    case Variant::DICTIONARY:
        push_dictionary(L, value);
        break;

    case Variant::VECTOR2:
        push_vector2(L, value);
        break;

    case Variant::VECTOR2I:
        push_vector2i(L, value);
        break;

    case Variant::VECTOR3:
        push_vector3(L, value);
        break;

    case Variant::VECTOR3I:
        push_vector3i(L, value);
        break;

    case Variant::COLOR:
        push_color(L, value);
        break;

    case Variant::VECTOR4:
        push_vector4(L, value);
        break;

    case Variant::VECTOR4I:
        push_vector4i(L, value);
        break;

    case Variant::RECT2:
        push_rect2(L, value);
        break;

    case Variant::RECT2I:
        push_rect2i(L, value);
        break;

    case Variant::AABB:
        push_aabb(L, value);
        break;

    case Variant::PLANE:
        push_plane(L, value);
        break;

    case Variant::QUATERNION:
        push_quaternion(L, value);
        break;

    case Variant::BASIS:
        push_basis(L, value);
        break;

    case Variant::TRANSFORM2D:
        push_transform2d(L, value);
        break;

    case Variant::TRANSFORM3D:
        push_transform3d(L, value);
        break;

    case Variant::PROJECTION:
        push_projection(L, value);
        break;

    case Variant::CALLABLE:
        push_callable(L, value);
        break;

    case Variant::OBJECT:
    {
        Object *obj = value;
        if (obj)
        {
            LuaState *state = Object::cast_to<LuaState>(obj);
            if (state)
            {
                ERR_FAIL_COND_MSG(state->get_lua_state() != L, "Cannot push Object of type LuaState from a different Lua state.");
                lua_pushthread(L);
            }
            else
            {
                String str = value;
                push_string(L, str);
                ERR_PRINT(vformat("Cannot push Object of type %s to Lua. Converted to string: %s", obj->get_class(), str));
            }
        }
        else
        {
            lua_pushnil(L);
        }
        break;
    }

    default:
    {
        // For unsupported types, convert to string representation
        String str = value;
        push_string(L, str);
        ERR_PRINT(vformat("Variant type %d not directly supported. Converted to string: %s", variant_type, str));
    }
    }
}

// =============================================================================
// Library Entry Point
// =============================================================================

int luaopen_godot(lua_State *L)
{
    // Register dictionary marker metatable (empty, just used as a marker)
    luaL_newmetatable(L, GODOT_DICTIONARY_MT);
    lua_pop(L, 1);

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
