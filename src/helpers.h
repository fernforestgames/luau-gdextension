#pragma once

#include <godot_cpp/variant/string_name.hpp>

struct lua_State;

#define LUA_NOTAG -1

namespace gdluau
{
    using namespace godot;

    // Generic __concat metamethod that converts values to strings
    int generic_lua_concat(lua_State *p_L);

    bool is_valid_index(lua_State *p_L, int p_index);
    StringName to_string_name(lua_State *p_L, int p_index);
} // namespace gdluau
