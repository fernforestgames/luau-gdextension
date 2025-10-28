extends Node

func _ready():
	print("Testing LuauScript resource loader/saver...")

	# Test 1: Load an existing .luau file
	print("\n=== Test 1: Loading existing script ===")
	var script: LuauScript = load("res://test_script.luau")
	if script:
		print("✓ Successfully loaded test_script.luau")
		print("  Source code length: ", script.get_source_code().length())
		print("  First 100 chars: ", script.get_source_code().substr(0, 100))
	else:
		print("✗ Failed to load test_script.luau")

	# Test 2: Create and save a new .lua file
	print("\n=== Test 2: Creating and saving new script ===")
	var new_script = LuauScript.new()
	new_script.set_source_code("""-- Test Lua script
print("Hello from Luau!")

function add(a, b)
	return a + b
end

print("2 + 3 = " .. add(2, 3))
""")

	var err = ResourceSaver.save(new_script, "user://test_saved_script.lua")
	if err == OK:
		print("✓ Successfully saved to user://test_saved_script.lua")
	else:
		print("✗ Failed to save script, error: ", err)

	# Test 3: Load the saved file back
	print("\n=== Test 3: Loading saved script ===")
	var loaded_script: LuauScript = load("user://test_saved_script.lua")
	if loaded_script:
		print("✓ Successfully loaded user://test_saved_script.lua")
		print("  Source matches: ", loaded_script.get_source_code() == new_script.get_source_code())
	else:
		print("✗ Failed to load saved script")

	# Test 4: Try loading both .lua and .luau extensions
	print("\n=== Test 4: Extension recognition ===")
	print("  .luau extension recognized: ", script != null)
	print("  .lua extension recognized: ", loaded_script != null)

	print("\n=== Tests complete ===")
	get_tree().quit()
