/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


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

TEUCHOS_UNIT_TEST(data_conversion, mat2mat)
{
  const int NROWS = 5;
  const int NCOLS = 3;
  RealMatrix test_mat(NROWS, NCOLS);
  test_mat.random();
  RealMatrix dest_mat;

  /////////////////  What we want to test
  copy_data(test_mat, dest_mat);
  /////////////////  What we want to test

  // Verify correct dimensions
  TEST_EQUALITY( NROWS, dest_mat.numRows() );
  TEST_EQUALITY( NCOLS, dest_mat.numCols() );

  // Verify contents of what we wrote and what we read
  for( size_t i=0; i<NROWS; ++i )
    for( int j=0; j<NCOLS; ++j )
    TEST_FLOATING_EQUALITY( test_mat(i,j), dest_mat(i,j), 1.e-14 );

  // Verifiy that the copy is a deep copy
  dest_mat(1,1) *= 1.5;
  double diff = dest_mat(1,1) - test_mat(1,1);
  TEST_FLOATING_EQUALITY( diff, 0.5*test_mat(1,1), 1.e-14 );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(data_conversion, mat2mat_sub)
{
  const int NROWS = 5;
  const int NCOLS = 3;
  RealMatrix test_mat(NROWS, NCOLS);
  test_mat.random();
  RealMatrix dest_mat;

  int roffset = 2;
  int coffset = 1;

  /////////////////  What we want to test
  copy_data(test_mat, dest_mat,
            /* num_rows */ 3,
            /* num_cols */ 2,
            roffset, coffset );
  /////////////////  What we want to test

  // Verify correct dimensions
  TEST_EQUALITY( 3, dest_mat.numRows() );
  TEST_EQUALITY( 2, dest_mat.numCols() );

  // Verify contents of what we wrote and what we read
  for( int i=0; i<dest_mat.numRows(); ++i )
    for( int j=0; j<dest_mat.numCols(); ++j )
    TEST_FLOATING_EQUALITY( test_mat(i+roffset,j+coffset), dest_mat(i,j), 1.e-14 );

  // Verifiy that the copy is a deep copy
  dest_mat(1,1) *= 1.5;
  double diff = dest_mat(1,1) - test_mat(1+roffset,1+coffset);
  TEST_FLOATING_EQUALITY( diff, 0.5*test_mat(1+roffset,1+coffset), 1.e-14 );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(data_conversion, mat2mat_sym)
{
  const int NROWS = 3;
  const int NCOLS = NROWS;
  RealSymMatrix test_mat(NROWS, NCOLS);
  test_mat.random();
  RealSymMatrix dest_mat;

  /////////////////  What we want to test
  copy_data(test_mat, dest_mat);
  /////////////////  What we want to test

  // Verify correct dimensions
  TEST_EQUALITY( NROWS, dest_mat.numRows() );
  TEST_EQUALITY( NCOLS, dest_mat.numCols() );

  // Verify contents of what we wrote and what we read
  for( size_t i=0; i<NROWS; ++i )
    for( int j=0; j<NCOLS; ++j )
    TEST_FLOATING_EQUALITY( test_mat(i,j), dest_mat(i,j), 1.e-14 );

  // Verifiy that the copy is a deep copy
  dest_mat(1,1) *= 1.5;
  double diff = dest_mat(1,1) - test_mat(1,1);
  TEST_FLOATING_EQUALITY( diff, 0.5*test_mat(1,1), 1.e-14 );
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
