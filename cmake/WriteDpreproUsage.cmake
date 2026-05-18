if(NOT DEFINED Python_EXECUTABLE)
  message(FATAL_ERROR "Python_EXECUTABLE is required")
endif()

if(NOT DEFINED DPREPRO_SCRIPT)
  message(FATAL_ERROR "DPREPRO_SCRIPT is required")
endif()

if(NOT DEFINED OUTPUT_FILE)
  message(FATAL_ERROR "OUTPUT_FILE is required")
endif()

execute_process(
  COMMAND "${Python_EXECUTABLE}" "${DPREPRO_SCRIPT}" --help
  OUTPUT_FILE "${OUTPUT_FILE}"
  RESULT_VARIABLE dprepro_usage_result
)

if(NOT dprepro_usage_result EQUAL 0)
  message(FATAL_ERROR "Failed to generate ${OUTPUT_FILE}")
endif()
