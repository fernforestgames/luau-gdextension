#pragma once

#include <godot_cpp/variant/variant.hpp>
#include <lua.h>

namespace godot
{
    // Userdata tags for each math type
    enum LuaMathTag
    {
        LUA_TAG_VECTOR2 = 1,
        LUA_TAG_VECTOR2I,
        // Vector3 uses native vector type, so does not have a userdata tag
        LUA_TAG_VECTOR3I = 3,
        LUA_TAG_VECTOR4,
        LUA_TAG_VECTOR4I,
        LUA_TAG_RECT2,
        LUA_TAG_RECT2I,
        LUA_TAG_AABB,
        LUA_TAG_COLOR,
        LUA_TAG_PLANE,
        LUA_TAG_QUATERNION,
        LUA_TAG_BASIS,
        LUA_TAG_TRANSFORM2D,
        LUA_TAG_TRANSFORM3D,
        LUA_TAG_PROJECTION,
    };

    void push_vector2(lua_State *L, const Vector2 &value);
    void push_vector2i(lua_State *L, const Vector2i &value);
    void push_vector3(lua_State *L, const Vector3 &value);
    void push_vector3i(lua_State *L, const Vector3i &value);
    void push_vector4(lua_State *L, const Vector4 &value);
    void push_vector4i(lua_State *L, const Vector4i &value);
    void push_rect2(lua_State *L, const Rect2 &value);
    void push_rect2i(lua_State *L, const Rect2i &value);
    void push_aabb(lua_State *L, const AABB &value);
    void push_color(lua_State *L, const Color &value);
    void push_plane(lua_State *L, const Plane &value);
    void push_quaternion(lua_State *L, const Quaternion &value);
    void push_basis(lua_State *L, const Basis &value);
    void push_transform2d(lua_State *L, const Transform2D &value);
    void push_transform3d(lua_State *L, const Transform3D &value);
    void push_projection(lua_State *L, const Projection &value);

    bool is_vector2(lua_State *L, int index);
    bool is_vector2i(lua_State *L, int index);
    bool is_vector3(lua_State *L, int index);
    bool is_vector3i(lua_State *L, int index);
    bool is_vector4(lua_State *L, int index);
    bool is_vector4i(lua_State *L, int index);
    bool is_rect2(lua_State *L, int index);
    bool is_rect2i(lua_State *L, int index);
    bool is_aabb(lua_State *L, int index);
    bool is_color(lua_State *L, int index);
    bool is_plane(lua_State *L, int index);
    bool is_quaternion(lua_State *L, int index);
    bool is_basis(lua_State *L, int index);
    bool is_transform2d(lua_State *L, int index);
    bool is_transform3d(lua_State *L, int index);
    bool is_projection(lua_State *L, int index);

    Vector2 to_vector2(lua_State *L, int index);
    Vector2i to_vector2i(lua_State *L, int index);
    Vector3 to_vector3(lua_State *L, int index);
    Vector3i to_vector3i(lua_State *L, int index);
    Vector4 to_vector4(lua_State *L, int index);
    Vector4i to_vector4i(lua_State *L, int index);
    Rect2 to_rect2(lua_State *L, int index);
    Rect2i to_rect2i(lua_State *L, int index);
    AABB to_aabb(lua_State *L, int index);
    Color to_color(lua_State *L, int index);
    Plane to_plane(lua_State *L, int index);
    Quaternion to_quaternion(lua_State *L, int index);
    Basis to_basis(lua_State *L, int index);
    Transform2D to_transform2d(lua_State *L, int index);
    Transform3D to_transform3d(lua_State *L, int index);
    Projection to_projection(lua_State *L, int index);

} // namespace godot

#ifdef __cplusplus
extern "C"
{
#endif

#define LUA_GODOTLIBNAME "godot"
    int luaopen_godot(lua_State *L);

#ifdef __cplusplus
}
#endif
