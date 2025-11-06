// Tests for LuauScript class

#include "doctest.h"
#include "test_fixtures.h"
#include "luau_script.h"

using namespace godot;

TEST_SUITE("LuauScript")
{
    TEST_CASE("constructor - creates empty script")
    {
        Ref<LuauScript> script = memnew(LuauScript);

        CHECK(script.is_valid());
        CHECK(script->get_source_code() == "");
    }

    TEST_CASE("set_source_code and get_source_code")
    {
        Ref<LuauScript> script = memnew(LuauScript);

        String code = "return 42";
        script->set_source_code(code);

        CHECK(script->get_source_code() == code);
    }

    TEST_CASE("set_source_code - multiline")
    {
        Ref<LuauScript> script = memnew(LuauScript);

        String code = R"(
function test()
    return "hello"
end
)";
        script->set_source_code(code);

        CHECK(script->get_source_code() == code);
    }

    TEST_CASE("set_source_code - empty string")
    {
        Ref<LuauScript> script = memnew(LuauScript);

        script->set_source_code("initial");
        CHECK(script->get_source_code() == "initial");

        script->set_source_code("");
        CHECK(script->get_source_code() == "");
    }

    TEST_CASE("integration - compile and execute")
    {
        Ref<LuauScript> script = memnew(LuauScript);
        script->set_source_code("return 1 + 2");

        // Compile the script
        PackedByteArray bytecode = Luau::compile(script->get_source_code());
        CHECK(bytecode.size() > 0);

        // Execute it
        LuaStateFixture f;
        f.state->load_bytecode(bytecode, "test_script");
        lua_Status status = f.state->resume();

        CHECK(status == LUA_OK);
        CHECK(f.state->to_number(-1) == 3.0);

        f.state->pop(1);
    }
}
