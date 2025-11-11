#pragma once

#include <godot_cpp/classes/object.hpp>

struct lua_State;

namespace gdluau
{
    using namespace godot;

    void push_object_metatable(lua_State *p_L);

    Object *to_full_object(lua_State *p_L, int p_index, int p_tag = -1);
    Object *to_light_object(lua_State *p_L, int p_index, int p_tag = -1);
    Object *to_object(lua_State *p_L, int p_index, int p_tag = -1);
    void push_full_object(lua_State *p_L, Object *p_obj, int p_tag = -1);
    void push_light_object(lua_State *p_L, Object *p_obj, int p_tag = -1);
} // namespace gdluau
