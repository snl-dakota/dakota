# ROL Interface Unit Tests using Dakota's GoogleTest framework

include(DakotaUnitTest)

# GoogleTest-based unit tests for ROL Interface (BoolDispatch only)
dakota_add_unit_test(NAME ROLInterfaceTest
  SOURCES ROLInterfaceTest.cpp
  LABELS ROL Interface
)

# Include ROL headers for the tests
target_include_directories(ROLInterfaceTest PRIVATE
    ../include
    ${ROL_INCLUDE_DIRS}
    ${Teuchos_INCLUDE_DIRS}
)

# ============================================================================
# Legacy tests (kept for reference but replaced by GoogleTest)
# ============================================================================

# Full AdapterModel integration test - requires Dakota core libraries
#add_executable(test_adapter_model
#    test_adapter_model.cpp
#)

#target_include_directories(test_adapter_model PRIVATE
#    ../include
#    ${CMAKE_CURRENT_SOURCE_DIR}/../../
#    ${ROL_INCLUDE_DIRS}
#    ${Teuchos_INCLUDE_DIRS}
#)
#
## Link against full Dakota libraries for AdapterModel
#target_link_libraries(test_adapter_model 
#    ${Dakota_LIBRARIES}  # Use the complete Dakota library set
#    ${Dakota_TPL_LIBRARIES}  # Include all TPL dependencies
#    rol_interface  # Ensure our interface is linked
#)
dakota_add_unit_test( NAME test_minimal_wiring 
                      SOURCES test_minimal_wiring.cpp
                      LINK_DAKOTA_LIBS
                      INCLUDE_DIRECTORIES 
                    
                    )
                                        
target_include_directories(test_minimal_wiring PRIVATE ../include                                  
                                               ${ROL_INCLUDE_DIRS}
                                               ${Teuchos_INCLUDE_DIRS})

#dakota_add_unit_test( NAME test_adaptor_model
#                      SOURCES test_adaptor_model.cpp
#                      LINK_DAKOTA_LIBS)
#                                        
#target_include_directories(test_adaptor_model PRIVATE ../include                                  
#                                               ${ROL_INCLUDE_DIRS}
#                                               ${Teuchos_INCLUDE_DIRS})
#    
#target_link_libraries(test_adapter_model rol_interface)
    

# Add legacy tests to CTest
# if(DAKOTA_ENABLE_TESTS)
#     add_test(NAME test_minimal_wiring COMMAND test_minimal_wiring)
#     add_test(NAME adapter_model_test COMMAND test_adapter_model)
#     set_property(TEST test_minimal_wiring PROPERTY LABELS ROL Interface Legacy)
#     set_property(TEST adapter_model_test PROPERTY LABELS ROL Interface Legacy)
# endif()

# Unit test for Constraint class
dakota_add_unit_test(NAME ConstraintTest
  SOURCES ConstraintTest.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS )

target_include_directories(ConstraintTest PRIVATE
    ../../
    ../include
    ${ROL_INCLUDE_DIRS}
    ${Teuchos_INCLUDE_DIRS}
)

# NOTE: LINK_DAKOTA_LIBS already provides rol_interface and dakota_src in correct order
# Removed explicit rol_interface link to avoid circular dependency issues

# Integration test for Constraint class
dakota_add_unit_test(NAME ROLConstraintIntegration
  SOURCES test_constraint_integration.cpp
  LABELS ROL Interface
  LINK_DAKOTA_LIBS
)

target_include_directories(ROLConstraintIntegration PRIVATE
    ../../
    ../include
    ${ROL_INCLUDE_DIRS}
    ${Teuchos_INCLUDE_DIRS}
)

# Link ROL interface library to tests that need it
# Removed explicit rol_interface links - already provided by LINK_DAKOTA_LIBS

# Copy input files for tests
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/constraint_test.in 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Copy ROL XML parameter file for tests
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/rol_input.xml
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Additional ROL interface tests
dakota_add_unit_test(NAME test_objective
  SOURCES test_objective.cpp
  LABELS ROL Interface
  LINK_DAKOTA_LIBS
)

target_include_directories(test_objective PRIVATE
    ../../
    ../include
    ${ROL_INCLUDE_DIRS}
    ${Teuchos_INCLUDE_DIRS}
)

# Removed - already provided by LINK_DAKOTA_LIBS

dakota_add_unit_test(NAME test_rosenbrock_integration
  SOURCES test_rosenbrock_integration.cpp
  LABELS ROL Interface
  LINK_DAKOTA_LIBS
)

target_include_directories(test_rosenbrock_integration PRIVATE
    ../../
    ../include
    ${ROL_INCLUDE_DIRS}
    ${Teuchos_INCLUDE_DIRS}
)

# Removed - already provided by LINK_DAKOTA_LIBS

dakota_add_unit_test(NAME test_wiring_concept
  SOURCES test_wiring_concept.cpp
  LABELS ROL Interface
  LINK_DAKOTA_LIBS
)

target_include_directories(test_wiring_concept PRIVATE
    ../../
    ../include
    ${ROL_INCLUDE_DIRS}
    ${Teuchos_INCLUDE_DIRS}
)

# Removed - already provided by LINK_DAKOTA_LIBS

