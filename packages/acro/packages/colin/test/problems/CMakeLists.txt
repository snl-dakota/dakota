
# Auxilliary test files
file(GLOB all_test_inputs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
  *.col *.mod *.nl *.row)
foreach(test_file ${all_test_inputs})
  configure_file(${test_file} ${CMAKE_CURRENT_BINARY_DIR}/${test_file} COPYONLY)
endforeach()
