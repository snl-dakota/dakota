#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2023
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

# BMA TODO: helpers to manage all these test registration steps

include(DakotaUnitTest)

# List of test files to copy into this directory
set(surrogates_unit_test_copied_files)

dakota_add_unit_test(NAME surrogates_gaussian_process
  SOURCES GaussianProcessTest.cpp
  # To force linking against fortran runtime needed to teuchosnumerics
  # dependence on BLAS/LAPACK (TODO: root cause the link chain)
    empty.f90
  LINK_LIBS dakota_surrogates Boost::boost Boost::system Boost::filesystem)

dakota_add_unit_test(NAME surrogates_polynomial_regression
  SOURCES PolynomialRegressionTest.cpp
  LINK_LIBS dakota_surrogates Boost::system Boost::filesystem)

dakota_add_unit_test(NAME surrogates_tools
  SOURCES SurrogatesToolsTest.cpp
  LINK_LIBS dakota_surrogates)

dakota_add_unit_test(NAME surrogates_eval_metrics_cross_val
  SOURCES EvalMetricsCrossValTest.cpp
  # To force linking against fortran runtime needed to teuchosnumerics
  # dependence on BLAS/LAPACK (TODO: root cause the link chain)
    empty.f90
  LINK_LIBS dakota_surrogates)

if(DAKOTA_PYTHON_SURROGATES)
  add_test(NAME surrogates_python
    COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_surrogate.py
    $<TARGET_FILE_DIR:surrogates>
    )
  set_tests_properties(surrogates_python PROPERTIES LABELS Unit)
endif()

# Rationale: These all include Teuchos headers and need to link to the
# ParameterList components
target_include_directories(surrogates_polynomial_regression PRIVATE "${Teuchos_INCLUDE_DIRS}")
target_include_directories(surrogates_eval_metrics_cross_val PRIVATE "${Teuchos_INCLUDE_DIRS}")


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
