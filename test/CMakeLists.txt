# CMakeLists.txt for DAKOTA tests

# The 'morris' tester depends on Boost
include_directories(${Boost_INCLUDE_DIRS})

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




#include_directories(${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/src
#                    ${Boost_INCLUDE_DIRS}
#                    ${Teuchos_INCLUDE_DIRS})
#add_definitions("-DHAVE_CONFIG_H")

#if(HAVE_LHS)
#  set(TPL_LIBS ${TPL_LIBS} lhs)
#endif(HAVE_LHS)

# Complete these for each specific test.
#add_executable(boost_test_dist boost_test_dist.cpp)
#target_link_libraries(boost_test_dist pecos pecos_src ${TPL_LIBS} ${LAPACK_LIBS} ${BLAS_LIBS} ${FLIBS} ${FCLIBS})
#add_test(boost_test_dist boost_test_dist)

# Complete these for each specific test.
#add_executable(pecos_ifft_g pecos_ifft_g.cpp)
#target_link_libraries(pecos_ifft_g pecos pecos_src ${TPL_LIBS} ${LAPACK_LIBS}
#${BLAS_LIBS} ${FLIBS} ${FCLIBS})
#add_test(pecos_ifft_g pecos_ifft_g)


