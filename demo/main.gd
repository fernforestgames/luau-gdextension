extends Node2D

var L: LuaState

func _ready() -> void:
    var test_script := FileAccess.get_file_as_string("res://demo/test_script.luau")

    L = LuaState.new()
    L.open_libs()
    L.register_math_types() # Register math type constructors and metatables

    var bytecode := Luau.compile(test_script)
    print("Luau script compiled")

    var load_status := L.load_bytecode(bytecode, "test_script")
    if load_status != Luau.LUA_OK:
        push_error("Failed to load Lua bytecode")
        return

    print("Luau bytecode loaded")
    L.break_after_steps = 50
    L.break.connect(self._on_break)

    print("Starting script execution")
    var resume_status := L.resume()
    if resume_status != Luau.LUA_BREAK and resume_status != Luau.LUA_OK:
        push_error("Failed to start Luau execution")
        return

func _on_break(state: LuaState) -> void:
    var step_count = state.get_step_count()
    print("Hit Luau breakpoint at step ", step_count)
    self._resume_after_break.call_deferred()

func _resume_after_break() -> void:
    print("Resuming Luau execution")
    var resume_status := L.resume()
    if resume_status != Luau.LUA_OK:
        push_error("Failed to resume Luau execution")
        return

    print("Luau script execution completed, exiting cleanly")
    get_tree().quit()
