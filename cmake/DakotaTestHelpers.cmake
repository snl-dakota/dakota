#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2023
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

include(AddApplicationTest)
include(CMakeParseArguments)
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


# Macro: dakota_get_test_properties; macro so can set in parent context.
#
# Input:
#   test_name (without .in extension), "serial" or "parallel",
#   variable containing all test properties to parse
# Output:
#   add_this_test = FALSE if test disallowed by DakotaConfig
#   last_test: last subtest found, -1 if none
#   test_labels: appended with any labels from the test file
#   test_req_files: auxilliary files this test requires
macro(dakota_get_test_properties test_name serpar_str test_props)
  # TODO: strip whitespace from parsed values?

  if (test_props)
    #message("DEBUG: ${test_props} ")

    # DakotaConfig: required configuration for this test
    string(REGEX MATCH
      "${test_name}:${serpar_str}: DakotaConfig=[A-Za-z0-9_,]+"
      dc_match ${test_props}
      )
    if (dc_match)
      string(REPLACE "${test_name}:${serpar_str}: DakotaConfig=" ""
	test_props_dc ${dc_match})
      # Get tokens by comma
      string(REGEX MATCHALL "[A-Za-z0-9_]+" prop_dc_list ${test_props_dc})
      foreach (req_config ${prop_dc_list})
	# if this CMake variable is not true, omit the test
	if (NOT "${${req_config}}")
	  message(STATUS
	    "Omitting test ${test_name}, ${serpar_str}; requires config: "
	    "${req_config}")
	  set(add_this_test FALSE)
	endif()
      endforeach()
    endif()

    # add CTest labels
    if (serpar_str STREQUAL "serial")
      list(APPEND test_labels SerialTest)
    elseif (serpar_str STREQUAL "parallel")
      list(APPEND test_labels ParallelTest)
    endif()
    string(REGEX MATCH
      "${test_name}:${serpar_str}: Label=[A-Za-z0-9_,]+"
      label_match ${test_props})
    if (label_match)
      string(REPLACE "${test_name}:${serpar_str}: Label=" ""
	test_props_labels ${label_match})
      # Get tokens by comma
      string(REGEX MATCHALL "[A-Za-z0-9_]+" props_labels_list
	${test_props_labels})
      list(APPEND test_labels "${props_labels_list}")
    endif()

    # test dependency files
    string(REGEX MATCH
      "${test_name}:${serpar_str}: ReqFiles=[A-Za-z0-9_,\\.]+"
      rf_match ${test_props})
    if (rf_match)
      string(REPLACE "${test_name}:${serpar_str}: ReqFiles=" ""
	test_props_rf ${rf_match})
      # Get tokens by comma
      string(REGEX MATCHALL "[A-Za-z0-9_\\.]+" props_rf_list
	${test_props_rf})
      # TODO: strip whitespace?
      foreach (req_file ${props_rf_list})
	list(APPEND test_req_files "${req_file}")
      endforeach()
    endif()

    # TODO: Could also consider use of
    # WILL_FAIL property on individual tests.

    # Find the number of the highest serial and parallel test.  Set
    # last_subtest as output variable.
    string(REGEX MATCH "${test_name}:${serpar_str}: Count=(-?[0-9]+)"
      count_match ${test_props})
    if (count_match)
      set(last_subtest ${CMAKE_MATCH_1})
      if(last_subtest LESS 0)
	#message(STATUS
	#  "Omitting test ${test_name}, ${serpar_str}; no subtests found")
	set(add_this_test FALSE)
      endif()
    endif()

  endif()
endmacro()


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


# Add CTest tests for a single Dakota input file.  Will create a
# subdirectory for each file and either run all tests as a single
# dakota_test.perl, or optionally register and run subtests
# 0.._last_subtest in succession in that directory.
function(dakota_app_test _input_name _last_subtest)
  # Parse options
  set(_option_args SERIAL PARALLEL) # SERIAL default if not specified
  set(_one_value_keyword_args BIN_DIR NAME_PREFIX)
  set(_multi_value_keyword_args FILE_DEPENDENCIES CONFIGURE_FILES LABELS)
  cmake_parse_arguments(
    _dat
    "${_option_args}"
    "${_one_value_keyword_args}"
    "${_multi_value_keyword_args}"
    ${ARGN}
    )

  # Default values and parallel overrides
  set(_test_input_file "${_input_name}.in")
  set(_ctest_name "${_dat_NAME_PREFIX}${_input_name}")
  set(_diffs_filename dakota_diffs.out)
  set(_par_clopt "")
  set(_par_mark "")
  if (_dat_PARALLEL)
    set(_ctest_name "${_dat_NAME_PREFIX}p${_input_name}")
    set(_diffs_filename dakota_pdiffs.out)
    set(_par_clopt "--parallel")
    set(_par_mark "p")
  endif()

  set(bin_dir ${CMAKE_CURRENT_BINARY_DIR})
  if (_dat_BIN_DIR)
    set(bin_dir ${_dat_BIN_DIR})
  endif()

  # Circumvent SIP on OSX by running dakota and other test executables
  # with custom generated .sh scripts.
##  set(binext "")
##  if(APPLE)
##    set(binext "--bin-ext=.sh")
##  endif()

  if (DAKOTA_EXPAND_SUBTESTS)
    # Add the 0th serial test. Don't create custom target for copied files.
    # Create the unique work directory here.
    add_application_test(
      ${_ctest_name}
      SUBTEST 0
      FILE_DEPENDENCIES "${_dat_FILE_DEPENDENCIES}"
      CONFIGURE_FILES "${_dat_CONFIGURE_FILES}"
      PREPROCESS COMMAND ${CMAKE_COMMAND} -E remove ${_diffs_filename}
      APPLICATION COMMAND ${PERL_EXECUTABLE}
        ${CMAKE_CURRENT_BINARY_DIR}/dakota_test.perl ${_par_clopt}
          --output-dir=${CMAKE_CURRENT_BINARY_DIR}/${_ctest_name}
          --bin-dir=${bin_dir}
##          ${binext}
          ${_test_input_file} 0
      UNIQUE_DIRECTORY
      NO_TARGET
      LABELS ${_dat_LABELS}
      )
    # Now add any subtests in the file, with dep on previous number,
    # running in same directory
    if (_last_subtest GREATER 0)
      foreach(st_num RANGE 1 ${_last_subtest})
        add_application_test(
    	${_ctest_name}
  	SUBTEST ${st_num}
	APPLICATION COMMAND ${PERL_EXECUTABLE}
            ${CMAKE_CURRENT_BINARY_DIR}/dakota_test.perl ${_par_clopt}
              --output-dir=${CMAKE_CURRENT_BINARY_DIR}/${_ctest_name}
              --bin-dir=${bin_dir}
##              ${binext}
              ${_test_input_file} ${st_num}
    	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_ctest_name}"
    	NO_TARGET
    	LABELS ${_dat_labels}
    	)
        math(EXPR prev_st "${st_num}-1")
        set_tests_properties("${_input_name}:${st_num}" PROPERTIES
  	  DEPENDS "${_input_name}:${prev_st}")
      endforeach()
    endif()
  else()
    # Add a single test for the whole file. Don't create custom target
    # for copied files.
    add_application_test(
      ${_ctest_name}
      FILE_DEPENDENCIES "${_dat_FILE_DEPENDENCIES}"
      CONFIGURE_FILES "${_dat_CONFIGURE_FILES}"
      PREPROCESS COMMAND ${CMAKE_COMMAND} -E remove ${_diffs_filename}
      APPLICATION COMMAND ${PERL_EXECUTABLE}
        ${CMAKE_CURRENT_BINARY_DIR}/dakota_test.perl ${_par_clopt}
          --output-dir=${CMAKE_CURRENT_BINARY_DIR}/${_ctest_name}
          --bin-dir=${bin_dir}
##          ${binext}
          ./${_test_input_file}
      UNIQUE_DIRECTORY
      NO_TARGET
      LABELS ${_dat_LABELS}
      )
  endif()
endfunction()


# Get properties for serial or parallel test and conditionally add it
# test_name: input_file name w/o extension
# TODO: for examples repo will need more general test names to avoid conflict
# Relys on:
#   dakota_test_configured_inputs in parent scope
#   test_copied_files in parent scope
function(dakota_regression_test test_name serpar_string test_props
    test_aux_files)

   # test properties may disable a specific test based on configuration
   set(add_this_test TRUE)
   set(last_subtest -1)
   set(test_labels Regression)
   set(test_req_files)  # files mentioned in input file ReqFiles=

   # updates above 4 variables
   dakota_get_test_properties("${test_name}" "${serpar_string}" "${test_props}")
   # TODO: apply properties on on a per-subtest basis

   if (add_this_test)

     # Add either one test with all, or the 0-th test, followed by all sub-tests
     string(TOUPPER "${serpar_string}" upper_serpar)
     if(${test_name} IN_LIST dakota_test_configured_inputs)
       dakota_app_test(${test_name} ${last_subtest} ${upper_serpar}
	 CONFIGURE_FILES "${test_name}.in"
	 FILE_DEPENDENCIES "${test_aux_files}" "${test_req_files}"
	 LABELS ${test_labels}
	 )
     else()
       dakota_app_test(${test_name} ${last_subtest} ${upper_serpar}
	 FILE_DEPENDENCIES
	 "${test_name}.in" "${test_aux_files}" "${test_req_files}"
	 LABELS ${test_labels}
	 )
     endif()

     set(_par_mark "")
     if (serpar_string STREQUAL "parallel")
       set(_par_mark "p")
     endif()

     # dakota_app_test makes a copy command without target, now track all
     # the copied files for later addition to a custom target
     set(copied_files_abs)
     foreach(file ${test_copied_files} ${test_req_files})
       set(copied_file
	 "${CMAKE_CURRENT_BINARY_DIR}/${_par_mark}${test_name}/${file}")
       list(APPEND copied_files_abs "${copied_file}")
     endforeach()

    set_tests_properties(${_par_mark}${test_name} PROPERTIES
      ENVIRONMENT "PYTHONPATH=${DAKOTA_PYTHON_PATH}:$ENV{PYTHONPATH}")

   endif()

   set(copied_test_files_abs ${copied_test_files_abs} ${copied_files_abs}
     PARENT_SCOPE)
   # TODO: promote this to parent to avoid clunky data flow
   set(test_all_req_files ${test_all_req_files} ${test_req_files} PARENT_SCOPE)

endfunction()


function(dakota_check_input suite_name input_name_we)
  add_test(NAME "${suite_name}-${input_name_we}-check"
   COMMAND "$<TARGET_FILE:dakota>" -check -input
    "${CMAKE_CURRENT_BINARY_DIR}/${suite_name}-${input_name}/${input_name_we}.in"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${suite_name}-${input_name}"
  )
  set_tests_properties( "${suite_name}-${input_name_we}-check" PROPERTIES LABELS "DakotaExamplesRepo")
endfunction()


# Add suite_name regression tests for any dakota_*.in input files
# found in ${src_dir}. Test them in a protected subdir
# $CMAKE_CURRENT_BINARY_DIR}/${suite_name}[p]dakota_*/
#
# Examples:
#   suite_name = official-bayes_calibration-multi_qoi-
#   src_dir = build/_deps/official/bayes_calibration/multiqoi
#
#   suite_name = official-centered_parameter_study
#   src_dir = "${Dakota_BINARY_DIR}/_deps/dakota_examples_repos-src/official/centered_parameter_study"
#
function(dakota_add_regression_tests suite_name src_dir)

  # glob test files dakota_*.in relative to specified source dir
  file(GLOB dakota_test_input_files RELATIVE ${src_dir}
    "${src_dir}/*.in")

  # generate test properties for the directory to suite_name_props
  #dakota_generate_test_properties(${src_dir} ${bin_dir} ${suite_name}_props)

  # conservatively assume all files are needed for each test
  file(GLOB dakota_test_all_files "${src_dir}/*")

  # workaround for dprepro for now; will aggressively copy these files
  # into all test dirs. Need to understand extent of dependencies and
  # cleanup throughout testing.
  list(APPEND dakota_test_all_files
    ${Dakota_SOURCE_DIR}/scripts/pyprepro/dprepro)

  # Iterate the list of test files and create tests for each
  set(suite_copied_files)
  foreach(test_input_file ${dakota_test_input_files})

    get_filename_component(input_name ${test_input_file} NAME_WE)

    # Basic test to validate the input file
    dakota_check_input("${suite_name}" "${input_name}")

    set(last_subtest 0)
    dakota_app_test(${input_name} ${last_subtest} SERIAL
      NAME_PREFIX "${suite_name}-"
      BIN_DIR "${Dakota_BINARY_DIR}/test"
      # Can't use since AddApplicationTest assumes current src dir
      # FILE_DEPENDENCIES "${dakota_test_all_files}"
      LABELS "DakotaExamplesRepo"
      )

    # absolute paths to generated files for adding to a parent target
    foreach(src_file_fq ${dakota_test_all_files})
      get_filename_component(file ${src_file_fq} NAME)
      # Assumes serial test only:
      set(dest_file_fq
	"${CMAKE_CURRENT_BINARY_DIR}/${suite_name}-${input_name}/${file}")
      add_file_copy_command("${src_file_fq}" "${dest_file_fq}")
      file(COPY
        "${Dakota_SOURCE_DIR}/interfaces/Python/dakota"
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${suite_name}-${input_name})
      list(APPEND suite_copied_files "${dest_file_fq}")
    endforeach()

  endforeach()

  set(${suite_name}_copied_files_abs ${suite_copied_files} PARENT_SCOPE)
  
endfunction()
