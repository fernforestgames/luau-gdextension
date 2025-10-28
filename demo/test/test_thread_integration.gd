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
	L.openlibs() # Load all libraries including coroutine

func after_each() -> void:
	if L:
		L.close()
		L = null


# Helper to verify Lua stack is balanced
func assert_stack_balanced(state: LuaState, expected_top: int = 0) -> void:
	assert_eq(state.gettop(), expected_top, "Lua stack should be balanced at %d, but is %d" % [expected_top, state.gettop()])


# ============================================================================
# Thread Creation and Basic Operations
# ============================================================================

func test_thread_creation() -> void:
	L.newthread()
	assert_true(L.isthread(-1), "newthread() should push a thread to the stack")
	assert_eq(L.gettop(), 1, "Stack should have one element after newthread()")

func test_newthread_conversion_to_luastate() -> void:
	var thread: LuaState = L.newthread()
	L.pop(1)

	assert_not_null(thread, "tothread() should return a LuaState")
	assert_true(thread is LuaState, "Returned value should be LuaState instance")

	assert_stack_balanced(L)

func test_tothread_conversion_to_luastate() -> void:
	L.newthread()
	var thread: LuaState = L.tothread(-1)
	L.pop(1)

	assert_not_null(thread, "tothread() should return a LuaState")
	assert_true(thread is LuaState, "Returned value should be LuaState instance")

	assert_stack_balanced(L)

func test_thread_has_separate_stack() -> void:
	# Push value to parent
	L.pushnumber(123)
	var parent_top: int = L.gettop()

	# Create thread
	var thread: LuaState = L.newthread()
	L.pop(1) # Remove thread from parent stack

	# Thread should have empty stack
	assert_eq(thread.gettop(), 0, "Thread should start with empty stack")

	# Push to thread
	thread.pushnumber(456)
	assert_eq(thread.gettop(), 1, "Thread stack should have one element")

	# Parent stack should be unaffected
	assert_eq(L.gettop(), parent_top, "Parent stack should be unaffected by thread operations")
	assert_eq(L.tonumber(-1), 123, "Parent stack value should be unchanged")


func test_thread_shares_globals() -> void:
	# Set global in parent
	var code: String = "shared_value = 42"
	assert_eq(L.dostring(code, "test"), 0, "Setting global should succeed")

	# Create thread
	var thread: LuaState = L.newthread()
	L.pop(1)

	# Access global from thread
	thread.getglobal("shared_value")
	assert_eq(thread.tonumber(-1), 42, "Thread should see parent globals")
	thread.pop(1)

	# Modify global from thread
	thread.pushnumber(100)
	thread.setglobal("shared_value")
	assert_engine_error("Calling LuaState.setglobal() on a Lua thread will affect all threads in the same VM.")

	# Verify change in parent
	L.getglobal("shared_value")
	assert_eq(L.tonumber(-1), 100, "Parent should see thread's global modifications")


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
	assert_eq(L.dostring(code, "test"), 0, "Loading coroutine function should succeed")

	# Create thread
	var thread: LuaState = L.newthread()
	L.pop(1)

	# Load function into thread
	thread.getglobal("simple_coro")
	assert_true(thread.isfunction(-1), "Function should be accessible from thread")

	# Resume and check yields
	assert_eq(thread.resume(0), 1, "First resume should yield") # LUA_YIELD = 1
	assert_eq(thread.tonumber(-1), 1, "First yield should return 1")
	thread.pop(1)

	assert_eq(thread.resume(0), 1, "Second resume should yield")
	assert_eq(thread.tonumber(-1), 2, "Second yield should return 2")
	thread.pop(1)

	assert_eq(thread.resume(0), 0, "Final resume should complete") # LUA_OK = 0
	assert_eq(thread.tonumber(-1), 3, "Final return should be 3")


func test_coroutine_with_parameters() -> void:
	var code: String = """
	function param_coro(a, b)
		local sum = a + b
		coroutine.yield(sum)
		return sum * 2
	end
	"""
	assert_eq(L.dostring(code, "test"), 0, "Loading coroutine should succeed")

	var thread: LuaState = L.newthread()
	L.pop(1)

	thread.getglobal("param_coro")
	thread.pushnumber(10)
	thread.pushnumber(20)

	# Resume with 2 arguments
	assert_eq(thread.resume(2), 1, "Should yield")
	assert_eq(thread.tonumber(-1), 30, "Should yield sum")
	thread.pop(1)

	assert_eq(thread.resume(0), 0, "Should complete")
	assert_eq(thread.tonumber(-1), 60, "Should return doubled value")


func test_coroutine_counter() -> void:
	var code: String = """
	function counter(max)
		for i = 1, max do
			coroutine.yield(i)
		end
		return "done"
	end
	"""
	assert_eq(L.dostring(code, "test"), 0, "Loading counter should succeed")

	var thread: LuaState = L.newthread()
	L.pop(1)

	thread.getglobal("counter")
	thread.pushnumber(3)

	# Count from 1 to 3
	for i in range(1, 4):
		assert_eq(thread.resume(1 if i == 1 else 0), 1, "Should yield at iteration %d" % i)
		assert_eq(thread.tonumber(-1), i, "Should yield %d" % i)
		thread.pop(1)

	assert_eq(thread.resume(0), 0, "Should complete after counting")
	assert_eq(thread.tostring(-1), "done", "Should return completion message")


# ============================================================================
# Thread Lifecycle and Reference Counting
# ============================================================================

func test_thread_survives_parent_scope() -> void:
	var thread: LuaState

	# Create thread in separate block (simulating scope)
	thread = L.newthread()
	L.pop(1)

	# Thread should still be valid
	assert_not_null(thread, "Thread should survive parent scope")
	thread.pushnumber(999)
	assert_eq(thread.tonumber(-1), 999, "Thread should still be functional")


func test_multiple_threads_same_parent() -> void:
	var thread1: LuaState = L.newthread()
	L.pop(1)

	var thread2: LuaState = L.newthread()
	L.pop(1)

	# Both threads should be independent
	thread1.pushnumber(111)
	thread2.pushnumber(222)

	assert_eq(thread1.gettop(), 1, "Thread1 should have one value")
	assert_eq(thread2.gettop(), 1, "Thread2 should have one value")
	assert_eq(thread1.tonumber(-1), 111, "Thread1 should have its own value")
	assert_eq(thread2.tonumber(-1), 222, "Thread2 should have its own value")


func test_close_thread_warns() -> void:
	var thread: LuaState = L.newthread()
	L.pop(1)

	# Closing thread should warn but not crash
	thread.close()
	assert_engine_error("LuaState.close() should not be called on Lua threads.")

	# Parent should still be functional
	L.pushnumber(456)
	assert_eq(L.tonumber(-1), 456, "Parent should remain functional after thread close")


func test_close_parent_invalidates_threads() -> void:
	var thread: LuaState = L.newthread()
	L.pop(1)

	thread.pushnumber(123)
	assert_eq(thread.tonumber(-1), 123, "Thread should work before parent closes")
	thread.pop(1)

	# Close parent
	L.close()

	# Thread operations should fail gracefully (print errors)
	# This test verifies no crash occurs
	thread.pushnumber(456) # Should fail with error message
	assert_engine_error("Lua state is invalid. Cannot push number.")


# ============================================================================
# Thread Bridging via Variants
# ============================================================================

func test_thread_as_variant() -> void:
	var thread: LuaState = L.newthread()
	L.pop(1)

	# Push thread as variant
	var thread_variant: Variant = thread
	thread.pushvariant(thread_variant)

	assert_true(thread.isthread(-1), "Pushed variant should be a thread on stack")

	# Convert back
	var thread2: LuaState = thread.tothread(-1)
	thread.pop(1)

	assert_not_null(thread2, "Should be able to convert thread back from stack")
	assert_stack_balanced(L)
	assert_stack_balanced(thread)


func test_thread_in_table() -> void:
	var thread: LuaState = L.newthread()
	L.pop(1)

	# Store thread in a table
	thread.pushdictionary({"thread_ref": thread, "name": "my_thread"})

	# Retrieve dictionary
	var table: Dictionary = thread.todictionary(-1)
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
	L.pushnumber(123)
	assert_false(L.isthread(-1), "Number should not be a thread")

	L.pushstring("test")
	assert_false(L.isthread(-1), "String should not be a thread")

	L.newtable()
	assert_false(L.isthread(-1), "Table should not be a thread")

	L.pushnil()
	assert_false(L.isthread(-1), "Nil should not be a thread")


func test_thread_error_handling() -> void:
	var code: String = """
	function error_func()
		error("Intentional error for testing")
	end
	"""
	assert_eq(L.dostring(code, "test"), 0, "Loading error function should succeed")

	var thread: LuaState = L.newthread()
	L.pop(1)

	thread.getglobal("error_func")

	# pcall should catch the error
	var status: int = thread.pcall(0, 0, 0)
	assert_ne(status, 0, "pcall should return error status")

	# Error message should be on stack
	var error_msg: String = thread.tostring(-1)
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
	assert_eq(L.dostring(code, "test"), 0)

	var producer_thread: LuaState = L.newthread()
	L.pop(1)

	producer_thread.getglobal("producer")

	# Consumer: collect all values
	# First resume: function is on stack, narg=0
	# Subsequent resumes: no args on stack, narg=0
	var collected: Array = []
	for i in range(5):
		var status: int = producer_thread.resume(0)
		assert_eq(status, 1, "Producer should yield value %d" % i)
		collected.append(int(producer_thread.tonumber(-1)))
		producer_thread.pop(1)

	assert_eq(collected, [10, 20, 30, 40, 50], "Should collect all produced values")

	# Producer should be done
	var status: int = producer_thread.resume(0)
	assert_eq(status, 0, "Producer should complete after all values")
