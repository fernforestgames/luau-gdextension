cmake_minimum_required(VERSION 3.28)

# Allow set() to override later option() calls in subprojects
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

# Set MSVC runtime library policy (required for CMAKE_MSVC_RUNTIME_LIBRARY)
cmake_policy(SET CMP0091 NEW)

project(gdluau)
include(FetchContent)

# Configure ccache if available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "ccache found and enabled: ${CCACHE_PROGRAM}")
endif()

# Configure Luau options
set(LUAU_BUILD_CLI OFF)
set(LUAU_BUILD_TESTS OFF)
set(LUAU_EXTERN_C ON)
set(LUAU_STATIC_CRT OFF)  # Use dynamic runtime (/MD or /MDd) to match godot-cpp

# Fetch dependencies
FetchContent_Declare(
    Luau
    GIT_REPOSITORY https://github.com/luau-lang/luau.git
    GIT_TAG 696
)
FetchContent_Declare(
    GodotCpp
    GIT_REPOSITORY https://github.com/godotengine/godot-cpp.git
    GIT_TAG godot-4.5-stable
)
FetchContent_MakeAvailable(Luau GodotCpp)

# Collect source files (exclude tests_runtime.cpp initially)
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
)
list(FILTER SOURCES EXCLUDE REGEX ".*tests_runtime\\.cpp$")

# GDExtension library target
add_library(${PROJECT_NAME} MODULE ${SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE
    # Disable C++ exceptions in our code (Godot doesn't use them, Luau uses setjmp/longjmp via LUAU_EXTERN_C)
    cxx_noexcept

    cxx_std_17
)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    godot-cpp
    Luau.Compiler
    Luau.VM
)

string(TOLOWER ${CMAKE_SYSTEM_NAME} PLATFORM_NAME)
string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} ARCH_NAME)
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "gdluau.${PLATFORM_NAME}.${ARCH_NAME}"
)

# Custom target to copy the library to the demo addon directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/demo/addons/luau_gdextension/bin"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${PROJECT_NAME}>
        "${CMAKE_CURRENT_SOURCE_DIR}/demo/addons/luau_gdextension/bin/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
    COMMENT "Copying library to demo addon directory"
)

# ============================================================================
# Runtime-Embedded Tests (Debug builds only)
# ============================================================================

# Add runtime test sources
target_sources(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:src/tests_runtime.cpp>
)

# Define ENABLE_LUAU_GDEXTENSION_TESTS for conditional compilation
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:ENABLE_LUAU_GDEXTENSION_TESTS>)

# Include tests directory for doctest.h
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/tests>
)
