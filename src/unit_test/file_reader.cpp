
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"

#include <string>

#include <Teuchos_UnitTestHarness.hpp> 

using namespace Dakota;

namespace {

  std::vector<std::string> used_filenames;

  RealVectorArray 
    create_test_array(int num_entries, int dim = 1, bool rand = false) {
      RealVectorArray test_array(num_entries);

      for( int i = 0; i < num_entries; ++i ) {
        RealVector & test_vec = test_array[i];
        test_vec.sizeUninitialized(dim);
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

  class Cleanup_Helper {
    public:
      Cleanup_Helper() { }

      ~Cleanup_Helper() { 
        remove_field_data_files();
      }
  };

}

// Automatically deletes generated test data files
// Could make deletion optional to allow the files to remain if running with verbosity enabled
Cleanup_Helper cleanup_helper;

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

TEUCHOS_UNIT_TEST(file_reader, read_sized_data)
{
  const int DATA_PTS = 10;
  const int DATA_DIM = 3;
  const std::string filename("test_sized_data");
  RealVectorArray field_data = create_test_array(DATA_PTS, DATA_DIM, true);
  create_field_data_file(filename, field_data);

  std::ifstream in_file;
  TabularIO::open_file(in_file, filename, "unit test read_sized_data");

  RealVectorArray test_vec_array;
  // This tests the ability for the reader to handle formerly populated arrays
  RealVector dummy_vec(DATA_PTS+5);
  dummy_vec.random();
  test_vec_array.push_back(dummy_vec);
  /////////////////  What we want to test
  read_sized_data(in_file, test_vec_array, DATA_PTS, DATA_DIM);
  /////////////////  What we want to test

  // Verify contents of what we wrote and what we read
  for( size_t i=0; i<test_vec_array.size(); ++i )
    for( int j=0; j<test_vec_array[i].length(); ++j )
    TEST_FLOATING_EQUALITY( field_data[i][j], test_vec_array[i][j], 1.e-14 );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(file_reader, read_fixed_rowsize_data)
{
  const int NUM_RESP = 7;
  const int RESP_DIM = 2;
  const std::string filename("test_rowsized_data");
  RealVectorArray field_data = create_test_array(NUM_RESP, RESP_DIM, true);
  create_field_data_file(filename, field_data);

  std::ifstream in_file;
  TabularIO::open_file(in_file, filename, "unit test test_rowsized_data");

  RealVectorArray test_vec_array;
  // This tests the ability for the reader to handle formerly populated arrays
  RealVector dummy_vec(NUM_RESP+5);
  dummy_vec.random();
  test_vec_array.push_back(dummy_vec);
  /////////////////  What we want to test
  read_fixed_rowsize_data(in_file, test_vec_array, RESP_DIM);
  /////////////////  What we want to test

  // Verify we obtained the correct number of rows (responses)
  TEST_EQUALITY( NUM_RESP, test_vec_array.size() );

  // Verify contents of what we wrote and what we read
  for( size_t i=0; i<test_vec_array.size(); ++i )
    for( int j=0; j<test_vec_array[i].length(); ++j )
    TEST_FLOATING_EQUALITY( field_data[i][j], test_vec_array[i][j], 1.e-14 );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(file_reader, read_unsized_data)
{
  const int NUM_RESP = 9;
  const int RESP_DIM = 4;
  const std::string filename("test_unsized_data");
  RealVectorArray field_data = create_test_array(NUM_RESP, RESP_DIM, true);
  create_field_data_file(filename, field_data);

  std::ifstream in_file;
  TabularIO::open_file(in_file, filename, "unit test test_unsized_data");

  RealVectorArray test_vec_array;
  // This tests the ability for the reader to handle formerly populated arrays
  RealVector dummy_vec(NUM_RESP+5);
  dummy_vec.random();
  test_vec_array.push_back(dummy_vec);
  /////////////////  What we want to test
  read_unsized_data(in_file, test_vec_array);
  /////////////////  What we want to test

  // Verify we obtained the correct number of rows (responses)
  TEST_EQUALITY( NUM_RESP, test_vec_array.size() );

  // Verify contents of what we wrote and what we read
  for( size_t i=0; i<test_vec_array.size(); ++i )
    for( int j=0; j<test_vec_array[i].length(); ++j )
    TEST_FLOATING_EQUALITY( field_data[i][j], test_vec_array[i][j], 1.e-14 );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(file_reader, read_sigma_scalar)
{
  const int NUM_RESP = 1;
  const int RESP_DIM = 1;
  const std::string filename("test_sigma_scalar_data");
  RealVectorArray field_data = create_test_array(NUM_RESP, RESP_DIM, true);
  create_field_data_file(filename, field_data);

  std::ifstream in_file;
  TabularIO::open_file(in_file, filename, "unit test test_sigma_scalar_data");

  RealVectorArray test_vec_array;
  /////////////////  What we want to test
  read_unsized_data(in_file, test_vec_array);
  /////////////////  What we want to test

  // Verify we obtained the correct number of rows and cols (responses)
  TEST_EQUALITY( NUM_RESP, test_vec_array.size() );
  TEST_EQUALITY( RESP_DIM, test_vec_array[0].length() );

  // Verify contents of what we wrote and what we read
  TEST_FLOATING_EQUALITY( field_data[0][0], test_vec_array[0][0], 1.e-14 );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(file_reader, read_sigma_vector_row)
{
  const int NUM_ROW = 1;
  const int NUM_COL = 7;
  const std::string filename("test_sigma_vector_row_data");
  RealVectorArray field_data = create_test_array(NUM_ROW, NUM_COL, true);
  create_field_data_file(filename, field_data);

  std::ifstream in_file;
  TabularIO::open_file(in_file, filename, "unit test test_sigma_vector_row_data");

  RealVectorArray test_vec_array;
  /////////////////  What we want to test
  read_unsized_data(in_file, test_vec_array);
  /////////////////  What we want to test

  // Verify we obtained the correct number of rows and cols (responses)
  TEST_EQUALITY( NUM_ROW, test_vec_array.size() );
  TEST_EQUALITY( NUM_COL, test_vec_array[0].length() );

  // Verify contents of what we wrote and what we read
  for( size_t i=0; i<test_vec_array.size(); ++i )
    for( int j=0; j<test_vec_array[i].length(); ++j )
      TEST_FLOATING_EQUALITY( field_data[i][j], test_vec_array[i][j], 1.e-14 );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(file_reader, read_sigma_vector_col)
{
  const int NUM_ROW = 7;
  const int NUM_COL = 1;
  const std::string filename("test_sigma_vector_col_data");
  RealVectorArray field_data = create_test_array(NUM_ROW, NUM_COL, true);
  create_field_data_file(filename, field_data);

  std::ifstream in_file;
  TabularIO::open_file(in_file, filename, "unit test test_sigma_vector_col_data");

  RealVectorArray test_vec_array;
  /////////////////  What we want to test
  read_unsized_data(in_file, test_vec_array);
  /////////////////  What we want to test

  // Verify we obtained the correct number of rows (responses)
  TEST_EQUALITY( NUM_ROW, test_vec_array.size() );

  // Verify contents of what we wrote and what we read
  for( size_t i=0; i<test_vec_array.size(); ++i )
    for( int j=0; j<test_vec_array[i].length(); ++j )
      TEST_FLOATING_EQUALITY( field_data[i][j], test_vec_array[i][j], 1.e-14 );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(file_reader, read_sigma_matrix)
{
  const int NUM_ROW = 5;
  const int NUM_COL = 5;
  const std::string filename("test_sigma_matrix_data");
  RealVectorArray field_data = create_test_array(NUM_ROW, NUM_COL, true);
  create_field_data_file(filename, field_data);

  std::ifstream in_file;
  TabularIO::open_file(in_file, filename, "unit test test_sigma_matrix_data");

  RealVectorArray test_vec_array;
  /////////////////  What we want to test
  read_unsized_data(in_file, test_vec_array);
  /////////////////  What we want to test

  // Verify we obtained the correct number of rows (responses)
  TEST_EQUALITY( NUM_ROW, test_vec_array.size() );
  TEST_EQUALITY( NUM_COL, test_vec_array[0].length() );

  // Verify contents of what we wrote and what we read
  for( int i=0; i<NUM_ROW; ++i )
    for( int j=0; j<NUM_COL; ++j )
      TEST_FLOATING_EQUALITY( field_data[i][j], test_vec_array[i][j], 1.e-14 );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(file_reader, read_bad_data1)
{
  const int NUM_ROW = 5;
  const int NUM_COL = 3;
  const std::string filename("test_bad_data1");
  RealVectorArray field_data = create_test_array(NUM_ROW, NUM_COL, true);

  // Modify by adding another row of too large length (# cols)
  // We should get a thrown exception which we test for later
  RealVector extra_vec(NUM_COL+1);
  extra_vec.random();
  field_data.push_back(extra_vec);
  create_field_data_file(filename, field_data);

  std::ifstream in_file;
  TabularIO::open_file(in_file, filename, "unit test test_bad_data1");

  RealVectorArray test_vec_array;
  /////////////////  What we want to test
  TEST_THROW(read_unsized_data(in_file, test_vec_array), std::runtime_error);
  /////////////////  What we want to test
}

//----------------------------------------------------------------
