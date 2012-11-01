# CMakeLists.txt for DAKOTA tests

include(AddApplicationTest)
include(AddFileCopyCommand)
include(CheckIncludeFile)

remove_definitions("-DHAVE_CONFIG_H")

check_include_file(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
  add_definitions("-DHAVE_UNISTD_H")
endif(HAVE_UNISTD_H)

# The 'morris' tester depends on Boost
include_directories(${Boost_INCLUDE_DIRS})

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
    )

  # supplemental data
  list(APPEND test_support_files
    dakota_coliny.dat dakota_logratio_pce.txt 
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

  # Copy any necessary executables into the BINARY_DIR/test directory
  # TODO: review for whether works on Windows and remove these on make clean
  get_target_property(EXE_LOC dakota LOCATION)
  get_target_property(LIBMODE_EXE_LOC dakota_library_mode LOCATION)
  get_target_property(LIBSPLIT_EXE_LOC dakota_library_split LOCATION)

  add_custom_target(test_dakota ALL
    ${CMAKE_COMMAND} -E copy ${EXE_LOC} ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS dakota)  

  add_custom_target(test_lib_mode_dakota ALL
    ${CMAKE_COMMAND} -E copy ${LIBMODE_EXE_LOC} ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS dakota_library_mode)  

  if(DAKOTA_HAVE_MPI)
    add_custom_target(test_lib_split ALL ${CMAKE_COMMAND} -E 
      copy ${LIBSPLIT_EXE_LOC} ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS dakota_library_split)
  endif()

  get_target_property(Dakota_EXECUTABLE_OUTPUT_DIR dakota LOCATION)
  get_filename_component(
    Dakota_EXECUTABLE_OUTPUT_DIR
    ${Dakota_EXECUTABLE_OUTPUT_DIR} PATH)

  # Create one CTest per DAKOTA input file, in a protected subdirectory
  set(copied_test_files_abs "")

  file(GLOB dakota_test_input_files 
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "dakota_*.in"
    )
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
      PREPROCESS COMMAND ${CMAKE_COMMAND} -E remove dakota_diffs.out
      APPLICATION COMMAND ${CMAKE_CURRENT_BINARY_DIR}/dakota_test.perl 
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
	  PREPROCESS COMMAND ${CMAKE_COMMAND} -E remove dakota_pdiffs.out
	  APPLICATION COMMAND ${CMAKE_CURRENT_BINARY_DIR}/dakota_test.perl 
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

endif() # BUILD_TESTING AND DAKOTA_ENABLE_TESTS


# Executables needed for DAKOTA testing, but NOT unit tests

add_executable(barnes barnes.C)
add_executable(barnes_lf barnes_lf.C)
add_executable(cantilever cantilever.C)
add_executable(container container.C)
add_executable(cyl_head cyl_head.C)
add_executable(herbie herbie.C)
add_executable(illumination illumination.C)
add_executable(lf_rosenbrock lf_rosenbrock.C)
add_executable(lf_poly_prod lf_poly_prod.C)
add_executable(log_ratio log_ratio.C)
add_executable(mod_cantilever mod_cantilever.C)
add_executable(mogatest1 mogatest1.C)
add_executable(mogatest2 mogatest2.C)
add_executable(mogatest3 mogatest3.C)
add_executable(morris morris.C)
add_executable(nl2func nl2func.C nl2test.C)
add_executable(poly_prod poly_prod.C)
add_executable(portal_frame portal_frame.C)
add_executable(quasi_sine_fcn quasi_sine_fcn.C)
add_executable(rosenbrock rosenbrock.C)
add_executable(rosenbrock_fail rosenbrock_fail.C)
add_executable(short_column short_column.C)
add_executable(shubert shubert.C)
add_executable(smooth_herbie smooth_herbie.C)
add_executable(sobol_g_function sobol_g_function.C)
add_executable(sobol_ishigami sobol_ishigami.C)
add_executable(sobol_rational sobol_rational.C)
add_executable(steel_column_cost steel_column_cost.C)
add_executable(steel_column_perf steel_column_perf.C)
add_executable(steel_section steel_section.C)
add_executable(text_book text_book.C)
add_executable(text_book1 text_book1.C)
add_executable(text_book2 text_book2.C)
add_executable(text_book3 text_book3.C)
add_executable(text_book_ouu text_book_ouu.C)
if(DAKOTA_HAVE_MPI)
  add_executable(text_book_par text_book_par.C)
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

  add_executable(stream_unit_tests stream_unit_tests.C)
  #target_link_libraries(stream_unit_tests dakota_src)
  add_test(stream_unit_tests stream_unit_tests)

  add_executable(container_unit_tests container_unit_tests.C)

  # dll_tester needs to include Dakota/src/dll_api.h
  include_directories(${PROJECT_SOURCE_DIR}/src)
  add_executable(dll_tester dll_tester.C)

endif()

# TODO: install to bin/?
install(TARGETS 
  barnes barnes_lf cantilever container cyl_head herbie illumination 
  lf_rosenbrock lf_poly_prod log_ratio mod_cantilever 
  mogatest1 mogatest2 mogatest3 morris nl2func poly_prod portal_frame 
  quasi_sine_fcn rosenbrock rosenbrock_fail short_column shubert smooth_herbie
  sobol_g_function sobol_ishigami sobol_rational 
  steel_column_cost steel_column_perf steel_section 
  text_book text_book1 text_book2 text_book3 text_book_ouu
  DESTINATION "${DAKOTA_TEST_INSTALL}/test")

if(DAKOTA_HAVE_MPI)
  install(TARGETS text_book_par DESTINATION "${DAKOTA_TEST_INSTALL}/test")
endif()
