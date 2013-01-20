# Configure the maximum output length for tests.
#
# Usage: configure_test_output_size(<passed_length> <failed_length>)

get_filename_component(_configure_test_output_size_path ${CMAKE_CURRENT_LIST_FILE} PATH)

function(configure_test_output_size _passed_length _failed_length)

  set(PASSED_TEST_OUTPUT_SIZE ${_passed_length})
  set(FAILED_TEST_OUTPUT_SIZE ${_failed_length})
  configure_file(
    ${_configure_test_output_size_path}/OutputLimits.CTestCustom.cmake.in
    ${CMAKE_BINARY_DIR}/CTestCustom.cmake
    @ONLY)

endfunction()
