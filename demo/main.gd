extends Node2D

var L: LuaState
var _step_count := 0

func _ready() -> void:
    var test_script := FileAccess.get_file_as_string("res://test_script.luau")

    L = LuaState.new()
    L.open_libs()
    L.register_math_types() # Register math type constructors and metatables

    L.singlestep(true)
    L.step.connect(self._on_step)

    var bytecode := Luau.compile(test_script)
    print("Luau script compiled")

    var load_status := L.load_bytecode(bytecode, "test_script")
    if load_status != Luau.LUA_OK:
        push_error("Failed to load Lua bytecode")
        return

    print("Luau bytecode loaded, starting execution")
    var resume_status := L.resume()
    if resume_status != Luau.LUA_BREAK and resume_status != Luau.LUA_OK:
        push_error("Failed to start Luau execution")
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
        push_error("Failed to resume Luau execution")
        return

    print("Luau script execution completed, exiting cleanly")
    get_tree().quit()
