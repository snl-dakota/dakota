#include "ExperimentDataUtils.hpp"
#include "dakota_global_defs.hpp"
// Boost.Test
#include <boost/test/minimal.hpp>

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
  Real exact_scaled_hessian_1_array[] = {8./std::sqrt(2),4./std::sqrt(2),
				   4./std::sqrt(2),4./std::sqrt(2)};
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
  Real exact_scaled_hessian_1_array[] = {8./std::sqrt(2),4./std::sqrt(2),
				   4./std::sqrt(2),4./std::sqrt(2)};
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
  matrices.resize( num_diags );
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
  Real exact_scaled_hessian_2_array[] = {8./std::sqrt(2),4./std::sqrt(2),
				   4./std::sqrt(2),4./std::sqrt(2)};
  Real exact_scaled_hessian_3_array[] = {8., 4., 4., 4.};
  Real exact_scaled_hessian_4_array[] = {4.,2.,2.,2.};
  Real exact_scaled_hessian_5_array[] = {4.53557367611073,2.26778683805536,
				   2.26778683805536,2.26778683805536};
  Real exact_scaled_hessian_6_array[] = {6.98297248755176,3.49148624377588,
				   3.49148624377588,3.49148624377588};
  Real exact_scaled_hessian_7_array[] = {8./std::sqrt(2),4./std::sqrt(2),
				   4./std::sqrt(2),4./std::sqrt(2)};
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
}

void test_linear_interpolate_1d_no_extrapolation()
{
  // Generate field coordinates on [0,1]
  int num_field_pts = 6;
  Real field_pts_array[] = {1./6.,2./5.,7./12.,2./3.,6./7.,1.};
  RealMatrix field_pts( Teuchos::View, field_pts_array, 1, 1, num_field_pts );
  // Generate field data which is the 3rd degree Legendre polynomial 1/2(5x^3-3x)
  RealVector field_vals( num_field_pts, false );
  for ( int i=0; i<num_field_pts; i++)
    field_vals[i] = 0.5*(5.*std::pow(field_pts(0,i),3)-3.*field_pts(0,i));
  
  // Generate simulation coordinates on equally spaced on [0,1]
  int num_sim_pts = 11;
  RealMatrix sim_pts( 1, num_sim_pts, false );
  Real dx = 1./(num_sim_pts-1); 
  for ( int i=0; i<num_sim_pts; i++)
    sim_pts(0,i) = i*dx;
  // Generate simulation data which is the 2nd degree Legendre polynomial
  // 1/2(3x^2-1)
  RealVector sim_vals( num_sim_pts, false );
  for ( int i=0; i<num_sim_pts; i++)
    sim_vals[i] = 0.5*(3.*std::pow(sim_pts(0,i),2)-1.);

  // Interpolate the simulation data onto the coordinates of the field data
  RealVector interp_vals;
  linear_interpolate_1d( sim_pts, sim_vals, field_pts, interp_vals );
  interp_vals -= field_vals;

  Real diff_array[] = {-2.16574074074074e-1,1.8e-1,3.91261574074074e-1,
		       4.29259259259259e-1,3.17084548104956e-1,0.0};
  RealVector diff( Teuchos::View, diff_array, num_field_pts );
  diff -= interp_vals;
  BOOST_CHECK( diff.normInf() < 10.*std::numeric_limits<double>::epsilon() );
}

void test_linear_interpolate_1d_with_extrapolation()
{
  // Linear interpolate uses constant extrapolation

  // Generate field coordinates on [-1/6,11/10]
  int num_field_pts = 6;
  Real field_pts_array[] = {-1./6.,2./5.,7./12.,2./3.,6./7.,11./10.};
  RealMatrix field_pts( Teuchos::View, field_pts_array, 1, 1, num_field_pts );
  // Generate field data which is the 3rd degree Legendre polynomial 1/2(5x^3-3x)
  RealVector field_vals( num_field_pts, false );
  for ( int i=0; i<num_field_pts; i++)
    field_vals[i] = 0.5*(5.*std::pow(field_pts(0,i),3)-3.*field_pts(0,i));
  
  // Generate simulation coordinates on equally spaced on [0,1]
  int num_sim_pts = 11;
  RealMatrix sim_pts( 1, num_sim_pts, false );
  Real dx = 1./(num_sim_pts-1); 
  for ( int i=0; i<num_sim_pts; i++)
    sim_pts(0,i) = i*dx;
  // Generate simulation data which is the 2nd degree Legendre polynomial
  // 1/2(3x^2-1)
  RealVector sim_vals( num_sim_pts, false );
  for ( int i=0; i<num_sim_pts; i++)
    sim_vals[i] = 0.5*(3.*std::pow(sim_pts(0,i),2)-1.);

  // Interpolate the simulation data onto the coordinates of the field data
  RealVector interp_vals;
  linear_interpolate_1d( sim_pts, sim_vals, field_pts, interp_vals );
  interp_vals -= field_vals;

  Real diff_array[] = {-7.38425925925926e-1,1.8e-1,3.91261574074074e-1,
		       4.29259259259259e-1,3.17084548104956e-1,-6.775e-1};
  RealVector diff( Teuchos::View, diff_array, num_field_pts );
  diff -= interp_vals;
  BOOST_CHECK( diff.normInf() < 10.*std::numeric_limits<double>::epsilon() );
}

} // end namespace TestFieldCovariance
} // end namespace Dakota

// NOTE: Boost.Test framework provides the main progran driver

//____________________________________________________________________________//

int test_main( int argc, char* argv[] )      // note the name!
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

  int run_result = 0;
  BOOST_CHECK( run_result == 0 || run_result == boost::exit_success );

  return boost::exit_success;
}
