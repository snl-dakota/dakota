# CMakeLists.txt for DAKOTA tests
remove_definitions("-DHAVE_CONFIG_H")

check_include_files(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
  add_definitions("-DHAVE_UNISTD_H")
endif(HAVE_UNISTD_H)

# The 'morris' tester depends on Boost
include_directories(${Boost_INCLUDE_DIRS})

if(BUILD_TESTING)

  # Copy the dakota_test support files into the build tree
  # with dependency to source files
  file(GLOB DAKOTA_SYSTEM_TESTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "dakota_*.in")
  set(TestFiles
    ${DAKOTA_SYSTEM_TESTS} dakota_test.perl dakota_diff.perl dakota_base.test
    dakota_coliny.dat dakota_pce.txt least_squares_test.dat container.script
    test_3pc_ac test_3pc_if test_3pc_of
    test_3pc_ac_verb test_3pc_if_verb test_3pc_of_verb
  )
  foreach(file ${TestFiles})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${file}
      ${CMAKE_CURRENT_BINARY_DIR}/${file}
      COPYONLY)
  endforeach()

  # Configure supplementary AMPL files
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ampl)
  #set(GLOB AmplTests ${CMAKE_CURRENT_SOURCE_DIR}/ampl/*.*)
  set(AmplTests
    fma fma.col fma.nl fma.row
    text_book.col text_book.mod text_book.nl text_book.row
    text_book_mixed.mod text_book_mixed_unc.mod    text_book_unc.col text_book_unc.mod text_book_unc.nl text_book_unc.row
  )
  foreach(file ${AmplTests})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ampl/${file}
      ${CMAKE_CURRENT_BINARY_DIR}/ampl/${file}
      COPYONLY)
  endforeach()

  # Copy any necessary executables into the BINARY_DIR/test directory
  get_target_property(EXE_LOC dakota LOCATION)
  get_target_property(LIBMODE_EXE_LOC dakota_library_mode LOCATION)

  add_custom_target(test_dakota ALL
    ${CMAKE_COMMAND} -E copy ${EXE_LOC} ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS dakota)  

  add_custom_target(test_lib_mode_dakota ALL
    ${CMAKE_COMMAND} -E copy ${LIBMODE_EXE_LOC} ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS dakota_library_mode)  

  # Create one CTest per DAKOTA input file
  foreach(DAKOTA_SYSTEST ${DAKOTA_SYSTEM_TESTS})
    add_test(${DAKOTA_SYSTEST}
             ${CMAKE_CURRENT_BINARY_DIR}/dakota_test.perl ${DAKOTA_SYSTEST})
    # TODO: The following regex needs to be based on output of dakota_diff.perl
    #       and account for consistency with baseline
    set_tests_properties(${DAKOTA_SYSTEST} PROPERTIES 
      FAIL_REGULAR_EXPRESSION "fail")
  endforeach()

  # Clean up test dir
  #file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/dakota_diffs.out)

endif(BUILD_TESTING)


# Executables needed for DAKOTA testing, but NOT unit tests

add_executable(barnes barnes.C)
add_executable(barnes_lf barnes_lf.C)
add_executable(cantilever cantilever.C)
add_executable(container container.C)
add_executable(cyl_head cyl_head.C)
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
add_executable(short_column short_column.C)
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
# WJB - ToDo - if HAVE_MPI: add_executable(text_book_par text_book_par.C)

# Enforce an "imaginary" dependency
add_dependencies(cantilever dakota)
add_dependencies(morris dakota)
add_dependencies(text_book dakota)


# UNIT TESTS

if(ENABLE_TESTS)
  # Complete these for each specific test.

  # WJB - ToDo:  add a probe for CppUnit
  add_executable(stream_unit_tests stream_unit_tests.C)
  #target_link_libraries(stream_unit_tests dakota_src)
  add_test(stream_unit_tests stream_unit_tests)

  add_executable(container_unit_tests container_unit_tests.C)

  # dll_tester needs to include Dakota/src/dll_api.h
  include_directories(${PROJECT_SOURCE_DIR}/src)
  add_executable(dll_tester dll_tester.C)

endif()

