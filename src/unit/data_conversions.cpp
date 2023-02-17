/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include "dakota_data_util.hpp"
#include "dakota_data_io.hpp"

#include <string>

#define BOOST_TEST_MODULE dakota_data_conversions
#include <boost/test/included/unit_test.hpp>

using namespace Dakota;
using dakota::MatrixXd;

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

BOOST_AUTO_TEST_CASE(test_data_conversion_rva2rm)
{
  const int NROWS = 5;
  const int NCOLS = 3;
  RealVectorArray test_rva = create_test_array(NROWS, NCOLS, true);
  RealMatrix test_rm;

  /////////////////  What we want to test
  copy_data(test_rva, test_rm);
  /////////////////  What we want to test

  // Verify correct dimensions
  BOOST_CHECK( NROWS == test_rm.numRows() );
  BOOST_CHECK( NCOLS == test_rm.numCols() );

  // Verify contents of what we wrote and what we read
  for( size_t i=0; i<NROWS; ++i )
    for( int j=0; j<NCOLS; ++j )
    BOOST_CHECK_CLOSE( test_rva[i][j], test_rm(i,j), 1.e-12 );
}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_data_conversion_mat2mat)
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
  BOOST_CHECK( NROWS == dest_mat.numRows() );
  BOOST_CHECK( NCOLS == dest_mat.numCols() );

  // Verify contents of what we wrote and what we read
  for( size_t i=0; i<NROWS; ++i )
    for( int j=0; j<NCOLS; ++j )
    BOOST_CHECK_CLOSE( test_mat(i,j), dest_mat(i,j), 1.e-12 );

  // Verifiy that the copy is a deep copy
  dest_mat(1,1) *= 1.5;
  double diff = dest_mat(1,1) - test_mat(1,1);
  BOOST_CHECK_CLOSE( diff, 0.5*test_mat(1,1), 1.e-12 );
}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_data_conversion_mat2mat_sub)
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
  BOOST_CHECK( 3 == dest_mat.numRows() );
  BOOST_CHECK( 2 == dest_mat.numCols() );

  // Verify contents of what we wrote and what we read
  for( int i=0; i<dest_mat.numRows(); ++i )
    for( int j=0; j<dest_mat.numCols(); ++j )
    BOOST_CHECK_CLOSE( test_mat(i+roffset,j+coffset), dest_mat(i,j), 1.e-12 );

  // Verifiy that the copy is a deep copy
  dest_mat(1,1) *= 1.5;
  double diff = dest_mat(1,1) - test_mat(1+roffset,1+coffset);
  BOOST_CHECK_CLOSE( diff, 0.5*test_mat(1+roffset,1+coffset), 1.e-12 );
}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_data_conversion_mat2mat_sym)
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
  BOOST_CHECK( NROWS == dest_mat.numRows() );
  BOOST_CHECK( NCOLS == dest_mat.numCols() );

  // Verify contents of what we wrote and what we read
  for( size_t i=0; i<NROWS; ++i )
    for( int j=0; j<NCOLS; ++j )
    BOOST_CHECK_CLOSE( test_mat(i,j), dest_mat(i,j), 1.e-12 );

  // Verifiy that the copy is a deep copy
  dest_mat(1,1) *= 1.5;
  double diff = dest_mat(1,1) - test_mat(1,1);
  BOOST_CHECK_CLOSE( diff, 0.5*test_mat(1,1), 1.e-12 );
}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_data_conversion_mat_swap)
{
  RealMatrix mat( 5, 5 );
  mat.random();
  RealMatrix copy_mat(Teuchos::Copy, mat);

  RealMatrix mat_swap( 2, 3 );
  mat_swap.random();
  RealMatrix copy_mat_swap(Teuchos::Copy, mat_swap);

  //Cout <<  "swap() -- swap the values and attributes of two matrices ";
  mat_swap.swap(mat);
  bool op_result = ( (mat_swap == copy_mat) && (mat == copy_mat_swap) );
  //Cout << "shallow swap results -- "<< (op_result ? "successful" : "failed" )<<std::endl;
  if( !op_result )
    BOOST_CHECK( false );
}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_data_conversion_sym_mat_swap)
{
  RealSymMatrix mat( 5 );
  mat.random();
  RealSymMatrix copy_mat(mat);

  RealSymMatrix mat_swap( 3 );
  mat_swap.random();
  RealSymMatrix copy_mat_swap(mat_swap);

  mat_swap.swap(mat);
  bool op_result = ( (mat_swap == copy_mat) && (mat == copy_mat_swap) );
  if( !op_result )
    BOOST_CHECK( false );
}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_data_conversion_apply_matrix)
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
  BOOST_CHECK( NROWS == v2.size() );

  // Verify correct values
  Real sum = 0.0;
  for( size_t i=0; i<NROWS; ++i )
  {
    sum = 0.0;
    for( int j=0; j<NCOLS; ++j )
      sum += mat(i,j);
    BOOST_CHECK_CLOSE( sum, v2[i], 1.e-12 );
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
  BOOST_CHECK( 2*NROWS == v4.size() );

  // Verify correct values
  for( size_t i=0; i<v4.size(); ++i )
  {
    if( i<NROWS )
    {
      sum = 0.0;
      for( int j=0; j<NCOLS; ++j )
        sum += mat(i,j);
      BOOST_CHECK_CLOSE( 2.0*sum, v4[i], 1.e-12 );
    }
    else
      BOOST_CHECK_CLOSE( (Real)i, v4[i], 1.e-12 );
  }
}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_data_conversion_apply_matrix_transpose)
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
  BOOST_CHECK( NCOLS == v2.size() );

  // Verify correct values
  Real sum = 0.0;
  for( size_t i=0; i<NCOLS; ++i )
  {
    sum = 0.0;
    for( int j=0; j<NROWS; ++j )
      sum += mat(j,i);
    BOOST_CHECK_CLOSE( sum, v2[i], 1.e-12 );
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
  BOOST_CHECK( 2*NCOLS == v4.size() );

  // Verify correct values
  for( size_t i=0; i<v4.size(); ++i )
  {
    if( i<NCOLS )
    {
      sum = 0.0;
      for( int j=0; j<NROWS; ++j )
        sum += mat(j,i);
      BOOST_CHECK_CLOSE( 2.0*sum, v4[i], 1.e-12 );
    }
    else
      BOOST_CHECK_CLOSE( (Real)i, v4[i], 1.e-12 );
  }
}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_data_conversion_rm2eigM)
{
  const int NROWS = 5;
  const int NCOLS = 3;
  RealMatrix rm(NROWS, NCOLS);
  rm.random();

  MatrixXd test_mat;

  /////////////////  What we want to test
  copy_data(rm, test_mat);
  /////////////////  What we want to test

  // Verify correct dimensions
  BOOST_CHECK( NROWS == test_mat.rows() );
  BOOST_CHECK( NCOLS == test_mat.cols() );

  // Verify contents
  for( size_t i=0; i<NROWS; ++i )
    for( int j=0; j<NCOLS; ++j )
    BOOST_CHECK_CLOSE( test_mat(i,j), rm(i,j), 1.e-12 );


  // Store the original norm to see if we change it via a view
  const Real orig_norm = rm.normOne();

  Eigen::Map<MatrixXd> view_mat(NULL, 0, 0);

  /////////////////  What we want to test
  view_data(rm, view_mat);
  view_mat *= 3.14;
  /////////////////  What we want to test

  // Verify correct behavior of view
  const Real new_norm = rm.normOne();
  BOOST_CHECK( 3.14*orig_norm == new_norm );
}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_data_conversion_eigM2rm)
{
  const int NROWS = 5;
  const int NCOLS = 3;
  MatrixXd em = MatrixXd::Random(NROWS, NCOLS);

  RealMatrix test_mat;

  /////////////////  What we want to test
  copy_data(em, test_mat);
  /////////////////  What we want to test

  // Verify correct dimensions
  BOOST_CHECK( NROWS == test_mat.numRows() );
  BOOST_CHECK( NCOLS == test_mat.numCols() );

  // Verify contents
  for( size_t i=0; i<NROWS; ++i )
    for( int j=0; j<NCOLS; ++j )
    BOOST_CHECK_CLOSE( test_mat(i,j), em(i,j), 1.e-12 );
}

//----------------------------------------------------------------
