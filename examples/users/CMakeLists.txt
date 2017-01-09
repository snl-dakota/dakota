# Extract files for examples/users
include(AddFileCopyCommand)

set(test_path ${Dakota_SOURCE_DIR}/test)

# Each line is "generated file source file test number"
file(STRINGS "${Dakota_BINARY_DIR}/test/dakota_usersexamples.props"
  users_test_inputs)

# Supplementary data files needed and copied files below
add_file_copy_command(${test_path}/dakota_textbook_nips.lsq.dat
  ${CMAKE_CURRENT_BINARY_DIR}/dakota_textbook_nips.lsq.dat)

set(all_generated_files 
  ${CMAKE_CURRENT_BINARY_DIR}/dakota_textbook_nips.lsq.dat)

# Now iterate over the list, copying to build tree
# Also create all_generated_inputs to use in dependency management
foreach(generated_input ${users_test_inputs})

  # generated a semicolon-separated list from the arguments so we can
  # treat as a list and extract the sub-elements
  separate_arguments(geninput_as_args UNIX_COMMAND ${generated_input})
  list(GET geninput_as_args 0 test_output)
  list(GET geninput_as_args 1 test_input)
  list(GET geninput_as_args 2 test_num)

  add_custom_command(
    DEPENDS "${test_path}/${test_input}.in"
    OUTPUT  "${test_output}.in"
    COMMAND "${PERL_EXECUTABLE}"
    ARGS    "${test_path}/dakota_test.perl" --extract "${test_path}/${test_input}.in" ${test_num} --file-extract="${CMAKE_CURRENT_BINARY_DIR}/${test_output}.in"
    )
  list(APPEND all_generated_files 
    "${CMAKE_CURRENT_BINARY_DIR}/${test_output}.in")

  # Copy all .sav files from source to binary tree, avoid glob to get deps
  if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.out.sav)
    add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.out.sav
      ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.out.sav)
    list(APPEND all_generated_files 
      ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.out.sav)
  endif()
  if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.stdout.sav)
    add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.stdout.sav
      ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.stdout.sav)
    list(APPEND all_generated_files 
      ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.stdout.sav)
  endif()
  if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.dat.sav)
    add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.dat.sav
      ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.dat.sav)
    list(APPEND all_generated_files 
      ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.dat.sav)
  endif()

endforeach()

add_custom_target(examples-users ALL DEPENDS ${all_generated_files})

install(FILES ${all_generated_files}
  DESTINATION "${DAKOTA_EXAMPLES_INSTALL}/examples/users")
