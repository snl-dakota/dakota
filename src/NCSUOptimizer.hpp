/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NCSU_OPTIMIZER_H
#define NCSU_OPTIMIZER_H

#include "DakotaOptimizer.hpp"


namespace Dakota {

/**
 * \brief A version of TraitsBase specialized for NCSU optimizers
 *
 */
class NCSUTraits: public TraitsBase
{
  public:

  /// default constructor
  NCSUTraits() { }

  /// destructor
  virtual ~NCSUTraits() { }

  /// A temporary query used in the refactor
  virtual bool is_derived() { return true; }

  /// Return the flag indicating whether method supports continuous variables
  bool supports_continuous_variables() { return true; }
};


/// Wrapper class for the NCSU DIRECT optimization library.

/** The NCSUOptimizer class provides a wrapper for a Fortran 77
    implementation of the DIRECT algorithm developed at North Carolina
    State University. It uses a function pointer approach for which passed 
    functions must be either global functions or static member functions.  
    Any attribute used within static member functions must be either local 
    to that function or accessed through a static pointer. */

class NCSUOptimizer: public Optimizer
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NCSUOptimizer(ProblemDescDB& problem_db, Model& model);

  /// alternate constructor for instantiations "on the fly"
  NCSUOptimizer(Model& model, size_t max_iter, size_t max_eval,
		double min_box_size = -1., double vol_box_size = -1.,
		double solution_target = -DBL_MAX);

  /// alternate constructor for Iterator instantiations by name
  NCSUOptimizer(Model& model);

  /// alternate constructor for instantiations "on the fly"
  NCSUOptimizer(//const RealVector& initial_pt,
		const RealVector& var_l_bnds, const RealVector& var_u_bnds,
		const RealMatrix& lin_ineq_coeffs,
		const RealVector& lin_ineq_l_bnds,
		const RealVector& lin_ineq_u_bnds,
		const RealMatrix& lin_eq_coeffs, const RealVector& lin_eq_tgts,
		const RealVector& nln_ineq_l_bnds,
		const RealVector& nln_ineq_u_bnds,
		const RealVector& nln_eq_tgts, size_t max_iter, size_t max_eval,
  	        double (*user_obj_eval) (const RealVector &x),
		double min_box_size = -1., double vol_box_size = -1.,
		double solution_target = -DBL_MAX);

  ~NCSUOptimizer(); ///< destructor
    
  //
  //- Heading: Virtual function redefinitions
  //

  //void initialize_run();
  void core_run();

  void declare_sources();

  void check_sub_iterator_conflict();

  //void initial_point(const RealVector& pt);
  void update_callback_data(const RealVector& cv_initial,
			    const RealVector& cv_lower_bnds,
			    const RealVector& cv_upper_bnds,
			    const RealMatrix& lin_ineq_coeffs,
			    const RealVector& lin_ineq_l_bnds,
			    const RealVector& lin_ineq_u_bnds,
			    const RealMatrix& lin_eq_coeffs,
			    const RealVector& lin_eq_targets,
			    const RealVector& nln_ineq_l_bnds,
			    const RealVector& nln_ineq_u_bnds,
			    const RealVector& nln_eq_targets);
  const RealMatrix& callback_linear_ineq_coefficients() const;
  const RealVector& callback_linear_ineq_lower_bounds() const;
  const RealVector& callback_linear_ineq_upper_bounds() const;

private:

  //
  //- Heading: Static member function passed by pointer to NCSUDirect
  //

  /// 'fep' in Griffin-modified NCSUDirect: computes the value of the
  /// objective function (potentially at multiple points, passed by function
  /// pointer to NCSUDirect).  Include unscaling from DIRECT.
  static int objective_eval(int *n, double c[], double l[], double u[],
		            int point[], int *maxI, int *start, int *maxfunc,
			    double fvec[], int iidata[], int *iisize,
			    double ddata[], int *idsize, char cdata[],
			    int *icsize);

  //
  //- Heading: Convenience member functions
  //

  void initialize(); ///< shared code among model-based constructors

  void check_inputs(); ///< verify problem respects NCSU DIRECT Fortran limits

  //
  //- Heading: Data
  //

  /// pointer to the active object instance used within the static evaluator
  /// functions in order to avoid the need for static data
  static NCSUOptimizer* ncsudirectInstance;

  /// controls iteration mode: SETUP_MODEL (normal usage) or SETUP_USERFUNC
  /// (user-supplied functions mode for "on the fly" instantiations).
  /// see enum in NCSUOptimizer.cpp
  /// NonDGlobalReliability currently uses the model mode.
  /// GaussProcApproximation currently uses the user_functions mode.
  short setUpType;
  /// holds the minimum boxsize
  Real minBoxSize;
  /// hold the minimum volume boxsize
  Real volBoxSize;
  /// holds the solution target minimum to drive towards
  Real solutionTarget; 
  /// holds function pointer for objective function evaluator passed in for
  /// "user_functions" mode.
  double (*userObjectiveEval) (const RealVector &x);

  // initial point used in "user_functions" mode (no starting point for DIRECT)
  //RealVector initialPoint;
  /// variable lower bounds used in "user_functions" mode
  RealVector lowerBounds;
  /// variable upper bounds used in "user_functions" mode
  RealVector upperBounds;
  /// linear inequality constraint coefficients used in "user_functions" mode
  RealMatrix linIneqCoeffs;
  /// linear inequality constraint lower bounds used in "user_functions" mode
  RealVector linIneqLowerBnds;
  /// linear inequality constraint upper bounds used in "user_functions" mode
  RealVector linIneqUpperBnds;
  /// linear equality constraint coefficients used in "user_functions" mode
  RealMatrix linEqCoeffs;
  /// linear equality constraint targets used in "user_functions" mode
  RealVector linEqTargets;
  /// nonlinear inequality constraint lower bounds used in "user_functions" mode
  RealVector nlnIneqLowerBnds;
  /// nonlinear inequality constraint upper bounds used in "user_functions" mode
  RealVector nlnIneqUpperBnds;
  /// nonlinear equality constraint targets used in "user_functions" mode
  RealVector nlnEqTargets;
};


inline const RealMatrix& NCSUOptimizer::
callback_linear_ineq_coefficients() const
{ return linIneqCoeffs; }


inline const RealVector& NCSUOptimizer::
callback_linear_ineq_lower_bounds() const
{ return linIneqLowerBnds; }


inline const RealVector& NCSUOptimizer::
callback_linear_ineq_upper_bounds() const
{ return linIneqUpperBnds; }

} // namespace Dakota

#endif
