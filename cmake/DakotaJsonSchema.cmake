if(DAKOTA_GENERATE_JSON_SCHEMA OR DAKOTA_CHECK_JSON_SCHEMA)
  # CONFIGURE_DEPENDS makes CMake reconfigure when Python model files are added
  # or removed. GLOB_RECURSE includes every .py file under spec subdirectories.
  file(GLOB_RECURSE dakota_schema_python_sources CONFIGURE_DEPENDS
    "${Dakota_SOURCE_DIR}/python/dakota/spec/*.py")

  set(dakota_generated_schema
    "${Dakota_BINARY_DIR}/generated/schema/dakota.json")

  set(dakota_schema_update_command)
  if(DAKOTA_GENERATE_JSON_SCHEMA)
    set(dakota_schema_update_command
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${dakota_generated_schema}" "${DAKOTA_SCHEMA_PATH}")
  else()
    set(dakota_schema_update_command
      COMMAND "${CMAKE_COMMAND}"
        "-DGENERATED_SCHEMA=${dakota_generated_schema}"
        "-DSOURCE_SCHEMA=${DAKOTA_SCHEMA_PATH}"
        -P "${Dakota_SOURCE_DIR}/cmake/CheckDakotaJsonSchema.cmake")
  endif()

  # This target intentionally runs whenever it participates in a build. The
  # source-tree schema is only touched when its generated contents differ, so
  # unchanged models do not trigger downstream parser regeneration.
  add_custom_target(dakota_json_schema ALL
    COMMAND "${CMAKE_COMMAND}" -E make_directory
      "${Dakota_BINARY_DIR}/generated/schema"
    COMMAND "${Python3_EXECUTABLE}" -c
      "import json, sys; sys.path.insert(0, r'${Dakota_SOURCE_DIR}/python'); from dakota.spec import DakotaStudy; schema = DakotaStudy.model_json_schema(mode='validation'); open(r'${dakota_generated_schema}', 'w').write(json.dumps(schema, indent=4))"
    ${dakota_schema_update_command}
    BYPRODUCTS "${dakota_generated_schema}"
    DEPENDS ${dakota_schema_python_sources}
    WORKING_DIRECTORY "${Dakota_SOURCE_DIR}/src"
    VERBATIM
  )

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
  add_dependencies(dakota_xml_grammar dakota_json_schema)
endif()
