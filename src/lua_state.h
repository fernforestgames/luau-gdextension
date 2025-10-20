#pragma once

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <lua.h>

namespace godot
{
    class LuaState : public RefCounted
    {
        GDCLASS(LuaState, RefCounted)

    private:
        lua_State *L;

    protected:
        static void _bind_methods();

    public:
        // Library selection flags for open_libs()
        enum LibraryFlags
        {
            LIB_NONE = 0,
            LIB_BASE = 1 << 0,       // Basic functions (_G, print, etc.)
            LIB_COROUTINE = 1 << 1,  // Coroutine library
            LIB_TABLE = 1 << 2,      // Table manipulation
            LIB_OS = 1 << 3,         // OS library (use with caution in sandboxed environments)
            LIB_STRING = 1 << 4,     // String manipulation
            LIB_BIT32 = 1 << 5,      // Bit manipulation
            LIB_BUFFER = 1 << 6,     // Buffer library
            LIB_UTF8 = 1 << 7,       // UTF-8 support
            LIB_MATH = 1 << 8,       // Math functions
            LIB_DEBUG = 1 << 9,      // Debug library (use with caution)
            LIB_VECTOR = 1 << 10,    // Luau vector type
            LIB_ALL = LIB_BASE | LIB_COROUTINE | LIB_TABLE | LIB_OS | LIB_STRING |
                      LIB_BIT32 | LIB_BUFFER | LIB_UTF8 | LIB_MATH | LIB_DEBUG | LIB_VECTOR
        };

        LuaState();
        ~LuaState();

        void openlibs(int libs = LIB_ALL);
        void register_math_types();
        void close();

        lua_Status load_bytecode(const PackedByteArray &bytecode, const String &chunk_name);
        lua_Status resume();

        void singlestep(bool enable);
        void pause(); // a.k.a. break

        // Stack manipulation
        int gettop();
        void settop(int index);
        void pop(int n);
        void pushvalue(int index);
        void remove(int index);
        void insert(int index);
        void replace(int index);

        // Type checking
        bool isnil(int index);
        bool isnumber(int index);
        bool isstring(int index);
        bool istable(int index);
        bool isfunction(int index);
        bool isuserdata(int index);
        bool isboolean(int index);
        int type(int index);
        String type_name(int type_id);

        // Value access
        String tostring(int index);
        double tonumber(int index);
        int tointeger(int index);
        bool toboolean(int index);

        // Push operations
        void pushnil();
        void pushnumber(double n);
        void pushinteger(int n);
        void pushstring(const String &s);
        void pushboolean(bool b);

        // Table operations
        void newtable();
        void createtable(int narr, int nrec);
        void gettable(int index);
        void settable(int index);
        void getfield(int index, const String &key);
        void setfield(int index, const String &key);
        void rawget(int index);
        void rawset(int index);
        void rawgeti(int index, int n);
        void rawseti(int index, int n);

        // Metatable operations
        bool getmetatable(int index);
        bool setmetatable(int index);

        // Function calls
        void call(int nargs, int nresults);
        lua_Status pcall(int nargs, int nresults, int errfunc);

        // Garbage collection
        int gc(lua_GCOp what, int data);

        // Godot integration helpers
        void getglobal(const String &key);
        Variant tovariant(int index);
        void pushvariant(const Variant &value);

        // Exposed inside Godot:
        //  signal step(state: LuaState)
    };
} // namespace godot

VARIANT_BITFIELD_CAST(godot::LuaState::LibraryFlags);
