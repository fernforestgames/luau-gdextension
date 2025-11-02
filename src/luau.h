#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <lua.h>

namespace godot
{
    class Luau : public Object
    {
        GDCLASS(Luau, Object)

    private:
        static void _bind_methods();

    public:
        static PackedByteArray compile(const String &p_source_code);
        static int upvalue_index(int p_upvalue);
        static bool is_pseudo(int p_index);
        static double clock();
    };

} // namespace godot

VARIANT_ENUM_CAST(lua_Status);
VARIANT_ENUM_CAST(lua_CoStatus);
VARIANT_ENUM_CAST(lua_Type);
VARIANT_ENUM_CAST(lua_GCOp);
