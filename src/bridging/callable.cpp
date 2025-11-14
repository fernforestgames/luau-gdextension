#include "bridging/callable.h"

#include "bridging/variant.h"
#include "helpers.h"
#include "lua_state.h"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/templates/hashfuncs.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <lualib.h>

using namespace gdluau;
using namespace godot;

static const char *const CALLABLE_METATABLE_NAME = "GDCallable";

static void callable_dtor(void *ud)
{
    Callable *callable = static_cast<Callable *>(ud);
    callable->~Callable();
}

// Callable.__tostring metamethod
static int callable_tostring(lua_State *L)
{
    Callable *callable = static_cast<Callable *>(lua_touserdata(L, 1));
    String str = Variant(*callable).stringify();
    lua_pop(L, 1);

    CharString utf8 = str.utf8();
    lua_pushlstring(L, utf8.get_data(), utf8.length());
    return 1;
}

// Callable.__eq metamethod
static int callable_eq(lua_State *L)
{
    Callable a = *static_cast<Callable *>(lua_touserdata(L, 1));
    Callable b = *static_cast<Callable *>(lua_touserdata(L, 2));
    lua_pop(L, 2);

    lua_pushboolean(L, a == b);
    return 1;
}

// Callable.__call metamethod
static int callable_call(lua_State *L)
{
    // First argument on the stack is the object being called
    Callable callable = *static_cast<Callable *>(lua_touserdata(L, 1));
    if (!callable.is_valid()) [[unlikely]]
    {
        luaL_argerror(L, 1, "Callable is not valid");
    }

    int arg_count = lua_gettop(L) - 1; // Subtract 1 for self
    int expected_args = callable.get_argument_count();
    if (expected_args >= 0 && arg_count < expected_args) [[unlikely]]
    {
        luaL_error(L, "Too few arguments for Callable (expected at least %d, got %d)", expected_args, arg_count);
    }

    Array args;
    args.resize(arg_count);
    for (int argi = 0; argi < arg_count; argi++)
    {
        // Skip `self` at index 1 (Lua stack is 1-based)
        int stack_idx = argi + 2;
        args[argi] = to_variant(L, stack_idx);
    }

    // Pop all arguments and `self` from the stack
    lua_pop(L, arg_count + 1);

    Variant result = callable.callv(args);

    // The Callable can return additional values by pushing them on the stack
    int additional_returns = lua_gettop(L);

    push_variant(L, result);
    return 1 + additional_returns;
}

static void push_callable_metatable(lua_State *L)
{
    if (!luaL_newmetatable(L, CALLABLE_METATABLE_NAME)) [[likely]]
    {
        // Metatable already exists
        return;
    }

    lua_pushcfunction(L, callable_tostring, "Callable.__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_pushcfunction(L, generic_lua_concat, "Callable.__concat");
    lua_setfield(L, -2, "__concat");

    lua_pushcfunction(L, callable_eq, "Callable.__eq");
    lua_setfield(L, -2, "__eq");

    lua_pushcfunction(L, callable_call, "Callable.__call");
    lua_setfield(L, -2, "__call");

    // Freeze metatable
    lua_setreadonly(L, -1, 1);
}

bool gdluau::is_godot_callable(lua_State *L, int p_index)
{
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, p_index), false, vformat("is_godot_callable(%d): Invalid stack index. Stack has %d elements.", p_index, lua_gettop(L)));
    ERR_FAIL_COND_V_MSG(!lua_checkstack(L, 2), false, vformat("is_godot_callable(%d): Stack overflow. Cannot grow stack.", p_index));

    if (!lua_getmetatable(L, p_index))
    {
        return false;
    }

    luaL_getmetatable(L, CALLABLE_METATABLE_NAME);
    bool mt_equal = lua_rawequal(L, -1, -2);
    lua_pop(L, 2); // Pop both metatables

    return mt_equal;
}

Callable gdluau::to_callable(lua_State *L, int p_index)
{
    ERR_FAIL_COND_V_MSG(!is_valid_index(L, p_index), Callable(), vformat("to_callable(%d): Invalid stack index. Stack has %d elements.", p_index, lua_gettop(L)));

    // Check if this is a Callable pushed via push_callable()
    // If so, return the original Callable instead of wrapping it again
    if (is_godot_callable(L, p_index))
    {
        Callable *callable = static_cast<Callable *>(lua_touserdata(L, p_index));
        return *callable;
    }

    int type = lua_type(L, p_index);
    ERR_FAIL_COND_V_MSG(type != LUA_TFUNCTION && type != LUA_TUSERDATA && type != LUA_TTABLE, Callable(), vformat("to_callable(%d): Expected a function, userdata, or table, got %s.", p_index, lua_typename(L, type)));

    // Protect the value from GC
    int value_ref = lua_ref(L, p_index);

    LuaState *state = LuaState::find_lua_state(L);
    ERR_FAIL_COND_V_MSG(!state, Callable(), "to_callable(): Could not find existing LuaState for the given lua_State.");

    LuaCallable *lc = memnew(LuaCallable(state, value_ref));
    return Callable(lc);
}

void gdluau::push_callable(lua_State *L, const Callable &p_callable)
{
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 2), "push_callable(): Stack overflow. Cannot grow stack."); // Callable + metatable

    // Check if this is a LuaCallable wrapping a Lua value
    // If so, push the original Lua value instead of wrapping it again
    LuaCallable *lc = dynamic_cast<LuaCallable *>(p_callable.get_custom());
    if (lc)
    {
        ERR_FAIL_COND_MSG(!lc->is_valid(), "push_callable(): LuaCallable is invalid.");

        LuaState *callable_state = lc->get_lua_state();
        ERR_FAIL_COND_MSG(callable_state->get_main_thread()->get_lua_state() != lua_mainthread(L), "push_callable(): Cannot push a Lua value from a different Luau VM.");

        // Push the original value back onto the stack
        lua_getref(L, lc->get_lua_ref());
        return;
    }

    void *ptr = lua_newuserdatadtor(L, sizeof(Callable), &callable_dtor);
    memnew_placement(ptr, Callable(p_callable));

    push_callable_metatable(L);
    lua_setmetatable(L, -2);
}

LuaCallable::LuaCallable(LuaState *p_state, int p_lua_ref)
    : lua_state_id(p_state->get_instance_id()), lua_ref(p_lua_ref)
{
}

LuaCallable::~LuaCallable()
{
    LuaState *state = get_lua_state();

    // Release the reference from the Lua registry
    if (state && lua_ref != LUA_NOREF && state->is_valid())
    {
        state->unref(lua_ref);
    }
}

bool LuaCallable::get_func_info(const char *p_what, lua_Debug &r_ar) const
{
    LuaState *state = get_lua_state();
    if (!state || !state->is_valid())
    {
        return false;
    }

    lua_State *L = state->get_lua_state();
    ERR_FAIL_COND_V_MSG(!lua_checkstack(L, 1), false, "LuaCallable.get_func_info(): Stack overflow. Cannot grow stack.");

    if (lua_getref(L, lua_ref) != LUA_TFUNCTION)
    {
        lua_pop(L, 1);
        return false;
    }

    bool result = (lua_getinfo(L, -1, p_what, &r_ar) != 0);
    lua_pop(L, 1);

    return result;
}

bool LuaCallable::get_func_from_callable_table_or_userdata(lua_State *L) const
{
    ERR_FAIL_COND_V_MSG(!lua_checkstack(L, 2), false, "LuaCallable.get_func_from_callable_table_or_userdata(): Stack overflow. Cannot grow stack.");
    ERR_FAIL_COND_V_MSG(lua_getmetatable(L, -1) == 0, false, vformat("LuaCallable.get_func_from_callable_table_or_userdata(): Expected userdata or table %s to have a __call metamethod, but no metatable found.", get_as_text()));

    int type = lua_getfield(L, -1, "__call");
    if (type == LUA_TFUNCTION)
    {
        lua_remove(L, -2); // Remove metatable
        return true;
    }
    else if (type == LUA_TTABLE || type == LUA_TUSERDATA)
    {
        lua_remove(L, -2); // Remove metatable
        int value_index = lua_gettop(L);
        bool result = get_func_from_callable_table_or_userdata(L);
        lua_remove(L, value_index); // Remove __call

        return result;
    }
    else
    {
        lua_pop(L, 2); // Pop metatable and __call
        return false;
    }
}

uint32_t LuaCallable::hash() const
{
    uint32_t h = HASH_MURMUR3_SEED;
    h = hash_murmur3_one_64(static_cast<uint64_t>(lua_state_id), h);
    h = hash_murmur3_one_64(static_cast<uint64_t>(lua_ref), h);
    return hash_fmix32(h);
}

String LuaCallable::get_as_text() const
{
    lua_Debug ar;
    if (get_func_info("n", ar))
    {
        return "lua:" + String(ar.name);
    }

    // If not able to get function info, use tostring()
    LuaState *state = get_lua_state();
    if (!state || !state->is_valid())
    {
        return "lua:<unknown>";
    }

    lua_State *L = state->get_lua_state();
    ERR_FAIL_COND_V_MSG(!lua_checkstack(L, 2), "<unknown>", "LuaCallable.get_as_text(): Stack overflow. Cannot grow stack.");

    lua_getref(L, lua_ref);

    size_t len;
    const char *str = luaL_tolstring(L, -1, &len);
    if (!str)
    {
        lua_pop(L, 2); // Pop ref and tostring result
        return "lua:<unknown>";
    }

    String result = "lua:" + String::utf8(str, len);
    lua_pop(L, 2); // Pop ref and tostring result

    return result;
}

CallableCustom::CompareEqualFunc LuaCallable::get_compare_equal_func() const
{
    return &LuaCallable::compare_equal;
}

CallableCustom::CompareLessFunc LuaCallable::get_compare_less_func() const
{
    return &LuaCallable::compare_less;
}

ObjectID LuaCallable::get_object() const
{
    return lua_state_id;
}

bool LuaCallable::is_valid() const
{
    LuaState *state = get_lua_state();
    return state != nullptr && state->is_valid();
}

int LuaCallable::get_argument_count(bool &r_is_valid) const
{
    lua_Debug ar;
    if (get_func_info("a", ar))
    {
        r_is_valid = true;
        return ar.nparams;
    }
    else
    {
        r_is_valid = false;
        return 0;
    }
}

void LuaCallable::call(const Variant **p_arguments, int p_argcount, Variant &r_return_value, GDExtensionCallError &r_call_error) const
{
    r_call_error.error = GDEXTENSION_CALL_OK;
    r_return_value = Variant(); // Default to nil

    LuaState *state = get_lua_state();
    if (!state) [[unlikely]]
    {
        ERR_PRINT("LuaCallable.call(): LuaState is null");
        r_call_error.error = GDEXTENSION_CALL_ERROR_INSTANCE_IS_NULL;
        return;
    }
    else if (!state->is_valid()) [[unlikely]]
    {
        ERR_PRINT("LuaCallable.call(): LuaState is not valid");
        r_call_error.error = GDEXTENSION_CALL_ERROR_INSTANCE_IS_NULL;
        return;
    }

    lua_State *L = state->get_lua_state();
    if (!L) [[unlikely]]
    {
        ERR_PRINT("LuaCallable.call(): lua_State is null");
        r_call_error.error = GDEXTENSION_CALL_ERROR_INSTANCE_IS_NULL;
        return;
    }

    // Referenced value + all arguments
    if (!lua_checkstack(L, 1 + p_argcount)) [[unlikely]]
    {
        ERR_PRINT(vformat("LuaCallable.call(): Stack overflow. Cannot grow stack for %d arguments.", p_argcount));
        r_call_error.error = GDEXTENSION_CALL_ERROR_TOO_MANY_ARGUMENTS;
        return;
    }

    int type = lua_getref(L, lua_ref);
    int self_arg = 0;
    switch (type)
    {
    case LUA_TFUNCTION:
        // Functions are directly callable, nothing to do
        break;

    case LUA_TUSERDATA:
        [[fallthrough]];
    case LUA_TTABLE:
        if (!get_func_from_callable_table_or_userdata(L))
        {
            ERR_PRINT(vformat("LuaCallable.call(): Expected userdata or table %s to have a __call metamethod", get_as_text()));
            lua_pop(L, 1);
            r_call_error.error = GDEXTENSION_CALL_ERROR_INVALID_METHOD;
            return;
        }

        lua_insert(L, -2); // Move function below userdata/table (which becomes the `self` arg)
        self_arg = 1;
        break;

    default:
        ERR_PRINT(vformat("LuaCallable.call(): Expected function, userdata, or table, got %s.", lua_typename(L, type)));
        lua_pop(L, 1);
        r_call_error.error = GDEXTENSION_CALL_ERROR_INVALID_METHOD;
        return;
    }

    for (int i = 0; i < p_argcount; i++)
    {
        push_variant(L, *(p_arguments[i]));
    }

    int status = lua_pcall(L, self_arg + p_argcount, 1, 0);
    if (status != LUA_OK)
    {
        const char *error_msg = lua_tostring(L, -1);
        ERR_PRINT(vformat("LuaCallable.call(): error during call to %s: %s", get_as_text(), error_msg));
        lua_pop(L, 1);

        // Don't set r_call_error, to avoid a fatal script error.
        return;
    }

    r_return_value = to_variant(L, -1);
    lua_pop(L, 1);
}

bool LuaCallable::compare_equal(const CallableCustom *p_a, const CallableCustom *p_b)
{
    const LuaCallable *a = static_cast<const LuaCallable *>(p_a);
    const LuaCallable *b = static_cast<const LuaCallable *>(p_b);
    return a->lua_state_id == b->lua_state_id && a->lua_ref == b->lua_ref;
}

bool LuaCallable::compare_less(const CallableCustom *p_a, const CallableCustom *p_b)
{
    const LuaCallable *a = static_cast<const LuaCallable *>(p_a);
    const LuaCallable *b = static_cast<const LuaCallable *>(p_b);

    if (a->lua_state_id != b->lua_state_id)
    {
        return a->lua_state_id < b->lua_state_id;
    }

    return a->lua_ref < b->lua_ref;
}

LuaState *LuaCallable::get_lua_state() const
{
    Object *obj = ObjectDB::get_instance(lua_state_id);
    return Object::cast_to<LuaState>(obj);
}

int LuaCallable::get_lua_ref() const
{
    return lua_ref;
}
