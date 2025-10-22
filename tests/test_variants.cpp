// Tests for generic Variant <-> Lua value conversions
// Tests pushvariant() and tovariant() with all supported types

#include "doctest.h"
#include "lua_state.h"
#include "lua_godotlib.h"
#include "luau.h"
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/core/memory.hpp>
#include <lua.h>
#include <lualib.h>

using namespace godot;

// Helper to create a LuaState with all libs
static LuaState *create_test_state()
{
    LuaState *state = memnew(LuaState);
    state->openlibs(LuaState::LIB_ALL);
    return state;
}

static void close_test_state(LuaState *state)
{
    memdelete(state);
}

TEST_CASE("Variant: Primitive type conversions")
{
    LuaState *L = create_test_state();

    SUBCASE("Nil variant")
    {
        Variant nil_var;
        L->pushvariant(nil_var);

        CHECK(L->isnil(-1));

        Variant retrieved = L->tovariant(-1);
        CHECK(retrieved.get_type() == Variant::NIL);
    }

    SUBCASE("Boolean true")
    {
        Variant bool_var = true;
        L->pushvariant(bool_var);

        CHECK(L->isboolean(-1));
        CHECK(L->toboolean(-1) == true);

        Variant retrieved = L->tovariant(-1);
        CHECK(retrieved.get_type() == Variant::BOOL);
        CHECK((bool)retrieved == true);
    }

    SUBCASE("Boolean false")
    {
        Variant bool_var = false;
        L->pushvariant(bool_var);

        CHECK(L->isboolean(-1));
        CHECK(L->toboolean(-1) == false);

        Variant retrieved = L->tovariant(-1);
        CHECK((bool)retrieved == false);
    }

    SUBCASE("Integer")
    {
        Variant int_var = 42;
        L->pushvariant(int_var);

        CHECK(L->isnumber(-1));
        CHECK(L->tointeger(-1) == 42);

        Variant retrieved = L->tovariant(-1);
        // May be INT or FLOAT depending on Lua representation
        int value = retrieved;
        CHECK(value == 42);
    }

    SUBCASE("Float")
    {
        Variant float_var = 3.14159;
        L->pushvariant(float_var);

        CHECK(L->isnumber(-1));
        CHECK(L->tonumber(-1) == doctest::Approx(3.14159));

        Variant retrieved = L->tovariant(-1);
        double value = retrieved;
        CHECK(value == doctest::Approx(3.14159));
    }

    SUBCASE("Negative numbers")
    {
        Variant neg_int = -100;
        L->pushvariant(neg_int);

        Variant retrieved = L->tovariant(-1);
        int value = retrieved;
        CHECK(value == -100);
    }

    SUBCASE("Zero")
    {
        Variant zero = 0;
        L->pushvariant(zero);

        Variant retrieved = L->tovariant(-1);
        int value = retrieved;
        CHECK(value == 0);
    }

    close_test_state(L);
}

TEST_CASE("Variant: String conversions")
{
    LuaState *L = create_test_state();

    SUBCASE("Simple string")
    {
        Variant str_var = String("hello world");
        L->pushvariant(str_var);

        CHECK(L->isstring(-1));
        CHECK(L->tostring(-1) == "hello world");

        Variant retrieved = L->tovariant(-1);
        CHECK(retrieved.get_type() == Variant::STRING);
        CHECK((String)retrieved == "hello world");
    }

    SUBCASE("Empty string")
    {
        Variant empty = String("");
        L->pushvariant(empty);

        CHECK(L->isstring(-1));

        Variant retrieved = L->tovariant(-1);
        CHECK((String)retrieved == "");
    }

    SUBCASE("String with special characters")
    {
        Variant special = String("Hello\nWorld\t!");
        L->pushvariant(special);

        Variant retrieved = L->tovariant(-1);
        CHECK((String)retrieved == "Hello\nWorld\t!");
    }

    SUBCASE("Unicode string")
    {
        Variant unicode = String("こんにちは 世界 🌍");
        L->pushvariant(unicode);

        Variant retrieved = L->tovariant(-1);
        CHECK((String)retrieved == "こんにちは 世界 🌍");
    }

    SUBCASE("StringName")
    {
        Variant strname = StringName("test_name");
        L->pushvariant(strname);

        CHECK(L->isstring(-1));

        Variant retrieved = L->tovariant(-1);
        // Should convert to string
        String str = retrieved;
        CHECK(str == "test_name");
    }

    close_test_state(L);
}

TEST_CASE("Variant: Math type conversions")
{
    LuaState *L = create_test_state();

    SUBCASE("Vector2")
    {
        Variant vec = Vector2(3.5, 4.5);
        L->pushvariant(vec);
        CHECK(is_vector2((lua_State *)nullptr, -1));

        Variant retrieved = L->tovariant(-1);
        CHECK(retrieved.get_type() == Variant::VECTOR2);

        Vector2 v = retrieved;
        CHECK(v.x == doctest::Approx(3.5));
        CHECK(v.y == doctest::Approx(4.5));
    }

    SUBCASE("Vector2i")
    {
        Variant vec = Vector2i(10, 20);
        L->pushvariant(vec);

        Variant retrieved = L->tovariant(-1);
        CHECK(retrieved.get_type() == Variant::VECTOR2I);

        Vector2i v = retrieved;
        CHECK(v.x == 10);
        CHECK(v.y == 20);
    }

    SUBCASE("Vector3")
    {
        Variant vec = Vector3(1.0, 2.0, 3.0);
        L->pushvariant(vec);
        CHECK(is_vector3((lua_State *)nullptr, -1));

        Variant retrieved = L->tovariant(-1);
        CHECK(retrieved.get_type() == Variant::VECTOR3);

        Vector3 v = retrieved;
        CHECK(v.x == doctest::Approx(1.0));
        CHECK(v.y == doctest::Approx(2.0));
        CHECK(v.z == doctest::Approx(3.0));
    }

    SUBCASE("Vector3i")
    {
        Variant vec = Vector3i(100, 200, 300);
        L->pushvariant(vec);

        Variant retrieved = L->tovariant(-1);
        CHECK(retrieved.get_type() == Variant::VECTOR3I);

        Vector3i v = retrieved;
        CHECK(v.x == 100);
        CHECK(v.y == 200);
        CHECK(v.z == 300);
    }

    SUBCASE("Color")
    {
        Variant col = Color(1.0, 0.5, 0.0, 0.8);
        L->pushvariant(col);

        Variant retrieved = L->tovariant(-1);
        CHECK(retrieved.get_type() == Variant::COLOR);

        Color c = retrieved;
        CHECK(c.r == doctest::Approx(1.0));
        CHECK(c.g == doctest::Approx(0.5));
        CHECK(c.b == doctest::Approx(0.0));
        CHECK(c.a == doctest::Approx(0.8));
    }

    close_test_state(L);
}

TEST_CASE("Variant: Collection conversions")
{
    LuaState *L = create_test_state();

    SUBCASE("Array variant")
    {
        Array arr;
        arr.push_back(1);
        arr.push_back("two");
        arr.push_back(3.0);

        Variant arr_var = arr;
        L->pushvariant(arr_var);

        CHECK(L->istable(-1));

        Variant retrieved = L->tovariant(-1);
        CHECK(retrieved.get_type() == Variant::ARRAY);

        Array retrieved_arr = retrieved;
        CHECK(retrieved_arr.size() == 3);
        CHECK((int)retrieved_arr[0] == 1);
        CHECK((String)retrieved_arr[1] == "two");
        CHECK((double)retrieved_arr[2] == doctest::Approx(3.0));
    }

    SUBCASE("Dictionary variant")
    {
        Dictionary dict;
        dict["name"] = "test";
        dict["value"] = 42;

        Variant dict_var = dict;
        L->pushvariant(dict_var);

        CHECK(L->istable(-1));

        Variant retrieved = L->tovariant(-1);
        CHECK(retrieved.get_type() == Variant::DICTIONARY);

        Dictionary retrieved_dict = retrieved;
        CHECK((String)retrieved_dict["name"] == "test");
        CHECK((int)retrieved_dict["value"] == 42);
    }

    SUBCASE("Empty array")
    {
        Array empty;
        Variant arr_var = empty;

        L->pushvariant(arr_var);

        Variant retrieved = L->tovariant(-1);
        Array retrieved_arr = retrieved;
        CHECK(retrieved_arr.size() == 0);
    }

    SUBCASE("Empty dictionary")
    {
        Dictionary empty;
        Variant dict_var = empty;

        L->pushvariant(dict_var);

        Variant retrieved = L->tovariant(-1);
        Dictionary retrieved_dict = retrieved;
        CHECK(retrieved_dict.size() == 0);
    }

    close_test_state(L);
}

TEST_CASE("Variant: Nested structure conversions")
{
    LuaState *L = create_test_state();

    SUBCASE("Array containing arrays")
    {
        Array inner1;
        inner1.push_back(1);
        inner1.push_back(2);

        Array inner2;
        inner2.push_back(3);
        inner2.push_back(4);

        Array outer;
        outer.push_back(inner1);
        outer.push_back(inner2);

        Variant var = outer;
        L->pushvariant(var);

        Variant retrieved = L->tovariant(-1);
        Array retrieved_outer = retrieved;

        CHECK(retrieved_outer.size() == 2);

        Array r_inner1 = retrieved_outer[0];
        CHECK((int)r_inner1[0] == 1);
        CHECK((int)r_inner1[1] == 2);

        Array r_inner2 = retrieved_outer[1];
        CHECK((int)r_inner2[0] == 3);
        CHECK((int)r_inner2[1] == 4);
    }

    SUBCASE("Dictionary containing arrays and dictionaries")
    {
        Array items;
        items.push_back(10);
        items.push_back(20);

        Dictionary meta;
        meta["count"] = 2;

        Dictionary outer;
        outer["items"] = items;
        outer["meta"] = meta;
        outer["name"] = "complex";

        Variant var = outer;
        L->pushvariant(var);

        Variant retrieved = L->tovariant(-1);
        Dictionary retrieved_dict = retrieved;

        CHECK((String)retrieved_dict["name"] == "complex");

        Array r_items = retrieved_dict["items"];
        CHECK(r_items.size() == 2);
        CHECK((int)r_items[0] == 10);

        Dictionary r_meta = retrieved_dict["meta"];
        CHECK((int)r_meta["count"] == 2);
    }

    SUBCASE("Array with mixed types including math types")
    {
        Array arr;
        arr.push_back(42);
        arr.push_back(Vector2(1, 2));
        arr.push_back("text");
        arr.push_back(Color(1, 0, 0, 1));

        Variant var = arr;
        L->pushvariant(var);

        Variant retrieved = L->tovariant(-1);
        Array r_arr = retrieved;

        CHECK(r_arr.size() == 4);
        CHECK((int)r_arr[0] == 42);

        Vector2 v = r_arr[1];
        CHECK(v.x == doctest::Approx(1.0));
        CHECK(v.y == doctest::Approx(2.0));

        CHECK((String)r_arr[2] == "text");

        Color c = r_arr[3];
        CHECK(c.r == doctest::Approx(1.0));
        CHECK(c.a == doctest::Approx(1.0));
    }

    close_test_state(L);
}

TEST_CASE("Variant: Round-trip through Lua execution")
{
    LuaState *L = create_test_state();

    SUBCASE("Modify variant in Lua")
    {
        Array original;
        original.push_back(1);
        original.push_back(2);

        Variant var = original;
        L->pushvariant(var);
        L->setglobal("arr");

        const char *code = R"(
            table.insert(arr, 3)
            table.insert(arr, 4)
            return arr
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L->load_bytecode(bytecode, "test");
        L->resume();

        Variant result = L->tovariant(-1);
        Array result_arr = result;

        CHECK(result_arr.size() == 4);
        CHECK((int)result_arr[2] == 3);
        CHECK((int)result_arr[3] == 4);
    }

    SUBCASE("Create complex structure in Lua")
    {
        const char *code = R"(
            return {
                position = Vector2(100, 200),
                color = Color(1, 0, 0, 1),
                items = {10, 20, 30},
                metadata = {
                    name = "entity",
                    active = true
                }
            }
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L->load_bytecode(bytecode, "test");
        L->resume();

        Variant result = L->tovariant(-1);
        Dictionary dict = result;

        CHECK(dict.has("position"));
        CHECK(dict.has("color"));
        CHECK(dict.has("items"));
        CHECK(dict.has("metadata"));

        Vector2 pos = dict["position"];
        CHECK(pos.x == doctest::Approx(100));
        CHECK(pos.y == doctest::Approx(200));

        Array items = dict["items"];
        CHECK(items.size() == 3);
        CHECK((int)items[0] == 10);

        Dictionary meta = dict["metadata"];
        CHECK((String)meta["name"] == "entity");
        CHECK((bool)meta["active"] == true);
    }

    close_test_state(L);
}

TEST_CASE("Variant: Type edge cases")
{
    LuaState *L = create_test_state();

    SUBCASE("Very large integer")
    {
        Variant large = 2147483647; // Max 32-bit int
        L->pushvariant(large);

        Variant retrieved = L->tovariant(-1);
        int64_t value = retrieved;
        CHECK(value == 2147483647);
    }

    SUBCASE("Very small float")
    {
        Variant small = 0.000001;
        L->pushvariant(small);

        Variant retrieved = L->tovariant(-1);
        double value = retrieved;
        CHECK(value == doctest::Approx(0.000001));
    }

    SUBCASE("Negative zero")
    {
        Variant neg_zero = -0.0;
        L->pushvariant(neg_zero);

        Variant retrieved = L->tovariant(-1);
        double value = retrieved;
        // -0.0 should be preserved
        CHECK(value == doctest::Approx(0.0));
    }

    SUBCASE("Multiple nil variants")
    {
        L->pushvariant(Variant());
        L->pushvariant(Variant());
        L->pushvariant(Variant());

        CHECK(L->gettop() == 3);
        CHECK(L->isnil(-1));
        CHECK(L->isnil(-2));
        CHECK(L->isnil(-3));
    }

    close_test_state(L);
}
