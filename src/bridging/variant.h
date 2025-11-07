#pragma once

#include <godot_cpp/variant/variant.hpp>

struct lua_State;

namespace gdluau
{
    using namespace godot;

    Variant to_variant(lua_State *p_L, int p_index);
    void push_variant(lua_State *p_L, const Variant &p_variant);
} // namespace gdluau
