cmake_minimum_required(VERSION 3.15)
project(dakota_json_validation VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =============================================================================
# Options
# =============================================================================
option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_EXAMPLES "Build example programs" OFF)

# Detect scikit-build-core
if(DEFINED SKBUILD)
    message(STATUS "Building with scikit-build-core")
    set(SKBUILD_BUILD TRUE)
    # scikit-build-core sets CMAKE_INSTALL_PREFIX to a staging directory
    # Python modules should install there; C++ artifacts are excluded from wheel
else()
    set(SKBUILD_BUILD FALSE)
endif()

# =============================================================================
# Find Python FIRST (before pybind11) to ensure consistent paths
# =============================================================================
if(BUILD_PYTHON_BINDINGS)
    find_package(Python REQUIRED COMPONENTS Interpreter Development)
    message(STATUS "Python executable: ${Python_EXECUTABLE}")
    message(STATUS "Python include dirs: ${Python_INCLUDE_DIRS}")
    
    # Validate Python include path exists
    if(Python_INCLUDE_DIRS)
        list(GET Python_INCLUDE_DIRS 0 _first_include_dir)
        if(NOT EXISTS "${_first_include_dir}")
            message(WARNING "Python include directory does not exist: ${_first_include_dir}")
            execute_process(
                COMMAND "${Python_EXECUTABLE}" -c 
                    "import sysconfig; print(sysconfig.get_path('include'))"
                OUTPUT_VARIABLE _python_include_from_sysconfig
                OUTPUT_STRIP_TRAILING_WHITESPACE
                RESULT_VARIABLE _sysconfig_result
            )
            if(_sysconfig_result EQUAL 0 AND EXISTS "${_python_include_from_sysconfig}")
                message(STATUS "Found valid Python include path: ${_python_include_from_sysconfig}")
                set(Python_INCLUDE_DIRS "${_python_include_from_sysconfig}" CACHE PATH "" FORCE)
            else()
                message(FATAL_ERROR "Cannot find valid Python include directory.")
            endif()
        endif()
    endif()
endif()

# =============================================================================
# FetchContent for dependencies
# =============================================================================
include(FetchContent)

# nlohmann_json
find_path(NLOHMANN_JSON_INCLUDE_DIR
  NAMES nlohmann/json.hpp
  HINTS ${NLOHMANN_JSON_ROOT}
  PATH_SUFFIXES include
)

if(NLOHMANN_JSON_INCLUDE_DIR)
  add_library(nlohmann_json INTERFACE)
  add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
  target_include_directories(nlohmann_json INTERFACE "${NLOHMANN_JSON_INCLUDE_DIR}")
  message(STATUS "Found valid nlohmann_json include path: ${NLOHMANN_JSON_INCLUDE_DIR}")
else()
  find_package(nlohmann_json 3.2.0 QUIET)
  if(NOT nlohmann_json_FOUND)
    message(STATUS "Fetching nlohmann_json from GitHub...")
    FetchContent_Declare(
      nlohmann_json
      GIT_REPOSITORY https://github.com/nlohmann/json.git
      GIT_TAG        v3.11.3
      GIT_SHALLOW    TRUE
    )
    set(JSON_Install OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(nlohmann_json)
  endif()
endif()

# pybind11 (for Python bindings)
if(BUILD_PYTHON_BINDINGS)
    message(STATUS "Fetching pybind11 from GitHub...")
    set(pybind11_DIR "" CACHE PATH "" FORCE)
    set(PYBIND11_FINDPYTHON OFF CACHE BOOL "" FORCE)
    set(PYTHON_EXECUTABLE "${Python_EXECUTABLE}" CACHE FILEPATH "" FORCE)
    list(GET Python_INCLUDE_DIRS 0 _py_include_dir)
    set(PYTHON_INCLUDE_DIR "${_py_include_dir}" CACHE PATH "" FORCE)
    
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG        v2.12.1
        GIT_SHALLOW    TRUE
        OVERRIDE_FIND_PACKAGE

        # If set to an existing directory, FetchContent uses it and skips download
        SOURCE_DIR     "${PYBIND11_ROOT_DIR}"
    )
    FetchContent_MakeAvailable(pybind11)
    
    # pybind11_json - header-only
    message(STATUS "Fetching pybind11_json from GitHub...")
    FetchContent_Declare(
        pybind11_json
        GIT_REPOSITORY https://github.com/pybind/pybind11_json.git
        GIT_TAG        0.2.15
        GIT_SHALLOW    TRUE
        SOURCE_SUBDIR  nonexistent

        # If set to an existing directory, FetchContent uses it and skips download
        SOURCE_DIR     "${PYBIND11_JSON_ROOT_DIR}"
    )
    FetchContent_MakeAvailable(pybind11_json)
    
    add_library(pybind11_json_iface INTERFACE)
    target_include_directories(pybind11_json_iface INTERFACE 
        ${pybind11_json_SOURCE_DIR}/include
    )
    target_link_libraries(pybind11_json_iface INTERFACE 
        pybind11::pybind11
        nlohmann_json::nlohmann_json
    )
endif()

# GoogleTest (for tests)
if(BUILD_TESTS AND NOT (TARGET gtest))
    message(STATUS "Fetching GTest from GitHub...")
    set(GTest_DIR "" CACHE PATH "" FORCE)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.15.2
        GIT_SHALLOW    TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    # Save user's BUILD_SHARED_LIBS setting, force OFF for gtest, then restore
    set(_SAVED_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
    set(BUILD_SHARED_LIBS OFF)
    FetchContent_MakeAvailable(googletest)
    set(BUILD_SHARED_LIBS ${_SAVED_BUILD_SHARED_LIBS})
    
    enable_testing()
endif()

# PEGTL - Parsing Expression Grammar Template Library (for Dakota parser)
message(STATUS "Fetching PEGTL from GitHub...")
FetchContent_Declare(
    pegtl
    GIT_REPOSITORY https://github.com/taocpp/PEGTL.git
    GIT_TAG        3.2.7
    GIT_SHALLOW    TRUE
    # If set to an existing directory, FetchContent uses it and skips download
    SOURCE_DIR     "${PEGTL_ROOT_DIR}"
)
FetchContent_MakeAvailable(pegtl)

# =============================================================================
# Python install directory
# =============================================================================
if(BUILD_PYTHON_BINDINGS)
    if(SKBUILD_BUILD)
        # scikit-build-core: install to package location within staging dir
        # The wheel.install-dir in pyproject.toml handles the final location
        set(PYTHON_INSTALL_DIR "dakota/spec/validation")
    elseif(NOT DEFINED PYTHON_INSTALL_DIR)
        # Direct CMake build: install alongside other libs (user must set PYTHONPATH)
        set(PYTHON_INSTALL_DIR "lib/python/dakota/spec/validation")
    endif()
    message(STATUS "Python modules will install to: \${CMAKE_INSTALL_PREFIX}/${PYTHON_INSTALL_DIR}")
endif()

# =============================================================================
# Subdirectories
# =============================================================================

# dakota_validation library (custom validators and computed fields)
add_subdirectory(dakota_validation)

# =============================================================================
# Dakota Parser (requires JSON schema to be generated first)
# =============================================================================

# Set schema path for parser
set(DAKOTA_SCHEMA_PATH "${CMAKE_BINARY_DIR}/schema/dakota_study_schema.json" CACHE PATH
    "Path to generated Dakota JSON schema")

# Add parser subdirectory
# Note: Parser generation happens at build time via custom commands that depend
# on DAKOTA_SCHEMA_PATH, so CMake automatically handles the dependency chain:
# dakota_json_schema â†’ schema file â†’ parser generation â†’ parser compilation
add_subdirectory(dakota_parser)

# =============================================================================
# Examples
# =============================================================================
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# =============================================================================
# Parser - Schema Dependency (for parallel builds)
# =============================================================================
# Ensure parser generation depends on schema target (must be after both targets exist)
if(TARGET dakota_generated_sources)
    if(TARGET dakota_json_schema)
        add_dependencies(dakota_generated_sources dakota_json_schema dakota_validation)
        message(STATUS "Parser: Added dependency on dakota_json_schema for parallel builds")
    elseif(TARGET dakota_schema_header)
        add_dependencies(dakota_generated_sources dakota_schema_header)
        message(STATUS "Parser: Added dependency on dakota_schema_header for parallel builds")
    endif()
endif()

# =============================================================================
# Installation summary
# =============================================================================
message(STATUS "")
message(STATUS "=== Dakota JSON Validation Configuration ===")
message(STATUS "  Version:              ${PROJECT_VERSION}")
message(STATUS "  C++ Standard:         ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Python bindings: ${BUILD_PYTHON_BINDINGS}")
message(STATUS "  Build tests:          ${BUILD_TESTS}")
message(STATUS "  Build examples:       ${BUILD_EXAMPLES}")
if(BUILD_PYTHON_BINDINGS)
    message(STATUS "  Python install dir:   ${PYTHON_INSTALL_DIR}")
endif()
message(STATUS "")
