# Extract files for examples/cantilever
include(AddFileCopyCommand)

set(test_path ${Dakota_SOURCE_DIR}/test)

set(cantilever_test_inputs
  "${test_path}/dakota_cantilever_examples.in 0 cantilever_vector.in"
  "${test_path}/dakota_cantilever_examples.in 1 cantilever_centered.in"
  "${test_path}/dakota_cantilever_examples.in 2 cantilever_grid.in"
  "${test_path}/dakota_cantilever_examples.in 3 cantilever_sa.in"
  "${test_path}/dakota_cantilever_examples.in 4 cantilever_uq_lhs.in"
  "${test_path}/dakota_cantilever_examples.in 5 cantilever_uq_rel.in"
  "${test_path}/dakota_cantilever_examples.in 6 cantilever_uq_pce.in"
  "${test_path}/dakota_cantilever_examples.in 7 cantilever_optimization.in"
  "${test_path}/dakota_cantilever_examples.in 8 cantilever_calibration.in"
)

add_file_copy_command(${test_path}/dakota_cantilever_examples.clean.dat
  ${CMAKE_CURRENT_BINARY_DIR}/dakota_cantilever_examples.clean.dat)
add_file_copy_command(${test_path}/dakota_cantilever_examples.error.dat
  ${CMAKE_CURRENT_BINARY_DIR}/dakota_cantilever_examples.error.dat)

set(all_copied_files 
  ${CMAKE_CURRENT_BINARY_DIR}/dakota_cantilever_examples.clean.dat
  ${CMAKE_CURRENT_BINARY_DIR}/dakota_cantilever_examples.error.dat)


# Now iterate over the list, parsing and extracting with dakota_test.perl.
# Also create all_generated_inputs to use in dependency management
set(all_generated_inputs)
foreach(generated_input ${cantilever_test_inputs})

  # generated a semicolon-separated list from the arguments so we can
  # treat as a list and extract the sub-elements
  separate_arguments(geninput_as_args UNIX_COMMAND ${generated_input})
  list(GET geninput_as_args 0 test_input)
  list(GET geninput_as_args 1 test_num)
  list(GET geninput_as_args 2 test_output)
  list(APPEND all_generated_inputs ${CMAKE_CURRENT_BINARY_DIR}/${test_output})

  add_custom_command(
    DEPENDS "${test_input}"
    OUTPUT  ${test_output}
    COMMAND ${PERL_EXECUTABLE} ${Dakota_SOURCE_DIR}/test/dakota_test.perl 
    ARGS    --extract ${test_input} ${test_num} --file-extract=${CMAKE_CURRENT_BINARY_DIR}/${test_output}
  )

endforeach()

add_custom_target(examples-cantilever ALL 
  DEPENDS ${all_generated_inputs} ${all_copied_files}) 

install(FILES ${all_generated_inputs} ${all_copied_files}
  DESTINATION "${DAKOTA_EXAMPLES_INSTALL}/examples/cantilever")
