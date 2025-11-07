#pragma once

#include <godot_cpp/variant/char_string.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace gdluau
{
    using namespace godot;

    void initialize_string_cache();
    void uninitialize_string_cache();

    int16_t create_atom(const char *p_str, size_t p_len);
    StringName string_name_for_atom(int p_atom);

    CharString char_string(const StringName &p_str_name);
} // namespace gdluau
