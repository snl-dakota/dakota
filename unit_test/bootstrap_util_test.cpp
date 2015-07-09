/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#define BOOST_TEST_MODULE dakota_bootstrap_utils
#include <iostream>
#include <boost/test/included/unit_test.hpp>
#include <boost/test/unit_test.hpp>

#include "dakota_data_types.hpp"
#include "dakota_stat_util.hpp"

//____________________________________________________________________________//

BOOST_AUTO_TEST_CASE( test_bootstrap_real_matrix )
{
  using namespace Dakota;

  double test_input_vals[] = {1,2,3,4,5,6};
  double test_output_vals[] = {5,1,6,6,1,6};

  RealMatrix test_matrix(Teuchos::Copy, test_input_vals, 1, 1, 6);

  BootstrapSampler<RealMatrix> bootstrapS(6, test_matrix);

  RealMatrix result = bootstrapS(6);

  BOOST_CHECK_EQUAL_COLLECTIONS(result[0], result[5],
                                test_output_vals, test_output_vals+5);

}

BOOST_AUTO_TEST_CASE( test_bootstrap_sampling )
{
  using namespace Dakota;

  double test_input_vals[] = {1,2,3,4,5,6,7,8,9};
  double test_output_vals[] = {7,8,9,1,2,3};

  RealMatrix test_matrix(Teuchos::Copy, test_input_vals, 1, 1, 9);

  BootstrapSamplerWithGS<RealMatrix,
                         RealMatrix(*)(int, const RealMatrix&),
                         void(*)(int, RealMatrix, RealMatrix&)>
                         bootstrapS(3, test_matrix,
                                           real_mat_getter_method<3>,
                                           real_mat_setter_method<3>);

  RealMatrix result(1, 6);

  bootstrapS(2, result);

  BOOST_CHECK_EQUAL_COLLECTIONS(result[0], result[5],
                                test_output_vals, test_output_vals+5);

}
