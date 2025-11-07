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

    TEST_CASE_FIXTURE(LuaStateFixture, "push_userdata and to_userdata - basic round-trip")
    {
        Object *obj = state.ptr();

        state->push_userdata(obj);
        CHECK(state->is_userdata(-1));
        CHECK(state->is_full_userdata(-1));

        Object *result = state->to_userdata(-1);
        CHECK(result == obj);

        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "push_userdata - with tag")
    {
        Object *obj = state.ptr();
        int tag = 99;

        state->push_userdata(obj, tag);

        int result_tag = state->userdata_tag(-1);
        CHECK(result_tag == tag);

        Object *result = state->to_userdata(-1, tag);
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
        state->push_userdata(obj);
        result = state->to_object(-1);
        CHECK(result == obj);
        state->pop(1);
    }

    TEST_CASE_FIXTURE(LuaStateFixture, "light_userdata - null object")
    {
        state->push_light_userdata(nullptr);
        CHECK(state->is_light_userdata(-1));

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
        state->push_userdata(obj1, tag1);
        CHECK(state->userdata_tag(-1) == tag1);

        // Change tag
        state->set_userdata_tag(-1, tag2);
        CHECK(state->userdata_tag(-1) == tag2);

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
        state->push_userdata(obj);
        uintptr_t ptr2 = state->to_pointer(-1);
        // Note: for full userdata, to_pointer returns address of the userdata block
        CHECK(ptr2 != 0);
        state->pop(1);
    }
}
