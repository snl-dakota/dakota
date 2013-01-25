# Extract files for examples/tutorial

set(test_path ${Dakota_SOURCE_DIR}/test)

set(tutorial_test_inputs
  "${test_path}/dakota_rosenbrock_users.in 0 rosen_multidim"
  "${test_path}/dakota_rosenbrock_users.in 2 rosen_grad_opt"
  "${test_path}/dakota_rosenbrock_users.in 6 rosen_sampling"
  "${test_path}/dakota_rosenbrock_users.in 7 rosen_syscall"
)

# Now iterate over the list, parsing and extracting with dakota_test.perl.
# Also create all_generated_inputs to use in dependency management
set(all_generated_inputs)
set(all_copied_files)
foreach(generated_input ${tutorial_test_inputs})

  # generated a semicolon-separated list from the arguments so we can
  # treat as a list and extract the sub-elements
  separate_arguments(geninput_as_args UNIX_COMMAND ${generated_input})
  list(GET geninput_as_args 0 test_input)
  list(GET geninput_as_args 1 test_num)
  list(GET geninput_as_args 2 test_output)
  list(APPEND all_generated_inputs ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.in)

  add_custom_command(
    OUTPUT  ${test_output}.in
    COMMAND ${PERL_EXECUTABLE} ${Dakota_SOURCE_DIR}/test/dakota_test.perl 
    ARGS    --extract ${test_input} ${test_num} --file-extract=${CMAKE_CURRENT_BINARY_DIR}/${test_output}.in
  )

  # Copy all .sav files from source to binary tree, avoid glob to get deps
  add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.out.sav
    ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.out.sav)
  add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.stdout.sav
    ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.stdout.sav)
  add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.dat.sav
    ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.dat.sav)

  list(APPEND all_copied_files ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.out.sav)
  list(APPEND all_copied_files ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.stdout.sav)
  list(APPEND all_copied_files ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.dat.sav)

endforeach()

add_custom_target(examples-tutorial ALL 
  DEPENDS ${all_generated_inputs} ${all_copied_files})

install(FILES ${all_generated_inputs} ${all_copied_files} 
  DESTINATION "${DAKOTA_EXAMPLES_INSTALL}/examples/tutorial")
