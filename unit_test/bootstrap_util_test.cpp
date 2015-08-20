/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

// To avoid min/max issues in Teuchos; consider putting in dakota_global_defs?
#include "dakota_windows.h"
#define BOOST_TEST_MODULE dakota_bootstrap_utils
#include <iostream>
#include <vector>
#include <boost/test/included/unit_test.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/assign.hpp>

#include "dakota_data_types.hpp"
#include "dakota_stat_util.hpp"
// #define BOOST_NUMERIC_FUNCTIONAL_STD_VECTOR_SUPPORT
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>

// #include <boost/accumulators/numeric/functional.hpp>

//____________________________________________________________________________//

BOOST_AUTO_TEST_CASE( test_bootstrap_real_matrix )
{
  using namespace Dakota;
  using namespace boost::numeric::operators;
  using namespace boost::accumulators;

  double test_input_vals[] = {1,2,3,4,5,6};
  float test_input_vals2[] = {5,1,6,6,1,6};
  double test_output_vals[] = {5,1,6,6,1,6};

  RealMatrix test_matrix(Teuchos::Copy, test_input_vals, 1, 1, 6);

  BootstrapSampler<RealMatrix> bootstrapS(test_matrix);

  RealMatrix result(test_matrix);

  bootstrapS(result);

  BOOST_CHECK_EQUAL_COLLECTIONS(result[0], result[5],
                                test_output_vals, test_output_vals+5);
}

BOOST_AUTO_TEST_CASE( test_bootstrap_seq_container )
{
  using namespace Dakota;
  using namespace boost::assign;
  using namespace boost::accumulators;

  std::vector<float> test_vector = list_of(1)(2)(3)(4)(5)(6);
  std::vector<float> test_output_vals = list_of(5)(1)(6)(6)(1)(6);

  BootstrapSampler<std::vector<float> > bootstrapS(test_vector);

  std::vector<float> result(test_vector);

  bootstrapS(result);

  BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(),
                                test_output_vals.begin(),
                                test_output_vals.end());
  // std::cout << "Mean: " << mean(bootstrapS.genBSStatistics(1000000)[0])

  // mean(bootstrapS.bootstrappedStatistics<accumulator_set<std::vector<float>,
  // stats<tag::mean> > >
    // (100));
}

BOOST_AUTO_TEST_CASE( test_bootstrap_sampling )
{
  using namespace Dakota;

  double test_input_vals[] = {1,2,3,4,5,6,7,8,9};
  double test_output_vals[] = {7,8,9,1,2,3};

  // RealMatrix test_matrix(Teuchos::Copy, test_input_vals, 1, 1, 9);

  // BootstrapSamplerWithGS<RealMatrix,
  //                        RealMatrix(*)(int, const RealMatrix&),
  //                        void(*)(int, RealMatrix, RealMatrix&)>
  //                        bootstrapS(3, test_matrix,
  //                                          real_mat_getter_method<3>,
  //                                          real_mat_setter_method<3>);

  // RealMatrix result(1, 6);

  // bootstrapS(2, result);

  // BOOST_CHECK_EQUAL_COLLECTIONS(result[0], result[5],
  //                               test_output_vals, test_output_vals+5);

}
