if(DAKOTA_GENERATE_JSON_SCHEMA)
  file(GLOB_RECURSE dakota_schema_python_sources CONFIGURE_DEPENDS
    "${Dakota_SOURCE_DIR}/python/dakota/*.py")

  add_custom_command(
    OUTPUT "${DAKOTA_SCHEMA_PATH}"
    COMMAND "${Python3_EXECUTABLE}" -c
      "import json, sys; sys.path.insert(0, r'${Dakota_SOURCE_DIR}/python'); from dakota.spec import DakotaStudy; schema = DakotaStudy.model_json_schema(mode='validation'); open(r'${DAKOTA_SCHEMA_PATH}', 'w').write(json.dumps(schema, indent=4))"
    DEPENDS
      ${dakota_schema_python_sources}
    WORKING_DIRECTORY "${Dakota_SOURCE_DIR}/src"
    VERBATIM
  )

  add_custom_target(dakota_json_schema DEPENDS "${DAKOTA_SCHEMA_PATH}")

  add_custom_command(
    OUTPUT "${DAKOTA_XML_INPUT}"
    COMMAND "${Python3_EXECUTABLE}"
      "${Dakota_SOURCE_DIR}/src/xml_codegen/generate_dakota_xml.py"
      --schema "${DAKOTA_SCHEMA_PATH}"
      --output "${DAKOTA_XML_INPUT}"
    DEPENDS
      "${DAKOTA_SCHEMA_PATH}"
      "${Dakota_SOURCE_DIR}/src/xml_codegen/generate_dakota_xml.py"
    WORKING_DIRECTORY "${Dakota_SOURCE_DIR}/src"
    VERBATIM
  )

  add_custom_target(dakota_xml_grammar DEPENDS "${DAKOTA_XML_INPUT}")
endif()
