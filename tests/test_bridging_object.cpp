// Tests for bridging/object - Object bridging between Godot and Lua
// to_object, push_light_object, push_full_object

#include "doctest.h"
#include "test_fixtures.h"
#include "lua_state.h"

using namespace godot;

TEST_SUITE("Bridging - Object")
{
    TEST_CASE("push_light_userdata and to_light_userdata - basic round-trip")
    {
        LuaStateFixture f;

        // Create a dummy object (we can use the LuaState itself)
        Object *obj = f.state.ptr();

        f.state->push_light_userdata(obj);
        CHECK(f.state->is_light_userdata(-1));

        Object *result = f.state->to_light_userdata(-1);
        CHECK(result == obj);

        f.state->pop(1);
    }

    TEST_CASE("push_light_userdata - with tag")
    {
        LuaStateFixture f;

        Object *obj = f.state.ptr();
        int tag = 42;

        f.state->push_light_userdata(obj, tag);
        CHECK(f.state->is_light_userdata(-1));

        int result_tag = f.state->light_userdata_tag(-1);
        CHECK(result_tag == tag);

        Object *result = f.state->to_light_userdata(-1, tag);
        CHECK(result == obj);

        f.state->pop(1);
    }

    TEST_CASE("push_userdata and to_userdata - basic round-trip")
    {
        LuaStateFixture f;

        Object *obj = f.state.ptr();

        f.state->push_userdata(obj);
        CHECK(f.state->is_userdata(-1));
        CHECK(f.state->is_full_userdata(-1));

        Object *result = f.state->to_userdata(-1);
        CHECK(result == obj);

        f.state->pop(1);
    }

    TEST_CASE("push_userdata - with tag")
    {
        LuaStateFixture f;

        Object *obj = f.state.ptr();
        int tag = 99;

        f.state->push_userdata(obj, tag);

        int result_tag = f.state->userdata_tag(-1);
        CHECK(result_tag == tag);

        Object *result = f.state->to_userdata(-1, tag);
        CHECK(result == obj);

        f.state->pop(1);
    }

    TEST_CASE("to_object - detects both light and full userdata")
    {
        LuaStateFixture f;

        Object *obj = f.state.ptr();

        // Test with light userdata
        f.state->push_light_userdata(obj);
        Object *result = f.state->to_object(-1);
        CHECK(result == obj);
        f.state->pop(1);

        // Test with full userdata
        f.state->push_userdata(obj);
        result = f.state->to_object(-1);
        CHECK(result == obj);
        f.state->pop(1);
    }

    TEST_CASE("light_userdata - null object")
    {
        LuaStateFixture f;

        f.state->push_light_userdata(nullptr);
        CHECK(f.state->is_light_userdata(-1));

        Object *result = f.state->to_light_userdata(-1);
        CHECK(result == nullptr);

        f.state->pop(1);
    }

    TEST_CASE("userdata - tag management")
    {
        LuaStateFixture f;

        Object *obj1 = f.state.ptr();
        int tag1 = 10;
        int tag2 = 20;

        // Push with tag1
        f.state->push_userdata(obj1, tag1);
        CHECK(f.state->userdata_tag(-1) == tag1);

        // Change tag
        f.state->set_userdata_tag(-1, tag2);
        CHECK(f.state->userdata_tag(-1) == tag2);

        f.state->pop(1);
    }

    TEST_CASE("light_userdata - tag names")
    {
        LuaStateFixture f;

        int tag = 42;
        StringName tag_name = "MyCustomTag";

        f.state->set_light_userdata_name(tag, tag_name);

        StringName result = f.state->get_light_userdata_name(tag);
        CHECK(result == tag_name);
    }

    TEST_CASE("to_pointer - gets pointer value")
    {
        LuaStateFixture f;

        Object *obj = f.state.ptr();

        // Test with light userdata
        f.state->push_light_userdata(obj);
        uintptr_t ptr1 = f.state->to_pointer(-1);
        CHECK(ptr1 == reinterpret_cast<uintptr_t>(obj));
        f.state->pop(1);

        // Test with full userdata
        f.state->push_userdata(obj);
        uintptr_t ptr2 = f.state->to_pointer(-1);
        // Note: for full userdata, to_pointer returns address of the userdata block
        CHECK(ptr2 != 0);
        f.state->pop(1);
    }
}
