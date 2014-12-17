
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"

#include <string>

#include <Teuchos_UnitTestHarness.hpp> 

using namespace Dakota;

namespace {

  const int NUM_EXPTS = 3;
  const int NCV = 4;

  const int NUM_FIELD_VALUES = 9;
  const int VECTOR_FIELD_DIM = 4;

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

TEUCHOS_UNIT_TEST(expt_data_reader, read_bad_config_vars_size)
{
  const std::string base_name = "expt_data_test_files/voltage";

  RealVectorArray config_data;
  TEST_THROW( read_config_vars_multifile( base_name, NUM_EXPTS, NCV+1, config_data), std::runtime_error );
  TEST_THROW( read_config_vars_singlefile(base_name, NUM_EXPTS, NCV+1, config_data), std::runtime_error );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(expt_data_reader, read_field_values_scalar)
{
  const std::string base_name = "expt_data_test_files/voltage";

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
  TEST_EQUALITY( field_vals_array.size(), 1 );
  TEST_EQUALITY( field_vals_array[0].length(), NUM_FIELD_VALUES );
  TEST_EQUALITY( field_vals.length(), NUM_FIELD_VALUES );
  for( size_t i=0; i<NUM_FIELD_VALUES; ++i )
  {
    TEST_FLOATING_EQUALITY( field_vals_array[0][i], gold_values[i], 1.e-14 );
    TEST_FLOATING_EQUALITY( field_vals[i], gold_values[i], 1.e-14 );
  }
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(expt_data_reader, read_field_values_vector)
{
  const std::string base_name = "expt_data_test_files/electric_field";

  RealVectorArray field_vals;
  read_field_values(base_name, 2 /* expt number */, field_vals);

  // Verify equality of field data
  TEST_EQUALITY( field_vals.size(), VECTOR_FIELD_DIM );
  TEST_EQUALITY( field_vals[0].length(), NUM_FIELD_VALUES );
  for( int i=0; i<VECTOR_FIELD_DIM; ++i )
    for( int j=0; j<NUM_FIELD_VALUES; ++j )
      TEST_FLOATING_EQUALITY( field_vals[i][j], double(j+1)+double(j+1)*pow(0.1,i), 1.e-14 );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(expt_data_reader, read_coord_values)
{
  const std::string base_name = "expt_data_test_files/voltage";

  RealMatrix coords;
  read_coord_values(base_name, 2 /* expt number */, coords);

  // Verify equality of coord data
  TEST_EQUALITY( coords.numCols(), VECTOR_FIELD_DIM );
  TEST_EQUALITY( coords.numRows(), NUM_FIELD_VALUES );
  for( int i=0; i<VECTOR_FIELD_DIM; ++i )
    for( int j=0; j<NUM_FIELD_VALUES; ++j )
      TEST_FLOATING_EQUALITY( coords[i][j], double(i)+double(j+1)*0.1, 1.e-14 );
}

//----------------------------------------------------------------
