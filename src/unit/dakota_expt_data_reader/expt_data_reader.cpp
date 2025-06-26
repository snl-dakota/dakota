/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
#include "DakotaVariables.hpp"

#include <string>

#include <gtest/gtest.h>

using namespace Dakota;

namespace {

  const int NUM_EXPTS = 3;
  const int NCV = 4;

  const int NUM_FIELD_VALUES = 9;
  const int VECTOR_FIELD_DIM = 4;

  // Variables object with num_config_vars continuous state vars...
  Variables gen_mock_vars(const int& num_config_vars) {
    std::pair<short,short> mock_vars_view(MIXED_DESIGN, MIXED_STATE);
    SizetArray mock_vars_comps_totals(NUM_VC_TOTALS, 0);
    mock_vars_comps_totals[TOTAL_CSV] = num_config_vars;
    SharedVariablesData mock_svd(mock_vars_view, mock_vars_comps_totals);
    Variables mock_vars(mock_svd);
    return mock_vars;
  }
}

//----------------------------------------------------------------

TEST(expt_data_reader_tests, test_expt_data_reader_read_config_vars)
{
  const std::string base_name = "../expt_data_test_files/voltage";

  std::vector<Variables> config_data1(NUM_EXPTS, gen_mock_vars(NCV));
  read_config_vars_multifile(base_name, NUM_EXPTS, NCV, config_data1);

  std::vector<Variables> config_data2(NUM_EXPTS, gen_mock_vars(NCV));
  read_config_vars_singlefile(base_name, NUM_EXPTS, NCV, config_data2);

  // Verify equality of config data
  EXPECT_TRUE(( config_data1.size() == config_data2.size() ));
  for( size_t i=0; i<config_data1.size(); ++i ) {
    const RealVector& config1 = config_data1[i].inactive_continuous_variables();
    const RealVector& config2 = config_data2[i].inactive_continuous_variables();
    EXPECT_TRUE((config1.length() == NCV));
    EXPECT_TRUE((config2.length() == NCV));
    for( int j=0; j<config1.length(); ++j )
      EXPECT_LT(std::fabs(1. - config1[j] / config2[j]), 1.e-12/100. );
  }
}

//----------------------------------------------------------------

TEST(expt_data_reader_tests, test_expt_data_reader_read_bad_config_vars_size)
{
  const std::string base_name = "../expt_data_test_files/voltage";

  std::vector<Variables> config_data(NUM_EXPTS, gen_mock_vars(NCV+1));
  Dakota::abort_mode = ABORT_THROWS;
  EXPECT_THROW( read_config_vars_multifile( base_name, NUM_EXPTS, NCV+1, config_data), std::runtime_error );
  EXPECT_THROW( read_config_vars_singlefile(base_name, NUM_EXPTS, NCV+1, config_data), std::runtime_error );
  Dakota::abort_mode = ABORT_EXITS;
}

//----------------------------------------------------------------

TEST(expt_data_reader_tests, test_expt_data_reader_read_field_values_scalar)
{
  const std::string base_name = "../expt_data_test_files/voltage";

  // This reads directly into a RealVector
  RealVector field_vals;
  read_field_values(base_name, 2 /* expt number */, field_vals);

  // ... while this reads into a RealVectorArray
  RealVectorArray field_vals_array;
  read_field_values(base_name, 2 /* expt number */, field_vals_array);

  // Totally contrived values ... and should be reflected in the corresponding
  // test data file, "expt_data_test_files/voltage.2.dat"
  double gold_values[] = { 2.34, 8.552, -3.654, 7.332, 0.01, -0.1509, -5.98, 4.74, 9.99 };

  // Verify equality of field data
  EXPECT_TRUE(( field_vals_array.size() == 1 ));
  EXPECT_TRUE(( field_vals_array[0].length() == NUM_FIELD_VALUES ));
  EXPECT_TRUE(( field_vals.length() == NUM_FIELD_VALUES ));
  for( size_t i=0; i<NUM_FIELD_VALUES; ++i )
  {
    EXPECT_LT(std::fabs(1. - field_vals_array[0][i] / gold_values[i]), 1.e-12/100. );
    EXPECT_LT(std::fabs(1. - field_vals[i] / gold_values[i]), 1.e-12/100. );
  }
}

//----------------------------------------------------------------

TEST(expt_data_reader_tests, test_expt_data_reader_read_field_values_vector)
{
  const std::string base_name = "../expt_data_test_files/electric_field";

  RealVectorArray field_vals;
  read_field_values(base_name, 2 /* expt number */, field_vals);

  // Verify equality of field data
  EXPECT_TRUE(( field_vals.size() == VECTOR_FIELD_DIM ));
  EXPECT_TRUE(( field_vals[0].length() == NUM_FIELD_VALUES ));
  for( int i=0; i<VECTOR_FIELD_DIM; ++i )
    for( int j=0; j<NUM_FIELD_VALUES; ++j )
      EXPECT_LT(std::fabs(1. - field_vals[i][j] / (double(j+1)+double(j+1)*pow(0.1,i))), 1.e-12/100. );
}

//----------------------------------------------------------------

TEST(expt_data_reader_tests, test_expt_data_reader_read_coord_values)
{
  const std::string base_name = "../expt_data_test_files/voltage";

  RealMatrix coords;
  read_coord_values(base_name, 2 /* expt number */, coords);

  // Verify equality of coord data
  EXPECT_TRUE(( coords.numCols() == VECTOR_FIELD_DIM ));
  EXPECT_TRUE(( coords.numRows() == NUM_FIELD_VALUES ));
  for( int i=0; i<VECTOR_FIELD_DIM; ++i )
    for( int j=0; j<NUM_FIELD_VALUES; ++j )
      EXPECT_LT(std::fabs(1. - coords[i][j] / (double(i)+double(j+1)*0.1)), 1.e-12/100. );
}

//----------------------------------------------------------------

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
