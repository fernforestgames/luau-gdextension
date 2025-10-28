#include "lua_state.h"
#include "luau.h"
#include "lua_godotlib.h"
#include "lua_callable.h"
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
    const char *error_msg = lua_gettop(L) > 0 ? lua_tostring(L, -1) : "Unknown error";
    ERR_PRINT(vformat("Luau panic! Error %d: %s. LuaState is now invalid and cannot be used further.", errcode, error_msg));

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
    ClassDB::bind_method(D_METHOD("openlibs", "libs"), &LuaState::openlibs, DEFVAL(LuaState::LIB_ALL));
    ClassDB::bind_method(D_METHOD("sandbox"), &LuaState::sandbox);
    ClassDB::bind_method(D_METHOD("close"), &LuaState::close);

    ClassDB::bind_method(D_METHOD("load_bytecode", "bytecode", "chunk_name"), &LuaState::load_bytecode);
    ClassDB::bind_method(D_METHOD("loadstring", "code", "chunk_name"), &LuaState::loadstring);
    ClassDB::bind_method(D_METHOD("dostring", "code", "chunk_name"), &LuaState::dostring);
    ClassDB::bind_method(D_METHOD("resume", "narg"), &LuaState::resume, DEFVAL(0));

    ClassDB::bind_method(D_METHOD("singlestep", "enable"), &LuaState::singlestep);
    ClassDB::bind_method(D_METHOD("pause"), &LuaState::pause);

    // Stack manipulation
    ClassDB::bind_method(D_METHOD("gettop"), &LuaState::gettop);
    ClassDB::bind_method(D_METHOD("settop", "index"), &LuaState::settop);
    ClassDB::bind_method(D_METHOD("checkstack", "extra"), &LuaState::checkstack);
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
    ClassDB::bind_method(D_METHOD("isthread", "index"), &LuaState::isthread);
    ClassDB::bind_method(D_METHOD("isarray", "index"), &LuaState::isarray);
    ClassDB::bind_method(D_METHOD("isdictionary", "index"), &LuaState::isdictionary);
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
    ClassDB::bind_method(D_METHOD("pushthread"), &LuaState::pushthread);

    // Table operations
    ClassDB::bind_method(D_METHOD("newtable"), &LuaState::newtable);
    ClassDB::bind_method(D_METHOD("createtable", "narr", "nrec"), &LuaState::createtable);
    ClassDB::bind_method(D_METHOD("gettable", "index"), &LuaState::gettable);
    ClassDB::bind_method(D_METHOD("settable", "index"), &LuaState::settable);
    ClassDB::bind_method(D_METHOD("getfield", "index", "key"), &LuaState::getfield);
    ClassDB::bind_method(D_METHOD("setfield", "index", "key"), &LuaState::setfield);
    ClassDB::bind_method(D_METHOD("getglobal", "key"), &LuaState::getglobal);
    ClassDB::bind_method(D_METHOD("setglobal", "key"), &LuaState::setglobal);
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

    // Thread operations
    ClassDB::bind_method(D_METHOD("newthread"), &LuaState::newthread);
    ClassDB::bind_method(D_METHOD("tothread", "index"), &LuaState::tothread);
    ClassDB::bind_method(D_METHOD("mainthread"), &LuaState::mainthread);

    // Garbage collection
    ClassDB::bind_method(D_METHOD("gc", "what", "data"), &LuaState::gc);

    // Godot integration helpers
    ClassDB::bind_method(D_METHOD("toarray", "index"), &LuaState::toarray);
    ClassDB::bind_method(D_METHOD("todictionary", "index"), &LuaState::todictionary);
    ClassDB::bind_method(D_METHOD("tovariant", "index"), &LuaState::tovariant);
    ClassDB::bind_method(D_METHOD("pusharray", "value"), &LuaState::pusharray);
    ClassDB::bind_method(D_METHOD("pushdictionary", "value"), &LuaState::pushdictionary);
    ClassDB::bind_method(D_METHOD("pushvariant", "value"), &LuaState::pushvariant);

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

    // Library flags
    BIND_BITFIELD_FLAG(LIB_NONE);
    BIND_BITFIELD_FLAG(LIB_BASE);
    BIND_BITFIELD_FLAG(LIB_COROUTINE);
    BIND_BITFIELD_FLAG(LIB_TABLE);
    BIND_BITFIELD_FLAG(LIB_OS);
    BIND_BITFIELD_FLAG(LIB_STRING);
    BIND_BITFIELD_FLAG(LIB_BIT32);
    BIND_BITFIELD_FLAG(LIB_BUFFER);
    BIND_BITFIELD_FLAG(LIB_UTF8);
    BIND_BITFIELD_FLAG(LIB_MATH);
    BIND_BITFIELD_FLAG(LIB_DEBUG);
    BIND_BITFIELD_FLAG(LIB_VECTOR);
    BIND_BITFIELD_FLAG(LIB_GODOT);
    BIND_BITFIELD_FLAG(LIB_ALL);

    // Pseudo-indices
    BIND_CONSTANT(LUA_REGISTRYINDEX);
    BIND_CONSTANT(LUA_ENVIRONINDEX);
    BIND_CONSTANT(LUA_GLOBALSINDEX);

    ADD_SIGNAL(MethodInfo("step", PropertyInfo(Variant::OBJECT, "state")));
}

LuaState::LuaState()
    : main_thread()
{
    L = luaL_newstate();
    ERR_FAIL_NULL_MSG(L, "Failed to create new Lua state.");

    set_callbacks();
}

// Private constructor for thread states
LuaState::LuaState(lua_State *thread_L, Ref<LuaState> main_thread)
    : L(thread_L), main_thread(main_thread)
{
    ERR_FAIL_NULL_MSG(thread_L, "Thread lua_State* is null.");
    ERR_FAIL_NULL_MSG(main_thread, "Main LuaState is null.");

    set_callbacks();
}

void LuaState::set_callbacks()
{
    lua_Callbacks *callbacks = lua_callbacks(L);
    callbacks->userdata = this;
    callbacks->debugstep = callback_debugstep;
    callbacks->panic = callback_panic;
}

LuaState::~LuaState()
{
    // Only close the main thread
    // Other threads are managed by Lua's GC, not manually closed
    if (main_thread.is_null())
    {
        close();
    }
}

// Helper function to validate stack index is within bounds
static bool is_valid_index(lua_State *L, int index)
{
    if (index == 0)
    {
        return false; // Index 0 is never valid in Lua
    }

    // Pseudo-indices (LUA_REGISTRYINDEX, LUA_ENVIRONINDEX, LUA_GLOBALSINDEX) are always valid
    if (lua_ispseudo(index))
    {
        return true;
    }

    int top = lua_gettop(L);

    // Positive indices must be <= top
    if (index > 0)
    {
        return index <= top;
    }

    // Negative indices: -1 is top, -2 is top-1, etc.
    // Valid range is [-top, -1]
    return index >= -top;
}

// Helper to check if we have at least n items on the stack
static bool has_n_items(lua_State *L, int n)
{
    return lua_gettop(L) >= n;
}

void LuaState::open_library(lua_CFunction func, const char *name)
{
    lua_pushcfunction(L, func, NULL);
    lua_pushstring(L, name);
    lua_call(L, 1, 0);
}

void LuaState::openlibs(int libs)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot open libraries.");

    if (main_thread.is_valid())
    {
        WARN_PRINT("Calling LuaState.openlibs() on a Lua thread will affect all threads in the same VM. Libraries should be opened on the main thread before creating threads.");
    }

    // Open individual libraries based on flags
    if (libs & LIB_BASE)
        open_library(luaopen_base, "");
    if (libs & LIB_COROUTINE)
        open_library(luaopen_coroutine, LUA_COLIBNAME);
    if (libs & LIB_TABLE)
        open_library(luaopen_table, LUA_TABLIBNAME);
    if (libs & LIB_OS)
        open_library(luaopen_os, LUA_OSLIBNAME);
    if (libs & LIB_STRING)
        open_library(luaopen_string, LUA_STRLIBNAME);
    if (libs & LIB_BIT32)
        open_library(luaopen_bit32, LUA_BITLIBNAME);
    if (libs & LIB_BUFFER)
        open_library(luaopen_buffer, LUA_BUFFERLIBNAME);
    if (libs & LIB_UTF8)
        open_library(luaopen_utf8, LUA_UTF8LIBNAME);
    if (libs & LIB_MATH)
        open_library(luaopen_math, LUA_MATHLIBNAME);
    if (libs & LIB_DEBUG)
        open_library(luaopen_debug, LUA_DBLIBNAME);
    if (libs & LIB_VECTOR)
        open_library(luaopen_vector, LUA_VECLIBNAME);
    if (libs & LIB_GODOT)
    {
        // Store LuaState pointer in registry so godot library functions can access it
        // This is needed for callable_call to create LuaCallables from Lua functions
        lua_pushlightuserdata(L, this);
        lua_setfield(L, LUA_REGISTRYINDEX, GDLUAU_STATE_REGISTRY_KEY);
        open_library(luaopen_godot, LUA_GODOTLIBNAME);
    }
}

void LuaState::sandbox()
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot sandbox.");

    if (main_thread.is_valid())
    {
        WARN_PRINT("Calling LuaState.sandbox() on a Lua thread will affect all threads in the same VM. Sandboxing should usually be done on the main thread before creating threads.");
    }

    luaL_sandbox(L);
}

void LuaState::close()
{
    if (!L)
    {
        return;
    }

    if (main_thread.is_valid())
    {
        WARN_PRINT("LuaState.close() should not be called on Lua threads. Threads will be automatically GC'd.");
        L = nullptr;
        main_thread.unref();
        return;
    }

    // This is the main thread - close it
    // This will invalidate all thread lua_State* pointers created from this state
    lua_close(L);
    L = nullptr;
}

lua_Status LuaState::load_bytecode(const PackedByteArray &bytecode, const String &chunk_name)
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), LUA_ERRMEM, "Lua state is invalid. Cannot load bytecode.");

    int status = luau_load(L, chunk_name.utf8(), reinterpret_cast<const char *>(bytecode.ptr()), bytecode.size(), 0);
    return static_cast<lua_Status>(status);
}

lua_Status LuaState::loadstring(const String &code, const String &chunk_name)
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), LUA_ERRMEM, "Lua state is invalid. Cannot load string.");
    return load_bytecode(Luau::compile(code), chunk_name);
}

lua_Status LuaState::dostring(const String &code, const String &chunk_name)
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), LUA_ERRMEM, "Lua state is invalid. Cannot run string.");

    lua_Status status = loadstring(code, chunk_name);
    if (status != LUA_OK)
    {
        return status;
    }

    // Execute the loaded chunk
    if (lua_pcall(L, 0, LUA_MULTRET, 0))
    {
        return LUA_ERRRUN;
    }
    else
    {
        return LUA_OK;
    }
}

lua_Status LuaState::resume(int narg)
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), LUA_ERRMEM, "Lua state is invalid. Cannot resume execution.");

    int status = lua_resume(L, nullptr, narg);
    return static_cast<lua_Status>(status);
}

void LuaState::singlestep(bool enable)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot set singlestep mode.");
    lua_singlestep(L, enable);
}

void LuaState::pause()
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot pause.");
    lua_break(L);
}

void LuaState::getglobal(const String &key)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot get global variable.");
    lua_getglobal(L, key.utf8());
}

void LuaState::setglobal(const String &key)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot set global variable.");

    if (main_thread.is_valid())
    {
        WARN_PRINT("Calling LuaState.setglobal() on a Lua thread will affect all threads in the same VM.");
    }

    lua_setglobal(L, key.utf8());
}

bool LuaState::isarray(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), false, "Lua state is invalid. Cannot check if table is array-like.");
    return is_array(L, index);
}

bool LuaState::isdictionary(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), false, "Lua state is invalid. Cannot check if table is dictionary.");
    return is_dictionary(L, index);
}

Array LuaState::toarray(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), Array(), "Lua state is invalid. Cannot convert to Array.");
    return to_array(L, index);
}

Dictionary LuaState::todictionary(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), Dictionary(), "Lua state is invalid. Cannot convert to Dictionary.");
    return to_dictionary(L, index);
}

Variant LuaState::tovariant(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), Variant(), "Lua state is invalid. Cannot convert to Variant.");
    return to_variant(L, index);
}

void LuaState::pusharray(const Array &arr)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot push Array.");
    push_array(L, arr);
}

void LuaState::pushdictionary(const Dictionary &dict)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot push Dictionary.");
    push_dictionary(L, dict);
}

void LuaState::pushvariant(const Variant &value)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot push Variant.");
    push_variant(L, value);
}

lua_State *LuaState::get_lua_state() const
{
    return L;
}

// Stack manipulation
int LuaState::gettop() const
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), 0, "Lua state is invalid. Cannot get stack top.");
    return lua_gettop(L);
}

void LuaState::settop(int index)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot set stack top.");

    // Validate the index
    // Positive index: must be <= current top (or top will be extended)
    // Negative index: must be valid relative to current top
    // Zero is valid (clears the stack)
    int top = lua_gettop(L);
    ERR_FAIL_COND_MSG(index < 0 && -index > top, vformat("LuaState.settop(%d): Invalid negative index. Stack only has %d elements.", index, top));

    lua_settop(L, index);
}

bool LuaState::checkstack(int size)
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), false, "Lua state is invalid. Cannot manipulate stack size.");
    return lua_checkstack(L, size) != 0;
}

void LuaState::pop(int n)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot pop stack.");

    // Validate we have enough items to pop
    int top = lua_gettop(L);
    ERR_FAIL_COND_MSG(n < 0, vformat("LuaState.pop(%d): Cannot pop negative number of elements.", n));
    ERR_FAIL_COND_MSG(n > top, vformat("LuaState.pop(%d): Stack underflow. Stack only has %d elements.", n, top));

    lua_pop(L, n);
}

void LuaState::pushvalue(int index)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot push value.");
    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.pushvalue(%d): Invalid stack index. Stack has %d elements.", index, lua_gettop(L)));

    lua_pushvalue(L, index);
}

void LuaState::remove(int index)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot remove value.");
    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.remove(%d): Invalid stack index. Stack has %d elements.", index, lua_gettop(L)));

    lua_remove(L, index);
}

void LuaState::insert(int index)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot insert value.");
    ERR_FAIL_COND_MSG(!has_n_items(L, 1), "LuaState.insert(): Stack is empty. Nothing to insert.");

    // For insert, index can be 1 beyond current top (insert at end)
    int top = lua_gettop(L);
    ERR_FAIL_COND_MSG(index == 0 || (index > 0 && index > top + 1) || (index < 0 && -index > top),
                      vformat("LuaState.insert(%d): Invalid stack index. Stack has %d elements.", index, top));

    lua_insert(L, index);
}

void LuaState::replace(int index)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot replace value.");
    ERR_FAIL_COND_MSG(!has_n_items(L, 1), "LuaState.replace(): Stack is empty. Nothing to replace with.");
    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.replace(%d): Invalid stack index. Stack has %d elements.", index, lua_gettop(L)));

    lua_replace(L, index);
}

// Type checking
bool LuaState::isnil(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), false, "Lua state is invalid. Cannot check type.");
    return lua_isnil(L, index);
}

bool LuaState::isnumber(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), false, "Lua state is invalid. Cannot check type.");
    return lua_isnumber(L, index) != 0;
}

bool LuaState::isstring(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), false, "Lua state is invalid. Cannot check type.");
    return lua_isstring(L, index) != 0;
}

bool LuaState::istable(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), false, "Lua state is invalid. Cannot check type.");
    return lua_istable(L, index);
}

bool LuaState::isfunction(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), false, "Lua state is invalid. Cannot check type.");
    return lua_isfunction(L, index);
}

bool LuaState::isuserdata(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), false, "Lua state is invalid. Cannot check type.");
    return lua_isuserdata(L, index) != 0;
}

bool LuaState::isboolean(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), false, "Lua state is invalid. Cannot check type.");
    return lua_isboolean(L, index);
}

bool LuaState::isthread(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), false, "Lua state is invalid. Cannot check type.");
    return lua_isthread(L, index);
}

int LuaState::type(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), LUA_TNONE, "Lua state is invalid. Cannot get type.");
    return lua_type(L, index);
}

String LuaState::type_name(int type_id) const
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), String(), "Lua state is invalid. Cannot get type name.");
    return String(lua_typename(L, type_id));
}

// Value access
String LuaState::tostring(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), String(), "Lua state is invalid. Cannot convert to string.");
    return godot::to_string(L, index);
}

double LuaState::tonumber(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), 0.0, "Lua state is invalid. Cannot convert to number.");
    return lua_tonumber(L, index);
}

int LuaState::tointeger(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), 0, "Lua state is invalid. Cannot convert to integer.");
    return lua_tointeger(L, index);
}

bool LuaState::toboolean(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), false, "Lua state is invalid. Cannot convert to boolean.");
    return lua_toboolean(L, index) != 0;
}

bool LuaState::is_valid_state() const
{
    if (!L)
    {
        return false;
    }

    // If this is a thread, check if parent is still valid
    if (main_thread.is_valid())
    {
        return main_thread->get_lua_state() != nullptr;
    }

    return true;
}

// Push operations
void LuaState::pushnil()
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot push nil.");
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.pushnil(): Stack overflow. Cannot grow stack.");
    lua_pushnil(L);
}

void LuaState::pushnumber(double n)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot push number.");
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.pushnumber(): Stack overflow. Cannot grow stack.");
    lua_pushnumber(L, n);
}

void LuaState::pushinteger(int n)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot push integer.");
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.pushinteger(): Stack overflow. Cannot grow stack.");
    lua_pushinteger(L, n);
}

void LuaState::pushstring(const String &s)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot push string.");
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.pushstring(): Stack overflow. Cannot grow stack.");
    push_string(L, s);
}

void LuaState::pushboolean(bool b)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot push boolean.");
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.pushboolean(): Stack overflow. Cannot grow stack.");
    lua_pushboolean(L, b ? 1 : 0);
}

bool LuaState::pushthread()
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), false, "Lua state is invalid. Cannot push thread.");
    ERR_FAIL_COND_V_MSG(!lua_checkstack(L, 1), false, "LuaState.pushthread(): Stack overflow. Cannot grow stack.");

    bool expected_main_thread = main_thread.is_null();
    bool is_main_thread = lua_pushthread(L) != 0;
    if (is_main_thread != expected_main_thread)
    {
        ERR_PRINT("LuaState.pushthread() inconsistency: Lua and GDExtension disagree about which is the main thread.");
    }

    return is_main_thread;
}

// Table operations
void LuaState::newtable()
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot create table.");
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.newtable(): Stack overflow. Cannot grow stack.");
    lua_newtable(L);
}

void LuaState::createtable(int narr, int nrec)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot create table.");
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.createtable(): Stack overflow. Cannot grow stack.");
    lua_createtable(L, narr, nrec);
}

void LuaState::gettable(int index)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot get table value.");

    int top = lua_gettop(L);
    ERR_FAIL_COND_MSG(top < 1, "LuaState.gettable(): Stack is empty. Need a key on the stack.");

    // The table index must not be the top item (the key at -1)
    // gettable pops the key and pushes the value, so -1 is replaced
    ERR_FAIL_COND_MSG(index == -1 || index == top, vformat("LuaState.gettable(%d): Table index cannot be the key (at index %d).", index, top));

    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.gettable(%d): Invalid table index. Stack has %d elements.", index, top));

    lua_gettable(L, index);
}

void LuaState::settable(int index)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot set table value.");

    int top = lua_gettop(L);
    ERR_FAIL_COND_MSG(top < 2, vformat("LuaState.settable(): Need key and value on stack. Stack has %d elements.", top));

    // The table index must not be the top two items (key and value at -2 and -1)
    // After settable consumes them, those indices would be invalid
    ERR_FAIL_COND_MSG(index == -1 || index == top, vformat("LuaState.settable(%d): Table index cannot be the value (at index %d).", index, top));
    ERR_FAIL_COND_MSG(index == -2 || index == top - 1, vformat("LuaState.settable(%d): Table index cannot be the key (at index %d).", index, top - 1));

    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.settable(%d): Invalid table index. Stack has %d elements.", index, top));

    lua_settable(L, index);
}

void LuaState::getfield(int index, const String &key)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot get field.");
    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.getfield(%d, \"%s\"): Invalid table index. Stack has %d elements.", index, key, lua_gettop(L)));

    lua_getfield(L, index, key.utf8());
}

void LuaState::setfield(int index, const String &key)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot set field.");

    int top = lua_gettop(L);
    ERR_FAIL_COND_MSG(top < 1, vformat("LuaState.setfield(): Need value on stack. Stack has %d elements.", top));

    // The table index must not be the top item (the value at -1) since setfield pops it
    ERR_FAIL_COND_MSG(index == -1 || index == top, vformat("LuaState.setfield(%d, \"%s\"): Table index cannot be the value (at index %d).", index, key, top));

    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.setfield(%d, \"%s\"): Invalid table index. Stack has %d elements.", index, key, top));

    lua_setfield(L, index, key.utf8());
}

void LuaState::rawget(int index)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot raw get.");

    int top = lua_gettop(L);
    ERR_FAIL_COND_MSG(top < 1, "LuaState.rawget(): Stack is empty. Need a key on the stack.");

    // The table index must not be the top item (the key at -1)
    ERR_FAIL_COND_MSG(index == -1 || index == top, vformat("LuaState.rawget(%d): Table index cannot be the key (at index %d).", index, top));

    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.rawget(%d): Invalid table index. Stack has %d elements.", index, top));

    lua_rawget(L, index);
}

void LuaState::rawset(int index)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot raw set.");

    int top = lua_gettop(L);
    ERR_FAIL_COND_MSG(top < 2, vformat("LuaState.rawset(): Need key and value on stack. Stack has %d elements.", top));

    // The table index must not be the top two items (key and value at -2 and -1)
    ERR_FAIL_COND_MSG(index == -1 || index == top, vformat("LuaState.rawset(%d): Table index cannot be the value (at index %d).", index, top));
    ERR_FAIL_COND_MSG(index == -2 || index == top - 1, vformat("LuaState.rawset(%d): Table index cannot be the key (at index %d).", index, top - 1));

    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.rawset(%d): Invalid table index. Stack has %d elements.", index, top));

    lua_rawset(L, index);
}

void LuaState::rawgeti(int index, int n)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot raw get index.");
    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.rawgeti(%d, %d): Invalid table index. Stack has %d elements.", index, n, lua_gettop(L)));

    lua_rawgeti(L, index, n);
}

void LuaState::rawseti(int index, int n)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot raw set index.");

    int top = lua_gettop(L);
    ERR_FAIL_COND_MSG(top < 1, vformat("LuaState.rawseti(): Need value on stack. Stack has %d elements.", top));

    // The table index must not be the top item (the value at -1) since rawseti pops it
    ERR_FAIL_COND_MSG(index == -1 || index == top, vformat("LuaState.rawseti(%d, %d): Table index cannot be the value (at index %d).", index, n, top));

    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.rawseti(%d, %d): Invalid table index. Stack has %d elements.", index, n, top));

    lua_rawseti(L, index, n);
}

// Metatable operations
bool LuaState::getmetatable(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), false, "Lua state is invalid. Cannot get metatable.");
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, index), false, vformat("LuaState.getmetatable(%d): Invalid stack index. Stack has %d elements.", index, lua_gettop(L)));

    return lua_getmetatable(L, index) != 0;
}

bool LuaState::setmetatable(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), false, "Lua state is invalid. Cannot set metatable.");

    int top = lua_gettop(L);
    ERR_FAIL_COND_V_MSG(top < 1, false, "LuaState.setmetatable(): Stack is empty. Need a metatable on the stack.");

    // The table/object index must not be the top item (the metatable at -1) since setmetatable pops it
    ERR_FAIL_COND_V_MSG(index == -1 || index == top, false, vformat("LuaState.setmetatable(%d): Index cannot be the metatable (at index %d).", index, top));

    ERR_FAIL_COND_V_MSG(!is_valid_index(L, index), false, vformat("LuaState.setmetatable(%d): Invalid stack index. Stack has %d elements.", index, top));

    return lua_setmetatable(L, index) != 0;
}

// Function calls
void LuaState::call(int nargs, int nresults)
{
    ERR_FAIL_COND_MSG(!is_valid_state(), "Lua state is invalid. Cannot call function.");
    ERR_FAIL_COND_MSG(nargs < 0, vformat("LuaState.call(%d, %d): nargs cannot be negative.", nargs, nresults));
    ERR_FAIL_COND_MSG(!has_n_items(L, nargs + 1), vformat("LuaState.call(%d, %d): Need function + %d arguments on stack. Stack has %d elements.", nargs, nresults, nargs, lua_gettop(L)));

    lua_call(L, nargs, nresults);
}

lua_Status LuaState::pcall(int nargs, int nresults, int errfunc)
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), LUA_ERRMEM, "Lua state is invalid. Cannot pcall function.");
    ERR_FAIL_COND_V_MSG(nargs < 0, LUA_ERRMEM, vformat("LuaState.pcall(%d, %d, %d): nargs cannot be negative.", nargs, nresults, errfunc));
    ERR_FAIL_COND_V_MSG(!has_n_items(L, nargs + 1), LUA_ERRMEM, vformat("LuaState.pcall(%d, %d, %d): Need function + %d arguments on stack. Stack has %d elements.", nargs, nresults, errfunc, nargs, lua_gettop(L)));
    ERR_FAIL_COND_V_MSG(errfunc != 0 && !is_valid_index(L, errfunc), LUA_ERRMEM, vformat("LuaState.pcall(%d, %d, %d): Invalid error function index. Stack has %d elements.", nargs, nresults, errfunc, lua_gettop(L)));

    int status = lua_pcall(L, nargs, nresults, errfunc);
    return static_cast<lua_Status>(status);
}

// Thread operations
Ref<LuaState> LuaState::bind_thread(lua_State *thread_L) const
{
    ERR_FAIL_NULL_V_MSG(thread_L, Ref<LuaState>(), "Failed to bind thread.");

    Ref<LuaState> thread_state = memnew(LuaState(thread_L, main_thread.is_valid() ? main_thread : Ref<LuaState>(this)));
    return thread_state;
}

Ref<LuaState> LuaState::newthread()
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), Ref<LuaState>(), "Lua state is invalid. Cannot create thread.");

    // Create a new thread and push it onto the stack
    lua_State *thread_L = lua_newthread(L);
    ERR_FAIL_NULL_V_MSG(thread_L, Ref<LuaState>(), "Failed to create new Lua thread.");

    return bind_thread(thread_L);
}

Ref<LuaState> LuaState::tothread(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), Ref<LuaState>(), "Lua state is invalid. Cannot convert to thread.");
    ERR_FAIL_COND_V_MSG(!isthread(index), Ref<LuaState>(), "Stack value at index is not a thread.");

    lua_State *thread_L = lua_tothread(L, index);
    ERR_FAIL_NULL_V_MSG(thread_L, Ref<LuaState>(), "Failed to get thread lua_State*.");

    return bind_thread(thread_L);
}

Ref<LuaState> LuaState::mainthread()
{
    return main_thread.is_valid() ? main_thread : Ref<LuaState>(this);
}

// Garbage collection
int LuaState::gc(lua_GCOp what, int data)
{
    ERR_FAIL_COND_V_MSG(!is_valid_state(), 0, "Lua state is invalid. Cannot control GC.");
    return lua_gc(L, what, data);
}
