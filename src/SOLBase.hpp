/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef SOL_BASE_H
#define SOL_BASE_H

#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
#include "DakotaModel.hpp"

namespace Dakota {

class Minimizer; // fwd declarations
class Model;


/// Base class for Stanford SOL software.

/** The SOLBase class provides a common base class for NPSOLOptimizer
    and NLSSOLLeastSq, both of which are Fortran 77 sequential
    quadratic programming algorithms from Stanford University marketed
    by Stanford Business Associates. */

class SOLBase
{
public:
  
  //
  //- Heading: Constructors and destructor
  //

  SOLBase();             ///< default constructor
  SOLBase(Model& model); ///< standard constructor
  ~SOLBase();            ///< destructor

protected:

  //
  //- Heading: New virtual functions
  //

  /// Resize and send option to NPSOL (npoptn) or NLSSOL (nloptn) via
  /// derived implementation.
  virtual void send_sol_option(std::string sol_option) = 0;

  //
  //- Heading: Convenience member functions
  //

  /// check for clash with nested use of Fortran code
  void check_sub_iterator_conflict(Model& model, unsigned short method_name);

  /// Allocates F77 linear constraint arrays for the SOL algorithms
  void allocate_linear_arrays(int num_cv, const RealMatrix& lin_ineq_coeffs,
			      const RealMatrix& lin_eq_coeffs);
  /// Allocates F77 nonlinear constraint arrays for the SOL algorithms.
  void allocate_nonlinear_arrays(int num_cv, size_t num_nln_con);
  /// Allocates F77 arrays for the SOL algorithms.
  void allocate_arrays(int num_cv, size_t num_nln_con,
		       const RealMatrix& lin_ineq_coeffs,
		       const RealMatrix& lin_eq_coeffs);
  /// Deallocates memory previously allocated by allocate_arrays().
  void deallocate_arrays();

  /// Updates arrays dependent on combined bounds size
  void size_bounds_array(size_t new_bnds_size);

  /// Allocates real and integer workspaces for the SOL algorithms.
  void allocate_workspace(int num_cv, int num_nln_con, int num_lin_con,
			  int num_lsq);

  /// Sets SOL method options using calls to npoptn2 / nloptn2.
  void set_options(bool speculative_flag, bool vendor_num_grad_flag, 
		   short output_lev, int verify_lev, Real fn_prec,
		   Real linesrch_tol, size_t max_iter, Real constr_tol,
		   Real conv_tol, const std::string& grad_type,
		   const RealVector& fdss);

  /// aggregate variable bounds with linear and nonlinear constraint bounds
  void aggregate_bounds(const RealVector& cv_lower_bnds,
			const RealVector& cv_upper_bnds,
			const RealVector& lin_ineq_l_bnds,
			const RealVector& lin_ineq_u_bnds,
			const RealVector& lin_eq_targets,
			const RealVector& nln_ineq_l_bnds,
			const RealVector& nln_ineq_u_bnds,
			const RealVector& nln_eq_targets,
			RealVector& aggregate_l_bnds,
			RealVector& aggregate_u_bnds);

  /*
  /// augments variable bounds with linear and nonlinear constraint bounds.
  void augment_bounds(RealVector& aggregate_l_bnds,
		      RealVector& aggregate_u_bnds,
		      const Model& model);
  /// augments variable bounds with linear and nonlinear constraint bounds.
  void augment_bounds(RealVector& aggregate_l_bnds,
		      RealVector& aggregate_u_bnds,
		      const RealVector& lin_ineq_l_bnds,
		      const RealVector& lin_ineq_u_bnds,
		      const RealVector& lin_eq_targets,
		      const RealVector& nln_ineq_l_bnds,
		      const RealVector& nln_ineq_u_bnds,
		      const RealVector& nln_eq_targets);

  /// update linear constraint arrays
  void replace_linear_arrays(size_t num_cv, size_t num_nln_con,
			     const RealMatrix& lin_ineq_coeffs,
			     const RealMatrix& lin_eq_coeffs);
  /// update nonlinear constraint arrays
  void replace_nonlinear_arrays(int num_cv, size_t num_lin_con,
				size_t num_nln_con);

  /// replace variable bounds within aggregate arrays
  void replace_variable_bounds(size_t num_lin_con, size_t num_nln_con,
			       RealVector& aggregate_l_bnds,
			       RealVector& aggregate_u_bnds,
			       const RealVector& cv_lower_bnds,
			       const RealVector& cv_upper_bnds);
  /// replace linear bounds within aggregate arrays
  void replace_linear_bounds(size_t num_cv, size_t num_nln_con,
			     RealVector& aggregate_l_bnds,
			     RealVector& aggregate_u_bnds,
			     const RealVector& lin_ineq_l_bnds,
			     const RealVector& lin_ineq_u_bnds,
			     const RealVector& lin_eq_targets);
  /// replace nonlinear bounds within aggregate arrays
  void replace_nonlinear_bounds(size_t num_cv, size_t num_lin_con,
				RealVector& aggregate_l_bnds,
				RealVector& aggregate_u_bnds,
				const RealVector& nln_ineq_l_bnds,
				const RealVector& nln_ineq_u_bnds,
				const RealVector& nln_eq_targets);
  */

  //
  //- Heading: Static member functions passed by pointer to NPSOL/NLSSOL
  //

  /// CONFUN in NPSOL manual: computes the values and first
  /// derivatives of the nonlinear constraint functions
  static void constraint_eval(int& mode, int& ncnln, int& n, int& nrowj,
			      int* needc, double* x, double* c, double* cjac,
			      int& nstate);

  //
  //- Heading: Data
  //

  /// pointer to the active object instance used within the static evaluator
  /// functions in order to avoid the need for static data
  static SOLBase* solInstance;
  /// pointer to the active base class object instance used within the static
  /// evaluator functions in order to avoid the need for static data
  static Minimizer* optLSqInstance;

  int       realWorkSpaceSize; ///< size of realWorkSpace
  int       intWorkSpaceSize;  ///< size of intWorkSpace
  RealArray realWorkSpace;     ///< real work space for NPSOL/NLSSOL
  IntArray  intWorkSpace;      ///< int work space for NPSOL/NLSSOL

  /// used for non-zero array sizing (nonlinear constraints)
  int nlnConstraintArraySize;
  /// used for non-zero array sizing (linear constraints)
  int linConstraintArraySize;

  /// CLAMBDA from NPSOL manual: Langrange multipliers
  RealArray cLambda;
  /// ISTATE from NPSOL manual: constraint status
  IntArray  constraintState;

  /// INFORM from NPSOL manual: optimization status on exit
  int informResult;
  /// ITER from NPSOL manual: number of (major) iterations performed
  int numberIterations;
  /// length of aggregated bounds arrays (variable bounds plus linear and
  /// nonlinear constraint bounds)
  int boundsArraySize;

  // These 3 matrices must have contiguous memory for passing to Fortran.  
  // Therefore, the RealMatrix type cannot be used.
  /// [A] matrix from NPSOL manual: linear constraint coefficients
  double *linConstraintMatrixF77;
  /// [R] matrix from NPSOL manual: upper Cholesky factor of the Hessian of
  /// the Lagrangian.
  double *upperFactorHessianF77;
  /// [CJAC] matrix from NPSOL manual: nonlinear constraint Jacobian
  double *constraintJacMatrixF77;

  int fnEvalCntr; ///< counter for testing against maxFunctionEvals

  /// used in constraint_eval() to bridge NLSSOLLeastSq::numLeastSqTerms
  /// and NPSOLOptimizer::numObjectiveFns
  size_t constrOffset;
};


inline SOLBase::SOLBase():
  boundsArraySize(0), linConstraintMatrixF77(NULL),
  upperFactorHessianF77(NULL), constraintJacMatrixF77(NULL)
{ }


inline SOLBase::~SOLBase() { }


inline void SOLBase::size_bounds_array(size_t new_bnds_size)
{
  if (boundsArraySize != new_bnds_size) {
    boundsArraySize = new_bnds_size;
    cLambda.resize(boundsArraySize);          // clambda[bnd_size]
    constraintState.resize(boundsArraySize);  // istate[bnd_size]
  }
}


/*
inline void SOLBase::
replace_linear_arrays(size_t num_cv, size_t num_nln_con,
		      const RealMatrix& lin_ineq_coeffs,
		      const RealMatrix& lin_eq_coeffs)
{
  allocate_linear_arrays(num_cv, lin_ineq_coeffs, lin_eq_coeffs);
  size_bounds_array(num_cv + lin_ineq_coeffs.numRows() +
		    lin_eq_coeffs.numRows() + num_nln_con);
}


inline void SOLBase::
replace_nonlinear_arrays(int num_cv, size_t num_lin_con, size_t num_nln_con)
{
  allocate_nonlinear_arrays(num_cv, num_nln_con);
  size_bounds_array(num_cv + num_lin_con + num_nln_con);
}


inline void SOLBase::
augment_bounds(RealVector& aggregate_l_bnds, RealVector& aggregate_u_bnds,
	       const Model& model)
{
  augment_bounds(aggregate_l_bnds, aggregate_u_bnds,
		 model.linear_ineq_constraint_lower_bounds(),
		 model.linear_ineq_constraint_upper_bounds(),
		 model.linear_eq_constraint_targets(),
		 model.nonlinear_ineq_constraint_lower_bounds(),
		 model.nonlinear_ineq_constraint_upper_bounds(),
		 model.nonlinear_eq_constraint_targets());
}
*/

} // namespace Dakota

#endif
