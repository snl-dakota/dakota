# Extract files for examples/tutorial

set(test_path ${Dakota_SOURCE_DIR}/test)

set(tutorial_test_inputs
  "${test_path}/dakota_rosenbrock_users.in 0 rosen_multidim.in"
  "${test_path}/dakota_rosenbrock_users.in 2 rosen_grad_opt.in"
  "${test_path}/dakota_rosenbrock_users.in 6 rosen_sampling.in"
  "${test_path}/dakota_rosenbrock_users.in 7 rosen_syscall.in"
)

# Now iterate over the list, parsing and extracting with dakota_test.perl.
# Also create all_generated_inputs to use in dependency management
set(all_generated_inputs)
foreach(generated_input ${tutorial_test_inputs})

  # generated a semicolon-separated list from the arguments so we can
  # treat as a list and extract the sub-elements
  separate_arguments(geninput_as_args UNIX_COMMAND ${generated_input})
  list(GET geninput_as_args 0 test_input)
  list(GET geninput_as_args 1 test_num)
  list(GET geninput_as_args 2 test_output)
  list(APPEND all_generated_inputs ${CMAKE_CURRENT_BINARY_DIR}/${test_output})

  add_custom_command(
    OUTPUT  ${test_output}
    COMMAND ${PERL_EXECUTABLE} ${Dakota_SOURCE_DIR}/test/dakota_test.perl 
    ARGS    --extract ${test_input} ${test_num} --file-extract=${CMAKE_CURRENT_BINARY_DIR}/${test_output}
  )

endforeach()

add_custom_target(examples-tutorial-inputs ALL DEPENDS ${all_generated_inputs})

install(FILES ${all_generated_inputs} DESTINATION
  "${DAKOTA_EXAMPLES_INSTALL}/examples/tutorial")
