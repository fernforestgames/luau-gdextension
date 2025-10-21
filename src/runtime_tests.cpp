// Runtime-embedded C++ tests using doctest
// Only available in Debug builds

#include "runtime_tests.h"

#ifdef ENABLE_RUNTIME_TESTS

#define DOCTEST_CONFIG_IMPLEMENT
#include "../tests/doctest.h"

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

RuntimeTests::RuntimeTests() {
}

RuntimeTests::~RuntimeTests() {
}

void RuntimeTests::_bind_methods() {
	ClassDB::bind_static_method("RuntimeTests", D_METHOD("run"), &RuntimeTests::run);
}

Dictionary RuntimeTests::run() {
	// Configure doctest
	doctest::Context context;
	context.setOption("no-breaks", true);     // Don't break on failures
	context.setOption("no-colors", false);    // Enable colors in output
	context.setOption("duration", true);      // Show test durations

	// Run all tests - doctest outputs results to stdout
	int result = context.run();

	// doctest doesn't expose internal stats directly via getters.
	// Results are printed to stdout, and run() returns 0 on success, non-zero on failure.
	// For detailed stats, parse the stdout output or use a custom reporter.
	// For now, we just return success/failure status.
	Dictionary results;
	results["success"] = (result == 0);

	return results;
}

#endif // ENABLE_RUNTIME_TESTS
