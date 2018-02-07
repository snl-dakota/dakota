
#include "dakota_data_util.hpp"
#include "dakota_data_io.hpp"

#include <string>

#include <Teuchos_UnitTestHarness.hpp> 

using namespace Dakota;

namespace {

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
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(data_conversion, rva2rm)
{
  const int NROWS = 5;
  const int NCOLS = 3;
  RealVectorArray test_rva = create_test_array(NROWS, NCOLS, true);
  RealMatrix test_rm;

  /////////////////  What we want to test
  copy_data(test_rva, test_rm);
  /////////////////  What we want to test

  // Verify correct dimensions
  TEST_EQUALITY( NROWS, test_rm.numRows() );
  TEST_EQUALITY( NCOLS, test_rm.numCols() );

  // Verify contents of what we wrote and what we read
  for( size_t i=0; i<NROWS; ++i )
    for( int j=0; j<NCOLS; ++j )
    TEST_FLOATING_EQUALITY( test_rva[i][j], test_rm(i,j), 1.e-14 );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(data_conversion, apply_matrix)
{
  const int NROWS = 5;
  const int NCOLS = 3;
  RealMatrix mat(NROWS, NCOLS);
  mat.random();

  std::vector<Real> v1(NCOLS, 1.0);
  std::vector<Real> v2;

  /////////////////  What we want to test
  apply_matrix_partial(mat, v1, v2);
  /////////////////  What we want to test

  // Verify correct dimensions
  TEST_EQUALITY( NROWS, v2.size() );

  // Verify correct values
  Real sum = 0.0;
  for( size_t i=0; i<NROWS; ++i )
  {
    sum = 0.0;
    for( int j=0; j<NCOLS; ++j )
      sum += mat(i,j);
    TEST_FLOATING_EQUALITY( sum, v2[i], 1.e-14 );
  }

  // Now test partial behavior

  std::vector<Real> v3(2*NCOLS, 2.0);
  std::vector<Real> v4(2*NROWS);

  for( size_t i=0; i<v4.size(); ++i )
    v4[i] = (Real)i;

  /////////////////  What we want to test
  apply_matrix_partial(mat, v3, v4);
  /////////////////  What we want to test

  // Verify correct dimensions
  TEST_EQUALITY( 2*NROWS, v4.size() );

  // Verify correct values
  for( size_t i=0; i<v4.size(); ++i )
  {
    if( i<NROWS )
    {
      sum = 0.0;
      for( int j=0; j<NCOLS; ++j )
        sum += mat(i,j);
      TEST_FLOATING_EQUALITY( 2.0*sum, v4[i], 1.e-14 );
    }
    else
      TEST_FLOATING_EQUALITY( (Real)i, v4[i], 1.e-14 );
  }
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(data_conversion, apply_matrix_transpose)
{
  const int NROWS = 5;
  const int NCOLS = 3;
  RealMatrix mat(NROWS, NCOLS);
  mat.random();

  std::vector<Real> v1(NROWS, 1.0);
  std::vector<Real> v2;

  /////////////////  What we want to test
  apply_matrix_transpose_partial(mat, v1, v2);
  /////////////////  What we want to test

  // Verify correct dimensions
  TEST_EQUALITY( NCOLS, v2.size() );

  // Verify correct values
  Real sum = 0.0;
  for( size_t i=0; i<NCOLS; ++i )
  {
    sum = 0.0;
    for( int j=0; j<NROWS; ++j )
      sum += mat(j,i);
    TEST_FLOATING_EQUALITY( sum, v2[i], 1.e-14 );
  }

  // Now test partial behavior

  std::vector<Real> v3(2*NROWS, 2.0);
  std::vector<Real> v4(2*NCOLS);

  for( size_t i=0; i<v4.size(); ++i )
    v4[i] = (Real)i;

  /////////////////  What we want to test
  apply_matrix_transpose_partial(mat, v3, v4);
  /////////////////  What we want to test

  // Verify correct dimensions
  TEST_EQUALITY( 2*NCOLS, v4.size() );

  // Verify correct values
  for( size_t i=0; i<v4.size(); ++i )
  {
    if( i<NCOLS )
    {
      sum = 0.0;
      for( int j=0; j<NROWS; ++j )
        sum += mat(j,i);
      TEST_FLOATING_EQUALITY( 2.0*sum, v4[i], 1.e-14 );
    }
    else
      TEST_FLOATING_EQUALITY( (Real)i, v4[i], 1.e-14 );
  }
}

//----------------------------------------------------------------
