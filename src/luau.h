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
        static PackedByteArray compile(const String &source_code);
    };

} // namespace godot

VARIANT_ENUM_CAST(lua_Status);
