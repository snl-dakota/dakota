/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SOLBase
//- Description: Wrapper class for NPSOL
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: SOLBase.hpp 7004 2010-10-04 17:55:00Z wjbohnh $

#ifndef SOL_BASE_H
#define SOL_BASE_H

#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"

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
  //- Heading: Convenience member functions
  //

  /// Allocates miscellaneous arrays for the SOL algorithms.
  void allocate_arrays(int num_cv, size_t num_nln_con,
		       const RealMatrix& lin_ineq_coeffs,
		       const RealMatrix& lin_eq_coeffs);

  /// Deallocates memory previously allocated by allocate_arrays().
  void deallocate_arrays();

  /// Allocates real and integer workspaces for the SOL algorithms.
  void allocate_workspace(int num_cv, int num_nln_con, int num_lin_con,
			  int num_lsq);

  /// Sets SOL method options using calls to npoptn2.
  void set_options(bool speculative_flag, bool vendor_num_grad_flag, 
		   short output_lev, int verify_lev, Real fn_prec,
		   Real linesrch_tol, int max_iter, Real constr_tol,
		   Real conv_tol, const std::string& grad_type,
		   const RealVector& fdss);

  /// augments variable bounds with linear and nonlinear constraint bounds.
  void augment_bounds(RealVector& augmented_l_bnds, 
		      RealVector& augmented_u_bnds,
		      const RealVector& lin_ineq_l_bnds,
		      const RealVector& lin_ineq_u_bnds,
		      const RealVector& lin_eq_targets,
		      const RealVector& nln_ineq_l_bnds,
		      const RealVector& nln_ineq_u_bnds,
		      const RealVector& nln_eq_targets);

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
  /// length of augmented bounds arrays (variable bounds plus linear and
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


inline SOLBase::SOLBase()  { }

inline SOLBase::~SOLBase() { }

} // namespace Dakota

#endif
