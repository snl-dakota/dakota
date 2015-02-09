# CMakeLists.txt for DAKOTA tests

# Macro: DakotaCopyTarget
# Copy the executable given by target_name to the current binary dir
# and add to provided list of dependencies
macro(DakotaCopyTarget target_name deplist_varname)
  set(executable_name ${target_name}${CMAKE_EXECUTABLE_SUFFIX})
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${executable_name}
    COMMAND ${CMAKE_COMMAND} 
      -E copy $<TARGET_FILE:${target_name}> ${CMAKE_CURRENT_BINARY_DIR}/
    # Since we pass it a target name, DEPENDS will create a file-level 
    # dependency so this command gets run whenever the executable is rebuilt.
    DEPENDS ${target_name}
    COMMENT "Copying ${executable_name} to ${CMAKE_CURRENT_BINARY_DIR}/"
  )
  list(APPEND ${deplist_varname}
    ${CMAKE_CURRENT_BINARY_DIR}/${executable_name})
endmacro()


include_directories(
  ${Dakota_SOURCE_DIR}/src
  ${Boost_INCLUDE_DIRS}
  ${Teuchos_INCLUDE_DIRS}
)

include(AddApplicationTest)
include(AddFileCopyCommand)
include(CheckIncludeFile)

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

  # Agressively omit tests for partial distributions to internal and
  # external web.  Initial set is aggressive, omitting any test that
  # induces FAIL due to DOT, NPSOL, or NLPQL.  Later will want to do
  # on a per test number basis.  These are conditional on TPL
  # presence, not distribution type to make it easier for
  # non-developers to run the tests.  Could also consider use of
  # WILL_FAIL property on individual tests.

  # Tests that fail due to missing DOT TPL
  if(NOT HAVE_DOT)
    list(REMOVE_ITEM dakota_test_input_files
      dakota_3pc.in # SHOULD BE ABLE TO FIX
      dakota_cantilever.in  # REVIEW
      dakota_cyl_head.in # REVIEW
      dakota_illum.in # REVIEW
      dakota_multiobj1.in # REVIEW
      dakota_multistart.in # REVIEW
      dakota_ouu1_cantilever.in
      dakota_ouu1_tbch.in 
      dakota_pareto.in # REVIEW
      dakota_pcbdo_cantilever_mixed.in
      dakota_rbdo_cantilever.in
      dakota_rbdo_cantilever_analytic.in
      dakota_rbdo_cantilever_analytic2.in
      dakota_rbdo_cantilever_mixed.in
      dakota_rbdo_cantilever_trsb2.in
      dakota_rbdo_short_column.in
      dakota_rbdo_short_column_analytic.in
      dakota_rbdo_short_column_analytic2.in
      dakota_rbdo_short_column_trsb.in
      dakota_rbdo_short_column_trsb2.in
      dakota_rbdo_steel_column.in
      dakota_rbdo_steel_column_analytic2.in
      dakota_rosenbrock.in # REVIEW
      dakota_sbnls_rosen.in # REVIEW
      dakota_sbouu3_cantilever.in
      dakota_sbouu3_tbch.in
      dakota_scaling.in  # SHOULD BE ABLE TO FIX
      dakota_scbdo_cantilever_mixed.in
      dakota_textbook.in # REVIEW
      dakota_users_qsf_multistart_strat.in # REVIEW
      dakota_users_textbook_pareto_strat.in # REVIEW
      dakota_var_views_opt.in      # SHOULD fix
      )
  endif()
  # Tests that fail due to missing NPSOL or NLPQL TPL
  if (NOT HAVE_NPSOL OR NOT HAVE_NLPQL)
    list(REMOVE_ITEM dakota_test_input_files
      dakota_addtnl_users.in
      dakota_ampl_diet.in
      dakota_ampl_fma.in
      dakota_ampl_tb.in
      dakota_container.in
      dakota_drivers.in
      dakota_logratio_taylor2.in
      dakota_multiobj2.in
      dakota_ouu1_tb.in
      dakota_pareto_pcbdo_short_column.in
      dakota_pcbdo_cantilever.in
      dakota_pcbdo_cantilever_mf.in
      dakota_pcbdo_cantilever_trsb.in
      dakota_pcbdo_rosenbrock.in
      dakota_pcbdo_rosenbrock_mf.in
      dakota_pcbdo_rosenbrock_trsb.in
      dakota_pcbdo_short_column.in
      dakota_pcbdo_short_column_mf.in
      dakota_pcbdo_short_column_trsb.in
      dakota_pcbdo_steel_column.in
      dakota_pcbdo_steel_column_mf.in
      dakota_pcbdo_steel_column_trsb.in
      dakota_rbdo_cantilever_analytic2_verify.in
      dakota_rbdo_cantilever_trsb.in
      dakota_rbdo_side_impact.in
      dakota_rbdo_steel_column_trsb2.in
      dakota_rosenbrock_fail.in
      dakota_sbo_barnes.in
      dakota_sbo_barnes_mf.in
      dakota_sbo_hier_poly_prod.in
      dakota_sbo_hierarchical.in
      dakota_sbouu2_cantilever.in
      dakota_sbouu2_tbch.in
      dakota_sbouu4_cantilever.in
      dakota_sbouu4_tbch.in
      dakota_scbdo_cantilever.in
      dakota_scbdo_cantilever_mf.in
      dakota_scbdo_cantilever_trsb.in
      dakota_scbdo_rosenbrock.in
      dakota_scbdo_rosenbrock_mf.in
      dakota_scbdo_rosenbrock_trsb.in
      dakota_scbdo_short_column.in
      dakota_scbdo_short_column_mf.in
      dakota_scbdo_short_column_trsb.in
      dakota_scbdo_steel_column.in
      dakota_scbdo_steel_column_mf.in
      dakota_scbdo_steel_column_trsb.in
      dakota_sebdo_short_column_adapt.in
      dakota_textbook_nips.in
      dakota_trsbouu2_cantilever.in
      dakota_trsbouu2_tbch.in
      dakota_trsbouu4_cantilever.in
      dakota_trsbouu4_tbch.in
      dakota_uq_cantilever_interval.in
      dakota_uq_cantilever_taylor2.in
      dakota_uq_textbook_dste.in
      dakota_users_cantilever_opt_npsol.in
      dakota_users_container_opt_npsol.in
      dakota_users_cylhead_opt_npsol.in
      dakota_users_textbook_opt_multiobj1.in
      dakota_users_textbook_opt_ouu1.in
      dakota_var_views_uq.in
      )
  endif()

  if(NOT HAVE_DREAM)
    list(REMOVE_ITEM dakota_test_input_files
    dakota_cantilever_dream.in 
    )
  endif()

  if(NOT HAVE_QUESO)
    list(REMOVE_ITEM dakota_test_input_files
    dakota_cantilever_queso.in 
    )
  endif()


  # Remove tests that fail on Windows (for now); the aborts could be
  # managed via gold standards if they don't pop a Windows error dialog
  if (WIN32)
    list(REMOVE_ITEM dakota_test_input_files
      dakota_3pc.in                         # abort: Unix shell
      dakota_container.in                   # abort: Unix shell
      dakota_drivers.in                     # 4-6 abort; refork?
      dakota_rbdo_steel_column_analytic2.in # abort: LAPACK error
      dakota_uq_timeseries_ivp_optinterf.in # workdir issue?
      dakota_uq_timeseries_sop_optinterf.in # workdir issue?
      dakota_uq_cantilever_ivp_exp.in       # mysteriously hangs when run by Jenkins
      dakota_rosenbrock_importance.in       # File IO/performance issue?
      dakota_workdir.in                     # not sufficiently generic
      dakota_win_ext.in                     # stability?
      )
  else()
    # Remove windows-specific tests from the UNIX suite
    list(REMOVE_ITEM dakota_test_input_files
      dakota_win_ext.in
      dakota_workdir_windows.in
      )
  endif()

  # Load lists of smoke and acceptance tests
  set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH})
  include(TestLabelsAcceptance)

  # Iterate the list of test files and create tests for each
  foreach(test_input_file ${dakota_test_input_files})

    # Glob up all files associated with this test input file
    # These get copied by add_application_test into test/dakota_*/
    get_filename_component(test_name ${test_input_file} NAME_WE)
    file(GLOB test_all_files
      RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${test_name}.*"
      )

    set(test_labels DakotaTest SerialTest)
    list(FIND dakota_smoke_tests ${test_name} dst_index)
    if (NOT dst_index EQUAL -1)
      list(APPEND test_labels SmokeTest)
    endif()
    list(FIND dakota_acceptance_tests ${test_name} dst_index)
    if (NOT dst_index EQUAL -1)
      list(APPEND test_labels AcceptanceTest)
    endif()
 
    # add serial test, but don't create custom target for copied files
    add_application_test(
      ${test_name}
      FILE_DEPENDENCIES "${test_all_files}"
      PREPROCESS COMMAND \"${CMAKE_COMMAND}\" -E remove dakota_diffs.out
      APPLICATION COMMAND \"${PERL_EXECUTABLE}\"
        ${CMAKE_CURRENT_BINARY_DIR}/dakota_test.perl
        --output-dir=${CMAKE_CURRENT_BINARY_DIR}/${test_name}
        --bin-dir=${CMAKE_CURRENT_BINARY_DIR}
        ${test_input_file}
      UNIQUE_DIRECTORY
      NO_TARGET
      LABELS ${test_labels}
      )

    # the test add makes a copy command without target, now track all
    # the copied files for later addition to a custom target
    foreach(file ${test_all_files})
      set(copied_file "${CMAKE_CURRENT_BINARY_DIR}/${test_name}/${file}")
      list(APPEND copied_test_files_abs "${copied_file}")
    endforeach()

    # Also copy the aux files into test/ to support direct testing with Perl
    set(test_aux_files ${test_all_files})
    list(REMOVE_ITEM test_aux_files ${test_input_file})

    foreach(file ${test_aux_files})
      add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/${file} 
	${CMAKE_CURRENT_BINARY_DIR}/${file})
      list(APPEND copied_test_files_abs "${CMAKE_CURRENT_BINARY_DIR}/${file}")  
    endforeach()

    # only add parallel tests when MPI enabled
    if (DAKOTA_HAVE_MPI)
      # if Dakota test has parallel tests (i.e. #p[0-9]+), add parallel test
      file(STRINGS ${test_input_file} PTestList REGEX "#p[0-9]+" LIMIT_COUNT 1)
      if (NOT ${PTestList} EQUAL "")

	set(test_labels DakotaTest ParallelTest)
	list(FIND dakota_smoke_tests p${test_name} dst_index)
	if (NOT dst_index EQUAL -1)
	  list(APPEND test_labels SmokeTest)
	endif()
	list(FIND dakota_acceptance_tests p${test_name} dst_index)
	if (NOT dst_index EQUAL -1)
	  list(APPEND test_labels AcceptanceTest)
	endif()
 
	add_application_test(
	  p${test_name}
	  FILE_DEPENDENCIES "${test_all_files}"
	  PREPROCESS COMMAND \"${CMAKE_COMMAND}\" -E remove dakota_pdiffs.out
	  APPLICATION COMMAND \"${PERL_EXECUTABLE}\"
	    ${CMAKE_CURRENT_BINARY_DIR}/dakota_test.perl
	    --parallel
 	    --output-dir=${CMAKE_CURRENT_BINARY_DIR}/p${test_name}
	    --bin-dir=${CMAKE_CURRENT_BINARY_DIR}
	    ${test_input_file}
	  UNIQUE_DIRECTORY
	  NO_TARGET
	  LABELS ${test_labels}
	  )
	# track all the copied files for later addition to a custom target
	foreach(file ${test_all_files})
	  set(copied_file "${CMAKE_CURRENT_BINARY_DIR}/p${test_name}/${file}")
	  list(APPEND copied_test_files_abs "${copied_file}")
	endforeach()
      endif() # add parallel test
    endif() # MPI enabled

    # TODO: The following regex needs to be based on output of dakota_diff.perl
    #       and account for consistency with baseline
    # set_tests_properties(${DAKOTA_SYSTEST} PROPERTIES 
    #  FAIL_REGULAR_EXPRESSION "fail")

  endforeach() # foreach test_input_file

  # Data files that can't follow the auxilliary naming convention due
  # to field data naming requirements:
  # TODO: include this use case in test system enhancements
  set(oscillator_aux_files 
    oscillator.1.dat  oscillator.1.sigma oscillatorA.1.dat oscillatorA.1.sigma)
  foreach(file ${oscillator_aux_files})
    add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/${file} 
      ${CMAKE_CURRENT_BINARY_DIR}/${file})
    list(APPEND copied_test_files_abs "${CMAKE_CURRENT_BINARY_DIR}/${file}")  
    add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/${file} 
      ${CMAKE_CURRENT_BINARY_DIR}/dakota_experiment_read/${file})
    list(APPEND copied_test_files_abs "${CMAKE_CURRENT_BINARY_DIR}/dakota_experiment_read/${file}")  
  endforeach()

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


# Executables needed for DAKOTA testing, but NOT unit tests
set(dakota_test_executables
  barnes
  barnes_lf
  cantilever
  cantilever3
  container
  cyl_head
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
