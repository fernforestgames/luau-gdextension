#pragma once

#include <godot_cpp/variant/string_name.hpp>

namespace gdluau
{
    using namespace godot;

    struct StaticStrings
    {
        StringName interrupt;
        StringName debugbreak;
        StringName debugstep;
    };

    extern StaticStrings *static_strings;

    void initialize_static_strings();
    void uninitialize_static_strings();
}
