#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2023
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________


# _users_test_inputs: contents of properties file as variable
# _output_dir: where to write generated examples
# _all_generated_files_out: name of variable to write in PARENT_SCOPE
#   with absolute paths to all generated files.
function(generate_users_examples _users_test_inputs _output_dir
    _all_generated_files_out)

  set(test_path ${Dakota_SOURCE_DIR}/test)
  set(all_generated_files)
  
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
      OUTPUT  "${_output_dir}/${test_output}.in"
      COMMAND "${PERL_EXECUTABLE}"
      ARGS    "${test_path}/dakota_test.perl" --extract "${test_path}/${test_input}.in" ${test_num} --file-extract="${_output_dir}/${test_output}.in"
      )
    list(APPEND all_generated_files 
      "${_output_dir}/${test_output}.in")
  
    # Copy all .sav files from source to binary tree, avoid glob to get deps
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.out.sav)
      add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.out.sav
        ${_output_dir}/${test_output}.out.sav)
      list(APPEND all_generated_files 
        ${_output_dir}/${test_output}.out.sav)
    endif()
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.stdout.sav)
      add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.stdout.sav
        ${_output_dir}/${test_output}.stdout.sav)
      list(APPEND all_generated_files 
        ${_output_dir}/${test_output}.stdout.sav)
    endif()
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.dat.sav)
      add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.dat.sav
        ${_output_dir}/${test_output}.dat.sav)
      list(APPEND all_generated_files 
        ${_output_dir}/${test_output}.dat.sav)
    endif()
  
  endforeach()

  # AMPL examples in Users_Interface.tex
  set(ampl_files dakota_ampl_fma.mod dakota_ampl_fma.nl dakota_ampl_fma.col
    dakota_ampl_fma.row dakota_ampl_tb.mod)
  foreach (file ${ampl_files})
    add_file_copy_command(${test_path}/${file}
      ${_output_dir}/${file})
    list(APPEND all_generated_files ${_output_dir}/${file})
  endforeach()

  set(${_all_generated_files_out} ${all_generated_files} PARENT_SCOPE)

endfunction()


# Generate a few stragglers to specified _output_dir
function(generate_users_examples_interfacing _output_dir
    _all_generated_files_out)

  # "Generic" script interface examples in Users_Interface.tex
  # Directories in which inputs are found
  set(generic_path "${Dakota_SOURCE_DIR}/dakota-examples/official/drivers/bash")
  add_file_copy_command("${generic_path}/dakota_rosenbrock.in"
    "${_output_dir}/dakota_rosenbrock.in")
  add_file_copy_command("${generic_path}/simulator_script.sh"
    "${_output_dir}/simulator_script.sh")
  add_file_copy_command("${generic_path}/templatedir/ros.template"
    "${_output_dir}/ros.template")
  add_custom_command(
    OUTPUT  "${_output_dir}/dprepro_usage"
    DEPENDS "${Dakota_SOURCE_DIR}/scripts/pyprepro/dprepro"
    COMMAND "${Python_EXECUTABLE}"
    ARGS    "${Dakota_SOURCE_DIR}/scripts/pyprepro/dprepro" --help > "${_output_dir}/dprepro_usage"
    )

  list(APPEND users_inputs_abs
    "${_output_dir}/dakota_rosenbrock.in"
    "${_output_dir}/simulator_script.sh"
    "${_output_dir}/dprepro_usage"
    "${_output_dir}/ros.template"
    )

  set(${_all_generated_files_out} ${users_inputs_abs} PARENT_SCOPE)

endfunction()
