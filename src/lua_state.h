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
        LuaState();
        ~LuaState();

        void open_libs();
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
