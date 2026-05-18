set(DAKOTA_SCHEMA_DUMP_SCRIPT
  "${Dakota_SOURCE_DIR}/../../model_generation/source/tools/dump_schema.py")

if(DAKOTA_GENERATE_JSON_SCHEMA)
  if(NOT EXISTS "${DAKOTA_SCHEMA_DUMP_SCRIPT}")
    message(FATAL_ERROR
      "Dakota schema dump script not found at ${DAKOTA_SCHEMA_DUMP_SCRIPT}.")
  endif()

  file(GLOB_RECURSE dakota_schema_python_sources CONFIGURE_DEPENDS
    "${Dakota_SOURCE_DIR}/python/dakota/*.py")

  add_custom_command(
    OUTPUT "${DAKOTA_SCHEMA_PATH}"
    COMMAND "${Python3_EXECUTABLE}" -c
      "import json, sys; sys.path.insert(0, r'${Dakota_SOURCE_DIR}/python'); from dakota.spec import DakotaStudy; schema = DakotaStudy.model_json_schema(mode='validation'); open(r'${DAKOTA_SCHEMA_PATH}', 'w').write(json.dumps(schema, indent=4))"
    DEPENDS
      "${DAKOTA_SCHEMA_DUMP_SCRIPT}"
      ${dakota_schema_python_sources}
    WORKING_DIRECTORY "${Dakota_SOURCE_DIR}/src"
    VERBATIM
  )

  add_custom_target(dakota_json_schema DEPENDS "${DAKOTA_SCHEMA_PATH}")
endif()
