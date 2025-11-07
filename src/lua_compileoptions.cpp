#include "lua_compileoptions.h"

using namespace gdluau;
using namespace godot;

LuaCompileOptions::LuaCompileOptions()
    : options{default_options()}
{
}

void LuaCompileOptions::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("set_optimization_level", "level"), &LuaCompileOptions::set_optimization_level);
    ClassDB::bind_method(D_METHOD("get_optimization_level"), &LuaCompileOptions::get_optimization_level);

    ClassDB::bind_method(D_METHOD("set_debug_level", "level"), &LuaCompileOptions::set_debug_level);
    ClassDB::bind_method(D_METHOD("get_debug_level"), &LuaCompileOptions::get_debug_level);

    ClassDB::bind_method(D_METHOD("set_type_info_level", "level"), &LuaCompileOptions::set_type_info_level);
    ClassDB::bind_method(D_METHOD("get_type_info_level"), &LuaCompileOptions::get_type_info_level);

    ClassDB::bind_method(D_METHOD("set_coverage_level", "level"), &LuaCompileOptions::set_coverage_level);
    ClassDB::bind_method(D_METHOD("get_coverage_level"), &LuaCompileOptions::get_coverage_level);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "optimization_level"), "set_optimization_level", "get_optimization_level");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "debug_level"), "set_debug_level", "get_debug_level");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "type_info_level"), "set_type_info_level", "get_type_info_level");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "coverage_level"), "set_coverage_level", "get_coverage_level");
}

void LuaCompileOptions::set_optimization_level(int p_level)
{
    options.optimizationLevel = p_level;
}

int LuaCompileOptions::get_optimization_level() const
{
    return options.optimizationLevel;
}

void LuaCompileOptions::set_debug_level(int p_level)
{
    options.debugLevel = p_level;
}

int LuaCompileOptions::get_debug_level() const
{
    return options.debugLevel;
}

void LuaCompileOptions::set_type_info_level(int p_level)
{
    options.typeInfoLevel = p_level;
}

int LuaCompileOptions::get_type_info_level() const
{
    return options.typeInfoLevel;
}

void LuaCompileOptions::set_coverage_level(int p_level)
{
    options.coverageLevel = p_level;
}

int LuaCompileOptions::get_coverage_level() const
{
    return options.coverageLevel;
}
