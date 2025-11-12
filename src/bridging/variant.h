#pragma once

#include <godot_cpp/variant/variant.hpp>

struct lua_State;

namespace gdluau
{
    using namespace godot;

    Variant to_variant(lua_State *p_L, int p_index);
    void push_variant(lua_State *p_L, const Variant &p_variant);

    bool call_togodot_metamethod(lua_State *p_L, int p_index, Variant &r_result, int p_tag = 0);
} // namespace gdluau
