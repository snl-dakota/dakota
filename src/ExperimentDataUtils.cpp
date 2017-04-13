#include "ExperimentDataUtils.hpp"
#include "DakotaResponse.hpp"
#include <iostream>

namespace Dakota {

/** This assumes the souce gradient/Hessian are size less or equal to
    the destination response, and that the leading part is to be populated. */
void copy_field_data(const RealVector& fn_vals, RealMatrix& fn_grad, 
		     const RealSymMatrixArray& fn_hess, size_t offset, 
		     size_t num_fns, Response& response)
{
  const ShortArray& asv = response.active_set_request_vector();
  for (size_t i=0; i<num_fns; i++) {
    if (asv[i] & 1) {
      response.function_value(fn_vals[i], offset+i);
    }
    // populate only the part of the gradients/Hessians for this
    // experiment, for the active submodel derivative variables
    if (asv[i] & 2) {
      size_t num_sm_cv = fn_grad.numRows();
      RealVector resp_grad = response.function_gradient_view(offset+i);
      resp_grad = 0.0;
      for (size_t j=0; j<num_sm_cv; ++j)
	resp_grad[j] = fn_grad(j, i);
    }
    if (asv[i] & 4) {
      size_t num_sm_cv = fn_hess[i].numRows();
      RealSymMatrix resp_hess = response.function_hessian_view(offset+i);
      resp_hess = 0.0;
      for (size_t j=0; j<num_sm_cv; ++j)
	for (size_t k=0; k<num_sm_cv; ++k)
	  resp_hess(j,k) = fn_hess[i](j,k);
    }
  }
}

/** This assumes the souce gradient/Hessian are size less or equal to
    the destination response, and that the leading part is to be populated. */
void copy_field_data(const RealVector& fn_vals, RealMatrix& fn_grad, 
		     const RealSymMatrixArray& fn_hess, size_t offset, 
		     size_t num_fns, short total_asv, Response& response)
{
  
  for (size_t i=0; i<num_fns; i++) {
    if (total_asv & 1) {
      response.function_value(fn_vals[i], offset+i);
    }
    if (total_asv & 2) {
      size_t num_sm_cv = fn_grad.numRows();
      RealVector resp_grad = response.function_gradient_view(offset+i);
      resp_grad = 0.0;
      for (size_t j=0; j<num_sm_cv; ++j)
	resp_grad[j] = fn_grad(j, i);
    }
    if (total_asv & 4) {
      size_t num_sm_cv = fn_hess[i].numRows();
      RealSymMatrix resp_hess = response.function_hessian_view(offset+i);
      resp_hess = 0.0;
      for (size_t j=0; j<num_sm_cv; ++j)
	for (size_t k=0; k<num_sm_cv; ++k)
	  resp_hess(j,k) = fn_hess[i](j,k); 
    }
  }
}

void interpolate_simulation_field_data( const Response &sim_resp, 
					const RealMatrix &exp_coords,
					size_t field_num, short total_asv,
					size_t interp_resp_offset,
					Response &interp_resp ){
  int outputLevel = 1;
  int DEBUG_OUTPUT = 0;

  const RealMatrix& sim_coords = sim_resp.field_coords_view(field_num);
  const RealVector sim_vals = sim_resp.field_values_view(field_num);
  RealMatrix sim_grads;
  // TODO(JDJ) : Decide if total_asv is fine grained enough. At the moment
  // it is per experiment. But we can use it at this level at the more
  // fine grained level of per field
  if ( total_asv & 2 )
    sim_grads = sim_resp.field_gradients_view(field_num);

  RealSymMatrixArray sim_hessians;
  if ( total_asv & 4 )
    sim_hessians = sim_resp.field_hessians_view(field_num);


  if (outputLevel >= DEBUG_OUTPUT){
    RealVector sim_values;
    sim_values = sim_resp.field_values_view(field_num);
    Cout << "sim_values " << sim_values << '\n';
  }

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "sim_coords " << sim_coords << '\n';
    Cout << "exp_coords " << exp_coords << '\n';
  }
  
  RealVector interp_vals;
  RealMatrix interp_grads;
  RealSymMatrixArray interp_hessians;
  linear_interpolate_1d( sim_coords, sim_vals, sim_grads, sim_hessians,
			 exp_coords, interp_vals, interp_grads, 
			 interp_hessians ); 

  size_t field_size = interp_vals.length();
  
  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "field pred " << interp_vals << '\n';
    Cout << "interp_resp_offset " << interp_resp_offset << '\n';
    Cout << "field_size " << field_size << '\n';
  }

  copy_field_data(interp_vals, interp_grads, interp_hessians, 
		  interp_resp_offset, field_size, total_asv, 
		  interp_resp);
}

void linear_interpolate_1d( const RealMatrix &build_pts, 
			    const RealVector &build_vals, 
			    const RealMatrix &build_grads, 
			    const RealSymMatrixArray &build_hessians,
			    const RealMatrix &pred_pts, 
			    RealVector &pred_vals,
			    RealMatrix &pred_grads, 
			    RealSymMatrixArray &pred_hessians )
{
  
  int num_pred_pts = pred_pts.numRows();
  int num_build_pts = build_pts.numRows();

  // Following code assumes that vals are always interpolated
  // and if hessians are requested that gradients are provided
  bool interp_grads = ( ( build_grads.numRows() > 0 ) && 
			( build_grads.numCols() > 0 ) );
  bool interp_hessians = ( build_hessians.size() > 0 );

  if ( ( interp_hessians ) && ( !interp_grads) )
    throw( std::runtime_error("Hessians were provided, but gradients were missing") );

  if ( build_pts.numCols()!=1 )
    throw( std::runtime_error("build pts must be Nx1") );
  if ( pred_pts.numCols()!=1 )
    throw( std::runtime_error("build pts must be Mx1") );
  // The following is for when we active multivariate interpolation of field data
  //if ( pred_pts.numCols()!=build_pts.numCols() )
  //  throw( std::runtime_error("build pts and pred pts must have the same number of columns") );

  int num_vars = build_grads.numRows();
  RealVector pred_pts_1d( Teuchos::View, pred_pts.values(), num_pred_pts );
  RealVector build_pts_1d( Teuchos::View, build_pts.values(), num_build_pts );

  // Initialize memory for interpolated data
  if ( pred_vals.length() != num_pred_pts )
    pred_vals.sizeUninitialized( num_pred_pts );
  // Only initialize gradient memory if build gradients was not empty
  if ( ( interp_grads ) && 
       ( ( pred_grads.numRows() != num_vars ) || 
	 ( pred_grads.numCols() != num_pred_pts ) ) )
    pred_grads.shapeUninitialized( num_vars, num_pred_pts );
  // Only initialize hessian memory if build hessians was not empty
  if ( ( interp_hessians ) && ( pred_hessians.size() != num_pred_pts ) )
    pred_hessians.resize( num_pred_pts );

  for ( int i = 0; i < num_pred_pts; i++ ){
    // enforce constant interpolation when interpolation is outside the
    // range of build_pts
    if ( pred_pts_1d[i] <= build_pts_1d[0] ){
      pred_vals[i] = build_vals[0];
      for (int k=0; k<build_grads.numRows();k++)
	pred_grads(k,i) = build_grads(k,0);
      if ( interp_hessians ){
	if ( pred_hessians[i].numRows() != num_vars )
	  pred_hessians[i].shapeUninitialized( num_vars );
	for (int k=0; k<build_grads.numRows();k++)
	  for (int j=0; j<=k; j++)
	    pred_hessians[i](j,k) = build_hessians[0](j,k);
      }
    }else if ( pred_pts_1d[i] >= build_pts_1d[num_build_pts-1] ){
      pred_vals[i] = build_vals[num_build_pts-1];
      for (int k=0; k<build_grads.numRows();k++)
	pred_grads(k,i) = build_grads(k,num_build_pts-1);
      if ( interp_hessians ){
	if ( pred_hessians[i].numRows() != num_vars )
	  pred_hessians[i].shapeUninitialized( num_vars );
	for (int k=0; k<build_grads.numRows();k++)
	  for (int j=0; j<=k; j++)
	    pred_hessians[i](j,k) = build_hessians[num_build_pts-1](j,k);
      }
    }else{
      // assumes binary search returns index of the closest point in 
      // build_pts to the left of pts(0,i)
      int index = binary_search( pred_pts_1d[i], build_pts_1d );

      // interpolate function values
      pred_vals[i] = build_vals[index] +
	( (build_vals[index+1]-build_vals[index] ) /
	  (build_pts_1d[index+1]-build_pts_1d[index]) ) *
	( pred_pts_1d[i] - build_pts_1d[index] );
      // interpolate gradient values
      if ( interp_grads ){
	for (int j=0; j<build_grads.numRows();j++){
	  pred_grads(j,i) = build_grads(j,index) +
	    ( (build_grads(j,index+1)-build_grads(j,index) ) /
	      (build_pts_1d[index+1]-build_pts_1d[index]) ) *
	    ( pred_pts_1d[i] - build_pts_1d[index] );
	}
      }
      // interpolate hessian values
      if ( interp_hessians ){
	if ( pred_hessians[i].numRows() != num_vars )
	  pred_hessians[i].shapeUninitialized( num_vars );
	for (int k=0; k<build_grads.numRows();k++){
	  for (int j=0; j<=k; j++){
	    pred_hessians[i](j,k) = build_hessians[index](j,k) +
	      ( (build_hessians[index+1](j,k)-build_hessians[index](j,k) ) /
		(build_pts_1d[index+1]-build_pts_1d[index]) ) *
	      ( pred_pts_1d[i] - build_pts_1d[index] );
	  }
	}
      }
    }
  }
}

CovarianceMatrix::CovarianceMatrix() : numDOF_(0), covIsDiagonal_(false) {}

CovarianceMatrix::CovarianceMatrix( const CovarianceMatrix &source ){
  copy( source );
}

CovarianceMatrix::~CovarianceMatrix(){}

void CovarianceMatrix::copy( const CovarianceMatrix &source ){
  numDOF_=source.numDOF_;
  covIsDiagonal_ = source.covIsDiagonal_;
  if ( source.covDiagonal_.length() > 0 )
    covDiagonal_=source.covDiagonal_;
  else if ( source.covMatrix_.numRows() > 0 )
  {
    covMatrix_ = source.covMatrix_;
    // Copy covariance matrix cholesky factor from source.
    // WARNING: Using Teuchos::SerialDenseSpdSolver prevents copying of
    // covariance cholesky factor so it must be done again here.
    factor_covariance_matrix(); // The source has already been factored in-place? RWH
  }
  else 
    // No covariance matrix is present in source
    return;
}

CovarianceMatrix& CovarianceMatrix::operator=( const CovarianceMatrix &source ){
  if(this == &source)
    return(*this); // Special case of source same as target
  
  copy( source );
}

void CovarianceMatrix::dense_covariance(RealSymMatrix &cov) const
{
  // reshape could be dangerous as will disconnect any Teuchos::View
  if (cov.numRows() != numDOF_)
    cov.shape(numDOF_);
  cov = 0.0;
  if ( !covIsDiagonal_ ) {
    for (int i=0; i<numDOF_; i++)
      for (int j=0; j<i; j++)
        cov(i,j) = covMatrix_(i,j);
  } else {
    for (int i=0; i<numDOF_; i++) {
      cov(i,i) = covDiagonal_[i];
    }
  }
}

void CovarianceMatrix::set_covariance( const RealMatrix &cov )
{
  if ( cov.numRows() != cov.numCols() ){
    std::string msg = "Covariance matrix must be square.";
    throw( std::runtime_error( msg ) );
  }

  numDOF_ = cov.numRows();
  covMatrix_.shape(numDOF_);
  for (int j=0; j<numDOF_; j++)
    for (int i=j; i<numDOF_; i++){
      covMatrix_(i,j) = cov(i,j);
      covMatrix_(j,i) = cov(i,j);
    }

  covIsDiagonal_ = false;
  factor_covariance_matrix();
}

void CovarianceMatrix::set_covariance( Real cov )
{
  RealVector cov_diag(1,false);
  cov_diag[0] = cov;
  set_covariance( cov_diag );
}

void CovarianceMatrix::set_covariance( const RealVector & cov )
{
  covDiagonal_.sizeUninitialized( cov.length() );
  covDiagonal_.assign( cov );
  covIsDiagonal_ = true;
  numDOF_ = cov.length();
}

Real CovarianceMatrix::apply_covariance_inverse( const RealVector &vector ) const
{
  RealVector result;
  apply_covariance_inverse_sqrt( vector, result );
  return result.dot( result );
}

void CovarianceMatrix::factor_covariance_matrix()
  {
    covCholFactor_ = covMatrix_;
    covSlvr_.setMatrix( rcp(&covCholFactor_, false) );
    // Equilibrate must be false (which is the default) when chol factor is
    // used to compute inverse sqrt
    //covSlvr_.factorWithEquilibration(true); 
    int info = covSlvr_.factor();
    if ( info > 0 ){
      std::string msg = "The covariance matrix is not positive definite\n";
      throw( std::runtime_error( msg ) );
    }
    invert_cholesky_factor();
  };

void CovarianceMatrix::invert_cholesky_factor()
{
  // covCholFactor is a symmetric matrix so must make a copy
  // that only copies the correct lower or upper triangular part
  cholFactorInv_.shape( numDOF_, numDOF_ );
  if ( covCholFactor_.UPLO()=='L' ){
      for (int j=0; j<numDOF_; j++)
	for (int i=j; i<numDOF_; i++)
	  cholFactorInv_(i,j)=covCholFactor_(i,j);
    }else{
      for (int i=0; i<numDOF_; i++)
	for (int j=i; j<numDOF_; j++)
	  cholFactorInv_(i,j)=covCholFactor_(i,j);
    }
    
    int info = 0;
    Teuchos::LAPACK<int, Real> la;
    // always assume non_unit_diag even if covSlvr_.equilibratedA()==true
    la.TRTRI( covCholFactor_.UPLO(), 'N', 
	      numDOF_, cholFactorInv_.values(), 
	      cholFactorInv_.stride(), &info );
    if ( info > 0 ){
      std::string msg = "Inverting the covariance Cholesky factor failed\n";
      throw( std::runtime_error( msg ) );
    }
}

void CovarianceMatrix::apply_covariance_inverse_sqrt( const RealVector &vector,
						      RealVector &result ) const
{
  if ( vector.length() != numDOF_ ){
    std::string msg = "Vector and covariance are incompatible for ";
    msg += "multiplication.";
    throw( std::runtime_error( msg ) );
  }

  if ( result.length() != numDOF_)
    result.sizeUninitialized( numDOF_ );
  if ( covIsDiagonal_ ) {
    for (int i=0; i<numDOF_; i++)
      result[i] = vector[i] / std::sqrt( covDiagonal_[i] ); 
  }else{
    result.multiply( Teuchos::NO_TRANS, Teuchos::NO_TRANS, 
		     1.0, cholFactorInv_, vector, 0.0 );
  }
}

void CovarianceMatrix::apply_covariance_inverse_sqrt_to_gradients( 
          const RealMatrix &gradients,
	  RealMatrix &result ) const
{
  if ( gradients.numCols() != numDOF_ ){
    std::string msg = "Gradients and covariance are incompatible for ";
    msg += "multiplication.";
    throw( std::runtime_error( msg ) );
  }
  int num_grads = gradients.numRows();
  // BMA: relaxed this to allow result to have extra rows that aren't
  // populated (don't want to resize the result gradients); may want
  // to throw error or change API.
  if ( ( result.numRows() < num_grads ) || ( result.numCols() != numDOF_ ) )
    result.shapeUninitialized( num_grads, numDOF_ );
  if ( covIsDiagonal_ ) {
    for (int j=0; j<numDOF_; j++)
      for (int i=0; i<num_grads; i++)
	result(i,j) = gradients(i,j) / std::sqrt( covDiagonal_[j] ); 
  }else{
    // Let A = cholFactorInv_ and B = gradients. We want to compute C' = AB'
    // so compute C = (AB')' = BA'
    result.multiply( Teuchos::NO_TRANS, Teuchos::TRANS, 
		     1.0, gradients, cholFactorInv_, 0.0 );
  }
}

void CovarianceMatrix::apply_covariance_inverse_sqrt_to_hessian( 
			  RealSymMatrixArray &hessians, int start ) const
{
  if ( (hessians.size()-start) < numDOF_ ){
    std::string msg = "Hessians and covariance are incompatible for ";
    msg += "multiplication.";
    throw( std::runtime_error( msg ) );
  }
  int num_rows = hessians[start].numRows();
  if (!num_rows) return; // if Hessian inactive for this fn, no contribution
  if ( covIsDiagonal_ ) {
    for (int k=0; k<numDOF_; k++){
      // Must only loop over lower or upper triangular part
      // because accessor function (i,j) adjusts both upper and lower triangular
      // part
      hessians[start+k] *= 1./std::sqrt( covDiagonal_[k] );
    }
  }else{
    for (int k=0; k<numDOF_; k++) {
      if (!hessians[start+k].numRows()) {
	Cerr << "Error: all Hessians must be defined in CovarianceMatrix::"
	     << "apply_covariance_inverse_sqrt_to_hessian()." << std::endl;
	abort_handler(OTHER_ERROR);
      }
    }
    RealVector hess_ij_res( numDOF_, false );
    RealVector scaled_hess_ij_res( numDOF_, false );
    // Must only loop over lower or upper triangular part
    // because accessor function (i,j) adjusts both upper and lower triangular
    // part
    for (int j=0; j<num_rows; j++){
      for (int i=0; i<=j; i++){
	// Extract the ij hessian components for each degree of freedom 
	for (int k=0; k<numDOF_; k++)
	  hess_ij_res[k] = hessians[start+k](i,j);
	// Apply covariance matrix to the extracted hessian components
	apply_covariance_inverse_sqrt( hess_ij_res, scaled_hess_ij_res );
	// Copy result back into hessians
	for (int k=0; k<numDOF_; k++)
	  hessians[start+k](i,j) = scaled_hess_ij_res[k];
      }
    }
  }
}

int CovarianceMatrix::num_dof() const {
  return numDOF_;
}

void CovarianceMatrix::print() const {
  if ( covIsDiagonal_ ) {
    std::cout << " Covariance is Diagonal " << '\n';
    covDiagonal_.print(std::cout);
  }
  else {  
    std::cout << " Covariance is Full " << '\n';
    covMatrix_.print(std::cout);
  }
}

void CovarianceMatrix::get_main_diagonal( RealVector &diagonal ) const {
  // Only resize matrix if the memory size has not already been allocated
  if ( diagonal.length() != num_dof() )
    diagonal.sizeUninitialized( num_dof() );
  
  if ( covIsDiagonal_ ) {
    for (int i=0; i<num_dof(); i++ )
      diagonal[i] = covDiagonal_[i];
  }else{
    for (int i=0; i<num_dof(); i++ )
      diagonal[i] = covMatrix_(i,i);
  }
}

void CovarianceMatrix::as_correlation(RealSymMatrix& corr_mat) const
{
  corr_mat = 0.0;
  if (covIsDiagonal_) {
    for (int i=0; i<num_dof(); ++i)
      corr_mat(i, i) = 1.0;
  } 
  else {
    for (int i=0; i<num_dof(); ++i) {
      corr_mat(i, i) = 1.0;
      for (int j=0; j<i; ++j)
        corr_mat(i,j) = covMatrix_(i,j) / std::sqrt(covMatrix_(i,i)) / 
          std::sqrt(covMatrix_(j,j));
    }
  }
}


Real CovarianceMatrix::determinant() const
{
  Real det = 1.0;
  if (covIsDiagonal_) {
    for (int i=0; i<num_dof(); i++)
      det *= covDiagonal_[i];
  }
  else {
    for (int i=0; i<num_dof(); i++)
      det *= covCholFactor_(i,i)*covCholFactor_(i,i);
  }
  return det;
}


Real CovarianceMatrix::log_determinant() const
{
  Real log_det = 0.0;
  if (covIsDiagonal_) {
    for (int i=0; i<num_dof(); i++)
      log_det += std::log(covDiagonal_[i]);
  }
  else {
    for (int i=0; i<num_dof(); i++)
      log_det += std::log(covCholFactor_(i,i))+std::log(covCholFactor_(i,i));
  }
  return log_det;
}


ExperimentCovariance & ExperimentCovariance::operator=(const ExperimentCovariance& source)
{
  numBlocks_ = source.numBlocks_;
  numDOF_ = source.numDOF_;
  covMatrices_.resize(source.covMatrices_.size());
  for( size_t i=0; i<source.covMatrices_.size(); ++i)
    covMatrices_[i] = source.covMatrices_[i];

  return *this;
}

void ExperimentCovariance::set_covariance_matrices( 
std::vector<RealMatrix> &matrices, 
std::vector<RealVector> &diagonals,
RealVector &scalars,
IntVector matrix_map_indices,
IntVector diagonal_map_indices, 
IntVector scalar_map_indices ){

  if ( matrices.size() != matrix_map_indices.length() ){
    std::string msg = "must specify a index map for each full ";
    msg += "covariance matrix.";
    throw( std::runtime_error( msg ) );
  }
  if ( diagonals.size() != diagonal_map_indices.length() ){
    std::string msg = "must specify a index map for each diagonal ";
    msg += "covariance matrix.";
    throw( std::runtime_error( msg ) );
  }
  if ( scalars.length() != scalar_map_indices.length() ){
    std::string msg = "must specify a index map for each scalar ";
    msg += "covariance matrix.";
    throw( std::runtime_error( msg ) );
  }

  numDOF_ = 0;

  numBlocks_ = matrix_map_indices.length() + diagonal_map_indices.length() + 
    scalar_map_indices.length();

  covMatrices_.resize( numBlocks_ );

  for (int i=0; i<matrices.size(); i++ ){
    int index = matrix_map_indices[i];
    if ( index >= numBlocks_ )
      throw( std::runtime_error( "matrix_map_indices was out of bounds." ) );
    covMatrices_[index].set_covariance( matrices[i] );
    numDOF_ += matrices[i].numRows();
  }

  for (int i=0; i<diagonals.size(); i++){
    int index = diagonal_map_indices[i];
    if ( index >= numBlocks_ )
      throw( std::runtime_error( "diagonal_map_indices was out of bounds." ) );
    covMatrices_[index].set_covariance( diagonals[i] );
    numDOF_ += diagonals[i].length();
  }

  for (int i=0; i<scalars.length(); i++ ){
    int index = scalar_map_indices[i];
    if ( index >= numBlocks_ )
      throw( std::runtime_error( "scalar_map_indices was out of bounds." ) );
    covMatrices_[index].set_covariance( scalars[i] );
  }
  numDOF_ += scalars.length();
}

Real ExperimentCovariance::apply_experiment_covariance( const RealVector &vector)
  const{
  if ( vector.length() != num_dof() ){
    throw(std::runtime_error("apply_covariance_inverse: vector is inconsistent with covariance matrix"));
  }

  int shift = 0;
  Real result = 0.;
  for (int i=0; i<covMatrices_.size(); i++ ){
    int num_dof = covMatrices_[i].num_dof();
    RealVector sub_vector( Teuchos::View, vector.values()+shift, num_dof );
    result += covMatrices_[i].apply_covariance_inverse( sub_vector );
    shift += num_dof;
  }
  return result;
}

void ExperimentCovariance::apply_experiment_covariance_inverse_sqrt( 
		  const RealVector &vector, RealVector &result ) const{
  if ( vector.length() != num_dof() )
    throw(std::runtime_error("apply_covariance_inverse_sqrt: vector is inconsistent with covariance matrix"));

  int shift = 0;
  result.sizeUninitialized( vector.length() );
  for (int i=0; i<covMatrices_.size(); i++ ){
    int num_dof = covMatrices_[i].num_dof();
    RealVector sub_vector( Teuchos::View, vector.values()+shift, num_dof );
    RealVector sub_result( Teuchos::View, result.values()+shift, num_dof );
    covMatrices_[i].apply_covariance_inverse_sqrt( sub_vector, sub_result );
    shift += num_dof;
  }
}

void ExperimentCovariance::apply_experiment_covariance_inverse_sqrt_to_gradients(
const RealMatrix &gradients, RealMatrix &result ) const{

  if ( gradients.numCols() != num_dof() )
    throw(std::runtime_error("apply_covariance_inverse_sqrt_to_gradients: gradients is inconsistent with covariance matrix"));

  int shift = 0;
  int num_grads = gradients.numRows();
  result.shape( num_grads, gradients.numCols() );
  for (int i=0; i<covMatrices_.size(); i++ ){
    int num_dof = covMatrices_[i].num_dof();
    RealMatrix sub_matrix( Teuchos::View,gradients,num_grads,num_dof,0,shift );
    RealMatrix sub_result( Teuchos::View, result, num_grads, num_dof, 0, shift );
    covMatrices_[i].apply_covariance_inverse_sqrt_to_gradients( sub_matrix, 
								sub_result );
    shift += num_dof;
  }
}

void ExperimentCovariance::apply_experiment_covariance_inverse_sqrt_to_hessians( 
	const RealSymMatrixArray &hessians, RealSymMatrixArray &result ) const {

  if ( hessians.size() != num_dof() )
    throw(std::runtime_error("apply_covariance_inverse_sqrt_to_hessians: hessians is inconsistent with covariance matrix"));

  // perform deep copy of hessians
  result.resize( hessians.size() );
  for (int i=0; i<hessians.size(); i++ )
    if (hessians[i].numRows()) { // else don't bother to assign empty matrix
      result[i].shapeUninitialized( hessians[i].numRows() );
      result[i].assign( hessians[i] );
    }
  int shift = 0;
  for (int i=0; i<covMatrices_.size(); i++ ){
    int num_dof = covMatrices_[i].num_dof();
    // modifify in place the hessian copies stored in result
    covMatrices_[i].apply_covariance_inverse_sqrt_to_hessian( result, shift );
    shift += num_dof;
  }
}

void ExperimentCovariance::print_covariance_blocks() const {
  
  for (int i=0; i<covMatrices_.size(); i++ ){
    std::cout << "Covariance Matrix " << i << '\n';
    covMatrices_[i].print();
  }
}

void ExperimentCovariance::get_main_diagonal( RealVector &diagonal ) const {
  
  // Get the number of entries along the main diagonal
  int num_diagonal_entries = 0;
  for (int i=0; i<covMatrices_.size(); i++ )
    num_diagonal_entries += covMatrices_[i].num_dof();
  diagonal.sizeUninitialized( num_diagonal_entries );
  
  // Extract the diagonal of each sub block of the experimental covariance matrix
  int global_row_num = 0;
  for (int i=0; i<covMatrices_.size(); i++ ){
    RealVector sub_diagonal( Teuchos::View, diagonal.values()+global_row_num,
			     covMatrices_[i].num_dof() );
      covMatrices_[i].get_main_diagonal( sub_diagonal );
    global_row_num += covMatrices_[i].num_dof();
  }
}


void ExperimentCovariance::dense_covariance(RealSymMatrix& cov_mat) const
{
  // reshape could be dangerous as will disconnect any Teuchos::View
  if (cov_mat.numRows() != num_dof())
    cov_mat.shape(num_dof());
  int global_row_num = 0;
  for (int i=0; i<covMatrices_.size(); ++i) {
    RealSymMatrix sub_matrix(Teuchos::View, cov_mat, covMatrices_[i].num_dof(), 
                             global_row_num);
    covMatrices_[i].dense_covariance(sub_matrix);
    global_row_num += covMatrices_[i].num_dof();
  }
}


void ExperimentCovariance::as_correlation(RealSymMatrix& corr_mat) const
{
  // reshape could be dangerous as will disconnect any Teuchos::View
  if (corr_mat.numRows() != num_dof())
    corr_mat.shape(num_dof());
  int global_row_num = 0;
  for (int i=0; i<covMatrices_.size(); ++i) {
    RealSymMatrix sub_matrix(Teuchos::View, corr_mat, covMatrices_[i].num_dof(), 
                             global_row_num);
    covMatrices_[i].as_correlation(sub_matrix);
    global_row_num += covMatrices_[i].num_dof();
  }
}


Real ExperimentCovariance::determinant() const
{
  Real det = 1.0;
  for (int i=0; i<numBlocks_; ++i)
    det *= covMatrices_[i].determinant();
  return det;
}


Real ExperimentCovariance::log_determinant() const
{
  Real log_det = 0.0;
  for (int i=0; i<numBlocks_; ++i)
    log_det += covMatrices_[i].log_determinant();
  return log_det;
}


void symmetric_eigenvalue_decomposition( const RealSymMatrix &matrix, 
					 RealVector &eigenvalues, 
					 RealMatrix &eigenvectors )
{
  Teuchos::LAPACK<int, Real> la;

  int N( matrix.numRows() );
  eigenvectors.shapeUninitialized( N, N );
  //eigenvectors.assign( matrix );
  for ( int j=0; j<N; j++)
    for ( int i=0; i<=j; i++)
      eigenvectors(i,j) = matrix( i,j );

  char jobz = 'V'; // compute eigenvectors
  char uplo = 'U'; // assume only upper triangular part of matrix is stored

  eigenvalues.sizeUninitialized( N );

  int info;        // Teuchos::LAPACK output flag
  RealVector work; // Teuchos::LAPACK work array;
  int lwork = -1;  // Size of Teuchos::LAPACK work array
  
  // Compute optimal size of work array
  work.sizeUninitialized( 1 ); // temporary work array
  la.SYEV( jobz, uplo, N, eigenvectors.values(), eigenvectors.stride(), 
	   eigenvalues.values(), work.values(), lwork, &info );

  lwork = (int)work[0];
  work.sizeUninitialized( lwork );
  
  la.SYEV( jobz, uplo, N, eigenvectors.values(), eigenvectors.stride(), 
	   eigenvalues.values(), work.values(), lwork, &info );

  if ( info > 0 )
    {
      std::stringstream msg;
      msg << "The algorithm failed to converge." << info
	  << " off-diagonal elements of an intermediate tridiagonal "
	  << "form did not converge to zero.";
      throw( std::runtime_error( msg.str() ) );
    }
  else if ( info < 0 )
    {
      std::stringstream msg;
      msg << " The " << std::abs( info ) << " argument had an illegal value.";
      throw( std::runtime_error( msg.str() ) );
    }
};

void compute_column_means( RealMatrix & matrix, RealVector & avg_vals )
{
  int num_cols = matrix.numCols();
  int num_rows = matrix.numRows();

  avg_vals.resize(num_cols);

  RealVector ones_vec(num_rows);
  ones_vec.putScalar(1.0);

  for( int i=0; i<num_cols; ++i ) {
    const RealVector & col_vec = Teuchos::getCol(Teuchos::View, matrix, i);
    avg_vals(i) = col_vec.dot(ones_vec)/(Real) num_rows;
  }
}

bool is_matrix_symmetric( const RealMatrix & matrix )
{
  int num_cols = matrix.numCols();
  int num_rows = matrix.numRows();

  if( num_cols != num_rows )
    return false;

  bool is_symmetric = true;

  for( int i=0; i<num_cols; ++i )
    for( int j=i+1; j<num_cols; ++j ) {
      if( matrix(i,j) != matrix(j,i) ) {
        is_symmetric = false;
        break;
      }
    }

  return is_symmetric;
}

} //namespace Dakota
