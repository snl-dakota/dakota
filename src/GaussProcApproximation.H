/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 GaussProcApproximation
//- Description: Class implementation of a Gaussian Process Approximation
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#ifndef GAUSS_PROC_APPROXIMATION_H
#define GAUSS_PROC_APPROXIMATION_H

#include "data_types.h"
#include "DakotaApproximation.H"
//#include "SNLLOptimizer.H"
//#include "DakotaNonD.H"

#include "Teuchos_SerialSpdDenseSolver.hpp"

#ifdef HAVE_OPTPP
namespace NEWMAT { 
  class ColumnVector;
  class Matrix;
}
#endif // HAVE_OPTPP


namespace Dakota {

class ProblemDescDB;

/// Derived approximation class for Gaussian Process implementation

/** The GaussProcApproximation class provides a global approximation 
    (surrogate) based on a Gaussian process.  The Gaussian process is built 
    after normalizing the function values, with zero mean.  Opt++ is used 
    to determine the optimal values of the covariance parameters, those 
    which minimize the negative log likelihood function.*/ 

class GaussProcApproximation: public Approximation
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// default constructor
  GaussProcApproximation();
  /// alternate constructor
  GaussProcApproximation(size_t num_vars, short data_order);
  /// standard constructor
  GaussProcApproximation(const ProblemDescDB& problem_db,
			 const size_t& num_acv);
  /// destructor
  ~GaussProcApproximation();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  int min_coefficients() const;

  int num_constraints()  const;

  /// find the covariance parameters governing the Gaussian process response
  void build();

  /// retrieve the function value for a given parameter set x
  const Real& get_value(const RealVector& x);

  /// retrieve the variance of the predicted value for a given parameter set x
  const Real& get_prediction_variance(const RealVector& x);

  /// retrieve the function gradient at the predicted value 
  /// for a given parameter set x
  const RealVector& get_gradient(const RealVector& x);
  
private: 

  //
  //- Heading: Member functions
  //

  /// Function to compute hyperparameters governing the GP.
  void GPmodel_build();

  /// Function returns a response value using the GP surface.
  /** The response value is computed at the design point specified
      by the RealVector function argument.*/
  void GPmodel_apply(const RealVector& new_x, bool variance_flag,
		     bool gradients_flag);

  /// Normalizes the initial inputs upon which the GP surface is based
  void normalize_training_data();
  /// Gets the trend (basis) functions for the calculation of the mean of the GP
  /// If the order = 0, the trend is a constant, if the order = 1, trend is 
  /// linear, if order = 2, trend is quadratic.
  void get_trend();
  /// Gets the beta coefficients for the calculation of the mean of the GP 
  void get_beta_coefficients();
  /// Gets the Cholesky factorization of the covariance matrix, with
  /// error checking
  int get_cholesky_factor();
  /// Gets the estimate of the process variance given the values of beta and 
  /// the correlation lengthscales  
  void get_process_variance();
  /// calculates the covariance matrix for a given set of input points
  void get_cov_matrix();
  /// calculates the covariance vector between a new point x and the 
  /// set of inputs upon which the GP is based
  void get_cov_vector();
  /// sets up and performs the optimization of the negative 
  /// log likelihood to determine the optimal values of the covariance
  /// parameters using NCSUDirect
  void optimize_theta_global();
  /// sets up and performs the optimization of the negative 
  /// log likelihood to determine the optimal values of the covariance
  /// parameters using a gradient-based solver and multiple starting points
  void optimize_theta_multipoint();
  /// Calculates the predicted new response value for x in normalized space
  void predict(bool variance_flag, bool gradients_flag);
  /// calculates the negative log likelihood function (based on covariance 
  /// matrix)
  Real calc_nll();
  /// Gets the gradient of the negative log likelihood function with respect 
  /// to the correlation lengthscales, theta  
  void calc_grad_nll();
  /// Calculate the derivatives of the covariance vector, with respect
  /// to each componeent of x.
  void get_grad_cov_vector();

#ifdef HAVE_OPTPP
  // Using dakota::Real here as we don't expect OPT++ Newmat to be
  // built with float
  /// static function used by OPT++ as the objective function to 
  /// optimize the hyperparameters in the covariance of the GP
  /// by minimizing the negative log likelihood
  static void negloglik(int mode, int n, const NEWMAT::ColumnVector& X,
			Real& fx, NEWMAT::ColumnVector& grad_x,
			int& result_mode);

  /// static function used by OPT++ as the constraint function 
  /// in the optimization of the negative log likelihood. Currently 
  /// this function is empty:  it is an unconstrained optimization. 
  static void constraint_eval(int mode, int n, const NEWMAT::ColumnVector& X,
			      NEWMAT::ColumnVector& g,
			      NEWMAT::Matrix& gradC, int& result_mode);
#endif
  /// function used by NCSUOptimizer to optimize negloglik objective
  static double negloglikNCSU(const RealVector &x);


  /// Runs the point selection algorithm, which will choose a subset
  /// of the training set with which to construct the GP model, and
  /// estimate the necessary parameters
  void run_point_selection();
  /// Initializes the point selection routine by choosing a small
  /// initial subset of the training points
  void initialize_point_selection();
  /// Uses the current GP model to compute predictions at all of the
  /// training points and find the errors
  void pointsel_get_errors(RealArray& delta);
  /// Adds a point to the effective training set.  Returns 1 on success.
  int addpoint(int, IntArray& added_index);
  /// Accepts a vector of unsorted prediction errors, determines which
  /// points should be added to the effective training set, and adds
  /// them
  int pointsel_add_sel(const RealArray& delta);
  /// Return the maximum value of the elements in a vector
  Real maxval(const RealArray&) const;
  /// Writes out the training set before and after point selection
  void pointsel_write_points();
  /// For problems with 2D input, evaluates the negative log
  /// likelihood on a grid
  void lhood_2d_grid_eval();
  /// Writes out the current training set (in original units) to a
  /// specified file
  void writex(const char[]);
  /// Writes out the covariance matrix to a specified file
  void writeCovMat(char[]);

  //
  //- Heading: Data
  //

  /// pointer to the active object instance used within the static evaluator
  static GaussProcApproximation* GPinstance;

  /// A 2-D array (num sample sites = rows, num vars = columns) 
  /// used to create the Gaussian process
  RealMatrix trainPoints;
  /// An array of response values; one response value per sample site
  RealMatrix trainValues;
  /// The mean of the input columns of trainPoints
  RealVector trainMeans;
  /// The standard deviation of the input columns of trainPoints
  RealVector trainStdvs;
  /// Current working set of normalized points upon which the GP is based
  RealMatrix normTrainPoints;
  /// matrix to hold the trend function
  RealMatrix trendFunction;
  /// matrix to hold the beta coefficients for the trend function
  RealMatrix betaCoeffs;
  /// The covariance matrix where each element (i,j) is the covariance 
  /// between points Xi and Xj in the initial set of samples
  RealSymMatrix covMatrix;
  /// The covariance vector where each element (j,0) is the covariance
  /// between a new point X and point Xj from the initial set of samples 
  RealMatrix covVector;
  /// Point at which a prediction is requested.  This is currently a
  /// single point, but it could be generalized to be a vector of points.
  RealMatrix approxPoint;
  /// matrix to hold the gradient of the negative log likelihood 
  /// with respect to the theta correlation terms
  RealMatrix gradNegLogLikTheta;
  /// The global solver for all computations involving the inverse of
  /// the covariance matrix
  Teuchos::SerialSpdDenseSolver<int, Real> covSlvr;
  /// A matrix, where each column is the derivative of the covVector
  /// with respect to a particular componenet of X
  RealMatrix gradCovVector;
  /// Set of all original samples available
  RealMatrix normTrainPointsAll;
  /// All original samples available
  RealMatrix trainValuesAll;
  /// Trend function values corresponding to all original samples
  RealMatrix trendFunctionAll;
  /// Matrix for storing inverse of correlation matrix Rinv*(Y-FB)
  RealMatrix Rinv_YFb;

  /// The number of observations on which the GP surface is built.
  size_t numObs;
  /// The original number of observations
  size_t numObsAll;
  /// The number of variables in each X variable (number of dimensions 
  /// of the problem).
  //size_t numVars;
  /// The order of the basis function for the mean of the GP
  /// If the order = 0, the trend is a constant, if the order = 1, trend is 
  /// linear, if order = 2, trend is quadratic.
  short trendOrder;
  /// Theta is the vector of covariance parameters for the GP. 
  /// We determine the values of theta by optimization
  /// Currently, the covariance function is 
  /// theta[0]*exp(-0.5*sume)+delta*pow(sige,2).  sume is the 
  /// sum squared of weighted distances; it involves a sum of 
  /// theta[1](Xi(1)-Xj(1))^2 + theta[2](Xi(2)-Xj(2))^2 + ...
  /// where Xi(1) is the first dimension value of multi-dimensional 
  /// variable Xi.  delta*pow(sige,2) is a jitter term used to 
  /// improve matrix computations. delta is zero for the covariance 
  /// between different points and 1 for the covariance between the 
  /// same point.  sige is the underlying process error.
  RealVector thetaParams;
  /// The process variance, the multiplier of the correlation matrix
  Real procVar;
  /// Used by the point selection algorithm, this vector keeps track
  /// all points which have been added
  IntArray pointsAddedIndex;
  /// A global indicator for success of the Cholesky factorization
  int cholFlag;
  /// a flag to indicate the use of point selection
  bool usePointSelection;
  //bool afterOptNLL;
};


/** alternate constructor used by EffGlobalOptimization and
    NonDGlobalReliability that does not use a problem database defaults
    here are no point selectinn and quadratic trend function. */
inline GaussProcApproximation::GaussProcApproximation():
  trendOrder(2), usePointSelection(false)
{ }


inline GaussProcApproximation::
GaussProcApproximation(size_t num_vars, short data_order):
  trendOrder(2), usePointSelection(false)
{ numVars = num_vars; dataOrder = data_order; }


inline GaussProcApproximation::~GaussProcApproximation()
{ }

} // namespace Dakota

#endif
