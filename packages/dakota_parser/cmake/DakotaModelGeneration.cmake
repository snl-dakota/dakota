# =============================================================================
# DakotaModelGeneration.cmake
# CMake module for generating Pydantic models from Dakota XML grammar
# =============================================================================

# Input files for model generation
set(DAKOTA_XML_INPUT "${CMAKE_SOURCE_DIR}/dakota.xml" 
    CACHE FILEPATH "Path to Dakota XML grammar file")
set(DAKOTA_MODEL_EXTENSIONS "${CMAKE_CURRENT_SOURCE_DIR}/model_extensions.json" 
    CACHE FILEPATH "Path to model extensions JSON file")

set(DAKOTA_KEY_MAPPING "${CMAKE_CURRENT_SOURCE_DIR}/key_mapping.json" 
    CACHE FILEPATH "Path to key mapping json file")

# Tool scripts (located in tools/ folder)
set(DAKOTA_TOOLS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tools"
    CACHE PATH "Directory containing tool scripts")
set(DAKOTA_GENERATION_SCRIPT "${DAKOTA_TOOLS_DIR}/xml_to_pydantic.py"
    CACHE FILEPATH "Path to the Pydantic model generation script")
set(DAKOTA_VALIDATION_SCRIPT "${DAKOTA_TOOLS_DIR}/validate_models.py"
    CACHE FILEPATH "Path to the model validation script")

# Output directory for generated models
set(DAKOTA_GENERATED_MODELS_DIR "${CMAKE_BINARY_DIR}/generated_models"
    CACHE PATH "Directory for generated Pydantic models")

# Combined package directory (generated + existing validation)
set(DAKOTA_COMBINED_PACKAGE_DIR "${CMAKE_BINARY_DIR}/dakota_package"
    CACHE PATH "Directory for combined Dakota Python package")

# Marker file to track generation completion
set(DAKOTA_GENERATION_MARKER "${DAKOTA_GENERATED_MODELS_DIR}/.generated")

# =============================================================================
# Function to set up model generation
# =============================================================================
function(dakota_setup_model_generation)
    if(NOT EXISTS "${DAKOTA_XML_INPUT}")
        message(WARNING "Dakota XML input not found: ${DAKOTA_XML_INPUT}")
        message(WARNING "Model generation will be skipped. Set DAKOTA_XML_INPUT to enable.")
        return()
    endif()
    
    if(NOT EXISTS "${DAKOTA_GENERATION_SCRIPT}")
        message(WARNING "Generation script not found: ${DAKOTA_GENERATION_SCRIPT}")
        message(WARNING "Model generation will be skipped.")
        return()
    endif()
    
    # Create the generation command
    add_custom_command(
        OUTPUT "${DAKOTA_GENERATION_MARKER}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DAKOTA_GENERATED_MODELS_DIR}"
        COMMAND ${Python_EXECUTABLE} "${DAKOTA_GENERATION_SCRIPT}"
                "${DAKOTA_XML_INPUT}"
                "${DAKOTA_GENERATED_MODELS_DIR}"
                "${DAKOTA_MODEL_EXTENSIONS}"
                "${DAKOTA_KEY_MAPPING}"
        COMMAND ${CMAKE_COMMAND} -E touch "${DAKOTA_GENERATION_MARKER}"
        DEPENDS
            "${DAKOTA_XML_INPUT}"
            "${DAKOTA_MODEL_EXTENSIONS}"
            "${DAKOTA_GENERATION_SCRIPT}"
             "${DAKOTA_KEY_MAPPING}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Generating Pydantic models from Dakota XML grammar..."
        VERBATIM
    )
    
    # Primary target for incremental Pydantic model generation
    add_custom_target(dakota_pydantic_models
        DEPENDS "${DAKOTA_GENERATION_MARKER}"
        COMMENT "Generate Dakota Pydantic models (incremental)"
    )

    # Convenience target to force regeneration even if dependencies are unchanged
    add_custom_target(dakota_pydantic_models_force
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DAKOTA_GENERATED_MODELS_DIR}"
        COMMAND ${Python_EXECUTABLE} "${DAKOTA_GENERATION_SCRIPT}"
                "${DAKOTA_XML_INPUT}"
                "${DAKOTA_GENERATED_MODELS_DIR}"
                "${DAKOTA_MODEL_EXTENSIONS}"
                "${DAKOTA_KEY_MAPPING}"
        COMMAND ${CMAKE_COMMAND} -E touch "${DAKOTA_GENERATION_MARKER}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Force regenerating Dakota Pydantic models"
        VERBATIM
    )

    # Backward-compatible target name
    add_custom_target(dakota_generate_models
        DEPENDS dakota_pydantic_models
        COMMENT "Dakota model generation target (compatibility alias)"
    )
    
    # Create combined package command - merges generated models with existing validation
    set(COMBINE_MARKER "${DAKOTA_COMBINED_PACKAGE_DIR}/.combined")
    
    # Find the built C++ extension module (platform-specific naming)
    # We'll use a glob pattern at build time since the exact name varies
    set(COPY_SO_SCRIPT "${CMAKE_BINARY_DIR}/copy_validation_so.cmake")
    file(WRITE "${COPY_SO_SCRIPT}" "
# Copy the built dakota_validation_cpp extension to the combined package
file(GLOB SO_FILES \"${CMAKE_BINARY_DIR}/dakota_validation/dakota_validation_cpp*.so\"
                   \"${CMAKE_BINARY_DIR}/dakota_validation/dakota_validation_cpp*.pyd\"
                   \"${CMAKE_BINARY_DIR}/dakota_validation/*/dakota_validation_cpp*.so\"
                   \"${CMAKE_BINARY_DIR}/dakota_validation/*/dakota_validation_cpp*.pyd\")

if(SO_FILES)
    foreach(SO_FILE \${SO_FILES})
        get_filename_component(SO_NAME \"\${SO_FILE}\" NAME)
        message(STATUS \"Copying \${SO_NAME} to combined package\")
        file(COPY \"\${SO_FILE}\" 
             DESTINATION \"${DAKOTA_COMBINED_PACKAGE_DIR}/dakota/spec/validation\")
    endforeach()
else()
    message(WARNING \"No dakota_validation_cpp extension found to copy\")
endif()
")
    
    add_custom_command(
        OUTPUT "${COMBINE_MARKER}"
        # Clean and create combined package directory
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${DAKOTA_COMBINED_PACKAGE_DIR}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DAKOTA_COMBINED_PACKAGE_DIR}"
        # Copy generated models (dakota/spec/*) - this includes the proper __init__.py
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${DAKOTA_GENERATED_MODELS_DIR}/dakota"
                "${DAKOTA_COMBINED_PACKAGE_DIR}/dakota"
        # Copy existing validation package (overwrites any generated validation/)
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/python/dakota/spec/validation"
                "${DAKOTA_COMBINED_PACKAGE_DIR}/dakota/spec/validation"
        # Copy dakota/__init__.py from python/ (package root)
        COMMAND ${CMAKE_COMMAND} -E copy
                "${CMAKE_CURRENT_SOURCE_DIR}/python/dakota/__init__.py"
                "${DAKOTA_COMBINED_PACKAGE_DIR}/dakota/__init__.py"
        # Copy the built C++ extension module
        COMMAND ${CMAKE_COMMAND} -P "${COPY_SO_SCRIPT}"
        # NOTE: Do NOT copy python/dakota/spec/__init__.py - the generated version
        # contains the proper imports (DakotaStudy, DakotaBaseModel, etc.)
        COMMAND ${CMAKE_COMMAND} -E touch "${COMBINE_MARKER}"
        DEPENDS 
            "${DAKOTA_GENERATION_MARKER}"
            "${CMAKE_CURRENT_SOURCE_DIR}/python/dakota/__init__.py"
            dakota_validation_cpp
        COMMENT "Combining generated models with existing validation package..."
        VERBATIM
    )
    
    add_custom_target(dakota_combine_package
        DEPENDS "${COMBINE_MARKER}"
        COMMENT "Dakota package combination target"
    )
    
    # Make sure combination happens after generation
    add_dependencies(dakota_combine_package dakota_pydantic_models)
    
    # Export variables for use by other parts of the build
    set(DAKOTA_MODELS_GENERATED TRUE PARENT_SCOPE)
    set(DAKOTA_COMBINED_PACKAGE_DIR "${DAKOTA_COMBINED_PACKAGE_DIR}" PARENT_SCOPE)
    
    message(STATUS "Dakota model generation configured:")
    message(STATUS "  XML input:     ${DAKOTA_XML_INPUT}")
    message(STATUS "  Extensions:    ${DAKOTA_MODEL_EXTENSIONS}")
    message(STATUS "  Target:        dakota_pydantic_models")
    message(STATUS "  Force target:  dakota_pydantic_models_force")
    message(STATUS "  Generated to:  ${DAKOTA_GENERATED_MODELS_DIR}")
    message(STATUS "  Combined pkg:  ${DAKOTA_COMBINED_PACKAGE_DIR}")
endfunction()

# =============================================================================
# Function to set up model validation test
# =============================================================================
function(dakota_setup_validation_test)
    if(NOT BUILD_TESTS)
        return()
    endif()
    
    if(NOT EXISTS "${DAKOTA_VALIDATION_SCRIPT}")
        message(WARNING "Validation script not found: ${DAKOTA_VALIDATION_SCRIPT}")
        return()
    endif()
    
    if(NOT EXISTS "${DAKOTA_XML_INPUT}")
        message(WARNING "Cannot configure validation test without XML input")
        return()
    endif()
    
    # Set up validation reports directory
    set(VALIDATION_REPORTS_DIR "${CMAKE_BINARY_DIR}/validation_reports")
    file(MAKE_DIRECTORY "${VALIDATION_REPORTS_DIR}")
    
    # The validation test needs:
    # 1. The generated models to exist (dakota_combine_package target)
    # 2. The C++ validation module to be built (dakota_validation_cpp target)
    # 3. lxml to be installed (for XML parsing)
    
    # Create a wrapper script that ensures dependencies are met
    set(VALIDATION_WRAPPER "${CMAKE_BINARY_DIR}/run_validation_test.cmake")
    file(WRITE "${VALIDATION_WRAPPER}" "
# Validation test wrapper script
# Runs the validation test with proper environment setup

set(PYTHON_EXE \"${Python_EXECUTABLE}\")
set(VALIDATION_SCRIPT \"${DAKOTA_VALIDATION_SCRIPT}\")
set(XML_INPUT \"${DAKOTA_XML_INPUT}\")
set(COMBINED_PACKAGE \"${DAKOTA_COMBINED_PACKAGE_DIR}\")
set(EXTENSIONS_FILE \"${DAKOTA_MODEL_EXTENSIONS}\")
set(REPORTS_DIR \"${VALIDATION_REPORTS_DIR}\")

# Check if combined package exists
if(NOT EXISTS \"\${COMBINED_PACKAGE}/dakota/spec\")
    message(FATAL_ERROR \"Generated models not found. Build 'dakota_models' target first.\")
endif()

# Set up PYTHONPATH - combined package contains everything including the .so
set(ENV{PYTHONPATH} \"\${COMBINED_PACKAGE}\")

# Run the validation script
# --output-dir expects the parent of dakota/spec (the script appends dakota/spec internally)
execute_process(
    COMMAND \${PYTHON_EXE} \${VALIDATION_SCRIPT}
            \${XML_INPUT}
            --output-dir \${COMBINED_PACKAGE}
            --extensions \${EXTENSIONS_FILE}
            --report-json \${REPORTS_DIR}/validation_report.json
            --report-txt \${REPORTS_DIR}/validation_report.txt
    WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}\"
    RESULT_VARIABLE RESULT
)

if(NOT RESULT EQUAL 0)
    message(FATAL_ERROR \"Validation test failed with exit code: \${RESULT}\")
endif()
")
    
    add_test(
        NAME dakota_model_validation
        COMMAND ${CMAKE_COMMAND} -P "${VALIDATION_WRAPPER}"
    )
    
    # The test should run after building the models
    # Note: This doesn't create a build dependency, just a test ordering hint
    set_tests_properties(dakota_model_validation PROPERTIES
        LABELS "validation;python"
        TIMEOUT 300  # 5 minutes should be plenty
    )
    
    message(STATUS "Dakota model validation test configured")
    message(STATUS "  Reports dir:   ${VALIDATION_REPORTS_DIR}")
    message(STATUS "  Note: Run 'cmake --build . --target dakota_pydantic_models' before 'ctest -R dakota_model_validation'")
endfunction()

# =============================================================================
# Function to install the generated Python package
# =============================================================================
function(dakota_install_generated_package)
    if(NOT BUILD_PYTHON_BINDINGS)
        return()
    endif()
    
    # Determine install destination
    if(SKBUILD_BUILD)
        # scikit-build-core: install to package root (wheel.install-dir handles final location)
        set(DAKOTA_PYTHON_INSTALL_BASE ".")
    else()
        # Direct CMake: install to lib/python
        set(DAKOTA_PYTHON_INSTALL_BASE "lib/python")
    endif()
    
    # Use install(CODE) to handle files that don't exist at configure time
    # This runs at install time, after the build has completed
    install(CODE "
        # Check if combined package exists
        set(COMBINED_PKG \"${DAKOTA_COMBINED_PACKAGE_DIR}\")
        set(INSTALL_BASE \"${DAKOTA_PYTHON_INSTALL_BASE}\")
        
        if(INSTALL_BASE STREQUAL \".\")
            set(DEST_PREFIX \"\${CMAKE_INSTALL_PREFIX}\")
        else()
            set(DEST_PREFIX \"\${CMAKE_INSTALL_PREFIX}/\${INSTALL_BASE}\")
        endif()
        
        if(EXISTS \"\${COMBINED_PKG}/dakota\")
            message(STATUS \"Installing generated Dakota package to: \${DEST_PREFIX}\")
            
            # Install dakota/__init__.py
            if(EXISTS \"\${COMBINED_PKG}/dakota/__init__.py\")
                file(INSTALL \"\${COMBINED_PKG}/dakota/__init__.py\"
                    DESTINATION \"\${DEST_PREFIX}/dakota\"
                    FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
            endif()
            
            # Install dakota/spec/__init__.py
            if(EXISTS \"\${COMBINED_PKG}/dakota/spec/__init__.py\")
                file(INSTALL \"\${COMBINED_PKG}/dakota/spec/__init__.py\"
                    DESTINATION \"\${DEST_PREFIX}/dakota/spec\"
                    FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
            endif()
            
            # Install dakota/spec/ .py files (not in subdirectories, not validation/)
            file(GLOB SPEC_ROOT_FILES \"\${COMBINED_PKG}/dakota/spec/*.py\")
            foreach(SRC_FILE \${SPEC_ROOT_FILES})
                get_filename_component(FNAME \"\${SRC_FILE}\" NAME)
                if(NOT FNAME STREQUAL \"__init__.py\")
                    file(INSTALL \"\${SRC_FILE}\"
                        DESTINATION \"\${DEST_PREFIX}/dakota/spec\"
                        FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
                endif()
            endforeach()
            
            # Install dakota/spec/method/ subdirectory
            if(EXISTS \"\${COMBINED_PKG}/dakota/spec/method\")
                file(GLOB_RECURSE METHOD_FILES \"\${COMBINED_PKG}/dakota/spec/method/*.py\")
                foreach(SRC_FILE \${METHOD_FILES})
                    file(RELATIVE_PATH REL_PATH \"\${COMBINED_PKG}/dakota/spec/method\" \"\${SRC_FILE}\")
                    get_filename_component(REL_DIR \"\${REL_PATH}\" DIRECTORY)
                    if(REL_DIR)
                        file(INSTALL \"\${SRC_FILE}\"
                            DESTINATION \"\${DEST_PREFIX}/dakota/spec/method/\${REL_DIR}\"
                            FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
                    else()
                        file(INSTALL \"\${SRC_FILE}\"
                            DESTINATION \"\${DEST_PREFIX}/dakota/spec/method\"
                            FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
                    endif()
                endforeach()
            endif()
            
            # Install dakota/spec/shared/ subdirectory
            if(EXISTS \"\${COMBINED_PKG}/dakota/spec/shared\")
                file(GLOB_RECURSE SHARED_FILES \"\${COMBINED_PKG}/dakota/spec/shared/*.py\")
                foreach(SRC_FILE \${SHARED_FILES})
                    file(RELATIVE_PATH REL_PATH \"\${COMBINED_PKG}/dakota/spec/shared\" \"\${SRC_FILE}\")
                    get_filename_component(REL_DIR \"\${REL_PATH}\" DIRECTORY)
                    if(REL_DIR)
                        file(INSTALL \"\${SRC_FILE}\"
                            DESTINATION \"\${DEST_PREFIX}/dakota/spec/shared/\${REL_DIR}\"
                            FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
                    else()
                        file(INSTALL \"\${SRC_FILE}\"
                            DESTINATION \"\${DEST_PREFIX}/dakota/spec/shared\"
                            FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
                    endif()
                endforeach()
            endif()
            
            # Note: validation/ is installed by dakota_validation/CMakeLists.txt
            
            message(STATUS \"Dakota generated package installation complete\")
        else()
            message(WARNING \"Combined package not found at: \${COMBINED_PKG}\")
            message(WARNING \"Run 'cmake --build . --target dakota_models' first\")
        endif()
    "
    COMPONENT python
    )
    
    message(STATUS "Generated package installation configured:")
    message(STATUS "  Install base:  \${CMAKE_INSTALL_PREFIX}/${DAKOTA_PYTHON_INSTALL_BASE}")
endfunction()

# =============================================================================
# Function to set up JSON schema generation target
# =============================================================================
function(dakota_setup_schema_generation)
    # Schema dump script
    set(SCHEMA_DUMP_SCRIPT "${DAKOTA_TOOLS_DIR}/dump_schema.py"
        CACHE FILEPATH "Path to the JSON schema dump script")
    
    # Header generation script
    set(SCHEMA_HEADER_SCRIPT "${DAKOTA_TOOLS_DIR}/generate_schema_header.py"
        CACHE FILEPATH "Path to the C++ header generation script")
    
    if(NOT EXISTS "${SCHEMA_DUMP_SCRIPT}")
        message(STATUS "Schema dump script not found: ${SCHEMA_DUMP_SCRIPT}")
        message(STATUS "JSON schema generation target will not be available")
        return()
    endif()
    
    # Output locations
    set(DAKOTA_SCHEMA_OUTPUT_DIR "${CMAKE_BINARY_DIR}/schema"
        CACHE PATH "Directory for generated JSON schema")
    set(DAKOTA_SCHEMA_FILE "${DAKOTA_SCHEMA_OUTPUT_DIR}/dakota_study_schema.json")
    set(DAKOTA_SCHEMA_HEADER "${DAKOTA_SCHEMA_OUTPUT_DIR}/schema_cbor.hpp")
    
    # Create a wrapper script for schema generation
    set(SCHEMA_WRAPPER "${CMAKE_BINARY_DIR}/run_schema_dump.cmake")
    file(WRITE "${SCHEMA_WRAPPER}" "
# Schema dump wrapper script
# Generates JSON schema from Pydantic models

set(PYTHON_EXE \"${Python_EXECUTABLE}\")
set(SCHEMA_SCRIPT \"${SCHEMA_DUMP_SCRIPT}\")
set(COMBINED_PACKAGE \"${DAKOTA_COMBINED_PACKAGE_DIR}\")
set(OUTPUT_DIR \"${DAKOTA_SCHEMA_OUTPUT_DIR}\")
set(OUTPUT_FILE \"${DAKOTA_SCHEMA_FILE}\")

# Check if combined package exists
if(NOT EXISTS \"\${COMBINED_PACKAGE}/dakota/spec\")
    message(FATAL_ERROR \"Generated models not found. Build 'dakota_models' target first.\")
endif()

# Create output directory
file(MAKE_DIRECTORY \"\${OUTPUT_DIR}\")

# Set up PYTHONPATH - combined package contains everything including the .so
set(ENV{PYTHONPATH} \"\${COMBINED_PACKAGE}\")

# Run the schema dump script
execute_process(
    COMMAND \${PYTHON_EXE} \${SCHEMA_SCRIPT}
    WORKING_DIRECTORY \"\${OUTPUT_DIR}\"
    RESULT_VARIABLE RESULT
    OUTPUT_VARIABLE OUTPUT
    ERROR_VARIABLE ERROR
)

if(NOT RESULT EQUAL 0)
    message(FATAL_ERROR \"Schema generation failed with exit code: \${RESULT}\\n\${ERROR}\")
endif()

# Rename output file if needed (script outputs schema.json)
if(EXISTS \"\${OUTPUT_DIR}/schema.json\" AND NOT \"\${OUTPUT_DIR}/schema.json\" STREQUAL \"\${OUTPUT_FILE}\")
    file(RENAME \"\${OUTPUT_DIR}/schema.json\" \"\${OUTPUT_FILE}\")
endif()

message(STATUS \"Generated JSON schema: \${OUTPUT_FILE}\")
")
    
    # Custom command to generate schema
    add_custom_command(
        OUTPUT "${DAKOTA_SCHEMA_FILE}"
        COMMAND ${CMAKE_COMMAND} -P "${SCHEMA_WRAPPER}"
        DEPENDS 
            "${DAKOTA_COMBINED_PACKAGE_DIR}/.combined"
            "${SCHEMA_DUMP_SCRIPT}"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Generating JSON schema from Pydantic models..."
        VERBATIM
    )
    
    # Custom target for schema generation
    add_custom_target(dakota_json_schema
        DEPENDS "${DAKOTA_SCHEMA_FILE}"
        COMMENT "Dakota JSON schema generation target"
    )
    
    # Schema generation depends on combined package
    add_dependencies(dakota_json_schema dakota_combine_package)
    
    # --- C++ Header Generation (requires cbor2 Python package) ---
    # Only generate if BUILD_SCHEMA_HEADER is enabled
    if(BUILD_SCHEMA_HEADER AND EXISTS "${SCHEMA_HEADER_SCRIPT}")
        # Wrapper script for header generation
        set(HEADER_WRAPPER "${CMAKE_BINARY_DIR}/run_schema_header.cmake")
        file(WRITE "${HEADER_WRAPPER}" "
# Schema header generation wrapper
# Generates C++ header with CBOR-encoded schema

set(PYTHON_EXE \"${Python_EXECUTABLE}\")
set(HEADER_SCRIPT \"${SCHEMA_HEADER_SCRIPT}\")
set(SCHEMA_FILE \"${DAKOTA_SCHEMA_FILE}\")
set(OUTPUT_HEADER \"${DAKOTA_SCHEMA_HEADER}\")

if(NOT EXISTS \"\${SCHEMA_FILE}\")
    message(FATAL_ERROR \"Schema file not found: \${SCHEMA_FILE}\")
endif()

execute_process(
    COMMAND \${PYTHON_EXE} \${HEADER_SCRIPT} \${SCHEMA_FILE} \${OUTPUT_HEADER}
    RESULT_VARIABLE RESULT
    OUTPUT_VARIABLE OUTPUT
    ERROR_VARIABLE ERROR
)

if(NOT RESULT EQUAL 0)
    message(FATAL_ERROR \"Header generation failed: \${RESULT}\\n\${ERROR}\")
endif()

message(STATUS \"\${OUTPUT}\")
")
        
        # Custom command to generate header
        add_custom_command(
            OUTPUT "${DAKOTA_SCHEMA_HEADER}"
            COMMAND ${CMAKE_COMMAND} -P "${HEADER_WRAPPER}"
            DEPENDS 
                "${DAKOTA_SCHEMA_FILE}"
                "${SCHEMA_HEADER_SCRIPT}"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            COMMENT "Generating C++ header with CBOR schema..."
            VERBATIM
        )
        
        # Custom target for header generation
        add_custom_target(dakota_schema_header
            DEPENDS "${DAKOTA_SCHEMA_HEADER}"
            COMMENT "Dakota schema C++ header generation target"
        )

        # Keep the recursive make graph serialized under Unix Makefiles. The
        # file-level dependency on DAKOTA_SCHEMA_FILE is sufficient for the
        # custom command itself, but the explicit target dependency prevents
        # parallel top-level targets from trying to reach the schema rule
        # independently through separate sub-makes.
        add_dependencies(dakota_schema_header dakota_json_schema)

        set(DAKOTA_SCHEMA_HEADER "${DAKOTA_SCHEMA_HEADER}" PARENT_SCOPE)
        
        message(STATUS "  C++ header:    ${DAKOTA_SCHEMA_HEADER}")
        message(STATUS "  Header target: dakota_schema_header")
    elseif(BUILD_SCHEMA_HEADER)
        message(STATUS "  Header script not found: ${SCHEMA_HEADER_SCRIPT}")
        message(STATUS "  C++ header generation will not be available")
    else()
        message(STATUS "  C++ header:    Disabled (set BUILD_SCHEMA_HEADER=ON to enable)")
    endif()
    
    # Export variables
    set(DAKOTA_SCHEMA_FILE "${DAKOTA_SCHEMA_FILE}" PARENT_SCOPE)
    
    message(STATUS "Dakota JSON schema generation configured:")
    message(STATUS "  Dump script:   ${SCHEMA_DUMP_SCRIPT}")
    message(STATUS "  Output:        ${DAKOTA_SCHEMA_FILE}")
    message(STATUS "  Target:        dakota_json_schema")
endfunction()

# =============================================================================
# Function to install schema files
# =============================================================================
function(dakota_install_schema)
    include(GNUInstallDirs)
    
    # Install JSON schema to share/dakota/
    install(CODE "
        set(SCHEMA_FILE \"${DAKOTA_SCHEMA_FILE}\")
        set(DEST_DIR \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/dakota\")
        
        if(EXISTS \"\${SCHEMA_FILE}\")
            message(STATUS \"Installing JSON schema to: \${DEST_DIR}\")
            file(MAKE_DIRECTORY \"\${DEST_DIR}\")
            file(INSTALL \"\${SCHEMA_FILE}\"
                DESTINATION \"\${DEST_DIR}\"
                FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
        else()
            message(WARNING \"Schema file not found: \${SCHEMA_FILE}\")
            message(WARNING \"Run 'cmake --build . --target dakota_json_schema' first\")
        endif()
    ")
    
    # Install C++ header to include/dakota/
    if(DEFINED DAKOTA_SCHEMA_HEADER)
        install(CODE "
            set(HEADER_FILE \"${DAKOTA_SCHEMA_HEADER}\")
            set(DEST_DIR \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/dakota\")
            
            if(EXISTS \"\${HEADER_FILE}\")
                message(STATUS \"Installing schema header to: \${DEST_DIR}\")
                file(MAKE_DIRECTORY \"\${DEST_DIR}\")
                file(INSTALL \"\${HEADER_FILE}\"
                    DESTINATION \"\${DEST_DIR}\"
                    FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
            else()
                message(WARNING \"Schema header not found: \${HEADER_FILE}\")
                message(WARNING \"Run 'cmake --build . --target dakota_schema_header' first\")
            endif()
        ")
    endif()
    
    message(STATUS "Schema installation configured:")
    message(STATUS "  JSON schema:   \${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/dakota/")
    message(STATUS "  C++ header:    \${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/dakota/")
endfunction()
