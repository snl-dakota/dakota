
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"

#include <string>

#include <Teuchos_UnitTestHarness.hpp> 

using namespace Dakota;

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(covariance_reader, read_constant_covariance)
{
  const std::string base_name = "expt_data_test_files/voltage";

  RealMatrix cov_values;
  read_covariance(base_name, 1 /* expt number */, cov_values); // implies CONSTANT

  // Verify equality of config data
  TEST_EQUALITY( cov_values.numRows(), 1 );
  TEST_EQUALITY( cov_values.numCols(), 1 );
  TEST_FLOATING_EQUALITY( cov_values[0][0], 3.67, 1.e-14 );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(covariance_reader, read_vector_covariance)
{
  const std::string base_name = "expt_data_test_files/voltage";

  RealMatrix cov_values;
  read_covariance(base_name, 2, // expt number
                  Dakota::CovarianceMatrix::VECTOR,
                  9, // num values in covariance VECTOR
                  cov_values);

  double gold_values[] = { 2.34, 8.552, -3.654, 7.332, 0.01, -0.1509, -5.98, 4.74, 9.99 };
  // Verify equality of config data
  TEST_EQUALITY( cov_values.numRows(), 1 );
  TEST_EQUALITY( cov_values.numCols(), 9 );
  for( int i=0; i<9; ++i )
    TEST_FLOATING_EQUALITY( cov_values[i][0], gold_values[i], 1.e-14 );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(covariance_reader, read_matrix_covariance)
{
  const std::string base_name = "expt_data_test_files/voltage";

  RealMatrix cov_values;
  read_covariance(base_name, 3, // expt number
                  Dakota::CovarianceMatrix::MATRIX,
                  9, // num values in covariance MATRIX row (total entries = 9*9)
                  cov_values);

  // Verify equality of config data
  TEST_EQUALITY( cov_values.numRows(), 9 );
  TEST_EQUALITY( cov_values.numCols(), 9 );
  for( int i=0; i<9; ++i )
    for( int j=0; j<9; ++j )
      TEST_FLOATING_EQUALITY( cov_values[i][j], (double)(i+j)/2.0, 1.e-14 );
}

//----------------------------------------------------------------
