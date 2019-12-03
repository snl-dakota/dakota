#  _______________________________________________________________________
#
#  PECOS: Parallel Environment for Creation Of Stochastics
#  Copyright (c) 2011, Sandia National Laboratories.
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Pecos directory.
#  _______________________________________________________________________

# CMakeLists.txt for linear_solvers "unit" tests
message("Teuchos_INCLUDE_DIRS ${Teuchos_INCLUDE_DIRS}")
message("Teuchos_LIBRARIES ${Teuchos_LIBRARIES}")

include_directories(${Dakota_SOURCE_DIR}/src/surrogates/models/src)
include_directories(${DAKOTA_INCDIRS})

# WDTD
#remove_definitions("-DHAVE_CONFIG_H")

set(gp_test
  gp_approximation_ts.cpp
)

add_executable(gp_test teuchos_unit_test_driver.cpp ${gp_test} )
target_link_libraries(gp_test surrogates_models ${LINK_DAKOTA_LIBS} ${Teuchos_LIBRARIES})

#add_test(gp_test gp_test)

