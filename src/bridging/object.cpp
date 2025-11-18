#include "bridging/object.h"

#include "bridging/variant.h"
#include "helpers.h"
#include "lua_state.h"
#include "static_strings.h"

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <lua.h>
#include <lualib.h>

using namespace gdluau;
using namespace godot;

static const char *const OBJECT_METATABLE_NAME = "GDObject";

static ObjectID get_userdata(void *ud)
{
    return *static_cast<ObjectID *>(ud);
}

static Object *get_userdata_instance(void *ud)
{
    return ObjectDB::get_instance(get_userdata(ud));
}

static void set_userdata_instance(void *ud, Object *p_obj)
{
    *static_cast<ObjectID *>(ud) = ObjectID(p_obj->get_instance_id());
}

static void inline_refcounted_dtor(void *ud)
{
    RefCounted *rc = static_cast<RefCounted *>(get_userdata_instance(ud));
    if (rc && rc->unreference())
    {
        memdelete(rc);
    }
}

static void tagged_object_dtor(lua_State *L, void *ud)
{
    ObjectID id = get_userdata(ud);
    if (!id.is_ref_counted())
    {
        // Non-RefCounted Objects are held weakly, nothing to do
        return;
    }

    RefCounted *rc = static_cast<RefCounted *>(ObjectDB::get_instance(id));
    if (rc && rc->unreference())
    {
        memdelete(rc);
    }
}

// Object.__tostring metamethod
static int object_tostring(lua_State *L)
{
    Object *obj = get_userdata_instance(lua_touserdata(L, 1));
    lua_pop(L, 1);

    if (!obj)
    {
        lua_pushliteral(L, "<null>");
        return 1;
    }

    CharString utf8 = obj->to_string().utf8();
    lua_pushlstring(L, utf8.get_data(), utf8.length());
    return 1;
}

// Object.__eq metamethod
static int object_eq(lua_State *L)
{
    ObjectID a = get_userdata(lua_touserdata(L, 1));
    ObjectID b = get_userdata(lua_touserdata(L, 2));
    lua_pop(L, 2);

    lua_pushboolean(L, a == b);
    return 1;
}

// Object.__lt metamethod
static int object_lt(lua_State *L)
{
    ObjectID a = get_userdata(lua_touserdata(L, 1));
    ObjectID b = get_userdata(lua_touserdata(L, 2));
    lua_pop(L, 2);

    lua_pushboolean(L, a < b);
    return 1;
}

// Object.__le metamethod
static int object_le(lua_State *L)
{
    ObjectID a = get_userdata(lua_touserdata(L, 1));
    ObjectID b = get_userdata(lua_touserdata(L, 2));
    lua_pop(L, 2);

    lua_pushboolean(L, !(b < a));
    return 1;
}

static bool has_object_metatable(lua_State *L, int p_index)
{
    ERR_FAIL_COND_V_MSG(!lua_checkstack(L, 3), false, vformat("has_object_metatable(%d): Stack overflow. Cannot grow stack.", p_index));

    // Negative indices will be invalidated by luaL_getmetatable
    int abs_index = lua_absindex(L, p_index);

    luaL_getmetatable(L, OBJECT_METATABLE_NAME);
    if (!lua_getmetatable(L, abs_index)) [[unlikely]]
    {
        lua_pop(L, 1); // Pop Object metatable
        return false;
    }

    while (!lua_rawequal(L, -1, -2))
    {
        // Look for "inherited" metatable
        int type = lua_rawgetfield(L, -1, "__index");
        lua_remove(L, -2); // Remove previous metatable

        if (type != LUA_TTABLE)
        {
            lua_pop(L, 2); // Pop __index and Object metatable
            return false;
        }
    }

    // Found Object metatable
    lua_pop(L, 2); // Pop both metatables
    return true;
}

void gdluau::push_object_metatable(lua_State *L)
{
    if (!luaL_newmetatable(L, OBJECT_METATABLE_NAME)) [[likely]]
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

    // Freeze metatable
    lua_setreadonly(L, -1, 1);
}

bool gdluau::is_object(lua_State *L, int p_index, int p_tag)
{
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, p_index), false, vformat("is_object(%d, %d): Invalid stack index. Stack has %d elements.", p_index, p_tag, lua_gettop(L)));

    switch (lua_type(L, p_index))
    {
    case LUA_TLIGHTUSERDATA:
        return p_tag == LUA_NOTAG || lua_lightuserdatatag(L, p_index) == p_tag;

    case LUA_TUSERDATA:
    {
        int actual_tag = lua_userdatatag(L, p_index);
        if (p_tag != LUA_NOTAG)
        {
            return actual_tag == p_tag;
        }
        else if (actual_tag < 0 || actual_tag >= LUA_UTAG_LIMIT)
        {
            // Userdata without a custom tag (probably an inline dtor)
            // Could be a Variant, Callable, or something else, so we need to check metatables
            if (!has_object_metatable(L, p_index))
            {
                return false;
            }
        }

        return true;
    }

    default:
        return false;
    }
}

Object *gdluau::to_full_object(lua_State *L, int p_index, int p_tag)
{
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, p_index), nullptr, vformat("to_object(%d): Invalid stack index. Stack has %d elements.", p_index, lua_gettop(L)));

    if (lua_type(L, p_index) != LUA_TUSERDATA || !is_object(L, p_index, p_tag)) [[unlikely]]
    {
        return nullptr;
    }

    return get_userdata_instance(lua_touserdata(L, p_index));
}

Object *gdluau::to_light_object(lua_State *L, int p_index, int p_tag)
{
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, p_index), nullptr, vformat("to_light_object(%d): Invalid stack index. Stack has %d elements.", p_index, lua_gettop(L)));

    if (!lua_islightuserdata(L, p_index)) [[unlikely]]
    {
        return nullptr;
    }

    void *ud = p_tag == LUA_NOTAG ? lua_tolightuserdata(L, p_index) : lua_tolightuserdatatagged(L, p_index, p_tag);
    return static_cast<Object *>(ud);
}

Object *gdluau::to_object(lua_State *L, int p_index, int p_tag)
{
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, p_index), nullptr, vformat("to_object(%d): Invalid stack index. Stack has %d elements.", p_index, lua_gettop(L)));

    Variant togodot_result;
    if (call_togodot_metamethod(L, p_index, togodot_result, p_tag)) [[unlikely]]
    {
        Variant::Type type = togodot_result.get_type();
        ERR_FAIL_COND_V_MSG(type != Variant::NIL && type != Variant::OBJECT, nullptr, vformat("to_object(%d): __togodot did not return an Object.", p_index));

        return static_cast<Object *>(togodot_result);
    }

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
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 2), "push_refcounted_object(): Stack overflow. Cannot grow stack.");

    if (!p_obj || !p_obj->init_ref()) [[unlikely]]
    {
        lua_pushnil(L);
        return;
    }

    // Use of an inline dtor is REQUIRED to not conflict with user's custom userdata tags
    void *ud = lua_newuserdatadtor(L, sizeof(ObjectID), inline_refcounted_dtor);
    set_userdata_instance(ud, p_obj);

    push_object_metatable(L);
    lua_setmetatable(L, -2);
}

static void push_refcounted_object_custom(lua_State *L, RefCounted *p_obj, int p_tag)
{
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 2), "push_refcounted_object_custom(): Stack overflow. Cannot grow stack.");

    if (!p_obj || !p_obj->init_ref()) [[unlikely]]
    {
        lua_pushnil(L);
        return;
    }

    void *ud = lua_newuserdatatagged(L, sizeof(ObjectID), p_tag);
    set_userdata_instance(ud, p_obj);
    lua_setuserdatadtor(L, p_tag, tagged_object_dtor);

    lua_getuserdatametatable(L, p_tag);
    lua_setmetatable(L, -2);
}

static void push_weak_object(lua_State *L, Object *p_obj)
{
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 2), "push_weak_object(): Stack overflow. Cannot grow stack.");

    if (!p_obj) [[unlikely]]
    {
        lua_pushnil(L);
        return;
    }

    // Use of the inline dtor constructor is REQUIRED to not conflict with user's custom userdata tags
    void *ud = lua_newuserdatadtor(L, sizeof(ObjectID), nullptr);
    set_userdata_instance(ud, p_obj);

    push_object_metatable(L);
    lua_setmetatable(L, -2);
}

static void push_weak_object_custom(lua_State *L, Object *p_obj, int p_tag)
{
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "push_weak_object_custom(): Stack overflow. Cannot grow stack.");

    if (!p_obj) [[unlikely]]
    {
        lua_pushnil(L);
        return;
    }

    void *ud = lua_newuserdatatagged(L, sizeof(ObjectID), p_tag);
    set_userdata_instance(ud, p_obj);
    // dtor not needed (but if this tag is reused for RefCounted objects, tagged_object_dtor will be compatible)

    lua_getuserdatametatable(L, p_tag);
    lua_setmetatable(L, -2);
}

void gdluau::push_full_object(lua_State *L, Object *p_obj, int p_tag)
{
    RefCounted *rc = Object::cast_to<RefCounted>(p_obj);
    if (rc && p_tag == LUA_NOTAG)
    {
        push_refcounted_object(L, rc);
    }
    else if (rc && p_tag != LUA_NOTAG)
    {
        push_refcounted_object_custom(L, rc, p_tag);
    }
    else if (p_tag == LUA_NOTAG)
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

    if (!p_obj) [[unlikely]]
    {
        lua_pushnil(L);
        return;
    }

    if (p_tag == LUA_NOTAG)
    {
        lua_pushlightuserdata(L, static_cast<void *>(p_obj));
    }
    else
    {
        lua_pushlightuserdatatagged(L, static_cast<void *>(p_obj), p_tag);
    }
}

void gdluau::push_object(lua_State *L, Object *p_obj, int p_tag)
{
    if (!p_obj) [[unlikely]]
    {
        ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "push_object(): Stack overflow. Cannot grow stack.");
        lua_pushnil(L);
        return;
    }
    else if (p_obj->has_method(static_strings->push_to_lua))
    {
        // Object has custom push_to_lua method; use that instead
        p_obj->call(static_strings->push_to_lua, LuaState::find_or_create_lua_state(L), p_tag);
        return;
    }
    else
    {
        push_full_object(L, p_obj, p_tag);
    }
}

void gdluau::update_full_object_tag(lua_State *L, int p_index, int p_tag)
{
    lua_setuserdatatag(L, p_index, p_tag);
    lua_setuserdatadtor(L, p_tag, tagged_object_dtor);
}
