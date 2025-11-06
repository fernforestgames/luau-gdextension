#include "lua_debug.h"

using namespace godot;

void LuaDebug::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_name"), &LuaDebug::get_name);
    ClassDB::bind_method(D_METHOD("get_what"), &LuaDebug::get_what);
    ClassDB::bind_method(D_METHOD("get_source"), &LuaDebug::get_source);
    ClassDB::bind_method(D_METHOD("get_short_src"), &LuaDebug::get_short_src);
    ClassDB::bind_method(D_METHOD("get_line_defined"), &LuaDebug::get_line_defined);
    ClassDB::bind_method(D_METHOD("get_current_line"), &LuaDebug::get_current_line);
    ClassDB::bind_method(D_METHOD("get_nupvals"), &LuaDebug::get_nupvals);
    ClassDB::bind_method(D_METHOD("get_nparams"), &LuaDebug::get_nparams);
    ClassDB::bind_method(D_METHOD("is_vararg"), &LuaDebug::is_vararg);
}

String LuaDebug::get_name() const
{
    return String(debug_info.name);
}

String LuaDebug::get_what() const
{
    return String(debug_info.what);
}

String LuaDebug::get_source() const
{
    return String(debug_info.source);
}

String LuaDebug::get_short_src() const
{
    return String(debug_info.short_src);
}

int LuaDebug::get_line_defined() const
{
    return debug_info.linedefined;
}

int LuaDebug::get_current_line() const
{
    return debug_info.currentline;
}

uint8_t LuaDebug::get_nupvals() const
{
    return static_cast<uint8_t>(debug_info.nupvals);
}

uint8_t LuaDebug::get_nparams() const
{
    return static_cast<uint8_t>(debug_info.nparams);
}

bool LuaDebug::is_vararg() const
{
    return debug_info.isvararg != 0;
}
