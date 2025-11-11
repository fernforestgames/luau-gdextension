#include "weakly_bound_callable.h"

#include <godot_cpp/templates/hashfuncs.hpp>

using namespace gdluau;
using namespace godot;

uint32_t WeaklyBoundCallable::hash() const
{
    uint32_t h = HASH_MURMUR3_SEED;
    h = hash_murmur3_one_32(static_cast<uint32_t>(callable.hash()), h);
    h = hash_murmur3_one_64(static_cast<uint64_t>(bound_id), h);
    return hash_fmix32(h);
}

String WeaklyBoundCallable::get_as_text() const
{
    return vformat("WeaklyBoundCallable(%s, bound_id=%d)", callable, static_cast<uint64_t>(bound_id));
}

CallableCustom::CompareEqualFunc WeaklyBoundCallable::get_compare_equal_func() const
{
    return &WeaklyBoundCallable::compare_equal;
}

CallableCustom::CompareLessFunc WeaklyBoundCallable::get_compare_less_func() const
{
    return &WeaklyBoundCallable::compare_less;
}

ObjectID WeaklyBoundCallable::get_object() const
{
    return ObjectID(callable.get_object_id());
}

bool WeaklyBoundCallable::is_valid() const
{
    return callable.is_valid();
}

void WeaklyBoundCallable::call(const Variant **p_arguments, int p_argcount, Variant &r_return_value, GDExtensionCallError &r_call_error) const
{
    r_call_error.error = GDEXTENSION_CALL_OK;

    Array expanded_args;
    expanded_args.resize(p_argcount + 1);
    expanded_args[0] = get_ref();
    for (int i = 0; i < p_argcount; i++)
    {
        expanded_args[i + 1] = *p_arguments[i];
    }

    r_return_value = callable.callv(expanded_args);
}

bool WeaklyBoundCallable::compare_equal(const CallableCustom *p_a, const CallableCustom *p_b)
{
    const WeaklyBoundCallable *a = static_cast<const WeaklyBoundCallable *>(p_a);
    const WeaklyBoundCallable *b = static_cast<const WeaklyBoundCallable *>(p_b);
    return a->callable == b->callable && a->bound_id == b->bound_id;
}

bool WeaklyBoundCallable::compare_less(const CallableCustom *p_a, const CallableCustom *p_b)
{
    const WeaklyBoundCallable *a = static_cast<const WeaklyBoundCallable *>(p_a);
    const WeaklyBoundCallable *b = static_cast<const WeaklyBoundCallable *>(p_b);

    if (a->bound_id != b->bound_id)
    {
        return a->bound_id < b->bound_id;
    }

    if (a->callable != b->callable)
    {
        return a->callable.hash() < b->callable.hash();
    }

    return false;
}
