// C++ tests using doctest
// Built as separate shared library (gdluau_tests)
// Run via: godot --headless --path demo/ -- --run-tests

#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;

class LuauGDExtensionTests : public Object
{
	GDCLASS(LuauGDExtensionTests, Object)

protected:
	static void _bind_methods();

public:
	LuauGDExtensionTests();
	~LuauGDExtensionTests();

	// Run all C++ tests using doctest
	// Returns Dictionary with: {success: bool}
	// Also prints detailed test output to stdout
	static Dictionary run();

	// List all available test cases
	// Returns Dictionary with: {tests: Array[String]}
	static Dictionary list_tests();

	// Run a single test case by name
	// Returns Dictionary with: {success: bool}
	static Dictionary run_single(const String &test_case);
};
