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
        Ref<LuaState> main_thread; // only set for non-main threads

        // Private constructor for threads created by lua_newthread
        LuaState(lua_State *thread_L, Ref<LuaState> main_thread);

        void set_callbacks();

        // Helper to check if this state is still valid (checks if parent is closed for threads)
        bool is_valid_state() const;

        Ref<LuaState> bind_thread(lua_State *thread_L);

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

        void open_libs(int libs = LIB_ALL);
        void sandbox();
        void close();

        // Helper to open a single library via lua_call
        void open_library(lua_CFunction func, const char *name);

        lua_Status load_bytecode(const PackedByteArray &bytecode, const String &chunk_name);
        lua_Status load_string(const String &code, const String &chunk_name);
        lua_Status do_string(const String &code, const String &chunk_name);
        lua_Status resume(int narg = 0);

        void single_step(bool enable);
        void pause(); // a.k.a. break

        // Stack manipulation
        int get_top() const;
        void set_top(int index);
        bool check_stack(int extra);
        void pop(int n);
        void push_value(int index);
        void remove(int index);
        void insert(int index);
        void replace(int index);

        // Type checking
        bool is_nil(int index) const;
        bool is_number(int index) const;
        bool is_string(int index) const;
        bool is_table(int index) const;
        bool is_function(int index) const;
        bool is_userdata(int index) const;
        bool is_boolean(int index) const;
        bool is_thread(int index) const;
        int type(int index) const;
        String type_name(int type_id) const;

        // Value access
        double to_number(int index);
        int to_integer(int index);
        bool to_boolean(int index);

        // Push operations
        void push_nil();
        void push_number(double n);
        void push_integer(int n);
        void push_boolean(bool b);
        bool push_thread(); // returns true if the thread is the main thread

        // Table operations
        void new_table();
        void create_table(int narr, int nrec);
        void get_table(int index);
        void set_table(int index);
        void get_field(int index, const String &key);
        void set_field(int index, const String &key);
        void get_global(const String &key);
        void set_global(const String &key);
        void raw_get(int index);
        void raw_set(int index);
        void raw_geti(int index, int n);
        void raw_seti(int index, int n);

        // Metatable operations
        bool get_metatable(int index);
        bool set_metatable(int index);

        // Function calls
        void call(int nargs, int nresults);
        lua_Status pcall(int nargs, int nresults, int errfunc);

        // Thread operations
        Ref<LuaState> new_thread();
        Ref<LuaState> to_thread(int index);
        Ref<LuaState> get_main_thread();

        // Garbage collection
        int gc(lua_GCOp what, int data);

        // Godot integration helpers
        bool is_array(int index);      // requires manipulating the stack, so cannot be const
        bool is_dictionary(int index); // requires manipulating the stack, so cannot be const
        Array to_array(int index);
        Dictionary to_dictionary(int index);
        String to_string(int index);
        Variant to_variant(int index);
        void push_array(const Array &arr);
        void push_dictionary(const Dictionary &dict);
        void push_string(const String &s);
        void push_variant(const Variant &value);

        // Internal state accessor (for LuaCallable and other internal use)
        lua_State *get_lua_state() const;

        // Exposed inside Godot:
        //  signal step(state: LuaState)
    };
} // namespace godot

VARIANT_BITFIELD_CAST(godot::LuaState::LibraryFlags);
