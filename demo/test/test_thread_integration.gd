extends GutTest
# Integration tests for Lua thread support in LuaState
# Tests thread creation, coroutine execution, and lifecycle from GDScript
#
# NOTE: Some tests intentionally trigger errors/warnings to verify error handling:
# - test_close_thread_warns: Expects warning about closing thread states
# - test_close_parent_invalidates_threads: Expects error about invalid state
# - test_tothread_on_non_thread: Expects error about value not being a thread

var L: LuaState

func before_each() -> void:
	L = LuaState.new()
	L.open_libs() # Load all libraries including coroutine

func after_each() -> void:
	if L:
		L.close()
		L = null


# Helper to verify Lua stack is balanced
func assert_stack_balanced(state: LuaState, expected_top: int = 0) -> void:
	assert_eq(state.get_top(), expected_top, "Lua stack should be balanced at %d, but is %d" % [expected_top, state.get_top()])


# ============================================================================
# Thread Creation and Basic Operations
# ============================================================================

func test_thread_creation() -> void:
	L.new_thread()
	assert_true(L.is_thread(-1), "newthread() should push a thread to the stack")
	assert_eq(L.get_top(), 1, "Stack should have one element after newthread()")

func test_newthread_conversion_to_luastate() -> void:
	var thread: LuaState = L.new_thread()
	L.pop(1)

	assert_not_null(thread, "tothread() should return a LuaState")
	assert_true(thread is LuaState, "Returned value should be LuaState instance")

	assert_stack_balanced(L)

func test_tothread_conversion_to_luastate() -> void:
	L.new_thread()
	var thread: LuaState = L.to_thread(-1)
	L.pop(1)

	assert_not_null(thread, "tothread() should return a LuaState")
	assert_true(thread is LuaState, "Returned value should be LuaState instance")

	assert_stack_balanced(L)

func test_thread_has_separate_stack() -> void:
	# Push value to parent
	L.push_number(123)
	var parent_top: int = L.get_top()

	# Create thread
	var thread: LuaState = L.new_thread()
	L.pop(1) # Remove thread from parent stack

	# Thread should have empty stack
	assert_eq(thread.get_top(), 0, "Thread should start with empty stack")

	# Push to thread
	thread.push_number(456)
	assert_eq(thread.get_top(), 1, "Thread stack should have one element")

	# Parent stack should be unaffected
	assert_eq(L.get_top(), parent_top, "Parent stack should be unaffected by thread operations")
	assert_eq(L.to_number(-1), 123, "Parent stack value should be unchanged")


func test_thread_shares_globals() -> void:
	# Set global in parent
	var code: String = "shared_value = 42"
	assert_eq(L.do_string(code, "test"), 0, "Setting global should succeed")

	# Create thread
	var thread: LuaState = L.new_thread()
	L.pop(1)

	# Access global from thread
	thread.get_global("shared_value")
	assert_eq(thread.to_number(-1), 42, "Thread should see parent globals")
	thread.pop(1)

	# Modify global from thread
	thread.push_number(100)
	thread.set_global("shared_value")
	assert_engine_error("Calling LuaState.set_global() on a Lua thread will affect all threads in the same VM.")

	# Verify change in parent
	L.get_global("shared_value")
	assert_eq(L.to_number(-1), 100, "Parent should see thread's global modifications")


# ============================================================================
# Coroutine Execution
# ============================================================================

func test_simple_coroutine() -> void:
	var code: String = """
	function simple_coro()
		coroutine.yield(1)
		coroutine.yield(2)
		return 3
	end
	"""
	assert_eq(L.do_string(code, "test"), 0, "Loading coroutine function should succeed")

	# Create thread
	var thread: LuaState = L.new_thread()
	L.pop(1)

	# Load function into thread
	thread.get_global("simple_coro")
	assert_true(thread.is_function(-1), "Function should be accessible from thread")

	# Resume and check yields
	assert_eq(thread.resume(0), 1, "First resume should yield") # LUA_YIELD = 1
	assert_eq(thread.to_number(-1), 1, "First yield should return 1")
	thread.pop(1)

	assert_eq(thread.resume(0), 1, "Second resume should yield")
	assert_eq(thread.to_number(-1), 2, "Second yield should return 2")
	thread.pop(1)

	assert_eq(thread.resume(0), 0, "Final resume should complete") # LUA_OK = 0
	assert_eq(thread.to_number(-1), 3, "Final return should be 3")


func test_coroutine_with_parameters() -> void:
	var code: String = """
	function param_coro(a, b)
		local sum = a + b
		coroutine.yield(sum)
		return sum * 2
	end
	"""
	assert_eq(L.do_string(code, "test"), 0, "Loading coroutine should succeed")

	var thread: LuaState = L.new_thread()
	L.pop(1)

	thread.get_global("param_coro")
	thread.push_number(10)
	thread.push_number(20)

	# Resume with 2 arguments
	assert_eq(thread.resume(2), 1, "Should yield")
	assert_eq(thread.to_number(-1), 30, "Should yield sum")
	thread.pop(1)

	assert_eq(thread.resume(0), 0, "Should complete")
	assert_eq(thread.to_number(-1), 60, "Should return doubled value")


func test_coroutine_counter() -> void:
	var code: String = """
	function counter(max)
		for i = 1, max do
			coroutine.yield(i)
		end
		return "done"
	end
	"""
	assert_eq(L.do_string(code, "test"), 0, "Loading counter should succeed")

	var thread: LuaState = L.new_thread()
	L.pop(1)

	thread.get_global("counter")
	thread.push_number(3)

	# Count from 1 to 3
	for i in range(1, 4):
		assert_eq(thread.resume(1 if i == 1 else 0), 1, "Should yield at iteration %d" % i)
		assert_eq(thread.to_number(-1), i, "Should yield %d" % i)
		thread.pop(1)

	assert_eq(thread.resume(0), 0, "Should complete after counting")
	assert_eq(thread.to_string(-1), "done", "Should return completion message")


# ============================================================================
# Thread Lifecycle and Reference Counting
# ============================================================================

func test_thread_survives_parent_scope() -> void:
	var thread: LuaState

	# Create thread in separate block (simulating scope)
	thread = L.new_thread()
	L.pop(1)

	# Thread should still be valid
	assert_not_null(thread, "Thread should survive parent scope")
	thread.push_number(999)
	assert_eq(thread.to_number(-1), 999, "Thread should still be functional")


func test_multiple_threads_same_parent() -> void:
	var thread1: LuaState = L.new_thread()
	L.pop(1)

	var thread2: LuaState = L.new_thread()
	L.pop(1)

	# Both threads should be independent
	thread1.push_number(111)
	thread2.push_number(222)

	assert_eq(thread1.get_top(), 1, "Thread1 should have one value")
	assert_eq(thread2.get_top(), 1, "Thread2 should have one value")
	assert_eq(thread1.to_number(-1), 111, "Thread1 should have its own value")
	assert_eq(thread2.to_number(-1), 222, "Thread2 should have its own value")


func test_close_thread_warns() -> void:
	var thread: LuaState = L.new_thread()
	L.pop(1)

	# Closing thread should warn but not crash
	thread.close()
	assert_engine_error("LuaState.close() should not be called on Lua threads.")

	# Parent should still be functional
	L.push_number(456)
	assert_eq(L.to_number(-1), 456, "Parent should remain functional after thread close")


func test_close_parent_invalidates_threads() -> void:
	var thread: LuaState = L.new_thread()
	L.pop(1)

	thread.push_number(123)
	assert_eq(thread.to_number(-1), 123, "Thread should work before parent closes")
	thread.pop(1)

	# Close parent
	L.close()

	# Thread operations should fail gracefully (print errors)
	# This test verifies no crash occurs
	thread.push_number(456) # Should fail with error message
	assert_engine_error("Lua state is invalid. Cannot push number.")


# ============================================================================
# Thread Bridging via Variants
# ============================================================================

func test_thread_as_variant() -> void:
	var thread: LuaState = L.new_thread()
	L.pop(1)

	# Push thread as variant
	var thread_variant: Variant = thread
	thread.push_variant(thread_variant)

	assert_true(thread.is_thread(-1), "Pushed variant should be a thread on stack")

	# Convert back
	var thread2: LuaState = thread.to_thread(-1)
	thread.pop(1)

	assert_not_null(thread2, "Should be able to convert thread back from stack")
	assert_stack_balanced(L)
	assert_stack_balanced(thread)


func test_thread_in_table() -> void:
	var thread: LuaState = L.new_thread()
	L.pop(1)

	# Store thread in a table
	thread.push_dictionary({"thread_ref": thread, "name": "my_thread"})

	# Retrieve dictionary
	var table: Dictionary = thread.to_dictionary(-1)
	thread.pop(1)

	var retrieved_thread: LuaState = table["thread_ref"]
	assert_not_null(retrieved_thread, "Should retrieve thread from table")
	assert_true(retrieved_thread is LuaState, "Retrieved value should be LuaState instance")

	assert_stack_balanced(L)
	assert_stack_balanced(thread)


# ============================================================================
# Error Handling
# ============================================================================

func test_isthread_returns_false_for_other_types() -> void:
	L.push_number(123)
	assert_false(L.is_thread(-1), "Number should not be a thread")

	L.push_string("test")
	assert_false(L.is_thread(-1), "String should not be a thread")

	L.new_table()
	assert_false(L.is_thread(-1), "Table should not be a thread")

	L.push_nil()
	assert_false(L.is_thread(-1), "Nil should not be a thread")


func test_thread_error_handling() -> void:
	var code: String = """
	function error_func()
		error("Intentional error for testing")
	end
	"""
	assert_eq(L.do_string(code, "test"), 0, "Loading error function should succeed")

	var thread: LuaState = L.new_thread()
	L.pop(1)

	thread.get_global("error_func")

	# pcall should catch the error
	var status: int = thread.pcall(0, 0, 0)
	assert_ne(status, 0, "pcall should return error status")

	# Error message should be on stack
	var error_msg: String = thread.to_string(-1)
	assert_true(error_msg.contains("Intentional error"), "Error message should be on stack")


# ============================================================================
# Real-world Use Case: Producer-Consumer Pattern
# ============================================================================

func test_producer_consumer_coroutine() -> void:
	var code: String = """
	function producer()
		for i = 1, 5 do
			coroutine.yield(i * 10)
		end
	end
	"""
	assert_eq(L.do_string(code, "test"), 0)

	var producer_thread: LuaState = L.new_thread()
	L.pop(1)

	producer_thread.get_global("producer")

	# Consumer: collect all values
	# First resume: function is on stack, narg=0
	# Subsequent resumes: no args on stack, narg=0
	var collected: Array = []
	for i in range(5):
		var status: int = producer_thread.resume(0)
		assert_eq(status, 1, "Producer should yield value %d" % i)
		collected.append(int(producer_thread.to_number(-1)))
		producer_thread.pop(1)

	assert_eq(collected, [10, 20, 30, 40, 50], "Should collect all produced values")

	# Producer should be done
	var status: int = producer_thread.resume(0)
	assert_eq(status, 0, "Producer should complete after all values")


# ============================================================================
# Single-Step Debugging with Thread Creation
# ============================================================================

func test_create_thread_during_single_step() -> void:
	# This test explores whether creating and executing a thread during
	# single-step debugging causes issues. The scenario is:
	# 1. Main state is running in single-step mode
	# 2. During a step callback, we pause the main state
	# 3. Create a new thread and execute a function in it to completion
	# 4. Resume the original state
	var code: String = """
	function main_func()
		local x = 1
		local y = 2
		return x + y
	end

	function thread_func()
		local a = 10
		local b = 20
		return a + b
	end
	"""
	assert_eq(L.do_string(code, "test"), 0, "Loading test functions should succeed")

	var lambda_captures := {
		"step_count": 0,
		"thread_executed": false,
		"thread_result": 0,
		"main_resumed": false,
	}

	# Set up step callback
	var on_step = func(state: LuaState) -> void:
		lambda_captures["step_count"] += 1

		# On the first step, pause and create a thread
		if lambda_captures["step_count"] == 1:
			state.pause()

			# Create a new thread
			var thread: LuaState = L.new_thread()
			L.pop(1) # Remove thread from parent stack

			# Execute a function in the thread
			thread.get_global("thread_func")
			var resume_status: int = thread.resume(0)

			assert_eq(resume_status, 0, "Thread execution should complete successfully")
			lambda_captures["thread_result"] = thread.to_number(-1)
			thread.pop(1)
			lambda_captures["thread_executed"] = true

			L.resume.call_deferred(0)
			lambda_captures["main_resumed"] = true

	L.step.connect(on_step)
	L.single_step(true)

	# Start executing main_func
	L.get_global("main_func")
	var resume_status: int = L.resume(0)
	assert_eq(resume_status, Luau.LUA_BREAK, "Main function should pause on first step")

	assert_eq(lambda_captures["step_count"], 1, "Should have received exactly one step callback")
	assert_true(lambda_captures["thread_executed"], "Thread should have been executed")
	assert_eq(lambda_captures["thread_result"], 30, "Thread should have computed correct result")

	# Wait for deferred resume to complete using GUT's wait_until
	await wait_until(func(): return lambda_captures["main_resumed"], 1, 'waiting for deferred resume to execute')

	# Main function should have completed
	assert_gt(L.get_top(), 0, "Main state should have a result on the stack")

	var main_result: float = L.to_number(-1)
	assert_eq(main_result, 3, "Main function should have computed correct result")
	L.pop(1)

	L.single_step(false)


func test_execute_preexisting_thread_during_single_step() -> void:
	# Create a thread BEFORE entering single-step mode, then execute it during
	# the step callback.
	var code: String = """
	function main_func()
		local x = 1
		local y = 2
		return x + y
	end

	function thread_func()
		local a = 10
		local b = 20
		return a + b
	end
	"""
	assert_eq(L.do_string(code, "test"), 0, "Loading test functions should succeed")

	# Create thread BEFORE enabling single-step mode
	var thread: LuaState = L.new_thread()
	L.pop(1)

	var lambda_captures := {
		"step_count": 0,
		"thread_executed": false,
		"thread_result": 0,
		"main_resumed": false,
	}

	# Set up step callback
	var on_step = func(state: LuaState) -> void:
		lambda_captures["step_count"] += 1

		# On the first step, pause and execute the pre-existing thread
		if lambda_captures["step_count"] == 1:
			state.pause()

			# Execute a function in the pre-existing thread
			thread.get_global("thread_func")
			var resume_status: int = thread.resume(0)

			assert_eq(resume_status, 0, "Thread execution should complete successfully")
			lambda_captures["thread_result"] = thread.to_number(-1)
			thread.pop(1)
			lambda_captures["thread_executed"] = true

			L.resume.call_deferred(0)
			lambda_captures["main_resumed"] = true

	L.step.connect(on_step)
	L.single_step(true)

	# Start executing main_func
	L.get_global("main_func")
	var resume_status: int = L.resume(0)
	assert_eq(resume_status, Luau.LUA_BREAK, "Main function should pause on first step")

	assert_eq(lambda_captures["step_count"], 1, "Should have received exactly one step callback")
	assert_true(lambda_captures["thread_executed"], "Thread should have been executed")
	assert_eq(lambda_captures["thread_result"], 30, "Thread should have computed correct result")

	# Wait for deferred resume to complete
	await wait_until(func(): return lambda_captures["main_resumed"], 1, 'waiting for deferred resume to execute')

	# Main function should have completed
	assert_gt(L.get_top(), 0, "Main state should have a result on the stack")

	var main_result: float = L.to_number(-1)
	assert_eq(main_result, 3, "Main function should have computed correct result")
	L.pop(1)

	L.single_step(false)
