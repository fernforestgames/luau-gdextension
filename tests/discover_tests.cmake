# CMake script to discover and register individual test cases with CTest
# This script is executed at build time (after test library is built)
#
# Required variables:
#   GODOT_EXECUTABLE - Path to Godot executable
#   DEMO_DIR - Path to demo directory
#   OUTPUT_FILE - Path to generated CTest file

cmake_minimum_required(VERSION 3.28)

message(STATUS "Discovering tests...")

# First, ensure Godot project is imported
execute_process(
    COMMAND "${GODOT_EXECUTABLE}" --headless --import --quit
    WORKING_DIRECTORY "${DEMO_DIR}"
    OUTPUT_VARIABLE IMPORT_OUTPUT
    ERROR_VARIABLE IMPORT_ERROR
    RESULT_VARIABLE IMPORT_RESULT
    TIMEOUT 60
)

# Godot import can return non-zero but still succeed, so we just log warnings
if(NOT IMPORT_RESULT EQUAL 0)
    message(STATUS "Godot import exited with code ${IMPORT_RESULT}")
    if(IMPORT_ERROR)
        message(STATUS "Import stderr: ${IMPORT_ERROR}")
    endif()
endif()

# Run list_tests.gd to discover all tests
execute_process(
    COMMAND "${GODOT_EXECUTABLE}" --headless -s list_tests.gd
    WORKING_DIRECTORY "${DEMO_DIR}"
    OUTPUT_VARIABLE TEST_LIST_OUTPUT
    ERROR_VARIABLE TEST_LIST_ERROR
    RESULT_VARIABLE TEST_LIST_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT TEST_LIST_RESULT EQUAL 0)
    message(WARNING "Test discovery failed: ${TEST_LIST_ERROR}")
    message(STATUS "Tests will not be registered individually")
    # Create empty output file
    file(WRITE "${OUTPUT_FILE}" "# Test discovery failed\n")
    return()
endif()

# Parse test list (one test per line)
string(REPLACE "\n" ";" TEST_CASES "${TEST_LIST_OUTPUT}")

# Generate CTest file
file(WRITE "${OUTPUT_FILE}" "# Auto-generated test list - do not edit\n")
file(APPEND "${OUTPUT_FILE}" "# Generated at build time by discover_tests.cmake\n\n")

set(CPP_COUNT 0)
set(GUT_COUNT 0)

foreach(TEST_CASE ${TEST_CASES})
    # Skip empty lines
    if(NOT TEST_CASE)
        continue()
    endif()

    # Parse test type and name
    if(TEST_CASE MATCHES "^cpp:(.+)$")
        set(TEST_NAME "${CMAKE_MATCH_1}")
        set(TEST_TYPE "cpp")
        math(EXPR CPP_COUNT "${CPP_COUNT} + 1")
    elseif(TEST_CASE MATCHES "^gut:(.+)$")
        set(TEST_NAME "${CMAKE_MATCH_1}")
        set(TEST_TYPE "gut")
        math(EXPR GUT_COUNT "${GUT_COUNT} + 1")
    else()
        message(WARNING "Unknown test format: ${TEST_CASE}")
        continue()
    endif()

    # Sanitize test name for CTest (replace special chars with underscores)
    string(REGEX REPLACE "[^a-zA-Z0-9_:]+" "_" SAFE_TEST_NAME "${TEST_NAME}")

    # Write add_test() call to output file
    file(APPEND "${OUTPUT_FILE}" "add_test(\n")
    file(APPEND "${OUTPUT_FILE}" "    NAME \"${TEST_TYPE}.${SAFE_TEST_NAME}\"\n")
    file(APPEND "${OUTPUT_FILE}" "    COMMAND \"${GODOT_EXECUTABLE}\" --headless -s run_single_test.gd -- \"${TEST_CASE}\"\n")
    file(APPEND "${OUTPUT_FILE}" "    WORKING_DIRECTORY \"${DEMO_DIR}\"\n")
    file(APPEND "${OUTPUT_FILE}" ")\n")
    file(APPEND "${OUTPUT_FILE}" "set_tests_properties(\"${TEST_TYPE}.${SAFE_TEST_NAME}\" PROPERTIES\n")
    file(APPEND "${OUTPUT_FILE}" "    TIMEOUT 60\n")
    file(APPEND "${OUTPUT_FILE}" "    LABELS \"${TEST_TYPE}\"\n")
    file(APPEND "${OUTPUT_FILE}" "    FIXTURES_REQUIRED GODOT\n")
    file(APPEND "${OUTPUT_FILE}" ")\n\n")
endforeach()

message(STATUS "Discovered ${CPP_COUNT} C++ tests and ${GUT_COUNT} GDScript tests")
