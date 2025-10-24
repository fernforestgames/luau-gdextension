#include "lua_callable.h"
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <lualib.h>

using namespace godot;

LuaCallable::LuaCallable(LuaState *p_state, const String &p_func_name, int p_func_ref)
    : state(p_state), func_name(p_func_name), func_ref(p_func_ref)
{
    // Manually increment reference count to keep LuaState alive
    if (state)
    {
        bool ref_success = state->reference();

        if (!ref_success)
        {
            // reference() returned false - object is being destroyed
            ERR_PRINT("LuaCallable: Failed to reference LuaState (object may be destroying)");
            state = nullptr;
        }
    }

    if (func_name.is_empty())
    {
        func_name = "<unknown>";
    }
}

LuaCallable::~LuaCallable()
{
    // Release the function reference from the Lua registry
    // Only do this if the lua_State* is still valid (user hasn't called close())
    if (state && func_ref != LUA_NOREF)
    {
        lua_State *L = get_lua_state();
        if (L)
        {
            lua_unref(L, func_ref);
        }
        // If L is null, the LuaState was closed but the object still exists
        // In this case we can't unref, but that's okay - the registry is gone anyway
    }

    // Manually decrement reference count
    // If unreference() returns true, we were the last reference and must free the object
    if (state)
    {
        bool should_delete = state->unreference();

        if (should_delete)
        {
            memdelete(state);
        }
    }
}

uint32_t LuaCallable::hash() const
{
    // Simple hash combining state pointer and func_ref
    uint64_t ptr_hash = (uint64_t)(uintptr_t)state;
    uint64_t combined = (ptr_hash << 32) | (uint32_t)func_ref;
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
    // No associated Object
    return ObjectID();
}

bool LuaCallable::is_valid() const
{
    // Callable is valid if we have a state and it hasn't been closed
    return state != nullptr && get_lua_state() != nullptr;
}

void LuaCallable::call(const Variant **p_arguments, int p_argcount, Variant &r_return_value, GDExtensionCallError &r_call_error) const
{
    r_call_error.error = GDEXTENSION_CALL_OK;
    r_return_value = Variant(); // Default to nil

    // Check if state is still valid
    if (!state)
    {
        ERR_PRINT(vformat("LuaCallable::call() - state pointer is null (refcount would be %d)", func_ref));
        r_call_error.error = GDEXTENSION_CALL_ERROR_INSTANCE_IS_NULL;
        return;
    }

    lua_State *L = get_lua_state();
    if (!L)
    {
        ERR_PRINT(vformat("LuaCallable::call() - lua_State* is null (LuaState was probably closed), refcount=%d", state->get_reference_count()));
        r_call_error.error = GDEXTENSION_CALL_ERROR_INSTANCE_IS_NULL;
        return;
    }

    // Save stack top before we start, so we can calculate results correctly
    // This is important for nested calls where the stack isn't empty
    int stack_top = lua_gettop(L);

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
        state->pushvariant(*p_arguments[i]);
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
        r_return_value = state->tovariant(-1);
        lua_pop(L, 1);
    }
    else
    {
        // Multiple return values - return first and warn
        WARN_PRINT(vformat("LuaCallable: Lua function returned %d values, returning only the first.", nresults));
        r_return_value = state->tovariant(-nresults); // Get first return value
        lua_pop(L, nresults);                         // Pop all return values
    }
}

bool LuaCallable::compare_equal(const CallableCustom *p_a, const CallableCustom *p_b)
{
    const LuaCallable *a = static_cast<const LuaCallable *>(p_a);
    const LuaCallable *b = static_cast<const LuaCallable *>(p_b);

    // Two LuaCallables are equal if they reference the same function in the same state
    return a->state == b->state && a->func_ref == b->func_ref;
}

bool LuaCallable::compare_less(const CallableCustom *p_a, const CallableCustom *p_b)
{
    const LuaCallable *a = static_cast<const LuaCallable *>(p_a);
    const LuaCallable *b = static_cast<const LuaCallable *>(p_b);

    // Compare by state pointer first, then by func_ref
    if (a->state != b->state)
    {
        return a->state < b->state;
    }
    return a->func_ref < b->func_ref;
}

lua_State *LuaCallable::get_lua_state() const
{
    if (!state)
    {
        return nullptr;
    }
    return state->get_lua_state();
}
