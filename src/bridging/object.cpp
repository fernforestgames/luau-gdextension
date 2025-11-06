#include "bridging/object.h"
#include "helpers.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <lua.h>
#include <lualib.h>

using namespace godot;

static const char *const OBJECT_METATABLE_NAME = "GDObject";

static void refcounted_dtor(void *ud)
{
    RefCounted *rc = static_cast<RefCounted *>(ud);
    if (rc->unreference())
    {
        memdelete(rc);
    }
}

static void object_dtor(lua_State *L, void *ud)
{
    Object *obj = static_cast<Object *>(ud);
    RefCounted *rc = Object::cast_to<RefCounted>(obj);
    if (rc && rc->unreference())
    {
        memdelete(rc);
    }
}

static Object *userdata_to_object(lua_State *L, int p_index, int p_tag)
{
    void *ud = p_tag == -1 ? lua_touserdata(L, p_index) : lua_touserdatatagged(L, p_index, p_tag);
    uint64_t instance_id = *static_cast<uint64_t *>(ud);
    return ObjectDB::get_instance(instance_id);
}

// Object.__tostring metamethod
static int object_tostring(lua_State *L)
{
    Object *obj = userdata_to_object(L, 1, -1);
    lua_pop(L, 1);

    CharString utf8 = obj->to_string().utf8();
    lua_pushlstring(L, utf8.get_data(), utf8.length());
    return 1;
}

// Object.__eq metamethod
static int object_eq(lua_State *L)
{
    uint64_t a = *static_cast<uint64_t *>(lua_touserdata(L, 1));
    uint64_t b = *static_cast<uint64_t *>(lua_touserdata(L, 2));
    lua_pop(L, 2);

    lua_pushboolean(L, a == b);
    return 1;
}

// Object.__lt metamethod
static int object_lt(lua_State *L)
{
    uint64_t a = *static_cast<uint64_t *>(lua_touserdata(L, 1));
    uint64_t b = *static_cast<uint64_t *>(lua_touserdata(L, 2));
    lua_pop(L, 2);

    lua_pushboolean(L, a < b);
    return 1;
}

// Object.__le metamethod
static int object_le(lua_State *L)
{
    uint64_t a = *static_cast<uint64_t *>(lua_touserdata(L, 1));
    uint64_t b = *static_cast<uint64_t *>(lua_touserdata(L, 2));
    lua_pop(L, 2);

    lua_pushboolean(L, a <= b);
    return 1;
}

static void push_object_metatable(lua_State *L)
{
    if (!luaL_newmetatable(L, OBJECT_METATABLE_NAME))
    {
        // Metatable already configured
        return;
    }

    // TODO: __namecall optimization

    lua_pushcfunction(L, object_tostring, "Object.__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_pushcfunction(L, generic_lua_concat, "Object.__concat");
    lua_setfield(L, -2, "__concat");

    lua_pushcfunction(L, object_eq, "Object.__eq");
    lua_setfield(L, -2, "__eq");

    lua_pushcfunction(L, object_lt, "Object.__lt");
    lua_setfield(L, -2, "__lt");

    lua_pushcfunction(L, object_le, "Object.__le");
    lua_setfield(L, -2, "__le");
}

Object *godot::to_full_object(lua_State *L, int p_index, int p_tag)
{
    ERR_FAIL_COND_V_MSG(p_index > lua_gettop(L), nullptr, vformat("to_object(%d): Invalid stack index. Stack has %d elements.", p_index, lua_gettop(L)));

    if (lua_type(L, p_index) == LUA_TUSERDATA && metatable_matches(L, p_index, OBJECT_METATABLE_NAME))
    {
        return userdata_to_object(L, p_index, p_tag);
    }
    else
    {
        return nullptr;
    }
}

Object *godot::to_light_object(lua_State *L, int p_index, int p_tag)
{
    ERR_FAIL_COND_V_MSG(p_index > lua_gettop(L), nullptr, vformat("to_light_object(%d): Invalid stack index. Stack has %d elements.", p_index, lua_gettop(L)));

    if (lua_islightuserdata(L, p_index))
    {
        void *ud = p_tag == -1 ? lua_tolightuserdata(L, p_index) : lua_tolightuserdatatagged(L, p_index, p_tag);
        return static_cast<Object *>(ud);
    }
    else
    {
        return nullptr;
    }
}

Object *godot::to_object(lua_State *L, int p_index, int p_tag)
{
    ERR_FAIL_COND_V_MSG(p_index > lua_gettop(L), nullptr, vformat("to_object(%d): Invalid stack index. Stack has %d elements.", p_index, lua_gettop(L)));

    switch (lua_type(L, p_index))
    {
    case LUA_TLIGHTUSERDATA:
        return to_light_object(L, p_index, p_tag);

    case LUA_TUSERDATA:
        return to_full_object(L, p_index, p_tag);

    default:
        return nullptr;
    }
}

void godot::push_full_object(lua_State *L, Object *p_obj, int p_tag)
{
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 2), "push_full_object(): Stack overflow. Cannot grow stack."); // Object + metatable

    void *ptr = nullptr;
    RefCounted *rc = Object::cast_to<RefCounted>(p_obj);
    if (rc && rc->reference())
    {
        if (p_tag == -1)
        {
            ptr = lua_newuserdatadtor(L, sizeof(uint64_t), refcounted_dtor);
        }
        else
        {
            ptr = lua_newuserdatatagged(L, sizeof(uint64_t), p_tag);

            // We can't guarantee this tag will only be applied to RefCounted objects, so set a destructor that works with any Object
            lua_setuserdatadtor(L, p_tag, object_dtor);
        }
    }

    if (!ptr)
    {
        if (p_tag == -1)
        {
            ptr = lua_newuserdata(L, sizeof(uint64_t));
        }
        else
        {
            ptr = lua_newuserdatatagged(L, sizeof(uint64_t), p_tag);
        }
    }

    *static_cast<uint64_t *>(ptr) = p_obj->get_instance_id();

    // Attach metatable that allows us to identify this as a bridged object
    push_object_metatable(L);
    lua_setmetatable(L, -2);
}

void godot::push_light_object(lua_State *L, Object *p_obj, int p_tag)
{
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "push_light_object(): Stack overflow. Cannot grow stack.");

    if (p_tag == -1)
    {
        lua_pushlightuserdata(L, static_cast<void *>(p_obj));
    }
    else
    {
        lua_pushlightuserdatatagged(L, static_cast<void *>(p_obj), p_tag);
    }
}
