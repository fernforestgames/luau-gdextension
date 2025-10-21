// Runtime-embedded C++ tests using doctest
// Only available in Debug builds
// Run via: godot --headless --path demo/ -- --run-runtime-tests

#ifndef RUNTIME_TESTS_H
#define RUNTIME_TESTS_H

#ifdef ENABLE_RUNTIME_TESTS

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;

class RuntimeTests : public Object {
	GDCLASS(RuntimeTests, Object)

protected:
	static void _bind_methods();

public:
	RuntimeTests();
	~RuntimeTests();

	// Run all embedded C++ tests
	// Returns Dictionary with: {passed: int, failed: int, assertions_passed: int, assertions_failed: int, success: bool}
	// Also prints detailed test output to stdout
	static Dictionary run();
};

#endif // ENABLE_RUNTIME_TESTS

#endif // RUNTIME_TESTS_H
