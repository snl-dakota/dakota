#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

include(ListFilterEmacsBackups)


# Generate the test properties for ${src_dir}/dakota_*.in to a file
# ${bin_dir}/dakota_tests.props by parsing test input files. Output
# the properties to the specified props_output_var in parent scope.
#
# This can be slow, so do once per directory, then parse the output. 
function(dakota_generate_test_properties src_dir bin_dir props_output_var)

  # Work in the source tree to avoid having to manage fully-qualified
  # paths to binary tree.
  execute_process(COMMAND "${PERL_EXECUTABLE}" 
    "${Dakota_SOURCE_DIR}/test/dakota_test.perl" 
    "--test-properties=${bin_dir}" 
    "dakota_*.in" 
    WORKING_DIRECTORY "${src_dir}"
    OUTPUT_VARIABLE test_props_output  # Let errors go to the console
    RESULT_VARIABLE test_props_code
    )
  if ( (NOT test_props_code EQUAL 0) OR 
       (NOT EXISTS "${bin_dir}/dakota_tests.props") )
    message(FATAL_ERROR "Could not get test properties.  Exit code: "
      "${test_props_code}; output:\n" ${test_props_output})
  endif()

  file(READ "${bin_dir}/dakota_tests.props" test_props)

  set(${props_output_var} ${test_props} PARENT_SCOPE)

endfunction()


# Glob up all files associated with a test input file ${test_name}.in,
# filtering out emacs tmp files. Set parent variables with all
# (includes the input file) and aux (omits the input file) file lists
function(dakota_test_dependent_files src_dir test_name
  all_files_output_var aux_files_output_var)

    file(GLOB test_all_files
      RELATIVE ${src_dir} "${test_name}.*" "${test_name}-*"
      )
    list_filter_emacs_backups(test_all_files)

    # All files associated with the test, less the input file itself
    set(test_aux_files ${test_all_files})
    list(REMOVE_ITEM test_aux_files "${test_name}.in")

    set(${all_files_output_var} ${test_all_files} PARENT_SCOPE)
    set(${aux_files_output_var} ${test_aux_files} PARENT_SCOPE)

endfunction()


# Add suite_name regression tests for any dakota_*.in input files
# found in ${src_dir}. Test them in a protected subdir
# ${bin_dir}/[p]dakota_*/, e.g., where
# bin_dir=CMAKE_CURRENT_BINARY_DIR
function(dakota_add_regression_tests suite_name src_dir bin_dir)

  # examples
  # dir_rel_to_src = source/test
  #                = build/_deps/official/bayes_calibration/multiqoi 

  # glob test files dakota_*.in relative to specified source dir
  file(GLOB dakota_test_input_files RELATIVE ${src_dir} "dakota_*.in")

  # generate test properties for the directory to suite_name_props
  dakota_generate_test_properties(${src_dir} ${bin_dir} ${suite_name}_props)

  # determine all associated files to copy and keep a list for build-time deps
  # publish to PARENT_SCOPE suite_name_deps

  # need option for whether to configure vs. copy the input file
  
  # foreach input file, add test with dakota_app_test

endfunction()


# Find input files in examples repo and make corresponding directories
# in build tree.
function(parse_examples_repo repo_root)

  foreach(example_repo_dir)
    # Populate with RWH code to
    # make build tree dir
    # generate serial and parallel tests specifying src_dir, bin_dir
    # aggregate all dep files
  endforeach()

  # generate all dep files for examples repo

endfunction()
