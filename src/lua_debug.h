#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <lua.h>

namespace gdluau
{
    using namespace godot;

    class LuaDebug : public RefCounted
    {
        GDCLASS(LuaDebug, RefCounted);

    private:
        lua_Debug debug_info;

    protected:
        static void _bind_methods();

    public:
        LuaDebug() {}
        LuaDebug(lua_Debug p_debug_info) : debug_info(p_debug_info) {}

        String get_name() const;
        String get_what() const;
        String get_source() const;
        String get_short_src() const;
        int get_line_defined() const;
        int get_current_line() const;
        uint8_t get_nupvals() const;
        uint8_t get_nparams() const;
        bool is_vararg() const;

        lua_Debug *ptrw() { return &debug_info; }
    };
} // namespace gdluau
