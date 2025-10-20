#pragma once

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
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

        void getglobal(const String &key);
        Variant to_variant(int index);
        void push_variant(const Variant &value);

        // Exposed inside Godot:
        //  signal step(state: LuaState)
    };
} // namespace godot
