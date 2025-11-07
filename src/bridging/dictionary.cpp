#include "bridging/dictionary.h"
#include "bridging/variant.h"
#include "helpers.h"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <lua.h>

using namespace gdluau;
using namespace godot;

Dictionary gdluau::to_dictionary(lua_State *L, int p_index, bool *r_success)
{
    if (r_success)
    {
        *r_success = false;
    }

    ERR_FAIL_COND_V_MSG(!is_valid_index(L, p_index), Dictionary(), vformat("to_dictionary(%d): Invalid stack index. Stack has %d elements.", p_index, lua_gettop(L)));

    if (!lua_istable(L, p_index))
    {
        // Not a table
        return Dictionary();
    }

    ERR_FAIL_COND_V_MSG(!lua_checkstack(L, 2), Dictionary(), vformat("to_dictionary(%d): Stack overflow. Cannot grow stack.", p_index));

    Dictionary dict;
    int iter = 0;
    while ((iter = lua_rawiter(L, p_index, iter)) >= 0)
    {
        Variant key = to_variant(L, -2);
        Variant value = to_variant(L, -1);
        lua_pop(L, 2);

        dict[key] = value;
    }

    if (r_success)
    {
        *r_success = true;
    }

    return dict;
}

void gdluau::push_dictionary(lua_State *L, const Dictionary &p_dict)
{
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 3), "push_dictionary(): Stack overflow. Cannot grow stack.");

    lua_createtable(L, 0, p_dict.size());
    for (const Variant &key : p_dict.keys())
    {
        Variant value = p_dict[key];
        push_variant(L, key);
        push_variant(L, value);
        lua_rawset(L, -3);
    }
}
