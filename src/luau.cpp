#include "luau.h"

#include "helpers.h"
#include "lua_compileoptions.h"

#include <godot_cpp/core/class_db.hpp>
#include <luacode.h>

using namespace gdluau;
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

    BIND_CONSTANT(LUA_NOTAG);

    // luaconf.h constants
    BIND_CONSTANT(LUA_IDSIZE);
    BIND_CONSTANT(LUAI_MAXCSTACK);
    BIND_CONSTANT(LUAI_MAXCALLS);
    BIND_CONSTANT(LUA_UTAG_LIMIT);
    BIND_CONSTANT(LUA_LUTAG_LIMIT);
    BIND_CONSTANT(LUA_MEMORY_CATEGORIES);
    BIND_CONSTANT(LUA_MAXCAPTURES);
    BIND_CONSTANT(LUA_VECTOR_SIZE);

    ClassDB::bind_static_method(Luau::get_class_static(), D_METHOD("compile", "source_code", "options"), &Luau::compile, DEFVAL(nullptr));
    ClassDB::bind_static_method(Luau::get_class_static(), D_METHOD("upvalue_index", "upvalue"), &Luau::upvalue_index);
    ClassDB::bind_static_method(Luau::get_class_static(), D_METHOD("is_pseudo", "index"), &Luau::is_pseudo);
    ClassDB::bind_static_method(Luau::get_class_static(), D_METHOD("clock"), &Luau::clock);
}

PackedByteArray Luau::compile(const String &p_source_code, const LuaCompileOptions *p_options)
{
    CharString utf8 = p_source_code.utf8();
    lua_CompileOptions options = p_options ? p_options->get_options() : LuaCompileOptions::default_options();

    size_t bytecode_size;
    char *bytecode = luau_compile(utf8.get_data(), utf8.length(), &options, &bytecode_size);

    PackedByteArray result;
    result.resize(bytecode_size);
    memcpy(result.ptrw(), bytecode, bytecode_size);
    return result;
}

int Luau::upvalue_index(int p_upvalue)
{
    return lua_upvalueindex(p_upvalue);
}

bool Luau::is_pseudo(int p_index)
{
    return lua_ispseudo(p_index) != 0;
}

double Luau::clock()
{
    return lua_clock();
}
