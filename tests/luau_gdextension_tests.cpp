// C++ tests using doctest
// Built as separate GDExtension (gdluau_tests)

#include "luau_gdextension_tests.h"

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

LuauGDExtensionTests::LuauGDExtensionTests()
{
}

LuauGDExtensionTests::~LuauGDExtensionTests()
{
}

void LuauGDExtensionTests::_bind_methods()
{
	ClassDB::bind_static_method("LuauGDExtensionTests", D_METHOD("run"), &LuauGDExtensionTests::run);
}

Dictionary LuauGDExtensionTests::run()
{
	// Configure doctest
	doctest::Context context;
	context.setOption("no-breaks", true);  // Don't break on failures
	context.setOption("no-colors", false); // Enable colors in output
	context.setOption("duration", true);   // Show test durations
	context.setOption("test-case", "*Edge cases: Stack management*");

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
