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

    ClassDB::bind_method(D_METHOD("getglobal", "key"), &LuaState::getglobal);
    ClassDB::bind_method(D_METHOD("to_variant", "index"), &LuaState::to_variant);
    ClassDB::bind_method(D_METHOD("push_variant", "value"), &LuaState::push_variant);

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

Variant LuaState::to_variant(int index)
{
    ERR_FAIL_NULL_V_MSG(L, Variant(), "Lua state is null. Cannot convert to Variant.");

    int type = lua_type(L, index);

    switch (type)
    {
    case LUA_TNIL:
        return Variant();

    case LUA_TBOOLEAN:
        return Variant(lua_toboolean(L, index) != 0);

    case LUA_TNUMBER:
        return Variant(lua_tonumber(L, index));

    case LUA_TSTRING:
    {
        size_t len;
        const char *str = lua_tolstring(L, index, &len);
        return Variant(String::utf8(str, len));
    }

    case LUA_TTABLE:
    {
        Dictionary dict;

        // Push nil as the first key for lua_next
        lua_pushnil(L);

        // Iterate over the table
        while (lua_next(L, index < 0 ? index - 1 : index) != 0)
        {
            // Key is at -2, value is at -1
            Variant key = to_variant(-2);
            Variant value = to_variant(-1);

            dict[key] = value;

            // Remove value, keep key for next iteration
            lua_pop(L, 1);
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
        ERR_PRINT(vformat("Unknown Lua type %d at index %d.", type, index));
        return Variant();
    }
}

void LuaState::push_variant(const Variant &value)
{
    ERR_FAIL_NULL_MSG(L, "Lua state is null. Cannot push Variant.");

    Variant::Type type = value.get_type();

    switch (type)
    {
    case Variant::NIL:
        lua_pushnil(L);
        break;

    case Variant::BOOL:
        lua_pushboolean(L, static_cast<bool>(value));
        break;

    case Variant::INT:
        lua_pushinteger(L, static_cast<int>(value));
        break;

    case Variant::FLOAT:
        lua_pushnumber(L, static_cast<double>(value));
        break;

    case Variant::STRING:
    case Variant::STRING_NAME:
    {
        String str = value;
        CharString utf8 = str.utf8();
        lua_pushlstring(L, utf8.get_data(), utf8.length());
        break;
    }

    case Variant::ARRAY:
    {
        Array arr = value;
        lua_createtable(L, arr.size(), 0);

        // Lua arrays are 1-indexed
        for (int i = 0; i < arr.size(); i++)
        {
            push_variant(arr[i]);
            lua_rawseti(L, -2, i + 1);
        }
        break;
    }

    case Variant::DICTIONARY:
    {
        Dictionary dict = value;
        lua_createtable(L, 0, dict.size());

        Array keys = dict.keys();
        for (int i = 0; i < keys.size(); i++)
        {
            Variant key = keys[i];
            Variant val = dict[key];

            // Push key and value
            push_variant(key);
            push_variant(val);

            // Set table[key] = value
            lua_settable(L, -3);
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
            CharString utf8 = str.utf8();
            lua_pushlstring(L, utf8.get_data(), utf8.length());
            ERR_PRINT(vformat("Variant math type %d not yet supported. Converted to string: %s", type, str));
        }
        break;

    default:
    {
        // For unsupported types, convert to string representation
        String str = value;
        CharString utf8 = str.utf8();
        lua_pushlstring(L, utf8.get_data(), utf8.length());
        ERR_PRINT(vformat("Variant type %d not directly supported. Converted to string: %s", type, str));
    }
    }
}
