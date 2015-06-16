
#include "ExperimentDataUtils.hpp"
#include "ReducedBasis.hpp"
#include "dakota_tabular_io.hpp"
#include "dakota_data_io.hpp"

#include <string>

#include <Teuchos_UnitTestHarness.hpp> 
#include <Teuchos_SerialDenseHelpers.hpp>

using namespace Dakota;

namespace {

  const int NUM_ROWS = 100;
  const int NUM_COLS = 53;

  // Create submatrix consisting of the two input parameter columns
  std::pair<RealMatrix, RealMatrix> get_parameter_and_response_submatrices() {
    static RealMatrix * param_sub_mat = NULL;
    static RealMatrix * response_sub_mat = NULL;
    if( !param_sub_mat )
    {
      const std::string filename = "dakota_tabular_svd.dat";
      std::ifstream in_file;
      TabularIO::open_file(in_file, filename, "unit test test_compute_svd_data");

      RealVectorArray test_data;
      read_fixed_rowsize_data(in_file, test_data, NUM_COLS, true /* row_major */);

      // Convert RealVectorArray --> RealMatrix
      RealMatrix test_mat;
      copy_data(test_data, test_mat);

      //test_mat.print(std::cout);

      // Create submatrix consisting of the two input parameter columns
      param_sub_mat = new RealMatrix(Teuchos::Copy, test_mat, test_mat.numRows(), 2, 0, 1);

      // Create submatrix consisting of the 50 response columns
      response_sub_mat = new RealMatrix(Teuchos::Copy, test_mat, test_mat.numRows(), 50, 0, 3);
    }
    return std::make_pair<RealMatrix, RealMatrix>(*param_sub_mat, *response_sub_mat);
  }
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(reduced_basis, compute_col_means)
{
  RealMatrix matrix = get_parameter_and_response_submatrices().first;

  // --------------- What we are testing
  RealVector column_means;
  compute_column_means(matrix, column_means);
  // --------------- What we are testing

  // Test against Column Means obtained from matlab for the same matrix
  TEST_EQUALITY( column_means.length(), 2 );
  TEST_FLOATING_EQUALITY( column_means[0], 0.299958060862, 1.e-12 )
  TEST_FLOATING_EQUALITY( column_means[1], 0.299814773583, 1.e-12 )
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(reduced_basis, compute_svd)
{
  RealMatrix matrix = get_parameter_and_response_submatrices().first;

  // --------------- What we are testing
  RealVector singular_values;
  RealMatrix VTranspose;
  compute_svd(matrix, NUM_ROWS, 2, singular_values, VTranspose);
  // --------------- What we are testing

  //std::cout << "\nSingular values:\n" << std::endl;
  //singular_values.print(std::cout);

  // Test against Singlular Values obtained from matlab for the same matrix
  TEST_EQUALITY( singular_values.length(), 2 );
  TEST_FLOATING_EQUALITY( singular_values[0], 4.39718, 1.e-5 )
  TEST_FLOATING_EQUALITY( singular_values[1], 1.14583, 1.e-5 )
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(reduced_basis, simple_api)
{
  RealMatrix matrix = get_parameter_and_response_submatrices().first;

  // --------------- What we are testing
  ReducedBasis reduced_basis;
  reduced_basis.set_matrix(matrix);
  reduced_basis.update_svd(false);
  const RealVector & singular_values = reduced_basis.get_singular_values();
  // --------------- What we are testing

  // Test against Singlular Values obtained from matlab for the same matrix
  TEST_EQUALITY( singular_values.length(), 2 );
  TEST_FLOATING_EQUALITY( singular_values[0], 4.39718, 1.e-5 )
  TEST_FLOATING_EQUALITY( singular_values[1], 1.14583, 1.e-5 )

  const RealMatrix & U_mat = reduced_basis.get_left_singular_vector();
  const RealMatrix & VT_mat = reduced_basis.get_right_singular_vector_transpose();

  RealMatrix S(2,2);
  S(0,0) = singular_values(0);
  S(1,1) = singular_values(1);

  // Reconstruct matrix using SVD and compare to original (implies original matrix does not get changed).
  RealMatrix reconstructed_mat( matrix.numRows(), matrix.numCols() );
  int ierr = reconstructed_mat.multiply(Teuchos::NO_TRANS, Teuchos::NO_TRANS, 1.0, U_mat, S, 0.0);
  RealMatrix temp_matrix(reconstructed_mat);
  ierr += reconstructed_mat.multiply(Teuchos::NO_TRANS, Teuchos::NO_TRANS, 1.0, temp_matrix, VT_mat, 0.0);
  TEST_EQUALITY( ierr, 0 )

  reconstructed_mat -= matrix;
  Real diff = 1.0 + reconstructed_mat.normFrobenius();
  TEST_FLOATING_EQUALITY( diff, 1.0, 1.e-14 )
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(reduced_basis, simple_api2)
{
  RealMatrix matrix1 = get_parameter_and_response_submatrices().first;
  RealMatrix matrix2 = get_parameter_and_response_submatrices().first;

  // Center columns about their means
  RealVector column_means;
  compute_column_means(matrix1, column_means);
  RealVector const_vec(matrix1.numRows());
  RealVector col1 = Teuchos::getCol(Teuchos::View, matrix1, 0);
  RealVector col2 = Teuchos::getCol(Teuchos::View, matrix1, 1);
  const_vec.putScalar(column_means(0));
  col1 -= const_vec;
  const_vec.putScalar(column_means(1));
  col2 -= const_vec;

  // --------------- What we are testing
  ReducedBasis reduced_basis;

  reduced_basis.set_matrix(matrix1);
  reduced_basis.update_svd(false);
  const RealVector & singular_values1 = reduced_basis.get_singular_values();

  reduced_basis.set_matrix(matrix2);
  reduced_basis.update_svd(/* true - by default */);
  const RealVector & singular_values2 = reduced_basis.get_singular_values();
  // --------------- What we are testing

  //std::cout << "Original mean-shifted matrix : \n";
  //matrix.print(std::cout);

  //std::cout << "\n\nCorresponding singular values: ["
  //          << singular_values(0) << ", "
  //          << singular_values(1) << "]" << std::endl;
  // Need to test something ...
  TEST_EQUALITY( singular_values1.length(), 2 );
  TEST_FLOATING_EQUALITY( singular_values1[0], 1.161389978, 1.e-6 )
  TEST_FLOATING_EQUALITY( singular_values1[1], 1.145814014, 1.e-6 )
  TEST_EQUALITY( singular_values1.length(), singular_values2.length() );
  for( int i=0; i<singular_values1.length(); ++i )
    TEST_FLOATING_EQUALITY( singular_values1[i], singular_values2[i], 1.e-6 )
}

//----------------------------------------------------------------

