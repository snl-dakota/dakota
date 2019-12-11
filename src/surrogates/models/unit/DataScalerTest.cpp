/*  _______________________________________________________________________

    PECOS: Parallel Environment for Creation Of Stochastics
    Copyright (c) 2011, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Pecos directory.
    _______________________________________________________________________ */

/////////////
// Imports //
/////////////

#include <Teuchos_UnitTestHarness.hpp>
#include "teuchos_data_types.hpp"
#include "DataScaler.hpp"
#include "Eigen/Dense"

///////////////
// Namespace //
///////////////

using namespace Surrogates;
using namespace Eigen;

///////////////
// Utilities //
///////////////

void error(const std::string msg)
{
  throw(std::runtime_error(msg));
}

bool matrix_equals(const MatrixXd &A, const MatrixXd &B, Real tol)
{
  if ( (A.rows()!=B.rows()) || (A.cols()!=B.cols())){
    std::cout << A.rows() << "," << A.cols() << std::endl;
    std::cout << B.rows() << "," << B.cols() << std::endl;
    error("matrix_equals() matrices sizes are inconsistent");
  }
  for (int j=0; j<A.cols(); j++){
    for (int i=0; i<A.rows(); i++){
      if (std::abs(A(i,j)-B(i,j))>tol)
	    return false;
    }
  }
  return true;
}

MatrixXd create_single_sample_matrix()
{
  MatrixXd single_sample_matrix(1, 7);
  single_sample_matrix << 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7;
  return single_sample_matrix;
}

////////////////
// Unit tests //
////////////////

TEUCHOS_UNIT_TEST(surrogates, NormalizationScaler_scaleSamples_TestMeanNormalizationTrue)
{
  const int norm_factor = 1.0;
  NormalizationScaler ns(create_single_sample_matrix(), true, norm_factor);

  MatrixXd scale_vector_actual(1, 7);
  MatrixXd scale_vector_expected(1, 7);
  
  scale_vector_actual = ns.getScaledFeatures();
  scale_vector_expected << -0.5, -0.333333, -0.166667, 0.0, 0.166667, 0.333333, 0.5;

  //std::cout << scale_vector_expected << std::endl;
  //std::cout << scale_vector_actual << std::endl;

  TEST_ASSERT(matrix_equals(scale_vector_actual, scale_vector_expected, 1.0e-4));
}

TEUCHOS_UNIT_TEST(surrogates, NormalizationScaler_scaleSamples_TestMeanNormalizationFalse)
{
  const int norm_factor = 1.0;
  NormalizationScaler ns(create_single_sample_matrix(), false, norm_factor);

  MatrixXd scale_vector_actual(1, 7);
  MatrixXd scale_vector_expected(1, 7);
  
  scale_vector_actual = ns.getScaledFeatures();
  scale_vector_expected << 0, 0.166667, 0.333333, 0.5, 0.666667, 0.833333, 1;

  //std::cout << scale_vector_expected << std::endl;
  //std::cout << scale_vector_actual << std::endl;

  TEST_ASSERT(matrix_equals(scale_vector_actual, scale_vector_expected, 1.0e-4));
}

TEUCHOS_UNIT_TEST(surrogates, NormalizationScaler_scaleSamples_TestNormFactor)
{
  const int norm_factor = 2.0;
  NormalizationScaler ns(create_single_sample_matrix(), true, norm_factor);

  MatrixXd scale_vector_actual(1, 7);
  MatrixXd scale_vector_expected(1, 7);
  
  scale_vector_actual = ns.getScaledFeatures();
  scale_vector_expected << -1, -0.666667, -0.333333, 1.85037e-16, 0.333333, 0.666667, 1;

  //std::cout << scale_vector_expected << std::endl;
  //std::cout << scale_vector_actual << std::endl;

  TEST_ASSERT(matrix_equals(scale_vector_actual, scale_vector_expected, 1.0e-4));
}

TEUCHOS_UNIT_TEST(surrogates, StandardizationScaler_scaleSamples_TestDefault)
{
  StandardizationScaler ss(create_single_sample_matrix());

  MatrixXd scale_vector_actual(1, 7);
  MatrixXd scale_vector_expected(1, 7);
  
  scale_vector_actual = ss.getScaledFeatures();
  scale_vector_expected << -1.5, -1, -0.5, 2.77556e-16, 0.5, 1, 1.5;

  //std::cout << scale_vector_expected << std::endl;
  //std::cout << scale_vector_actual << std::endl;

  TEST_ASSERT(matrix_equals(scale_vector_actual, scale_vector_expected, 1.0e-4));
  TEST_ASSERT(scale_vector_actual.row(0).mean() < 1.0e-14); // For StandardizationScaler, mean should be effectively zero.
}

TEUCHOS_UNIT_TEST(surrogates, NoScaler_scaleSamples_TestDefault)
{
  NoScaler ns(create_single_sample_matrix());

  MatrixXd scale_vector_actual(1, 7);
  MatrixXd scale_vector_expected(1, 7);
  
  scale_vector_actual = ns.getScaledFeatures();
  scale_vector_expected  << 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7;

  //std::cout << scale_vector_expected << std::endl;
  //std::cout << scale_vector_actual << std::endl;

  TEST_ASSERT(matrix_equals(scale_vector_actual, scale_vector_expected, 1.0e-4));
}