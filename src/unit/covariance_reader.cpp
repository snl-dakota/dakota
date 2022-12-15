/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"

#include <string>

#define BOOST_TEST_MODULE dakota_covariance_reader
#include <boost/test/included/unit_test.hpp>

using namespace Dakota;

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_covariance_reader_read_constant_covariance)
{
  const std::string base_name = "expt_data_test_files/voltage";

  RealMatrix cov_values;
  read_covariance(base_name, 1 /* expt number */, cov_values); // implies CONSTANT

  // Verify equality of config data
  BOOST_CHECK( cov_values.numRows() == 1 );
  BOOST_CHECK( cov_values.numCols() == 1 );
  BOOST_CHECK_CLOSE( cov_values[0][0], 3.67, 1.e-12 );
}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_covariance_reader_read_vector_covariance)
{
  const std::string base_name = "expt_data_test_files/voltage";

  RealMatrix cov_values;
  read_covariance(base_name, 2, // expt number
                  Dakota::CovarianceMatrix::VECTOR,
                  9, // num values in covariance VECTOR
                  cov_values);

  double gold_values[] = { 2.34, 8.552, -3.654, 7.332, 0.01, -0.1509, -5.98, 4.74, 9.99 };
  // Verify equality of config data
  BOOST_CHECK( cov_values.numRows() == 1 );
  BOOST_CHECK( cov_values.numCols() == 9 );
  for( int i=0; i<9; ++i )
    BOOST_CHECK_CLOSE( cov_values[i][0], gold_values[i], 1.e-12 );
}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_covariance_reader_read_matrix_covariance)
{
  const std::string base_name = "expt_data_test_files/voltage";

  RealMatrix cov_values;
  read_covariance(base_name, 3, // expt number
                  Dakota::CovarianceMatrix::MATRIX,
                  9, // num values in covariance MATRIX row (total entries = 9*9)
                  cov_values);

  // Verify equality of config data
  BOOST_CHECK( cov_values.numRows() == 9 );
  BOOST_CHECK( cov_values.numCols() == 9 );
  for( int i=0; i<9; ++i )
    for( int j=0; j<9; ++j )
      BOOST_CHECK_CLOSE( cov_values[i][j], (double)(i+j)/2.0, 1.e-12 );
}

//----------------------------------------------------------------
