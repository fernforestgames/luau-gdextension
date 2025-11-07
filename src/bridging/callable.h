#pragma once

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/callable_custom.hpp>

struct lua_State;

namespace gdluau
{
    using namespace godot;

    class LuaState;

    Callable to_callable(lua_State *p_L, int p_index);
    void push_callable(lua_State *p_L, const Callable &p_callable);

    // Custom Callable that wraps a Lua function.
    // Allows Lua functions to be called from Godot as Callables.
    class LuaCallable : public CallableCustom
    {
    private:
        ObjectID lua_state_id;   // Weak reference to LuaState
        Ref<LuaState> lua_state; // Optional strong reference to LuaState

        int func_ref;     // Reference to Lua function in registry
        String func_name; // Optional function name for debugging

    public:
        LuaCallable(LuaState *p_state, bool p_weak_state_ref, const String &p_func_name, int p_func_ref);
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

        LuaState *get_lua_state() const;
        int get_func_ref() const { return func_ref; }
    };
} // namespace gdluau
