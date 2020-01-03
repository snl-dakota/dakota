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
set(surrogates_unit_test_copied_files)

dakota_add_unit_test(NAME GaussianProcessTest
  SOURCES teuchos_unit_test_driver.cpp gp_approximation_ts.cpp
  )

# BMA NOTE: I think we only need state the immediate deps? So no dakota_util
target_include_directories(GaussianProcessTest PRIVATE "${Teuchos_INCLUDE_DIRS}")
target_link_libraries(GaussianProcessTest PRIVATE dakota_surrogates ${Teuchos_LIBRARIES})

dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/rol_params.xml"
  "${CMAKE_CURRENT_BINARY_DIR}/rol_params.xml"
  surrogates_unit_test_copied_files
  )

dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/gp_test_data"
  "${CMAKE_CURRENT_BINARY_DIR}/gp_test_data"
  surrogates_unit_test_copied_files
  )

# Add a single target for all copied files
add_custom_target(surrogates_unit_test_files ALL
  DEPENDS ${surrogates_unit_test_copied_files})
