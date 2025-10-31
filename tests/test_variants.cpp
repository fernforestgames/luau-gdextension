// Tests for generic Variant <-> Lua value conversions
// Tests pushvariant() and tovariant() with all supported types

#include "doctest.h"
#include "test_fixtures.h"
#include "lua_godotlib.h"
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/color.hpp>

using namespace godot;

TEST_CASE_FIXTURE(LuaStateFixture, "Variant: Primitive type conversions")
{

    SUBCASE("Nil variant")
    {
        Variant nil_var;
        state->push_variant(nil_var);

        CHECK(state->is_nil(-1));

        Variant retrieved = state->to_variant(-1);
        CHECK(retrieved.get_type() == Variant::NIL);

        state->pop(1); // Pop the nil value
    }

    SUBCASE("Boolean true")
    {
        Variant bool_var = true;
        state->push_variant(bool_var);

        CHECK(state->is_boolean(-1));
        CHECK(state->to_boolean(-1) == true);

        Variant retrieved = state->to_variant(-1);
        CHECK(retrieved.get_type() == Variant::BOOL);
        CHECK((bool)retrieved == true);

        state->pop(1); // Pop the boolean
    }

    SUBCASE("Boolean false")
    {
        Variant bool_var = false;
        state->push_variant(bool_var);

        CHECK(state->is_boolean(-1));
        CHECK(state->to_boolean(-1) == false);

        Variant retrieved = state->to_variant(-1);
        CHECK((bool)retrieved == false);

        state->pop(1); // Pop the boolean
    }

    SUBCASE("Integer")
    {
        Variant int_var = 42;
        state->push_variant(int_var);

        CHECK(state->is_number(-1));
        CHECK(state->to_integer(-1) == 42);

        Variant retrieved = state->to_variant(-1);
        // May be INT or FLOAT depending on Lua representation
        int value = retrieved;
        CHECK(value == 42);

        state->pop(1); // Pop the number
    }

    SUBCASE("Float")
    {
        Variant float_var = 3.14159;
        state->push_variant(float_var);

        CHECK(state->is_number(-1));
        CHECK(state->to_number(-1) == doctest::Approx(3.14159));

        Variant retrieved = state->to_variant(-1);
        double value = retrieved;
        CHECK(value == doctest::Approx(3.14159));

        state->pop(1); // Pop the number
    }

    SUBCASE("Negative numbers")
    {
        Variant neg_int = -100;
        state->push_variant(neg_int);

        Variant retrieved = state->to_variant(-1);
        int value = retrieved;
        CHECK(value == -100);

        state->pop(1); // Pop the number
    }

    SUBCASE("Zero")
    {
        Variant zero = 0;
        state->push_variant(zero);

        Variant retrieved = state->to_variant(-1);
        int value = retrieved;
        CHECK(value == 0);

        state->pop(1); // Pop the number
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Variant: String conversions")
{

    SUBCASE("Simple string")
    {
        Variant str_var = String("hello world");
        state->push_variant(str_var);

        CHECK(state->is_string(-1));
        CHECK(state->to_string(-1) == "hello world");

        Variant retrieved = state->to_variant(-1);
        CHECK(retrieved.get_type() == Variant::STRING);
        CHECK((String)retrieved == "hello world");

        state->pop(1); // Pop the string
    }

    SUBCASE("Empty string")
    {
        Variant empty = String("");
        state->push_variant(empty);

        CHECK(state->is_string(-1));

        Variant retrieved = state->to_variant(-1);
        CHECK((String)retrieved == "");

        state->pop(1); // Pop the string
    }

    SUBCASE("String with special characters")
    {
        Variant special = String("Hello\nWorld\t!");
        state->push_variant(special);

        Variant retrieved = state->to_variant(-1);
        CHECK((String)retrieved == "Hello\nWorld\t!");

        state->pop(1); // Pop the string
    }

    SUBCASE("Unicode string")
    {
        Variant unicode = String("こんにちは 世界 🌍");
        state->push_variant(unicode);

        Variant retrieved = state->to_variant(-1);
        CHECK((String)retrieved == "こんにちは 世界 🌍");

        state->pop(1); // Pop the string
    }

    SUBCASE("StringName")
    {
        // StringName converts to Lua string (for performance)
        // On round-trip, it becomes a String, not StringName
        Variant strname = StringName("test_name");
        state->push_variant(strname);

        CHECK(state->is_string(-1));

        // Round-trip converts StringName → String
        Variant retrieved = state->to_variant(-1);
        CHECK(retrieved.get_type() == Variant::STRING);
        CHECK((String)retrieved == "test_name");

        state->pop(1); // Pop the first StringName

        // Empty StringName
        state->push_variant(Variant(StringName()));
        CHECK(state->is_string(-1));
        CHECK((String)state->to_variant(-1) == "");

        state->pop(1); // Pop the empty StringName

        // StringName with special characters
        state->push_variant(Variant(StringName("_ready")));
        CHECK((String)state->to_variant(-1) == "_ready");

        state->pop(1); // Pop the StringName with special chars
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Variant: Math type conversions")
{

    SUBCASE("Vector2")
    {
        Variant vec = Vector2(3.5, 4.5);
        state->push_variant(vec);
        CHECK(is_vector2(L, -1));

        Variant retrieved = state->to_variant(-1);
        CHECK(retrieved.get_type() == Variant::VECTOR2);

        Vector2 v = retrieved;
        CHECK(v.x == doctest::Approx(3.5));
        CHECK(v.y == doctest::Approx(4.5));

        state->pop(1); // Pop the Vector2 userdata
    }

    SUBCASE("Vector2i")
    {
        Variant vec = Vector2i(10, 20);
        state->push_variant(vec);

        Variant retrieved = state->to_variant(-1);
        CHECK(retrieved.get_type() == Variant::VECTOR2I);

        Vector2i v = retrieved;
        CHECK(v.x == 10);
        CHECK(v.y == 20);

        state->pop(1); // Pop the Vector2i userdata
    }

    SUBCASE("Vector3")
    {
        Variant vec = Vector3(1.0, 2.0, 3.0);
        state->push_variant(vec);
        CHECK(is_vector3(L, -1));

        Variant retrieved = state->to_variant(-1);
        CHECK(retrieved.get_type() == Variant::VECTOR3);

        Vector3 v = retrieved;
        CHECK(v.x == doctest::Approx(1.0));
        CHECK(v.y == doctest::Approx(2.0));
        CHECK(v.z == doctest::Approx(3.0));

        state->pop(1); // Pop the Vector3 (native vector type)
    }

    SUBCASE("Vector3i")
    {
        Variant vec = Vector3i(100, 200, 300);
        state->push_variant(vec);

        Variant retrieved = state->to_variant(-1);
        CHECK(retrieved.get_type() == Variant::VECTOR3I);

        Vector3i v = retrieved;
        CHECK(v.x == 100);
        CHECK(v.y == 200);
        CHECK(v.z == 300);

        state->pop(1); // Pop the Vector3i userdata
    }

    SUBCASE("Color")
    {
        Variant col = Color(1.0, 0.5, 0.0, 0.8);
        state->push_variant(col);

        Variant retrieved = state->to_variant(-1);
        CHECK(retrieved.get_type() == Variant::COLOR);

        Color c = retrieved;
        CHECK(c.r == doctest::Approx(1.0));
        CHECK(c.g == doctest::Approx(0.5));
        CHECK(c.b == doctest::Approx(0.0));
        CHECK(c.a == doctest::Approx(0.8));

        state->pop(1); // Pop the Color userdata
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Variant: Collection conversions")
{

    SUBCASE("Array variant")
    {
        Array arr;
        arr.push_back(1);
        arr.push_back("two");
        arr.push_back(3.0);

        Variant arr_var = arr;
        state->push_variant(arr_var);

        CHECK(state->is_table(-1));

        Variant retrieved = state->to_variant(-1);
        CHECK(retrieved.get_type() == Variant::ARRAY);

        Array retrieved_arr = retrieved;
        CHECK(retrieved_arr.size() == 3);
        CHECK((int)retrieved_arr[0] == 1);
        CHECK((String)retrieved_arr[1] == "two");
        CHECK((double)retrieved_arr[2] == doctest::Approx(3.0));

        state->pop(1); // Pop the table
    }

    SUBCASE("Dictionary variant")
    {
        Dictionary dict;
        dict["name"] = "test";
        dict["value"] = 42;

        Variant dict_var = dict;
        state->push_variant(dict_var);

        CHECK(state->is_table(-1));

        Variant retrieved = state->to_variant(-1);
        CHECK(retrieved.get_type() == Variant::DICTIONARY);

        Dictionary retrieved_dict = retrieved;
        CHECK((String)retrieved_dict["name"] == "test");
        CHECK((int)retrieved_dict["value"] == 42);

        state->pop(1); // Pop the table
    }

    SUBCASE("Empty array")
    {
        Array empty;
        Variant arr_var = empty;

        state->push_variant(arr_var);

        Variant retrieved = state->to_variant(-1);
        Array retrieved_arr = retrieved;
        CHECK(retrieved_arr.size() == 0);

        state->pop(1); // Pop the empty table
    }

    SUBCASE("Empty dictionary")
    {
        Dictionary empty;
        Variant dict_var = empty;

        state->push_variant(dict_var);

        Variant retrieved = state->to_variant(-1);
        Dictionary retrieved_dict = retrieved;
        CHECK(retrieved_dict.size() == 0);

        state->pop(1); // Pop the empty table
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Variant: Nested structure conversions")
{

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
        state->push_variant(var);

        Variant retrieved = state->to_variant(-1);
        Array retrieved_outer = retrieved;

        CHECK(retrieved_outer.size() == 2);

        Array r_inner1 = retrieved_outer[0];
        CHECK((int)r_inner1[0] == 1);
        CHECK((int)r_inner1[1] == 2);

        Array r_inner2 = retrieved_outer[1];
        CHECK((int)r_inner2[0] == 3);
        CHECK((int)r_inner2[1] == 4);

        state->pop(1); // Pop the nested array table
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
        state->push_variant(var);

        Variant retrieved = state->to_variant(-1);
        Dictionary retrieved_dict = retrieved;

        CHECK((String)retrieved_dict["name"] == "complex");

        Array r_items = retrieved_dict["items"];
        CHECK(r_items.size() == 2);
        CHECK((int)r_items[0] == 10);

        Dictionary r_meta = retrieved_dict["meta"];
        CHECK((int)r_meta["count"] == 2);

        state->pop(1); // Pop the nested dictionary table
    }

    SUBCASE("Array with mixed types including math types")
    {
        Array arr;
        arr.push_back(42);
        arr.push_back(Vector2(1, 2));
        arr.push_back("text");
        arr.push_back(Color(1, 0, 0, 1));

        Variant var = arr;
        state->push_variant(var);

        Variant retrieved = state->to_variant(-1);
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

        state->pop(1); // Pop the mixed type array table
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Variant: Round-trip through Lua execution")
{

    SUBCASE("Modify variant in Lua")
    {
        Array original;
        original.push_back(1);
        original.push_back(2);

        Variant var = original;
        state->push_variant(var);
        state->set_global("arr");

        const char *code = R"(
            table.insert(arr, 3)
            table.insert(arr, 4)
            return arr
        )";

        exec_lua(code);

        Variant result = state->to_variant(-1);
        Array result_arr = result;

        CHECK(result_arr.size() == 4);
        CHECK((int)result_arr[2] == 3);
        CHECK((int)result_arr[3] == 4);

        state->pop(1); // Pop the return value from exec_lua
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

        exec_lua(code);

        Variant result = state->to_variant(-1);
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

        state->pop(1); // Pop the return value from exec_lua
    }
}

TEST_CASE_FIXTURE(LuaStateFixture, "Variant: Type edge cases")
{

    SUBCASE("Very large integer")
    {
        Variant large = 2147483647; // Max 32-bit int
        state->push_variant(large);

        Variant retrieved = state->to_variant(-1);
        int64_t value = retrieved;
        CHECK(value == 2147483647);

        state->pop(1); // Pop the number
    }

    SUBCASE("Very small float")
    {
        Variant small = 0.000001;
        state->push_variant(small);

        Variant retrieved = state->to_variant(-1);
        double value = retrieved;
        CHECK(value == doctest::Approx(0.000001));

        state->pop(1); // Pop the number
    }

    SUBCASE("Negative zero")
    {
        Variant neg_zero = -0.0;
        state->push_variant(neg_zero);

        Variant retrieved = state->to_variant(-1);
        double value = retrieved;
        // -0.0 should be preserved
        CHECK(value == doctest::Approx(0.0));

        state->pop(1); // Pop the number
    }

    SUBCASE("Multiple nil variants")
    {
        state->push_variant(Variant());
        state->push_variant(Variant());
        state->push_variant(Variant());

        CHECK(state->get_top() == 3);
        CHECK(state->is_nil(-1));
        CHECK(state->is_nil(-2));
        CHECK(state->is_nil(-3));

        state->pop(3); // Pop all three nil values
    }
}
