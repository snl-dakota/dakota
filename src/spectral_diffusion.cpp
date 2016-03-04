#include "spectral_diffusion.hpp"
#include "sandia_rules.hpp"
#include "LinearAlgebra.hpp"
#include "dakota_linear_algebra.hpp"

namespace Dakota {

#define disp( a ) std::cout << #a << ": " << ( a ) << std::endl

#define PI 4.0*atan(1.0)

void gauss_legendre_pts_wts_1D( int level, RealVector& result_0, 
				RealVector& result_1 )
{
  Real wt_factor = 0.5;
  result_0.sizeUninitialized( level );
  result_1.sizeUninitialized( level );
  Real *x_ptr = result_0.values(), *w_ptr = result_1.values();
  if ( level <= 33 ) {
      // retrieve full precision tabulated values
      webbur::legendre_lookup_points( level, x_ptr );
      webbur::legendre_lookup_weights( level, w_ptr );
  }else{
    // compute pts and weights
    webbur::legendre_compute( level, x_ptr, w_ptr );
  }
  //result_1 *= wt_factor;  // polynomial weight fn -> PDF
};

void lagrange_interpolation_1d( const RealVector &samples, 
				const RealVector &abscissa,
				const RealVector &values,
				RealVector &result )
{
  int num_samples = samples.length(), num_abscissa = abscissa.length();
  result.sizeUninitialized( num_samples );
  for ( int k = 0; k < num_samples; k++ ){
    Real sample = samples[k];
    result[k] = 0.;
    for ( int i = 0; i < num_abscissa; i++ ){
      Real x_i = abscissa[i];
      Real basis = 1.;
      for ( int j = 0; j < num_abscissa; j++ ){
	if ( i != j ){
	  Real x_j = abscissa[j];
	  basis *= ( sample - x_j ) / ( x_i - x_j );
	}
      }
      result[k] += values[i] * basis;
    }
  }
}

void kronecker_product_2d(const RealMatrix & matrix1, const RealMatrix &matrix2,
			  RealMatrix &matrix ){
  if ( ( matrix1.numRows() != matrix2.numRows() ) || 
       ( matrix1.numCols() != matrix2.numCols() ) ){
    throw(std::runtime_error("kronecker_product_2d: matrices are inconsistent"));
  }
  
  int block_num_rows = matrix1.numRows();
  int matrix_num_rows = block_num_rows*block_num_rows;
  
  matrix.shapeUninitialized(matrix_num_rows, matrix_num_rows);

  // loop through blocks
  int start_col = 0;
  for (int j=0; j<block_num_rows; j++){
    int start_row = 0;
    for (int i=0; i<block_num_rows; i++){
      for (int k=0; k<block_num_rows; k++)
	for (int m=0; m<block_num_rows; m++)
	  matrix(m+start_row,k+start_col)=matrix2(m,k)*matrix1(i,j);
      start_row += block_num_rows;
    }
    start_col += block_num_rows;
  }
}

void get_chebyshev_points( int order, RealVector &points ){
  if ( order < 2 ){
    throw(std::runtime_error("get_chebshev_points: must specify order > 1"));
  }
  points.sizeUninitialized( order+1 );
  Real h =  PI / (Real)(order);
  for (int i=0; i<order+1; i++)
    points[i] = -std::cos( (Real)i*h );
}

void chebyshev_derivative_matrix(int order, RealMatrix &derivative_matrix,
				 RealVector &points ){
  if ( order < 2 ){
    std::string msg = "chebyshev_derivative_matrix: must specify order > 1";
    throw(std::runtime_error(msg));
  }
  
  // point will be returned in reverse order of those returned by the 
  // matlab cheb function
  get_chebyshev_points( order, points );
  
  RealVector scalars( order+1, false );
  scalars[0] = 2.; scalars[order] = 2.;
  for (int i=1; i<order; i++){
    if ( i%2==1 ) scalars[i] = -1.;
    else scalars[i] = 1.;
  }

  derivative_matrix.shapeUninitialized( order+1, order+1 );
  for (int i=0; i<order+1; i++){
    Real row_sum = 0.;	
    for (int j=0; j<order+1; j++){	
      Real denominator = (i==j) ? 1. : ( points[i]-points[j] );
      Real numerator = scalars[i] / scalars[j];
      derivative_matrix(i,j) = numerator / denominator;
      row_sum += derivative_matrix(i,j);
    }
    derivative_matrix(i,i) -= row_sum;
  }
    
  // I return points and calculate derivatives using reverse order of points
  // compared to what is used by Matlab cheb function thus the 
  // derivative matrix I return will be the negative of the matlab version
}

SpectralDiffusionModel::
SpectralDiffusionModel() : order_(0), numQOI_(1), kernel_(""), positivity_(true),
                           p_(1.0), L_(1.0), fieldMean_(1.0), fieldStdDev_(1.0) {};

SpectralDiffusionModel::~SpectralDiffusionModel() {};

void SpectralDiffusionModel::
initialize( int order, String kernel, const RealVector &bndry_conds,
            const RealVector &domain_limits ){

  // Set boundary_conditions
    if ( bndry_conds.length() != 2 ){
    std::string msg = "initialize: bndry_conds must have length 2";
    throw(std::runtime_error(msg));
  }
  boundaryConditions_.sizeUninitialized( 2 );
  boundaryConditions_.assign( bndry_conds );
  
  // Set limits of physical domain;
  if ( domain_limits.length() != 2 ){
    std::string msg = "initialize: domain_limits must have length 2";
    throw(std::runtime_error(msg));
  }
  physicalDomainLimits_.sizeUninitialized( 2 );
  physicalDomainLimits_.assign( domain_limits );

  // Get canonical collocation_points and derivative matrix
  order_ = order;
  RealVector chebyshev_points;
  chebyshev_derivative_matrix(order_, derivativeMatrix_, chebyshev_points );

  // Map chebyshev points on [-1,1] to [a,b]
  collocationPoints_.shapeUninitialized( 1, order_+1 );
  Real range = (physicalDomainLimits_[1]-physicalDomainLimits_[0]);
  for (int i=0; i<order_+1; i++)
    collocationPoints_(0,i) = range*(chebyshev_points[i]+1.)/2. + 
      physicalDomainLimits_[0];

  // Transform derivative matrix to account for mapping of points 
  // from [-1,1] to [a,b]
  for (int j=0; j<order_+1; j++)
    for (int i=0; i<order_+1; i++)
      derivativeMatrix_(i,j) *= 2. / range;

  // Get kernel specification String
  kernel_ = kernel;
  if (kernel_ == "exponential") {
    int num_mesh_points =collocationPoints_.numCols();

    // Form correlation matrix:
    RealMatrix Corr(num_mesh_points,num_mesh_points);
    for (int i = 0; i < num_mesh_points; i++) {
      for (int j = 0; j < num_mesh_points; j++) {
        Corr(i,j) = std::exp(-std::pow(std::abs(collocationPoints_(0,i) - collocationPoints_(0,j))/L_,p_));
      }
    }

    leftSingularVectors_ = Corr;
    RealMatrix VT;
    // svd gives: Corr = leftSingularVectors_*singularValues_*VT;
    svd(leftSingularVectors_, singularValues_, VT);
  }
}

void SpectralDiffusionModel::
form_collocation_matrix( const RealVector &diagonal, 
			 RealMatrix &collocation_matrix ) const {
  RealMatrix scaled_matrix( derivativeMatrix_.numRows(), 
			    derivativeMatrix_.numCols(), false );
  collocation_matrix.shapeUninitialized( derivativeMatrix_.numRows(), 
					 derivativeMatrix_.numCols() );
  for (int j=0; j<order_+1; j++)
    for (int i=0; i<order_+1; i++)
      scaled_matrix(i,j) = derivativeMatrix_(i,j) * diagonal[i];
  collocation_matrix.multiply( Teuchos::NO_TRANS, Teuchos::NO_TRANS, 1.0, 
			       derivativeMatrix_, scaled_matrix, 0.0 );
}


void SpectralDiffusionModel::
apply_boundary_conditions( RealMatrix &collocation_matrix, 
                           RealVector &forcing ) const {
  
  for (int j=0; j<order_+1; j++){
    collocation_matrix(0,j) = 0.; 
    collocation_matrix(order_,j) = 0.;  
  }
  collocation_matrix(0,0) = 1.; collocation_matrix(order_,order_) = 1.;
  forcing[0] = boundaryConditions_[0]; forcing[order_] = boundaryConditions_[1];
}

void SpectralDiffusionModel::
solve( const RealVector &diffusivity, const RealVector &forcing_in, 
       RealVector &solution ) const {
  // Ensure diffusivity is not negative
  bool negative_diffusivity = false;
  for (int i=0; i<order_+1; i++){
    if ( diffusivity[i] <= std::numeric_limits<double>::epsilon() ){
      negative_diffusivity = true;
      break;
    }
  }

  if ( negative_diffusivity ){
    std::string msg = "solve: diffusivity has negative components";
    throw(std::runtime_error(msg));
  }
  // Form linear system
  RealMatrix collocation_matrix;
  form_collocation_matrix( diffusivity, collocation_matrix );

  // Apply boundary conditions
  RealVector forcing( forcing_in ); // Copy forcing because it will be modified
  apply_boundary_conditions( collocation_matrix, forcing );

  // Solve linear system
  Pecos::qr_solve( collocation_matrix, forcing, solution );
}

void SpectralDiffusionModel::run( const RealVector &sample, 
                                  RealVector &solution ) const {

  // Evaluate diffusivity for given random sample
  RealVector diffusivity;
  diffusivity_function( sample, collocationPoints_, diffusivity );

  // Evaluate forcing for given random sample
  RealVector forcing;
  forcing_function( sample, collocationPoints_, forcing );

  // Solve the governing equations
  solve( diffusivity, forcing, solution );
}

void SpectralDiffusionModel::evaluate( const RealVector &sample, 
				       RealVector &qoi ) {
  RealVector solution;
  run( sample, solution );

  qoi_functional( solution, qoi );
}

void SpectralDiffusionModel::interpolate( const RealVector &mesh_values,
                                          const RealVector &interp_samples,
                                          RealVector &interp_values ) const {
  RealVector mesh_points( Teuchos::View, collocationPoints_.values(), order_+1 );
  lagrange_interpolation_1d( interp_samples, mesh_points, mesh_values, 
                             interp_values );
}

double SpectralDiffusionModel::integrate( const RealVector &mesh_values ) const {

  // Get Gauss-Legendre rule
  RealVector gl_pts, gl_wts;
  gauss_legendre_pts_wts_1D( order_, gl_pts, gl_wts );

  // Scale points from [-1,1] to to physical domain
  int range = physicalDomainLimits_[1] - physicalDomainLimits_[0];
  for (int i=0; i<gl_pts.length(); i++)
    gl_pts[i] = range*( gl_pts[i]+1.)/2.+physicalDomainLimits_[0];
  gl_wts *= range;
  
  // Interpolate mesh values onto quadrature nodes
  RealVector gl_vals;
  interpolate( mesh_values, gl_pts, gl_vals );
  return gl_vals.dot( gl_wts );
}

void SpectralDiffusionModel::qoi_functional( const RealVector &solution, 
                                             RealVector &result ) const {
  interpolate( solution, qoiCoords_, result );
};

void SpectralDiffusionModel::
diffusivity_function( const RealVector &sample, const RealMatrix &mesh_points,
                      RealVector &diffusivity ) const {
  int num_mesh_points = mesh_points.numCols(), num_stoch_dims = sample.length();
  diffusivity.size( num_mesh_points ); // initialize to zero

  if (kernel_ == "exponential") {
    for (int d=0; d<num_stoch_dims; d++) {
      for (int i=0; i<num_mesh_points; i++) {
        diffusivity[i] += singularValues_[d]*leftSingularVectors_(i,d)*sample[d];
      }
    }
    for (int i=0; i<num_mesh_points; i++) {
      if (positivity_)
        diffusivity[i] = std::exp(diffusivity[i] * fieldStdDev_) + fieldMean_;
      else
        diffusivity[i] = diffusivity[i] * fieldStdDev_ + fieldMean_;
    }
  }
  else if (kernel_ == "cosine" || kernel_ == "default") {
    for (int d=0; d<num_stoch_dims; d++) {
      Real dPI = PI*(Real)(d+1);
      for (int i=0; i<num_mesh_points; i++) {
        diffusivity[i] += std::cos(2.*dPI*mesh_points(0,i))/(dPI*dPI)*sample[d];
      }
    }
    for (int i=0; i<num_mesh_points; i++)
      diffusivity[i] = diffusivity[i] * fieldStdDev_ + fieldMean_;
  }
  else {
    std::string msg = "diffusivity_function(): unknown kernel";
    throw(std::runtime_error(msg));
  }
};
  
void SpectralDiffusionModel::forcing_function( const RealVector &sample, 
                                               const RealMatrix &mesh_points,
                                               RealVector &forcing ) const {
  int num_mesh_points = mesh_points.numCols();
  forcing.sizeUninitialized( num_mesh_points );
  for (int i=0; i<num_mesh_points; i++) {
    forcing[i] = -1.;
  }
};

void SpectralDiffusionModel::set_num_qoi( int num_qoi ){
  numQOI_ = num_qoi;
}

void SpectralDiffusionModel::get_collocation_points( RealMatrix &result ) const{
  result = collocationPoints_;
}

void SpectralDiffusionModel::get_derivative_matrix( RealMatrix &result) const{
  result = derivativeMatrix_;
}

} // namepsace Dakota
