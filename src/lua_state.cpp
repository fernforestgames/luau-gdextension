#include "lua_state.h"
#include "luau.h"
#include "lua_godotlib.h"
#include "lua_callable.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/object.hpp>
#include <lualib.h>

using namespace godot;

static void callback_interrupt(lua_State *L, int gc)
{
    LuaState *state = static_cast<LuaState *>(lua_getthreaddata(L));
    state->emit_signal("interrupt", state, gc);
}

// This handler is called when Lua encounters an unprotected error.
// If we don't handle this, Luau will longjmp across Godot's stack frames,
// causing resource leaks and potential crashes.
static void callback_panic(lua_State *L, int errcode)
{
    const char *error_msg = lua_gettop(L) > 0 ? lua_tostring(L, -1) : "Unknown error";
    ERR_PRINT(vformat("Luau panic! Error %d: %s. LuaState is now invalid and cannot be used further.", errcode, error_msg));

    LuaState *state = static_cast<LuaState *>(lua_getthreaddata(L));
    state->close();
}

static void callback_debugstep(lua_State *L, lua_Debug *ar)
{
    LuaState *state = static_cast<LuaState *>(lua_getthreaddata(L));
    state->emit_signal("debugstep", state);
}

void LuaState::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("open_libs", "libs"), &LuaState::open_libs, DEFVAL(LuaState::LIB_ALL));
    ClassDB::bind_method(D_METHOD("sandbox"), &LuaState::sandbox);
    ClassDB::bind_method(D_METHOD("sandbox_thread"), &LuaState::sandbox_thread);
    ClassDB::bind_method(D_METHOD("close"), &LuaState::close);
    ClassDB::bind_method(D_METHOD("is_valid"), &LuaState::is_valid);

    ClassDB::bind_method(D_METHOD("load_bytecode", "bytecode", "chunk_name"), &LuaState::load_bytecode);
    ClassDB::bind_method(D_METHOD("load_string", "code", "chunk_name"), &LuaState::load_string);
    ClassDB::bind_method(D_METHOD("do_string", "code", "chunk_name"), &LuaState::do_string);
    ClassDB::bind_method(D_METHOD("resume", "narg"), &LuaState::resume, DEFVAL(0));

    ClassDB::bind_method(D_METHOD("status"), &LuaState::status);
    ClassDB::bind_method(D_METHOD("single_step", "enable"), &LuaState::single_step);
    ClassDB::bind_method(D_METHOD("yield", "nresults"), &LuaState::yield);
    ClassDB::bind_method(D_METHOD("is_yieldable"), &LuaState::is_yieldable);
    ClassDB::bind_method(D_METHOD("pause"), &LuaState::pause);

    // Stack manipulation
    ClassDB::bind_method(D_METHOD("abs_index", "index"), &LuaState::abs_index);
    ClassDB::bind_method(D_METHOD("get_top"), &LuaState::get_top);
    ClassDB::bind_method(D_METHOD("set_top", "index"), &LuaState::set_top);
    ClassDB::bind_method(D_METHOD("check_stack", "extra"), &LuaState::check_stack);
    ClassDB::bind_method(D_METHOD("pop", "n"), &LuaState::pop);
    ClassDB::bind_method(D_METHOD("push_value", "index"), &LuaState::push_value);
    ClassDB::bind_method(D_METHOD("remove", "index"), &LuaState::remove);
    ClassDB::bind_method(D_METHOD("insert", "index"), &LuaState::insert);
    ClassDB::bind_method(D_METHOD("replace", "index"), &LuaState::replace);

    // Type checking
    ClassDB::bind_method(D_METHOD("is_nil", "index"), &LuaState::is_nil);
    ClassDB::bind_method(D_METHOD("is_number", "index"), &LuaState::is_number);
    ClassDB::bind_method(D_METHOD("is_string", "index"), &LuaState::is_string);
    ClassDB::bind_method(D_METHOD("is_table", "index"), &LuaState::is_table);
    ClassDB::bind_method(D_METHOD("is_function", "index"), &LuaState::is_function);
    ClassDB::bind_method(D_METHOD("is_userdata", "index"), &LuaState::is_userdata);
    ClassDB::bind_method(D_METHOD("is_boolean", "index"), &LuaState::is_boolean);
    ClassDB::bind_method(D_METHOD("is_thread", "index"), &LuaState::is_thread);
    ClassDB::bind_method(D_METHOD("is_array", "index"), &LuaState::is_array);
    ClassDB::bind_method(D_METHOD("is_dictionary", "index"), &LuaState::is_dictionary);
    ClassDB::bind_method(D_METHOD("type", "index"), &LuaState::type);
    ClassDB::bind_method(D_METHOD("type_name", "type_id"), &LuaState::type_name);

    // Value access
    ClassDB::bind_method(D_METHOD("to_string", "index"), &LuaState::to_string);
    ClassDB::bind_method(D_METHOD("to_number", "index"), &LuaState::to_number);
    ClassDB::bind_method(D_METHOD("to_integer", "index"), &LuaState::to_integer);
    ClassDB::bind_method(D_METHOD("to_boolean", "index"), &LuaState::to_boolean);
    ClassDB::bind_method(D_METHOD("obj_len", "index"), &LuaState::obj_len);

    // Comparisons
    ClassDB::bind_method(D_METHOD("equal", "index1", "index2"), &LuaState::equal);
    ClassDB::bind_method(D_METHOD("raw_equal", "index1", "index2"), &LuaState::raw_equal);
    ClassDB::bind_method(D_METHOD("less_than", "index1", "index2"), &LuaState::less_than);

    // Push operations
    ClassDB::bind_method(D_METHOD("push_nil"), &LuaState::push_nil);
    ClassDB::bind_method(D_METHOD("push_number", "n"), &LuaState::push_number);
    ClassDB::bind_method(D_METHOD("push_integer", "n"), &LuaState::push_integer);
    ClassDB::bind_method(D_METHOD("push_string", "s"), &LuaState::push_string);
    ClassDB::bind_method(D_METHOD("push_boolean", "b"), &LuaState::push_boolean);
    ClassDB::bind_method(D_METHOD("push_thread"), &LuaState::push_thread);

    // Table operations
    ClassDB::bind_method(D_METHOD("new_table"), &LuaState::new_table);
    ClassDB::bind_method(D_METHOD("create_table", "narr", "nrec"), &LuaState::create_table);
    ClassDB::bind_method(D_METHOD("get_table", "index"), &LuaState::get_table);
    ClassDB::bind_method(D_METHOD("set_table", "index"), &LuaState::set_table);
    ClassDB::bind_method(D_METHOD("get_field", "index", "key"), &LuaState::get_field);
    ClassDB::bind_method(D_METHOD("set_field", "index", "key"), &LuaState::set_field);
    ClassDB::bind_method(D_METHOD("get_global", "key"), &LuaState::get_global);
    ClassDB::bind_method(D_METHOD("set_global", "key"), &LuaState::set_global);
    ClassDB::bind_method(D_METHOD("raw_get", "index"), &LuaState::raw_get);
    ClassDB::bind_method(D_METHOD("raw_set", "index"), &LuaState::raw_set);
    ClassDB::bind_method(D_METHOD("raw_get_field", "index", "key"), &LuaState::raw_get_field);
    ClassDB::bind_method(D_METHOD("raw_set_field", "index", "key"), &LuaState::raw_set_field);
    ClassDB::bind_method(D_METHOD("raw_geti", "index", "n"), &LuaState::raw_geti);
    ClassDB::bind_method(D_METHOD("raw_seti", "index", "n"), &LuaState::raw_seti);
    ClassDB::bind_method(D_METHOD("get_read_only", "index"), &LuaState::get_read_only);
    ClassDB::bind_method(D_METHOD("set_read_only", "index", "read_only"), &LuaState::set_read_only);
    ClassDB::bind_method(D_METHOD("get_fenv", "index"), &LuaState::get_fenv);
    ClassDB::bind_method(D_METHOD("set_fenv", "index"), &LuaState::set_fenv);
    ClassDB::bind_method(D_METHOD("next", "index"), &LuaState::next);

    // Metatable operations
    ClassDB::bind_method(D_METHOD("get_metatable", "index"), &LuaState::get_metatable);
    ClassDB::bind_method(D_METHOD("set_metatable", "index"), &LuaState::set_metatable);

    // Function calls
    ClassDB::bind_method(D_METHOD("call", "nargs", "nresults"), &LuaState::call);
    ClassDB::bind_method(D_METHOD("pcall", "nargs", "nresults", "errfunc"), &LuaState::pcall);

    // Thread operations
    ClassDB::bind_method(D_METHOD("new_thread"), &LuaState::new_thread);
    ClassDB::bind_method(D_METHOD("to_thread", "index"), &LuaState::to_thread);
    ClassDB::bind_method(D_METHOD("get_main_thread"), &LuaState::get_main_thread);
    ClassDB::bind_method(D_METHOD("reset_thread"), &LuaState::reset_thread);
    ClassDB::bind_method(D_METHOD("is_thread_reset"), &LuaState::is_thread_reset);
    ClassDB::bind_method(D_METHOD("xmove", "to_state", "n"), &LuaState::xmove);
    ClassDB::bind_method(D_METHOD("xpush", "to_state", "index"), &LuaState::xpush);
    ClassDB::bind_method(D_METHOD("co_status", "co"), &LuaState::co_status);

    // Garbage collection
    ClassDB::bind_method(D_METHOD("gc", "what", "data"), &LuaState::gc);
    ClassDB::bind_method(D_METHOD("ref", "index"), &LuaState::ref);
    ClassDB::bind_method(D_METHOD("get_ref", "ref"), &LuaState::get_ref);
    ClassDB::bind_method(D_METHOD("unref", "ref"), &LuaState::unref);

    // Godot integration helpers
    ClassDB::bind_method(D_METHOD("to_array", "index"), &LuaState::to_array);
    ClassDB::bind_method(D_METHOD("to_dictionary", "index"), &LuaState::to_dictionary);
    ClassDB::bind_method(D_METHOD("to_variant", "index"), &LuaState::to_variant);
    ClassDB::bind_method(D_METHOD("push_array", "value"), &LuaState::push_array);
    ClassDB::bind_method(D_METHOD("push_dictionary", "value"), &LuaState::push_dictionary);
    ClassDB::bind_method(D_METHOD("push_variant", "value"), &LuaState::push_variant);

    // Godot math type wrappers - Push operations
    ClassDB::bind_method(D_METHOD("push_vector2", "value"), &LuaState::push_vector2);
    ClassDB::bind_method(D_METHOD("push_vector2i", "value"), &LuaState::push_vector2i);
    ClassDB::bind_method(D_METHOD("push_vector3", "value"), &LuaState::push_vector3);
    ClassDB::bind_method(D_METHOD("push_vector3i", "value"), &LuaState::push_vector3i);
    ClassDB::bind_method(D_METHOD("push_vector4", "value"), &LuaState::push_vector4);
    ClassDB::bind_method(D_METHOD("push_vector4i", "value"), &LuaState::push_vector4i);
    ClassDB::bind_method(D_METHOD("push_rect2", "value"), &LuaState::push_rect2);
    ClassDB::bind_method(D_METHOD("push_rect2i", "value"), &LuaState::push_rect2i);
    ClassDB::bind_method(D_METHOD("push_aabb", "value"), &LuaState::push_aabb);
    ClassDB::bind_method(D_METHOD("push_color", "value"), &LuaState::push_color);
    ClassDB::bind_method(D_METHOD("push_plane", "value"), &LuaState::push_plane);
    ClassDB::bind_method(D_METHOD("push_quaternion", "value"), &LuaState::push_quaternion);
    ClassDB::bind_method(D_METHOD("push_basis", "value"), &LuaState::push_basis);
    ClassDB::bind_method(D_METHOD("push_transform2d", "value"), &LuaState::push_transform2d);
    ClassDB::bind_method(D_METHOD("push_transform3d", "value"), &LuaState::push_transform3d);
    ClassDB::bind_method(D_METHOD("push_projection", "value"), &LuaState::push_projection);
    ClassDB::bind_method(D_METHOD("push_callable", "value"), &LuaState::push_callable);

    // Godot math type wrappers - Type checking operations
    ClassDB::bind_method(D_METHOD("is_vector2", "index"), &LuaState::is_vector2);
    ClassDB::bind_method(D_METHOD("is_vector2i", "index"), &LuaState::is_vector2i);
    ClassDB::bind_method(D_METHOD("is_vector3", "index"), &LuaState::is_vector3);
    ClassDB::bind_method(D_METHOD("is_vector3i", "index"), &LuaState::is_vector3i);
    ClassDB::bind_method(D_METHOD("is_vector4", "index"), &LuaState::is_vector4);
    ClassDB::bind_method(D_METHOD("is_vector4i", "index"), &LuaState::is_vector4i);
    ClassDB::bind_method(D_METHOD("is_rect2", "index"), &LuaState::is_rect2);
    ClassDB::bind_method(D_METHOD("is_rect2i", "index"), &LuaState::is_rect2i);
    ClassDB::bind_method(D_METHOD("is_aabb", "index"), &LuaState::is_aabb);
    ClassDB::bind_method(D_METHOD("is_color", "index"), &LuaState::is_color);
    ClassDB::bind_method(D_METHOD("is_plane", "index"), &LuaState::is_plane);
    ClassDB::bind_method(D_METHOD("is_quaternion", "index"), &LuaState::is_quaternion);
    ClassDB::bind_method(D_METHOD("is_basis", "index"), &LuaState::is_basis);
    ClassDB::bind_method(D_METHOD("is_transform2d", "index"), &LuaState::is_transform2d);
    ClassDB::bind_method(D_METHOD("is_transform3d", "index"), &LuaState::is_transform3d);
    ClassDB::bind_method(D_METHOD("is_projection", "index"), &LuaState::is_projection);
    ClassDB::bind_method(D_METHOD("is_callable", "index"), &LuaState::is_callable);

    // Godot math type wrappers - Conversion operations
    ClassDB::bind_method(D_METHOD("to_vector2", "index"), &LuaState::to_vector2);
    ClassDB::bind_method(D_METHOD("to_vector2i", "index"), &LuaState::to_vector2i);
    ClassDB::bind_method(D_METHOD("to_vector3", "index"), &LuaState::to_vector3);
    ClassDB::bind_method(D_METHOD("to_vector3i", "index"), &LuaState::to_vector3i);
    ClassDB::bind_method(D_METHOD("to_vector4", "index"), &LuaState::to_vector4);
    ClassDB::bind_method(D_METHOD("to_vector4i", "index"), &LuaState::to_vector4i);
    ClassDB::bind_method(D_METHOD("to_rect2", "index"), &LuaState::to_rect2);
    ClassDB::bind_method(D_METHOD("to_rect2i", "index"), &LuaState::to_rect2i);
    ClassDB::bind_method(D_METHOD("to_aabb", "index"), &LuaState::to_aabb);
    ClassDB::bind_method(D_METHOD("to_color", "index"), &LuaState::to_color);
    ClassDB::bind_method(D_METHOD("to_plane", "index"), &LuaState::to_plane);
    ClassDB::bind_method(D_METHOD("to_quaternion", "index"), &LuaState::to_quaternion);
    ClassDB::bind_method(D_METHOD("to_basis", "index"), &LuaState::to_basis);
    ClassDB::bind_method(D_METHOD("to_transform2d", "index"), &LuaState::to_transform2d);
    ClassDB::bind_method(D_METHOD("to_transform3d", "index"), &LuaState::to_transform3d);
    ClassDB::bind_method(D_METHOD("to_projection", "index"), &LuaState::to_projection);
    ClassDB::bind_method(D_METHOD("to_callable", "index"), &LuaState::to_callable);

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

    ADD_SIGNAL(MethodInfo("interrupt", PropertyInfo(Variant::OBJECT, "state"), PropertyInfo(Variant::INT, "gc_state")));
    ADD_SIGNAL(MethodInfo("debugstep", PropertyInfo(Variant::OBJECT, "state")));
}

LuaState::LuaState()
    : main_thread()
{
    L = luaL_newstate();
    ERR_FAIL_NULL_MSG(L, "Failed to create new Lua state.");

    lua_setthreaddata(L, this);
    set_callbacks();
}

// Private constructor for thread states
LuaState::LuaState(lua_State *thread_L, Ref<LuaState> main_thread)
    : L(thread_L), main_thread(main_thread)
{
    ERR_FAIL_NULL_MSG(thread_L, "Thread lua_State* is null.");
    ERR_FAIL_NULL_MSG(main_thread, "Main LuaState is null.");

    lua_setthreaddata(L, this);
}

void LuaState::set_callbacks()
{
    // NB: Callbacks are shared among all threads in the same Lua VM
    lua_Callbacks *callbacks = lua_callbacks(L);
    callbacks->interrupt = callback_interrupt;
    callbacks->panic = callback_panic;
    callbacks->debugstep = callback_debugstep;
}

LuaState::~LuaState()
{
    // Only close the main thread
    // Other threads are managed by Lua's GC, not manually closed
    if (main_thread.is_null() && L)
    {
        lua_close(L);
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

void LuaState::open_libs(int libs)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot open libraries.");

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
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot sandbox.");

    if (main_thread.is_valid())
    {
        WARN_PRINT("Calling LuaState.sandbox() on a Lua thread will affect all threads in the same VM. Sandboxing should usually be done on the main thread before creating threads.");
    }

    luaL_sandbox(L);
}

void LuaState::sandbox_thread()
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot sandbox thread.");
    ERR_FAIL_COND_MSG(!main_thread.is_valid(), "LuaState.sandbox_thread() can only be called on Lua threads, not the main thread.");

    luaL_sandboxthread(L);
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
    ERR_FAIL_COND_V_MSG(!is_valid(), LUA_ERRMEM, "Lua state is invalid. Cannot load bytecode.");

    int status = luau_load(L, chunk_name.utf8(), reinterpret_cast<const char *>(bytecode.ptr()), bytecode.size(), 0);
    return static_cast<lua_Status>(status);
}

lua_Status LuaState::load_string(const String &code, const String &chunk_name)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), LUA_ERRMEM, "Lua state is invalid. Cannot load string.");
    return load_bytecode(Luau::compile(code), chunk_name);
}

lua_Status LuaState::do_string(const String &code, const String &chunk_name)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), LUA_ERRMEM, "Lua state is invalid. Cannot run string.");

    lua_Status status = load_string(code, chunk_name);
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
    ERR_FAIL_COND_V_MSG(!is_valid(), LUA_ERRMEM, "Lua state is invalid. Cannot resume execution.");

    int status = lua_resume(L, nullptr, narg);
    return static_cast<lua_Status>(status);
}

lua_Status LuaState::status() const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), LUA_ERRMEM, "Lua state is invalid. Cannot get status.");
    return static_cast<lua_Status>(lua_status(L));
}

void LuaState::single_step(bool enable)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot set singlestep mode.");
    lua_singlestep(L, enable);
}

void LuaState::yield(int nresults)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot yield.");
    ERR_FAIL_COND_MSG(!has_n_items(L, nresults), "LuaState.yield(): Not enough values on the stack to yield.");
    lua_yield(L, nresults);
}

bool LuaState::is_yieldable() const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check if yieldable.");
    return lua_isyieldable(L);
}

void LuaState::pause()
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot pause.");
    lua_break(L);
}

void LuaState::get_global(const String &key)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot get global variable.");
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.getglobal(): Stack overflow. Cannot grow stack.");
    lua_getglobal(L, key.utf8());
}

void LuaState::set_global(const String &key)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot set global variable.");

    if (main_thread.is_valid())
    {
        WARN_PRINT("Calling LuaState.set_global() on a Lua thread will affect all threads in the same VM.");
    }

    lua_setglobal(L, key.utf8());
}

bool LuaState::is_array(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check if table is array-like.");
    return godot::is_array(L, index);
}

bool LuaState::is_dictionary(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check if table is dictionary.");
    return godot::is_dictionary(L, index);
}

Array LuaState::to_array(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Array(), "Lua state is invalid. Cannot convert to Array.");
    return godot::to_array(L, index);
}

Dictionary LuaState::to_dictionary(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Dictionary(), "Lua state is invalid. Cannot convert to Dictionary.");
    return godot::to_dictionary(L, index);
}

Variant LuaState::to_variant(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Variant(), "Lua state is invalid. Cannot convert to Variant.");
    return godot::to_variant(L, index);
}

void LuaState::push_array(const Array &arr)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push Array.");
    godot::push_array(L, arr);
}

void LuaState::push_dictionary(const Dictionary &dict)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push Dictionary.");
    godot::push_dictionary(L, dict);
}

void LuaState::push_variant(const Variant &value)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push Variant.");
    godot::push_variant(L, value);
}

// Godot math type wrappers - Push operations
void LuaState::push_vector2(const Vector2 &value)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push Vector2.");
    godot::push_vector2(L, value);
}

void LuaState::push_vector2i(const Vector2i &value)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push Vector2i.");
    godot::push_vector2i(L, value);
}

void LuaState::push_vector3(const Vector3 &value)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push Vector3.");
    godot::push_vector3(L, value);
}

void LuaState::push_vector3i(const Vector3i &value)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push Vector3i.");
    godot::push_vector3i(L, value);
}

void LuaState::push_vector4(const Vector4 &value)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push Vector4.");
    godot::push_vector4(L, value);
}

void LuaState::push_vector4i(const Vector4i &value)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push Vector4i.");
    godot::push_vector4i(L, value);
}

void LuaState::push_rect2(const Rect2 &value)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push Rect2.");
    godot::push_rect2(L, value);
}

void LuaState::push_rect2i(const Rect2i &value)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push Rect2i.");
    godot::push_rect2i(L, value);
}

void LuaState::push_aabb(const AABB &value)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push AABB.");
    godot::push_aabb(L, value);
}

void LuaState::push_color(const Color &value)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push Color.");
    godot::push_color(L, value);
}

void LuaState::push_plane(const Plane &value)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push Plane.");
    godot::push_plane(L, value);
}

void LuaState::push_quaternion(const Quaternion &value)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push Quaternion.");
    godot::push_quaternion(L, value);
}

void LuaState::push_basis(const Basis &value)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push Basis.");
    godot::push_basis(L, value);
}

void LuaState::push_transform2d(const Transform2D &value)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push Transform2D.");
    godot::push_transform2d(L, value);
}

void LuaState::push_transform3d(const Transform3D &value)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push Transform3D.");
    godot::push_transform3d(L, value);
}

void LuaState::push_projection(const Projection &value)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push Projection.");
    godot::push_projection(L, value);
}

void LuaState::push_callable(const Callable &value)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push Callable.");
    godot::push_callable(L, value);
}

// Godot math type wrappers - Type checking operations
bool LuaState::is_vector2(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return godot::is_vector2(L, index);
}

bool LuaState::is_vector2i(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return godot::is_vector2i(L, index);
}

bool LuaState::is_vector3(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return godot::is_vector3(L, index);
}

bool LuaState::is_vector3i(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return godot::is_vector3i(L, index);
}

bool LuaState::is_vector4(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return godot::is_vector4(L, index);
}

bool LuaState::is_vector4i(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return godot::is_vector4i(L, index);
}

bool LuaState::is_rect2(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return godot::is_rect2(L, index);
}

bool LuaState::is_rect2i(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return godot::is_rect2i(L, index);
}

bool LuaState::is_aabb(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return godot::is_aabb(L, index);
}

bool LuaState::is_color(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return godot::is_color(L, index);
}

bool LuaState::is_plane(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return godot::is_plane(L, index);
}

bool LuaState::is_quaternion(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return godot::is_quaternion(L, index);
}

bool LuaState::is_basis(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return godot::is_basis(L, index);
}

bool LuaState::is_transform2d(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return godot::is_transform2d(L, index);
}

bool LuaState::is_transform3d(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return godot::is_transform3d(L, index);
}

bool LuaState::is_projection(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return godot::is_projection(L, index);
}

bool LuaState::is_callable(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return godot::is_callable(L, index);
}

// Godot math type wrappers - Conversion operations
Vector2 LuaState::to_vector2(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Vector2(), "Lua state is invalid. Cannot convert to Vector2.");
    return godot::to_vector2(L, index);
}

Vector2i LuaState::to_vector2i(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Vector2i(), "Lua state is invalid. Cannot convert to Vector2i.");
    return godot::to_vector2i(L, index);
}

Vector3 LuaState::to_vector3(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Vector3(), "Lua state is invalid. Cannot convert to Vector3.");
    return godot::to_vector3(L, index);
}

Vector3i LuaState::to_vector3i(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Vector3i(), "Lua state is invalid. Cannot convert to Vector3i.");
    return godot::to_vector3i(L, index);
}

Vector4 LuaState::to_vector4(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Vector4(), "Lua state is invalid. Cannot convert to Vector4.");
    return godot::to_vector4(L, index);
}

Vector4i LuaState::to_vector4i(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Vector4i(), "Lua state is invalid. Cannot convert to Vector4i.");
    return godot::to_vector4i(L, index);
}

Rect2 LuaState::to_rect2(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Rect2(), "Lua state is invalid. Cannot convert to Rect2.");
    return godot::to_rect2(L, index);
}

Rect2i LuaState::to_rect2i(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Rect2i(), "Lua state is invalid. Cannot convert to Rect2i.");
    return godot::to_rect2i(L, index);
}

AABB LuaState::to_aabb(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), AABB(), "Lua state is invalid. Cannot convert to AABB.");
    return godot::to_aabb(L, index);
}

Color LuaState::to_color(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Color(), "Lua state is invalid. Cannot convert to Color.");
    return godot::to_color(L, index);
}

Plane LuaState::to_plane(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Plane(), "Lua state is invalid. Cannot convert to Plane.");
    return godot::to_plane(L, index);
}

Quaternion LuaState::to_quaternion(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Quaternion(), "Lua state is invalid. Cannot convert to Quaternion.");
    return godot::to_quaternion(L, index);
}

Basis LuaState::to_basis(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Basis(), "Lua state is invalid. Cannot convert to Basis.");
    return godot::to_basis(L, index);
}

Transform2D LuaState::to_transform2d(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Transform2D(), "Lua state is invalid. Cannot convert to Transform2D.");
    return godot::to_transform2d(L, index);
}

Transform3D LuaState::to_transform3d(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Transform3D(), "Lua state is invalid. Cannot convert to Transform3D.");
    return godot::to_transform3d(L, index);
}

Projection LuaState::to_projection(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Projection(), "Lua state is invalid. Cannot convert to Projection.");
    return godot::to_projection(L, index);
}

Callable LuaState::to_callable(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Callable(), "Lua state is invalid. Cannot convert to Callable.");
    return godot::to_callable(L, index);
}

lua_State *LuaState::get_lua_state() const
{
    return L;
}

// Stack manipulation
int LuaState::abs_index(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), 0, "Lua state is invalid. Cannot convert to absolute index.");
    return lua_absindex(L, index);
}

int LuaState::get_top() const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), 0, "Lua state is invalid. Cannot get stack top.");
    return lua_gettop(L);
}

void LuaState::set_top(int index)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot set stack top.");

    // Validate the index
    // Positive index: must be <= current top (or top will be extended)
    // Negative index: must be valid relative to current top
    // Zero is valid (clears the stack)
    int top = lua_gettop(L);
    ERR_FAIL_COND_MSG(index < 0 && -index > top, vformat("LuaState.settop(%d): Invalid negative index. Stack only has %d elements.", index, top));

    lua_settop(L, index);
}

bool LuaState::check_stack(int size)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot manipulate stack size.");
    return lua_checkstack(L, size) != 0;
}

void LuaState::pop(int n)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot pop stack.");

    // Validate we have enough items to pop
    int top = lua_gettop(L);
    ERR_FAIL_COND_MSG(n < 0, vformat("LuaState.pop(%d): Cannot pop negative number of elements.", n));
    ERR_FAIL_COND_MSG(n > top, vformat("LuaState.pop(%d): Stack underflow. Stack only has %d elements.", n, top));

    lua_pop(L, n);
}

void LuaState::push_value(int index)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push value.");
    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.pushvalue(%d): Invalid stack index. Stack has %d elements.", index, lua_gettop(L)));
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.pushvalue(): Stack overflow. Cannot grow stack.");

    lua_pushvalue(L, index);
}

void LuaState::remove(int index)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot remove value.");
    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.remove(%d): Invalid stack index. Stack has %d elements.", index, lua_gettop(L)));

    lua_remove(L, index);
}

void LuaState::insert(int index)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot insert value.");
    ERR_FAIL_COND_MSG(!has_n_items(L, 1), "LuaState.insert(): Stack is empty. Nothing to insert.");

    // For insert, index can be 1 beyond current top (insert at end)
    int top = lua_gettop(L);
    ERR_FAIL_COND_MSG(index == 0 || (index > 0 && index > top + 1) || (index < 0 && -index > top),
                      vformat("LuaState.insert(%d): Invalid stack index. Stack has %d elements.", index, top));

    lua_insert(L, index);
}

void LuaState::replace(int index)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot replace value.");
    ERR_FAIL_COND_MSG(!has_n_items(L, 1), "LuaState.replace(): Stack is empty. Nothing to replace with.");
    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.replace(%d): Invalid stack index. Stack has %d elements.", index, lua_gettop(L)));

    lua_replace(L, index);
}

// Type checking
bool LuaState::is_nil(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return lua_isnil(L, index);
}

bool LuaState::is_number(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return lua_isnumber(L, index) != 0;
}

bool LuaState::is_string(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return lua_isstring(L, index) != 0;
}

bool LuaState::is_table(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return lua_istable(L, index);
}

bool LuaState::is_function(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return lua_isfunction(L, index);
}

bool LuaState::is_userdata(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return lua_isuserdata(L, index) != 0;
}

bool LuaState::is_boolean(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return lua_isboolean(L, index);
}

bool LuaState::is_thread(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check type.");
    return lua_isthread(L, index);
}

lua_Type LuaState::type(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), static_cast<lua_Type>(LUA_TNONE), "Lua state is invalid. Cannot get type.");
    return static_cast<lua_Type>(lua_type(L, index));
}

String LuaState::type_name(int type_id) const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), String(), "Lua state is invalid. Cannot get type name.");
    return String(lua_typename(L, type_id));
}

// Value access
String LuaState::to_string(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), String(), "Lua state is invalid. Cannot convert to string.");
    return godot::to_string(L, index);
}

double LuaState::to_number(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), 0.0, "Lua state is invalid. Cannot convert to number.");
    return lua_tonumber(L, index);
}

int LuaState::to_integer(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), 0, "Lua state is invalid. Cannot convert to integer.");
    return lua_tointeger(L, index);
}

bool LuaState::to_boolean(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot convert to boolean.");
    return lua_toboolean(L, index) != 0;
}

int LuaState::obj_len(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), 0, "Lua state is invalid. Cannot get object length.");
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, index), 0, vformat("LuaState.objlen(%d): Invalid stack index. Stack has %d elements.", index, lua_gettop(L)));
    return static_cast<int>(lua_objlen(L, index));
}

// Comparisons
bool LuaState::equal(int index1, int index2) const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot compare values.");
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, index1), false, vformat("LuaState.equal(%d, %d): First index is invalid. Stack has %d elements.", index1, index2, lua_gettop(L)));
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, index2), false, vformat("LuaState.equal(%d, %d): Second index is invalid. Stack has %d elements.", index1, index2, lua_gettop(L)));
    return lua_equal(L, index1, index2) != 0;
}

bool LuaState::raw_equal(int index1, int index2) const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot compare values.");
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, index1), false, vformat("LuaState.rawequal(%d, %d): First index is invalid. Stack has %d elements.", index1, index2, lua_gettop(L)));
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, index2), false, vformat("LuaState.rawequal(%d, %d): Second index is invalid. Stack has %d elements.", index1, index2, lua_gettop(L)));
    return lua_rawequal(L, index1, index2) != 0;
}

bool LuaState::less_than(int index1, int index2) const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot compare values.");
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, index1), false, vformat("LuaState.less_than(%d, %d): First index is invalid. Stack has %d elements.", index1, index2, lua_gettop(L)));
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, index2), false, vformat("LuaState.less_than(%d, %d): Second index is invalid. Stack has %d elements.", index1, index2, lua_gettop(L)));
    return lua_lessthan(L, index1, index2) != 0;
}

bool LuaState::is_valid() const
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
void LuaState::push_nil()
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push nil.");
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.pushnil(): Stack overflow. Cannot grow stack.");
    lua_pushnil(L);
}

void LuaState::push_number(double n)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push number.");
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.pushnumber(): Stack overflow. Cannot grow stack.");
    lua_pushnumber(L, n);
}

void LuaState::push_integer(int n)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push integer.");
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.pushinteger(): Stack overflow. Cannot grow stack.");
    lua_pushinteger(L, n);
}

void LuaState::push_string(const String &s)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push string.");
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.pushstring(): Stack overflow. Cannot grow stack.");
    godot::push_string(L, s);
}

void LuaState::push_boolean(bool b)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push boolean.");
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.pushboolean(): Stack overflow. Cannot grow stack.");
    lua_pushboolean(L, b ? 1 : 0);
}

bool LuaState::push_thread()
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot push thread.");
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
void LuaState::new_table()
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot create table.");
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.newtable(): Stack overflow. Cannot grow stack.");
    lua_newtable(L);
}

void LuaState::create_table(int narr, int nrec)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot create table.");
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.createtable(): Stack overflow. Cannot grow stack.");
    lua_createtable(L, narr, nrec);
}

void LuaState::get_table(int index)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot get table value.");

    int top = lua_gettop(L);
    ERR_FAIL_COND_MSG(top < 1, "LuaState.gettable(): Stack is empty. Need a key on the stack.");

    // The table index must not be the top item (the key at -1)
    // gettable pops the key and pushes the value, so -1 is replaced
    ERR_FAIL_COND_MSG(index == -1 || index == top, vformat("LuaState.gettable(%d): Table index cannot be the key (at index %d).", index, top));

    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.gettable(%d): Invalid table index. Stack has %d elements.", index, top));

    lua_gettable(L, index);
}

void LuaState::set_table(int index)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot set table value.");

    int top = lua_gettop(L);
    ERR_FAIL_COND_MSG(top < 2, vformat("LuaState.settable(): Need key and value on stack. Stack has %d elements.", top));

    // The table index must not be the top two items (key and value at -2 and -1)
    // After settable consumes them, those indices would be invalid
    ERR_FAIL_COND_MSG(index == -1 || index == top, vformat("LuaState.settable(%d): Table index cannot be the value (at index %d).", index, top));
    ERR_FAIL_COND_MSG(index == -2 || index == top - 1, vformat("LuaState.settable(%d): Table index cannot be the key (at index %d).", index, top - 1));

    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.settable(%d): Invalid table index. Stack has %d elements.", index, top));

    lua_settable(L, index);
}

void LuaState::get_field(int index, const String &key)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot get field.");
    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.getfield(%d, \"%s\"): Invalid table index. Stack has %d elements.", index, key, lua_gettop(L)));
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.getfield(): Stack overflow. Cannot grow stack.");

    lua_getfield(L, index, key.utf8());
}

void LuaState::set_field(int index, const String &key)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot set field.");

    int top = lua_gettop(L);
    ERR_FAIL_COND_MSG(top < 1, vformat("LuaState.setfield(): Need value on stack. Stack has %d elements.", top));

    // The table index must not be the top item (the value at -1) since setfield pops it
    ERR_FAIL_COND_MSG(index == -1 || index == top, vformat("LuaState.setfield(%d, \"%s\"): Table index cannot be the value (at index %d).", index, key, top));

    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.setfield(%d, \"%s\"): Invalid table index. Stack has %d elements.", index, key, top));

    lua_setfield(L, index, key.utf8());
}

void LuaState::raw_get(int index) const
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot raw get.");

    int top = lua_gettop(L);
    ERR_FAIL_COND_MSG(top < 1, "LuaState.rawget(): Stack is empty. Need a key on the stack.");

    // The table index must not be the top item (the key at -1)
    ERR_FAIL_COND_MSG(index == -1 || index == top, vformat("LuaState.rawget(%d): Table index cannot be the key (at index %d).", index, top));

    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.rawget(%d): Invalid table index. Stack has %d elements.", index, top));

    lua_rawget(L, index);
}

void LuaState::raw_get_field(int index, const String &key) const
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot raw get field.");
    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.rawgetfield(%d, \"%s\"): Invalid table index. Stack has %d elements.", index, key, lua_gettop(L)));
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.rawgetfield(): Stack overflow. Cannot grow stack.");

    lua_rawgetfield(L, index, key.utf8());
}

void LuaState::raw_set_field(int index, const String &key)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot raw set field.");

    int top = lua_gettop(L);
    ERR_FAIL_COND_MSG(top < 1, vformat("LuaState.rawsetfield(): Need value on stack. Stack has %d elements.", top));

    // The table index must not be the top item (the value at -1) since rawsetfield pops it
    ERR_FAIL_COND_MSG(index == -1 || index == top, vformat("LuaState.rawsetfield(%d, \"%s\"): Table index cannot be the value (at index %d).", index, key, top));

    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.rawsetfield(%d, \"%s\"): Invalid table index. Stack has %d elements.", index, key, top));

    lua_rawsetfield(L, index, key.utf8());
}

void LuaState::raw_set(int index)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot raw set.");

    int top = lua_gettop(L);
    ERR_FAIL_COND_MSG(top < 2, vformat("LuaState.rawset(): Need key and value on stack. Stack has %d elements.", top));

    // The table index must not be the top two items (key and value at -2 and -1)
    ERR_FAIL_COND_MSG(index == -1 || index == top, vformat("LuaState.rawset(%d): Table index cannot be the value (at index %d).", index, top));
    ERR_FAIL_COND_MSG(index == -2 || index == top - 1, vformat("LuaState.rawset(%d): Table index cannot be the key (at index %d).", index, top - 1));

    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.rawset(%d): Invalid table index. Stack has %d elements.", index, top));

    lua_rawset(L, index);
}

void LuaState::raw_geti(int index, int n) const
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot raw get index.");
    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.rawgeti(%d, %d): Invalid table index. Stack has %d elements.", index, n, lua_gettop(L)));
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.rawgeti(): Stack overflow. Cannot grow stack.");

    lua_rawgeti(L, index, n);
}

void LuaState::raw_seti(int index, int n)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot raw set index.");

    int top = lua_gettop(L);
    ERR_FAIL_COND_MSG(top < 1, vformat("LuaState.rawseti(): Need value on stack. Stack has %d elements.", top));

    // The table index must not be the top item (the value at -1) since rawseti pops it
    ERR_FAIL_COND_MSG(index == -1 || index == top, vformat("LuaState.rawseti(%d, %d): Table index cannot be the value (at index %d).", index, n, top));

    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.rawseti(%d, %d): Invalid table index. Stack has %d elements.", index, n, top));

    lua_rawseti(L, index, n);
}

bool LuaState::get_read_only(int index) const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot get read-only status.");
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, index), false, vformat("LuaState.getreadonly(%d): Invalid stack index. Stack has %d elements.", index, lua_gettop(L)));

    return lua_getreadonly(L, index) != 0;
}

void LuaState::set_read_only(int index, bool readonly)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot set read-only status.");
    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.setreadonly(%d, %s): Invalid stack index. Stack has %d elements.", index, readonly ? "true" : "false", lua_gettop(L)));

    lua_setreadonly(L, index, readonly ? 1 : 0);
}

void LuaState::get_fenv(int index)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot get fenv.");

    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.getfenv(%d): Invalid stack index. Stack has %d elements.", index, lua_gettop(L)));
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.getfenv(): Stack overflow. Cannot grow stack.");

    lua_getfenv(L, index);
}

bool LuaState::set_fenv(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot set fenv.");

    int top = lua_gettop(L);
    ERR_FAIL_COND_V_MSG(top < 1, false, "LuaState.setfenv(): Stack is empty. Need an environment table on the stack.");

    // The object index must not be the top item (the environment table at -1) since setfenv pops it
    ERR_FAIL_COND_V_MSG(index == -1 || index == top, false, vformat("LuaState.setfenv(%d): Index cannot be the environment table (at index %d).", index, top));

    ERR_FAIL_COND_V_MSG(!is_valid_index(L, index), false, vformat("LuaState.setfenv(%d): Invalid stack index. Stack has %d elements.", index, top));

    return lua_setfenv(L, index) != 0;
}

bool LuaState::next(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot get next table entry.");

    int top = lua_gettop(L);
    ERR_FAIL_COND_V_MSG(top < 1, false, "LuaState.next(): Stack is empty. Need a table and key on the stack.");

    // The table index must not be the top item (the key at -1)
    ERR_FAIL_COND_V_MSG(index == -1 || index == top, false, vformat("LuaState.next(%d): Table index cannot be the key (at index %d).", index, top));

    ERR_FAIL_COND_V_MSG(!is_valid_index(L, index), false, vformat("LuaState.next(%d): Invalid table index. Stack has %d elements.", index, top));

    return lua_next(L, index) != 0;
}

// Metatable operations
bool LuaState::get_metatable(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot get metatable.");
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, index), false, vformat("LuaState.getmetatable(%d): Invalid stack index. Stack has %d elements.", index, lua_gettop(L)));
    ERR_FAIL_COND_V_MSG(!lua_checkstack(L, 1), false, "LuaState.getmetatable(): Stack overflow. Cannot grow stack.");

    return lua_getmetatable(L, index) != 0;
}

bool LuaState::set_metatable(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot set metatable.");

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
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot call function.");
    ERR_FAIL_COND_MSG(nargs < 0, vformat("LuaState.call(%d, %d): nargs cannot be negative.", nargs, nresults));
    ERR_FAIL_COND_MSG(!has_n_items(L, nargs + 1), vformat("LuaState.call(%d, %d): Need function + %d arguments on stack. Stack has %d elements.", nargs, nresults, nargs, lua_gettop(L)));

    lua_call(L, nargs, nresults);
}

lua_Status LuaState::pcall(int nargs, int nresults, int errfunc)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), LUA_ERRMEM, "Lua state is invalid. Cannot pcall function.");
    ERR_FAIL_COND_V_MSG(nargs < 0, LUA_ERRMEM, vformat("LuaState.pcall(%d, %d, %d): nargs cannot be negative.", nargs, nresults, errfunc));
    ERR_FAIL_COND_V_MSG(!has_n_items(L, nargs + 1), LUA_ERRMEM, vformat("LuaState.pcall(%d, %d, %d): Need function + %d arguments on stack. Stack has %d elements.", nargs, nresults, errfunc, nargs, lua_gettop(L)));
    ERR_FAIL_COND_V_MSG(errfunc != 0 && !is_valid_index(L, errfunc), LUA_ERRMEM, vformat("LuaState.pcall(%d, %d, %d): Invalid error function index. Stack has %d elements.", nargs, nresults, errfunc, lua_gettop(L)));

    int status = lua_pcall(L, nargs, nresults, errfunc);
    return static_cast<lua_Status>(status);
}

// Thread operations
Ref<LuaState> LuaState::bind_thread(lua_State *thread_L)
{
    ERR_FAIL_NULL_V_MSG(thread_L, Ref<LuaState>(), "Failed to bind thread.");

    Ref<LuaState> thread_state = memnew(LuaState(thread_L, main_thread.is_valid() ? main_thread : Ref<LuaState>(this)));
    return thread_state;
}

Ref<LuaState> LuaState::new_thread()
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Ref<LuaState>(), "Lua state is invalid. Cannot create thread.");
    ERR_FAIL_COND_V_MSG(!lua_checkstack(L, 1), Ref<LuaState>(), "LuaState.newthread(): Stack overflow. Cannot grow stack.");

    // Create a new thread and push it onto the stack
    lua_State *thread_L = lua_newthread(L);
    ERR_FAIL_NULL_V_MSG(thread_L, Ref<LuaState>(), "Failed to create new Lua thread.");

    return bind_thread(thread_L);
}

Ref<LuaState> LuaState::to_thread(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), Ref<LuaState>(), "Lua state is invalid. Cannot convert to thread.");
    ERR_FAIL_COND_V_MSG(!is_thread(index), Ref<LuaState>(), "Stack value at index is not a thread.");

    lua_State *thread_L = lua_tothread(L, index);
    ERR_FAIL_NULL_V_MSG(thread_L, Ref<LuaState>(), "Failed to get thread lua_State*.");

    return bind_thread(thread_L);
}

Ref<LuaState> LuaState::get_main_thread()
{
    return main_thread.is_valid() ? main_thread : Ref<LuaState>(this);
}

void LuaState::reset_thread()
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot reset thread.");
    lua_resetthread(L);
}

bool LuaState::is_thread_reset() const
{
    ERR_FAIL_COND_V_MSG(!is_valid(), false, "Lua state is invalid. Cannot check if thread is reset.");
    return lua_isthreadreset(L) != 0;
}

void LuaState::xmove(LuaState *to_state, int n)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot move stack values.");
    ERR_FAIL_COND_MSG(n < 0, vformat("LuaState.xmove(%p, %d): n cannot be negative.", to_state, n));
    ERR_FAIL_COND_MSG(!to_state || !to_state->is_valid(), "Destination Lua state is invalid. Cannot move stack values.");
    ERR_FAIL_COND_MSG(get_main_thread() != to_state->get_main_thread(), "Cannot xmove between different Luau VMs.");
    ERR_FAIL_COND_MSG(!has_n_items(L, n), vformat("LuaState.xmove(%p, %d): Not enough items on source stack. Stack has %d elements.", to_state, n, lua_gettop(L)));
    ERR_FAIL_COND_MSG(!to_state->check_stack(n), vformat("LuaState.xmove(%p, %d): Stack overflow on destination. Cannot grow stack.", to_state, n));

    lua_xmove(L, to_state->L, n);
}

void LuaState::xpush(LuaState *to_state, int index)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot push stack values.");
    ERR_FAIL_COND_MSG(!to_state || !to_state->is_valid(), "Destination Lua state is invalid. Cannot push stack values.");
    ERR_FAIL_COND_MSG(get_main_thread() != to_state->get_main_thread(), "Cannot xpush between different Luau VMs.");
    ERR_FAIL_COND_MSG(!is_valid_index(L, index), vformat("LuaState.xpush(%p, %d): Source index is invalid.", to_state, index));
    ERR_FAIL_COND_MSG(!to_state->check_stack(1), vformat("LuaState.xpush(%p, %d): Stack overflow on destination. Cannot grow stack.", to_state, 1));

    lua_xpush(L, to_state->L, index);
}

lua_CoStatus LuaState::co_status(LuaState *co)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), LUA_CORUN, "Lua state is invalid. Cannot get coroutine status.");
    ERR_FAIL_COND_V_MSG(!co || !co->is_valid(), LUA_CORUN, "Coroutine Lua state is invalid. Cannot get coroutine status.");
    ERR_FAIL_COND_V_MSG(get_main_thread() != co->get_main_thread(), LUA_CORUN, "Cannot get coroutine status for a different Luau VM.");

    return static_cast<lua_CoStatus>(lua_costatus(L, co->L));
}

// Garbage collection
int LuaState::gc(lua_GCOp what, int data)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), 0, "Lua state is invalid. Cannot control GC.");
    return lua_gc(L, what, data);
}

int LuaState::ref(int index)
{
    ERR_FAIL_COND_V_MSG(!is_valid(), LUA_NOREF, "Lua state is invalid. Cannot create reference.");
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, index), LUA_NOREF, vformat("LuaState.ref(%d): Invalid stack index. Stack has %d elements.", index, lua_gettop(L)));

    return lua_ref(L, index);
}

void LuaState::get_ref(int ref)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot get reference.");
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 1), "LuaState.getref(): Stack overflow. Cannot grow stack.");

    lua_getref(L, ref);
}

void LuaState::unref(int ref)
{
    ERR_FAIL_COND_MSG(!is_valid(), "Lua state is invalid. Cannot release reference.");
    lua_unref(L, ref);
}
