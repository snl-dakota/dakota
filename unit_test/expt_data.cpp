
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
  StringArray fn_labels(NUM_FIELD_VALUES);
  fn_labels[0] = base_name;
  mock_srd.function_labels(fn_labels);
  expt_data.shared_data(mock_srd);


  expt_data.load_data(base_name, 
                      "expt_data unit test call", 
                      true /* expDataFileAnnotated is N/A with new readers for now */,
                      true /* calc_sigma_from_data is N/A with new readers for now */,
                      false /* = sclar_data_file, signifies use of new data readers */);


  const RealVector& config_vars = expt_data.config_vars(0);
  TEST_EQUALITY( config_vars.length(), NUM_CONFIG_VARS );
}

