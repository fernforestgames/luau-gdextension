// GDExtension registration for test library
#pragma once

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void initialize_gdluau_tests(ModuleInitializationLevel p_level);
void uninitialize_gdluau_tests(ModuleInitializationLevel p_level);
