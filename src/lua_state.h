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
            LIB_BASE = 1 << 0,      // Basic functions (_G, print, etc.)
            LIB_COROUTINE = 1 << 1, // Coroutine library
            LIB_TABLE = 1 << 2,     // Table manipulation
            LIB_OS = 1 << 3,        // OS library (use with caution in sandboxed environments)
            LIB_STRING = 1 << 4,    // String manipulation
            LIB_BIT32 = 1 << 5,     // Bit manipulation
            LIB_BUFFER = 1 << 6,    // Buffer library
            LIB_UTF8 = 1 << 7,      // UTF-8 support
            LIB_MATH = 1 << 8,      // Math functions
            LIB_DEBUG = 1 << 9,     // Debug library (use with caution)
            LIB_VECTOR = 1 << 10,   // Luau vector type
            LIB_GODOT = 1 << 11,    // Godot math types (Vector2, Vector3, Color, etc.)
            LIB_ALL = LIB_BASE | LIB_COROUTINE | LIB_TABLE | LIB_OS | LIB_STRING |
                      LIB_BIT32 | LIB_BUFFER | LIB_UTF8 | LIB_MATH | LIB_DEBUG | LIB_VECTOR | LIB_GODOT
        };

        LuaState();
        ~LuaState();

        void openlibs(int libs = LIB_ALL);
        void sandbox();
        void close();

        // Helper to open a single library via lua_call
        void open_library(lua_CFunction func, const char *name);

        lua_Status load_bytecode(const PackedByteArray &bytecode, const String &chunk_name);
        lua_Status loadstring(const String &code, const String &chunk_name);
        lua_Status dostring(const String &code, const String &chunk_name);
        lua_Status resume();

        void singlestep(bool enable);
        void pause(); // a.k.a. break

        // Stack manipulation
        int gettop() const;
        void settop(int index);
        void pop(int n);
        void pushvalue(int index);
        void remove(int index);
        void insert(int index);
        void replace(int index);

        // Type checking
        bool isnil(int index) const;
        bool isnumber(int index) const;
        bool isstring(int index) const;
        bool istable(int index) const;
        bool isfunction(int index) const;
        bool isuserdata(int index) const;
        bool isboolean(int index) const;
        bool isarray(int index);      // requires manipulating the stack, so cannot be const
        bool isdictionary(int index); // requires manipulating the stack, so cannot be const
        int type(int index) const;
        String type_name(int type_id) const;

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
        void getglobal(const String &key);
        void setglobal(const String &key);
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
        Array toarray(int index);
        Dictionary todictionary(int index);
        Variant tovariant(int index);
        void pusharray(const Array &arr);
        void pushdictionary(const Dictionary &dict);
        void pushvariant(const Variant &value);

        // Internal state accessor (for LuaCallable and other internal use)
        lua_State *get_lua_state() const { return L; }

        // Exposed inside Godot:
        //  signal step(state: LuaState)
    };
} // namespace godot

VARIANT_BITFIELD_CAST(godot::LuaState::LibraryFlags);
