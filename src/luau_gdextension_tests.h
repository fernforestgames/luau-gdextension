// Runtime-embedded C++ tests using doctest
// Only available in Debug builds
// Run via: godot --headless --path demo/ -- --run-tests

#pragma once

#ifdef ENABLE_LUAU_GDEXTENSION_TESTS

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

	// Run all embedded C++ tests
	// Returns Dictionary with: {passed: int, failed: int, assertions_passed: int, assertions_failed: int, success: bool}
	// Also prints detailed test output to stdout
	static Dictionary run();
};

#endif // ENABLE_LUAU_GDEXTENSION_TESTS
