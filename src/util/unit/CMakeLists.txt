#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014 Sandia Corporation.
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

# BMA TODO: helpers to manage all these test registration steps

include(DakotaUnitTest)

# List of test files to copy into this directory
#set(dakota_unit_test_copied_files)

dakota_add_unit_test(NAME DataScalerTest
  SOURCES util_test_driver.cpp DataScalerTest.cpp
  )

target_include_directories(DataScalerTest PRIVATE "${Teuchos_INCLUDE_DIRS}")
target_link_libraries(DataScalerTest PRIVATE dakota_util ${Teuchos_LIBRARIES})

dakota_add_unit_test(NAME TeuchosEigenConversionTest
  SOURCES util_test_driver.cpp teuchos_eigen_conversion.cpp
  )

target_include_directories(TeuchosEigenConversionTest PRIVATE "${Teuchos_INCLUDE_DIRS}")
target_link_libraries(TeuchosEigenConversionTest PRIVATE dakota_util ${Teuchos_LIBRARIES})

dakota_add_unit_test(NAME LinearSolverTest
  SOURCES util_test_driver.cpp LinearSolverTest.cpp
  )

target_include_directories(LinearSolverTest PRIVATE "${Teuchos_INCLUDE_DIRS}")
target_link_libraries(LinearSolverTest PRIVATE dakota_util ${Teuchos_LIBRARIES})

#target_include_directories(DataScalerTest PRIVATE
#  "${CMAKE_CURRENT_SOURCE_DIR}/.." "${Teuchos_INCLUDE_DIRS}")

# Add a single target for all copied files
#add_custom_target(util_unit_test_files ALL
#  DEPENDS ${util_unit_test_copied_files})
