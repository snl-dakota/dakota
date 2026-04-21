cmake_minimum_required(VERSION 3.15)

# Detect if this is a standalone build or subdirectory build
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(DAKOTA_VALIDATION_STANDALONE TRUE)
else()
    set(DAKOTA_VALIDATION_STANDALONE FALSE)
endif()

# Only set project if standalone
if(DAKOTA_VALIDATION_STANDALONE)
    project(dakota_validation_cpp VERSION 0.1.0 LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    
    option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)
    option(BUILD_TESTS "Build tests" ON)
    
    include(FetchContent)
    
    # ==========================================================================
    # Dependencies for standalone build
    # ==========================================================================
    
    # Python
    if(BUILD_PYTHON_BINDINGS)
        find_package(Python REQUIRED COMPONENTS Interpreter Development)
        message(STATUS "Python executable: ${Python_EXECUTABLE}")
        
        # Validate include path (conda workaround)
        if(Python_INCLUDE_DIRS)
            list(GET Python_INCLUDE_DIRS 0 _first_include_dir)
            if(NOT EXISTS "${_first_include_dir}")
                execute_process(
                    COMMAND "${Python_EXECUTABLE}" -c 
                        "import sysconfig; print(sysconfig.get_path('include'))"
                    OUTPUT_VARIABLE _python_include_from_sysconfig
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    RESULT_VARIABLE _sysconfig_result
                )
                if(_sysconfig_result EQUAL 0 AND EXISTS "${_python_include_from_sysconfig}")
                    set(Python_INCLUDE_DIRS "${_python_include_from_sysconfig}" CACHE PATH "" FORCE)
                endif()
            endif()
        endif()
    endif()
    
    # nlohmann_json
    find_package(nlohmann_json 3.2.0 QUIET)
    if(NOT nlohmann_json_FOUND)
        message(STATUS "Fetching nlohmann_json...")
        FetchContent_Declare(
            nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG        v3.11.3
            GIT_SHALLOW    TRUE
        )
        set(JSON_Install OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(nlohmann_json)
        set(NLOHMANN_JSON_FETCHED TRUE)
    else()
        set(NLOHMANN_JSON_FETCHED FALSE)
    endif()
    
    # pybind11
    if(BUILD_PYTHON_BINDINGS)
        message(STATUS "Fetching pybind11...")
        set(PYBIND11_FINDPYTHON OFF CACHE BOOL "" FORCE)
        set(PYTHON_EXECUTABLE "${Python_EXECUTABLE}" CACHE FILEPATH "" FORCE)
        list(GET Python_INCLUDE_DIRS 0 _py_include_dir)
        set(PYTHON_INCLUDE_DIR "${_py_include_dir}" CACHE PATH "" FORCE)
        
        FetchContent_Declare(
            pybind11
            GIT_REPOSITORY https://github.com/pybind/pybind11.git
            GIT_TAG        v2.13.6
            GIT_SHALLOW    TRUE
            OVERRIDE_FIND_PACKAGE
        )
        FetchContent_MakeAvailable(pybind11)
        
        # pybind11_json
        message(STATUS "Fetching pybind11_json...")
        FetchContent_Declare(
            pybind11_json
            GIT_REPOSITORY https://github.com/pybind/pybind11_json.git
            GIT_TAG        0.2.15
            GIT_SHALLOW    TRUE
            SOURCE_SUBDIR  nonexistent
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
    
    # GoogleTest
    if(BUILD_TESTS)
        message(STATUS "Fetching GoogleTest...")
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        v1.15.2
            GIT_SHALLOW    TRUE
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
        enable_testing()
    endif()
    
    if(NOT DEFINED PYTHON_INSTALL_DIR)
        set(PYTHON_INSTALL_DIR "dakota/spec/validation")
    endif()
else()
    # Subdirectory build - parent provides all dependencies
    set(NLOHMANN_JSON_FETCHED TRUE)
endif()

# =============================================================================
# Core C++ library
# =============================================================================
# Library type (STATIC/SHARED) controlled by BUILD_SHARED_LIBS
add_library(dakota_validation
    src/dakota_validation.cpp
    src/dakota_variable_validators.cpp
    src/dakota_computed_fields.cpp
)

target_include_directories(dakota_validation 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(dakota_validation PUBLIC nlohmann_json::nlohmann_json)

# Alias for use by other targets
add_library(dakota::validation ALIAS dakota_validation)

# =============================================================================
# Python bindings
# =============================================================================
if(BUILD_PYTHON_BINDINGS)
    pybind11_add_module(dakota_validation_cpp
        src/bindings.cpp
    )
    
    target_link_libraries(dakota_validation_cpp PRIVATE 
        dakota_validation
        pybind11_json_iface
    )
    
    # Install Python module
    install(TARGETS dakota_validation_cpp
        LIBRARY DESTINATION "${PYTHON_INSTALL_DIR}"
        COMPONENT python
    )
    
    # Install Python source files
    if(SKBUILD_BUILD)
        # scikit-build-core: install from top-level python/ directory
        install(DIRECTORY ${CMAKE_SOURCE_DIR}/python/dakota/spec/validation/
            DESTINATION "${PYTHON_INSTALL_DIR}"
            COMPONENT python
            FILES_MATCHING PATTERN "*.py"
        )
        # Install dakota/__init__.py for package structure
        install(FILES ${CMAKE_SOURCE_DIR}/python/dakota/__init__.py
            DESTINATION "dakota"
            COMPONENT python
        )
        # NOTE: Do NOT install python/dakota/spec/__init__.py here.
        # When GENERATE_PYDANTIC_MODELS is ON, the generated __init__.py 
        # (with proper imports) is installed by DakotaModelGeneration.cmake.
        # When GENERATE_PYDANTIC_MODELS is OFF, user must provide their own spec package.
    else()
        # Direct CMake: install from subdirectory (may not exist)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/python/dakota/spec/validation")
            install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python/dakota/spec/validation/
                DESTINATION "${PYTHON_INSTALL_DIR}"
                COMPONENT python
                FILES_MATCHING PATTERN "*.py"
            )
        endif()
    endif()
endif()

# =============================================================================
# Tests
# =============================================================================
if(BUILD_TESTS)
    if(DAKOTA_VALIDATION_STANDALONE)
        enable_testing()
    endif()

    add_executable(test_validation test/test_validation.cpp)
    target_link_libraries(test_validation 
        dakota_validation 
        GTest::gtest_main
    )
    add_test(NAME test_validation COMMAND test_validation)

    if(BUILD_PYTHON_BINDINGS)
        # Check Python test requirements before registering pytest-based tests.
        execute_process(
            COMMAND ${Python_EXECUTABLE} -c
                    "import importlib.util, sys; sys.exit(0 if importlib.util.find_spec('pytest') else 1)"
            RESULT_VARIABLE _pytest_result
        )
        if(_pytest_result EQUAL 0)
            set(PYTEST_AVAILABLE TRUE)
        else()
            set(PYTEST_AVAILABLE FALSE)
        endif()

        execute_process(
            COMMAND ${Python_EXECUTABLE} -c
                    "import pydantic, sys; sys.stdout.write(pydantic.__version__)"
            RESULT_VARIABLE _pydantic_result
            OUTPUT_VARIABLE PYDANTIC_VERSION
            ERROR_VARIABLE PYDANTIC_ERROR
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
        )
        if(_pydantic_result EQUAL 0)
            if(PYDANTIC_VERSION VERSION_LESS 2.12)
                set(PYDANTIC_AVAILABLE FALSE)
                set(PYDANTIC_ERROR "Pydantic version ${PYDANTIC_VERSION} is less than required 2.12")
            else()
                set(PYDANTIC_AVAILABLE TRUE)
            endif()
        else()
            set(PYDANTIC_AVAILABLE FALSE)
            if(NOT PYDANTIC_VERSION)
                set(PYDANTIC_VERSION "unknown")
            endif()
        endif()

        # Report status
        if(PYTEST_AVAILABLE)
            message(STATUS "pytest is available")
        else()
            message(WARNING "pytest is not available - Python validation tests will be skipped")
        endif()

        if(PYDANTIC_AVAILABLE)
            message(STATUS "Pydantic ${PYDANTIC_VERSION} is available")
        else()
            if(PYDANTIC_ERROR)
                message(WARNING "Pydantic requirement not met: ${PYDANTIC_ERROR}")
            else()
                message(WARNING "Pydantic is not available")
            endif()
        endif()

        # Only add Python tests if both pytest and pydantic are available
        if(PYTEST_AVAILABLE AND PYDANTIC_AVAILABLE)
            add_test(
                NAME python_validation_tests
                COMMAND ${Python_EXECUTABLE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}/test -v
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )

            # PYTHONPATH needs:
            # 1. Combined package (for dakota.spec.* modules) - when model generation is enabled
            # 2. Current build dir (for the compiled dakota_validation_cpp.so)
            # The combined package also contains the .so after the combine step
            set(_PYTEST_PYTHONPATH "${CMAKE_BINARY_DIR}/dakota_package:${CMAKE_CURRENT_BINARY_DIR}")

            set_tests_properties(python_validation_tests PROPERTIES
                ENVIRONMENT "PYTHONPATH=${_PYTEST_PYTHONPATH}"
            )
        else()
            message(STATUS "Skipping Python validation tests due to missing dependencies")
        endif()
    endif()
endif()

# =============================================================================
# Installation (C++ artifacts - skip for scikit-build-core wheel builds)
# =============================================================================
if(NOT SKBUILD_BUILD)
    install(DIRECTORY include/
        DESTINATION include
    )

    if(NOT NLOHMANN_JSON_FETCHED)
        # Full CMake export when using system nlohmann_json
        install(TARGETS dakota_validation
            EXPORT dakota_validation_targets
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib
        )
        
        install(EXPORT dakota_validation_targets
            FILE dakota_validationTargets.cmake
            NAMESPACE dakota::
            DESTINATION lib/cmake/dakota_validation
        )

        include(CMakePackageConfigHelpers)

        configure_package_config_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake/dakota_validationConfig.cmake.in
            ${CMAKE_CURRENT_BINARY_DIR}/dakota_validationConfig.cmake
            INSTALL_DESTINATION lib/cmake/dakota_validation
        )

        write_basic_package_version_file(
            ${CMAKE_CURRENT_BINARY_DIR}/dakota_validationConfigVersion.cmake
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY SameMajorVersion
        )

        install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/dakota_validationConfig.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/dakota_validationConfigVersion.cmake
            DESTINATION lib/cmake/dakota_validation
        )
    else()
        # Simple install when using fetched dependencies
        install(TARGETS dakota_validation
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib
        )
    endif()
endif()
