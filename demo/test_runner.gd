## Runtime test runner for Godot-Luau GDExtension
## Entry point script that checks for --run-runtime-tests flag
## If present, runs embedded C++ tests. Otherwise, loads normal demo scene.
extends Node

func _ready():
	# Check for --run-runtime-tests flag
	var args = OS.get_cmdline_user_args()

	if "--run-runtime-tests" in args:
		run_runtime_tests()
	else:
		# Normal demo behavior - load main scene
		get_tree().change_scene_to_file("res://main.tscn")

func run_runtime_tests():
	print("=== Running Runtime-Embedded C++ Tests ===")
	print("")

	# Check if RuntimeTests class is available (debug build only)
	if not ClassDB.class_exists("RuntimeTests"):
		push_error("RuntimeTests class not found!")
		push_error("Runtime tests are only available in Debug builds.")
		push_error("Please build with: cmake --preset default && cmake --build --preset default")
		get_tree().quit(1)
		return

	# Run the tests - doctest prints detailed results to stdout
	var results = RuntimeTests.run()

	# Check success (doctest already printed detailed results)
	print("")
	if results.success:
		print("✓ All tests passed!")
		get_tree().quit(0)
	else:
		print("✗ Some tests failed! See output above for details.")
		get_tree().quit(1)
