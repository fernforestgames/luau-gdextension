// Tests for bridging/object - Object bridging between Godot and Lua
// to_object, push_light_object, push_full_object

#include "doctest.h"
#include "test_fixtures.h"
#include "lua_state.h"

using namespace gdluau;
using namespace godot;

TEST_SUITE("Bridging - Object")
{
    TEST_CASE_FIXTURE(LuaStateFixture, "push_light_userdata and to_light_userdata - basic round-trip")
    {
        // Create a dummy object (we can use the LuaState itself)
        Object *obj = state.ptr();

        state->push_light_userdata(obj);
        CHECK(state->is_light_userdata(-1));

        Object *result = state->to_light_userdata(-1);
        CHECK(result == obj);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_light_userdata - with tag")
    {
        Object *obj = state.ptr();
        int tag = 42;

        state->push_light_userdata(obj, tag);
        CHECK(state->is_light_userdata(-1));

        int result_tag = state->light_userdata_tag(-1);
        CHECK(result_tag == tag);

        Object *result = state->to_light_userdata(-1, tag);
        CHECK(result == obj);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_object and to_full_userdata - basic round-trip")
    {
        Object *obj = state.ptr();

        state->push_object(obj);
        CHECK(state->is_userdata(-1));
        CHECK(state->is_full_userdata(-1));

        Object *result = state->to_full_userdata(-1);
        CHECK(result == obj);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_object - with tag")
    {
        Object *obj = state.ptr();
        int tag = 99;

        state->push_object(obj, tag);

        int result_tag = state->full_userdata_tag(-1);
        CHECK(result_tag == tag);

        Object *result = state->to_full_userdata(-1, tag);
        CHECK(result == obj);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_object - detects both light and full userdata")
    {
        Object *obj = state.ptr();

        // Test with light userdata
        state->push_light_userdata(obj);
        Object *result = state->to_object(-1);
        CHECK(result == obj);
        state->pop(1);

        // Test with full userdata
        state->push_object(obj);
        result = state->to_object(-1);
        CHECK(result == obj);
        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "light_userdata - null object")
    {
        state->push_light_userdata(nullptr);
        CHECK(state->is_nil(-1));

        Object *result = state->to_light_userdata(-1);
        CHECK(result == nullptr);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "userdata - tag management")
    {
        Object *obj1 = state.ptr();
        int tag1 = 10;
        int tag2 = 20;

        // Push with tag1
        state->push_object(obj1, tag1);
        CHECK(state->full_userdata_tag(-1) == tag1);

        // Change tag
        state->set_full_userdata_tag(-1, tag2);
        CHECK(state->full_userdata_tag(-1) == tag2);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "light_userdata - tag names")
    {
        int tag = 42;
        StringName tag_name = "MyCustomTag";

        state->set_light_userdata_name(tag, tag_name);

        StringName result = state->get_light_userdata_name(tag);
        CHECK(result == tag_name);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_pointer - gets pointer value")
    {
        Object *obj = state.ptr();

        // Test with light userdata
        state->push_light_userdata(obj);
        uintptr_t ptr1 = state->to_pointer(-1);
        CHECK(ptr1 == reinterpret_cast<uintptr_t>(obj));
        state->pop(1);

        // Test with full userdata
        state->push_object(obj);
        uintptr_t ptr2 = state->to_pointer(-1);
        // Note: for full userdata, to_pointer returns address of the userdata block
        CHECK(ptr2 != 0);
        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_default_object_metatable - exposes default metatable")
    {
        state->push_default_object_metatable();
        CHECK(state->is_table(-1));

        // Verify it has expected metamethods
        state->get_field(-1, "__tostring");
        CHECK(state->is_function(-1));
        state->pop(1);

        state->get_field(-1, "__eq");
        CHECK(state->is_function(-1));
        state->pop(1);

        state->get_field(-1, "__lt");
        CHECK(state->is_function(-1));
        state->pop(1);

        state->get_field(-1, "__le");
        CHECK(state->is_function(-1));
        state->pop(1);

        state->pop(1); // Pop metatable
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "custom metatable with __index inheritance")
    {
        Object *obj = state.ptr();

        // Push object
        state->push_object(obj);

        // Create custom metatable that inherits from default
        CHECK(state->new_metatable_named("CustomObject"));
        state->push_default_object_metatable();
        state->set_field(-2, "__index");

        // Set the custom metatable
        state->set_metatable(-2);

        // Should still be able to convert to object (inherits from Object metatable)
        Object *result = state->to_full_userdata(-1);
        CHECK(result == obj);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "custom metatable multiple levels of inheritance")
    {
        Object *obj = state.ptr();

        // Push object
        state->push_object(obj);

        // Create first level: inherits from default
        CHECK(state->new_metatable_named("Level1"));
        state->push_default_object_metatable();
        state->set_field(-2, "__index");
        state->set_metatable(-2);

        // Replace with second level: inherits from Level1
        CHECK(state->get_metatable(-1));
        state->create_table(); // Create Level2 metatable
        state->push_value(-2); // Push Level1 metatable
        state->set_field(-2, "__index");
        state->set_metatable(-3);
        state->pop(1); // Pop Level1 metatable

        // Should still be able to convert (walks inheritance chain)
        Object *result = state->to_full_userdata(-1);
        CHECK(result == obj);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "tagged userdata without custom metatable still works")
    {
        Object *obj = state.ptr();
        int tag = 60;

        // Don't create custom metatable for this tag

        // Push tagged object
        state->push_object(obj, tag);

        // Should be able to convert
        Object *result = state->to_full_userdata(-1, tag);
        CHECK(result == obj);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "tag mismatch returns null")
    {
        Object *obj = state.ptr();
        int tag1 = 70;
        int tag2 = 71;

        // Push with tag1
        state->push_object(obj, tag1);

        // Try to read with tag2
        Object *result = state->to_full_userdata(-1, tag2);
        CHECK(result == nullptr);

        // Reading with tag1 should work
        result = state->to_full_userdata(-1, tag1);
        CHECK(result == obj);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "untagged userdata requires object metatable")
    {
        // Create a RefCounted object to test
        Ref<RefCounted> ref = memnew(RefCounted);

        // Push it normally (should have Object metatable)
        state->push_object(ref.ptr());

        // Should be able to convert
        Object *result = state->to_full_userdata(-1);
        CHECK(result == ref.ptr());

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "default object metatable has basic metamethods")
    {
        Object *obj = state.ptr();

        // Test __tostring
        exec_lua_ok(R"(
            function test_tostring(o)
                return tostring(o)
            end
        )");
        state->get_global("test_tostring");
        state->push_object(obj);
        state->pcall(1, 1);
        CHECK(state->is_string(-1));
        state->pop(1);

        // Test __eq
        exec_lua_ok(R"(
            function test_eq(a, b)
                return a == b
            end
        )");
        state->get_global("test_eq");
        state->push_object(obj);
        state->push_object(obj);
        state->pcall(2, 1);
        CHECK(state->to_boolean(-1));
        state->pop(1);

        // Test __lt with different objects
        Ref<RefCounted> obj1 = memnew(RefCounted);
        Ref<RefCounted> obj2 = memnew(RefCounted);

        exec_lua_ok(R"(
            function test_lt(a, b)
                return a < b
            end
        )");
        state->get_global("test_lt");
        state->push_object(obj1.ptr());
        state->push_object(obj2.ptr());
        state->pcall(2, 1);
        // Result depends on object IDs, just verify it returns a boolean
        CHECK(state->is_boolean(-1));
        state->pop(1);

        // Test __le with different objects
        exec_lua_ok(R"(
            function test_le(a, b)
                return a <= b
            end
        )");
        state->get_global("test_le");
        state->push_object(obj1.ptr());
        state->push_object(obj2.ptr());
        state->pcall(2, 1);
        CHECK(state->is_boolean(-1));
        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "to_full_userdata returns null for non-object userdata")
    {
        SUBCASE("Variant userdata")
        {
            // Push a Variant (which creates userdata with GDVariant metatable)
            Variant test_variant = Vector2(1.0f, 2.0f);
            state->push_variant(test_variant);
            CHECK(state->is_userdata(-1));

            // to_full_userdata should return null because this is not an Object
            Object *result = state->to_full_userdata(-1);
            CHECK(result == nullptr);

            state->pop(1);
        }

        SUBCASE("Callable userdata")
        {
            Callable test_callable = Callable(state.ptr(), "test_func");

            // Push the callable (creates userdata with GDCallable metatable)
            state->push_callable(test_callable);
            CHECK(state->is_userdata(-1));

            // to_full_userdata should return null because this is not an Object
            Object *result = state->to_full_userdata(-1);
            CHECK(result == nullptr);

            state->pop(1);
        }
    }
}
