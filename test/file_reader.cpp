
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"

#include <string>

#include <Teuchos_UnitTestHarness.hpp> 

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
namespace bfs = boost::filesystem;

using namespace Dakota;

namespace {

  std::vector<std::string> used_filenames;

  RealVectorArray 
    create_test_array(int num_entries, int dim = 1, bool rand = false) {
      RealVectorArray test_array(num_entries);

      for( int i = 0; i < num_entries; ++i ) {
        RealVector & test_vec = test_array[i];
        test_vec.size(dim);
        if( rand )
          test_vec.random();
        // For now, just fill values using counting numbers - user can modify afterward
        else
          for( int i=0; i < test_vec.length(); ++i )
            test_vec(i) = double(i);
      }
      return test_array;
    }


  void 
    create_field_data_file(const std::string & filename, const RealVectorArray & field_data) {

      //if( bfs::is_regular_file(filename) )
      //  return;

      std::ofstream out_file;
      TabularIO::open_file(out_file, filename, "unit test write");
      out_file << std::setprecision(15) << std::resetiosflags(std::ios::floatfield);

      for( size_t i=0; i<field_data.size(); ++i)
      {
        for( int j=0; j<field_data[i].length(); ++j)
          out_file << field_data[i][j] << "\t";
        out_file << "\n";
      }
      out_file.flush();
      out_file.close();
      used_filenames.push_back(filename);
    }

  void 
    remove_field_data_files() {

      for( size_t i=0; i<used_filenames.size(); ++i)
       if( remove(used_filenames[i].c_str()) != 0 )
        std::cout << "Error removing test data file \"" << used_filenames[i] << "\"" << std::endl; 
    }
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(file_reader, basic_write)
{
  const int DATA_PTS = 5;
  const int DATA_DIM = 3;
  RealVectorArray field_data = create_test_array(DATA_PTS, DATA_DIM, true);
  create_field_data_file("test_table_out", field_data);

  // Test that we made it to this point
  TEST_ASSERT( true );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(file_reader, read_config_data)
{
  const int DATA_PTS = 10;
  const int DATA_DIM = 3;
  const std::string filename("test_config_data");
  RealVectorArray field_data = create_test_array(DATA_PTS, DATA_DIM, true);
  create_field_data_file(filename, field_data);

  std::ifstream in_file;
  TabularIO::open_file(in_file, filename, "unit test read_config_data");

  RealVectorArray test_vec_array;
  /////////////////  What we want to test
  read_configuration_data(in_file, test_vec_array, DATA_PTS, DATA_DIM);
  /////////////////  What we want to test

  // Verify contents of what we wrote and what we read
  for( size_t i=0; i<test_vec_array.size(); ++i )
    for( int j=0; j<test_vec_array[i].length(); ++j )
    TEST_FLOATING_EQUALITY( field_data[i][j], test_vec_array[i][j], 1.e-14 );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(file_reader, read_functional_data)
{
  const int NUM_RESP = 7;
  const int RESP_DIM = 2;
  const std::string filename("test_function_data");
  RealVectorArray field_data = create_test_array(NUM_RESP, RESP_DIM, true);
  create_field_data_file(filename, field_data);

  std::ifstream in_file;
  TabularIO::open_file(in_file, filename, "unit test test_function_data");

  RealVectorArray test_vec_array;
  /////////////////  What we want to test
  read_functional_data(in_file, test_vec_array, RESP_DIM);
  /////////////////  What we want to test

  // Verify we obtained the correct number of rows (responses)
  TEST_EQUALITY( NUM_RESP, test_vec_array.size() );

  // Verify contents of what we wrote and what we read
  for( size_t i=0; i<test_vec_array.size(); ++i )
    for( int j=0; j<test_vec_array[i].length(); ++j )
    TEST_FLOATING_EQUALITY( field_data[i][j], test_vec_array[i][j], 1.e-14 );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(file_reader, read_coordinate_data)
{
  const int NUM_RESP = 9;
  const int RESP_DIM = 4;
  const std::string filename("test_coordinate_data");
  RealVectorArray field_data = create_test_array(NUM_RESP, RESP_DIM, true);
  create_field_data_file(filename, field_data);

  std::ifstream in_file;
  TabularIO::open_file(in_file, filename, "unit test test_coordinate_data");

  RealVectorArray test_vec_array;
  /////////////////  What we want to test
  read_coordinate_data(in_file, test_vec_array);
  /////////////////  What we want to test

  // Verify we obtained the correct number of rows (responses)
  TEST_EQUALITY( NUM_RESP, test_vec_array.size() );

  // Verify contents of what we wrote and what we read
  for( size_t i=0; i<test_vec_array.size(); ++i )
    for( int j=0; j<test_vec_array[i].length(); ++j )
    TEST_FLOATING_EQUALITY( field_data[i][j], test_vec_array[i][j], 1.e-14 );

  // The last can call this to remove the test data files (or perhaps
  // leave them if verbose flag is on...)
  remove_field_data_files();
}

//----------------------------------------------------------------
