cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =============================================================================
# Options
# =============================================================================

# Option to enable debug output
option(DAKOTA_PARSER_DEBUG "Enable debug output in parser" OFF)

# =============================================================================
# Code Generation Setup
# =============================================================================

# Use Python from parent (already found)
if(NOT Python_FOUND AND NOT Python3_FOUND)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
endif()

# Use Python_EXECUTABLE if available (from parent), otherwise Python3_EXECUTABLE
if(DEFINED Python_EXECUTABLE)
    set(PYTHON_EXE ${Python_EXECUTABLE})
else()
    set(PYTHON_EXE ${Python3_EXECUTABLE})
endif()

# Schema path from parent project
if(NOT DEFINED DAKOTA_SCHEMA_PATH)
    set(DAKOTA_SCHEMA_PATH "${CMAKE_BINARY_DIR}/schema/dakota_study_schema.json")
endif()

# Generator script
set(GENERATOR_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tools/generate_parser_from_json.py")

# Generated files directory (in build dir)
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(GENERATED_TEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated_tests")

# Validate generator script exists
if(NOT EXISTS "${GENERATOR_SCRIPT}")
    message(FATAL_ERROR "Generator script not found at: ${GENERATOR_SCRIPT}")
endif()

# =============================================================================
# Build-Time Generation (runs during make, after schema is built)
# =============================================================================

# List all blocks
set(ALL_BLOCKS environment method model variables interface responses)

# Build list of generated source files
set(GENERATED_SOURCES)
foreach(block ${ALL_BLOCKS})
    list(APPEND GENERATED_SOURCES
        ${GENERATED_DIR}/src/${block}_parser.cpp
    )
endforeach()

# Add the default expander source (generated)
list(APPEND GENERATED_SOURCES
    ${GENERATED_DIR}/dakota_default_expander.cpp
)

# Single command to generate ALL blocks at once
# (If we generate each block separately with --block, each one creates stubs for the others)
# The generator also produces dakota_default_expander.cpp/.hpp and per-block defaults headers
add_custom_command(
    OUTPUT ${GENERATED_SOURCES}
    COMMAND ${PYTHON_EXE} ${GENERATOR_SCRIPT}
            ${DAKOTA_SCHEMA_PATH}
            --output-dir ${CMAKE_CURRENT_BINARY_DIR}
            --force
    DEPENDS ${DAKOTA_SCHEMA_PATH}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating parser and default expander for all blocks from JSON schema"
    VERBATIM
)

# =============================================================================
# Main Parser Executable
# =============================================================================

# Create object library for generated sources
# CMake will automatically trigger the custom command to generate these files
add_library(dakota_generated_sources OBJECT ${GENERATED_SOURCES})

# Set include directories for generated sources
target_include_directories(dakota_generated_sources PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include    # For dakota_ast.hpp, dakota_validator.hpp, etc.
    ${GENERATED_DIR}                       # For generated grammar/actions/error messages/default expander
)
foreach(block ${ALL_BLOCKS})
    target_include_directories(dakota_generated_sources PRIVATE
        ${GENERATED_DIR}/${block}          # For block-specific files including defaults
    )
endforeach()

# Link PEGTL to generated sources (PEGTL fetched by parent)
target_link_libraries(dakota_generated_sources PRIVATE taocpp::pegtl)

# Main parser executable
add_executable(dakota_parser 
    src/dakota_parser.cpp
)

# Link the generated sources object library
target_link_libraries(dakota_parser PRIVATE dakota_generated_sources)

# Also link PEGTL directly since dakota_parser.cpp includes PEGTL headers
target_link_libraries(dakota_parser PRIVATE taocpp::pegtl)

# =============================================================================
# Compiler Options
# =============================================================================

# Increase template depth for complex grammars
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(DAKOTA_COMPILE_OPTIONS
        -fbracket-depth=1024
        -Wall -Wextra -Wpedantic
    )
elseif(MSVC)
    set(DAKOTA_COMPILE_OPTIONS
        /constexpr:depth1024
        /W4
    )
endif()

# Apply compile options
target_compile_options(dakota_generated_sources PRIVATE ${DAKOTA_COMPILE_OPTIONS})
target_compile_options(dakota_parser PRIVATE ${DAKOTA_COMPILE_OPTIONS})

# Enable debug output if requested
if(DAKOTA_PARSER_DEBUG)
    target_compile_definitions(dakota_parser PRIVATE DAKOTA_PARSER_DEBUG=1)
    target_compile_definitions(dakota_generated_sources PRIVATE DAKOTA_PARSER_DEBUG=1)
    message(STATUS "Debug output enabled for parser")
endif()

# Set include directories for main parser
target_include_directories(dakota_parser PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${GENERATED_DIR}
)
foreach(block ${ALL_BLOCKS})
    target_include_directories(dakota_parser PRIVATE
        ${GENERATED_DIR}/${block}
    )
endforeach()

# Link nlohmann_json (fetched by parent project, needed for JSON output)
target_link_libraries(dakota_parser PRIVATE nlohmann_json::nlohmann_json dakota_validation)
target_link_libraries(dakota_generated_sources PRIVATE nlohmann_json::nlohmann_json dakota_validation)

# =============================================================================
# Parser Library (for embedding)
# =============================================================================

add_library(dakota_parser_lib STATIC 
    src/dakota_parser.cpp
)

target_link_libraries(dakota_parser_lib PUBLIC 
    dakota_generated_sources 
    taocpp::pegtl 
    nlohmann_json::nlohmann_json
    dakota_validation
)

target_include_directories(dakota_parser_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${GENERATED_DIR}
)
foreach(block ${ALL_BLOCKS})
    target_include_directories(dakota_parser_lib PUBLIC
        ${GENERATED_DIR}/${block}
    )
endforeach()

target_compile_options(dakota_parser_lib PRIVATE ${DAKOTA_COMPILE_OPTIONS})

# Enable debug output for library if requested
if(DAKOTA_PARSER_DEBUG)
    target_compile_definitions(dakota_parser_lib PRIVATE DAKOTA_PARSER_DEBUG=1)
endif()

# =============================================================================
# Testing Support
# =============================================================================

if(BUILD_TESTS)
    enable_testing()
    
    # Find Dakota input test files
    set(DAKOTA_TEST_INPUT_FILES)
    
    file(GLOB SOURCE_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.in")
    list(APPEND DAKOTA_TEST_INPUT_FILES ${SOURCE_TEST_FILES})
    
    # Look for manually created error tests in source tree
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_errors")
        file(GLOB TEST_ERROR_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test_errors/*.in")
        list(APPEND DAKOTA_TEST_INPUT_FILES ${TEST_ERROR_FILES})
    endif()
    
    # Look for generated tests in build tree (after first build)
    if(EXISTS "${GENERATED_TEST_DIR}")
        file(GLOB_RECURSE GENERATED_TEST_FILES "${GENERATED_TEST_DIR}/*.in")
        list(APPEND DAKOTA_TEST_INPUT_FILES ${GENERATED_TEST_FILES})
    endif()
    
    # Add tests for each input file
    foreach(test_file ${DAKOTA_TEST_INPUT_FILES})
        # Create unique test name using full relative path from appropriate root
        if(test_file MATCHES "^${CMAKE_CURRENT_SOURCE_DIR}")
            file(RELATIVE_PATH test_rel_path "${CMAKE_CURRENT_SOURCE_DIR}" "${test_file}")
        else()
            file(RELATIVE_PATH test_rel_path "${CMAKE_CURRENT_BINARY_DIR}" "${test_file}")
        endif()
        
        string(REPLACE "/" "_" test_name "${test_rel_path}")
        string(REPLACE ".in" "" test_name "${test_name}")
        
        # Determine if this is an error test (should fail)
        get_filename_component(test_basename "${test_file}" NAME)
        
        if(test_basename MATCHES "^error_" OR test_file MATCHES "/negative/")
            add_test(NAME ${test_name}
                     COMMAND dakota_parser "${test_file}"
                     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
            set_tests_properties(${test_name} PROPERTIES WILL_FAIL TRUE)
        else()
            add_test(NAME ${test_name}
                     COMMAND dakota_parser "${test_file}"
                     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        endif()
    endforeach()
    
    list(LENGTH DAKOTA_TEST_INPUT_FILES num_tests)
    if(num_tests GREATER 0)
        message(STATUS "Found ${num_tests} Dakota input test files")
    endif()
endif()

# =============================================================================
# Custom Targets (for development)
# =============================================================================

# Target to force regeneration
add_custom_target(regenerate_parser
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${GENERATED_DIR}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${GENERATED_TEST_DIR}
    COMMAND ${PYTHON_EXE} ${GENERATOR_SCRIPT} 
            ${DAKOTA_SCHEMA_PATH}
            --output-dir ${CMAKE_CURRENT_BINARY_DIR}
            --generate-tests
            --force
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Force regenerating parser and tests..."
    VERBATIM
)
# Note: For manual regeneration, ensure schema has been built first


# =============================================================================
# Installation
# =============================================================================

install(TARGETS dakota_parser
    RUNTIME DESTINATION bin
    COMPONENT parser
)

install(TARGETS dakota_parser_lib
    ARCHIVE DESTINATION lib
    COMPONENT parser
)

install(DIRECTORY include/
    DESTINATION include/dakota/parser
    COMPONENT parser
    FILES_MATCHING PATTERN "*.hpp"
)

# =============================================================================
# Status
# =============================================================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "Dakota Parser Configuration:")
message(STATUS "  Schema: ${DAKOTA_SCHEMA_PATH}")
message(STATUS "  Generated code: ${GENERATED_DIR}")
message(STATUS "  Blocks: ${ALL_BLOCKS}")
message(STATUS "  Generation: At build time (after schema)")
message(STATUS "  Debug output: ${DAKOTA_PARSER_DEBUG}")
message(STATUS "========================================")
message(STATUS "")
