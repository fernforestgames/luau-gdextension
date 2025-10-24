#pragma once

#include <godot_cpp/variant/callable_custom.hpp>
#include <godot_cpp/variant/variant.hpp>
#include "lua_state.h"
#include <lua.h>
#include <gdextension_interface.h>

namespace godot
{
    // Custom Callable that wraps a Lua function
    // Allows Lua functions to be called from Godot as Callables
    class LuaCallable : public CallableCustom
    {
    private:
        LuaState *state;  // Raw pointer with manual reference counting
        int func_ref;     // Reference to Lua function in registry
        String func_name; // Optional function name for debugging

    public:
        LuaCallable(LuaState *p_state, const String &p_func_name, int p_func_ref);
        ~LuaCallable();

        // CallableCustom interface
        virtual uint32_t hash() const override;
        virtual String get_as_text() const override;
        virtual CompareEqualFunc get_compare_equal_func() const override;
        virtual CompareLessFunc get_compare_less_func() const override;
        virtual ObjectID get_object() const override;
        virtual bool is_valid() const override;
        virtual void call(const Variant **p_arguments, int p_argcount, Variant &r_return_value, GDExtensionCallError &r_call_error) const override;

        // Comparison functions
        static bool compare_equal(const CallableCustom *p_a, const CallableCustom *p_b);
        static bool compare_less(const CallableCustom *p_a, const CallableCustom *p_b);

        // Helper to get the underlying lua_State* for internal use
        lua_State *get_lua_state() const;
        int get_func_ref() const { return func_ref; }
    };

} // namespace godot
