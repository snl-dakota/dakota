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
  Real prod = exper_cov.apply_experiment_covariance( residual );
  BOOST_CHECK( prod == 7. ); 
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
  Real prod = exper_cov.apply_experiment_covariance( residual );
  BOOST_CHECK( prod == 7. );

  
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
  Real prod = exper_cov.apply_experiment_covariance( residual );
  BOOST_CHECK( std::abs( prod - 16./3. ) < 
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
  Real residual_array[] = {1.,2.,4.};
  RealVector residual( Teuchos::Copy, residual_array, num_residuals );
  Real prod = exper_cov.apply_experiment_covariance( residual );
  BOOST_CHECK( std::abs( prod - 58./3. ) < 
	       10.*std::numeric_limits<double>::epsilon() );
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

  // Test field interpolation functions
  test_linear_interpolate_1d_no_extrapolation();
  test_linear_interpolate_1d_with_extrapolation();

  int run_result = 0;
  BOOST_CHECK( run_result == 0 || run_result == boost::exit_success );

  return boost::exit_success;
}
