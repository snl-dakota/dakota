
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"

#include <string>

#include <Teuchos_UnitTestHarness.hpp> 

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
namespace bfs = boost::filesystem;

using namespace Dakota;

namespace {

  const int NUM_EXPTS = 3;
  const int NCV = 4;

}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(expt_data_reader, read_config_vars)
{
  const std::string base_name = "expt_data_test_files/voltage";

  RealVectorArray config_data1;
  read_config_vars_multifile(base_name, NUM_EXPTS, NCV, config_data1);

  RealVectorArray config_data2;
  read_config_vars_singlefile(base_name, NUM_EXPTS, NCV, config_data2);

  // Verify equality of config data
  TEST_EQUALITY( config_data1.size(), config_data2.size() );
  TEST_EQUALITY( config_data1[0].length(), config_data2[0].length() );
  for( size_t i=0; i<config_data1.size(); ++i )
    for( int j=0; j<config_data1[i].length(); ++j )
      TEST_FLOATING_EQUALITY( config_data1[i][j], config_data2[i][j], 1.e-14 );
}

//----------------------------------------------------------------
