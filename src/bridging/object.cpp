#include "bridging/object.h"
#include "helpers.h"

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <lua.h>
#include <lualib.h>

using namespace gdluau;
using namespace godot;

static const char *const OBJECT_METATABLE_NAME = "GDObject";

static Object *get_userdata_instance(void *ud)
{
    uint64_t instance_id = *static_cast<uint64_t *>(ud);
    return ObjectDB::get_instance(instance_id);
}

static void set_userdata_instance(void *ud, Object *p_obj)
{
    *static_cast<uint64_t *>(ud) = p_obj->get_instance_id();
}

static void refcounted_dtor(void *ud)
{
    RefCounted *rc = static_cast<RefCounted *>(get_userdata_instance(ud));
    if (rc && rc->unreference())
    {
        memdelete(rc);
    }
}

static void object_dtor(lua_State *L, void *ud)
{
    RefCounted *rc = Object::cast_to<RefCounted>(get_userdata_instance(ud));
    if (rc && rc->unreference())
    {
        memdelete(rc);
    }
}

static Object *to_userdata(lua_State *L, int p_index, int p_tag)
{
    void *ud = p_tag == -1 ? lua_touserdata(L, p_index) : lua_touserdatatagged(L, p_index, p_tag);
    if (ud)
    {
        return get_userdata_instance(ud);
    }
    else
    {
        return nullptr;
    }
}

// Object.__tostring metamethod
static int object_tostring(lua_State *L)
{
    Object *obj = to_userdata(L, 1, -1);
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

void gdluau::push_object_metatable(lua_State *L)
{
    if (!luaL_newmetatable(L, OBJECT_METATABLE_NAME))
    {
        // Metatable already configured
        return;
    }

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

Object *gdluau::to_full_object(lua_State *L, int p_index, int p_tag)
{
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, p_index), nullptr, vformat("to_object(%d): Invalid stack index. Stack has %d elements.", p_index, lua_gettop(L)));

    if (lua_type(L, p_index) == LUA_TUSERDATA && metatable_matches(L, p_index, OBJECT_METATABLE_NAME))
    {
        return to_userdata(L, p_index, p_tag);
    }
    else
    {
        return nullptr;
    }
}

Object *gdluau::to_light_object(lua_State *L, int p_index, int p_tag)
{
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, p_index), nullptr, vformat("to_light_object(%d): Invalid stack index. Stack has %d elements.", p_index, lua_gettop(L)));

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

Object *gdluau::to_object(lua_State *L, int p_index, int p_tag)
{
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, p_index), nullptr, vformat("to_object(%d): Invalid stack index. Stack has %d elements.", p_index, lua_gettop(L)));

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

static void push_refcounted_object(lua_State *L, RefCounted *p_obj)
{
    if (!p_obj->init_ref())
    {
        lua_pushnil(L);
        return;
    }

    // Simple, inline RefCounted destructor
    void *ud = lua_newuserdatadtor(L, sizeof(uint64_t), refcounted_dtor);
    set_userdata_instance(ud, p_obj);

    // Attach normal bridged object metatable
    push_object_metatable(L);
    lua_setmetatable(L, -2);
}

static void push_refcounted_object_custom(lua_State *L, RefCounted *p_obj, int p_tag)
{
    if (!p_obj->init_ref())
    {
        lua_pushnil(L);
        return;
    }

    void *ud = lua_newuserdatatagged(L, sizeof(uint64_t), p_tag);
    set_userdata_instance(ud, p_obj);

    // We can't guarantee this tag will only be applied to RefCounted, so need to set a destructor that works with any Object
    lua_setuserdatadtor(L, p_tag, object_dtor);

    // Attach custom metatable
    lua_getuserdatametatable(L, p_tag);
    lua_setmetatable(L, -2);
}

static void push_weak_object(lua_State *L, Object *p_obj)
{
    // We'll just hold a weak reference, with no destructor
    void *ud = lua_newuserdata(L, sizeof(uint64_t));
    set_userdata_instance(ud, p_obj);

    // Attach normal bridged object metatable
    push_object_metatable(L);
    lua_setmetatable(L, -2);
}

static void push_weak_object_custom(lua_State *L, Object *p_obj, int p_tag)
{
    // We'll just hold a weak reference, with no destructor
    void *ud = lua_newuserdatatagged(L, sizeof(uint64_t), p_tag);
    set_userdata_instance(ud, p_obj);

    // Attach custom metatable
    lua_getuserdatametatable(L, p_tag);
    lua_setmetatable(L, -2);
}

void gdluau::push_full_object(lua_State *L, Object *p_obj, int p_tag)
{
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 2), "push_full_object(): Stack overflow. Cannot grow stack."); // Object + metatable

    RefCounted *rc = Object::cast_to<RefCounted>(p_obj);
    if (rc && p_tag == -1)
    {
        push_refcounted_object(L, rc);
    }
    else if (rc && p_tag != -1)
    {
        push_refcounted_object_custom(L, rc, p_tag);
    }
    else if (p_tag == -1)
    {
        push_weak_object(L, p_obj);
    }
    else
    {
        push_weak_object_custom(L, p_obj, p_tag);
    }
}

void gdluau::push_light_object(lua_State *L, Object *p_obj, int p_tag)
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
