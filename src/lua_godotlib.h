#pragma once

#include <lua.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define LUA_GODOTLIBNAME "godot"
    int luaopen_godot(lua_State *L);

#ifdef __cplusplus
}
#endif
