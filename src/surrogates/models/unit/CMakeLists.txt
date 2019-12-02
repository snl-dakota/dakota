#  _______________________________________________________________________
#
#  PECOS: Parallel Environment for Creation Of Stochastics
#  Copyright (c) 2011, Sandia National Laboratories.
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Pecos directory.
#  _______________________________________________________________________

# CMakeLists.txt for linear_solvers "unit" tests
message("${Surrogates_SOURCE_DIR}")

include_directories(${Surrogates_SOURCE_DIR}/models/src
                    ${Teuchos_INCLUDE_DIRS})

# WDTD
#remove_definitions("-DHAVE_CONFIG_H")

set(gp_test
  gp_approximation_ts.cpp
)

add_executable(gp_test teuchos_unit_test_driver.cpp ${gp_test} )
target_link_libraries(gp_test surrogates_models ${Surrogates_TPL_LIBS})

#add_test(gp_test gp_test)

