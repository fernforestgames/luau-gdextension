#pragma once

#include <godot_cpp/variant/string_name.hpp>

namespace godot
{
    class StringName;

    void initialize_atom_cache();
    void cleanup_atom_cache();

    int16_t create_atom(const char *p_str, size_t p_len);
    StringName string_name_for_atom(int p_atom);
} // namespace godot
