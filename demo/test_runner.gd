## Runtime test runner for Godot-Luau GDExtension
## Entry point script that checks for --run-tests flag
## If present, runs embedded C++ tests. Otherwise, loads normal demo scene.
extends Node

func _ready():
	# Check for --run-tests flag
	var args = OS.get_cmdline_user_args()

	if "--run-tests" in args:
		run_runtime_tests()
	else:
		# Normal demo behavior - load main scene
		get_tree().change_scene_to_file("res://main.tscn")

func run_runtime_tests():
	var cpp_success = run_cpp_tests()
	var gut_success = await run_gut_tests()

	# Exit with combined result
	print("")
	print("=== Test Summary ===")
	if cpp_success and gut_success:
		print("✓ All tests passed!")
		get_tree().quit(0)
	else:
		if not cpp_success:
			print("✗ C++ tests failed")
		if not gut_success:
			print("✗ GDScript tests failed")
		get_tree().quit(1)

func run_cpp_tests() -> bool:
	print("=== Running C++ Tests ===")
	print("")

	# Check if LuauGDExtensionTests class is available
	# This class is in a separate test library (gdluau_tests) built when BUILD_TESTING=ON
	if not ClassDB.class_exists("LuauGDExtensionTests"):
		print("⚠ LuauGDExtensionTests class not found - C++ tests not available")
		print("  The test library (gdluau_tests) was not loaded.")
		print("  To enable C++ tests, build with:")
		print("    cmake --preset <platform>-debug")
		print("    cmake --build --preset <platform>-debug")
		print("")
		print("  Continuing with GDScript tests only...")
		print("")
		return true  # Not a failure, just skip C++ tests

	# Run the tests - doctest prints detailed results to stdout
	var results = LuauGDExtensionTests.run()

	# Check success (doctest already printed detailed results)
	print("")
	if results.success:
		print("✓ C++ tests passed!")
		return true
	else:
		print("✗ C++ tests failed! See output above for details.")
		return false

func run_gut_tests() -> bool:
	print("")
	print("=== Running GDScript Integration Tests (GUT) ===")
	print("")

	# Load GUT configuration
	var GutConfig = load("res://addons/gut/gut_config.gd")
	var gut_config = GutConfig.new()

	# Configure test options
	gut_config.options.dirs = ["res://test"]
	gut_config.options.prefix = "test_"
	gut_config.options.suffix = ".gd"
	gut_config.options.log_level = 1
	gut_config.options.should_exit = false
	gut_config.options.should_print_to_console = true

	# Load and instantiate GutRunner scene
	var GutRunner = load("res://addons/gut/gui/GutRunner.tscn")
	var runner = GutRunner.instantiate()
	runner.set_gut_config(gut_config)

	# Add to tree (deferred to avoid busy parent error)
	get_tree().root.call_deferred("add_child", runner)

	# Wait for runner to be ready
	await runner.ready

	# Run the tests
	runner.run_tests()

	# Wait for tests to complete - check what signal GutRunner emits
	var gut = runner.get_gut()
	if gut:
		await gut.end_run

		# Get results
		var passed = gut.get_pass_count()
		var failed = gut.get_fail_count()

		print("")
		print("GUT Results: %d passed, %d failed" % [passed, failed])

		return failed == 0
	else:
		push_error("Failed to get Gut instance from runner")
		return false
