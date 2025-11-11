#pragma once

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/callable_custom.hpp>

namespace gdluau
{
    using namespace godot;

    // Wraps another Callable, binding its last argument to a weak Object reference.
    // Note that this is different than holding a weak reference to the Callable's target object.
    class WeaklyBoundCallable : public CallableCustom
    {
    private:
        Callable callable;
        ObjectID bound_id; // Weak reference

    public:
        WeaklyBoundCallable(const Callable &p_callable, ObjectID p_bound_id)
            : callable(p_callable), bound_id(p_bound_id) {}

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

        Object *get_ref() const
        {
            return ObjectDB::get_instance(bound_id);
        }
    };
} // namespace gdluau
