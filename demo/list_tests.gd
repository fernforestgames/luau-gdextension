#!/usr/bin/env -S godot -s
## Test discovery script for CTest integration
## Lists all available test cases (both C++ and GDScript) one per line
## Must be run from within the demo project to properly load GDExtensions
extends SceneTree

func _init():
	var all_tests = []

	# Discover C++ tests (if test library is available)
	if ClassDB.class_exists("LuauGDExtensionTests"):
		var cpp_tests = discover_cpp_tests()
		for test_name in cpp_tests:
			all_tests.append("cpp:" + test_name)

	# Discover GDScript tests (GUT framework)
	var gut_tests = discover_gut_tests()
	for test_name in gut_tests:
		all_tests.append("gut:" + test_name)

	# Output test names (one per line)
	for test_name in all_tests:
		print(test_name)

	quit()

func discover_cpp_tests() -> Array:
	# Use the LuauGDExtensionTests.list_tests() method which uses
	# a custom doctest reporter to get all test case names
	var results = LuauGDExtensionTests.list_tests()
	if results and results.has("tests"):
		return results.tests
	return []

func discover_gut_tests() -> Array:
	# Use GUT's TestCollector to discover tests - this matches GUT's own discovery logic
	var TestCollector = load("res://addons/gut/test_collector.gd")
	var collector = TestCollector.new()

	# Configure collector to search test directory
	collector.set_test_class_prefix("Test") # Default GUT prefix
	collector.add_directory("res://test")

	# Collect all test scripts
	var collected_scripts = collector.scripts

	var tests = []
	for script_obj in collected_scripts:
		# CollectedScript has properties: path, inner_class_name, tests (array of test names)
		var script_name = script_obj.path.get_file().trim_suffix(".gd")
		for test_info in script_obj.tests:
			# test_info has properties like: name, has_parameters, etc.
			tests.append(script_name + "::" + test_info.name)

	return tests
