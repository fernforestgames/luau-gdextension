// Tests for lua_godotlib - Godot type constructors and property accessors
// Validates that all Godot Variant constructors work from Lua
// and that property accessors (e.g., Vector2.x, Rect2.size) function correctly

#include "doctest.h"
#include "test_fixtures.h"

#include "bridging/variant.h"
#include "lua_state.h"

using namespace gdluau;
using namespace godot;

// ============================================================================
// Vector2 Constructor and Property Tests
// ============================================================================

TEST_SUITE("lua_godotlib - Vector2")
{
	TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector2 constructor")
	{
		exec_lua("return Vector2(3.5, 4.2)");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::VECTOR2);

		Vector2 v = result;
		CHECK(v.x == doctest::Approx(3.5));
		CHECK(v.y == doctest::Approx(4.2));

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector2 property access - read")
	{
		exec_lua(R"(
			v = Vector2(7.5, 8.5)
			return v.x, v.y
		)");

		double y = lua_tonumber(L, -1);
		double x = lua_tonumber(L, -2);

		CHECK(x == doctest::Approx(7.5));
		CHECK(y == doctest::Approx(8.5));

		lua_pop(L, 2);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector2 property access - write")
	{
		exec_lua(R"(
			v = Vector2(1.0, 2.0)
			v.x = 99.5
			v.y = 88.5
			return v
		)");

		Variant result = to_variant(L, -1);
		Vector2 v = result;

		CHECK(v.x == doctest::Approx(99.5));
		CHECK(v.y == doctest::Approx(88.5));

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector2 zero constructor")
	{
		exec_lua("return Vector2(0, 0)");

		Variant result = to_variant(L, -1);
		Vector2 v = result;

		CHECK(v.x == 0.0);
		CHECK(v.y == 0.0);

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector2 negative values")
	{
		exec_lua("return Vector2(-10.5, -20.3)");

		Variant result = to_variant(L, -1);
		Vector2 v = result;

		CHECK(v.x == doctest::Approx(-10.5));
		CHECK(v.y == doctest::Approx(-20.3));

		lua_pop(L, 1);
	}
}

// ============================================================================
// Vector2i Constructor and Property Tests
// ============================================================================

TEST_SUITE("lua_godotlib - Vector2i")
{
	TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector2i constructor")
	{
		exec_lua("return Vector2i(10, 20)");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::VECTOR2I);

		Vector2i v = result;
		CHECK(v.x == 10);
		CHECK(v.y == 20);

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector2i property access")
	{
		exec_lua(R"(
			v = Vector2i(100, 200)
			return v.x, v.y
		)");

		int y = lua_tointeger(L, -1);
		int x = lua_tointeger(L, -2);

		CHECK(x == 100);
		CHECK(y == 200);

		lua_pop(L, 2);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector2i negative values")
	{
		exec_lua("return Vector2i(-50, -100)");

		Variant result = to_variant(L, -1);
		Vector2i v = result;

		CHECK(v.x == -50);
		CHECK(v.y == -100);

		lua_pop(L, 1);
	}
}

// ============================================================================
// Vector3 Constructor Tests - Native Vector Type
// ============================================================================

TEST_SUITE("lua_godotlib - Vector3 Native")
{
	TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector3() returns native vector type")
	{
		exec_lua("return Vector3(1.5, 2.5, 3.5)");

		// Vector3 should be represented as Luau's native vector type
		CHECK(lua_type(L, -1) == LUA_TVECTOR);

		const float *vec = lua_tovector(L, -1);
		CHECK(vec[0] == doctest::Approx(1.5f));
		CHECK(vec[1] == doctest::Approx(2.5f));
		CHECK(vec[2] == doctest::Approx(3.5f));

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector3 native vector bridges to Variant correctly")
	{
		exec_lua("return Vector3(7.0, 8.0, 9.0)");

		// to_variant should convert native vector to Vector3 Variant
		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::VECTOR3);

		Vector3 v = result;
		CHECK(v.x == doctest::Approx(7.0));
		CHECK(v.y == doctest::Approx(8.0));
		CHECK(v.z == doctest::Approx(9.0));

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector3 property access on native vector")
	{
		// Note: Native vectors use .x, .y, .z for property access
		exec_lua(R"(
			v = Vector3(10, 20, 30)
			return v.x, v.y, v.z
		)");

		double z = lua_tonumber(L, -1);
		double y = lua_tonumber(L, -2);
		double x = lua_tonumber(L, -3);

		CHECK(x == doctest::Approx(10.0));
		CHECK(y == doctest::Approx(20.0));
		CHECK(z == doctest::Approx(30.0));

		lua_pop(L, 3);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector3 native arithmetic")
	{
		// Native vector type supports inline arithmetic
		exec_lua(R"(
			v1 = Vector3(1, 2, 3)
			v2 = Vector3(4, 5, 6)
			return v1 + v2
		)");

		CHECK(lua_type(L, -1) == LUA_TVECTOR);

		const float *vec = lua_tovector(L, -1);
		CHECK(vec[0] == doctest::Approx(5.0f));
		CHECK(vec[1] == doctest::Approx(7.0f));
		CHECK(vec[2] == doctest::Approx(9.0f));

		lua_pop(L, 1);
	}
}

// ============================================================================
// Vector3i Constructor and Property Tests
// ============================================================================

TEST_SUITE("lua_godotlib - Vector3i")
{
	TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector3i constructor from integers")
	{
		exec_lua("return Vector3i(100, 200, 300)");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::VECTOR3I);

		Vector3i v = result;
		CHECK(v.x == 100);
		CHECK(v.y == 200);
		CHECK(v.z == 300);

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector3i constructor from Vector3")
	{
		exec_lua("return Vector3i(Vector3(1.9, 2.1, 3.5))");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::VECTOR3I);

		Vector3i v = result;
		CHECK(v.x == 1);
		CHECK(v.y == 2);
		CHECK(v.z == 3);

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector3i property access")
	{
		exec_lua(R"(
			v = Vector3i(50, 60, 70)
			return v.x, v.y, v.z
		)");

		int z = lua_tointeger(L, -1);
		int y = lua_tointeger(L, -2);
		int x = lua_tointeger(L, -3);

		CHECK(x == 50);
		CHECK(y == 60);
		CHECK(z == 70);

		lua_pop(L, 3);
	}
}

// ============================================================================
// Rect2 Constructor and Property Tests
// ============================================================================

TEST_SUITE("lua_godotlib - Rect2")
{
	TEST_CASE_FIXTURE(RawLuaStateFixture, "Rect2 constructor from scalars")
	{
		exec_lua("return Rect2(10.0, 20.0, 100.0, 200.0)");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::RECT2);

		Rect2 r = result;
		CHECK(r.position.x == doctest::Approx(10.0));
		CHECK(r.position.y == doctest::Approx(20.0));
		CHECK(r.size.x == doctest::Approx(100.0));
		CHECK(r.size.y == doctest::Approx(200.0));

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Rect2 constructor from Vector2s")
	{
		exec_lua("return Rect2(Vector2(5, 10), Vector2(50, 100))");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::RECT2);

		Rect2 r = result;
		CHECK(r.position.x == doctest::Approx(5.0));
		CHECK(r.position.y == doctest::Approx(10.0));
		CHECK(r.size.x == doctest::Approx(50.0));
		CHECK(r.size.y == doctest::Approx(100.0));

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Rect2 property access - position and size")
	{
		exec_lua(R"(
			r = Rect2(1, 2, 3, 4)
			return r.position, r.size
		)");

		Variant size_v = to_variant(L, -1);
		Variant pos_v = to_variant(L, -2);

		Vector2 pos = pos_v;
		Vector2 size = size_v;

		CHECK(pos.x == doctest::Approx(1.0));
		CHECK(pos.y == doctest::Approx(2.0));
		CHECK(size.x == doctest::Approx(3.0));
		CHECK(size.y == doctest::Approx(4.0));

		lua_pop(L, 2);
	}
}

// ============================================================================
// Rect2i Constructor and Property Tests
// ============================================================================

TEST_SUITE("lua_godotlib - Rect2i")
{
	TEST_CASE_FIXTURE(RawLuaStateFixture, "Rect2i constructor from scalars")
	{
		exec_lua("return Rect2i(5, 10, 50, 100)");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::RECT2I);

		Rect2i r = result;
		CHECK(r.position.x == 5);
		CHECK(r.position.y == 10);
		CHECK(r.size.x == 50);
		CHECK(r.size.y == 100);

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Rect2i constructor from Vector2is")
	{
		exec_lua("return Rect2i(Vector2i(1, 2), Vector2i(10, 20))");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::RECT2I);

		Rect2i r = result;
		CHECK(r.position.x == 1);
		CHECK(r.position.y == 2);
		CHECK(r.size.x == 10);
		CHECK(r.size.y == 20);

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Rect2i property access")
	{
		exec_lua(R"(
			r = Rect2i(3, 4, 30, 40)
			return r.position, r.size
		)");

		Variant size_v = to_variant(L, -1);
		Variant pos_v = to_variant(L, -2);

		Vector2i pos = pos_v;
		Vector2i size = size_v;

		CHECK(pos.x == 3);
		CHECK(pos.y == 4);
		CHECK(size.x == 30);
		CHECK(size.y == 40);

		lua_pop(L, 2);
	}
}

// ============================================================================
// Vector4 Constructor and Property Tests
// ============================================================================

TEST_SUITE("lua_godotlib - Vector4")
{
	TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector4 constructor")
	{
		exec_lua("return Vector4(1.5, 2.5, 3.5, 4.5)");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::VECTOR4);

		Vector4 v = result;
		CHECK(v.x == doctest::Approx(1.5));
		CHECK(v.y == doctest::Approx(2.5));
		CHECK(v.z == doctest::Approx(3.5));
		CHECK(v.w == doctest::Approx(4.5));

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector4 property access")
	{
		exec_lua(R"(
			v = Vector4(10, 20, 30, 40)
			return v.x, v.y, v.z, v.w
		)");

		double w = lua_tonumber(L, -1);
		double z = lua_tonumber(L, -2);
		double y = lua_tonumber(L, -3);
		double x = lua_tonumber(L, -4);

		CHECK(x == doctest::Approx(10.0));
		CHECK(y == doctest::Approx(20.0));
		CHECK(z == doctest::Approx(30.0));
		CHECK(w == doctest::Approx(40.0));

		lua_pop(L, 4);
	}
}

// ============================================================================
// Vector4i Constructor and Property Tests
// ============================================================================

TEST_SUITE("lua_godotlib - Vector4i")
{
	TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector4i constructor from integers")
	{
		exec_lua("return Vector4i(10, 20, 30, 40)");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::VECTOR4I);

		Vector4i v = result;
		CHECK(v.x == 10);
		CHECK(v.y == 20);
		CHECK(v.z == 30);
		CHECK(v.w == 40);

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector4i constructor from Vector4")
	{
		exec_lua("return Vector4i(Vector4(1.9, 2.1, 3.5, 4.8))");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::VECTOR4I);

		Vector4i v = result;
		CHECK(v.x == 1);
		CHECK(v.y == 2);
		CHECK(v.z == 3);
		CHECK(v.w == 4);

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Vector4i property access")
	{
		exec_lua(R"(
			v = Vector4i(100, 200, 300, 400)
			return v.x, v.y, v.z, v.w
		)");

		int w = lua_tointeger(L, -1);
		int z = lua_tointeger(L, -2);
		int y = lua_tointeger(L, -3);
		int x = lua_tointeger(L, -4);

		CHECK(x == 100);
		CHECK(y == 200);
		CHECK(z == 300);
		CHECK(w == 400);

		lua_pop(L, 4);
	}
}

// ============================================================================
// Color Constructor and Property Tests
// ============================================================================

TEST_SUITE("lua_godotlib - Color")
{
	TEST_CASE_FIXTURE(RawLuaStateFixture, "Color constructor RGB")
	{
		exec_lua("return Color(1.0, 0.5, 0.0)");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::COLOR);

		Color c = result;
		CHECK(c.r == doctest::Approx(1.0));
		CHECK(c.g == doctest::Approx(0.5));
		CHECK(c.b == doctest::Approx(0.0));
		CHECK(c.a == doctest::Approx(1.0)); // Default alpha

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Color constructor RGBA")
	{
		exec_lua("return Color(0.2, 0.4, 0.6, 0.8)");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::COLOR);

		Color c = result;
		CHECK(c.r == doctest::Approx(0.2));
		CHECK(c.g == doctest::Approx(0.4));
		CHECK(c.b == doctest::Approx(0.6));
		CHECK(c.a == doctest::Approx(0.8));

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Color constructor from hex string")
	{
		exec_lua("return Color('#ff8000')");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::COLOR);

		Color c = result;
		CHECK(c.r == doctest::Approx(1.0));
		CHECK(c.g == doctest::Approx(0.5).epsilon(0.01));
		CHECK(c.b == doctest::Approx(0.0));

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Color property access")
	{
		exec_lua(R"(
			c = Color(0.1, 0.2, 0.3, 0.4)
			return c.r, c.g, c.b, c.a
		)");

		double a = lua_tonumber(L, -1);
		double b = lua_tonumber(L, -2);
		double g = lua_tonumber(L, -3);
		double r = lua_tonumber(L, -4);

		CHECK(r == doctest::Approx(0.1));
		CHECK(g == doctest::Approx(0.2));
		CHECK(b == doctest::Approx(0.3));
		CHECK(a == doctest::Approx(0.4));

		lua_pop(L, 4);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Color property modification")
	{
		exec_lua(R"(
			c = Color(1, 0, 0, 1)
			c.g = 0.5
			c.a = 0.7
			return c
		)");

		Variant result = to_variant(L, -1);
		Color c = result;

		CHECK(c.r == doctest::Approx(1.0));
		CHECK(c.g == doctest::Approx(0.5));
		CHECK(c.b == doctest::Approx(0.0));
		CHECK(c.a == doctest::Approx(0.7));

		lua_pop(L, 1);
	}
}

// ============================================================================
// Plane Constructor and Property Tests
// ============================================================================

TEST_SUITE("lua_godotlib - Plane")
{
	TEST_CASE_FIXTURE(RawLuaStateFixture, "Plane constructor from components")
	{
		exec_lua("return Plane(1.0, 0.0, 0.0, 5.0)");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::PLANE);

		Plane p = result;
		CHECK(p.normal.x == doctest::Approx(1.0));
		CHECK(p.normal.y == doctest::Approx(0.0));
		CHECK(p.normal.z == doctest::Approx(0.0));
		CHECK(p.d == doctest::Approx(5.0));

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Plane constructor from normal and d")
	{
		exec_lua("return Plane(Vector3(0, 1, 0), 10.0)");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::PLANE);

		Plane p = result;
		CHECK(p.normal.x == doctest::Approx(0.0));
		CHECK(p.normal.y == doctest::Approx(1.0));
		CHECK(p.normal.z == doctest::Approx(0.0));
		CHECK(p.d == doctest::Approx(10.0));

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Plane property access")
	{
		exec_lua(R"(
			p = Plane(1, 0, 0, 5)
			return p.normal, p.d
		)");

		double d = lua_tonumber(L, -1);
		Variant normal_v = to_variant(L, -2);
		Vector3 normal = normal_v;

		CHECK(normal.x == doctest::Approx(1.0));
		CHECK(normal.y == doctest::Approx(0.0));
		CHECK(normal.z == doctest::Approx(0.0));
		CHECK(d == doctest::Approx(5.0));

		lua_pop(L, 2);
	}
}

// ============================================================================
// AABB Constructor and Property Tests
// ============================================================================

TEST_SUITE("lua_godotlib - AABB")
{
	TEST_CASE_FIXTURE(RawLuaStateFixture, "AABB constructor")
	{
		exec_lua("return AABB(Vector3(1, 2, 3), Vector3(10, 20, 30))");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::AABB);

		AABB box = result;
		CHECK(box.position.x == doctest::Approx(1.0));
		CHECK(box.position.y == doctest::Approx(2.0));
		CHECK(box.position.z == doctest::Approx(3.0));
		CHECK(box.size.x == doctest::Approx(10.0));
		CHECK(box.size.y == doctest::Approx(20.0));
		CHECK(box.size.z == doctest::Approx(30.0));

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "AABB property access")
	{
		exec_lua(R"(
			box = AABB(Vector3(5, 10, 15), Vector3(50, 100, 150))
			return box.position, box.size
		)");

		Variant size_v = to_variant(L, -1);
		Variant pos_v = to_variant(L, -2);

		Vector3 pos = pos_v;
		Vector3 size = size_v;

		CHECK(pos.x == doctest::Approx(5.0));
		CHECK(pos.y == doctest::Approx(10.0));
		CHECK(pos.z == doctest::Approx(15.0));
		CHECK(size.x == doctest::Approx(50.0));
		CHECK(size.y == doctest::Approx(100.0));
		CHECK(size.z == doctest::Approx(150.0));

		lua_pop(L, 2);
	}
}

// ============================================================================
// Quaternion Constructor and Property Tests
// ============================================================================

TEST_SUITE("lua_godotlib - Quaternion")
{
	TEST_CASE_FIXTURE(RawLuaStateFixture, "Quaternion constructor from components")
	{
		exec_lua("return Quaternion(0.0, 0.0, 0.0, 1.0)");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::QUATERNION);

		Quaternion q = result;
		CHECK(q.x == doctest::Approx(0.0));
		CHECK(q.y == doctest::Approx(0.0));
		CHECK(q.z == doctest::Approx(0.0));
		CHECK(q.w == doctest::Approx(1.0));

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Quaternion constructor from axis-angle")
	{
		exec_lua("return Quaternion(Vector3(0, 1, 0), 1.5708)"); // 90 degrees around Y

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::QUATERNION);

		Quaternion q = result;
		CHECK(q.y == doctest::Approx(0.7071).epsilon(0.001));
		CHECK(q.w == doctest::Approx(0.7071).epsilon(0.001));

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Quaternion property access")
	{
		exec_lua(R"(
			q = Quaternion(0.1, 0.2, 0.3, 0.9)
			return q.x, q.y, q.z, q.w
		)");

		double w = lua_tonumber(L, -1);
		double z = lua_tonumber(L, -2);
		double y = lua_tonumber(L, -3);
		double x = lua_tonumber(L, -4);

		CHECK(x == doctest::Approx(0.1));
		CHECK(y == doctest::Approx(0.2));
		CHECK(z == doctest::Approx(0.3));
		CHECK(w == doctest::Approx(0.9));

		lua_pop(L, 4);
	}
}

// ============================================================================
// Transform2D Constructor Tests
// ============================================================================

TEST_SUITE("lua_godotlib - Transform2D")
{
	TEST_CASE_FIXTURE(RawLuaStateFixture, "Transform2D constructor from rotation and position")
	{
		exec_lua("return Transform2D(1.5708, Vector2(10, 20))"); // 90 degrees

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::TRANSFORM2D);

		Transform2D t = result;
		CHECK(t.get_origin().x == doctest::Approx(10.0));
		CHECK(t.get_origin().y == doctest::Approx(20.0));

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Transform2D constructor from vectors")
	{
		exec_lua("return Transform2D(Vector2(1, 0), Vector2(0, 1), Vector2(5, 10))");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::TRANSFORM2D);

		Transform2D t = result;
		CHECK(t.get_origin().x == doctest::Approx(5.0));
		CHECK(t.get_origin().y == doctest::Approx(10.0));

		lua_pop(L, 1);
	}
}

// ============================================================================
// Basis Constructor Tests
// ============================================================================

TEST_SUITE("lua_godotlib - Basis")
{
	TEST_CASE_FIXTURE(RawLuaStateFixture, "Basis constructor from axis-angle")
	{
		exec_lua("return Basis(Vector3(0, 1, 0), 1.5708)"); // 90 degrees around Y

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::BASIS);

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Basis constructor from vectors")
	{
		exec_lua("return Basis(Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1))");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::BASIS);

		Basis b = result;
		CHECK(b == Basis()); // Identity basis

		lua_pop(L, 1);
	}
}

// ============================================================================
// Transform3D Constructor Tests
// ============================================================================

TEST_SUITE("lua_godotlib - Transform3D")
{
	TEST_CASE_FIXTURE(RawLuaStateFixture, "Transform3D constructor from Basis and origin")
	{
		exec_lua("return Transform3D(Basis(Vector3(0, 1, 0), 0), Vector3(10, 20, 30))");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::TRANSFORM3D);

		Transform3D t = result;
		CHECK(t.origin.x == doctest::Approx(10.0));
		CHECK(t.origin.y == doctest::Approx(20.0));
		CHECK(t.origin.z == doctest::Approx(30.0));

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "Transform3D constructor from vectors")
	{
		exec_lua("return Transform3D(Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), Vector3(5, 10, 15))");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::TRANSFORM3D);

		Transform3D t = result;
		CHECK(t.origin.x == doctest::Approx(5.0));
		CHECK(t.origin.y == doctest::Approx(10.0));
		CHECK(t.origin.z == doctest::Approx(15.0));

		lua_pop(L, 1);
	}
}

// ============================================================================
// Projection Constructor Tests
// ============================================================================

TEST_SUITE("lua_godotlib - Projection")
{
	TEST_CASE_FIXTURE(RawLuaStateFixture, "Projection constructor from vectors")
	{
		exec_lua("return Projection(Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), Vector4(0, 0, 0, 1))");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::PROJECTION);

		Projection p = result;
		CHECK(p == Projection()); // Identity projection

		lua_pop(L, 1);
	}
}

// ============================================================================
// RID Constructor Tests
// ============================================================================

TEST_SUITE("lua_godotlib - RID")
{
	TEST_CASE_FIXTURE(RawLuaStateFixture, "RID constructor")
	{
		exec_lua("return RID()");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::RID);

		RID rid = result;
		CHECK(!rid.is_valid());

		lua_pop(L, 1);
	}
}

// ============================================================================
// Packed Array Constructor Tests
// ============================================================================

TEST_SUITE("lua_godotlib - PackedArrays")
{
	TEST_CASE_FIXTURE(RawLuaStateFixture, "PackedByteArray constructor empty")
	{
		exec_lua("return PackedByteArray()");

		// PackedByteArray() creates an empty buffer
		CHECK(lua_type(L, -1) == LUA_TBUFFER);

		size_t len;
		lua_tobuffer(L, -1, &len);
		CHECK(len == 0);

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "PackedInt32Array constructor empty")
	{
		exec_lua("return PackedInt32Array()");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::PACKED_INT32_ARRAY);

		PackedInt32Array arr = result;
		CHECK(arr.size() == 0);

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "PackedInt64Array constructor empty")
	{
		exec_lua("return PackedInt64Array()");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::PACKED_INT64_ARRAY);

		PackedInt64Array arr = result;
		CHECK(arr.size() == 0);

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "PackedFloat32Array constructor empty")
	{
		exec_lua("return PackedFloat32Array()");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::PACKED_FLOAT32_ARRAY);

		PackedFloat32Array arr = result;
		CHECK(arr.size() == 0);

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "PackedFloat64Array constructor empty")
	{
		exec_lua("return PackedFloat64Array()");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::PACKED_FLOAT64_ARRAY);

		PackedFloat64Array arr = result;
		CHECK(arr.size() == 0);

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "PackedStringArray constructor empty")
	{
		exec_lua("return PackedStringArray()");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::PACKED_STRING_ARRAY);

		PackedStringArray arr = result;
		CHECK(arr.size() == 0);

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "PackedVector2Array constructor empty")
	{
		exec_lua("return PackedVector2Array()");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::PACKED_VECTOR2_ARRAY);

		PackedVector2Array arr = result;
		CHECK(arr.size() == 0);

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "PackedVector3Array constructor empty")
	{
		exec_lua("return PackedVector3Array()");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::PACKED_VECTOR3_ARRAY);

		PackedVector3Array arr = result;
		CHECK(arr.size() == 0);

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "PackedColorArray constructor empty")
	{
		exec_lua("return PackedColorArray()");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::PACKED_COLOR_ARRAY);

		PackedColorArray arr = result;
		CHECK(arr.size() == 0);

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "PackedVector4Array constructor empty")
	{
		exec_lua("return PackedVector4Array()");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::PACKED_VECTOR4_ARRAY);

		PackedVector4Array arr = result;
		CHECK(arr.size() == 0);

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "PackedInt32Array constructor from array")
	{
		exec_lua("return PackedInt32Array({10, 20, 30})");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::PACKED_INT32_ARRAY);

		PackedInt32Array arr = result;
		CHECK(arr.size() == 3);
		CHECK(arr[0] == 10);
		CHECK(arr[1] == 20);
		CHECK(arr[2] == 30);

		lua_pop(L, 1);
	}

	TEST_CASE_FIXTURE(RawLuaStateFixture, "PackedVector2Array constructor from array")
	{
		exec_lua("return PackedVector2Array({Vector2(1, 2), Vector2(3, 4)})");

		Variant result = to_variant(L, -1);
		CHECK(result.get_type() == Variant::PACKED_VECTOR2_ARRAY);

		PackedVector2Array arr = result;
		CHECK(arr.size() == 2);
		CHECK(arr[0].x == doctest::Approx(1.0));
		CHECK(arr[0].y == doctest::Approx(2.0));
		CHECK(arr[1].x == doctest::Approx(3.0));
		CHECK(arr[1].y == doctest::Approx(4.0));

		lua_pop(L, 1);
	}
}
