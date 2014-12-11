#include "ExperimentDataUtils.hpp"
#include <iostream>

namespace Dakota {

void linear_interpolate_1d( RealMatrix &build_pts, RealVector &build_vals, 
			    RealMatrix &pred_pts, RealVector &pred_vals )
{
  if ( build_pts.numRows()!=1 )
    throw( std::runtime_error("build pts must be 1xN") );
  if ( pred_pts.numRows()!=1 )
    throw( std::runtime_error("build pts must be 1xM") );
  RealVector pred_pts_1d( Teuchos::View, pred_pts.values(), pred_pts.numCols() );
  RealVector build_pts_1d( Teuchos::View, build_pts.values(), 
			   build_pts.numCols() );
  int num_pred_pts = pred_pts.numCols();
  int num_build_pts = build_pts.numCols();
  if ( pred_vals.length() != num_pred_pts )
    pred_vals.sizeUninitialized( num_pred_pts );
  for ( int i = 0; i < num_pred_pts; i++ )
    {
      // enforce constant interpolation when interpolation is outside the
      // range of build_pts
      if ( pred_pts_1d[i] <= build_pts_1d[0] )
	pred_vals[i] = build_vals[0];
      else if ( pred_pts_1d[i] >= build_pts_1d[num_build_pts-1] )
	pred_vals[i] = build_vals[num_build_pts-1];
      else
	{
	  // assumes binary search returns index of the closest point in 
	  // build_pts to the left of pts(0,i)
	  int index = binary_search( pred_pts_1d[i], build_pts_1d );
	  pred_vals[i] = 
	    build_vals[index] + ( (build_vals[index+1]-build_vals[index] ) /
				  (build_pts_1d[index+1]-build_pts_1d[index]) ) *
	    ( pred_pts_1d[i] - build_pts_1d[index] );
	}
    }
}

void function_difference_1d( RealMatrix &coords_1, RealVector &values_1,
			     RealMatrix &coords_2, RealVector &values_2,
			     RealVector &diff )
{
  linear_interpolate_1d( coords_1, values_1, coords_2, diff );
  diff -= values_2;
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
    covMatrix_ = source.covMatrix_;
  else 
    // No covariance matrix is present in source
    return;
  
  // Copy covariance matrix cholesky factor from source.
  // WARNING: Using Teuchos::SerialDenseSpdSolver prevents copying of
  // covariance cholesky factor so it must be done again here.
  factor_covariance_matrix();
}

CovarianceMatrix& CovarianceMatrix::operator=( const CovarianceMatrix &source ){
  if(this == &source)
    return(*this); // Special case of source same as target
  
  copy( source );
}

void CovarianceMatrix::get_covariance( RealMatrix &cov ) const
{
  cov.shape( numDOF_, numDOF_ );
  if ( !covIsDiagonal_ ){
    for (int j=0; j<numDOF_; j++)
      for (int i=j; i<numDOF_; i++){
	cov(i,j) = covMatrix_(i,j);
	cov(j,i) = covMatrix_(i,j);
      }
  }else{
    for (int j=0; j<numDOF_; j++)
      cov(j,j) = covMatrix_(j,j);
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

Real CovarianceMatrix::apply_covariance_inverse( RealVector &vector )
{
  if ( vector.length() != numDOF_ ){
    std::string msg = "Vector and covariance are incompatible for ";
    msg += "multiplication.";
    throw( std::runtime_error( msg ) );
  }

  RealVector cov_inv_vector( numDOF_, false );
  if ( covIsDiagonal_ ) {
    cov_inv_vector.assign( vector );
    for (int i=0; i<numDOF_; i++)
      cov_inv_vector[i] /= covDiagonal_[i];
  }else{
    // must copy vector as covSlvr_.solve() changes both arguments to set_vectors
    RealVector vector_copy( vector );
    covSlvr_.setVectors( rcp(&cov_inv_vector, false), rcp(&vector_copy, false) );
    covSlvr_.solve();
  }
  return vector.dot( cov_inv_vector );
}

int CovarianceMatrix::num_dof() const{
  return numDOF_;
}

void CovarianceMatrix::print() {
  if ( covIsDiagonal_ ) {
    std::cout << " Covariance is Diagonal " << '\n';
    covDiagonal_.print(std::cout);
  }
  else {  
    std::cout << " Covariance is Full " << '\n';
    covMatrix_.print(std::cout);
  }
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
    std::string msg = "must specify a index map for each full ";
    msg += "covariance matrix.";
    throw( std::runtime_error( msg ) );
  }
  if ( scalars.length() != scalar_map_indices.length() ){
    std::string msg = "must specify a index map for each full ";
    msg += "covariance matrix.";
    throw( std::runtime_error( msg ) );
  }

  numBlocks_ = matrix_map_indices.length() + diagonal_map_indices.length() + 
    scalar_map_indices.length();

  covMatrices_.resize( numBlocks_ );

  for (int i=0; i<matrices.size(); i++ ){
    int index = matrix_map_indices[i];
    if ( index >= numBlocks_ )
      throw( std::runtime_error( "matrix_map_indices was out of bounds." ) );
    covMatrices_[index].set_covariance( matrices[i] );
  }

  for (int i=0; i<diagonals.size(); i++){
    int index = diagonal_map_indices[i];
    if ( index >= numBlocks_ )
      throw( std::runtime_error( "matrix_map_indices was out of bounds." ) );
    covMatrices_[index].set_covariance( diagonals[i] );
  }

  for (int i=0; i<scalars.length(); i++ ){
    int index = scalar_map_indices[i];
    if ( index >= numBlocks_ )
      throw( std::runtime_error( "matrix_map_indices was out of bounds." ) );
    covMatrices_[index].set_covariance( scalars[i] );
    }
  }

Real ExperimentCovariance::apply_experiment_covariance( RealVector &vector ){
  int shift = 0;
  Real result = 0.;
  for (int i=0; i<covMatrices_.size(); i++ ){
    int num_dof = covMatrices_[i].num_dof();
    RealVector sub_vector( Teuchos::View, vector.values()+shift, num_dof );
    result += covMatrices_[i].apply_covariance_inverse( sub_vector );
    shift += num_dof;
  }
}

void ExperimentCovariance::print_cov() {
  
  for (int i=0; i<covMatrices_.size(); i++ ){
    std::cout << "Covariance Matrix " << i << '\n';
    covMatrices_[i].print();
  }
}

} //namespace Dakota
