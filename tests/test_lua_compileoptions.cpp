// Tests for LuaCompileOptions class

#include "doctest.h"
#include "test_fixtures.h"
#include "lua_compileoptions.h"
#include "luau.h"

using namespace godot;

TEST_SUITE("LuaCompileOptions")
{
    TEST_CASE("constructor - uses defaults")
    {
        LuaCompileOptions *opts = memnew(LuaCompileOptions);

        CHECK(opts->get_optimization_level() == 1);
        CHECK(opts->get_debug_level() == 1);
        CHECK(opts->get_type_info_level() == 0);
        CHECK(opts->get_coverage_level() == 0);

        memdelete(opts);
    }

    TEST_CASE("set_optimization_level")
    {
        LuaCompileOptions *opts = memnew(LuaCompileOptions);

        opts->set_optimization_level(0);
        CHECK(opts->get_optimization_level() == 0);

        opts->set_optimization_level(2);
        CHECK(opts->get_optimization_level() == 2);

        memdelete(opts);
    }

    TEST_CASE("set_debug_level")
    {
        LuaCompileOptions *opts = memnew(LuaCompileOptions);

        opts->set_debug_level(0);
        CHECK(opts->get_debug_level() == 0);

        opts->set_debug_level(2);
        CHECK(opts->get_debug_level() == 2);

        memdelete(opts);
    }

    TEST_CASE("set_type_info_level")
    {
        LuaCompileOptions *opts = memnew(LuaCompileOptions);

        opts->set_type_info_level(1);
        CHECK(opts->get_type_info_level() == 1);

        memdelete(opts);
    }

    TEST_CASE("set_coverage_level")
    {
        LuaCompileOptions *opts = memnew(LuaCompileOptions);

        opts->set_coverage_level(2);
        CHECK(opts->get_coverage_level() == 2);

        memdelete(opts);
    }

    TEST_CASE("use with Luau::compile")
    {
        LuaCompileOptions *opts = memnew(LuaCompileOptions);
        opts->set_optimization_level(2);
        opts->set_debug_level(2);

        String code = "return 42";
        PackedByteArray bytecode = Luau::compile(code, opts);

        CHECK(bytecode.size() > 0);

        memdelete(opts);
    }

    TEST_CASE("default_options - returns default config")
    {
        lua_CompileOptions defaults = LuaCompileOptions::default_options();

        CHECK(defaults.optimizationLevel == 1);
        CHECK(defaults.debugLevel == 1);
        CHECK(defaults.typeInfoLevel == 0);
        CHECK(defaults.coverageLevel == 0);
    }
}
