#pragma once

#include <godot_cpp/variant/array.hpp>

struct lua_State;

namespace godot
{
    bool is_array(lua_State *p_L, int p_index);
    Array to_array(lua_State *p_L, int p_index, bool *r_is_array = nullptr);
    void push_array(lua_State *p_L, const Array &p_arr);
} // namespace godot
