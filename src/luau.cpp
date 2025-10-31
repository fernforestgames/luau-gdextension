#include "luau.h"
#include <godot_cpp/core/class_db.hpp>
#include <luacode.h>

using namespace godot;

void Luau::_bind_methods()
{
    BIND_CONSTANT(LUA_MULTRET);

    BIND_CONSTANT(LUA_REGISTRYINDEX);
    BIND_CONSTANT(LUA_ENVIRONINDEX);
    BIND_CONSTANT(LUA_GLOBALSINDEX);

    BIND_ENUM_CONSTANT(LUA_OK);
    BIND_ENUM_CONSTANT(LUA_YIELD);
    BIND_ENUM_CONSTANT(LUA_ERRRUN);
    BIND_ENUM_CONSTANT(LUA_ERRSYNTAX);
    BIND_ENUM_CONSTANT(LUA_ERRMEM);
    BIND_ENUM_CONSTANT(LUA_ERRERR);
    BIND_ENUM_CONSTANT(LUA_BREAK);

    BIND_ENUM_CONSTANT(LUA_CORUN);
    BIND_ENUM_CONSTANT(LUA_COSUS);
    BIND_ENUM_CONSTANT(LUA_CONOR);
    BIND_ENUM_CONSTANT(LUA_COFIN);
    BIND_ENUM_CONSTANT(LUA_COERR);

    BIND_CONSTANT(LUA_TNONE);

    BIND_ENUM_CONSTANT(LUA_TNIL);
    BIND_ENUM_CONSTANT(LUA_TBOOLEAN);
    BIND_ENUM_CONSTANT(LUA_TLIGHTUSERDATA);
    BIND_ENUM_CONSTANT(LUA_TNUMBER);
    BIND_ENUM_CONSTANT(LUA_TVECTOR);
    BIND_ENUM_CONSTANT(LUA_TSTRING);
    BIND_ENUM_CONSTANT(LUA_TTABLE);
    BIND_ENUM_CONSTANT(LUA_TFUNCTION);
    BIND_ENUM_CONSTANT(LUA_TUSERDATA);
    BIND_ENUM_CONSTANT(LUA_TTHREAD);
    BIND_ENUM_CONSTANT(LUA_TBUFFER);
    BIND_ENUM_CONSTANT(LUA_TPROTO);
    BIND_ENUM_CONSTANT(LUA_TUPVAL);
    BIND_ENUM_CONSTANT(LUA_TDEADKEY);
    BIND_ENUM_CONSTANT(LUA_T_COUNT);

    BIND_ENUM_CONSTANT(LUA_GCSTOP);
    BIND_ENUM_CONSTANT(LUA_GCRESTART);
    BIND_ENUM_CONSTANT(LUA_GCCOLLECT);
    BIND_ENUM_CONSTANT(LUA_GCCOUNT);
    BIND_ENUM_CONSTANT(LUA_GCCOUNTB);
    BIND_ENUM_CONSTANT(LUA_GCISRUNNING);
    BIND_ENUM_CONSTANT(LUA_GCSTEP);
    BIND_ENUM_CONSTANT(LUA_GCSETGOAL);
    BIND_ENUM_CONSTANT(LUA_GCSETSTEPMUL);
    BIND_ENUM_CONSTANT(LUA_GCSETSTEPSIZE);

    BIND_CONSTANT(LUA_NOREF);
    BIND_CONSTANT(LUA_REFNIL);

    ClassDB::bind_static_method(Luau::get_class_static(), D_METHOD("compile", "source_code"), &Luau::compile);
    ClassDB::bind_static_method(Luau::get_class_static(), D_METHOD("upvalue_index", "i"), &Luau::upvalue_index);
    ClassDB::bind_static_method(Luau::get_class_static(), D_METHOD("is_pseudo", "index"), &Luau::is_pseudo);
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

int Luau::upvalue_index(int i)
{
    return lua_upvalueindex(i);
}

bool Luau::is_pseudo(int index)
{
    return lua_ispseudo(index) != 0;
}
