#include "helpers.h"

#include "string_cache.h"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <lua.h>
#include <lualib.h>

using namespace gdluau;
using namespace godot;

bool gdluau::metatable_matches(lua_State *L, int p_index, const char *p_metatable_name)
{
    ERR_FAIL_COND_V_MSG(!lua_checkstack(L, 2), false, vformat("metatable_matches(%d, %s): Stack overflow. Cannot grow stack.", p_index, p_metatable_name));

    if (!lua_getmetatable(L, p_index))
    {
        return false;
    }

    luaL_getmetatable(L, p_metatable_name);
    bool mt_equal = lua_rawequal(L, -1, -2);
    lua_pop(L, 2); // Pop both metatables

    return mt_equal;
}

int gdluau::generic_lua_concat(lua_State *L)
{
    luaL_checklstring(L, 1, NULL);
    luaL_checklstring(L, 2, NULL);
    lua_concat(L, 2);
    return 1;
}

bool gdluau::is_valid_index(lua_State *L, int p_index)
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

StringName gdluau::to_stringname(lua_State *L, int p_index)
{
    ERR_FAIL_COND_V_MSG(lua_type(L, p_index) != LUA_TSTRING, StringName(), vformat("to_stringname(%d): Value is not a string.", p_index));

    size_t len;
    int atom = -1;
    const char *str = lua_tolstringatom(L, p_index, &len, &atom);
    if (!str)
    {
        return StringName();
    }

    const StringName &cached = string_name_for_atom(atom);
    if (cached.is_empty())
    {
        return StringName(String::utf8(str, len));
    }
    else
    {
        return cached;
    }
}
