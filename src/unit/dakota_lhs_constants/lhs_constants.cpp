/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


/** \file lhs_constants.cpp Test LHS when variables degenerate to constants */

#include "opt_tpl_test.hpp"
#include "LibraryEnvironment.hpp"

#include <gtest/gtest.h>

std::string lhs_constants_input = R"(
# Verify LHS operation with some variables that degenerate to
# constants (lb == ub or set of size 1) to protect bug fix in DP-798

# This test will generate lots of NaN correlations, hence moved from
# regression to unit test. If causes problems cross-platform, can find
# another approach to test. Verification of this test is partially
# manual, via inspecting the tabular data file, though some weak
# protection is offered by the tests below.

environment
  tabular_data tabular_data_file 'dakota_lhs_constants.dat'
    freeform

method
  sampling
    samples  60
    seed  1
    sample_type  lhs
  output silent

variables
  active  all

  continuous_design  2
    descriptors   'cd1_const'  'cd2'
    lower_bounds  1.0           1.9
    upper_bounds  1.0           2.1

  discrete_design_range  2
    descriptors   'ddr3'  'ddr4_const'
    lower_bounds  2.0     4.0
    upper_bounds  4.0     4.0

  discrete_design_set
    integer  2
      descriptors     'dds5_const'  'dds6'
      num_set_values  1             3
          set_values  5             5 6 7

  # NOTE: uniform doesn't support const, while continuous design does...
  uniform_uncertain  1
    descriptors   'uu7'
    lower_bounds  0.06
    upper_bounds  0.08

  poisson_uncertain  1
    descriptors  'pu8'
    lambdas      8.0

  # NOTE: point histograms allow single values (constant), but can't
  # be correlated with others, so we verify that if correlation is
  # small enough it works.
  histogram_point_uncertain
    real  2
    descriptors         'hpui9'    'hpui10_const'
    pairs_per_variable  3          1 
    abscissas           8.9 9 9.1  10  
    counts              1   2 1    1

  # Should work as long as negligible correlation with hpui10
  uncertain_correlation_matrix
    # uu7  pu8  hpui9  hpui10
      1.0  0.2  0.3    0.0
      0.2  1.0  0.4    0.0
      0.3  0.4  1.0    0.0
      0.0  0.0  0.0    1.0

  continuous_state  1
    descriptors  'cs1_const'
    lower_bound  0.1
    upper_bound  0.1

  discrete_state_set
    integer  1
      descriptors  'dss2'
      set_values   2 22
  
    real  3
      descriptors            'dssr3_const' 'dssr4' 'dssr5_const'
      elements_per_variable  1             2       1
      set_values
        3
        0.4 4.0
        0.5

interface
  direct
    analysis_driver = 'text_book'

responses
  descriptors  'f' 'g1' 'g2'
  response_functions  3
  no_gradients
  no_hessians
)";


void check_constant_col(const Dakota::RealMatrix samples, int col_ind,
			double constant_value)
{
  for (size_t i=0; i<samples.numRows(); ++i)
    EXPECT_LT(std::fabs(1. - samples(i, col_ind) / constant_value), 1.0e-15 ); // It was BOOST_TEST with tolerance
}

double column_mean(const Dakota::RealMatrix samples, int col_ind)
{
  double mean = 0.0, num_rows = (double) samples.numRows();
  for (size_t i=0; i<samples.numRows(); ++i)
    mean += samples(i, col_ind) / num_rows;
  return mean;
}

TEST(lhs_constants_tests, test_lhs_constants)
{

  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Dakota::Opt_TPL_Test::create_env(lhs_constants_input));

  // Verify no DP-798 seg fault:
  p_env->execute();

  // Want to test on moments, but not correlations, but no way to get
  // the data from the Environment...

  // For now, make sure constants are indeed constant...
  Dakota::RealMatrix samples;
  size_t num_vars = 15, num_resp = 3, num_samples = 60;
  Dakota::TabularIO::read_data_tabular("dakota_lhs_constants.dat", "test_lhs_constants", samples, num_samples, num_vars + num_resp, Dakota::TABULAR_NONE, true);

  check_constant_col(samples, 0, 1.0);
  check_constant_col(samples, 3, 4.0);
  check_constant_col(samples, 4, 5);
  check_constant_col(samples, 9, 10);
  check_constant_col(samples, 10, 0.1);
  check_constant_col(samples, 12, 3);
  check_constant_col(samples, 14, 0.5);

  // And that means don't regress...
  EXPECT_LT(std::fabs(1. - column_mean(samples, 15) / 1.1403053861e+05), 1.0e-6 ); // It was BOOST_TEST with tolerance
  EXPECT_LT(std::fabs(1. - column_mean(samples, 16) / -2.2595567629e-05), 1.0e-6 ); // It was BOOST_TEST with tolerance
  EXPECT_LT(std::fabs(1. - column_mean(samples, 17) / 3.5035244300e+00) , 1.0e-6 ); // It was BOOST_TEST with tolerance
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
