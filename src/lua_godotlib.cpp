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
    String str = vformat("Vector2(%f, %f)", v->x, v->y);
    lua_pushstring(L, str.utf8().get_data());
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
    String str = vformat("Vector2i(%d, %d)", v->x, v->y);
    lua_pushstring(L, str.utf8().get_data());
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

// Vector3 now uses Luau's native vector type - no metatable needed
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
    String str = vformat("Vector3i(%d, %d, %d)", v->x, v->y, v->z);
    lua_pushstring(L, str.utf8().get_data());
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
    String str = vformat("Color(%f, %f, %f, %f)", c->r, c->g, c->b, c->a);
    lua_pushstring(L, str.utf8().get_data());
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

// Stub implementations for other types (to be implemented later)
void godot::push_vector4(lua_State *L, const Vector4 &value) { /* TODO */ }
void godot::push_vector4i(lua_State *L, const Vector4i &value) { /* TODO */ }
void godot::push_rect2(lua_State *L, const Rect2 &value) { /* TODO */ }
void godot::push_rect2i(lua_State *L, const Rect2i &value) { /* TODO */ }
void godot::push_aabb(lua_State *L, const AABB &value) { /* TODO */ }
void godot::push_plane(lua_State *L, const Plane &value) { /* TODO */ }
void godot::push_quaternion(lua_State *L, const Quaternion &value) { /* TODO */ }
void godot::push_basis(lua_State *L, const Basis &value) { /* TODO */ }
void godot::push_transform2d(lua_State *L, const Transform2D &value) { /* TODO */ }
void godot::push_transform3d(lua_State *L, const Transform3D &value) { /* TODO */ }
void godot::push_projection(lua_State *L, const Projection &value) { /* TODO */ }

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

// Stub implementations for other types
bool godot::is_vector4(lua_State *L, int index) { return false; /* TODO */ }
bool godot::is_vector4i(lua_State *L, int index) { return false; /* TODO */ }
bool godot::is_rect2(lua_State *L, int index) { return false; /* TODO */ }
bool godot::is_rect2i(lua_State *L, int index) { return false; /* TODO */ }
bool godot::is_aabb(lua_State *L, int index) { return false; /* TODO */ }
bool godot::is_plane(lua_State *L, int index) { return false; /* TODO */ }
bool godot::is_quaternion(lua_State *L, int index) { return false; /* TODO */ }
bool godot::is_basis(lua_State *L, int index) { return false; /* TODO */ }
bool godot::is_transform2d(lua_State *L, int index) { return false; /* TODO */ }
bool godot::is_transform3d(lua_State *L, int index) { return false; /* TODO */ }
bool godot::is_projection(lua_State *L, int index) { return false; /* TODO */ }

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

// Stub implementations for other types
Vector4 godot::to_vector4(lua_State *L, int index) { return Vector4(); /* TODO */ }
Vector4i godot::to_vector4i(lua_State *L, int index) { return Vector4i(); /* TODO */ }
Rect2 godot::to_rect2(lua_State *L, int index) { return Rect2(); /* TODO */ }
Rect2i godot::to_rect2i(lua_State *L, int index) { return Rect2i(); /* TODO */ }
AABB godot::to_aabb(lua_State *L, int index) { return AABB(); /* TODO */ }
Plane godot::to_plane(lua_State *L, int index) { return Plane(); /* TODO */ }
Quaternion godot::to_quaternion(lua_State *L, int index) { return Quaternion(); /* TODO */ }
Basis godot::to_basis(lua_State *L, int index) { return Basis(); /* TODO */ }
Transform2D godot::to_transform2d(lua_State *L, int index) { return Transform2D(); /* TODO */ }
Transform3D godot::to_transform3d(lua_State *L, int index) { return Transform3D(); /* TODO */ }
Projection godot::to_projection(lua_State *L, int index) { return Projection(); /* TODO */ }

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
    register_color_metatable(L);

    // TODO: Register other types

    return 1;
}
