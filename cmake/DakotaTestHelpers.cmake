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
