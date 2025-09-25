message("\n\n=========[ Building Dakota/ROL Interface ]========\n\n")
add_definitions("-DHAVE_DAKOTA_ROL_INTERFACE")

add_library(
  rol_interface STATIC
  src/DakotaROLVector.cpp
  src/DakotaROLObjective.cpp
  src/DakotaROLConstraint.cpp
  src/DakotaROLOptimizer.cpp
  src/ROLOptimizerCompatibility.cpp
)

# Ensure Trilinos is built before rol_interface
if(TARGET TrilinosFramework)
  add_dependencies(rol_interface TrilinosFramework)
endif()

get_target_property(TEUCHOS_CORE_INCLUDES teuchoscore INCLUDE_DIRECTORIES)


target_include_directories(
  rol_interface
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${TEUCHOS_CORE_INCLUDES}
)

target_link_libraries(
  rol_interface
  ${ROL_LIBRARIES}
)

install(TARGETS rol_interface EXPORT ${ExportTarget} DESTINATION lib)

# Add tests subdirectory
if(DAKOTA_ENABLE_TESTS OR BUILD_TESTING)
  add_subdirectory(test)
endif()
