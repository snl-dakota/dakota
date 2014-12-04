#ifndef EXPERIMENT_DATA_UTILS 
#define EXPERIMENT_DATA_UTILS 

#include "dakota_data_types.hpp"
#include "Teuchos_SerialSpdDenseSolver.hpp"

namespace Dakota {

/**
 * \brief find the interval containing a target value. 
 * This function assumes the data is in ascending order.
 */
template<typename O, typename T>
int binary_search( T target, Teuchos::SerialDenseVector<O,T> &data )
{
  O low = 0, high = data.length()-1, mid;
  while ( low <= high )
    {
      mid = low + ( high - low ) / 2;
      if ( target < data[mid] ) high = mid - 1;
      else if ( target > data[mid] ) low = mid + 1;
      else return mid;
    }
  if ( high < 0 ) return 0;
  else if ( high < low ) return high;
  else return low;
}

/**
 * \brief Returns the value of at 1D function f at the points of 
 * vector pred_pts using linear interpolation. 
 * The vector build_pts specifies the coordinates of the underlying interval at
 * which the values (build_vals) of the function f are known. The length of 
 * output pred_vals is equal to the length of pred_pts. 
 * This function assumes the build_pts is in ascending order */
void linear_interpolate_1d( RealMatrix &build_pts, RealVector &build_vals, 
			    RealMatrix &pred_pts, RealVector &pred_vals );

/**
 * \brief Given two sets of coordinates and corresponding function values this
 * function returns the difference between the two functions
 * at the coordinates of the second set */
void function_difference_1d( RealMatrix &coords_1, RealVector &values_1,
			     RealMatrix &coords_2, RealVector &values_2,
			     RealVector &diff );

class CovarianceMatrix
{
private:

  /// The number of rows in the covariance matrix
  int numDOF_;

  /// The covariance matrix where each element (i,j) is the covariance 
  /// between points Xi and Xj in the initial set of samples
  RealSymMatrix covMatrix_;

  /// The diagonal entries of a diagonal covariance matrix.
  RealVector covDiagonal_;

  /// The inverse of the covariance matrix
  RealMatrix covMatrixInv_;

  /// Flag specifying if the covariance matrix is diagonal
  bool covIsDiagonal_;

  /// The global solver for all computations involving the inverse of
  /// the covariance matrix
  Teuchos::SerialSpdDenseSolver<int, Real> covSlvr_;

  /// Compute the inverse of the covariance matrix
  void factor_covariance_matrix()
  {
    covSlvr_.setMatrix( rcp(&covMatrix_, false) );
    covSlvr_.factorWithEquilibration(true);
    int info = covSlvr_.factor();
    if ( info > 0 ){
      std::string msg = "The covariance matrix is not positive definite\n";
      throw( std::runtime_error( msg ) );
    }
  };

  /// Copy the values from one existing CovarianceMatrix to another. 
  void copy( const CovarianceMatrix &source );

public:

  /// Default Constructor
  CovarianceMatrix();

  /// Copy Constructor. Needed for creating std::vector<CovarianceMatrix>
  CovarianceMatrix( const CovarianceMatrix &cov_mat );

  /// Deconstructor
  ~CovarianceMatrix();

  /// The operator= copies the values from one existing CovarianceMatrix to 
  /// another. 
  CovarianceMatrix& operator=( const CovarianceMatrix &source );

  /// Return the matrix specifying the correlation between the field data
  void get_covariance( RealMatrix &cov ) const;

  /// Set the matrix specifying the correlation between the field data
  void set_covariance( const RealMatrix &cov );

  /// Set the matrix specifying the correlation between the field data. 
  // A scalar covariance is a degenerate case of a diagonal covariance matrix
  void set_covariance( Real cov );

  /// Set the diagonal of the matrix specifying the correlation between 
  // the field data
  void set_covariance( const RealVector & cov );

  /// Multiply a vector A by the inverse of the covariance matrix
  Real apply_covariance_inverse( RealVector &vector );

  // Return the number of rows in the covariance matrix
  int num_dof() const;
};


class ExperimentCovariance
{
private:
  /// A list of block covariance matrices. ExperimentCovariance consists of
  /// multiple block covariance matrices. These are stored in a list
  /// rather than a dense matrix to save memory
  std::vector<CovarianceMatrix> covMatrices_;

  /// The number of block covariance matrices
  int numBlocks_;
public:

  /// Default Constructor
  ExperimentCovariance() : numBlocks_(0) {};

  /// Deconstructor
  ~ExperimentCovariance(){};

  /// Set the experiment covariance matrix blocks
  void set_covariance_matrices( std::vector<RealMatrix> &matrices, 
				std::vector<RealVector> &diagonals,
				RealVector &scalars,
				IntVector matrix_map_indices,
				IntVector diagonal_map_indices, 
				IntVector scalar_map_indices );

  /// Compute the triple product v'*inv(C)*v
  Real apply_experiment_covariance( RealVector &vector );
};

} // namespace dakota

#endif //EXPERIMENT_DATA_UTILS
