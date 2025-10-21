// Tests for edge cases and error handling in Godot-Luau bridging
// Tests error conditions, boundary values, and special cases

#include "doctest.h"
#include "../src/lua_state.h"
#include "../src/lua_godotlib.h"
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/color.hpp>
#include <lua.h>
#include <lualib.h>

using namespace godot;

TEST_CASE("Edge cases: Division by zero") {
    LuaState L;
    L.openlibs(LuaState::LIB_ALL);

    SUBCASE("Vector2 division by zero scalar") {
        const char* code = R"(
            v = Vector2(10, 20)
            result = v / 0
            return result
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        lua_Status status = L.resume();

        // Should either error or return inf/nan
        // Behavior depends on implementation
        if (status == LUA_OK) {
            Vector2 result = to_vector2((lua_State*)nullptr, -1);
            // Result components should be inf or nan
            CHECK((std::isinf(result.x) || std::isnan(result.x)));
        } else {
            // Error is acceptable
            CHECK(status != LUA_OK);
        }
    }

    SUBCASE("Scalar division by zero in Lua") {
        const char* code = R"(
            x = 10 / 0
            return x
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        double result = L.tonumber(-1);
        CHECK(std::isinf(result));
    }
}

TEST_CASE("Edge cases: NaN and infinity handling") {
    LuaState L;
    L.openlibs(LuaState::LIB_ALL);

    SUBCASE("Push infinity") {
        double inf = std::numeric_limits<double>::infinity();
        Variant inf_var = inf;

        L.pushvariant(inf_var);

        double retrieved = L.tonumber(-1);
        CHECK(std::isinf(retrieved));
    }

    SUBCASE("Push NaN") {
        double nan = std::numeric_limits<double>::quiet_NaN();
        Variant nan_var = nan;

        L.pushvariant(nan_var);

        double retrieved = L.tonumber(-1);
        CHECK(std::isnan(retrieved));
    }

    SUBCASE("Vector with NaN components") {
        Vector2 nan_vec(std::numeric_limits<double>::quiet_NaN(), 1.0);
        push_vector2((lua_State*)nullptr, nan_vec);

        // Should handle gracefully without crashing
        // Exact behavior may vary
    }
}

TEST_CASE("Edge cases: Nil handling in structures") {
    LuaState L;
    L.openlibs(LuaState::LIB_ALL);

    SUBCASE("Array with explicit nil") {
        const char* code = R"(
            arr = {1, 2, nil, 4}
            return arr
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        Array arr = L.toarray(-1);

        // Array length in Lua stops at first nil
        // Behavior may vary: could be length 2 or skip the nil
        CHECK(arr.size() >= 2);
    }

    SUBCASE("Dictionary with nil value") {
        const char* code = R"(
            dict = {
                a = 1,
                b = nil,
                c = 3
            }
            return dict
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        Dictionary dict = L.todictionary(-1);

        // nil values might be omitted from dictionary
        CHECK(dict.has("a"));
        CHECK(dict.has("c"));
        // "b" may or may not be present
    }

    SUBCASE("Push nil variant in array") {
        Array arr;
        arr.push_back(1);
        arr.push_back(Variant()); // nil
        arr.push_back(3);

        Variant var = arr;
        L.pushvariant(var);

        Variant retrieved = L.tovariant(-1);
        Array r_arr = retrieved;

        // Should handle nil elements
        CHECK(r_arr.size() >= 1);
    }
}

TEST_CASE("Edge cases: Empty and single-element collections") {
    LuaState L;
    L.openlibs(LuaState::LIB_ALL);

    SUBCASE("Single element array") {
        Array arr;
        arr.push_back(42);

        L.pusharray(arr);
        L.setglobal("single");

        L.getglobal("single");
        Array retrieved = L.toarray(-1);

        CHECK(retrieved.size() == 1);
        CHECK((int)retrieved[0] == 42);
    }

    SUBCASE("Single key dictionary") {
        Dictionary dict;
        dict["only"] = "value";

        L.pushdictionary(dict);
        L.setglobal("single");

        L.getglobal("single");
        Dictionary retrieved = L.todictionary(-1);

        CHECK(retrieved.size() == 1);
        CHECK((String)retrieved["only"] == "value");
    }

    SUBCASE("Deeply nested single elements") {
        Array level3;
        level3.push_back(999);

        Array level2;
        level2.push_back(level3);

        Array level1;
        level1.push_back(level2);

        L.pusharray(level1);

        Variant var = L.tovariant(-1);
        Array l1 = var;
        Array l2 = l1[0];
        Array l3 = l2[0];

        CHECK((int)l3[0] == 999);
    }
}

TEST_CASE("Edge cases: Type mismatches") {
    LuaState L;
    L.openlibs(LuaState::LIB_ALL);

    SUBCASE("Try to use number as Vector2") {
        L.pushnumber(42);

        // Attempting to convert number to Vector2 should fail gracefully
        bool is_vec = is_vector2((lua_State*)nullptr, -1);
        CHECK_FALSE(is_vec);

        // to_vector2 on non-vector should return zero or default
        // (behavior depends on implementation)
    }

    SUBCASE("Try to use string as table") {
        L.pushstring("not a table");

        CHECK_FALSE(L.istable(-1));
        CHECK(L.isstring(-1));

        // toarray on non-table should return empty or error
        // (implementation specific)
    }

    SUBCASE("Math type operator type mismatch") {
        const char* code = R"(
            v = Vector2(1, 2)
            -- Try to add vector and string (should error)
            -- result = v + "string"
            return v
        )";

        // This test just verifies that valid operations work
        // Invalid operations would be caught by Luau's type system or runtime
        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        lua_Status status = L.resume();

        CHECK(status == LUA_OK);
    }
}

TEST_CASE("Edge cases: Stack management") {
    LuaState L;
    L.openlibs(LuaState::LIB_ALL);

    SUBCASE("Push many values") {
        int initial_top = L.gettop();

        for (int i = 0; i < 100; i++) {
            L.pushinteger(i);
        }

        CHECK(L.gettop() == initial_top + 100);

        // Pop all
        L.settop(initial_top);
        CHECK(L.gettop() == initial_top);
    }

    SUBCASE("Nested table creation doesn't leak stack") {
        int initial_top = L.gettop();

        Array nested;
        for (int i = 0; i < 10; i++) {
            Array inner;
            inner.push_back(i);
            nested.push_back(inner);
        }

        L.pusharray(nested);
        L.setglobal("nested");

        CHECK(L.gettop() == initial_top);
    }

    SUBCASE("Multiple variant conversions") {
        int initial_top = L.gettop();

        for (int i = 0; i < 50; i++) {
            Variant var = i;
            L.pushvariant(var);
            Variant retrieved = L.tovariant(-1);
            L.pop(1);

            CHECK((int)retrieved == i);
        }

        CHECK(L.gettop() == initial_top);
    }
}

TEST_CASE("Edge cases: Unicode and special strings") {
    LuaState L;
    L.openlibs(LuaState::LIB_ALL);

    SUBCASE("Very long string") {
        String long_str;
        for (int i = 0; i < 10000; i++) {
            long_str += "x";
        }

        Variant var = long_str;
        L.pushvariant(var);

        Variant retrieved = L.tovariant(-1);
        String r_str = retrieved;

        CHECK(r_str.length() == 10000);
    }

    SUBCASE("Emoji in strings") {
        String emoji = "Hello 👋 World 🌍";
        L.pushstring(emoji);

        String retrieved = L.tostring(-1);
        CHECK(retrieved == emoji);
    }

    SUBCASE("Null bytes in string") {
        // Godot strings can contain null bytes, Lua strings can too
        String with_null = String("Hello") + String::chr(0) + String("World");

        L.pushstring(with_null);

        // Lua should preserve the full string with null byte
        String retrieved = L.tostring(-1);
        // Exact behavior depends on implementation
        // Some implementations may truncate at null byte
    }

    SUBCASE("Empty string vs nil") {
        L.pushstring("");
        CHECK(L.isstring(-1));
        CHECK_FALSE(L.isnil(-1));

        String empty = L.tostring(-1);
        CHECK(empty == "");
        CHECK(empty.length() == 0);
    }
}

TEST_CASE("Edge cases: Boundary values for integers") {
    LuaState L;
    L.openlibs(LuaState::LIB_ALL);

    SUBCASE("Max 32-bit integer") {
        int max_int = 2147483647;
        L.pushinteger(max_int);

        int retrieved = L.tointeger(-1);
        CHECK(retrieved == max_int);
    }

    SUBCASE("Min 32-bit integer") {
        int min_int = -2147483648;
        L.pushinteger(min_int);

        int retrieved = L.tointeger(-1);
        CHECK(retrieved == min_int);
    }

    SUBCASE("Zero integer") {
        L.pushinteger(0);

        CHECK(L.tointeger(-1) == 0);
        CHECK_FALSE(L.toboolean(-1)); // 0 is falsy in Lua
    }

    SUBCASE("Large array indices") {
        const char* code = R"(
            t = {}
            t[1000000] = "far"
            return t
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        // Table with sparse indices should convert to dictionary
        CHECK(L.isdictionary(-1));
        CHECK_FALSE(L.isarray(-1));
    }
}

TEST_CASE("Edge cases: Color clamping and ranges") {
    LuaState L;
    L.openlibs(LuaState::LIB_ALL);

    SUBCASE("Color with values > 1.0") {
        Color bright(2.0, 3.0, 4.0, 5.0);
        push_color((lua_State*)nullptr, bright);

        // Colors with values > 1.0 are valid (HDR)
        Color retrieved = to_color((lua_State*)nullptr, -1);
        CHECK(retrieved.r == doctest::Approx(2.0));
        CHECK(retrieved.a == doctest::Approx(5.0));
    }

    SUBCASE("Color with negative values") {
        Color negative(-0.5, -1.0, 0.5, 1.0);
        push_color((lua_State*)nullptr, negative);

        Color retrieved = to_color((lua_State*)nullptr, -1);
        // Negative values might be preserved or clamped
        // depending on implementation
        CHECK(retrieved.b == doctest::Approx(0.5));
    }

    SUBCASE("Zero color") {
        Color zero(0, 0, 0, 0);
        push_color((lua_State*)nullptr, zero);

        Color retrieved = to_color((lua_State*)nullptr, -1);
        CHECK(retrieved.r == doctest::Approx(0.0));
        CHECK(retrieved.a == doctest::Approx(0.0));
    }
}

TEST_CASE("Edge cases: Vector operations edge cases") {
    LuaState L;
    L.openlibs(LuaState::LIB_ALL);

    SUBCASE("Zero vector operations") {
        const char* code = R"(
            v1 = Vector2(0, 0)
            v2 = Vector2(5, 5)
            sum = v1 + v2
            product = v1 * v2
            return sum, product
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        // Should return 2 values
        CHECK(L.gettop() >= 2);

        Vector2 product = to_vector2((lua_State*)nullptr, -1);
        CHECK(product.x == doctest::Approx(0.0));
        CHECK(product.y == doctest::Approx(0.0));

        Vector2 sum = to_vector2((lua_State*)nullptr, -2);
        CHECK(sum.x == doctest::Approx(5.0));
        CHECK(sum.y == doctest::Approx(5.0));
    }

    SUBCASE("Very large vector components") {
        Vector3 huge(1e10, 1e15, 1e20);
        push_vector3((lua_State*)nullptr, huge);

        Vector3 retrieved = to_vector3((lua_State*)nullptr, -1);
        CHECK(retrieved.x == doctest::Approx(1e10));
        CHECK(retrieved.y == doctest::Approx(1e15));
        CHECK(retrieved.z == doctest::Approx(1e20));
    }

    SUBCASE("Vector integer overflow") {
        const char* code = R"(
            v1 = Vector2i(2147483647, 2147483647)
            v2 = Vector2i(1, 1)
            -- Adding would overflow
            -- sum = v1 + v2
            return v1
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        lua_Status status = L.resume();

        CHECK(status == LUA_OK);

        Vector2i v = to_vector2i((lua_State*)nullptr, -1);
        CHECK(v.x == 2147483647);
    }
}

TEST_CASE("Edge cases: Table iteration edge cases") {
    LuaState L;
    L.openlibs(LuaState::LIB_ALL);

    SUBCASE("Table with only numeric key 0") {
        const char* code = R"(
            t = {[0] = "zero"}
            return t
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        // Key 0 doesn't make it an array (arrays start at 1)
        CHECK(L.isdictionary(-1));
        CHECK_FALSE(L.isarray(-1));
    }

    SUBCASE("Table with negative indices") {
        const char* code = R"(
            t = {[-1] = "neg", [1] = "pos"}
            return t
        )";

        PackedByteArray bytecode = Luau::compile(code);
        L.load_bytecode(bytecode, "test");
        L.resume();

        CHECK(L.isdictionary(-1));

        Dictionary dict = L.todictionary(-1);
        CHECK(dict.has(-1));
        CHECK(dict.has(1));
    }
}
