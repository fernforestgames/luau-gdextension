#pragma once

#include <godot_cpp/variant/dictionary.hpp>

struct lua_State;

namespace godot
{
    Dictionary to_dictionary(lua_State *p_L, int p_index, bool *r_success = nullptr);
    void push_dictionary(lua_State *p_L, const Dictionary &p_dict);
} // namespace godot
