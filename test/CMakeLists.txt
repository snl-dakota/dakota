# CMakeLists.txt for DAKOTA tests

include(AddApplicationTest)
include(AddFileCopyCommand)
include(CheckIncludeFile)
include(CMakeParseArguments)
include(DakotaCopyTarget)
include(ListFilterEmacsBackups)

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


# Add CTest tests for a single Dakota input file.  Will create a
# subdirectory for each file and either run all tests as a single
# dakota_test.perl, or optionally register and run subtests
# 0.._last_subtest in succession in that directory.
function(dakota_app_test _test_name _last_subtest)
  # Parse options
  set(_option_args PARALLEL)
  set(_multi_value_keyword_args FILE_DEPENDENCIES LABELS)
  cmake_parse_arguments(
    _dat
    "${_option_args}"
    "${_one_value_keyword_args}"
    "${_multi_value_keyword_args}"
    ${ARGN}
    )

  # Default values and parallel overrides
  set(_test_input_file "${_test_name}.in")
  set(_diffs_filename dakota_diffs.out)
  set(_par_clopt "")
  set(_par_mark "")
  if (_dat_PARALLEL)
    set(_test_name "p${_test_name}")
    set(_diffs_filename dakota_pdiffs.out)
    set(_par_clopt "--parallel")
    set(_par_mark "p")
  endif()
  
  # Circumvent SIP on OSX by running dakota and other test executables
  # with custom generated .sh scripts.
  set(binext "")
  if(APPLE)
    set(binext "--bin-ext=.sh")
  endif()

  if (DAKOTA_EXPAND_SUBTESTS)
    # Add the 0th serial test. Don't create custom target for copied files.
    # Create the unique work directory here.
    add_application_test(
      ${_test_name}
      SUBTEST 0
      FILE_DEPENDENCIES "${_dat_FILE_DEPENDENCIES}"
      PREPROCESS COMMAND \"${CMAKE_COMMAND}\" -E remove ${_diffs_filename}
      APPLICATION COMMAND \"${PERL_EXECUTABLE}\"
        ${CMAKE_CURRENT_BINARY_DIR}/dakota_test.perl ${_par_clopt}
          --output-dir=${CMAKE_CURRENT_BINARY_DIR}/${_test_name}
          --bin-dir=${CMAKE_CURRENT_BINARY_DIR}
          ${binext}
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
    	${_test_name}
  	SUBTEST ${st_num}
    	APPLICATION COMMAND \"${PERL_EXECUTABLE}\"
            ${CMAKE_CURRENT_BINARY_DIR}/dakota_test.perl ${_par_clopt}
              --output-dir=${CMAKE_CURRENT_BINARY_DIR}/${_test_name}
              --bin-dir=${CMAKE_CURRENT_BINARY_DIR}
              ${binext}
              ${test_input_file} ${st_num}
    	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_test_name}"
    	NO_TARGET
    	LABELS ${test_labels}
    	)
        math(EXPR prev_st "${st_num}-1")
        set_tests_properties("${_test_name}:${st_num}" PROPERTIES 
  	  DEPENDS "${_test_name}:${prev_st}")
      endforeach()
    endif()
  else()
    # Add a single test for the whole file. Don't create custom target
    # for copied files.
    add_application_test(
      ${_test_name}
      FILE_DEPENDENCIES "${_dat_FILE_DEPENDENCIES}"
      PREPROCESS COMMAND \"${CMAKE_COMMAND}\" -E remove ${_diffs_filename}
      APPLICATION COMMAND \"${PERL_EXECUTABLE}\"
        ${CMAKE_CURRENT_BINARY_DIR}/dakota_test.perl ${_par_clopt}
          --output-dir=${CMAKE_CURRENT_BINARY_DIR}/${_test_name}
          --bin-dir=${CMAKE_CURRENT_BINARY_DIR}
          ${binext}
          ${_test_input_file}
      UNIQUE_DIRECTORY
      NO_TARGET
      LABELS ${_dat_LABELS}
      )
  endif()
endfunction()


include_directories(
  ${Dakota_SOURCE_DIR}/src
  ${Boost_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${Teuchos_INCLUDE_DIRS}
)

remove_definitions("-DHAVE_CONFIG_H")

check_include_file(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
  add_definitions("-DHAVE_UNISTD_H")
endif(HAVE_UNISTD_H)

if(BUILD_TESTING AND DAKOTA_ENABLE_TESTS)

  # list of all destination files copied into test/ or subdirs test/dakota_*
  # so we can have a custom target depend on them
  set(copied_test_files_abs)

  # Configure or copy the dakota_test support files into the build
  # tree with dependency to source files (needed for legacy behavior,
  # but ultimately would like to eliminate)

  # configured primary scripts
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dakota_test.perl
    ${CMAKE_CURRENT_BINARY_DIR}/dakota_test.perl @ONLY)

  # copied primary scripts
  set(file "dakota_diff.perl")
  add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/${file} 
    ${CMAKE_CURRENT_BINARY_DIR}/${file})
  list(APPEND copied_test_files_abs "${CMAKE_CURRENT_BINARY_DIR}/${file}")  


  # Create one CTest per DAKOTA input file, in a protected subdirectory

  file(GLOB dakota_test_input_files 
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "dakota_*.in")
     
  # Generate the test properties to a file by parsing test input
  # files.  This can be slow, so do once, then parse the output.  Work
  # in the source tree to avoid having to manage fully-qualified paths
  # to binary tree.
  execute_process(COMMAND "${PERL_EXECUTABLE}" 
    "${CMAKE_CURRENT_SOURCE_DIR}/dakota_test.perl" 
    "--test-properties=${CMAKE_CURRENT_BINARY_DIR}" 
    "dakota_*.in" 
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE test_props_output  # Let errors go to the console
    RESULT_VARIABLE test_props_code
    )
  if ( (NOT test_props_code EQUAL 0) OR 
       (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/dakota_tests.props") )
    message(FATAL_ERROR "Could not get test properties.  Exit code: "
      "${test_props_code}; output:\n" ${test_props_output})
  endif()

  file(READ "${CMAKE_CURRENT_BINARY_DIR}/dakota_tests.props" test_props)

  # Iterate the list of test files and create tests for each
  foreach(test_input_file ${dakota_test_input_files})

    get_filename_component(test_name ${test_input_file} NAME_WE)

    # Glob up all files associated with this test input file
    # These get copied by add_application_test into test/dakota_*/
    file(GLOB test_all_files
      RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${test_name}.*" "${test_name}-*"
      )
    list_filter_emacs_backups(test_all_files)

    # TODO: apply properties on on a per-subtest basis

    # --- Add serial test ---
    # test properties may disable a specific test based on configuration
    set(add_this_test TRUE)
    set(last_subtest -1)
    set(test_labels DakotaTest SerialTest)
    set(test_req_files)  # files mentioned in input file ReqFiles=
    # updates above 4 variables
    dakota_get_test_properties("${test_name}" "serial" "${test_props}")
    if (add_this_test)

      # Add either one test with all, or the 0-th test, followed by all sub-tests
      dakota_app_test(${test_name} ${last_subtest}
	FILE_DEPENDENCIES "${test_all_files}" "${test_req_files}"
	LABELS ${test_labels})

      # the test add makes a copy command without target, now track all
      # the copied files for later addition to a custom target
      foreach(file ${test_all_files} ${test_req_files})
	set(copied_file "${CMAKE_CURRENT_BINARY_DIR}/${test_name}/${file}")
	list(APPEND copied_test_files_abs "${copied_file}")
      endforeach()

      # Also copy the aux files into test/ to support direct testing with Perl
      set(test_aux_files ${test_all_files})
      list(REMOVE_ITEM test_aux_files ${test_input_file})
      foreach(file ${test_aux_files} ${test_req_files})
	add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/${file} 
	  ${CMAKE_CURRENT_BINARY_DIR}/${file})
	list(APPEND copied_test_files_abs "${CMAKE_CURRENT_BINARY_DIR}/${file}")
      endforeach()

    endif()  # add_this_test

    # --- Add parallel test when MPI enabled, and file contains #p<int> ---
    if (DAKOTA_HAVE_MPI)

      # test properties may disable a specific test based on configuration
      set(add_this_test TRUE)
      set(last_subtest -1)
      set(test_labels DakotaTest ParallelTest)
      set(test_req_files)  # files mentioned in the input file ReqFiles=
      # updates above 4 variables
      dakota_get_test_properties("${test_name}" "parallel" "${test_props}")

      if (add_this_test)
	dakota_app_test(${test_name} ${last_subtest} PARALLEL 
	  FILE_DEPENDENCIES "${test_all_files}" "${test_req_files}"
	  LABELS ${test_labels}
	  )

	# track all the copied files for later addition to a custom target
	foreach(file ${test_all_files} ${test_req_files})
	  set(copied_file "${CMAKE_CURRENT_BINARY_DIR}/p${test_name}/${file}")
	  list(APPEND copied_test_files_abs "${copied_file}")
	endforeach()

	# This is necessary if files needed by a parallel test aren't
	# called out for a serial test, or the serial test is omitted
	# above.  However generates the same output from two copies;
	# revisit if causes problems.
	set(test_aux_files ${test_all_files})
	list(REMOVE_ITEM test_aux_files ${test_input_file})
	foreach(file ${test_aux_files} ${test_req_files})
	  add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/${file} 
	    ${CMAKE_CURRENT_BINARY_DIR}/${file})
	  list(APPEND copied_test_files_abs "${CMAKE_CURRENT_BINARY_DIR}/${file}")
	endforeach()
      endif()

    endif() # MPI enabled

    # TODO: The following regex needs to be based on output of dakota_diff.perl
    #       and account for consistency with baseline
    # set_tests_properties(${DAKOTA_SYSTEST} PROPERTIES 
    #  FAIL_REGULAR_EXPRESSION "fail")

  endforeach() # foreach test_input_file

  # Copy targets from src/ to test/ to facilitate testing.  We place
  # the rules for copying executables from src/ in the test/
  # directory, so they get rebuilt when make-ing in test/.  Previously
  # these were POST_BUILD actions on targets in src/.  CMake generator
  # expressions don't work in OUTPUTs, so use a manually specified
  # executable name with the CMake exe suffix
  set(test_exe_deps)
  DakotaCopyTarget(dakota test_exe_deps)
  DakotaCopyTarget(dakota_library_mode test_exe_deps)
  if(DAKOTA_HAVE_MPI)
    DakotaCopyTarget(dakota_library_split test_exe_deps)
  endif()
  if(DAKOTA_DLL_API)
    DakotaCopyTarget(dll_tester test_exe_deps)
  endif()

  # Create a single target to copy all the testing files
  add_custom_target(dakota_test_files ALL 
    DEPENDS ${copied_test_files_abs} ${test_exe_deps}
    COMMENT 
      "Copy DAKOTA test inputs, aux files, and executables to binary test/ and subdirectories"
    )

  # Clean up test dir
  #file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/dakota_diffs.out)

  add_custom_target(dakota-diffs-clean
    "${CMAKE_COMMAND}" -E remove dakota_diffs.out dakota_pdiffs.out
                          dakota_*/dakota_diffs.out pdakota_*/dakota_pdiffs.out
			  dakota_results.log
    COMMENT "Remove all dakota_[p]diffs.out files from test/ and subdirs."
    )

  add_custom_target(dakota-diffs
    "${CMAKE_COMMAND}"
      -D CMAKE_MODULE_PATH:STRING="${Dakota_SOURCE_DIR}/cmake/utilities"
      -D Dakota_BINARY_DIR:FILEPATH="${Dakota_BINARY_DIR}"
      -P "${Dakota_SOURCE_DIR}/cmake/utilities/DakotaConcatDiffs.cmake"
    COMMENT "Concatenate all dakota_[p]diffs.out files from test/ subdirs."
    )

endif() # BUILD_TESTING AND DAKOTA_ENABLE_TESTS


# Executables/drivers needed for DAKOTA system level testing
set(dakota_test_executables
  barnes
  barnes_lf
  cantilever
  container
  cyl_head
  expdesign_high
  expdesign_low
  gerstner_string
  herbie
  illumination
  lf_rosenbrock
  lf_poly_prod
  log_ratio
  mod_cantilever
  mogatest1
  mogatest2
  mogatest3
  morris
  poly_prod
  portal_frame
  quasi_sine_fcn
  rosenbrock
  rosenbrock_fail
  short_column
  shubert
  smooth_herbie
  sobol_g_function
  sobol_ishigami
  sobol_rational
  steel_column_cost
  steel_column_perf
  steel_section
  text_book
  text_book1
  text_book2
  text_book3
  text_book_ouu
  trajectory
  trajectory_post
  )

foreach(dte ${dakota_test_executables})
  add_executable(${dte} ${dte}.cpp)
endforeach()

# nl2func has two source files
list(APPEND dakota_test_executables nl2func)
add_executable(nl2func nl2func.cpp nl2test.cpp)

# parallel tester
if(DAKOTA_HAVE_MPI)
  add_executable(text_book_par text_book_par.cpp)
  list(APPEND dakota_test_executables text_book_par)
  DakotaApplyMPISettings(text_book_par)
endif()

# Enforce an "imaginary" dependency
add_dependencies(cantilever dakota)
add_dependencies(morris dakota)
add_dependencies(text_book dakota)

if(DAKOTA_ENABLE_TESTS AND PYTHONINTERP_FOUND)
  # Test restart translation capability.
  # (Example of system-level test that's not a Dakota study regression.)
  add_test(NAME sys_restart_neutral COMMAND ${PYTHON_EXECUTABLE} 
    ${CMAKE_CURRENT_SOURCE_DIR}/test_sys_restart_neutral.py 
    $<TARGET_FILE:dakota> $<TARGET_FILE:dakota_restart_util>
    )
endif()

# If needed, copy files from test/Debug or test/Release into test/.
# Also temporary workaround until Dakota can properly detect .exe as
# analysis driver: copy the .exe to a file with no extension. These
# files will be generated to the build tree, but not installed.
if (WIN32 AND ${CMAKE_GENERATOR} MATCHES "Visual Studio")
  foreach(dte ${dakota_test_executables})
    # Hang a custom command off the test executable targets
    add_custom_command(TARGET ${dte} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${dte}>" 
                                         "${CMAKE_CURRENT_BINARY_DIR}/${dte}"
      DEPENDS "$<TARGET_FILE:${dte}>"
      )
    add_custom_command(TARGET ${dte} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${dte}>"
                                         "${CMAKE_CURRENT_BINARY_DIR}/${dte}.exe"
      DEPENDS "$<TARGET_FILE:${dte}>"
      )
  endforeach()
endif()

# TODO: install to bin/?
install(TARGETS ${dakota_test_executables} 
  DESTINATION "${DAKOTA_TEST_INSTALL}/test")

install(FILES dakota_test.perl
  DESTINATION "${DAKOTA_TEST_INSTALL}/test" PERMISSIONS
  OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(FILES ${test_support_files}
  DESTINATION "${DAKOTA_TEST_INSTALL}/test" PERMISSIONS
  OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
