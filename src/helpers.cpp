#include "helpers.h"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <lua.h>
#include <lualib.h>

using namespace godot;

bool godot::metatable_matches(lua_State *L, int p_index, const char *p_metatable_name)
{
    ERR_FAIL_COND_V_MSG(!lua_checkstack(L, 2), nullptr, vformat("metatable_matches(%d, %s): Stack overflow. Cannot grow stack.", p_index, p_metatable_name));

    if (!lua_getmetatable(L, p_index))
    {
        return false;
    }

    luaL_getmetatable(L, p_metatable_name);
    bool mt_equal = lua_rawequal(L, -1, -2);
    lua_pop(L, 2); // Pop both metatables

    return mt_equal;
}

int godot::generic_lua_concat(lua_State *L)
{
    if (!lua_tostring(L, 1))
    {
        luaL_error(L, "generic_lua_concat(): First argument cannot be converted to string.");
    }
    if (!lua_tostring(L, 2))
    {
        luaL_error(L, "generic_lua_concat(): Second argument cannot be converted to string.");
    }

    lua_concat(L, 2);
    return 1;
}

bool godot::is_valid_index(lua_State *L, int p_index)
{
    if (p_index == 0)
    {
        return false; // Index 0 is never valid in Lua
    }
    else if (lua_ispseudo(p_index))
    {
        return true;
    }

    int top = lua_gettop(L);

    if (p_index > 0)
    {
        // Positive indices must be <= top
        return p_index <= top;
    }
    else
    {
        // Negative indices: -1 is top, -2 is top-1, etc.
        // Valid range is [-top, -1]
        return p_index >= -top;
    }
}
