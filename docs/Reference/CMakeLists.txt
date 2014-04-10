find_package(Java)

add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/DakotaKeywords.dox
          ${CMAKE_CURRENT_BINARY_DIR}/DakotaTopics.dox
  DEPENDS ${Dakota_SOURCE_DIR}/src/dakota.input.nspec
  COMMAND ${Java_JAVA_EXECUTABLE} 
  ARGS -jar ${Dakota_SOURCE_DIR}/local/miscscripts/nidr2refman.jar ${Dakota_SOURCE_DIR}/src/dakota.input.nspec ${Dakota_SOURCE_DIR}/docs/Reference ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(docs-new-ref
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/DakotaKeywords.dox)

