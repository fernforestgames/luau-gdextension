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
            LIB_OS = 1 << 3,        // OS library
            LIB_STRING = 1 << 4,    // String manipulation
            LIB_BIT32 = 1 << 5,     // Bit manipulation
            LIB_BUFFER = 1 << 6,    // Buffer library
            LIB_UTF8 = 1 << 7,      // UTF-8 support
            LIB_MATH = 1 << 8,      // Math functions
            LIB_DEBUG = 1 << 9,     // Debug library
            LIB_VECTOR = 1 << 10,   // Luau vector type
            LIB_GODOT = 1 << 11,    // Godot math types (Vector2, Vector3, Color, etc.)
            LIB_ALL = LIB_BASE | LIB_COROUTINE | LIB_TABLE | LIB_OS | LIB_STRING |
                      LIB_BIT32 | LIB_BUFFER | LIB_UTF8 | LIB_MATH | LIB_DEBUG | LIB_VECTOR | LIB_GODOT
        };

        LuaState();
        ~LuaState();

        void open_libs(int libs = LIB_ALL);
        void sandbox();
        void sandbox_thread();
        void close();

        // Helper to open a single library via lua_call
        void open_library(lua_CFunction func, const char *name);

        lua_Status load_bytecode(const PackedByteArray &bytecode, const String &chunk_name);
        lua_Status load_string(const String &code, const String &chunk_name);
        lua_Status do_string(const String &code, const String &chunk_name);
        lua_Status resume(int narg = 0);

        lua_Status status() const;
        void single_step(bool enable);
        void yield(int nresults);
        bool is_yieldable() const;
        void pause(); // a.k.a. break

        // Stack manipulation
        int abs_index(int index) const;
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
        lua_Type type(int index) const;
        String type_name(int type_id) const;

        // Value access
        double to_number(int index) const;
        int to_integer(int index) const;
        bool to_boolean(int index) const;
        int obj_len(int index);

        // Comparisons
        bool equal(int index1, int index2) const;
        bool raw_equal(int index1, int index2) const;
        bool less_than(int index1, int index2) const;

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
        void raw_get(int index) const;
        void raw_set(int index);
        void raw_get_field(int index, const String &key) const;
        void raw_set_field(int index, const String &key);
        void raw_geti(int index, int n) const;
        void raw_seti(int index, int n);
        bool get_read_only(int index) const;
        void set_read_only(int index, bool read_only);
        void get_fenv(int index);
        bool set_fenv(int index);
        bool next(int index);

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
        void reset_thread();
        bool is_thread_reset() const;
        void xmove(LuaState *to_state, int n);
        void xpush(LuaState *to_state, int index);
        lua_CoStatus co_status(LuaState *co);

        // Garbage collection
        int gc(lua_GCOp what, int data);
        int ref(int index);
        void get_ref(int ref);
        void unref(int ref);

        // Godot integration helpers
        bool is_array(int index);      // requires manipulating the stack, so cannot be const
        bool is_dictionary(int index); // requires manipulating the stack, so cannot be const
        Array to_array(int index);
        Dictionary to_dictionary(int index);
        String to_string(int index) const;
        Variant to_variant(int index);
        void push_array(const Array &arr);
        void push_dictionary(const Dictionary &dict);
        void push_string(const String &s);
        void push_variant(const Variant &value);

        // Godot math type wrappers (wrapping lua_godotlib functions)
        // Push operations
        void push_vector2(const Vector2 &value);
        void push_vector2i(const Vector2i &value);
        void push_vector3(const Vector3 &value);
        void push_vector3i(const Vector3i &value);
        void push_vector4(const Vector4 &value);
        void push_vector4i(const Vector4i &value);
        void push_rect2(const Rect2 &value);
        void push_rect2i(const Rect2i &value);
        void push_aabb(const AABB &value);
        void push_color(const Color &value);
        void push_plane(const Plane &value);
        void push_quaternion(const Quaternion &value);
        void push_basis(const Basis &value);
        void push_transform2d(const Transform2D &value);
        void push_transform3d(const Transform3D &value);
        void push_projection(const Projection &value);
        void push_callable(const Callable &value);

        // Type checking operations
        bool is_vector2(int index);
        bool is_vector2i(int index);
        bool is_vector3(int index);
        bool is_vector3i(int index);
        bool is_vector4(int index);
        bool is_vector4i(int index);
        bool is_rect2(int index);
        bool is_rect2i(int index);
        bool is_aabb(int index);
        bool is_color(int index);
        bool is_plane(int index);
        bool is_quaternion(int index);
        bool is_basis(int index);
        bool is_transform2d(int index);
        bool is_transform3d(int index);
        bool is_projection(int index);
        bool is_callable(int index);

        // Conversion operations
        Vector2 to_vector2(int index);
        Vector2i to_vector2i(int index);
        Vector3 to_vector3(int index);
        Vector3i to_vector3i(int index);
        Vector4 to_vector4(int index);
        Vector4i to_vector4i(int index);
        Rect2 to_rect2(int index);
        Rect2i to_rect2i(int index);
        AABB to_aabb(int index);
        Color to_color(int index);
        Plane to_plane(int index);
        Quaternion to_quaternion(int index);
        Basis to_basis(int index);
        Transform2D to_transform2d(int index);
        Transform3D to_transform3d(int index);
        Projection to_projection(int index);
        Callable to_callable(int index);

        // Internal state accessor (for LuaCallable and other internal use)
        lua_State *get_lua_state() const;

        // Exposed inside Godot:
        //  signal debugstep(state: LuaState)
        //  signal interrupt(state: LuaState, gc_state: int)
    };
} // namespace godot

VARIANT_BITFIELD_CAST(godot::LuaState::LibraryFlags);
