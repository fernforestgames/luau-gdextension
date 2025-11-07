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

# Import GodotCPP module for documentation generation
list(APPEND CMAKE_MODULE_PATH "${godotcpp_SOURCE_DIR}/cmake")
include(GodotCPPModule)

# Collect source files
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Create object library for shared code (used by main and test libraries)
add_library(${PROJECT_NAME}_core OBJECT ${SOURCES})
target_compile_features(${PROJECT_NAME}_core PRIVATE
    # Disable C++ exceptions in our code (Godot doesn't use them, Luau uses setjmp/longjmp via LUAU_EXTERN_C)
    cxx_noexcept
    cxx_std_20
)
target_include_directories(${PROJECT_NAME}_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(${PROJECT_NAME}_core PRIVATE
    godot-cpp
    Luau.Compiler
    Luau.VM
)

# Apply custom warnings (only to our source files)
set_source_files_properties(
    ${SOURCES}
    PROPERTIES COMPILE_OPTIONS "${WARNING_FLAGS}"
)

# GDExtension library target
add_library(${PROJECT_NAME} MODULE
    $<TARGET_OBJECTS:${PROJECT_NAME}_core>
)
target_compile_features(${PROJECT_NAME} PRIVATE
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

# Generate documentation source and add to target
file(GLOB_RECURSE DOC_XML "${CMAKE_CURRENT_SOURCE_DIR}/doc_classes/*.xml")
set(DOC_SOURCE_FILE "${CMAKE_CURRENT_BINARY_DIR}/gen/doc_source.cpp")
generate_doc_source("${DOC_SOURCE_FILE}" "${DOC_XML}")
target_sources(${PROJECT_NAME} PRIVATE "${DOC_SOURCE_FILE}")
add_dependencies(${PROJECT_NAME} generate_doc_source)

string(TOLOWER ${CMAKE_SYSTEM_NAME} PLATFORM_NAME)
string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} ARCH_NAME)
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "gdluau.${PLATFORM_NAME}.${ARCH_NAME}"
    SUFFIX ".${LIBRARY_SUFFIX}"
)

# Copy the library to dist/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/dist/addons/luau_gdextension/bin"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${PROJECT_NAME}>
        "${CMAKE_CURRENT_SOURCE_DIR}/dist/addons/luau_gdextension/bin/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
    COMMENT "Copying extension library to dist/addons/"
)

# Copy PDB files if COPY_PDB_FILE is enabled (Windows debug builds)
if(COPY_PDB_FILE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_PDB_FILE:${PROJECT_NAME}>
            "${CMAKE_CURRENT_SOURCE_DIR}/dist/addons/luau_gdextension/bin/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
        COMMENT "Copying PDB file to dist/addons/"
    )
endif()

# ============================================================================
# Test Library (separate shared library, only when BUILD_TESTING=ON)
# ============================================================================

# Enable CTest support
include(CTest)

if(BUILD_TESTING)
    message(STATUS "Test library enabled: gdluau_tests will be built")

    # Collect test source files
    file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")

    # Test library target (uses core object library + test files)
    add_library(${PROJECT_NAME}_tests MODULE
        $<TARGET_OBJECTS:${PROJECT_NAME}_core>
        ${TEST_SOURCES}
    )
    target_compile_features(${PROJECT_NAME}_tests PRIVATE
        cxx_noexcept
        cxx_std_17
    )
    target_include_directories(${PROJECT_NAME}_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )
    target_link_libraries(${PROJECT_NAME}_tests PRIVATE
        godot-cpp
        Luau.Compiler
        Luau.VM
    )

    # Apply same warning settings to test library source files
    set_source_files_properties(${TEST_SOURCES}
        PROPERTIES COMPILE_OPTIONS "${WARNING_FLAGS}"
    )

    set_target_properties(${PROJECT_NAME}_tests PROPERTIES
        DEBUG_POSTFIX ""
        OUTPUT_NAME "gdluau_tests.${PLATFORM_NAME}.${ARCH_NAME}"
        SUFFIX ".${LIBRARY_SUFFIX}"
    )

    # Reuse the same doc source file for tests
    target_sources(${PROJECT_NAME}_tests PRIVATE "${DOC_SOURCE_FILE}")
    add_dependencies(${PROJECT_NAME}_tests generate_doc_source)

    # Copy test library to demo addon directory
    add_custom_command(TARGET ${PROJECT_NAME}_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${PROJECT_NAME}_tests>
            "${CMAKE_CURRENT_SOURCE_DIR}/demo/addons/luau_gdextension_tests/bin/$<TARGET_FILE_NAME:${PROJECT_NAME}_tests>"
        COMMENT "Copying test library to demo addons"
    )

    # Copy PDB files if COPY_PDB_FILE is enabled (Windows debug builds)
    if(COPY_PDB_FILE)
        add_custom_command(TARGET ${PROJECT_NAME}_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_PDB_FILE:${PROJECT_NAME}_tests>
                "${CMAKE_CURRENT_SOURCE_DIR}/demo/addons/luau_gdextension_tests/bin/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}_tests>"
            COMMENT "Copying test PDB file to demo addons"
        )
    endif()

    # Add CTest for running Godot tests
    # Note: Godot executable must be in PATH or specified via GODOT_EXECUTABLE env var
    find_program(GODOT_EXECUTABLE
        NAMES godot godot.exe Godot Godot.exe Godot.app Godot_v4.5.1-stable_win64.exe Godot_v4.5.1-stable_linux.x86_64
        HINTS
            ENV GODOT_EXECUTABLE
            ENV RUNNER_TEMP
            "C:/Program Files/Godot"
            "/Applications"
            "$ENV{HOME}/Applications"
            "/usr/local/bin"
            "/usr/bin"
        DOC "Path to Godot executable"
    )

    if(GODOT_EXECUTABLE)
        message(STATUS "Found Godot executable: ${GODOT_EXECUTABLE}")

        add_test(
            NAME GodotImport
            COMMAND "${GODOT_EXECUTABLE}" --headless --import
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/demo"
        )

        set_tests_properties(GodotImport PROPERTIES FIXTURES_SETUP GODOT)

        add_test(
            NAME GodotTests
            COMMAND "${GODOT_EXECUTABLE}" --headless -- --run-tests
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/demo"
        )

        set_tests_properties(GodotTests PROPERTIES
            TIMEOUT 300  # 5 minute timeout
            LABELS "integration"
            FIXTURES_REQUIRED GODOT
        )
    else()
        message(WARNING "Godot executable not found. Tests can still be run manually.")
        message(STATUS "Set GODOT_EXECUTABLE to Godot's path to enable CTest integration.")
    endif()
endif()
