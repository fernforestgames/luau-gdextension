#include "register_types.h"

#include "lua_compileoptions.h"
#include "lua_debug.h"
#include "lua_state.h"
#include "luau.h"
#include "luau_script.h"
#include "static_strings.h"
#include "string_cache.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/resource_saver.hpp>
#include <Luau/Common.h>

using namespace gdluau;
using namespace godot;

static int assertionHandler(const char *expr, const char *file, int line, const char *function)
{
    ERR_PRINT(vformat("Luau assertion failed: %s, function %s, file %s, line %d", expr, function, file, line));
    return 1;
}

static Ref<ResourceFormatLoaderLuauScript> resource_loader_luau;
static Ref<ResourceFormatSaverLuauScript> resource_saver_luau;

void initialize_gdluau(ModuleInitializationLevel p_level)
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        return;
    }

    // Initialize statics (must be done after Godot is initialized, not during DLL static init)
    initialize_static_strings();
    initialize_string_cache();

    // We generally try to avoid using the Luau C++ API (in favor of the C API),
    // for maximum compatibility with base Lua, but this appears to be the only
    // way to install a custom assertion handler. Without this, assertions will
    // be checked but never logged.
    ::Luau::assertHandler() = assertionHandler;

    GDREGISTER_RUNTIME_CLASS(gdluau::Luau);
    GDREGISTER_RUNTIME_CLASS(LuaCompileOptions);
    GDREGISTER_RUNTIME_CLASS(LuaDebug);
    GDREGISTER_RUNTIME_CLASS(LuaState);
    GDREGISTER_RUNTIME_CLASS(LuauScript);
    GDREGISTER_RUNTIME_CLASS(ResourceFormatLoaderLuauScript);
    GDREGISTER_RUNTIME_CLASS(ResourceFormatSaverLuauScript);

    // Register resource loader and saver for .lua and .luau files
    resource_loader_luau.instantiate();
    ResourceLoader::get_singleton()->add_resource_format_loader(resource_loader_luau);

    resource_saver_luau.instantiate();
    ResourceSaver::get_singleton()->add_resource_format_saver(resource_saver_luau);
}

void uninitialize_gdluau(ModuleInitializationLevel p_level)
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        return;
    }

    // Unregister resource loader and saver
    ResourceLoader::get_singleton()->remove_resource_format_loader(resource_loader_luau);
    resource_loader_luau.unref();

    ResourceSaver::get_singleton()->remove_resource_format_saver(resource_saver_luau);
    resource_saver_luau.unref();

    // Cleanup statics
    uninitialize_string_cache();
    uninitialize_static_strings();
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
