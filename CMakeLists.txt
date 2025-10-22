cmake_minimum_required(VERSION 3.28)

# Allow set() to override later option() calls in subprojects
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

# Set MSVC runtime library policy (required for CMAKE_MSVC_RUNTIME_LIBRARY)
cmake_policy(SET CMP0091 NEW)

project(gdluau)
include(FetchContent)

if(MSVC)
    # Force /MD (dynamic runtime) on MSVC for all targets
    # This ensures gdluau, godot-cpp, and Luau all use the same runtime library
    # /MD = MultiThreadedDLL (Release), /MDd = MultiThreadedDebugDLL (Debug)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable position-independent code for Unix-like systems (required for shared libraries on Linux/macOS)
if(UNIX)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

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
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    godot-cpp
    Luau.Compiler
    Luau.VM
)

# Disable C++ exceptions in our code (Godot doesn't use them, Luau uses setjmp/longjmp via LUAU_EXTERN_C)
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /EHs-c-)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _HAS_EXCEPTIONS=0)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -fno-exceptions)
endif()

# Platform detection
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PLATFORM_NAME "linux")
    set(PLATFORM_EXT ".so")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PLATFORM_NAME "windows")
    set(PLATFORM_EXT ".dll")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(PLATFORM_NAME "macos")
    set(PLATFORM_EXT ".dylib")
endif()

# Architecture detection
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(ARCH_NAME "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(ARCH_NAME "arm64")
endif()

# Set output properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    DEBUG_POSTFIX ".debug"
    OUTPUT_NAME "libgdluau.${PLATFORM_NAME}.${ARCH_NAME}"
    PREFIX ""
    SUFFIX "${PLATFORM_EXT}"
    # Prevent MSVC/MSBuild from adding Debug/Release subdirectories
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/${PLATFORM_NAME}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/${PLATFORM_NAME}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin/${PLATFORM_NAME}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin/${PLATFORM_NAME}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin/${PLATFORM_NAME}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin/${PLATFORM_NAME}"
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

# Enable compile_commands.json for better IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
