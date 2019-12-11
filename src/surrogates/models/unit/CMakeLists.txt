#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014 Sandia Corporation.
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

include(DakotaUnitTest)

# List of test files to copy into this directory
set(dakota_unit_test_copied_files)

message("Teuchos_INCLUDE_DIRS ${Teuchos_INCLUDE_DIRS}")
message("Teuchos_LIBRARIES ${Teuchos_LIBRARIES}")

include_directories(${Dakota_SOURCE_DIR}/src/surrogates/models/src)
include_directories(${DAKOTA_INCDIRS})

dakota_add_unit_test(NAME gp_test
  SOURCES teuchos_unit_test_driver.cpp gp_approximation_ts.cpp
  LINK_DAKOTA_LIBS
  )

dakota_add_unit_test(NAME DataScalerTest
  SOURCES teuchos_unit_test_driver.cpp DataScalerTest.cpp
  LINK_DAKOTA_LIBS
  )

dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/rol_params.xml"
  "${CMAKE_CURRENT_BINARY_DIR}/rol_params.xml"
  dakota_unit_test_copied_files
  )

dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/gp_test_data"
  "${CMAKE_CURRENT_BINARY_DIR}/gp_test_data"
  dakota_unit_test_copied_files
  )

# add_executable(gp_test teuchos_unit_test_driver.cpp ${gp_test} )
target_link_libraries(gp_test        surrogates_models ${LINK_DAKOTA_LIBS} ${Teuchos_LIBRARIES})
target_link_libraries(DataScalerTest surrogates_models ${LINK_DAKOTA_LIBS} ${Teuchos_LIBRARIES})

# Add a single target for all copied files
add_custom_target(surrogate_unit_test_files ALL
  DEPENDS ${dakota_unit_test_copied_files})

