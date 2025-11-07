#pragma once

struct lua_State;

namespace godot
{
    // Destructor definitions for any Godot object userdata, for use with lua_setuserdatadtor() or lua_newuserdatadtor()
    template <typename T>
    void userdata_dtor(void *p_ud)
    {
        T *obj = static_cast<T *>(p_ud);
        if (obj)
        {
            obj->~T();
        }
    }

    template <typename T>
    void userdata_dtor(lua_State *p_state, void *p_ud)
    {
        T *obj = static_cast<T *>(p_ud);
        if (obj)
        {
            obj->~T();
        }
    }

    bool metatable_matches(lua_State *p_L, int p_index, const char *p_metatable_name);

    // Generic __concat metamethod that converts values to strings
    int generic_lua_concat(lua_State *p_L);

    bool is_valid_index(lua_State *p_L, int p_index);
} // namespace godot
