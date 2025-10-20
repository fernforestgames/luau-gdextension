#include "lua_state.h"
#include "luau.h"
#include "lua_math_types.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/object.hpp>
#include <lualib.h>

using namespace godot;

// This handler is called when Lua encounters an unprotected error.
// If we don't handle this, Luau will longjmp across Godot's stack frames,
// causing resource leaks and potential crashes.
static void callback_panic(lua_State *L, int errcode)
{
    ERR_PRINT(vformat("Luau panic (error code %d)! LuaState is now invalid and cannot be used further.", errcode));

    LuaState *state = static_cast<LuaState *>(lua_callbacks(L)->userdata);
    state->close();
}

static void callback_debugstep(lua_State *L, lua_Debug *ar)
{
    LuaState *state = static_cast<LuaState *>(lua_callbacks(L)->userdata);
    state->emit_signal("step", state);
}

void LuaState::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("open_libs"), &LuaState::open_libs);
    ClassDB::bind_method(D_METHOD("register_math_types"), &LuaState::register_math_types);
    ClassDB::bind_method(D_METHOD("close"), &LuaState::close);

    ClassDB::bind_method(D_METHOD("load_bytecode", "bytecode", "chunk_name"), &LuaState::load_bytecode);
    ClassDB::bind_method(D_METHOD("resume"), &LuaState::resume);

    ClassDB::bind_method(D_METHOD("singlestep", "enable"), &LuaState::singlestep);
    ClassDB::bind_method(D_METHOD("pause"), &LuaState::pause);

    // Stack manipulation
    ClassDB::bind_method(D_METHOD("gettop"), &LuaState::gettop);
    ClassDB::bind_method(D_METHOD("settop", "index"), &LuaState::settop);
    ClassDB::bind_method(D_METHOD("pop", "n"), &LuaState::pop);
    ClassDB::bind_method(D_METHOD("pushvalue", "index"), &LuaState::pushvalue);
    ClassDB::bind_method(D_METHOD("remove", "index"), &LuaState::remove);
    ClassDB::bind_method(D_METHOD("insert", "index"), &LuaState::insert);
    ClassDB::bind_method(D_METHOD("replace", "index"), &LuaState::replace);

    // Type checking
    ClassDB::bind_method(D_METHOD("isnil", "index"), &LuaState::isnil);
    ClassDB::bind_method(D_METHOD("isnumber", "index"), &LuaState::isnumber);
    ClassDB::bind_method(D_METHOD("isstring", "index"), &LuaState::isstring);
    ClassDB::bind_method(D_METHOD("istable", "index"), &LuaState::istable);
    ClassDB::bind_method(D_METHOD("isfunction", "index"), &LuaState::isfunction);
    ClassDB::bind_method(D_METHOD("isuserdata", "index"), &LuaState::isuserdata);
    ClassDB::bind_method(D_METHOD("isboolean", "index"), &LuaState::isboolean);
    ClassDB::bind_method(D_METHOD("type", "index"), &LuaState::type);
    ClassDB::bind_method(D_METHOD("type_name", "type_id"), &LuaState::type_name);

    // Value access
    ClassDB::bind_method(D_METHOD("tostring", "index"), &LuaState::tostring);
    ClassDB::bind_method(D_METHOD("tonumber", "index"), &LuaState::tonumber);
    ClassDB::bind_method(D_METHOD("tointeger", "index"), &LuaState::tointeger);
    ClassDB::bind_method(D_METHOD("toboolean", "index"), &LuaState::toboolean);

    // Push operations
    ClassDB::bind_method(D_METHOD("pushnil"), &LuaState::pushnil);
    ClassDB::bind_method(D_METHOD("pushnumber", "n"), &LuaState::pushnumber);
    ClassDB::bind_method(D_METHOD("pushinteger", "n"), &LuaState::pushinteger);
    ClassDB::bind_method(D_METHOD("pushstring", "s"), &LuaState::pushstring);
    ClassDB::bind_method(D_METHOD("pushboolean", "b"), &LuaState::pushboolean);

    // Table operations
    ClassDB::bind_method(D_METHOD("newtable"), &LuaState::newtable);
    ClassDB::bind_method(D_METHOD("createtable", "narr", "nrec"), &LuaState::createtable);
    ClassDB::bind_method(D_METHOD("gettable", "index"), &LuaState::gettable);
    ClassDB::bind_method(D_METHOD("settable", "index"), &LuaState::settable);
    ClassDB::bind_method(D_METHOD("getfield", "index", "key"), &LuaState::getfield);
    ClassDB::bind_method(D_METHOD("setfield", "index", "key"), &LuaState::setfield);
    ClassDB::bind_method(D_METHOD("rawget", "index"), &LuaState::rawget);
    ClassDB::bind_method(D_METHOD("rawset", "index"), &LuaState::rawset);
    ClassDB::bind_method(D_METHOD("rawgeti", "index", "n"), &LuaState::rawgeti);
    ClassDB::bind_method(D_METHOD("rawseti", "index", "n"), &LuaState::rawseti);

    // Metatable operations
    ClassDB::bind_method(D_METHOD("getmetatable", "index"), &LuaState::getmetatable);
    ClassDB::bind_method(D_METHOD("setmetatable", "index"), &LuaState::setmetatable);

    // Function calls
    ClassDB::bind_method(D_METHOD("call", "nargs", "nresults"), &LuaState::call);
    ClassDB::bind_method(D_METHOD("pcall", "nargs", "nresults", "errfunc"), &LuaState::pcall);

    // Garbage collection
    ClassDB::bind_method(D_METHOD("gc", "what", "data"), &LuaState::gc);

    BIND_ENUM_CONSTANT(LUA_GCSTOP);
    BIND_ENUM_CONSTANT(LUA_GCRESTART);
    BIND_ENUM_CONSTANT(LUA_GCCOLLECT);
    BIND_ENUM_CONSTANT(LUA_GCCOUNT);
    BIND_ENUM_CONSTANT(LUA_GCCOUNTB);
    BIND_ENUM_CONSTANT(LUA_GCISRUNNING);
    BIND_ENUM_CONSTANT(LUA_GCSTEP);
    BIND_ENUM_CONSTANT(LUA_GCSETGOAL);
    BIND_ENUM_CONSTANT(LUA_GCSETSTEPMUL);
    BIND_ENUM_CONSTANT(LUA_GCSETSTEPSIZE);

    // Godot integration helpers
    ClassDB::bind_method(D_METHOD("getglobal", "key"), &LuaState::getglobal);
    ClassDB::bind_method(D_METHOD("tovariant", "index"), &LuaState::tovariant);
    ClassDB::bind_method(D_METHOD("pushvariant", "value"), &LuaState::pushvariant);

    ADD_SIGNAL(MethodInfo("step", PropertyInfo(Variant::OBJECT, "state")));
}

LuaState::LuaState()
{
    L = luaL_newstate();
    ERR_FAIL_NULL_MSG(L, "Failed to create new Lua state.");

    lua_Callbacks *callbacks = lua_callbacks(L);
    callbacks->userdata = this;
    callbacks->debugstep = callback_debugstep;
    callbacks->panic = callback_panic;
}

LuaState::~LuaState()
{
    close();
}

void LuaState::open_libs()
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot open libraries.");

    luaL_openlibs(L);
}

void LuaState::close()
{
    if (L)
    {
        lua_close(L);
        L = nullptr;
    }
}

void LuaState::register_math_types()
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot register math types.");

    godot::register_math_types(L);
}

lua_Status LuaState::load_bytecode(const PackedByteArray &bytecode, const String &chunk_name)
{
    ERR_FAIL_NULL_V_MSG(L, LUA_ERRMEM, "Lua state is null. Cannot load bytecode.");

    int status = luau_load(L, chunk_name.utf8(), reinterpret_cast<const char *>(bytecode.ptr()), bytecode.size(), 0);
    return static_cast<lua_Status>(status);
}

lua_Status LuaState::resume()
{
    ERR_FAIL_NULL_V_MSG(L, LUA_ERRMEM, "Lua state is null. Cannot resume execution.");

    int status = lua_resume(L, nullptr, 0);
    return static_cast<lua_Status>(status);
}

void LuaState::singlestep(bool enable)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot set singlestep mode.");
    lua_singlestep(L, enable);
}

void LuaState::pause()
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot pause.");
    lua_break(L);
}

void LuaState::getglobal(const String &key)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot get global variable.");
    lua_getglobal(L, key.utf8());
}

Variant LuaState::tovariant(int index)
{
    ERR_FAIL_NULL_V_MSG(L, Variant(), "Lua state is null. Cannot convert to Variant.");

    int type_id = type(index);

    switch (type_id)
    {
    case LUA_TNIL:
        return Variant();

    case LUA_TBOOLEAN:
        return Variant(toboolean(index));

    case LUA_TNUMBER:
        return Variant(tonumber(index));

    case LUA_TSTRING:
        return Variant(tostring(index));

    case LUA_TTABLE:
    {
        Dictionary dict;

        // Push nil as the first key for lua_next
        pushnil();

        // Iterate over the table
        while (lua_next(L, index < 0 ? index - 1 : index) != 0)
        {
            // Key is at -2, value is at -1
            Variant key = tovariant(-2);
            Variant value = tovariant(-1);

            dict[key] = value;

            // Remove value, keep key for next iteration
            pop(1);
        }

        return Variant(dict);
    }

    case LUA_TFUNCTION:
        ERR_PRINT("Cannot convert Lua function to Variant.");
        return Variant();

    case LUA_TUSERDATA:
    {
        // Check for math types
        if (is_vector2(L, index))
            return Variant(to_vector2(L, index));
        else if (is_vector2i(L, index))
            return Variant(to_vector2i(L, index));
        else if (is_vector3(L, index))
            return Variant(to_vector3(L, index));
        else if (is_vector3i(L, index))
            return Variant(to_vector3i(L, index));
        else if (is_color(L, index))
            return Variant(to_color(L, index));
        // TODO: Add more types as they are implemented

        ERR_PRINT("Cannot convert Lua userdata to Variant (unknown type).");
        return Variant();
    }

    case LUA_TTHREAD:
        ERR_PRINT("Cannot convert Lua thread to Variant.");
        return Variant();

    default:
        ERR_PRINT(vformat("Unknown Lua type %d at index %d.", type_id, index));
        return Variant();
    }
}

void LuaState::pushvariant(const Variant &value)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot push Variant.");

    Variant::Type variant_type = value.get_type();

    switch (variant_type)
    {
    case Variant::NIL:
        pushnil();
        break;

    case Variant::BOOL:
        pushboolean(static_cast<bool>(value));
        break;

    case Variant::INT:
        pushinteger(static_cast<int>(value));
        break;

    case Variant::FLOAT:
        pushnumber(static_cast<double>(value));
        break;

    case Variant::STRING:
    case Variant::STRING_NAME:
        pushstring(value);
        break;

    case Variant::ARRAY:
    {
        Array arr = value;
        createtable(arr.size(), 0);

        // Lua arrays are 1-indexed
        for (int i = 0; i < arr.size(); i++)
        {
            pushvariant(arr[i]);
            rawseti(-2, i + 1);
        }
        break;
    }

    case Variant::DICTIONARY:
    {
        Dictionary dict = value;
        createtable(0, dict.size());

        Array keys = dict.keys();
        for (int i = 0; i < keys.size(); i++)
        {
            Variant key = keys[i];
            Variant val = dict[key];

            // Push key and value
            pushvariant(key);
            pushvariant(val);

            // Set table[key] = value
            settable(-3);
        }
        break;
    }

    case Variant::VECTOR2:
        push_vector2(L, value);
        break;

    case Variant::VECTOR2I:
        push_vector2i(L, value);
        break;

    case Variant::VECTOR3:
        push_vector3(L, value);
        break;

    case Variant::VECTOR3I:
        push_vector3i(L, value);
        break;

    case Variant::COLOR:
        push_color(L, value);
        break;

    // TODO: Add more math types as they are implemented
    case Variant::VECTOR4:
    case Variant::VECTOR4I:
    case Variant::RECT2:
    case Variant::RECT2I:
    case Variant::AABB:
    case Variant::PLANE:
    case Variant::QUATERNION:
    case Variant::BASIS:
    case Variant::TRANSFORM2D:
    case Variant::TRANSFORM3D:
    case Variant::PROJECTION:
        // For now, convert unsupported math types to string
        {
            String str = value;
            pushstring(str);
            ERR_PRINT(vformat("Variant math type %d not yet supported. Converted to string: %s", variant_type, str));
        }
        break;

    default:
    {
        // For unsupported types, convert to string representation
        String str = value;
        pushstring(str);
        ERR_PRINT(vformat("Variant type %d not directly supported. Converted to string: %s", variant_type, str));
    }
    }
}

// Stack manipulation
int LuaState::gettop()
{
    ERR_FAIL_NULL_V_MSG(L, 0, "Lua state is null. Cannot get stack top.");
    return lua_gettop(L);
}

void LuaState::settop(int index)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot set stack top.");
    lua_settop(L, index);
}

void LuaState::pop(int n)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot pop stack.");
    lua_pop(L, n);
}

void LuaState::pushvalue(int index)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot push value.");
    lua_pushvalue(L, index);
}

void LuaState::remove(int index)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot remove value.");
    lua_remove(L, index);
}

void LuaState::insert(int index)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot insert value.");
    lua_insert(L, index);
}

void LuaState::replace(int index)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot replace value.");
    lua_replace(L, index);
}

// Type checking
bool LuaState::isnil(int index)
{
    ERR_FAIL_NULL_V_MSG(L, false, "Lua state is null. Cannot check type.");
    return lua_isnil(L, index);
}

bool LuaState::isnumber(int index)
{
    ERR_FAIL_NULL_V_MSG(L, false, "Lua state is null. Cannot check type.");
    return lua_isnumber(L, index) != 0;
}

bool LuaState::isstring(int index)
{
    ERR_FAIL_NULL_V_MSG(L, false, "Lua state is null. Cannot check type.");
    return lua_isstring(L, index) != 0;
}

bool LuaState::istable(int index)
{
    ERR_FAIL_NULL_V_MSG(L, false, "Lua state is null. Cannot check type.");
    return lua_istable(L, index);
}

bool LuaState::isfunction(int index)
{
    ERR_FAIL_NULL_V_MSG(L, false, "Lua state is null. Cannot check type.");
    return lua_isfunction(L, index);
}

bool LuaState::isuserdata(int index)
{
    ERR_FAIL_NULL_V_MSG(L, false, "Lua state is null. Cannot check type.");
    return lua_isuserdata(L, index) != 0;
}

bool LuaState::isboolean(int index)
{
    ERR_FAIL_NULL_V_MSG(L, false, "Lua state is null. Cannot check type.");
    return lua_isboolean(L, index);
}

int LuaState::type(int index)
{
    ERR_FAIL_NULL_V_MSG(L, LUA_TNONE, "Lua state is null. Cannot get type.");
    return lua_type(L, index);
}

String LuaState::type_name(int type_id)
{
    ERR_FAIL_NULL_V_MSG(L, String(), "Lua state is null. Cannot get type name.");
    return String(lua_typename(L, type_id));
}

// Value access
String LuaState::tostring(int index)
{
    ERR_FAIL_NULL_V_MSG(L, String(), "Lua state is null. Cannot convert to string.");
    size_t len;
    const char *str = lua_tolstring(L, index, &len);
    return str ? String::utf8(str, len) : String();
}

double LuaState::tonumber(int index)
{
    ERR_FAIL_NULL_V_MSG(L, 0.0, "Lua state is null. Cannot convert to number.");
    return lua_tonumber(L, index);
}

int LuaState::tointeger(int index)
{
    ERR_FAIL_NULL_V_MSG(L, 0, "Lua state is null. Cannot convert to integer.");
    return lua_tointeger(L, index);
}

bool LuaState::toboolean(int index)
{
    ERR_FAIL_NULL_V_MSG(L, false, "Lua state is null. Cannot convert to boolean.");
    return lua_toboolean(L, index) != 0;
}

// Push operations
void LuaState::pushnil()
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot push nil.");
    lua_pushnil(L);
}

void LuaState::pushnumber(double n)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot push number.");
    lua_pushnumber(L, n);
}

void LuaState::pushinteger(int n)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot push integer.");
    lua_pushinteger(L, n);
}

void LuaState::pushstring(const String &s)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot push string.");
    CharString utf8 = s.utf8();
    lua_pushlstring(L, utf8.get_data(), utf8.length());
}

void LuaState::pushboolean(bool b)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot push boolean.");
    lua_pushboolean(L, b ? 1 : 0);
}

// Table operations
void LuaState::newtable()
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot create table.");
    lua_newtable(L);
}

void LuaState::createtable(int narr, int nrec)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot create table.");
    lua_createtable(L, narr, nrec);
}

void LuaState::gettable(int index)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot get table value.");
    lua_gettable(L, index);
}

void LuaState::settable(int index)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot set table value.");
    lua_settable(L, index);
}

void LuaState::getfield(int index, const String &key)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot get field.");
    lua_getfield(L, index, key.utf8());
}

void LuaState::setfield(int index, const String &key)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot set field.");
    lua_setfield(L, index, key.utf8());
}

void LuaState::rawget(int index)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot raw get.");
    lua_rawget(L, index);
}

void LuaState::rawset(int index)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot raw set.");
    lua_rawset(L, index);
}

void LuaState::rawgeti(int index, int n)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot raw get index.");
    lua_rawgeti(L, index, n);
}

void LuaState::rawseti(int index, int n)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot raw set index.");
    lua_rawseti(L, index, n);
}

// Metatable operations
bool LuaState::getmetatable(int index)
{
    ERR_FAIL_NULL_V_MSG(L, false, "Lua state is null. Cannot get metatable.");
    return lua_getmetatable(L, index) != 0;
}

bool LuaState::setmetatable(int index)
{
    ERR_FAIL_NULL_V_MSG(L, false, "Lua state is null. Cannot set metatable.");
    return lua_setmetatable(L, index) != 0;
}

// Function calls
void LuaState::call(int nargs, int nresults)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot call function.");
    lua_call(L, nargs, nresults);
}

lua_Status LuaState::pcall(int nargs, int nresults, int errfunc)
{
    ERR_FAIL_NULL_V_MSG(L, LUA_ERRMEM, "Lua state is null. Cannot pcall function.");
    int status = lua_pcall(L, nargs, nresults, errfunc);
    return static_cast<lua_Status>(status);
}

// Garbage collection
int LuaState::gc(lua_GCOp what, int data)
{
    ERR_FAIL_NULL_V_MSG(L, 0, "Lua state is null. Cannot control GC.");
    return lua_gc(L, what, data);
}
