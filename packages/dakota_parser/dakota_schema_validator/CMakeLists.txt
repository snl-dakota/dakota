cmake_minimum_required(VERSION 3.15)

# Schema validator library
# Library type (STATIC/SHARED) controlled by BUILD_SHARED_LIBS
add_library(dakota_schema_validator
    src/dakota_schema_validator.cpp
)

target_include_directories(dakota_schema_validator
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link to validation library (must be available from parent project)
target_link_libraries(dakota_schema_validator
    PUBLIC
        dakota::validation
        nlohmann_json::nlohmann_json
)

# Alias for consistency
add_library(dakota::schema_validator ALIAS dakota_schema_validator)

# Python bindings
if(BUILD_PYTHON_BINDINGS)
    pybind11_add_module(dakota_schema_validator_cpp
        src/bindings.cpp
    )
    
    target_link_libraries(dakota_schema_validator_cpp PRIVATE
        dakota_schema_validator
        pybind11_json_iface
    )
    
    # Install Python module
    install(TARGETS dakota_schema_validator_cpp
        LIBRARY DESTINATION "${PYTHON_INSTALL_DIR}"
        COMPONENT python
    )
endif()

# Tests
if(BUILD_TESTS)
    add_executable(test_schema_validator test/test_schema_validator.cpp)
    target_link_libraries(test_schema_validator
        dakota_schema_validator
        GTest::gtest_main
    )
    add_test(NAME test_schema_validator COMMAND test_schema_validator)
endif()

# Install C++ artifacts (skip for scikit-build-core wheel builds)
if(NOT SKBUILD_BUILD)
    install(DIRECTORY include/
        DESTINATION include
    )

    install(TARGETS dakota_schema_validator
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
    )
endif()
