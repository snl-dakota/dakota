
#include "ExperimentData.hpp"

#include <string>

#include <Teuchos_UnitTestHarness.hpp> 


using namespace Dakota;

namespace {

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
  IntVector field_lengths(NUM_FIELDS);
  field_lengths[0] = NUM_FIELD_VALUES;
  mock_srd.field_lengths(field_lengths);

  // convention appears to be field label is filename ?
  const std::string base_name = "new_voltage";
  const std::string working_dir = "expt_data_test_files";
  StringArray fn_labels(NUM_FIELD_VALUES); // note that there must be a label for each field value.
  // I would have expected a label for each field (1 in this case).
  // Using 1 instead of NUM_FIELD_VALUES gives the error: Error with function labels in Response::write.
  fn_labels[0] = base_name;
  mock_srd.function_labels(fn_labels);

  ExperimentData expt_data(NUM_EXPTS, NUM_CONFIG_VARS, working_dir, 
			   mock_srd, 0 /* SILENT_OUTPUT */);

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

  const RealVector& config_vars = expt_data.config_vars(0);
  TEST_EQUALITY( config_vars.length(), NUM_CONFIG_VARS );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(expt_data, twofield)
{
  const size_t  SECOND_NUM_FIELD_VALUES = 9;
  const int     SECOND_FIELD_DIM        = 1;

  IntVector field_lengths(NUM_FIELDS+1);
  field_lengths[0] = NUM_FIELD_VALUES;
  field_lengths[1] = SECOND_NUM_FIELD_VALUES;
  mock_srd.field_lengths(field_lengths);

  // convention appears to be field label is filename ?
  const std::string first_base_name = "new_voltage";
  const std::string second_base_name = "pressure";
  const std::string working_dir = "expt_data_test_files";
  StringArray fn_labels(NUM_FIELD_VALUES+SECOND_NUM_FIELD_VALUES); // note that there must be a label for each field value.
  // I would have expected a label for each field (2 in this case).
  // Using 1 instead of NUM_FIELD_VALUES gives the error: Error with function labels in Response::write.
  fn_labels[0] = first_base_name;
  fn_labels[1] = second_base_name;
  mock_srd.function_labels(fn_labels);

  ExperimentData expt_data(NUM_EXPTS, NUM_CONFIG_VARS, working_dir, 
			   mock_srd, 0 /* SILENT_OUTPUT */);

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

  const RealVector& config_vars = expt_data.config_vars(0);
  TEST_EQUALITY( config_vars.length(), NUM_CONFIG_VARS );
}

