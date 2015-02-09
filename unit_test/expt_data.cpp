
#include "ExperimentData.hpp"
#include "dakota_data_io.hpp"

#include <string>

#include <Teuchos_UnitTestHarness.hpp> 


using namespace Dakota;

namespace {

  // Base case specifications
  const int     NUM_EXPTS        = 1;
  const int     NUM_CONFIG_VARS  = 3;
  const size_t  NUM_FIELDS       = 1;
  const size_t  NUM_FIELD_VALUES = 401;
  const int     FIELD_DIM        = 1;

  ActiveSet mock_as(0); // arg specifies num scalar values
  SharedResponseData mock_srd(mock_as);

}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(expt_data, basic)
{
  // create an SRD with 0 scalars and 1 field of length NUM_FIELD_VALUES
  IntVector field_lengths(NUM_FIELDS);
  field_lengths[0] = NUM_FIELD_VALUES;
  mock_srd.field_lengths(field_lengths);

  StringArray variance_types(NUM_FIELDS);
  variance_types[0] = "diagonal";

  // convention appears to be field label is filename ?
  const std::string base_name = "new_voltage";
  const std::string working_dir = "expt_data_test_files";
  StringArray field_labels(NUM_FIELDS); 
  field_labels[0] = base_name;
  mock_srd.field_group_labels(field_labels);

  ExperimentData expt_data(NUM_EXPTS, NUM_CONFIG_VARS, working_dir, 
			   mock_srd, variance_types, 0 /* SILENT_OUTPUT */);

  //  const std::string config_vars_basename = base_name;
  expt_data.load_data("expt_data unit test call", 
                      true /* calc_sigma_from_data is N/A with new readers for now */);

  // Test general correctness
  TEST_EQUALITY( expt_data.num_experiments(), NUM_EXPTS );
  TEST_EQUALITY( expt_data.num_scalars(), 0 );
  TEST_EQUALITY( expt_data.num_fields(), NUM_FIELDS );

  // Test data correctness
  RealVector field_vals_view = expt_data.field_data_view(0, 0);
  std::string filename = working_dir + "/" + base_name;
  RealVector gold_dat;
  read_field_values(filename, 1, gold_dat);
  TEST_EQUALITY( gold_dat.length(), field_vals_view.length() );
  for( int i=0; i<field_vals_view.length(); ++i )
    TEST_FLOATING_EQUALITY( gold_dat[i], field_vals_view[i], 1.e-14 );

  // Test coords correctness
  RealMatrix field_coords_view = expt_data.field_coords_view(0, 0);
  RealMatrix gold_coords;
  read_coord_values(filename, 1, gold_coords);
  TEST_EQUALITY( gold_coords.numRows(), field_coords_view.numRows() );
  TEST_EQUALITY( gold_coords.numCols(), field_coords_view.numCols() );
  for( int i=0; i<field_coords_view.numRows(); ++i )
    for( int j=0; j<field_coords_view.numCols(); ++j )
      TEST_FLOATING_EQUALITY( gold_coords(i,j), field_coords_view(i,j), 1.e-14 );

  // Test config vars correctness
  const RealVector& config_vars = expt_data.config_vars(0);
  TEST_EQUALITY( config_vars.length(), NUM_CONFIG_VARS );

  // Test covariance correctness
  RealVector resid_vals(field_vals_view.length());
  resid_vals = 1.0;
  Real triple_prod = expt_data.apply_covariance(resid_vals, 0);
  //std::cout << "triple_prod = " << triple_prod << std::endl;
  TEST_FLOATING_EQUALITY( triple_prod, 3.06251e+14, 1.e9 );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(expt_data, twofield)
{
  const size_t  SECOND_NUM_FIELD_VALUES = 9;

  IntVector field_lengths(NUM_FIELDS+1);
  field_lengths[0] = NUM_FIELD_VALUES;
  field_lengths[1] = SECOND_NUM_FIELD_VALUES;
  mock_srd.field_lengths(field_lengths);

  StringArray variance_types(NUM_FIELDS+1);
  variance_types[0] = "diagonal";
  variance_types[1] = "matrix";

  // convention appears to be field label is filename ?
  const std::string first_base_name = "new_voltage";
  const std::string second_base_name = "pressure";
  const std::string working_dir = "expt_data_test_files";
  StringArray field_labels(NUM_FIELDS+1);
  field_labels[0] = first_base_name;
  field_labels[1] = second_base_name;
  mock_srd.field_group_labels(field_labels);

  ExperimentData expt_data(NUM_EXPTS, NUM_CONFIG_VARS, working_dir, 
			   mock_srd, variance_types, 0 /* SILENT_OUTPUT */);

  //  const std::string config_vars_basename = first_base_name;
  expt_data.load_data("expt_data unit test call", 
                      true /* calc_sigma_from_data is N/A with new readers for now */);

  // Test general correctness
  TEST_EQUALITY( expt_data.num_experiments(), NUM_EXPTS );
  TEST_EQUALITY( expt_data.num_scalars(), 0 );
  TEST_EQUALITY( expt_data.num_fields(), NUM_FIELDS+1 );

  // Test data correctness
  RealVector voltage_vals_view = expt_data.field_data_view(0 /* response */, 0 /* experiment */);
  RealVector pressure_vals_view = expt_data.field_data_view(1 /* response */, 0 /* experiment */);
  std::string filename1= working_dir + "/" + first_base_name;
  std::string filename2= working_dir + "/" + second_base_name;
  RealVector gold_dat1;
  RealVector gold_dat2;
  read_field_values(filename1, 1, gold_dat1);
  read_field_values(filename2, 1, gold_dat2);
  TEST_EQUALITY( gold_dat1.length(), voltage_vals_view.length() );
  TEST_EQUALITY( gold_dat2.length(), pressure_vals_view.length() );
  for( int i=0; i<voltage_vals_view.length(); ++i )
    TEST_FLOATING_EQUALITY( gold_dat1[i], voltage_vals_view[i], 1.e-14 );
  for( int i=0; i<pressure_vals_view.length(); ++i )
    TEST_FLOATING_EQUALITY( gold_dat2[i], pressure_vals_view[i], 1.e-14 );

  // Test coords correctness
  RealMatrix field_coords_view1 = expt_data.field_coords_view(0, 0);
  RealMatrix field_coords_view2 = expt_data.field_coords_view(1, 0);
  RealMatrix gold_coords1;
  RealMatrix gold_coords2;
  read_coord_values(filename1, 1, gold_coords1);
  read_coord_values(filename2, 1, gold_coords2);
  TEST_EQUALITY( gold_coords1.numRows(), field_coords_view1.numRows() );
  TEST_EQUALITY( gold_coords1.numCols(), field_coords_view1.numCols() );
  TEST_EQUALITY( gold_coords2.numRows(), field_coords_view2.numRows() );
  TEST_EQUALITY( gold_coords2.numCols(), field_coords_view2.numCols() );
  for( int i=0; i<field_coords_view1.numRows(); ++i )
    for( int j=0; j<field_coords_view1.numCols(); ++j )
      TEST_FLOATING_EQUALITY( gold_coords1(i,j), field_coords_view1(i,j), 1.e-14 );
  for( int i=0; i<field_coords_view2.numRows(); ++i )
    for( int j=0; j<field_coords_view2.numCols(); ++j )
      TEST_FLOATING_EQUALITY( gold_coords2(i,j), field_coords_view2(i,j), 1.e-14 );

  // Test config vars correctness
  const RealVector& config_vars = expt_data.config_vars(0);
  TEST_EQUALITY( config_vars.length(), NUM_CONFIG_VARS );
  //
  // Test covariance correctness
  RealVector resid_vals(voltage_vals_view.length() + pressure_vals_view.length());
  resid_vals = 1.0;
  Real triple_prod = expt_data.apply_covariance(resid_vals, 0);
  //std::cout << "triple_prod = " << triple_prod << std::endl;
  TEST_FLOATING_EQUALITY( triple_prod, 3.06251e+14, 1.e9 );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(expt_data, mixedfield)
{
  // Mixed case specifications
  //const size_t  NUM_FIELDS       = 1;
  //const size_t  NUM_FIELD_VALUES = 401;
  //const int     FIELD_DIM        = 1;

  ActiveSet mock_mixed_as(2); // arg specifies num scalar values
  SharedResponseData mock_mixed_srd(mock_mixed_as);

  const size_t  MIXED_NUM_FIELDS = 4;

  IntVector field_lengths(MIXED_NUM_FIELDS);
  field_lengths[0] = 4;
  field_lengths[1] = 2;
  field_lengths[2] = 3;
  field_lengths[3] = 2;
  mock_mixed_srd.field_lengths(field_lengths);

  StringArray variance_types(MIXED_NUM_FIELDS+2);
  variance_types[0] = "scalar"; // first scalar non-field
  variance_types[1] = "none";   // second scalar non-field
  variance_types[2] = "none";   // first field, etc.
  variance_types[3] = "matrix";
  variance_types[4] = "scalar";
  variance_types[5] = "diagonal";

  // convention is first field label is filename
  StringArray field_labels(MIXED_NUM_FIELDS);
  std::vector<std::string> base_names(MIXED_NUM_FIELDS);
  for( size_t i=0; i < MIXED_NUM_FIELDS; ++i )
  {
    base_names[i] = "mock_field_" + convert_to_string(i);
    field_labels[i] = base_names[i];
  }
  mock_mixed_srd.field_group_labels(field_labels);
  const std::string working_dir = "expt_data_test_files";

  ExperimentData expt_data(NUM_EXPTS, NUM_CONFIG_VARS, working_dir, 
			   mock_mixed_srd, variance_types, 0 /* SILENT_OUTPUT */);

  //  const std::string config_vars_basename = first_base_name;
  expt_data.load_data("expt_data unit test call", 
                      true /* calc_sigma_from_data is N/A with new readers for now */);

  // Test general correctness
  TEST_EQUALITY( expt_data.num_experiments(), NUM_EXPTS );
  TEST_EQUALITY( expt_data.num_scalars(), 2 );
  TEST_EQUALITY( expt_data.num_fields(), MIXED_NUM_FIELDS );

  // Test data correctness
  int total_response_length = 2; // start with non-field scalars
  for( int i=0; i<MIXED_NUM_FIELDS; ++i )
  {
    RealVector mock_data_copy(expt_data.field_data_view(i /* response */, 0 /* experiment */));
    std::string filename= working_dir + "/" + base_names[i];
    RealVector gold_dat;
    read_field_values(filename, 1, gold_dat);
    TEST_EQUALITY( gold_dat.length(), mock_data_copy.length() );
    for( int i=0; i<gold_dat.length(); ++i )
      TEST_FLOATING_EQUALITY( gold_dat[i], mock_data_copy[i], 1.e-14 );

    total_response_length += mock_data_copy.length(); // add field data lengths
  }

  // Test coords correctness
  for( int i=0; i<MIXED_NUM_FIELDS; ++i )
  {
    RealMatrix field_coords_view(expt_data.field_coords_view(i /* response */, 0 /* experiment */));
    TEST_ASSERT( field_coords_view.empty() );
  }

  // Test covariance correctness
  RealVector resid_vals(total_response_length);
  resid_vals = 1.0;
  Real triple_prod = expt_data.apply_covariance(resid_vals, 0);
  //std::cout << "triple_prod = " << triple_prod << std::endl;
  TEST_FLOATING_EQUALITY( triple_prod, 11.1721, 1.e-4 );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(expt_data, disallowOldAndNewReads)
{
  // Create an invalid ExperimentData object that combines old-style scalar data
  // and new-style field-data readers. 
  StringArray variance_types(1);
  TEST_THROW( 
      ExperimentData expt_data(NUM_EXPTS, NUM_CONFIG_VARS, "" /* working_dir */, 
                               mock_srd, variance_types, 0 /* SILENT_OUTPUT */,
                               "dummyscalarDataFilename"),
      std::runtime_error );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(expt_data, allowNoConfigFile)
{
  // Create an ExperimentData object that expects NUM_CONFIG_VARS > 0 but
  // that does not have a corresponding experiment.1.confif file.
  StringArray variance_types;
  const std::string working_dir = "no_such_dir";
  ExperimentData expt_data(NUM_EXPTS, NUM_CONFIG_VARS, working_dir, 
			   mock_srd, variance_types, 0 /* SILENT_OUTPUT */);
  TEST_THROW( 
      expt_data.load_data("expt_data unit test call", 
        true /* calc_sigma_from_data is N/A with new readers for now */),
      std::runtime_error );
}
