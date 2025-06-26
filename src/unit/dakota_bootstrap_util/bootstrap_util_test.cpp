/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

// To avoid min/max issues in Teuchos; consider leveraging a portable.hpp?
#include "util_windows.hpp"
#include <gtest/gtest.h>
#include <boost/assign.hpp>
#include <vector>

#include "dakota_data_types.hpp"
#include "BootstrapSampler.hpp"

//____________________________________________________________________________//

TEST( bootstrap_util_test_tests, test_bootstrap_real_matrix )
{
  using namespace Dakota;

  double test_input_vals[] = {1,2,3,4,5,6};
  float test_input_vals2[] = {5,1,6,6,1,6};
  double test_output_vals[] = {5,1,6,6,1,6};

  RealMatrix test_matrix(Teuchos::Copy, test_input_vals, 1, 1, 6);

  BootstrapSampler<RealMatrix> bootstrapS(test_matrix);

  RealMatrix result(test_matrix);

  bootstrapS(result);

  for (size_t i(0); i < result.numCols(); ++i) {
    EXPECT_EQ(result(0,i), test_output_vals[i]);
  }
}

TEST( bootstrap_util_test_tests, test_bootstrap_seq_container )
{
  using namespace Dakota;
  using namespace boost::assign;

  std::vector<float> test_vector = list_of(1)(2)(3)(4)(5)(6);
  std::vector<float> test_output_vals = list_of(5)(1)(6)(6)(1)(6);

  BootstrapSampler<std::vector<float> > bootstrapS(test_vector);

  std::vector<float> result(test_vector);

  bootstrapS(result);

  EXPECT_EQ(result, test_output_vals);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
