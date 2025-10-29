// Tests for Godot Dictionary <-> Lua table bridging
// Tests dictionary conversions, nested dictionaries, and key handling

#include "doctest.h"
#include "test_fixtures.h"
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/variant.hpp>

using namespace godot;

TEST_CASE_FIXTURE(LuaStateFixture, "Dictionary: Simple dictionary Godot -> Lua")
{
    SUBCASE("String keys")
    {
        Dictionary dict;
        dict["name"] = "test";
        dict["value"] = 42;
        dict["active"] = true;

        state->push_dictionary(dict);
        state->set_global("d");

        // Access values from Lua
        state->get_global("d");
        state->push_string("name");
        state->get_table(-2);
        CHECK(state->to_string(-1) == "test");
        state->pop(1);

        state->push_string("value");
        state->get_table(-2);
        CHECK(state->to_integer(-1) == 42);
        state->pop(1);

        state->push_string("active");
        state->get_table(-2);
        CHECK(state->to_boolean(-1) == true);
    }

    SUBCASE("Integer keys")
    {
        Dictionary dict;
        dict[1] = "one";
        dict[2] = "two";
        dict[100] = "hundred";

        state->push_dictionary(dict);
        state->set_global("d");

        const char *code = R"(
            v1 = d[1]
            v2 = d[2]
            v100 = d[100]
        )";

        exec_lua(code);

        state->get_global("v1");
        CHECK(state->to_string(-1) == "one");
        state->pop(1);

        state->get_global("v100");
        CHECK(state->to_string(-1) == "hundred");
    }

    SUBCASE("Empty dictionary")
    {
        Dictionary dict;

        state->push_dictionary(dict);
        CHECK(state->is_table(-1));

        // Empty table should have no keys
        const char *code = R"(
            count = 0
            for k, v in pairs(d) do
                count = count + 1
            end
            return count
        )";

        state->set_global("d");

        exec_lua(code);

        CHECK(state->to_integer(-1) == 0);
    }

}

TEST_CASE_FIXTURE(LuaStateFixture, "Dictionary: Lua table -> Godot Dictionary")
{

    SUBCASE("Simple string-keyed table")
    {
        const char *code = R"(
            return {
                name = "Alice",
                age = 30,
                active = true
            }
        )";

        exec_lua(code);

        Dictionary dict = state->to_dictionary(-1);

        CHECK(dict.has("name"));
        CHECK((String)dict["name"] == "Alice");
        CHECK((int)dict["age"] == 30);
        CHECK((bool)dict["active"] == true);
    }

    SUBCASE("Integer-keyed table")
    {
        const char *code = R"(
            return {
                [10] = "ten",
                [20] = "twenty",
                [30] = "thirty"
            }
        )";

        exec_lua(code);

        Dictionary dict = state->to_dictionary(-1);

        CHECK(dict.has(10));
        CHECK((String)dict[10] == "ten");
        CHECK((String)dict[20] == "twenty");
        CHECK((String)dict[30] == "thirty");
    }

    SUBCASE("Mixed key types")
    {
        const char *code = R"(
            return {
                name = "test",
                [1] = "first",
                [2] = "second",
                count = 99
            }
        )";

        exec_lua(code);

        Dictionary dict = state->to_dictionary(-1);

        CHECK(dict.has("name"));
        CHECK(dict.has(1));
        CHECK(dict.has(2));
        CHECK(dict.has("count"));

        CHECK((String)dict["name"] == "test");
        CHECK((String)dict[1] == "first");
        CHECK((int)dict["count"] == 99);
    }

}

TEST_CASE_FIXTURE(LuaStateFixture, "Dictionary: Nested dictionaries")
{

    SUBCASE("Dictionary containing dictionary")
    {
        Dictionary inner;
        inner["x"] = 10;
        inner["y"] = 20;

        Dictionary outer;
        outer["position"] = inner;
        outer["name"] = "entity";

        state->push_dictionary(outer);
        state->set_global("entity");

        const char *code = R"(
            pos = entity.position
            x = pos.x
            y = pos.y
            name = entity.name
        )";

        exec_lua(code);

        state->get_global("x");
        CHECK(state->to_integer(-1) == 10);
        state->pop(1);

        state->get_global("y");
        CHECK(state->to_integer(-1) == 20);
        state->pop(1);

        state->get_global("name");
        CHECK(state->to_string(-1) == "entity");
    }

    SUBCASE("Lua nested table to Dictionary")
    {
        const char *code = R"(
            return {
                user = {
                    name = "Bob",
                    age = 25
                },
                settings = {
                    volume = 80,
                    fullscreen = true
                }
            }
        )";

        exec_lua(code);

        Dictionary dict = state->to_dictionary(-1);

        CHECK(dict.has("user"));
        CHECK(dict.has("settings"));

        Dictionary user = dict["user"];
        CHECK((String)user["name"] == "Bob");
        CHECK((int)user["age"] == 25);

        Dictionary settings = dict["settings"];
        CHECK((int)settings["volume"] == 80);
        CHECK((bool)settings["fullscreen"] == true);
    }

}

TEST_CASE_FIXTURE(LuaStateFixture, "Dictionary: Mixed with arrays")
{

    SUBCASE("Dictionary containing arrays")
    {
        Array items;
        items.push_back(1);
        items.push_back(2);
        items.push_back(3);

        Dictionary dict;
        dict["items"] = items;
        dict["count"] = 3;

        state->push_dictionary(dict);
        state->set_global("data");

        const char *code = R"(
            items = data.items
            first = items[1]
            second = items[2]
            count = data.count
        )";

        exec_lua(code);

        state->get_global("first");
        CHECK(state->to_integer(-1) == 1);
        state->pop(1);

        state->get_global("second");
        CHECK(state->to_integer(-1) == 2);
        state->pop(1);

        state->get_global("count");
        CHECK(state->to_integer(-1) == 3);
    }

    SUBCASE("Array containing dictionaries")
    {
        Dictionary item1;
        item1["id"] = 1;
        item1["name"] = "Item A";

        Dictionary item2;
        item2["id"] = 2;
        item2["name"] = "Item B";

        Array items;
        items.push_back(item1);
        items.push_back(item2);

        state->push_array(items);
        state->set_global("items");

        const char *code = R"(
            first_item = items[1]
            first_name = first_item.name
            second_id = items[2].id
        )";

        exec_lua(code);

        state->get_global("first_name");
        CHECK(state->to_string(-1) == "Item A");
        state->pop(1);

        state->get_global("second_id");
        CHECK(state->to_integer(-1) == 2);
    }

}

TEST_CASE_FIXTURE(LuaStateFixture, "Dictionary: Round-trip conversion")
{

    SUBCASE("Simple dictionary round-trip")
    {
        Dictionary original;
        original["name"] = "test";
        original["value"] = 42;
        original["flag"] = true;

        state->push_dictionary(original);
        state->set_global("dict");

        state->get_global("dict");
        Dictionary retrieved = state->to_dictionary(-1);

        CHECK(retrieved.has("name"));
        CHECK(retrieved.has("value"));
        CHECK(retrieved.has("flag"));

        CHECK((String)retrieved["name"] == "test");
        CHECK((int)retrieved["value"] == 42);
        CHECK((bool)retrieved["flag"] == true);
    }

    SUBCASE("Nested structure round-trip")
    {
        Dictionary inner;
        inner["x"] = 100;
        inner["y"] = 200;

        Array items;
        items.push_back(10);
        items.push_back(20);

        Dictionary original;
        original["position"] = inner;
        original["items"] = items;
        original["name"] = "complex";

        state->push_dictionary(original);
        state->set_global("complex");

        state->get_global("complex");
        Dictionary retrieved = state->to_dictionary(-1);

        CHECK((String)retrieved["name"] == "complex");

        Dictionary retrieved_pos = retrieved["position"];
        CHECK((int)retrieved_pos["x"] == 100);
        CHECK((int)retrieved_pos["y"] == 200);

        Array retrieved_items = retrieved["items"];
        CHECK(retrieved_items.size() == 2);
        CHECK((int)retrieved_items[0] == 10);
        CHECK((int)retrieved_items[1] == 20);
    }

}

TEST_CASE_FIXTURE(LuaStateFixture, "Dictionary: Edge cases")
{

    SUBCASE("Nil values")
    {
        const char *code = R"(
            return {
                a = 1,
                b = nil,
                c = 3
            }
        )";

        exec_lua(code);

        Dictionary dict = state->to_dictionary(-1);

        // Nil values may or may not be included depending on implementation
        CHECK(dict.has("a"));
        CHECK(dict.has("c"));
        // "b" might be missing or have a nil Variant
    }

    SUBCASE("Large dictionary")
    {
        Dictionary large;
        for (int i = 0; i < 100; i++)
        {
            String key = "key" + String::num_int64(i);
            large[key] = i;
        }

        state->push_dictionary(large);
        state->set_global("large");

        state->get_global("large");
        Dictionary retrieved = state->to_dictionary(-1);
        CHECK(retrieved.size() == 100);
        CHECK((int)retrieved["key0"] == 0);
        CHECK((int)retrieved["key50"] == 50);
        CHECK((int)retrieved["key99"] == 99);
    }

    SUBCASE("Special string keys")
    {
        Dictionary dict;
        dict["with space"] = 1;
        dict["with.dot"] = 2;
        dict["with-dash"] = 3;
        dict[""] = 4; // Empty string key

        state->push_dictionary(dict);
        state->set_global("special");

        const char *code = R"(
            v1 = special["with space"]
            v2 = special["with.dot"]
            v3 = special["with-dash"]
            v4 = special[""]
        )";

        exec_lua(code);

        state->get_global("v1");
        CHECK(state->to_integer(-1) == 1);
        state->pop(1);

        state->get_global("v2");
        CHECK(state->to_integer(-1) == 2);
        state->pop(1);

        state->get_global("v4");
        CHECK(state->to_integer(-1) == 4);
    }

}
