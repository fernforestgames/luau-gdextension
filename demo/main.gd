extends Node2D

var L: LuaState
var _step_count := 0

func _ready() -> void:
    var test_script := FileAccess.get_file_as_string("res://test_script.luau")

    L = LuaState.new()
    L.openlibs() # Loads all libraries including godot math types

    # Set up test globals BEFORE sandboxing
    # Note: Use floats since Lua numbers round-trip as floats
    var godot_array := [100.0, 200.0, 300.0, 400.0]
    L.pushvariant(godot_array)
    L.setglobal("test_array")

    var godot_dict := {"foo": "bar", "count": 42.0, "active": true}
    L.pushvariant(godot_dict)
    L.setglobal("test_dict")

    L.sandbox() # Now lock it down

    L.singlestep(true)
    L.step.connect(self._on_step)

    var bytecode := Luau.compile(test_script)
    print("Luau script compiled")

    var load_status := L.load_bytecode(bytecode, "test_script")
    if load_status != Luau.LUA_OK:
        push_error("Failed to load Lua bytecode: ", load_status)
        return

    print("Luau bytecode loaded, starting execution")
    var resume_status := L.resume()
    if resume_status != Luau.LUA_BREAK and resume_status != Luau.LUA_OK:
        var error_msg := L.tostring(-1) if L.gettop() > 0 else "Unknown error"
        push_error("Failed to start Luau execution: error ", resume_status, ": ", error_msg)
        return

func _on_step(state: LuaState) -> void:
    self._step_count += 1
    if self._step_count < 50:
        return

    print("Pausing at step ", self._step_count)
    state.pause()

    # Disable single-step mode and resume execution after a brief pause
    L.singlestep(false)
    self._resume_after_break.call_deferred()

func _resume_after_break() -> void:
    print("Resuming Luau execution")
    var resume_status := L.resume()
    if resume_status != Luau.LUA_OK:
        var error_msg := L.tostring(-1) if L.gettop() > 0 else "Unknown error"
        push_error("Failed to resume Luau execution: error ", resume_status, ": ", error_msg)
        L.pop(1) # Pop error message
        return

    print("Luau script execution completed")
    _test_array_dict_conversion()
    print("Exiting cleanly")
    get_tree().quit()

func _test_array_dict_conversion() -> void:
    print("\n=== Testing Array/Dictionary Conversion from GDScript ===")

    # Expected values (set before sandboxing in _ready)
    var expected_array := [100.0, 200.0, 300.0, 400.0]
    var expected_dict := {"foo": "bar", "count": 42.0, "active": true}

    # Test retrieving array-like table from Lua
    L.getglobal("test_array")
    var retrieved_array := L.toarray(-1)
    L.pop(1)
    print("Retrieved array: ", retrieved_array, " (type: ", typeof(retrieved_array), ")")
    assert(retrieved_array is Array, "Should be an Array")
    assert(retrieved_array == expected_array, "Array values should match")

    # Test retrieving dictionary from Lua
    L.getglobal("test_dict")
    var retrieved_dict := L.todictionary(-1)
    L.pop(1)
    print("Retrieved dictionary: ", retrieved_dict, " (type: ", typeof(retrieved_dict), ")")
    assert(retrieved_dict is Dictionary, "Should be a Dictionary")

    # Test nested structures (use floats for round-trip)
    var nested := {
        "numbers": [1.0, 2.0, 3.0, 4.0, 5.0],
        "data": {"x": 10.0, "y": 20.0},
        "name": "test"
    }
    L.pushvariant(nested)
    var retrieved_nested := L.todictionary(-1)
    L.pop(1)
    print("Nested structure round-trip successful")
    assert(retrieved_nested["numbers"] is Array, "Nested array should be Array")
    assert(retrieved_nested["data"] is Dictionary, "Nested dict should be Dictionary")

    print("=== All Array/Dictionary tests passed! ===")
