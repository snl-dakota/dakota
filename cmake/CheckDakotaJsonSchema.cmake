if(NOT DEFINED GENERATED_SCHEMA OR NOT DEFINED SOURCE_SCHEMA)
  message(FATAL_ERROR
    "CheckDakotaJsonSchema.cmake requires GENERATED_SCHEMA and SOURCE_SCHEMA")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E compare_files
    "${GENERATED_SCHEMA}" "${SOURCE_SCHEMA}"
  RESULT_VARIABLE dakota_schema_compare_result
)

if(NOT dakota_schema_compare_result EQUAL 0)
  message(FATAL_ERROR
    "The checked-in Dakota JSON schema is out of date.\n"
    "  Generated schema: ${GENERATED_SCHEMA}\n"
    "  Checked-in schema: ${SOURCE_SCHEMA}\n"
    "Reconfigure with DAKOTA_CHECK_JSON_SCHEMA=OFF and DAKOTA_GENERATE_JSON_SCHEMA=ON to update it.")
endif()
