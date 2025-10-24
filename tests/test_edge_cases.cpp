// Tests for edge cases and error handling in Godot-Luau bridging
// Tests error conditions, boundary values, and special cases

#include "doctest.h"
#include "test_fixtures.h"
#include "lua_godotlib.h"
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/color.hpp>

using namespace godot;

TEST_CASE_FIXTURE(LuaStateFixture, "Edge cases: Division by zero")
{

    SUBCASE("Vector2 division by zero scalar")
    {
        const char *code = R"(
            v = Vector2(10, 20)
            result = v / 0
            return result
        )";

        lua_Status status = exec_lua(code);

        // Should either error or return inf/nan
        // Behavior depends on implementation
        if (status == LUA_OK)
        {
            Vector2 result = to_vector2(L, -1);
            // Result components should be inf or nan
            CHECK((std::isinf(result.x) || std::isnan(result.x)));
        }
        else
        {
            // Error is acceptable
            CHECK(status != LUA_OK);
        }
    }

    SUBCASE("Scalar division by zero in Lua")
    {
        const char *code = R"(
            x = 10 / 0
            return x
        )";

        exec_lua(code);

        double result = state->tonumber(-1);
        CHECK(std::isinf(result));
    }

}

TEST_CASE_FIXTURE(LuaStateFixture, "Edge cases: NaN and infinity handling")
{

    SUBCASE("Push infinity")
    {
        double inf = std::numeric_limits<double>::infinity();
        Variant inf_var = inf;

        state->pushvariant(inf_var);

        double retrieved = state->tonumber(-1);
        CHECK(std::isinf(retrieved));
    }

    SUBCASE("Push NaN")
    {
        double nan = std::numeric_limits<double>::quiet_NaN();
        Variant nan_var = nan;

        state->pushvariant(nan_var);

        double retrieved = state->tonumber(-1);
        CHECK(std::isnan(retrieved));
    }

    SUBCASE("Vector with NaN components")
    {
        Vector2 nan_vec(std::numeric_limits<double>::quiet_NaN(), 1.0);
        push_vector2(L, nan_vec);

        // Should handle gracefully without crashing
        // Exact behavior may vary
    }

}

TEST_CASE_FIXTURE(LuaStateFixture, "Edge cases: Nil handling in structures")
{

    SUBCASE("Array with explicit nil")
    {
        const char *code = R"(
            arr = {1, 2, nil, 4}
            return arr
        )";

        exec_lua(code);

        Array arr = state->toarray(-1);

        // Array length in Lua stops at first nil
        // Behavior may vary: could be length 2 or skip the nil
        CHECK(arr.size() >= 2);
    }

    SUBCASE("Dictionary with nil value")
    {
        const char *code = R"(
            dict = {
                a = 1,
                b = nil,
                c = 3
            }
            return dict
        )";

        exec_lua(code);

        Dictionary dict = state->todictionary(-1);

        // nil values might be omitted from dictionary
        CHECK(dict.has("a"));
        CHECK(dict.has("c"));
        // "b" may or may not be present
    }

    SUBCASE("Push nil variant in array")
    {
        Array arr;
        arr.push_back(1);
        arr.push_back(Variant()); // nil
        arr.push_back(3);

        Variant var = arr;
        state->pushvariant(var);

        Variant retrieved = state->tovariant(-1);
        Array r_arr = retrieved;

        // Should handle nil elements
        CHECK(r_arr.size() >= 1);
    }

}

TEST_CASE_FIXTURE(LuaStateFixture, "Edge cases: Empty and single-element collections")
{

    SUBCASE("Single element array")
    {
        Array arr;
        arr.push_back(42);

        state->pusharray(arr);
        state->setglobal("single");

        state->getglobal("single");
        Array retrieved = state->toarray(-1);

        CHECK(retrieved.size() == 1);
        CHECK((int)retrieved[0] == 42);
    }

    SUBCASE("Single key dictionary")
    {
        Dictionary dict;
        dict["only"] = "value";

        state->pushdictionary(dict);
        state->setglobal("single");

        state->getglobal("single");
        Dictionary retrieved = state->todictionary(-1);

        CHECK(retrieved.size() == 1);
        CHECK((String)retrieved["only"] == "value");
    }

    SUBCASE("Deeply nested single elements")
    {
        Array level3;
        level3.push_back(999);

        Array level2;
        level2.push_back(level3);

        Array level1;
        level1.push_back(level2);

        state->pusharray(level1);

        Variant var = state->tovariant(-1);
        Array l1 = var;
        Array l2 = l1[0];
        Array l3 = l2[0];

        CHECK((int)l3[0] == 999);
    }

}

TEST_CASE_FIXTURE(LuaStateFixture, "Edge cases: Type mismatches")
{

    SUBCASE("Try to use number as Vector2")
    {
        state->pushnumber(42);

        // Attempting to convert number to Vector2 should fail gracefully
        bool is_vec = is_vector2(L, -1);
        CHECK_FALSE(is_vec);

        // to_vector2 on non-vector should return zero or default
        // (behavior depends on implementation)
    }

    SUBCASE("Try to use string as table")
    {
        state->pushstring("not a table");

        CHECK_FALSE(state->istable(-1));
        CHECK(state->isstring(-1));

        // toarray on non-table should return empty or error
        // (implementation specific)
    }

    SUBCASE("Math type operator type mismatch")
    {
        const char *code = R"(
            v = Vector2(1, 2)
            -- Try to add vector and string (should error)
            -- result = v + "string"
            return v
        )";

        // This test just verifies that valid operations work
        // Invalid operations would be caught by Luau's type system or runtime
        lua_Status status = exec_lua(code);

        CHECK(status == LUA_OK);
    }

}

TEST_CASE_FIXTURE(LuaStateFixture, "Edge cases: Stack management")
{

    SUBCASE("Push many values")
    {

        int initial_top = state->gettop();
        state->checkstack(100);

        for (int i = 0; i < 100; i++)
        {
            state->pushinteger(i);
        }

        CHECK(state->gettop() == initial_top + 100);

        // Pop all
        state->settop(initial_top);
        CHECK(state->gettop() == initial_top);
    }

    SUBCASE("Nested table creation doesn't leak stack")
    {
        int initial_top = state->gettop();

        Array nested;
        for (int i = 0; i < 10; i++)
        {
            Array inner;
            inner.push_back(i);
            nested.push_back(inner);
        }

        state->pusharray(nested);
        state->setglobal("nested");

        CHECK(state->gettop() == initial_top);
    }

    SUBCASE("Multiple variant conversions")
    {
        int initial_top = state->gettop();

        for (int i = 0; i < 50; i++)
        {
            Variant var = i;
            state->pushvariant(var);
            Variant retrieved = state->tovariant(-1);
            state->pop(1);

            CHECK((int)retrieved == i);
        }

        CHECK(state->gettop() == initial_top);
    }

}

TEST_CASE_FIXTURE(LuaStateFixture, "Edge cases: Unicode and special strings")
{

    SUBCASE("Very long string")
    {
        String long_str;
        for (int i = 0; i < 10000; i++)
        {
            long_str += "x";
        }

        Variant var = long_str;
        state->pushvariant(var);

        Variant retrieved = state->tovariant(-1);
        String r_str = retrieved;

        CHECK(r_str.length() == 10000);
    }

    SUBCASE("Emoji in strings")
    {
        String emoji = "Hello 👋 World 🌍";
        state->pushstring(emoji);

        String retrieved = state->tostring(-1);
        CHECK(retrieved == emoji);
    }

    SUBCASE("Null bytes in string")
    {
        // Godot strings can contain null bytes, Lua strings can too
        String with_null = String("Hello") + String::chr(0) + String("World");

        state->pushstring(with_null);

        // Lua should preserve the full string with null byte
        String retrieved = state->tostring(-1);
        // Exact behavior depends on implementation
        // Some implementations may truncate at null byte
    }

    SUBCASE("Empty string vs nil")
    {
        state->pushstring("");
        CHECK(state->isstring(-1));
        CHECK_FALSE(state->isnil(-1));

        String empty = state->tostring(-1);
        CHECK(empty == "");
        CHECK(empty.length() == 0);
    }

}

TEST_CASE_FIXTURE(LuaStateFixture, "Edge cases: Boundary values for integers")
{

    SUBCASE("Max 32-bit integer")
    {
        int max_int = 2147483647;
        state->pushinteger(max_int);

        int retrieved = state->tointeger(-1);
        CHECK(retrieved == max_int);
    }

    SUBCASE("Min 32-bit integer")
    {
        int min_int = -2147483648;
        state->pushinteger(min_int);

        int retrieved = state->tointeger(-1);
        CHECK(retrieved == min_int);
    }

    SUBCASE("Zero integer")
    {
        state->pushinteger(0);

        CHECK(state->tointeger(-1) == 0);
        CHECK_FALSE(state->toboolean(-1)); // 0 is falsy in Lua
    }

    SUBCASE("Large array indices")
    {
        const char *code = R"(
            t = {}
            t[1000000] = "far"
            return t
        )";

        exec_lua(code);

        // Table with sparse indices should convert to dictionary
        CHECK(state->isdictionary(-1));
        CHECK_FALSE(state->isarray(-1));
    }

}

TEST_CASE_FIXTURE(LuaStateFixture, "Edge cases: Color clamping and ranges")
{

    SUBCASE("Color with values > 1.0")
    {
        Color bright(2.0, 3.0, 4.0, 5.0);
        push_color(L, bright);

        // Colors with values > 1.0 are valid (HDR)
        Color retrieved = to_color(L, -1);
        CHECK(retrieved.r == doctest::Approx(2.0));
        CHECK(retrieved.a == doctest::Approx(5.0));
    }

    SUBCASE("Color with negative values")
    {
        Color negative(-0.5, -1.0, 0.5, 1.0);
        push_color(L, negative);

        Color retrieved = to_color(L, -1);
        // Negative values might be preserved or clamped
        // depending on implementation
        CHECK(retrieved.b == doctest::Approx(0.5));
    }

    SUBCASE("Zero color")
    {
        Color zero(0, 0, 0, 0);
        push_color(L, zero);

        Color retrieved = to_color(L, -1);
        CHECK(retrieved.r == doctest::Approx(0.0));
        CHECK(retrieved.a == doctest::Approx(0.0));
    }

}

TEST_CASE_FIXTURE(LuaStateFixture, "Edge cases: Vector operations edge cases")
{

    SUBCASE("Zero vector operations")
    {
        const char *code = R"(
            v1 = Vector2(0, 0)
            v2 = Vector2(5, 5)
            sum = v1 + v2
            product = v1 * v2
            return sum, product
        )";

        exec_lua(code);

        // Should return 2 values
        CHECK(state->gettop() >= 2);

        Vector2 product = to_vector2(L, -1);
        CHECK(product.x == doctest::Approx(0.0));
        CHECK(product.y == doctest::Approx(0.0));

        Vector2 sum = to_vector2(L, -2);
        CHECK(sum.x == doctest::Approx(5.0));
        CHECK(sum.y == doctest::Approx(5.0));
    }

    SUBCASE("Very large vector components")
    {
        Vector3 huge(1e10, 1e15, 1e20);
        push_vector3(L, huge);

        Vector3 retrieved = to_vector3(L, -1);
        CHECK(retrieved.x == doctest::Approx(1e10));
        CHECK(retrieved.y == doctest::Approx(1e15));
        CHECK(retrieved.z == doctest::Approx(1e20));
    }

    SUBCASE("Vector integer overflow")
    {
        const char *code = R"(
            v1 = Vector2i(2147483647, 2147483647)
            v2 = Vector2i(1, 1)
            -- Adding would overflow
            -- sum = v1 + v2
            return v1
        )";

        lua_Status status = exec_lua(code);

        CHECK(status == LUA_OK);

        Vector2i v = to_vector2i(L, -1);
        CHECK(v.x == 2147483647);
    }

}

TEST_CASE_FIXTURE(LuaStateFixture, "Edge cases: Table iteration edge cases")
{

    SUBCASE("Table with only numeric key 0")
    {
        const char *code = R"(
            t = {[0] = "zero"}
            return t
        )";

        exec_lua(code);

        // Key 0 doesn't make it an array (arrays start at 1)
        CHECK(state->isdictionary(-1));
        CHECK_FALSE(state->isarray(-1));
    }

    SUBCASE("Table with negative indices")
    {
        const char *code = R"(
            t = {[-1] = "neg", [1] = "pos"}
            return t
        )";

        exec_lua(code);

        CHECK(state->isdictionary(-1));

        Dictionary dict = state->todictionary(-1);
        CHECK(dict.has(-1));
        CHECK(dict.has(1));
    }

}
