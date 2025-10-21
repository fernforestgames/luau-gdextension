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

# Collect all source files
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
)

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

# Build type
string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER)
if(BUILD_TYPE_LOWER STREQUAL "")
    set(BUILD_TYPE_LOWER "debug")
endif()

# Set output properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "libgdluau.${PLATFORM_NAME}.${BUILD_TYPE_LOWER}.${ARCH_NAME}"
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
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${PROJECT_NAME}>
        "${CMAKE_CURRENT_SOURCE_DIR}/demo/addons/luau_gdextension/bin/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
    COMMENT "Copying library to demo addon directory"
)

# Enable compile_commands.json for better IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================================
# Test executable
# ============================================================================

# Collect test source files
file(GLOB TEST_SOURCES
    "tests/test_*.cpp"
    "tests/test_main.cpp"
)

# Create test executable
add_executable(gdluau_tests ${TEST_SOURCES} ${SOURCES})

# Include directories for tests (same as main library plus tests directory)
target_include_directories(gdluau_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

# Link libraries for tests (same as main library)
target_link_libraries(gdluau_tests PRIVATE
    godot-cpp
    Luau.Compiler
    Luau.VM
)

# Same compiler options as main library for consistency
if(MSVC)
    target_compile_options(gdluau_tests PRIVATE /EHs-c-)
    target_compile_definitions(gdluau_tests PRIVATE _HAS_EXCEPTIONS=0)
else()
    target_compile_options(gdluau_tests PRIVATE -fno-exceptions)
endif()

# Set output directory for test executable
set_target_properties(gdluau_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)

# Enable CTest integration
enable_testing()
add_test(NAME gdluau_tests COMMAND gdluau_tests)
