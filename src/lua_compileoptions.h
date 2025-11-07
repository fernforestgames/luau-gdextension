#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <luacode.h>

namespace gdluau
{
    using namespace godot;

    class LuaCompileOptions : public RefCounted
    {
        GDCLASS(LuaCompileOptions, RefCounted)

    private:
        lua_CompileOptions options;

    protected:
        static void _bind_methods();

    public:
        LuaCompileOptions();

        void set_optimization_level(int p_level);
        int get_optimization_level() const;

        void set_debug_level(int p_level);
        int get_debug_level() const;

        void set_type_info_level(int p_level);
        int get_type_info_level() const;

        void set_coverage_level(int p_level);
        int get_coverage_level() const;

        static lua_CompileOptions default_options()
        {
            lua_CompileOptions options = {0};
            options.optimizationLevel = 1;
            options.debugLevel = 1;
            options.typeInfoLevel = 0;
            options.coverageLevel = 0;

            options.vectorLib = nullptr;
            options.vectorCtor = "Vector3";
            options.vectorType = "Vector3";

            return options;
        }

        const lua_CompileOptions &get_options() const { return options; }
    };
} // namespace gdluau
