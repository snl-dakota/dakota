include(AddFileCopyCommand)

find_package(Java)

# Generate the keyword and topic pages with a Java utility
add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/DakotaKeywords.dox
          ${CMAKE_CURRENT_BINARY_DIR}/DakotaTopics.dox
  DEPENDS ${Dakota_SOURCE_DIR}/src/dakota.input.nspec
    ${Dakota_SOURCE_DIR}/docs/Reference/TopicTree
  COMMAND ${Java_JAVA_EXECUTABLE} 
  ARGS -jar ${Dakota_SOURCE_DIR}/local/miscscripts/nidr2refman.jar 
    ${Dakota_SOURCE_DIR}/src/dakota.input.nspec 
    ${Dakota_SOURCE_DIR}/docs/Reference 
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Copied Doxygen-related sources
add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/DoxygenFiles/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/DoxygenFiles/Ref_Bibliography.dox ${CMAKE_CURRENT_BINARY_DIR}/Ref_Bibliography.dox)
# For now, shortcut to copy common directory
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/DoxygenFiles/common" "${CMAKE_CURRENT_BINARY_DIR}/common"
  )

# Generated Doxygen files (from PopulateMdTemplate.sh)

# TODO: Add DEPEND on files in $ScriptDir that are used by gen_final_doxygen
add_custom_command(
  DEPENDS  ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/gen_final_doxygen.sh
    ${Dakota_SOURCE_DIR}/src/dakota.input.summary
    ${CMAKE_CURRENT_BINARY_DIR}/DakotaKeywords.dox
    ${CMAKE_CURRENT_BINARY_DIR}/DakotaTopics.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/RefManScripts/DoxygenFile-main.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/RefManScripts/DoxygenFile-input_file_examples.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/RefManScripts/DoxygenFile-input_spec.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/RefManScripts/DoxygenFile-input_spec_summary.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/RefManScripts/DoxygenFile-topics.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/RefManScripts/DoxygenFile-keywords.txt
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/gen_final_doxygen.sh
  ARGS ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/RefManScripts
    ${Dakota_SOURCE_DIR}/src/dakota.input.summary 
    ${CMAKE_CURRENT_BINARY_DIR}/DakotaReferenceManual.dox
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/DakotaReferenceManual.dox
    ${CMAKE_CURRENT_BINARY_DIR}/DakotaInputSpec.dox
)

# Run the final Doxygen to generate output formats
add_custom_target(docs-new-ref
  DEPENDS 
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    ${CMAKE_CURRENT_BINARY_DIR}/Ref_Bibliography.dox
    ${CMAKE_CURRENT_BINARY_DIR}/DakotaReferenceManual.dox
  COMMAND doxygen Doxyfile
)
