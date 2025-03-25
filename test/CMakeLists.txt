#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2024
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

# CMakeLists.txt for DAKOTA tests

include(AddFileCopyCommand)
include(DakotaCopyTarget)
include(DakotaTestHelpers)
include(ModifyTargetRpath)

# Configured input files: These files will be configured into the
# build tree instead of being copied. Macro is currently limited to
# input files themselves since it meets current use cases. NOTE: leave
# .in extension off the names here:
set(dakota_test_configured_inputs
  dakota_batch
  dakota_workdir
  dakota_workdir_windows
  dakota_json_io
  dakota_json_di
  )

include_directories(
  ${Dakota_SOURCE_DIR}/src
  ${Teuchos_INCLUDE_DIRS}
)

remove_definitions("-DHAVE_CONFIG_H")

# Generate regression tests for all dakota_*.in in this directory
if(BUILD_TESTING AND DAKOTA_ENABLE_TESTS)

  if(NOT DAKOTA_PYTHON_PATH)
    set(DAKOTA_PYTHON_PATH "${Dakota_SOURCE_DIR}/interfaces/Python")
  endif()

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


  # Create one CTest per Dakota input file, each in a protected subdirectory

  file(GLOB dakota_test_input_files 
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "dakota_*.in")

  # Parse dakota_*.in to generate test properties to file and test_props var
  dakota_generate_test_properties(${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR} test_props)

  # Iterate the list of test files and create tests for each
  foreach(test_input_file ${dakota_test_input_files})

    get_filename_component(test_name ${test_input_file} NAME_WE)

    # Glob up all files associated with this test input file
    # These get copied by add_application_test into test/dakota_*/
    dakota_test_dependent_files(${CMAKE_CURRENT_SOURCE_DIR} ${test_name}
      test_all_files test_aux_files)

    # Copy (or configure) ALL test-associated files into test/ to
    # support direct testing with Perl
    # TODO: consider a property to specify whether it's configured...
    # TODO: should these include ReqFiles specified in the dakota_*.in
    if(${test_name} IN_LIST dakota_test_configured_inputs)
      set(test_copied_files ${test_aux_files})
      configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${test_input_file}
	${CMAKE_CURRENT_BINARY_DIR}/${test_input_file} @ONLY)
    else()
      set(test_copied_files ${test_all_files})
    endif()

    # All required aux files parsed from test properties
    set(test_all_req_files)

    # --- Add serial test ---
    dakota_regression_test("${test_name}" "serial" "${test_props}"
      "${test_aux_files}")

    # --- Add parallel test when MPI enabled, and file contains #p<int> ---
    if (DAKOTA_HAVE_MPI)
      dakota_regression_test("${test_name}" "parallel" "${test_props}"
	"${test_aux_files}")
    endif()

    # Copy ALL test-associated files into bindir/test/ to
    # support direct testing with Perl
    list(APPEND test_copied_files ${test_all_req_files})
    list(REMOVE_DUPLICATES test_copied_files)
    foreach(file ${test_copied_files})
      add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/${file}
	${CMAKE_CURRENT_BINARY_DIR}/${file})
      list(APPEND copied_test_files_abs "${CMAKE_CURRENT_BINARY_DIR}/${file}")
    endforeach()

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
      -D CMAKE_MODULE_PATH:STRING="${Dakota_SOURCE_DIR}/cmake"
      -D Dakota_BINARY_DIR:FILEPATH="${Dakota_BINARY_DIR}"
      -P "${Dakota_SOURCE_DIR}/cmake/DakotaConcatDiffs.cmake"
    COMMENT "Concatenate all dakota_[p]diffs.out files from test/ subdirs."
    )

  # This gets built in test/ so can be packaged in binaries even when
  # not building docs
  add_subdirectory(examples-users)


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
  gerstner_int
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
  svanberg
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
  ModifyTargetRpath(${dte} 3)
endforeach()

target_link_libraries(morris Boost::boost)
target_link_libraries(trajectory_post Boost::boost)

# nl2func has two source files
list(APPEND dakota_test_executables nl2func)
add_executable(nl2func nl2func.cpp nl2test.cpp)
ModifyTargetRpath(nl2func 3)

# parallel tester
if(DAKOTA_HAVE_MPI)
  add_executable(text_book_par text_book_par.cpp)
  list(APPEND dakota_test_executables text_book_par)
  DakotaApplyMPISettings(text_book_par)
  ModifyTargetRpath(text_book_par 3)
endif()

# Enforce an "imaginary" dependency
add_dependencies(cantilever dakota)
add_dependencies(morris dakota)
add_dependencies(text_book dakota)

if(DAKOTA_PYTHON)
  # Test restart translation capability.
  # (Example of system-level test that's not a Dakota study regression.)
  add_test(NAME sys_restart_neutral COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/test_sys_restart_neutral.py 
    $<TARGET_FILE:dakota> $<TARGET_FILE:dakota_restart_util>
    )
  set_property(TEST sys_restart_neutral PROPERTY LABELS Unit Python)
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
