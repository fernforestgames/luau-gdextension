#!/usr/bin/env -S godot -s
## Test runner script for CTest integration
## Runs a single test case (C++ or GDScript) and exits with appropriate code
## Must be run from within the demo project to properly load GDExtensions
##
## Usage: godot --headless -s run_single_test.gd -- <test_name>
##   test_name format: "cpp:TestCategory: TestName" or "gut:test_file::test_method"
extends SceneTree

func _init():
	var args = OS.get_cmdline_user_args()

	if args.size() == 0:
		printerr("Error: No test name provided")
		printerr("Usage: godot --headless -s run_single_test.gd -- <test_name>")
		quit(1)
		return

	var test_name = args[0]

	# Parse test type from prefix
	if test_name.begins_with("cpp:"):
		run_cpp_test(test_name.substr(4))
	elif test_name.begins_with("gut:"):
		run_gut_test(test_name.substr(4))
	else:
		printerr("Error: Invalid test name format: " + test_name)
		printerr("Expected 'cpp:...' or 'gut:...'")
		quit(1)

func run_cpp_test(test_case: String):
	if not ClassDB.class_exists("LuauGDExtensionTests"):
		printerr("Error: LuauGDExtensionTests class not found")
		quit(1)
		return

	# Run single doctest test case
	var results = LuauGDExtensionTests.run_single(test_case)

	if results.success:
		quit(0)
	else:
		quit(1)

func run_gut_test(test_spec: String):
	# Parse test spec: "test_file::test_method"
	var parts = test_spec.split("::")
	if parts.size() != 2:
		printerr("Error: Invalid GUT test spec: " + test_spec)
		quit(1)
		return

	var script_name = parts[0]
	var method_name = parts[1]

	# Load GUT configuration
	var GutConfig = load("res://addons/gut/gut_config.gd")
	var gut_config = GutConfig.new()

	# Configure to run specific test
	gut_config.options.dirs = ["res://test"]
	gut_config.options.prefix = "test_"
	gut_config.options.suffix = ".gd"
	gut_config.options.log_level = 1
	gut_config.options.should_exit = false
	gut_config.options.should_print_to_console = true
	gut_config.options.selected = script_name + "." + method_name

	# Load and instantiate GutRunner scene
	var GutRunner = load("res://addons/gut/gui/GutRunner.tscn")
	var runner = GutRunner.instantiate()
	runner.set_gut_config(gut_config)

	# Add to tree and run
	root.add_child(runner)

	# Wait for runner to be ready and run tests
	await runner.ready
	runner.run_tests()

	# Wait for tests to complete
	var gut = runner.get_gut()
	if gut:
		await gut.end_run

		var failed = gut.get_fail_count()
		if failed == 0:
			quit(0)
		else:
			quit(1)
	else:
		printerr("Error: Failed to get Gut instance")
		quit(1)
