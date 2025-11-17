#pragma once

#include <godot_cpp/classes/object.hpp>

#include "helpers.h"

struct lua_State;

namespace gdluau
{
    using namespace godot;

    void push_object_metatable(lua_State *p_L);

    bool is_object(lua_State *p_L, int p_index, int p_tag = LUA_NOTAG);

    Object *to_full_object(lua_State *p_L, int p_index, int p_tag = LUA_NOTAG);
    Object *to_light_object(lua_State *p_L, int p_index, int p_tag = LUA_NOTAG);
    Object *to_object(lua_State *p_L, int p_index, int p_tag = LUA_NOTAG);

    void push_full_object(lua_State *p_L, Object *p_obj, int p_tag);
    void push_light_object(lua_State *p_L, Object *p_obj, int p_tag);
    void push_object(lua_State *p_L, Object *p_obj, int p_tag);
} // namespace gdluau
