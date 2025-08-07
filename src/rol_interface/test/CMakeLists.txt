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

# Minimal wiring test - BoolDispatch only  
#add_executable(test_minimal_wiring
#    test_minimal_wiring.cpp
#)

target_include_directories(test_minimal_wiring PRIVATE
    ../include
)

# Full AdapterModel integration test - requires Dakota core libraries
#add_executable(test_adapter_model
#    test_adapter_model.cpp
#)

target_include_directories(test_adapter_model PRIVATE
    ../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
    ${ROL_INCLUDE_DIRS}
    ${Teuchos_INCLUDE_DIRS}
)

# Link against full Dakota libraries for AdapterModel
target_link_libraries(test_adapter_model 
    ${Dakota_LIBRARIES}  # Use the complete Dakota library set
    ${Dakota_TPL_LIBRARIES}  # Include all TPL dependencies
    rol_interface  # Ensure our interface is linked
)

# Add legacy tests to CTest
# if(DAKOTA_ENABLE_TESTS)
#     add_test(NAME minimal_wiring_test COMMAND test_minimal_wiring)
#     add_test(NAME adapter_model_test COMMAND test_adapter_model)
#     set_property(TEST minimal_wiring_test PROPERTY LABELS ROL Interface Legacy)
#     set_property(TEST adapter_model_test PROPERTY LABELS ROL Interface Legacy)
# endif()
