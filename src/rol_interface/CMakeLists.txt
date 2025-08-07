message("\n\n=========[ Building Dakota/ROL Interface ]========\n\n")
add_definitions("-DHAVE_DAKOTA_ROL_INTERFACE")

add_library(
  rol_interface 
  src/DakotaROLVector.cpp 
  src/DakotaROLObjective.cpp 
  src/DakotaROLConstraint.cpp 
  src/DakotaROLOptimizer.cpp
  src/ROLOptimizerCompatibility.cpp 
)

target_include_directories(
  rol_interface
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  ${Teuchos_INCLUDE_DIRS}
  ${ROL_INCLUDE_DIRS}
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
  rol_interface
  ${Teuchos_LIBRARIES}
  ${ROL_LIBRARIES}
)

install(TARGETS rol_interface EXPORT ${ExportTarget} DESTINATION lib)

# Add tests subdirectory
if(DAKOTA_ENABLE_TESTS OR BUILD_TESTING)
  add_subdirectory(test)
endif()
