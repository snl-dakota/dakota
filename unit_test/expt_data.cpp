
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
  ExperimentData expt_data(0 /* SILENT_OUTPUT */);
  expt_data.num_experiments(NUM_EXPTS);
  expt_data.num_config_vars(NUM_CONFIG_VARS);
  IntVector field_lengths(NUM_FIELDS);
  field_lengths[0] = NUM_FIELD_VALUES;
  mock_srd.field_lengths(field_lengths);

  // convention appears to be field label is filename ?
  const std::string base_name = "expt_data_test_files/new_voltage";
  StringArray fn_labels(NUM_FIELD_VALUES); // note that there must be a label for each field value.
  // I would have expected a label for each field (1 in this case).
  // Using 1 instead of NUM_FIELD_VALUES gives the error: Error with function labels in Response::write.
  fn_labels[0] = base_name;
  mock_srd.function_labels(fn_labels);
  expt_data.shared_data(mock_srd);


  const std::string config_vars_basename = base_name;
  expt_data.load_data(config_vars_basename, 
                      "expt_data unit test call", 
                      true /* expDataFileAnnotated is N/A with new readers for now */,
                      true /* calc_sigma_from_data is N/A with new readers for now */,
                      false /* = sclar_data_file, signifies use of new data readers */);


  const RealVector& config_vars = expt_data.config_vars(0);
  TEST_EQUALITY( config_vars.length(), NUM_CONFIG_VARS );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(expt_data, twofield)
{
  const size_t  SECOND_NUM_FIELD_VALUES = 9;
  const int     SECOND_FIELD_DIM        = 1;

  ExperimentData expt_data(0 /* SILENT_OUTPUT */);
  expt_data.num_experiments(NUM_EXPTS);
  expt_data.num_config_vars(NUM_CONFIG_VARS);
  IntVector field_lengths(NUM_FIELDS+1);
  field_lengths[0] = NUM_FIELD_VALUES;
  field_lengths[1] = SECOND_NUM_FIELD_VALUES;
  mock_srd.field_lengths(field_lengths);

  // convention appears to be field label is filename ?
  const std::string first_base_name = "expt_data_test_files/new_voltage";
  const std::string second_base_name = "expt_data_test_files/pressure";
  StringArray fn_labels(NUM_FIELD_VALUES+SECOND_NUM_FIELD_VALUES); // note that there must be a label for each field value.
  // I would have expected a label for each field (2 in this case).
  // Using 1 instead of NUM_FIELD_VALUES gives the error: Error with function labels in Response::write.
  fn_labels[0] = first_base_name;
  fn_labels[1] = second_base_name;
  mock_srd.function_labels(fn_labels);
  expt_data.shared_data(mock_srd);


  const std::string config_vars_basename = first_base_name;
  expt_data.load_data(config_vars_basename, 
                      "expt_data unit test call", 
                      true /* expDataFileAnnotated is N/A with new readers for now */,
                      true /* calc_sigma_from_data is N/A with new readers for now */,
                      false /* = sclar_data_file, signifies use of new data readers */);


  const RealVector& config_vars = expt_data.config_vars(0);
  TEST_EQUALITY( config_vars.length(), NUM_CONFIG_VARS );
}

