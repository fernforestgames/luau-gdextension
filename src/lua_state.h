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
        int step_count;
        int break_after_steps;

        friend void callback_debugstep(lua_State *L, lua_Debug *ar);

    protected:
        static void _bind_methods();

    public:
        LuaState();
        ~LuaState();

        void open_libs();
        void register_math_types();
        void close();

        int get_step_count() const;

        int get_break_after_steps() const;
        void set_break_after_steps(int steps);

        lua_Status load_bytecode(const PackedByteArray &bytecode, const String &chunk_name);
        lua_Status resume();

        void getglobal(const String &key);
        Variant to_variant(int index);
        void push_variant(const Variant &value);
    };
} // namespace godot
