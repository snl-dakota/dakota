set(DAKOTA_IR_GENERATED_DIR "${Dakota_BINARY_DIR}/generated/src/ir")
set(DAKOTA_IR_CODEGEN_DIR "${Dakota_SOURCE_DIR}/src/ir_codegen")
set(DAKOTA_IR_DEFAULTS_EXTRACTOR "${DAKOTA_IR_CODEGEN_DIR}/extract_defaults_from_schema.py")
set(DAKOTA_IR_GENERATOR "${DAKOTA_IR_CODEGEN_DIR}/generate_ir_tables.py")
set(DAKOTA_IR_SCHEMA "${Dakota_SOURCE_DIR}/src/dakota.json")
set(DAKOTA_IR_OVERRIDE_REGISTRY "${Dakota_SOURCE_DIR}/src/default_overrides_registry.json")
set(DAKOTA_IR_POLICY_REGISTRY "${Dakota_SOURCE_DIR}/src/default_policy_registry.json")
set(DAKOTA_IR_SCHEMA_DEFAULTS "${DAKOTA_IR_GENERATED_DIR}/schema_defaults_extracted.json")

if(NOT EXISTS "${DAKOTA_IR_DEFAULTS_EXTRACTOR}")
  message(FATAL_ERROR
    "IR schema-default extractor not found at ${DAKOTA_IR_DEFAULTS_EXTRACTOR}. "
    "Expected the IR code generation scripts under ${DAKOTA_IR_CODEGEN_DIR}.")
endif()

if(NOT EXISTS "${DAKOTA_IR_GENERATOR}")
  message(FATAL_ERROR
    "IR table generator not found at ${DAKOTA_IR_GENERATOR}. "
    "Expected the IR code generation scripts under ${DAKOTA_IR_CODEGEN_DIR}.")
endif()

list(APPEND DAKOTA_INCDIRS "${DAKOTA_IR_GENERATED_DIR}")
list(APPEND DAKOTA_INCDIRS "${Dakota_SOURCE_DIR}/src/generated_ir_tables")

set(dakota_ir_generated_files
  "${DAKOTA_IR_GENERATED_DIR}/generated_ir_environment.cpp"
  "${DAKOTA_IR_GENERATED_DIR}/generated_ir_interface.cpp"
  "${DAKOTA_IR_GENERATED_DIR}/generated_ir_method.cpp"
  "${DAKOTA_IR_GENERATED_DIR}/generated_ir_model.cpp"
  "${DAKOTA_IR_GENERATED_DIR}/generated_ir_registry.cpp"
  "${DAKOTA_IR_GENERATED_DIR}/generated_ir_responses.cpp"
  "${DAKOTA_IR_GENERATED_DIR}/generated_ir_variables.cpp"
  "${DAKOTA_IR_GENERATED_DIR}/generated_ir_types.hpp"
  )

add_custom_command(
  OUTPUT ${dakota_ir_generated_files}
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${DAKOTA_IR_GENERATED_DIR}"
  COMMAND "${Python3_EXECUTABLE}" "${DAKOTA_IR_DEFAULTS_EXTRACTOR}"
    --schema "${DAKOTA_IR_SCHEMA}"
    --output-json "${DAKOTA_IR_SCHEMA_DEFAULTS}"
  COMMAND "${Python3_EXECUTABLE}" "${DAKOTA_IR_GENERATOR}"
    --schema-defaults "${DAKOTA_IR_SCHEMA_DEFAULTS}"
    --override-registry "${DAKOTA_IR_OVERRIDE_REGISTRY}"
    --policy-registry "${DAKOTA_IR_POLICY_REGISTRY}"
    --schema "${DAKOTA_IR_SCHEMA}"
    --output-dir "${DAKOTA_IR_GENERATED_DIR}"
  DEPENDS
    "${DAKOTA_IR_DEFAULTS_EXTRACTOR}"
    "${DAKOTA_IR_GENERATOR}"
    "${DAKOTA_IR_OVERRIDE_REGISTRY}"
    "${DAKOTA_IR_POLICY_REGISTRY}"
    "${DAKOTA_IR_SCHEMA}"
  VERBATIM
  )

add_custom_target(dakota_ir_generated_tables DEPENDS ${dakota_ir_generated_files})
