#include "bridging/array.h"

#include "bridging/variant.h"
#include "helpers.h"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <lua.h>

using namespace gdluau;
using namespace godot;

bool gdluau::is_array(lua_State *L, int p_index)
{
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, p_index), false, vformat("is_array(%d): Invalid stack index. Stack has %d elements.", p_index, lua_gettop(L)));

    if (!lua_istable(L, p_index))
    {
        // Not a table
        return false;
    }

    ERR_FAIL_COND_V_MSG(!lua_checkstack(L, 2), false, vformat("is_array(%d): Stack overflow. Cannot grow stack.", p_index));

    int iter = 0;
    while ((iter = lua_rawiter(L, p_index, iter)) >= 0)
    {
        int isnum;
        double num = lua_tonumberx(L, -2, &isnum);
        lua_pop(L, 2); // Pop key and value

        if (!isnum || nearbyint(num) != num)
        {
            // Non-integer key found
            return false;
        }

        if (static_cast<int>(num) != iter)
        {
            // Key found that doesn't sequentially count from 1
            return false;
        }
    }

    return true;
}

Array gdluau::to_array(lua_State *L, int p_index, bool *r_is_array)
{
    if (r_is_array)
    {
        *r_is_array = false;
    }

    ERR_FAIL_COND_V_MSG(!is_valid_index(L, p_index), Array(), vformat("to_array(%d): Invalid stack index. Stack has %d elements.", p_index, lua_gettop(L)));

    if (!lua_istable(L, p_index)) [[unlikely]]
    {
        // Not a table
        return Array();
    }

    ERR_FAIL_COND_V_MSG(!lua_checkstack(L, 2), Array(), vformat("to_array(%d): Stack overflow. Cannot grow stack.", p_index));

    Array arr;
    int iter = 0;
    bool is_array = true;
    while ((iter = lua_rawiter(L, p_index, iter)) >= 0)
    {
        int isnum;
        double num = lua_tonumberx(L, -2, &isnum);

        if (!isnum || nearbyint(num) != num || static_cast<int>(num) != iter)
        {
            // Found non-array portion of the table
            is_array = false;
            lua_pop(L, 2); // Pop key and value
            break;
        }

        Variant value = to_variant(L, -1);
        arr.push_back(value);
        lua_pop(L, 2);
    }

    if (r_is_array)
    {
        *r_is_array = is_array;
    }

    return arr;
}

void gdluau::push_array(lua_State *L, const Array &p_arr)
{
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 2), "LuaState.push_array(): Stack overflow. Cannot grow stack.");

    lua_createtable(L, p_arr.size(), 0);
    for (int i = 0; i < p_arr.size(); i++)
    {
        push_variant(L, p_arr[i]);
        lua_rawseti(L, -2, i + 1); // Lua arrays are 1-based
    }
}
