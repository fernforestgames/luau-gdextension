#include "register_types.h"

#include "lua_state.h"
#include "luau.h"

#ifdef ENABLE_LUAU_GDEXTENSION_TESTS
#include "luau_gdextension_tests.h"
#endif

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <Luau/Common.h>

using namespace godot;

static int assertionHandler(const char *expr, const char *file, int line, const char *function)
{
    ERR_PRINT(vformat("Luau assertion failed: %s, function %s, file %s, line %d", expr, function, file, line));
    return 1;
}

void initialize_gdluau(ModuleInitializationLevel p_level)
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        return;
    }

    // We generally try to avoid using the Luau C++ API (in favor of the C API),
    // for maximum compatibility with base Lua, but this appears to be the only
    // way to install a custom assertion handler. Without this, assertions will
    // be checked but never logged.
    ::Luau::assertHandler() = assertionHandler;

    GDREGISTER_RUNTIME_CLASS(godot::Luau);
    GDREGISTER_RUNTIME_CLASS(LuaState);

#ifdef ENABLE_LUAU_GDEXTENSION_TESTS
#pragma message("Emitting code to register Luau GDExtension tests.")
    print_line("Luau GDExtension Tests enabled.");
    GDREGISTER_RUNTIME_CLASS(LuauGDExtensionTests);
#endif
}

void uninitialize_gdluau(ModuleInitializationLevel p_level)
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        return;
    }
}

extern "C"
{
    // Initialization.
    GDExtensionBool GDE_EXPORT gdluau_entrypoint(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization)
    {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(initialize_gdluau);
        init_obj.register_terminator(uninitialize_gdluau);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}
