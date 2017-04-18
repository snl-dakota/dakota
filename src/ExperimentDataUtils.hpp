#ifndef EXPERIMENT_DATA_UTILS 
#define EXPERIMENT_DATA_UTILS 

#include "dakota_data_types.hpp"
#include "Teuchos_SerialSpdDenseSolver.hpp"

namespace Dakota {

class Response;


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

void copy_field_data(const RealVector& fn_vals, RealMatrix& fn_grad, 
		     const RealSymMatrixArray& fn_hess, size_t offset, 
		     size_t num_fns, Response& response);

void interpolate_simulation_field_data( const Response &sim_resp, 
					const RealMatrix &exp_coords,
					size_t field_num, short total_asv,
					size_t interp_resp_offset,
					Response &interp_resp );

/**
 * \brief Returns the value of at 1D function f and its gradient and hessians
 * (if available) at the points of  vector pred_pts using linear interpolation. 
 * The vector build_pts specifies the coordinates of the underlying interval at
 * which the values (build_vals) of the function f are known. The length of 
 * output pred_vals is equal to the length of pred_pts. 
 * This function assumes the build_pts is in ascending order */
void linear_interpolate_1d( const RealMatrix &build_pts, 
			    const RealVector &build_vals, 
			    const RealMatrix &build_grads, 
			    const RealSymMatrixArray &build_hessians,
			    const RealMatrix &pred_pts, 
			    RealVector &pred_vals,
			    RealMatrix &pred_grads, 
			    RealSymMatrixArray &pred_hessians );

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
  RealSymMatrix covCholFactor_;

  /// The inverse of the Cholesky factor of the covariance matrix
  RealMatrix cholFactorInv_;

  /// Flag specifying if the covariance matrix is diagonal
  bool covIsDiagonal_;

  /// The global solver for all computations involving the inverse of
  /// the covariance matrix
  Teuchos::SerialSpdDenseSolver<int, Real> covSlvr_;

  /// Compute the Cholesky factorization of the covariance matrix
  void factor_covariance_matrix();

  /// Compute the inverse of the Cholesky factor of the covariance matrix
  void invert_cholesky_factor();

  /// Copy the values from one existing CovarianceMatrix to another. 
  void copy( const CovarianceMatrix &source );

public:

  /// Specification for covariance type
  enum FORMAT   {  CONSTANT,
                   VECTOR,
                   MATRIX };

  /// Default Constructor
  CovarianceMatrix();

  /// Copy Constructor. Needed for creating std::vector<CovarianceMatrix>
  CovarianceMatrix( const CovarianceMatrix &cov_mat );

  /// Deconstructor
  ~CovarianceMatrix();

  /// The operator= copies the values from one existing CovarianceMatrix to 
  /// another. 
  CovarianceMatrix& operator=( const CovarianceMatrix &source );

  /// Return the full dense matrix specifying the covariance among the
  /// field data
  void dense_covariance(RealSymMatrix &cov) const;

  /// Retrieve the covariance as a correlation matrix
  void as_correlation(RealSymMatrix& corr_mat) const;

  /// Set the matrix specifying the correlation between the field data
  void set_covariance( const RealMatrix &cov );

  /// Set the matrix specifying the correlation between the field data. 
  // A scalar covariance is a degenerate case of a diagonal covariance matrix
  void set_covariance( Real cov );

  /// Set the diagonal of the matrix specifying the correlation between 
  // the field data
  void set_covariance( const RealVector & cov );

  /// Compute the triple product of r'*inv(C)*r where r is a vector r and C
  /// is the covariance matrix
  Real apply_covariance_inverse( const RealVector &vector ) const;

  /// Multiply a vector r by the sqrt of the inverse covariance matrix C, i.e.
  /// compute L'*r where L is the cholesky factor of the positive definite 
  /// covariance matrix
  void apply_covariance_inverse_sqrt( const RealVector &vector, 
				      RealVector &result ) const;

  /// Multiply a matrix of gradients g (each column is a gradient vector) 
  /// by the sqrt of the inverse covariance matrix C, i.e.
  /// compute L'*g where L is the cholesky factor of the positive definite 
  /// covariance matrix
  void apply_covariance_inverse_sqrt_to_gradients( const RealMatrix &gradients, 
						   RealMatrix &result ) const;

  /// Apply the sqrt of the inverse covariance matrix to a list of Hessians.
  /// the argument hessians is a numDOF_ list of num_grads x num_grads Hessian
  /// matrices. \param start is an index pointing to the first Hessian to
  /// consider in the list. This helps avoid copying large Hessian matrix
  /// when applying block covariances using ExperimentCovariance class
  void apply_covariance_inverse_sqrt_to_hessian( RealSymMatrixArray &hessians,
						 int start) const;

  /// Return the number of rows in the covariance matrix
  int num_dof() const;

  /// Print a covariance matrix
  void print() const;

  /// Return a (copy) vector containing the entries of the main diagonal of the
  /// covariance matrix
  void get_main_diagonal( RealVector &diagonal ) const;

  /// The determinant of the covariance matrix
  Real determinant() const;

  /// The log(determinant) of the covariance matrix
  Real log_determinant() const;

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

  /// The number of entries along the diagonal;
  int numDOF_;

public:

  /// Default Constructor
  ExperimentCovariance() : numBlocks_(0), numDOF_(0) {};

  /// Assignment operator
  ExperimentCovariance & operator=(const ExperimentCovariance& source);

  /// Deconstructor
  ~ExperimentCovariance(){numBlocks_=0;numDOF_=0;};

  /// Set the experiment covariance matrix blocks
  void set_covariance_matrices( std::vector<RealMatrix> &matrices, 
				std::vector<RealVector> &diagonals,
				RealVector &scalars,
				IntVector matrix_map_indices,
				IntVector diagonal_map_indices, 
				IntVector scalar_map_indices );

  /// Compute the triple product v'*inv(C)*v
  Real apply_experiment_covariance( const RealVector &vector ) const;

  /// Compute the product inv(L)*v where L is the Cholesky factor of the 
  /// covariance matrix C
  void apply_experiment_covariance_inverse_sqrt( const RealVector &vector,
						 RealVector &result ) const;

  /// Compute the product inv(L)*G where L is the Cholesky factor of the 
  /// covariance matrix C and G is a matrix whose columns are gradient vectors
  /// for each degree of freedom
  void apply_experiment_covariance_inverse_sqrt_to_gradients(
           const RealMatrix &grads,
	   RealMatrix &result) const;

  /// Compute the products inv(L)*H where L is the Cholesky factor of the 
  /// covariance matrix C and H is a Hessian matrix. The product is computed
  /// for each Hessian of every degree of freedom.
  void apply_experiment_covariance_inverse_sqrt_to_hessians( 
	   const RealSymMatrixArray &hesians, RealSymMatrixArray &result ) const;
  
  /// Print each block of the covariance matrix
  void print_covariance_blocks() const;

  /// Return a (copy) vector containing the main diagonal entries of the 
  /// experimental covariance matrix
  void get_main_diagonal( RealVector &diagonal ) const;

  /// Retrieve a full/dense representation of the covariance
  void dense_covariance(RealSymMatrix& cov_mat) const;

  /// Retrieve the covariance as a (dense) correlation matrix
  void as_correlation(RealSymMatrix& corr_mat) const;

  /// Determinant of the total ExperimentCovariance structure
  Real determinant() const;

  /// Log of determinant of the total ExperimentCovariance structure
  Real log_determinant() const;

  /// Return the number of diagonal blocks in the entire matrix
  int num_blocks() const{
    return numBlocks_;
  };

  int num_dof() const{
    return numDOF_;
  }

};

/**
 * \brief Computes the eigenvalues and, optionally, eigenvectors of a
 *  real symmetric matrix A. 
 * 
 * Eigenvalues are returned in ascending order.
 */
void symmetric_eigenvalue_decomposition( const RealSymMatrix &matrix, 
					 RealVector &eigenvalues, 
					 RealMatrix &eigenvectors );

/**
 * \brief Compute the means of each column of an arbitrary matrix
 */
void compute_column_means( RealMatrix & matrix, RealVector & avg_vals );

/**
 * \brief Test if incoming matrix is symmetric
 */
bool is_matrix_symmetric( const RealMatrix & matrix );

} // namespace dakota

#endif //EXPERIMENT_DATA_UTILS
