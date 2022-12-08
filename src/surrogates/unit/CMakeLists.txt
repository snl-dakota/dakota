#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014-2022
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

# BMA TODO: helpers to manage all these test registration steps

include(DakotaUnitTest)

# List of test files to copy into this directory
set(surrogates_unit_test_copied_files)

dakota_add_unit_test(NAME GaussianProcessTest SOURCES
  GaussianProcessTest.cpp
  # To force linking against fortran runtime needed to teuchosnumerics
  # dependence on BLAS/LAPACK (TODO: root cause the link chain)
  empty.f90
  )

dakota_add_unit_test(NAME PolynomialRegressionTest SOURCES
  PolynomialRegressionTest.cpp
)

dakota_add_unit_test(NAME SurrogatesToolsTest SOURCES
  SurrogatesToolsTest.cpp
)

dakota_add_unit_test(NAME EvalMetricsCrossValTest SOURCES
  EvalMetricsCrossValTest.cpp
  # To force linking against fortran runtime needed to teuchosnumerics
  # dependence on BLAS/LAPACK (TODO: root cause the link chain)
  empty.f90
  )

if(DAKOTA_PYTHON_SURROGATES)
  add_test(NAME surrogates_python
    COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_surrogate.py
    $<TARGET_FILE_DIR:surrogates>
    )
  set_tests_properties(surrogates_python PROPERTIES LABELS Unit)
endif()

# Rationale: These all include Teuchos headers and need to link to the
# unit test system or ParameterList components
target_include_directories(PolynomialRegressionTest PRIVATE "${Teuchos_INCLUDE_DIRS}")
target_include_directories(EvalMetricsCrossValTest PRIVATE "${Teuchos_INCLUDE_DIRS}")

target_link_libraries(GaussianProcessTest PRIVATE dakota_surrogates Boost::filesystem)
target_link_libraries(PolynomialRegressionTest PRIVATE dakota_surrogates
  ${Teuchos_LIBRARIES} Boost::filesystem Boost::serialization Boost::boost Boost::system)
target_link_libraries(SurrogatesToolsTest PRIVATE dakota_surrogates)
target_link_libraries(EvalMetricsCrossValTest PRIVATE dakota_surrogates
  ${Teuchos_LIBRARIES})

dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/gp_test_data"
  "${CMAKE_CURRENT_BINARY_DIR}/gp_test_data"
  surrogates_unit_test_copied_files
  )

dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/pr_test_data"
  "${CMAKE_CURRENT_BINARY_DIR}/pr_test_data"
  surrogates_unit_test_copied_files
  )

# Add a single target for all copied files
add_custom_target(surrogates_unit_test_files ALL
  DEPENDS ${surrogates_unit_test_copied_files})
