// Tests for Godot Dictionary <-> Lua table bridging
// Tests dictionary conversions, nested dictionaries, and key handling

#include "doctest.h"
#include "../src/lua_state.h"
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <lua.h>
#include <lualib.h>

using namespace godot;

TEST_CASE("Dictionary: Simple dictionary Godot -> Lua") {
    LuaState L;
    L.openlibs(LuaState::LIB_ALL);

    SUBCASE("String keys") {
        Dictionary dict;
        dict["name"] = "test";
        dict["value"] = 42;
        dict["active"] = true;

        L.pushdictionary(dict);
        L.setglobal("d");

        // Access values from Lua
        L.getglobal("d");
        L.pushstring("name");
        L.gettable(-2);
        CHECK(L.tostring(-1) == "test");
        L.pop(1);

        L.pushstring("value");
        L.gettable(-2);
        CHECK(L.tointeger(-1) == 42);
        L.pop(1);

        L.pushstring("active");
        L.gettable(-2);
        CHECK(L.toboolean(-1) == true);
    }

    SUBCASE("Integer keys") {
        Dictionary dict;
        dict[1] = "one";
        dict[2] = "two";
        dict[100] = "hundred";

        L.pushdictionary(dict);
        L.setglobal("d");

        const char* code = R"(
            v1 = d[1]
            v2 = d[2]
            v100 = d[100]
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        L.getglobal("v1");
        CHECK(L.tostring(-1) == "one");
        L.pop(1);

        L.getglobal("v100");
        CHECK(L.tostring(-1) == "hundred");
    }

    SUBCASE("Empty dictionary") {
        Dictionary dict;

        L.pushdictionary(dict);
        CHECK(L.istable(-1));

        // Empty table should have no keys
        const char* code = R"(
            count = 0
            for k, v in pairs(d) do
                count = count + 1
            end
            return count
        )";

        L.setglobal("d");

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        CHECK(L.tointeger(-1) == 0);
    }
}

TEST_CASE("Dictionary: Lua table -> Godot Dictionary") {
    LuaState L;
    L.openlibs(LuaState::LIB_ALL);

    SUBCASE("Simple string-keyed table") {
        const char* code = R"(
            return {
                name = "Alice",
                age = 30,
                active = true
            }
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        Dictionary dict = L.todictionary(-1);

        CHECK(dict.has("name"));
        CHECK((String)dict["name"] == "Alice");
        CHECK((int)dict["age"] == 30);
        CHECK((bool)dict["active"] == true);
    }

    SUBCASE("Integer-keyed table") {
        const char* code = R"(
            return {
                [10] = "ten",
                [20] = "twenty",
                [30] = "thirty"
            }
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        Dictionary dict = L.todictionary(-1);

        CHECK(dict.has(10));
        CHECK((String)dict[10] == "ten");
        CHECK((String)dict[20] == "twenty");
        CHECK((String)dict[30] == "thirty");
    }

    SUBCASE("Mixed key types") {
        const char* code = R"(
            return {
                name = "test",
                [1] = "first",
                [2] = "second",
                count = 99
            }
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        Dictionary dict = L.todictionary(-1);

        CHECK(dict.has("name"));
        CHECK(dict.has(1));
        CHECK(dict.has(2));
        CHECK(dict.has("count"));

        CHECK((String)dict["name"] == "test");
        CHECK((String)dict[1] == "first");
        CHECK((int)dict["count"] == 99);
    }
}

TEST_CASE("Dictionary: Nested dictionaries") {
    LuaState L;
    L.openlibs(LuaState::LIB_ALL);

    SUBCASE("Dictionary containing dictionary") {
        Dictionary inner;
        inner["x"] = 10;
        inner["y"] = 20;

        Dictionary outer;
        outer["position"] = inner;
        outer["name"] = "entity";

        L.pushdictionary(outer);
        L.setglobal("entity");

        const char* code = R"(
            pos = entity.position
            x = pos.x
            y = pos.y
            name = entity.name
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        L.getglobal("x");
        CHECK(L.tointeger(-1) == 10);
        L.pop(1);

        L.getglobal("y");
        CHECK(L.tointeger(-1) == 20);
        L.pop(1);

        L.getglobal("name");
        CHECK(L.tostring(-1) == "entity");
    }

    SUBCASE("Lua nested table to Dictionary") {
        const char* code = R"(
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

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        Dictionary dict = L.todictionary(-1);

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

TEST_CASE("Dictionary: Mixed with arrays") {
    LuaState L;
    L.openlibs(LuaState::LIB_ALL);

    SUBCASE("Dictionary containing arrays") {
        Array items;
        items.push_back(1);
        items.push_back(2);
        items.push_back(3);

        Dictionary dict;
        dict["items"] = items;
        dict["count"] = 3;

        L.pushdictionary(dict);
        L.setglobal("data");

        const char* code = R"(
            items = data.items
            first = items[1]
            second = items[2]
            count = data.count
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        L.getglobal("first");
        CHECK(L.tointeger(-1) == 1);
        L.pop(1);

        L.getglobal("second");
        CHECK(L.tointeger(-1) == 2);
        L.pop(1);

        L.getglobal("count");
        CHECK(L.tointeger(-1) == 3);
    }

    SUBCASE("Array containing dictionaries") {
        Dictionary item1;
        item1["id"] = 1;
        item1["name"] = "Item A";

        Dictionary item2;
        item2["id"] = 2;
        item2["name"] = "Item B";

        Array items;
        items.push_back(item1);
        items.push_back(item2);

        L.pusharray(items);
        L.setglobal("items");

        const char* code = R"(
            first_item = items[1]
            first_name = first_item.name
            second_id = items[2].id
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        L.getglobal("first_name");
        CHECK(L.tostring(-1) == "Item A");
        L.pop(1);

        L.getglobal("second_id");
        CHECK(L.tointeger(-1) == 2);
    }
}

TEST_CASE("Dictionary: Round-trip conversion") {
    LuaState L;
    L.openlibs(LuaState::LIB_ALL);

    SUBCASE("Simple dictionary round-trip") {
        Dictionary original;
        original["name"] = "test";
        original["value"] = 42;
        original["flag"] = true;

        L.pushdictionary(original);
        L.setglobal("dict");

        L.getglobal("dict");
        Dictionary retrieved = L.todictionary(-1);

        CHECK(retrieved.has("name"));
        CHECK(retrieved.has("value"));
        CHECK(retrieved.has("flag"));

        CHECK((String)retrieved["name"] == "test");
        CHECK((int)retrieved["value"] == 42);
        CHECK((bool)retrieved["flag"] == true);
    }

    SUBCASE("Nested structure round-trip") {
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

        L.pushdictionary(original);
        L.setglobal("complex");

        L.getglobal("complex");
        Dictionary retrieved = L.todictionary(-1);

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

TEST_CASE("Dictionary: Edge cases") {
    LuaState L;
    L.openlibs(LuaState::LIB_ALL);

    SUBCASE("Nil values") {
        const char* code = R"(
            return {
                a = 1,
                b = nil,
                c = 3
            }
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        Dictionary dict = L.todictionary(-1);

        // Nil values may or may not be included depending on implementation
        CHECK(dict.has("a"));
        CHECK(dict.has("c"));
        // "b" might be missing or have a nil Variant
    }

    SUBCASE("Large dictionary") {
        Dictionary large;
        for (int i = 0; i < 100; i++) {
            String key = "key" + String::num(i);
            large[key] = i;
        }

        L.pushdictionary(large);
        L.setglobal("large");

        L.getglobal("large");
        Dictionary retrieved = L.todictionary(-1);

        CHECK(retrieved.size() == 100);
        CHECK((int)retrieved["key0"] == 0);
        CHECK((int)retrieved["key50"] == 50);
        CHECK((int)retrieved["key99"] == 99);
    }

    SUBCASE("Special string keys") {
        Dictionary dict;
        dict["with space"] = 1;
        dict["with.dot"] = 2;
        dict["with-dash"] = 3;
        dict[""] = 4; // Empty string key

        L.pushdictionary(dict);
        L.setglobal("special");

        const char* code = R"(
            v1 = special["with space"]
            v2 = special["with.dot"]
            v3 = special["with-dash"]
            v4 = special[""]
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        L.getglobal("v1");
        CHECK(L.tointeger(-1) == 1);
        L.pop(1);

        L.getglobal("v2");
        CHECK(L.tointeger(-1) == 2);
        L.pop(1);

        L.getglobal("v4");
        CHECK(L.tointeger(-1) == 4);
    }
}
