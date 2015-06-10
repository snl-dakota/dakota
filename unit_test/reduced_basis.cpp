
#include "ExperimentDataUtils.hpp"
#include "ReducedBasis.hpp"
#include "dakota_tabular_io.hpp"
#include "dakota_data_io.hpp"

#include <string>

#include <Teuchos_UnitTestHarness.hpp> 
#include <Teuchos_SerialDenseHelpers.hpp>

using namespace Dakota;

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(reduced_basis, compute_svd)
{
  const int NUM_ROWS = 100;
  const int NUM_COLS = 53;

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
  RealMatrix sub_mat(Teuchos::Copy, test_mat, test_mat.numRows(), 2, 0, 1);
  //sub_mat.print(std::cout);

  // --------------- What we are testing
  RealVector singular_values;
  RealMatrix VTranspose;
  compute_svd(sub_mat, NUM_ROWS, 2, singular_values, VTranspose);
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
  const int NUM_ROWS = 100;
  const int NUM_COLS = 53;

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
  RealMatrix sub_mat(Teuchos::Copy, test_mat, test_mat.numRows(), 2, 0, 1);
  //sub_mat.print(std::cout);

  //std::cout << "simple_api unit test : original matrix:\n";
  //sub_mat.print(std::cout);

  // --------------- What we are testing
  ReducedBasis reduced_basis;
  reduced_basis.set_matrix(sub_mat);
  reduced_basis.update_svd();
  const RealVector & singular_values = reduced_basis.get_singlular_values();
  // --------------- What we are testing

  //std::cout << "\nSingular values:\n" << std::endl;
  //singular_values.print(std::cout);

  // Test against Singlular Values obtained from matlab for the same matrix
  TEST_EQUALITY( singular_values.length(), 2 );
  TEST_FLOATING_EQUALITY( singular_values[0], 4.39718, 1.e-5 )
  TEST_FLOATING_EQUALITY( singular_values[1], 1.14583, 1.e-5 )

  const RealMatrix & U_mat = reduced_basis.get_left_singular_vector();
  const RealMatrix & VT_mat = reduced_basis.get_right_singular_vector_transpose();

  // Reconstruct matrix using SVD and compare to original (implies original matrix does njot get changed).
  RealMatrix reconstructed_mat(U_mat);
  RealVector col1 = Teuchos::getCol(Teuchos::View, reconstructed_mat, 0);
  RealVector col2 = Teuchos::getCol(Teuchos::View, reconstructed_mat, 1);
  col1.scale(singular_values[0]);
  col2.scale(singular_values[1]);
  // Manually do matrix multiply; I had hoped RealMatrix would support this ...
  Real prod1, prod2;
  for( int row=0; row<reconstructed_mat.numRows(); ++row ) {
    prod1 = reconstructed_mat(row, 0)*VT_mat(0, 0) +
                 reconstructed_mat(row, 1)*VT_mat(1, 0);
    prod2 = reconstructed_mat(row, 0)*VT_mat(0, 1) +
                 reconstructed_mat(row, 1)*VT_mat(1, 1);
    reconstructed_mat(row, 0) = prod1;
    reconstructed_mat(row, 1) = prod2;
  }

  reconstructed_mat -= sub_mat;
  Real diff = 1.0 + reconstructed_mat.normFrobenius();
  TEST_FLOATING_EQUALITY( diff, 1.0, 1.e-14 )

  //std::cout << "simple_api unit test : U matrix:\n";
  //U_mat.print(std::cout);

  //std::cout << "simple_api unit test : VT matrix:\n";
  //VT_mat.print(std::cout);

  //std::cout << "simple_api unit test : original matrix:\n";
  //sub_mat.print(std::cout);

  //std::cout << "simple_api unit test : reconstructed matrix:\n";
  //reconstructed_mat.print(std::cout);

}

//----------------------------------------------------------------
