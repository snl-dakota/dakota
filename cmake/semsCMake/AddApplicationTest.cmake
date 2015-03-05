# Add an application level test.
#
# An application level test consists of a preprocessing step, running
# the application itself, and a postprocessing step. In addition, file
# dependencies can be specified and an overall job weight can be
# specified. The function should be used as follows:
#
# add_application_test(
#   <test-name>
#   [SUBTEST <st_num>]
#   [WEIGHT <n>]
#   [TIMEOUT <seconds>]
#   [FILE_DEPENDENCIES file1 [file2 [file3[...]]]]
#   [PREPROCESS COMMAND <command> [EXIT_CODE <exit-code>]] 
#   [APPLICATION COMMAND <command> [EXIT_CODE <exit-code>]] 
#   [POSTPROCESS COMMAND <command> [EXIT_CODE <exit-code>]] 
#   [LABELS [label1 [label2 [...]]]]
#   [UNIQUE_DIRECTORY]
#   [WORKING_DIRECTORY <work-dir>]
#   [NO_TARGET]
#   )
#
# SUBTEST gives a subtest number which changes added test name to
# test-name:st_num, but runs in directory test-name
#
# The WEIGHT argument specifies the number of processes that will be occupied
# by the test. The TIMEOUT argument specifies how long the test should be
# allowed to run in seconds. The FILE_DEPENDENCIES argument specifies any input
# files the application test requires. These files will be copied to the test
# execution directory at build time. The PREPROCESS, APPLICATION, and
# POSTPROCESS arguments specify a preprocessing step, how to run the
# application itself and any post processing steps that are required. Finally,
# the LABELS argument specifies any labels to attach to the test.
#
# The UNIQUE_DIRECTORY toggle will ensure that the test runs in an
# isolated subdirectory of the current binary directory, named for the
# test-name.  The directory will be created.
# 
# WORKING_DIRECTORY will run the test in the specified directory,
# which must exist.
# 
# If specified, the NO_TARGET toggle will create a custom command for
# this test, but not a custom target (so the caller can group tests
# into a single custom target)

include(AddFileCopyTarget)
include(CMakeParseArguments)

get_filename_component(_add_application_test_dir ${CMAKE_CURRENT_LIST_FILE} PATH)

function(add_application_test _test_name)
  set(_option_args UNIQUE_DIRECTORY NO_TARGET)
  set(_one_value_keyword_args 
    WEIGHT
    TIMEOUT
    SUBTEST
    WORKING_DIRECTORY)
  set(_multi_value_keyword_args
    FILE_DEPENDENCIES
    PREPROCESS
    APPLICATION 
    POSTPROCESS
    LABELS)

  cmake_parse_arguments(
    _application_test
    "${_option_args}"
    "${_one_value_keyword_args}"
    "${_multi_value_keyword_args}"
    ${ARGN})

  # Setup default values for the arguments.
  if(NOT _application_test_WEIGHT)
    set(_application_test_WEIGHT 1)
  endif()
  if(NOT _application_test_TIMEOUT)
    set(_application_test_TIMEOUT 0)
  endif()
  if(NOT _application_test_WORKING_DIRECTORY)
    set(_application_test_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  set(_full_test_name ${_test_name})
  if(DEFINED _application_test_SUBTEST)
    set(_full_test_name "${_test_name}:${_application_test_SUBTEST}")
  endif()

  # If an isolated subdirectory was requested, create the unique subdirectory
  # to contain the test's input files.
  if(_application_test_UNIQUE_DIRECTORY)
    set(_application_test_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_test_name}")
    file(MAKE_DIRECTORY "${_application_test_WORKING_DIRECTORY}")
  endif()

  # Add targets to copy all of the test dependencies to the working
  # directory at build time.
  foreach(_file ${_application_test_FILE_DEPENDENCIES}) 
    get_filename_component(_filename ${_file} NAME)

    # Add custom command and optional target
    if(_application_test_NO_TARGET)
      add_file_copy_command(
	${CMAKE_CURRENT_SOURCE_DIR}/${_file}
	${_application_test_WORKING_DIRECTORY}/${_file}
	)
    else()
      add_file_copy_target(
	copy_${_full_test_name}_${_filename}
	${CMAKE_CURRENT_SOURCE_DIR}/${_file}
	${_application_test_WORKING_DIRECTORY}/${_file})
    endif()

  endforeach() # file in dependencies

  # Parse each of the PREPROCESS, APPLICATION, POSTPROCESS argument lists.
  set(_phases PREPROCESS APPLICATION POSTPROCESS)
  foreach(_phase ${_phases})
    cmake_parse_arguments(
      _application_test_${_phase}
      ""
      "EXIT_CODE"
      "COMMAND"
      "${_application_test_${_phase}}")
    if(NOT _application_test_${_phase}_EXIT_CODE)
      set(_application_test_${_phase}_EXIT_CODE 0)
    endif()
  endforeach()

  # Configure the driver script.
  set(_test_driver_file ${_application_test_WORKING_DIRECTORY}/TEST-${_full_test_name}.cmake)
  configure_file(
    ${_add_application_test_dir}/ApplicationTest.cmake.in
    ${_test_driver_file}
    @ONLY)
  
  # Add the test that runs the driver.
  add_test(NAME ${_full_test_name}
    WORKING_DIRECTORY "${_application_test_WORKING_DIRECTORY}"
    COMMAND ${CMAKE_COMMAND} -P ${_test_driver_file})
  
  # Set up the test properties.
  set_tests_properties( 
    ${_full_test_name}
    PROPERTIES
      PROCESSORS ${_application_test_WEIGHT}
      TIMEOUT ${_application_test_TIMEOUT}
      LABELS "${_application_test_LABELS}")
    

endfunction()
