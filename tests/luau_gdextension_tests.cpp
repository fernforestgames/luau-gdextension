// C++ tests using doctest
// Built as separate GDExtension (gdluau_tests)

#include "luau_gdextension_tests.h"

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <vector>
#include <string>

using namespace godot;

// Custom doctest reporter to capture test case names during query operations
struct TestListReporter : public doctest::IReporter
{
	// Static storage for test names so we can access them after Context::run()
	static std::vector<std::string> test_names;

	TestListReporter(const doctest::ContextOptions &)
	{
		godot::UtilityFunctions::print("TestListReporter constructor called");
		// Clear previous results
		test_names.clear();
	}

	void report_query(const doctest::QueryData &qd) override
	{
		godot::UtilityFunctions::print("report_query() called");
		godot::UtilityFunctions::print("  qd.data: ", (void*)qd.data);
		godot::UtilityFunctions::print("  qd.num_data: ", qd.num_data);
		godot::UtilityFunctions::print("  qd.run_stats: ", (void*)qd.run_stats);

		// Called when using --list-test-cases
		// qd.data contains array of test case info
		if (qd.data && !qd.run_stats)
		{
			godot::UtilityFunctions::print("  Processing ", qd.num_data, " test cases");
			for (unsigned i = 0; i < qd.num_data; ++i)
			{
				godot::UtilityFunctions::print("  Test [", i, "]: ", qd.data[i]->m_name);
				test_names.push_back(qd.data[i]->m_name);
			}
		}
	}

	void test_run_start() override
	{
		godot::UtilityFunctions::print("test_run_start() called");
	}

	void test_run_end(const doctest::TestRunStats &) override
	{
		godot::UtilityFunctions::print("test_run_end() called");
	}

	void test_case_start(const doctest::TestCaseData &) override {}
	void test_case_reenter(const doctest::TestCaseData &) override {}
	void test_case_end(const doctest::CurrentTestCaseStats &) override {}
	void test_case_exception(const doctest::TestCaseException &) override {}
	void subcase_start(const doctest::SubcaseSignature &) override {}
	void subcase_end() override {}
	void log_assert(const doctest::AssertData &) override {}
	void log_message(const doctest::MessageData &) override {}
	void test_case_skipped(const doctest::TestCaseData &) override {}
};

// Static storage definition
std::vector<std::string> TestListReporter::test_names;

// Register the custom reporter with doctest
DOCTEST_REGISTER_REPORTER("test_list", 0, TestListReporter);

LuauGDExtensionTests::LuauGDExtensionTests()
{
}

LuauGDExtensionTests::~LuauGDExtensionTests()
{
}

void LuauGDExtensionTests::_bind_methods()
{
	ClassDB::bind_static_method("LuauGDExtensionTests", D_METHOD("run"), &LuauGDExtensionTests::run);
	ClassDB::bind_static_method("LuauGDExtensionTests", D_METHOD("list_tests"), &LuauGDExtensionTests::list_tests);
	ClassDB::bind_static_method("LuauGDExtensionTests", D_METHOD("run_single", "test_case"), &LuauGDExtensionTests::run_single);
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

Dictionary LuauGDExtensionTests::list_tests()
{
	UtilityFunctions::print("list_tests() called");

	// Configure doctest to list test cases using our custom reporter
	UtilityFunctions::print("Creating doctest Context...");
	doctest::Context context;

	UtilityFunctions::print("Setting options...");
	context.setOption("list-test-cases", true);
	context.setOption("reporters", "test_list");
	context.setOption("no-colors", true);

	// Run doctest - this will populate TestListReporter::test_names
	UtilityFunctions::print("Running doctest context...");
	int result = context.run();
	UtilityFunctions::print("doctest context.run() returned: ", result);

	UtilityFunctions::print("TestListReporter::test_names size: ", (int)TestListReporter::test_names.size());

	// Convert test names to Godot Array
	Array test_array;
	for (const auto &test_name : TestListReporter::test_names)
	{
		UtilityFunctions::print("Adding test: ", test_name.c_str());
		test_array.push_back(String(test_name.c_str()));
	}

	UtilityFunctions::print("Returning results with ", test_array.size(), " tests");

	Dictionary results;
	results["tests"] = test_array;
	return results;
}

Dictionary LuauGDExtensionTests::run_single(const String &test_case)
{
	// Configure doctest to run a specific test case
	doctest::Context context;
	context.setOption("no-breaks", true);
	context.setOption("no-colors", false);
	context.setOption("duration", true);

	// Set test case filter
	CharString test_case_utf8 = test_case.utf8();
	context.setOption("test-case", test_case_utf8.get_data());

	// Run the specified test
	int result = context.run();

	Dictionary results;
	results["success"] = (result == 0);

	return results;
}
