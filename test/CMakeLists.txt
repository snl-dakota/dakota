# CMakeLists.txt for DAKOTA tests

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

  # Configure or copy the dakota_test support files into the build
  # tree with dependency to source files (needed for legacy behavior,
  # but ultimately would like to eliminate)

  # configured primary scripts
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dakota_test.perl
    ${CMAKE_CURRENT_BINARY_DIR}/dakota_test.perl @ONLY)

  # copied primary scripts
  set(test_support_files dakota_diff.perl) 

  # analysis driver scripts
  list(APPEND test_support_files
    dakota_container.sh dakota_3pc.ac.sh dakota_3pc.if.sh dakota_3pc.of.sh
    dakota_3pc.ac_verb.sh dakota_3pc.if_verb.sh dakota_3pc.of_verb.sh
    dakota_workdir.sh
    )

  # supplemental data
  list(APPEND test_support_files
    dakota_coliny.dat dakota_logratio_pce.txt 
    dakota_pstudy.3.dat dakota_pstudy.7.dat
    dakota_textbook_nips.lsq.dat dakota_textbook_nips.lsq.ann
    dakota_uq_textbook_lhs_approx.annotated 
    dakota_uq_textbook_lhs_approx.freeform
    )

  # supplementary AMPL files
  list(APPEND test_support_files
    dakota_ampl_fma.nl dakota_ampl_fma.row dakota_ampl_fma.col
    dakota_ampl_tb.nl dakota_ampl_tb.row dakota_ampl_tb.col
    dakota_ampl_tb.unc.nl dakota_ampl_tb.unc.row dakota_ampl_tb.unc.col
    )

  # Create a custom command to copy each file to the build directory, but
  # group them all into a single target
  set(test_support_files_abs "")
  foreach(file ${test_support_files})
    add_file_copy_command(
      ${CMAKE_CURRENT_SOURCE_DIR}/${file} 
      ${CMAKE_CURRENT_BINARY_DIR}/${file}
      )
    list(APPEND test_support_files_abs "${CMAKE_CURRENT_BINARY_DIR}/${file}")  
  endforeach()
  add_custom_target(dakota_test_support_files ALL 
    DEPENDS ${test_support_files_abs}
    COMMENT "Copy DAKOTA test support files to binary test/ directory"
    )

  # Create one CTest per DAKOTA input file, in a protected subdirectory
  set(copied_test_files_abs "")

  if (WIN32)
    file(GLOB dakota_test_input_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
      "dakota_4*" "dakota_a*.in" "dakota_b*.in" "dakota_c*.in" "dakota_d*.in"
      "dakota_h*" "dakota_i*.in" "dakota_k*.in" "dakota_m*.in" "dakota_n*.in"
      "dakota_p*" "dakota_r*.in" "dakota_s*.in"
    )
  else()
    file(GLOB dakota_test_input_files 
      RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "dakota_*.in"
    )
  endif()

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
  # Remove tests that fail on Windows (for now)
  if (WIN32)
    list(REMOVE_ITEM dakota_test_input_files
      dakota_3pc.in
      dakota_coliny_p.in
      dakota_container.in
      dakota_drivers.in
      dakota_cantilever_ego.in
      dakota_mogatest.in
      dakota_rbdo_short_column_analytic2.in
      dakota_rbdo_short_column_trsb2.in
      dakota_sebdo_short_column_adapt.in
      dakota_short_column_taylor2.in
      dakota_rbdo_short_column_analytic2.in
      dakota_rbdo_short_column_trsb2.in
      dakota_short_column_taylor2.in
      dakota_uq_cantilever_sop_exp.in
      dakota_uq_cantilever_sop_exp.in
      dakota_uq_rosenbrock_adapt_pce_lls.in
      dakota_uq_rosenbrock_global.in
      dakota_users_mogatest1.in
      dakota_users_mogatest2.in
      dakota_users_mogatest3.in
      dakota_lps_pce_rosenbrock_fail.in
      dakota_rosenbrock_fail.in
      dakota_uq_rosenbrock_pce_lls_cv_fail.in
      dakota_nomad.in
      dakota_pcbdo_cantilever.in
      dakota_pof.in
      dakota_uq_ishigami_dste_exp.in
      dakota_uq_rosenbrock_adapt_exp.in
      dakota_uq_rosenbrock_pce_lls.in
      dakota_uq_rosenbrock_pce_lls_cv.in
      dakota_uq_rosenbrock_sc.in
      dakota_uq_short_column_adapt_gsg.in
      dakota_rbdo_cantilever_trsb.in
      dakota_rbdo_short_column_trsb.in
      dakota_rbdo_steel_column_analytic2.in
      dakota_workdir.in
    )
  endif()

  foreach(test_input_file ${dakota_test_input_files})

    # Glob up all files associated with this test input file
    get_filename_component(test_name ${test_input_file} NAME_WE)
    file(GLOB test_all_files
      RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${test_name}.*"
      )

    # add serial test, but don't create custom target
    add_application_test(
      ${test_name}
      FILE_DEPENDENCIES "${test_all_files}"
      PREPROCESS COMMAND "${CMAKE_COMMAND}" -E remove dakota_diffs.out
      APPLICATION COMMAND "${PERL_EXECUTABLE}"
        ${CMAKE_CURRENT_BINARY_DIR}/dakota_test.perl
        --output-dir=${CMAKE_CURRENT_BINARY_DIR}/${test_name}
        --bin-dir=${CMAKE_CURRENT_BINARY_DIR}
        ${test_input_file}
      UNIQUE_DIRECTORY
      NO_TARGET
      )

    # track all the copied files for later addition to a custom target
    foreach(file ${test_all_files})
      set(copied_file "${CMAKE_CURRENT_BINARY_DIR}/${test_name}/${file}")
      list(APPEND copied_test_files_abs "${copied_file}")
    endforeach()


    # only add parallel tests when MPI enabled
    if (DAKOTA_HAVE_MPI)
      # if Dakota test has parallel tests (i.e. #p[0-9]+), add parallel test
      file(STRINGS ${test_input_file} PTestList REGEX "#p[0-9]+" LIMIT_COUNT 1)
      if (NOT ${PTestList} EQUAL "")
	add_application_test(
	  p${test_name}
	  FILE_DEPENDENCIES "${test_all_files}"
	  PREPROCESS COMMAND "${CMAKE_COMMAND}" -E remove dakota_pdiffs.out
	  APPLICATION COMMAND "${PERL_EXECUTABLE}"
	    ${CMAKE_CURRENT_BINARY_DIR}/dakota_test.perl
	    --parallel
 	    --output-dir=${CMAKE_CURRENT_BINARY_DIR}/p${test_name}
	    --bin-dir=${CMAKE_CURRENT_BINARY_DIR}
	    ${test_input_file}
	  UNIQUE_DIRECTORY
	  NO_TARGET
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

  # Create a single target to copy all the testing files
  add_custom_target(dakota_test_files ALL 
    DEPENDS ${copied_test_files_abs}
    COMMENT 
      "Copy DAKOTA test inputs and aux files to binary test/ subdirectories"
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
  container
  cyl_head
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
endif()

# Enforce an "imaginary" dependency
add_dependencies(cantilever dakota)
add_dependencies(morris dakota)
add_dependencies(text_book dakota)


# UNIT TESTS

# WJB - ToDo:  decide on a unit test framework and probe for it
option(DAKOTA_ENABLE_UNIT_TESTS "Enable unit tests in DAKOTA" OFF)
if(DAKOTA_ENABLE_UNIT_TESTS)
  # Complete these for each specific test.

  add_executable(stream_unit_tests stream_unit_tests.cpp)
  #target_link_libraries(stream_unit_tests dakota_src)
  add_test(stream_unit_tests stream_unit_tests)

  add_executable(container_unit_tests container_unit_tests.cpp)

endif()

if(DAKOTA_HAVE_HDF5 
    #AND DAKOTA_ENABLE_UNIT_TESTS
    )
  # BinaryIO tester depends on Teuchos and HDF5
  include_directories(${HDF5_INCLUDE_DIRS})
  add_executable(binary_io_test binary_io_test.cpp)
  target_link_libraries(binary_io_test ${Teuchos_LIBRARIES} ${TEUCHOS_LIBRARY}
    ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})
  link_directories(binary_io_test ${Teuchos_LIBRARY_DIRS} ${HDF5_LIBRARY_DIRS})
endif()

if(DAKOTA_DLL_API)
  # dll_tester needs to include Dakota/src/dll_api.h
  include_directories(${Dakota_SOURCE_DIR}/src)
  if (DAKOTA_MODELCENTER)
    # setup link dirs before inclusion of dll_api library
    link_directories(
      "${Dakota_SOURCE_DIR}/examples/linked_interfaces/ModelCenter/lib/i686-cygwin-cygwinNT")
  endif()
  list(APPEND dakota_test_executables dll_tester)
  add_executable(dll_tester dll_tester.cpp)
  
  target_link_libraries(dll_tester dll_api)
  if (DAKOTA_MODELCENTER)
    # likely not needed due to CMake dependency managemane of dll_api library
    target_link_libraries(dll_tester PHXCppApi)
  endif()
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
