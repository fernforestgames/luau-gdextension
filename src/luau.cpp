#include "luau.h"
#include <godot_cpp/core/class_db.hpp>
#include <luacode.h>

using namespace godot;

void Luau::_bind_methods()
{
    BIND_ENUM_CONSTANT(LUA_OK);
    BIND_ENUM_CONSTANT(LUA_YIELD);
    BIND_ENUM_CONSTANT(LUA_ERRRUN);
    BIND_ENUM_CONSTANT(LUA_ERRSYNTAX);
    BIND_ENUM_CONSTANT(LUA_ERRMEM);
    BIND_ENUM_CONSTANT(LUA_ERRERR);
    BIND_ENUM_CONSTANT(LUA_BREAK);

    ClassDB::bind_static_method(Luau::get_class_static(), D_METHOD("compile", "source_code"), &Luau::compile);
}

PackedByteArray Luau::compile(const String &source_code)
{
    CharString utf8 = source_code.utf8();

    lua_CompileOptions options = {0};
    // TODO: Expose compile options

    size_t bytecode_size;
    char *bytecode = luau_compile(utf8.get_data(), utf8.length(), &options, &bytecode_size);

    PackedByteArray result;
    result.resize(bytecode_size);
    memcpy(result.ptrw(), bytecode, bytecode_size);
    return result;
}
