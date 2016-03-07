#include "ExperimentDataUtils.hpp"
#include "dakota_global_defs.hpp"
#include "DataMethod.hpp"
#include "DakotaResponse.hpp"
#include "NonDBayesCalibration.hpp"
// Boost.Test
#define BOOST_TEST_MODULE dakota_field_covariance_utils
#include <boost/test/included/unit_test.hpp>

//#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>

#include <cassert>
#include <iostream>


namespace Dakota {
namespace TestFieldCovariance {
  
void test_multiple_scalar_covariance_matrix()
{
  std::vector<RealMatrix> matrices;
  std::vector<RealVector> diagonals;
  RealVector scalars;
  IntVector matrix_map_indices, diagonal_map_indices, scalar_map_indices;

  int num_scalars = 3;
  Real scalar_array[] = {1.,2.,4.};
  int scalar_map_index_array[] = {0, 1, 2};
  scalars.sizeUninitialized( num_scalars );
  scalar_map_indices.sizeUninitialized( num_scalars );
  for ( int i=0; i<num_scalars; i++ ){
    scalars[i] = scalar_array[i];
    scalar_map_indices[i] = scalar_map_index_array[i];
  }

  ExperimentCovariance exper_cov; 
  exper_cov.set_covariance_matrices( matrices, diagonals, scalars,
				     matrix_map_indices,
				     diagonal_map_indices, 
				     scalar_map_indices );

  // Test determinant and log_determinant
  BOOST_CHECK_CLOSE(exper_cov.determinant(), 8.0, 1.0e-12);
  BOOST_CHECK_CLOSE(exper_cov.log_determinant(), std::log(8.0), 1.0e-12);

  int num_residuals = 3;
  Real residual_array[] = {1.,2.,4.};
  RealVector residual( Teuchos::Copy, residual_array, num_residuals );

  // Test application of the covariance inverse to residual vector
  Real prod = exper_cov.apply_experiment_covariance( residual );
  BOOST_CHECK(  std::abs( prod - 7. ) < 
		10.*std::numeric_limits<double>::epsilon() );

  // Test application of the sqrt of the covariance inverse to residual vector
  RealVector result;
  exper_cov.apply_experiment_covariance_inverse_sqrt( residual, result );
  prod = result.dot( result );
  BOOST_CHECK(  std::abs( prod - 7. ) < 
		10.*std::numeric_limits<double>::epsilon() );

  // Test application of the sqrt of the covariance inverse to matrix of 
  // gradient vectors
  RealMatrix scaled_grads;
  Real grads_array[] = {1.,2.,2.,4.,4.,8.};
  RealMatrix grads( Teuchos::Copy, grads_array, 2, 2, 3 );
  exper_cov.apply_experiment_covariance_inverse_sqrt_to_gradients( grads, 
								   scaled_grads);

  RealMatrix grammian( grads.numRows(), grads.numRows(), false );
  grammian.multiply( Teuchos::NO_TRANS, Teuchos::TRANS, 1.0, scaled_grads, 
		     scaled_grads, 0. );
  BOOST_CHECK(  std::abs( grammian(0,0) - 7. ) < 
		10.*std::numeric_limits<double>::epsilon() );
  BOOST_CHECK(  std::abs( grammian(1,1) - 28. ) < 
		10.*std::numeric_limits<double>::epsilon() );

  // Test application of the sqrt of the covariance inverse to matrix of 
  // Hessian matrices
  Real hessian_0_array[] = {4., 2., 2., 2.};
  RealSymMatrix hessian_0( Teuchos::Copy, false, hessian_0_array, 2, 2 );
  RealSymMatrix hessian_1( hessian_0 );
  RealSymMatrix hessian_2( hessian_0 );
  hessian_1 *= 2.;
  hessian_2 *= 4.;

  RealSymMatrixArray hessians( 3 );
  hessians[0] = hessian_0;
  hessians[1] = hessian_1;
  hessians[2] = hessian_2;
  RealSymMatrixArray scaled_hessians;
  exper_cov.apply_experiment_covariance_inverse_sqrt_to_hessians( hessians,
							       scaled_hessians );

  Real exact_scaled_hessian_0_array[] = {4.,2.,2.,2.};
  Real exact_scaled_hessian_1_array[] = {8./std::sqrt(2.),4./std::sqrt(2.),
				   4./std::sqrt(2.),4./std::sqrt(2.)};
  Real exact_scaled_hessian_2_array[] = {8., 4., 4., 4.};
  RealSymMatrix exact_scaled_hessian_0( Teuchos::View, false, 
				  exact_scaled_hessian_0_array, 2, 2 );
  RealSymMatrix exact_scaled_hessian_1( Teuchos::View, false, 
				  exact_scaled_hessian_1_array, 2, 2 );
  RealSymMatrix exact_scaled_hessian_2( Teuchos::View, false, 
				  exact_scaled_hessian_2_array, 2, 2 );

  scaled_hessians[0] -= exact_scaled_hessian_0;
  scaled_hessians[1] -= exact_scaled_hessian_1;
  scaled_hessians[2] -= exact_scaled_hessian_2;

  BOOST_CHECK( scaled_hessians[0].normInf() < 
	       10.*std::numeric_limits<double>::epsilon() );
  BOOST_CHECK( scaled_hessians[1].normInf() < 
	       10.*std::numeric_limits<double>::epsilon() );
  BOOST_CHECK( scaled_hessians[2].normInf() < 
	       10.*std::numeric_limits<double>::epsilon() );

  // Test extraction of main diagonal
  RealVector diagonal;
  exper_cov.get_main_diagonal( diagonal );

  Real exact_diagonal_array[] = {1.,2.,4.};
  RealVector exact_diagonal(Teuchos::View, exact_diagonal_array, 3 );
  
  exact_diagonal -= diagonal;
  BOOST_CHECK( exact_diagonal.normInf() < 
	       10.*std::numeric_limits<double>::epsilon() );

}

void test_single_diagonal_block_covariance_matrix()
{
  std::vector<RealMatrix> matrices;
  std::vector<RealVector> diagonals;
  RealVector scalars;
  IntVector matrix_map_indices, diagonal_map_indices, scalar_map_indices;

  int num_diags = 1;
  int num_diag_entries = 3;
  Real diagonal_array[] = {1.,2.,4.};
  diagonal_map_indices.sizeUninitialized( num_diags );
  diagonal_map_indices[0] = 0;
  diagonals.resize( num_diags );
  diagonals[0].sizeUninitialized( num_diag_entries );
  for ( int i=0; i<num_diag_entries; i++ ){
    diagonals[0][i] = diagonal_array[i];
  }

  ExperimentCovariance exper_cov; 
  exper_cov.set_covariance_matrices( matrices, diagonals, scalars,
				     matrix_map_indices,
				     diagonal_map_indices, 
				     scalar_map_indices );


  // Test determinant and log_determinant
  BOOST_CHECK_CLOSE(exper_cov.determinant(), 8.0, 1.0e-12);
  BOOST_CHECK_CLOSE(exper_cov.log_determinant(), std::log(8.0), 1.0e-12);

  int num_residuals = 3;
  Real residual_array[] = {1.,2.,4.};
  RealVector residual( Teuchos::Copy, residual_array, num_residuals );

  // Test application of the covariance inverse to residual vector
  Real prod = exper_cov.apply_experiment_covariance( residual );
  BOOST_CHECK(  std::abs( prod - 7. ) < 
		10.*std::numeric_limits<double>::epsilon() );
  
  // Test application of the sqrt of the covariance inverse to residual vector
  RealVector result;
  exper_cov.apply_experiment_covariance_inverse_sqrt( residual, result );
  prod = result.dot( result );
  BOOST_CHECK(  std::abs( prod - 7. ) < 
		10.*std::numeric_limits<double>::epsilon() );

  // Test application of the sqrt of the covariance inverse to matrix of 
  // gradient vectors
  RealMatrix scaled_grads;
  Real grads_array[] = {1.,2.,2.,4.,4.,8.};
  RealMatrix grads( Teuchos::Copy, grads_array, 2, 2, 3 );
  exper_cov.apply_experiment_covariance_inverse_sqrt_to_gradients( grads, 
								   scaled_grads);

  RealMatrix grammian( grads.numRows(), grads.numRows(), false );
  grammian.multiply( Teuchos::NO_TRANS, Teuchos::TRANS, 1.0, scaled_grads, 
		     scaled_grads, 0. );
  BOOST_CHECK(  std::abs( grammian(0,0) - 7. ) < 
		10.*std::numeric_limits<double>::epsilon() );
  BOOST_CHECK(  std::abs( grammian(1,1) - 28. ) < 
		10.*std::numeric_limits<double>::epsilon() );

  // Test application of the sqrt of the covariance inverse to matrix of 
  // Hessian matrices
  Real hessian_0_array[] = {4., 2., 2., 2.};
  RealSymMatrix hessian_0( Teuchos::Copy, false, hessian_0_array, 2, 2 );
  RealSymMatrix hessian_1( hessian_0 );
  RealSymMatrix hessian_2( hessian_0 );
  hessian_1 *= 2.;
  hessian_2 *= 4.;

  RealSymMatrixArray hessians( 3 );
  hessians[0] = hessian_0;
  hessians[1] = hessian_1;
  hessians[2] = hessian_2;
  RealSymMatrixArray scaled_hessians;
  exper_cov.apply_experiment_covariance_inverse_sqrt_to_hessians( hessians,
							       scaled_hessians );

  Real exact_scaled_hessian_0_array[] = {4.,2.,2.,2.};
  Real exact_scaled_hessian_1_array[] = {8./std::sqrt(2.),4./std::sqrt(2.),
				   4./std::sqrt(2.),4./std::sqrt(2.)};
  Real exact_scaled_hessian_2_array[] = {8., 4., 4., 4.};
  RealSymMatrix exact_scaled_hessian_0( Teuchos::View, false, 
				  exact_scaled_hessian_0_array, 2, 2 );
  RealSymMatrix exact_scaled_hessian_1( Teuchos::View, false, 
				  exact_scaled_hessian_1_array, 2, 2 );
  RealSymMatrix exact_scaled_hessian_2( Teuchos::View, false, 
				  exact_scaled_hessian_2_array, 2, 2 );

  scaled_hessians[0] -= exact_scaled_hessian_0;
  scaled_hessians[1] -= exact_scaled_hessian_1;
  scaled_hessians[2] -= exact_scaled_hessian_2;

  BOOST_CHECK( scaled_hessians[0].normInf() < 
	       10.*std::numeric_limits<double>::epsilon() );
  BOOST_CHECK( scaled_hessians[1].normInf() < 
	       10.*std::numeric_limits<double>::epsilon() );
  BOOST_CHECK( scaled_hessians[2].normInf() < 
	       10.*std::numeric_limits<double>::epsilon() );
}

void test_single_full_block_covariance_matrix()
{
  std::vector<RealMatrix> matrices;
  std::vector<RealVector> diagonals;
  RealVector scalars;
  IntVector matrix_map_indices, diagonal_map_indices, scalar_map_indices;

  int num_matrices = 1;
  int num_matrix_rows = 3;
  Real matrix_array[] = {1.,0.5,0.25,0.5,2.,0.5,0.25,0.5,4.};
  matrix_map_indices.sizeUninitialized( num_matrices );
  matrix_map_indices[0] = 0;
  matrices.resize( num_matrices );
  matrices[0].shapeUninitialized( num_matrix_rows, num_matrix_rows );
  for ( int j=0; j<num_matrix_rows; j++ ){
    for ( int i=0; i<num_matrix_rows; i++ )
      matrices[0](i,j) = matrix_array[j*num_matrix_rows+i];
  }

  ExperimentCovariance exper_cov; 
  exper_cov.set_covariance_matrices( matrices, diagonals, scalars,
				     matrix_map_indices,
				     diagonal_map_indices, 
				     scalar_map_indices );

  // Test determinant and log_determinant
  BOOST_CHECK_CLOSE(exper_cov.determinant(), 6.75, 1.0e-12);
  BOOST_CHECK_CLOSE(exper_cov.log_determinant(), std::log(6.75), 1.0e-12);

  int num_residuals = 3;
  Real residual_array[] = {1.,2.,4.};
  RealVector residual( Teuchos::Copy, residual_array, num_residuals );

  // Test application of the covariance inverse to residual vector
  Real prod = exper_cov.apply_experiment_covariance( residual );
  BOOST_CHECK(  std::abs( prod - 16./3. ) < 
		10.*std::numeric_limits<double>::epsilon() );

  // Test application of the sqrt of the covariance inverse to residual vector
  RealVector result;
  exper_cov.apply_experiment_covariance_inverse_sqrt( residual, result );
  prod = result.dot( result );
  BOOST_CHECK( std::abs( prod - 16./3. ) < 
	       10.*std::numeric_limits<double>::epsilon() ); 
  
  // Test application of the sqrt of the covariance inverse to matrix of 
  // gradient vectors
  RealMatrix scaled_grads;
  Real grads_array[] = {1.,2.,2.,4.,4.,8.};
  RealMatrix grads( Teuchos::Copy, grads_array, 2, 2, 3 );
  exper_cov.apply_experiment_covariance_inverse_sqrt_to_gradients( grads, 
								   scaled_grads);

  RealMatrix grammian( grads.numRows(), grads.numRows(), false );
  grammian.multiply( Teuchos::NO_TRANS, Teuchos::TRANS, 1.0, scaled_grads, 
		     scaled_grads, 0. );
  BOOST_CHECK(  std::abs( grammian(0,0) - 16./3. ) < 
		10.*std::numeric_limits<double>::epsilon() );
  BOOST_CHECK(  std::abs( grammian(1,1) - 64./3. ) < 
		20.*std::numeric_limits<double>::epsilon() );

  // Test application of the sqrt of the covariance inverse to matrix of 
  // Hessian matrices
  Real hessian_0_array[] = {4., 2., 2., 2.};
  RealSymMatrix hessian_0( Teuchos::Copy, false, hessian_0_array, 2, 2 );
  RealSymMatrix hessian_1( hessian_0 );
  RealSymMatrix hessian_2( hessian_0 );
  hessian_1 *= 2.;
  hessian_2 *= 4.;

  RealSymMatrixArray hessians( 3 );
  hessians[0] = hessian_0;
  hessians[1] = hessian_1;
  hessians[2] = hessian_2;
  RealSymMatrixArray scaled_hessians;
  exper_cov.apply_experiment_covariance_inverse_sqrt_to_hessians( hessians,
							  scaled_hessians );

  Real exact_scaled_hessian_0_array[] = {4.,2.,2.,2.};
  Real exact_scaled_hessian_1_array[] = {4.53557367611073,2.26778683805536,
				   2.26778683805536,2.26778683805536};
  Real exact_scaled_hessian_2_array[] = {6.98297248755176,3.49148624377588,
				   3.49148624377588,3.49148624377588};
  RealSymMatrix exact_scaled_hessian_0( Teuchos::View, false, 
				  exact_scaled_hessian_0_array, 2, 2 );
  RealSymMatrix exact_scaled_hessian_1( Teuchos::View, false, 
				  exact_scaled_hessian_1_array, 2, 2 );
  RealSymMatrix exact_scaled_hessian_2( Teuchos::View, false, 
				  exact_scaled_hessian_2_array, 2, 2 );

  scaled_hessians[0] -= exact_scaled_hessian_0;
  scaled_hessians[1] -= exact_scaled_hessian_1;
  scaled_hessians[2] -= exact_scaled_hessian_2;
  
  BOOST_CHECK( scaled_hessians[0].normInf() < 
	       100.*std::numeric_limits<double>::epsilon() );
  BOOST_CHECK( scaled_hessians[1].normInf() < 
	       100.*std::numeric_limits<double>::epsilon() );
  BOOST_CHECK( scaled_hessians[2].normInf() < 
	       100.*std::numeric_limits<double>::epsilon() );

  // Test extraction of main diagonal
  RealVector diagonal;
  exper_cov.get_main_diagonal( diagonal );

  Real exact_diagonal_array[] = {1.,2.,4.};
  RealVector exact_diagonal(Teuchos::View, exact_diagonal_array, 3 );
  
  exact_diagonal -= diagonal;
  BOOST_CHECK( exact_diagonal.normInf() < 
	       10.*std::numeric_limits<double>::epsilon() );
  
}

void test_mixed_scalar_diagonal_full_block_covariance_matrix()
{
  std::vector<RealMatrix> matrices;
  std::vector<RealVector> diagonals;
  RealVector scalars;
  IntVector matrix_map_indices, diagonal_map_indices, scalar_map_indices;

  // Experiment covariance matrix consists of the following blocks
  // scalar_1, diagonal_1, matrix_1, scalar_2, scalar_3,

  // MATLAB CODE: 
  // A = [1,0.5,0.25;0.5,2,0.5;0.25,0.5,4.]; B = eye(9); B(1,1)=1.; 
  // B(8,8) = 2.; B(9,9) = 4.; B(3,3) = 2.; B(4,4) = 4.; B(5:7,5:7)=A;
  // d = [1., 1., 2., 4., 1., 2., 4., 2., 4.]';
  // chol(inv(B))*d; r'*r

  // Generate scalar matrix blocks
  int num_scalars = 3;
  Real scalar_array[] = {1.,2.,4.};
  int scalar_map_index_array[] = {0, 3, 4};
  scalars.sizeUninitialized( num_scalars );
  scalar_map_indices.sizeUninitialized( num_scalars );
  for ( int i=0; i<num_scalars; i++ ){
    scalars[i] = scalar_array[i];
    scalar_map_indices[i] = scalar_map_index_array[i];
  }

  // Generate diagonal covariance matrix blocks
  int num_diags = 1;
  int num_diag_entries = 3;
  Real diagonal_array[] = {1.,2.,4.};
  diagonal_map_indices.sizeUninitialized( num_diags );
  diagonal_map_indices[0] = 1;
  diagonals.resize( num_diags );
  diagonals[0].sizeUninitialized( num_diag_entries );
  for ( int i=0; i<num_diag_entries; i++ ){
    diagonals[0][i] = diagonal_array[i];
  }
  
  // Generate full covariance matrix blocks
  int num_matrices=1;
  int num_matrix_rows = 3;
  Real matrix_array[] = {1.,0.5,0.25,0.5,2.,0.5,0.25,0.5,4.};
  matrix_map_indices.sizeUninitialized( num_matrices );
  matrix_map_indices[0] = 2;
  matrices.resize( num_matrices );
  matrices[0].shapeUninitialized( num_matrix_rows, num_matrix_rows );
  for ( int j=0; j<num_matrix_rows; j++ ){
    for ( int i=0; i<num_matrix_rows; i++ )
      matrices[0](i,j) = matrix_array[j*num_matrix_rows+i];
  }

  ExperimentCovariance exper_cov; 
  exper_cov.set_covariance_matrices( matrices, diagonals, scalars,
				     matrix_map_indices,
				     diagonal_map_indices, 
				     scalar_map_indices );

  // Test determinant and log_determinant
  BOOST_CHECK_CLOSE(exper_cov.determinant(), 432.0, 1.0e-12);
  BOOST_CHECK_CLOSE(exper_cov.log_determinant(), std::log(432.0), 1.0e-12);

  int num_residuals = 9;
  Real residual_array[] = {1., 1., 2., 4., 1., 2., 4., 2., 4.};
  RealVector residual( Teuchos::Copy, residual_array, num_residuals );

  // Test application of the covariance inverse to residual vector
  Real prod = exper_cov.apply_experiment_covariance( residual );
  BOOST_CHECK( std::abs( prod - 58./3. ) < 
	       20.*std::numeric_limits<double>::epsilon() );

  // Test application of the sqrt of the covariance inverse to residual vector
  RealVector result;
  exper_cov.apply_experiment_covariance_inverse_sqrt( residual, result );
  prod = result.dot( result );
  BOOST_CHECK( std::abs( prod - 58./3. ) < 
	       10.*std::numeric_limits<double>::epsilon() );

  // Test application of the sqrt of the covariance inverse to matrix of 
  // gradient vectors
  RealMatrix scaled_grads;
  Real grads_array[] = { 1., 2., 1., 2., 2., 4., 4., 8., 1., 2., 2., 4., 4., 8.,
			 2., 4., 4., 8. };
  RealMatrix grads( Teuchos::Copy, grads_array, 2, 2, 9 );
  exper_cov.apply_experiment_covariance_inverse_sqrt_to_gradients( grads, 
								   scaled_grads);

  RealMatrix grammian( grads.numRows(), grads.numRows(), false );
  grammian.multiply( Teuchos::NO_TRANS, Teuchos::TRANS, 1.0, scaled_grads, 
		     scaled_grads, 0. );
  BOOST_CHECK(  std::abs( grammian(0,0) - 58./3. ) < 
		20.*std::numeric_limits<double>::epsilon() );
  BOOST_CHECK(  std::abs( grammian(1,1) - 232./3. ) < 
		20.*std::numeric_limits<double>::epsilon() );

  // Test application of the sqrt of the covariance inverse to matrix of 
  // Hessian matrices
  Real hessian_0_array[] = {4., 2., 2., 2.};
  RealSymMatrix hessian_0( Teuchos::Copy, false, hessian_0_array, 2, 2 );
  RealSymMatrix hessian_1( hessian_0 ), hessian_2( hessian_0 ),
    hessian_3( hessian_0 ),  hessian_4( hessian_0 ),
    hessian_5( hessian_0 ),  hessian_6( hessian_0 ),
    hessian_7( hessian_0 ), hessian_8( hessian_0 );
  hessian_1 *= 1.;  hessian_2 *= 2.;  hessian_3 *= 4.;  hessian_4 *= 1.;
  hessian_5 *= 2.;  hessian_6 *= 4.;  hessian_7 *= 2.;  hessian_8 *= 4.;

  RealSymMatrixArray hessians( 9 );
  hessians[0] = hessian_0;  hessians[1] = hessian_1;
  hessians[2] = hessian_2;  hessians[3] = hessian_3;
  hessians[4] = hessian_4;  hessians[5] = hessian_5;
  hessians[6] = hessian_6;  hessians[7] = hessian_7;
  hessians[8] = hessian_8;

  RealSymMatrixArray scaled_hessians;
  exper_cov.apply_experiment_covariance_inverse_sqrt_to_hessians( hessians, 
							       scaled_hessians );

  Real exact_scaled_hessian_0_array[] = {4.,2.,2.,2.};
  Real exact_scaled_hessian_1_array[] = {4.,2.,2.,2.};
  Real exact_scaled_hessian_2_array[] = {8./std::sqrt(2.),4./std::sqrt(2.),
				   4./std::sqrt(2.),4./std::sqrt(2.)};
  Real exact_scaled_hessian_3_array[] = {8., 4., 4., 4.};
  Real exact_scaled_hessian_4_array[] = {4.,2.,2.,2.};
  Real exact_scaled_hessian_5_array[] = {4.53557367611073,2.26778683805536,
				   2.26778683805536,2.26778683805536};
  Real exact_scaled_hessian_6_array[] = {6.98297248755176,3.49148624377588,
				   3.49148624377588,3.49148624377588};
  Real exact_scaled_hessian_7_array[] = {8./std::sqrt(2.),4./std::sqrt(2.),
				   4./std::sqrt(2.),4./std::sqrt(2.)};
  Real exact_scaled_hessian_8_array[] = {8., 4., 4., 4.};

  RealSymMatrix exact_scaled_hessian_0( Teuchos::View, false, 
				  exact_scaled_hessian_0_array, 2, 2 );
  RealSymMatrix exact_scaled_hessian_1( Teuchos::View, false, 
				  exact_scaled_hessian_1_array, 2, 2 );
  RealSymMatrix exact_scaled_hessian_2( Teuchos::View, false, 
				  exact_scaled_hessian_2_array, 2, 2 );
  RealSymMatrix exact_scaled_hessian_3( Teuchos::View, false, 
				  exact_scaled_hessian_3_array, 2, 2 );
  RealSymMatrix exact_scaled_hessian_4( Teuchos::View, false, 
				  exact_scaled_hessian_4_array, 2, 2 );
  RealSymMatrix exact_scaled_hessian_5( Teuchos::View, false, 
				  exact_scaled_hessian_5_array, 2, 2 );
  RealSymMatrix exact_scaled_hessian_6( Teuchos::View, false, 
				  exact_scaled_hessian_6_array, 2, 2 );
  RealSymMatrix exact_scaled_hessian_7( Teuchos::View, false, 
				  exact_scaled_hessian_7_array, 2, 2 );
  RealSymMatrix exact_scaled_hessian_8( Teuchos::View, false, 
				  exact_scaled_hessian_8_array, 2, 2 );

  scaled_hessians[0] -= exact_scaled_hessian_0;
  scaled_hessians[1] -= exact_scaled_hessian_1;
  scaled_hessians[2] -= exact_scaled_hessian_2;
  scaled_hessians[3] -= exact_scaled_hessian_3;
  scaled_hessians[4] -= exact_scaled_hessian_4;
  scaled_hessians[5] -= exact_scaled_hessian_5;
  scaled_hessians[6] -= exact_scaled_hessian_6;
  scaled_hessians[7] -= exact_scaled_hessian_7;
  scaled_hessians[8] -= exact_scaled_hessian_8;
  
  BOOST_CHECK( scaled_hessians[0].normInf() < 
	       100.*std::numeric_limits<double>::epsilon() );
  BOOST_CHECK( scaled_hessians[1].normInf() < 
	       100.*std::numeric_limits<double>::epsilon() );
  BOOST_CHECK( scaled_hessians[2].normInf() < 
	       100.*std::numeric_limits<double>::epsilon() );
  BOOST_CHECK( scaled_hessians[3].normInf() < 
	       100.*std::numeric_limits<double>::epsilon() );
  BOOST_CHECK( scaled_hessians[4].normInf() < 
	       100.*std::numeric_limits<double>::epsilon() );
  BOOST_CHECK( scaled_hessians[5].normInf() < 
	       100.*std::numeric_limits<double>::epsilon() );
  BOOST_CHECK( scaled_hessians[6].normInf() < 
	       100.*std::numeric_limits<double>::epsilon() );
  BOOST_CHECK( scaled_hessians[7].normInf() < 
	       100.*std::numeric_limits<double>::epsilon() );
  BOOST_CHECK( scaled_hessians[8].normInf() < 
	       100.*std::numeric_limits<double>::epsilon() );


  // Test extraction of main diagonal
  RealVector diagonal;
  exper_cov.get_main_diagonal( diagonal );

  Real exact_diagonal_array[] = {1.,1.,2.,4.,1.,2.,4.,2.,4.};
  RealVector exact_diagonal(Teuchos::View, exact_diagonal_array, 9 );
  
  exact_diagonal -= diagonal;
  BOOST_CHECK( exact_diagonal.normInf() < 
	       10.*std::numeric_limits<double>::epsilon() );

  // Test conversion to correlation matrix
  // Matlab for correlation matrix
  // std_dev = sqrt(diag(B)); correl_mat = diag(1./std_dev)*B*diag(1./std_dev);
  RealSymMatrix exact_correl(9);
  for (int i=0; i<9; ++i)
    exact_correl(i,i) = 1.0;
  // update off-diagonal entries for the full matrix block
  for (int i=0; i<3; ++i)
    for (int j=0; j<i; ++j)
      exact_correl(4+i,4+j) = matrices[0](i,j) / 
        std::sqrt(matrices[0](i,i)) / std::sqrt(matrices[0](j,j));

  RealSymMatrix calc_correl;
  exper_cov.as_correlation(calc_correl);

  calc_correl -= exact_correl;
  BOOST_CHECK( calc_correl.normInf() < 
               10.*std::numeric_limits<double>::epsilon() );
}

void test_linear_interpolate_1d_no_extrapolation()
{
  // Generate field coordinates on [0,1]
  int num_field_pts = 6;
  Real field_pts_array[] = {1./6.,2./5.,7./12.,2./3.,6./7.,1.};
  RealMatrix field_pts( Teuchos::View, field_pts_array, 1, num_field_pts, 1 );
  // Generate field data which is the 3rd degree Legendre polynomial 1/2(5x^3-3x)
  RealVector field_vals( num_field_pts, false );
  for ( int i=0; i<num_field_pts; i++)
    field_vals[i] = 0.5*(5.*std::pow(field_pts(i,0),3)-3.*field_pts(i,0));
  
  // Generate simulation coordinates on equally spaced on [0,1]
  int num_sim_pts = 11;
  RealMatrix sim_pts( num_sim_pts, 1, false );
  Real dx = 1./(num_sim_pts-1); 
  for ( int i=0; i<num_sim_pts; i++)
    sim_pts(i,0) = i*dx;
  // Generate simulation data which is the 2nd degree Legendre polynomial
  // 1/2(3x^2-1)
  RealVector sim_vals( num_sim_pts, false );
  for ( int i=0; i<num_sim_pts; i++)
    sim_vals[i] = 0.5*(3.*std::pow(sim_pts(i,0),2)-1.);
  // Generate gradients of simulation data which is [3x,4x]
  int num_vars = 2;
  RealMatrix sim_grads( num_vars, num_sim_pts, false );
  for ( int i=0; i<num_sim_pts; i++){
    sim_grads(0,i) = 3.*sim_pts(i,0);
    sim_grads(1,i) = 4.*sim_pts(i,0);
  }
  // Generate gradients of simulation data which we assume is also 
  // [3x+0.2,3x+0.05;3x+0.05,3x+0.1]
  // Note interp sets num_vars from gradients and so hessian must be
  // consistent with grads
  RealSymMatrixArray sim_hessians( num_sim_pts );
  for ( int i=0; i<num_sim_pts; i++){
    sim_hessians[i].shapeUninitialized(num_vars);
    //symmetric so do not have to set all entries, just upper triangular ones
    sim_hessians[i](0,0)=3.*sim_pts(i,0)+0.2;
    sim_hessians[i](0,1)=3.*sim_pts(i,0)+0.05;
    sim_hessians[i](1,1)=3.*sim_pts(i,0)+0.1;
  }

  // Interpolate the simulation data onto the coordinates of the field data
  RealVector interp_vals;
  RealMatrix interp_grads;
  RealSymMatrixArray interp_hessians;
  linear_interpolate_1d( sim_pts, sim_vals, sim_grads, sim_hessians, 
			 field_pts, interp_vals, interp_grads, interp_hessians );
  interp_vals -= field_vals;

  Real diff_array[] = {-2.16574074074074e-1,1.8e-1,3.91261574074074e-1,
		       4.29259259259259e-1,3.17084548104956e-1,0.0};
  RealVector diff( Teuchos::View, diff_array, num_field_pts );
  diff -= interp_vals;
  BOOST_CHECK( diff.normInf() < 10.*std::numeric_limits<double>::epsilon() );

  RealMatrix true_grads(num_vars,num_field_pts,false);
  for ( int i=0; i<num_field_pts; i++){
    true_grads(0,i) = 3.*field_pts(i,0);
    true_grads(1,i) = 4.*field_pts(i,0);
  }
  true_grads -= interp_grads;
  BOOST_CHECK( true_grads.normInf()<10.*std::numeric_limits<double>::epsilon() );

  RealSymMatrixArray true_hessians( num_sim_pts );
  for ( int i=0; i<num_field_pts; i++){
    true_hessians[i].shapeUninitialized(num_vars);
    //symmetric so do not have to set all entries, just upper triangular ones
    true_hessians[i](0,0)=3.*field_pts(i,0)+0.2;
    true_hessians[i](0,1)=3.*field_pts(i,0)+0.05;
    true_hessians[i](1,1)=3.*field_pts(i,0)+0.1;
    true_hessians[i] -= interp_hessians[i];
    BOOST_CHECK( true_hessians[i].normInf()<
		 10.*std::numeric_limits<double>::epsilon() );
  }
}

void test_linear_interpolate_1d_with_extrapolation()
{
  // Linear interpolate uses constant extrapolation

  // Generate field coordinates on [-1/6,11/10]
  int num_field_pts = 6;
  Real field_pts_array[] = {-1./6.,2./5.,7./12.,2./3.,6./7.,11./10.};
  RealMatrix field_pts( Teuchos::View, field_pts_array, 1, num_field_pts, 1 );
  // Generate field data which is the 3rd degree Legendre polynomial 1/2(5x^3-3x)
  RealVector field_vals( num_field_pts, false );
  for ( int i=0; i<num_field_pts; i++)
    field_vals[i] = 0.5*(5.*std::pow(field_pts(i,0),3)-3.*field_pts(i,0));
  
  // Generate simulation coordinates on equally spaced on [0,1]
  int num_sim_pts = 11;
  RealMatrix sim_pts( num_sim_pts, 1, false );
  Real dx = 1./(num_sim_pts-1); 
  for ( int i=0; i<num_sim_pts; i++)
    sim_pts(i,0) = i*dx;
  // Generate simulation data which is the 2nd degree Legendre polynomial
  // 1/2(3x^2-1)
  RealVector sim_vals( num_sim_pts, false );
  for ( int i=0; i<num_sim_pts; i++)
    sim_vals[i] = 0.5*(3.*std::pow(sim_pts(i,0),2)-1.);

  // Interpolate the simulation data onto the coordinates of the field data
  RealVector interp_vals;
  RealMatrix sim_grads, interp_grads;
  RealSymMatrixArray sim_hessians, interp_hessians;
  linear_interpolate_1d( sim_pts, sim_vals, sim_grads, sim_hessians, 
			 field_pts, interp_vals, interp_grads, interp_hessians );
  interp_vals -= field_vals;

  Real diff_array[] = {-7.38425925925926e-1,1.8e-1,3.91261574074074e-1,
		       4.29259259259259e-1,3.17084548104956e-1,-6.775e-1};
  RealVector diff( Teuchos::View, diff_array, num_field_pts );
  diff -= interp_vals;
  BOOST_CHECK( diff.normInf() < 10.*std::numeric_limits<double>::epsilon() );
}

/*void test_build_hessian_of_sum_square_residuals_from_function_hessians()
{
  int num_residuals = 3;
  RealSymMatrixArray func_hessians( num_residuals );
  RealMatrix func_gradients( 2, num_residuals, false );
  RealVector residuals( num_residuals );

  Real pts_array[] = {-1,-1,-0.5,0.5,1./3.,2./3.};
  RealMatrix pts( Teuchos::View, pts_array, 2, 2, 3 );

  for ( int i=0; i<num_residuals; i++ ){
    Real x = pts(0,i), y = pts(1,i);
    Real x2 = x*x, y2 = y*y;
    residuals[i] = (2.*x2-y2)*(2.*x2-y2)/10.-x2*y2; 
    // The following will not work. Build hessian assumes 
    // residual = (approx-data)
    //residuals[i] = x2*y2-(2.*x2-y2)*(2.*x2-y2)/10.;
    func_gradients(0,i) = 4./5.*x*(2.*x2-y2);
    func_gradients(1,i) = 2./5.*y*(y2-2.*x2);
    func_hessians[i].shape( 2 );
    func_hessians[i](0,0) = -4./5.*(y2-6.*x2);
    func_hessians[i](1,0) = -8.*x*y/5.;
    func_hessians[i](1,1) = 2./5.*(3.*y2-2.*x2);
  }

  ActiveSet set(num_residuals, 2); set.request_values(7);
  Response resp(SIMULATION_RESPONSE, set);
  resp.function_values(residuals);
  resp.function_gradients(func_gradients);
  resp.function_hessians(func_hessians);

  // -------------------------------------- //
  // Build hessian without noise covariance
  // -------------------------------------- //

  // If no noise covariance specify exper_cov as empty
  ExperimentCovariance exper_cov;

  RealSymMatrix ssr_hessian;
  build_hessian_of_sum_square_residuals_from_response(resp, exper_cov,
						      ssr_hessian);
  // hessian computed for ssr= r'r/2
  RealSymMatrix truth_ssr_hessian( 2 );
  for ( int i=0; i<num_residuals; i++ ){
    Real x = pts(0,i), y = pts(1,i);
    Real x2 = x*x, x3 = x2*x, x4 = x2*x2, x5 = x3*x2, x6 = x4*x2,
      y2 = y*y, y3 = y2*y, y4 = y2*y2, y5 = y3*y2, y6 = y4*y2;
    truth_ssr_hessian(0,0) += 2./25.*( 10.*x2*y2*(y2-6.*x2)-
					(y2-14.*x2)*(2.*x2-y2)*(2.*x2-y2) );
    truth_ssr_hessian(1,0) += 4./25.*y*x*( 10.*x2*y2-3.*(2.*x2-y2)*(2.*x2-y2) );
    truth_ssr_hessian(1,1) += 1./25.*( 10.*x2*y2*(2.*x2-3.*y2) + 
				       (7.*y2-2.*x2)*(2.*x2-y2)*(2.*x2-y2) );
  }
  // hack until build hessian can use covariance for multiple experiments
  truth_ssr_hessian *=2;
  
  truth_ssr_hessian -= ssr_hessian;
  BOOST_CHECK( truth_ssr_hessian.normInf() < 
	       10.*std::numeric_limits<double>::epsilon() );

  // -------------------------------------- //
  // Build hessian with noise covariance
  // -------------------------------------- //

  // Fill exper_cov with noise covariance
  std::vector<RealMatrix> matrices;
  std::vector<RealVector> diagonals;
  RealVector scalars;
  IntVector matrix_map_indices, diagonal_map_indices, scalar_map_indices;

  // Experiment covariance matrix consists of the following blocks
  // scalar_1, matrix_1

  // MATLAB CODE: 
  //
  //  A = [1,0.5;0.5,2.]; S = eye(3); S(1,1)=1.; S(2:3,2:3)=A;
  //  U = chol( inv(S) );

  //  r = [-0.9 -0.05625 -0.04444444444444444]';
  //  g = [-0.8, -0.1, -0.05925925925925925; 0.4, -0.05, 0.05925925925925925];
  //  h1 = [4,-1.6;-1.6,0.4]; h2=[1 0.4;0.4,0.1]; 
  //  h3 =[0.1777777777777778,-0.3555555555555555;
  //  -0.3555555555555555, 0.4444444444444445];

  //  gnewton_hess = g*inv(S)*g';
  //  rs = r'*inv(S);
  //  hess = gnewton_hess + rs(1)*h1+rs(2)*h2+rs(3)*h3   

  // Generate scalar matrix blocks
  int num_scalars = 1;
  Real scalar_array[] = {1.};
  int scalar_map_index_array[] = {0};
  scalars.sizeUninitialized( num_scalars );
  scalar_map_indices.sizeUninitialized( num_scalars );
  for ( int i=0; i<num_scalars; i++ ){
    scalars[i] = scalar_array[i];
    scalar_map_indices[i] = scalar_map_index_array[i];
  }

  // Generate full covariance matrix blocks
  int num_matrices=1;
  int num_matrix_rows = 2;
  Real matrix_array[] = {1.,0.5,0.5,2.};
  matrix_map_indices.sizeUninitialized( num_matrices );
  matrix_map_indices[0] = 1;
  matrices.resize( num_matrices );
  matrices[0].shapeUninitialized( num_matrix_rows, num_matrix_rows );
  for ( int j=0; j<num_matrix_rows; j++ ){
    for ( int i=0; i<num_matrix_rows; i++ )
      matrices[0](i,j) = matrix_array[j*num_matrix_rows+i];
  }

  exper_cov.set_covariance_matrices( matrices, diagonals, scalars,
				     matrix_map_indices,
				     diagonal_map_indices, 
				     scalar_map_indices );
  
  // must reset ssr_hessian because if it is the right size
  // the build_hessians... function will assume we want to add to it
  ssr_hessian.shape(0);
  build_hessian_of_sum_square_residuals_from_response(resp, exper_cov,
						      ssr_hessian);

  Real truth_noise_scaled_ssr_hessian_array[] = {-3.00319615912208e+00,
						 1.10723495982755e+00,
						 1.10723495982755e+00,
						 -2.02746423672350e-01};
  RealSymMatrix truth_noise_scaled_ssr_hessian( Teuchos::View, true,
					  truth_noise_scaled_ssr_hessian_array,
					  2, 2 );

  // hack until build hessian can use covariance for multiple experiments
  truth_noise_scaled_ssr_hessian *=2;
  
  truth_noise_scaled_ssr_hessian -= ssr_hessian;
  BOOST_CHECK( truth_noise_scaled_ssr_hessian.normInf() < 
	       100.*std::numeric_limits<double>::epsilon() );

  // -------------------------------------- //
  // Build Gauss-Newton hessian with noise covariance
  // -------------------------------------- //
  ActiveSet set1(num_residuals, 2); set1.request_values(3);
  Response resp1(SIMULATION_RESPONSE, set1);
  resp1.function_values(residuals);
  resp1.function_gradients(func_gradients);
  resp1.function_hessians(func_hessians);
  
  // must reset ssr_hessian because if it is the right size
  // the build_hessians... function will assume we want to add to it
  ssr_hessian.shape(0);
  build_hessian_of_sum_square_residuals_from_response(resp1, exper_cov,
						      ssr_hessian);

  Real truth_noise_scaled_gn_ssr_hessian_array[] = {6.50048990789732e-01,
						    -3.15445816186557e-01,
						    -3.15445816186557e-01,
						    1.66556927297668e-01};
  RealSymMatrix truth_noise_scaled_gn_ssr_hessian( Teuchos::View, true,
					truth_noise_scaled_gn_ssr_hessian_array,
					2, 2 );

  // hack until build hessian can use covariance for multiple experiments
  truth_noise_scaled_gn_ssr_hessian *=2;
  
  truth_noise_scaled_gn_ssr_hessian -= ssr_hessian;
  BOOST_CHECK( truth_noise_scaled_gn_ssr_hessian.normInf() < 
	       100.*std::numeric_limits<double>::epsilon() );
}*/

void test_symmetric_eigenvalue_decomposition()
{
  Real matrix_array[] = { 1.64, 0.48, 0.48, 1.36 };
  RealSymMatrix matrix( Teuchos::View, false, matrix_array, 2, 2 );

  RealVector eigenvalues;
  RealMatrix eigenvectors;
  symmetric_eigenvalue_decomposition( matrix, eigenvalues, eigenvectors );

  Real truth_eigenvalues_array[] = {1.,2.};
  RealVector truth_eigenvalues( Teuchos::View, truth_eigenvalues_array, 2 );
 
  truth_eigenvalues -=  eigenvalues;
  BOOST_CHECK( truth_eigenvalues.normInf() < 
	       10.*std::numeric_limits<double>::epsilon() );


  Real truth_eigenvectors_array[] ={ 0.6, -0.8, -0.8, -0.6 };
  RealMatrix truth_eigenvectors( Teuchos::View, truth_eigenvectors_array, 2,2,2);

  truth_eigenvectors -= eigenvectors;
  BOOST_CHECK( truth_eigenvectors.normInf() < 
	       10.*std::numeric_limits<double>::epsilon() );
}

void test_get_positive_definite_covariance_from_hessian()
{
  // uncorrelated prior
  Real prior_chol_fact1[] = { 0.2, 0., 0., 0.2 };
  RealMatrix prior_L1(Teuchos::View, prior_chol_fact1, 2, 2, 2);

  // non positive definite matrix
  Real misfit_h_array1[] = { 0.92, 1.44, 1.44, 0.08 };
  RealSymMatrix misfit_hessian1(Teuchos::View, false, misfit_h_array1, 2, 2 );

  RealSymMatrix covariance1;
  NonDBayesCalibration::
    get_positive_definite_covariance_from_hessian(misfit_hessian1, prior_L1,
						  covariance1, NORMAL_OUTPUT);

  // MATLAB result (no truncation of eigenvalues)
  //Real truth_cov1_array[] ={  0.038703703703704, -0.002222222222222,
  //			       -0.002222222222222,  0.04 };
  // Dakota result (truncation of 1 eigenvalue)
  Real truth_cov1_array[] ={ 3.81037037037037e-02, -1.42222222222222e-03,
			    -1.42222222222222e-03,  3.89333333333333e-02 };
  RealSymMatrix truth_covariance1(Teuchos::View, false, truth_cov1_array, 2, 2);

  truth_covariance1 -= covariance1;
  BOOST_CHECK( truth_covariance1.normInf() < 
	       10.*std::numeric_limits<double>::epsilon() );

  //////////////////////////////////////////////////////////////////////////////

  // correlated prior
  Real prior_chol_fact2[] = { 0.2, 0.05, 0., 0.2 };
  RealMatrix prior_L2(Teuchos::View, prior_chol_fact2, 2, 2, 2);

  // positive definite matrix
  Real misfit_h_array2[] = { 1.64, 0.48, 0.48, 1.36 };
  RealSymMatrix misfit_hessian2(Teuchos::View, false, misfit_h_array2, 2, 2);

  RealSymMatrix covariance2;
  NonDBayesCalibration::
    get_positive_definite_covariance_from_hessian(misfit_hessian2, prior_L2,
						  covariance2, NORMAL_OUTPUT);

  // MATLAB and Dakota result (no truncation of eigenvalues):
  Real truth_cov2_array[] = { 0.037120225312445, 0.008125330047527,
			      0.008125330047527, 0.039714838936807 };
  RealSymMatrix truth_covariance2(Teuchos::View, false, truth_cov2_array, 2, 2);

  truth_covariance2 -= covariance2;
  BOOST_CHECK( truth_covariance2.normInf() < 
	       10.*std::numeric_limits<double>::epsilon() );

  //////////////////////////////////////////////////////////////////////////////
  /*
  // zero misfit matrix: proposal covariance = prior covariance
  // can't perform symmetric eigen-deomposition for L'HL = 0
  Real misfit_h_array3[] = { 0., 0., 0., 0. };
  RealSymMatrix misfit_hessian3(Teuchos::View, false, misfit_h_array3, 2, 2);

  RealSymMatrix covariance3;
  NonDBayesCalibration::
    get_positive_definite_covariance_from_hessian(misfit_hessian3, prior_L2,
						  covariance3, NORMAL_OUTPUT);

  Real truth_cov3_array[] = { 4., 0.1, 0.1, 4.0025 };
  RealSymMatrix truth_covariance3(Teuchos::View, false, truth_cov3_array, 2, 2);

  truth_covariance3 -= covariance3;
  BOOST_CHECK( truth_covariance3.normInf() < 
	       10.*std::numeric_limits<double>::epsilon() );
  */
}

void test_matrix_symmetry()
{
  // Test non-square matrix
  RealMatrix test_rect_matrix(4,3, true);
  bool is_symm = is_matrix_symmetric(test_rect_matrix);
  BOOST_CHECK( !is_symm );

  Real matrix_array[] = { 1.0, 0.7, 0.7, 0.7, 
                          0.7, 1.0, 0.7, 0.7, 
                          0.7, 0.7, 1.0, 0.7, 
                          0.7, 0.7, 0.7, 1.0 };

  // Test symmetric matrix
  RealMatrix test_symm_mat(Teuchos::Copy, matrix_array, 4, 4, 4);
  is_symm = is_matrix_symmetric(test_symm_mat);
  BOOST_CHECK( is_symm );

  // Test non-symmetric square matrix
  RealMatrix test_nonsymm_mat(test_symm_mat);
  test_nonsymm_mat(1,0) = 0.5;
  is_symm = is_matrix_symmetric(test_nonsymm_mat);
  BOOST_CHECK( !is_symm );
}

} // end namespace TestFieldCovariance
} // end namespace Dakota

// NOTE: Boost.Test framework provides the main progran driver

//____________________________________________________________________________//

BOOST_AUTO_TEST_CASE( test_main )
//int test_main( int argc, char* argv[] )      // note the name!
{
  using namespace Dakota::TestFieldCovariance;

  // Test ExperimentData covariance matrix
  test_multiple_scalar_covariance_matrix();
  test_single_diagonal_block_covariance_matrix();
  test_single_full_block_covariance_matrix();
  test_mixed_scalar_diagonal_full_block_covariance_matrix();

  // Test field interpolation functions
  test_linear_interpolate_1d_no_extrapolation();
  test_linear_interpolate_1d_with_extrapolation();

  // Test hessian functions
  // Turn following test off until I can create an ExperimentData object
  //test_build_hessian_of_sum_square_residuals_from_function_hessians();
  test_get_positive_definite_covariance_from_hessian();

  // Test linear algebra routines
  test_symmetric_eigenvalue_decomposition();
  test_matrix_symmetry();

  int run_result = 0;
  BOOST_CHECK( run_result == 0 || run_result == boost::exit_success );

  //  return boost::exit_success;
}
