#include "lua_callable.h"
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <lualib.h>

using namespace godot;

LuaCallable::LuaCallable(LuaState *p_state, const String &p_func_name, int p_func_ref)
    : func_name(p_func_name), func_ref(p_func_ref)
{
    ERR_FAIL_COND_MSG(!p_state, "LuaCallable: p_state cannot be null");

    lua_state_id = p_state->get_instance_id();

    if (func_name.is_empty())
    {
        func_name = "<unknown>";
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
    // Simple hash combining state ID and func_ref
    uint64_t combined = ((uint64_t)lua_state_id << 32) | (uint64_t)func_ref;
    return (uint32_t)(combined ^ (combined >> 32));
}

String LuaCallable::get_as_text() const
{
    return vformat("LuaCallable(%s, ref=%d)", func_name, func_ref);
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
    return get_lua_state() != nullptr && get_lua_state()->is_valid();
}

void LuaCallable::call(const Variant **p_arguments, int p_argcount, Variant &r_return_value, GDExtensionCallError &r_call_error) const
{
    r_call_error.error = GDEXTENSION_CALL_OK;
    r_return_value = Variant(); // Default to nil

    // Check if state is still valid
    LuaState *state = get_lua_state();
    if (!state)
    {
        ERR_PRINT("LuaCallable::call() - LuaState is null");
        r_call_error.error = GDEXTENSION_CALL_ERROR_INSTANCE_IS_NULL;
        return;
    }

    if (!state->is_valid())
    {
        ERR_PRINT(vformat("LuaCallable::call() - LuaState is not valid, refcount=%d", state->get_reference_count()));
        r_call_error.error = GDEXTENSION_CALL_ERROR_INSTANCE_IS_NULL;
        return;
    }

    lua_State *L = state->get_lua_state();
    if (!L)
    {
        ERR_PRINT("LuaCallable::call() - lua_State is unexpectedly null after is_valid() check");
        r_call_error.error = GDEXTENSION_CALL_ERROR_INSTANCE_IS_NULL;
        return;
    }

    // Save stack top before we start, so we can calculate results correctly
    // This is important for nested calls where the stack isn't empty
    int stack_top = lua_gettop(L);

    // Check stack space for function + all arguments
    // We need: 1 slot for the function, p_argcount slots for arguments
    if (!lua_checkstack(L, 1 + p_argcount))
    {
        ERR_PRINT(vformat("LuaCallable::call() - Stack overflow. Cannot grow stack for %d arguments.", p_argcount));
        r_call_error.error = GDEXTENSION_CALL_ERROR_INVALID_ARGUMENT;
        return;
    }

    // Get the function from the registry
    lua_rawgeti(L, LUA_REGISTRYINDEX, func_ref);

    if (!lua_isfunction(L, -1))
    {
        ERR_PRINT("LuaCallable: Function reference is no longer valid.");
        lua_pop(L, 1);
        r_call_error.error = GDEXTENSION_CALL_ERROR_INSTANCE_IS_NULL;
        return;
    }

    // Push all arguments
    for (int i = 0; i < p_argcount; i++)
    {
        state->push_variant(*p_arguments[i]);
    }

    // Call the function using pcall for error handling
    int status = lua_pcall(L, p_argcount, LUA_MULTRET, 0);

    if (status != LUA_OK)
    {
        // Error occurred
        const char *error_msg = lua_tostring(L, -1);
        ERR_PRINT(vformat("LuaCallable: Unhandled error during call to Lua function %s: %s", func_name, error_msg ? error_msg : "Unknown error"));
        lua_pop(L, 1); // Pop error message

        // Don't set r_call_error, to prevent a fatal script error.
        return;
    }

    // Get number of return values relative to our starting position
    // lua_pcall replaces function + args with results, so new_top - old_top = nresults
    int nresults = lua_gettop(L) - stack_top;

    if (nresults == 0)
    {
        // No return values, return nil
        r_return_value = Variant();
    }
    else if (nresults == 1)
    {
        // Single return value
        r_return_value = state->to_variant(-1);
        lua_pop(L, 1);
    }
    else
    {
        // Multiple return values - return first and warn
        WARN_PRINT(vformat("LuaCallable: Lua function returned %d values, returning only the first.", nresults));
        r_return_value = state->to_variant(-nresults); // Get first return value
        lua_pop(L, nresults);                          // Pop all return values
    }
}

bool LuaCallable::compare_equal(const CallableCustom *p_a, const CallableCustom *p_b)
{
    const LuaCallable *a = static_cast<const LuaCallable *>(p_a);
    const LuaCallable *b = static_cast<const LuaCallable *>(p_b);

    // Two LuaCallables are equal if they reference the same function in the same state
    return a->lua_state_id == b->lua_state_id && a->func_ref == b->func_ref;
}

bool LuaCallable::compare_less(const CallableCustom *p_a, const CallableCustom *p_b)
{
    const LuaCallable *a = static_cast<const LuaCallable *>(p_a);
    const LuaCallable *b = static_cast<const LuaCallable *>(p_b);

    // Compare by state pointer first, then by func_ref
    if (a->lua_state_id != b->lua_state_id)
    {
        return a->lua_state_id < b->lua_state_id;
    }

    return a->func_ref < b->func_ref;
}

LuaState *LuaCallable::get_lua_state() const
{
    Object *obj = ObjectDB::get_instance(lua_state_id);
    if (!obj)
    {
        return nullptr;
    }

    return Object::cast_to<LuaState>(obj);
}
