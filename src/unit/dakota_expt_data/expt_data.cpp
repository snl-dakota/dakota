/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <gtest/gtest.h>

#include <string>

#include "DakotaVariables.hpp"
#include "ExperimentData.hpp"
#include "dakota_data_io.hpp"

using namespace Dakota;

namespace {

// Base case specifications
const int NUM_EXPTS = 1;
const int NUM_CONFIG_VARS = 3;
const size_t NUM_FIELDS = 1;
const size_t NUM_FIELD_VALUES = 401;
const int FIELD_DIM = 1;

// Variables object with NUM_CONFIG_VARS continuous state vars...
Variables gen_mock_vars() {
  std::pair<short, short> mock_vars_view(MIXED_DESIGN, MIXED_STATE);
  SizetArray mock_vars_comps_totals(NUM_VC_TOTALS, 0);
  mock_vars_comps_totals[TOTAL_CSV] = NUM_CONFIG_VARS;
  SharedVariablesData mock_svd(mock_vars_view, mock_vars_comps_totals);
  Variables mock_vars(mock_svd);
  return mock_vars;
}

ActiveSet mock_as(0);  // arg specifies num scalar values
SharedResponseData mock_srd(mock_as);

}  // namespace

//----------------------------------------------------------------

TEST(exp_data_tests, test_expt_data_basic) {
  // create an SRD with 0 scalars and 1 field of length NUM_FIELD_VALUES
  IntVector field_lengths(NUM_FIELDS);
  field_lengths[0] = NUM_FIELD_VALUES;
  mock_srd.field_lengths(field_lengths);

  StringArray variance_types(NUM_FIELDS);
  variance_types[0] = "diagonal";

  // convention appears to be field label is filename ?
  const std::string base_name = "new_voltage";
  const std::string working_dir = "../expt_data_test_files";
  StringArray field_labels(NUM_FIELDS);
  field_labels[0] = base_name;
  mock_srd.field_group_labels(field_labels);

  ExperimentData expt_data(NUM_EXPTS, NUM_CONFIG_VARS, working_dir, mock_srd,
                           variance_types, 0 /* SILENT_OUTPUT */);

  //  const std::string config_vars_basename = base_name;
  expt_data.load_data("expt_data unit test call", gen_mock_vars());

  // Test general correctness
  EXPECT_TRUE((expt_data.num_experiments() == NUM_EXPTS));
  EXPECT_TRUE((expt_data.num_scalar_primary() == 0));
  EXPECT_TRUE((expt_data.num_fields() == NUM_FIELDS));

  // Test data correctness
  RealVector field_vals_view = expt_data.field_data_view(0, 0);
  std::string filename = working_dir + "/" + base_name;
  RealVector gold_dat;
  read_field_values(filename, 1, gold_dat);
  EXPECT_TRUE((gold_dat.length() == field_vals_view.length()));
  for (int i = 0; i < field_vals_view.length(); ++i) {
    if (field_vals_view[i] == 0.) {
      EXPECT_LT(std::fabs(gold_dat[i]), 1.e-12 / 100.);
    } else {
      EXPECT_LT(std::fabs(1. - gold_dat[i] / field_vals_view[i]),
                1.e-12 / 100.);
    }
  }

  // Test coords correctness
  const RealMatrix field_coords_view = expt_data.field_coords_view(0, 0);
  RealMatrix gold_coords;
  read_coord_values(filename, 1, gold_coords);
  EXPECT_TRUE((gold_coords.numRows() == field_coords_view.numRows()));
  EXPECT_TRUE((gold_coords.numCols() == field_coords_view.numCols()));
  for (int i = 0; i < field_coords_view.numRows(); ++i)
    for (int j = 0; j < field_coords_view.numCols(); ++j) {
      if (field_coords_view(i, j) == 0.) {
        EXPECT_LT(std::fabs(gold_coords(i, j)), 1.e-12 / 100.);
      } else {
        EXPECT_LT(std::fabs(1. - gold_coords(i, j) / field_coords_view(i, j)),
                  1.e-12 / 100.);
      }
    }

  // Test config vars correctness
  // BMA TODO: Need a stronger test here across variable types
  const RealVector& config_vars =
      expt_data.configuration_variables()[0].inactive_continuous_variables();
  EXPECT_TRUE((config_vars.length() == NUM_CONFIG_VARS));

  // Test covariance correctness
  RealVector resid_vals(field_vals_view.length());
  resid_vals = 1.0;
  Real triple_prod = expt_data.apply_covariance(resid_vals, 0);
  // std::cout << "triple_prod = " << triple_prod << std::endl;
  EXPECT_LT(std::fabs(1. - triple_prod / 3.06251e+14), 2.e-4 / 100.);
}

//----------------------------------------------------------------

TEST(exp_data_tests, test_expt_data_twofield) {
  const size_t SECOND_NUM_FIELD_VALUES = 9;

  IntVector field_lengths(NUM_FIELDS + 1);
  field_lengths[0] = NUM_FIELD_VALUES;
  field_lengths[1] = SECOND_NUM_FIELD_VALUES;
  mock_srd.field_lengths(field_lengths);

  StringArray variance_types(NUM_FIELDS + 1);
  variance_types[0] = "diagonal";
  variance_types[1] = "matrix";

  // convention appears to be field label is filename ?
  const std::string first_base_name = "new_voltage";
  const std::string second_base_name = "pressure";
  const std::string working_dir = "../expt_data_test_files";
  StringArray field_labels(NUM_FIELDS + 1);
  field_labels[0] = first_base_name;
  field_labels[1] = second_base_name;
  mock_srd.field_group_labels(field_labels);

  ExperimentData expt_data(NUM_EXPTS, NUM_CONFIG_VARS, working_dir, mock_srd,
                           variance_types, 0 /* SILENT_OUTPUT */);

  //  const std::string config_vars_basename = first_base_name;
  expt_data.load_data("expt_data unit test call", gen_mock_vars());

  // Test general correctness
  EXPECT_TRUE((expt_data.num_experiments() == NUM_EXPTS));
  EXPECT_TRUE((expt_data.num_scalar_primary() == 0));
  EXPECT_TRUE((expt_data.num_fields() == NUM_FIELDS + 1));

  // Test data correctness
  RealVector voltage_vals_view =
      expt_data.field_data_view(0 /* response */, 0 /* experiment */);
  RealVector pressure_vals_view =
      expt_data.field_data_view(1 /* response */, 0 /* experiment */);
  std::string filename1 = working_dir + "/" + first_base_name;
  std::string filename2 = working_dir + "/" + second_base_name;
  RealVector gold_dat1;
  RealVector gold_dat2;
  read_field_values(filename1, 1, gold_dat1);
  read_field_values(filename2, 1, gold_dat2);
  EXPECT_TRUE((gold_dat1.length() == voltage_vals_view.length()));
  EXPECT_TRUE((gold_dat2.length() == pressure_vals_view.length()));
  for (int i = 0; i < voltage_vals_view.length(); ++i) {
    if (voltage_vals_view[i] == 0.) {
      EXPECT_LT(std::fabs(gold_dat1[i]), 1.e-12 / 100.);
    } else {
      EXPECT_LT(std::fabs(1. - gold_dat1[i] / voltage_vals_view[i]),
                1.e-12 / 100.);
    }
  }
  for (int i = 0; i < pressure_vals_view.length(); ++i)
    EXPECT_LT(std::fabs(1. - gold_dat2[i] / pressure_vals_view[i]),
              1.e-12 / 100.);

  // Test coords correctness
  RealMatrix field_coords_view1 = expt_data.field_coords_view(0, 0);
  RealMatrix field_coords_view2 = expt_data.field_coords_view(1, 0);
  RealMatrix gold_coords1;
  RealMatrix gold_coords2;
  read_coord_values(filename1, 1, gold_coords1);
  read_coord_values(filename2, 1, gold_coords2);
  EXPECT_TRUE((gold_coords1.numRows() == field_coords_view1.numRows()));
  EXPECT_TRUE((gold_coords1.numCols() == field_coords_view1.numCols()));
  EXPECT_TRUE((gold_coords2.numRows() == field_coords_view2.numRows()));
  EXPECT_TRUE((gold_coords2.numCols() == field_coords_view2.numCols()));
  for (int i = 0; i < field_coords_view1.numRows(); ++i)
    for (int j = 0; j < field_coords_view1.numCols(); ++j) {
      if (field_coords_view1(i, j) == 0.) {
        EXPECT_LT(std::fabs(gold_coords1(i, j)), 1.e-12 / 100.);
      } else {
        EXPECT_LT(std::fabs(1. - gold_coords1(i, j) / field_coords_view1(i, j)),
                  1.e-12 / 100.);
      }
    }
  for (int i = 0; i < field_coords_view2.numRows(); ++i)
    for (int j = 0; j < field_coords_view2.numCols(); ++j)
      EXPECT_LT(std::fabs(1. - gold_coords2(i, j) / field_coords_view2(i, j)),
                1.e-12 / 100.);

  // Test config vars correctness
  // BMA TODO: Need a stronger test here across variable types
  const RealVector& config_vars =
      expt_data.configuration_variables()[0].inactive_continuous_variables();
  EXPECT_TRUE((config_vars.length() == NUM_CONFIG_VARS));

  // Test covariance correctness
  RealVector resid_vals(voltage_vals_view.length() +
                        pressure_vals_view.length());
  resid_vals = 1.0;
  Real triple_prod = expt_data.apply_covariance(resid_vals, 0);
  // std::cout << "triple_prod = " << triple_prod << std::endl;
  EXPECT_LT(std::fabs(1. - triple_prod / 3.06251e+14), 2.e-4 / 100.);
}

//----------------------------------------------------------------

TEST(exp_data_tests, test_expt_data_allowNoConfigFile) {
  // Create an ExperimentData object that expects NUM_CONFIG_VARS > 0 but
  // that does not have a corresponding experiment.1.config file.
  StringArray variance_types;
  const std::string working_dir = "no_such_dir";
  ExperimentData expt_data(NUM_EXPTS, NUM_CONFIG_VARS, working_dir, mock_srd,
                           variance_types, 0 /* SILENT_OUTPUT */);
  Dakota::abort_mode = ABORT_THROWS;
  EXPECT_THROW(expt_data.load_data("expt_data unit test call", gen_mock_vars()),
               std::runtime_error);
  Dakota::abort_mode = ABORT_EXITS;
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
