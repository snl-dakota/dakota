
#include "ExperimentDataUtils.hpp"
#include "dakota_tabular_io.hpp"
#include "dakota_data_io.hpp"

#include <string>

#include <Teuchos_UnitTestHarness.hpp> 

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
  compute_svd(sub_mat, NUM_ROWS, 2, singular_values);
  // --------------- What we are testing

  //std::cout << "\nSingular values:\n" << std::endl;
  //singular_values.print(std::cout);

  // Test against Singlular Values obtained from matlab for the same matrix
  TEST_EQUALITY( singular_values.length(), 2 );
  TEST_FLOATING_EQUALITY( singular_values[0], 4.39718, 1.e-5 )
  TEST_FLOATING_EQUALITY( singular_values[1], 1.14583, 1.e-5 )
}

//----------------------------------------------------------------
