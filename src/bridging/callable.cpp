#include "bridging/callable.h"
#include "bridging/variant.h"
#include "helpers.h"
#include "lua_state.h"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/templates/hashfuncs.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <lualib.h>

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
    if (!callable.is_valid())
    {
        luaL_error(L, "Callable is not valid");
    }

    int arg_count = lua_gettop(L) - 1; // Subtract 1 for self
    int expected_args = callable.get_argument_count();
    if (expected_args >= 0 && arg_count != expected_args)
    {
        luaL_error(L, "Callable expects %d arguments, got %d", expected_args, arg_count);
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
    push_variant(L, result);
    return 1;
}

static void push_callable_metatable(lua_State *L)
{
    if (!luaL_newmetatable(L, CALLABLE_METATABLE_NAME))
    {
        // Metatable already exists
        return;
    }

    // TODO: __namecall optimization

    lua_pushcfunction(L, callable_tostring, "Callable.__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_pushcfunction(L, generic_lua_concat, "Callable.__concat");
    lua_setfield(L, -2, "__concat");

    lua_pushcfunction(L, callable_eq, "Callable.__eq");
    lua_setfield(L, -2, "__eq");

    lua_pushcfunction(L, callable_call, "Callable.__call");
    lua_setfield(L, -2, "__call");
}

static String lua_function_name(lua_State *L, int p_index)
{
    String funcname;
    lua_Debug ar;
    if (lua_getinfo(L, p_index, "n", &ar))
    {
        funcname = ar.name;
    }

    return funcname.is_empty() ? "<unknown>" : funcname;
}

Callable godot::to_callable(lua_State *L, int p_index)
{
    ERR_FAIL_COND_V_MSG(p_index > lua_gettop(L), Callable(), vformat("to_callable(%d): Invalid stack index. Stack has %d elements.", p_index, lua_gettop(L)));

    if (!lua_isfunction(L, p_index))
    {
        return Callable();
    }

    // Protect the function from GC
    int func_ref = lua_ref(L, p_index);

    LuaState *state = LuaState::find_lua_state(L);
    String func_name = lua_function_name(L, p_index);
    LuaCallable *lc;
    if (state)
    {
        lc = memnew(LuaCallable(state, true, func_name, func_ref));
    }
    else
    {
        Ref<LuaState> created_state = LuaState::find_or_create_lua_state(L);
        lc = memnew(LuaCallable(created_state.ptr(), false, func_name, func_ref));
    }

    return Callable(lc);
}

void godot::push_callable(lua_State *L, const Callable &p_callable)
{
    ERR_FAIL_COND_MSG(!lua_checkstack(L, 2), "push_callable(): Stack overflow. Cannot grow stack."); // Callable + metatable

    // Check if this is a LuaCallable wrapping a Lua function
    // If so, push the original Lua function instead of wrapping it again
    LuaCallable *lc = dynamic_cast<LuaCallable *>(p_callable.get_custom());
    if (lc)
    {
        ERR_FAIL_COND_MSG(!lc->is_valid(), "push_callable(): LuaCallable is invalid.");

        LuaState *callable_state = lc->get_lua_state();
        ERR_FAIL_COND_MSG(callable_state->get_main_thread()->get_lua_state() != lua_mainthread(L), "push_callable(): Cannot push a Lua function from a different Luau VM.");

        // Push the original function back onto the stack
        lua_getref(L, lc->get_func_ref());
        return;
    }

    void *ptr = lua_newuserdatadtor(L, sizeof(Callable), &callable_dtor);
    memnew_placement(ptr, Callable(p_callable));

    push_callable_metatable(L);
    lua_setmetatable(L, -2);
}

LuaCallable::LuaCallable(LuaState *p_state, bool p_weak_state_ref, const String &p_func_name, int p_func_ref)
    : lua_state_id(p_state->get_instance_id()), lua_state(), func_name(p_func_name), func_ref(p_func_ref)
{
    if (!p_weak_state_ref)
    {
        lua_state.reference_ptr(p_state);
    }
}

LuaCallable::~LuaCallable()
{
    LuaState *state = get_lua_state();

    // Release the function reference from the Lua registry
    if (state && func_ref != LUA_NOREF && state->is_valid())
    {
        state->unref(func_ref);
    }
}

uint32_t LuaCallable::hash() const
{
    uint32_t h = HASH_MURMUR3_SEED;
    h = hash_murmur3_one_64(static_cast<uint64_t>(lua_state_id), h);
    h = hash_murmur3_one_64(static_cast<uint64_t>(func_ref), h);
    return hash_fmix32(h);
}

String LuaCallable::get_as_text() const
{
    return "lua:" + func_name;
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

void LuaCallable::call(const Variant **p_arguments, int p_argcount, Variant &r_return_value, GDExtensionCallError &r_call_error) const
{
    r_call_error.error = GDEXTENSION_CALL_OK;
    r_return_value = Variant(); // Default to nil

    LuaState *state = get_lua_state();
    if (!state)
    {
        ERR_PRINT("LuaCallable.call(): LuaState is null");
        r_call_error.error = GDEXTENSION_CALL_ERROR_INSTANCE_IS_NULL;
        return;
    }
    else if (!state->is_valid())
    {
        ERR_PRINT("LuaCallable.call(): LuaState is not valid");
        r_call_error.error = GDEXTENSION_CALL_ERROR_INSTANCE_IS_NULL;
        return;
    }

    lua_State *L = state->get_lua_state();
    if (!L)
    {
        ERR_PRINT("LuaCallable.call(): lua_State is null");
        r_call_error.error = GDEXTENSION_CALL_ERROR_INSTANCE_IS_NULL;
        return;
    }

    int stack_top = lua_gettop(L);

    // Function + all arguments
    if (!lua_checkstack(L, 1 + p_argcount))
    {
        ERR_PRINT(vformat("LuaCallable.call(): Stack overflow. Cannot grow stack for %d arguments.", p_argcount));
        r_call_error.error = GDEXTENSION_CALL_ERROR_TOO_MANY_ARGUMENTS;
        return;
    }

    lua_getref(L, func_ref);
    if (!lua_isLfunction(L, -1))
    {
        ERR_PRINT("LuaCallable.call(): function reference is not valid");
        lua_pop(L, 1);
        r_call_error.error = GDEXTENSION_CALL_ERROR_INVALID_METHOD;
        return;
    }

    for (int i = 0; i < p_argcount; i++)
    {
        push_variant(L, *(p_arguments[i]));
    }

    int status = lua_pcall(L, p_argcount, 1, 0);
    if (status != LUA_OK)
    {
        const char *error_msg = lua_tostring(L, -1);
        ERR_PRINT(vformat("LuaCallable.call(): error during call to %s: %s", func_name, error_msg));
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
    return a->lua_state_id == b->lua_state_id && a->func_ref == b->func_ref;
}

bool LuaCallable::compare_less(const CallableCustom *p_a, const CallableCustom *p_b)
{
    const LuaCallable *a = static_cast<const LuaCallable *>(p_a);
    const LuaCallable *b = static_cast<const LuaCallable *>(p_b);

    if (a->lua_state_id != b->lua_state_id)
    {
        return a->lua_state_id < b->lua_state_id;
    }

    return a->func_ref < b->func_ref;
}

LuaState *LuaCallable::get_lua_state() const
{
    Object *obj = ObjectDB::get_instance(lua_state_id);
    return Object::cast_to<LuaState>(obj);
}
