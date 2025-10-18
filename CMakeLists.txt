cmake_minimum_required(VERSION 3.28)

# Allow set() to override later option() calls in subprojects
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

project(gdluau)
include(FetchContent)

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

set(LUAU_BUILD_CLI OFF)
set(LUAU_BUILD_TESTS OFF)
set(LUAU_EXTERN_C ON)

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

# GDExtension library target
add_library(${PROJECT_NAME} SHARED
    src/lua_state.cpp
    src/luau.cpp
    src/lua_math_types.cpp
    src/register_types.cpp
)

# Statically linked dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE
    godot-cpp Luau.Compiler Luau.VM
)

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

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "gdluau.${PLATFORM_NAME}.${BUILD_TYPE_LOWER}.${ARCH_NAME}"
    PREFIX "lib"
    SUFFIX "${PLATFORM_EXT}"
    # Prevent MSVC/MSBuild from adding Debug/Release subdirectories
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}"
)
