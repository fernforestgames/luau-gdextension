#pragma once

struct lua_State;

#ifdef __cplusplus
extern "C"
{
#endif

#define LUA_GODOTLIBNAME "godot"
    // Registers conveniences for Godot bridging into Lua globals.
    //
    // For example, constructors (e.g., `Vector2()`) are defined for all Godot
    // Variant types that don't have a native Luau equivalent.
    int luaopen_godot(lua_State *L);

#ifdef __cplusplus
}
#endif
