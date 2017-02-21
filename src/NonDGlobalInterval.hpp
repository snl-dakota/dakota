/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDGlobalInterval
//- Description: Class for interval bound estimation for epistemic UQ
//- Owner:	 Laura Swiler
//- Checked by:
//- Version:

#ifndef NOND_GLOBAL_INTERVAL_H
#define NOND_GLOBAL_INTERVAL_H

#include "NonDInterval.hpp"
#include "dakota_data_types.hpp"

namespace Dakota {


/// Class for using global nongradient-based optimization approaches
/// to calculate interval bounds for epistemic uncertainty quantification

/** The NonDGlobalInterval class supports global nongradient-based
    optimization apporaches to determining interval bounds for
    epistemic UQ.  The interval bounds may be on the entire function
    in the case of pure interval analysis (e.g. intervals on input =
    intervals on output), or the intervals may be on statistics of an
    "inner loop" aleatory analysis such as intervals on means,
    variances, or percentile levels.  The preliminary implementation
    will use a Gaussian process surrogate to determine interval bounds. */

class NonDGlobalInterval: public NonDInterval
{
public:

  //
  //- Heading: Constructors and destructor
  //

  NonDGlobalInterval(ProblemDescDB& problem_db, Model& model); ///< constructor
  ~NonDGlobalInterval();                                       ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  void derived_init_communicators(ParLevLIter pl_iter);
  void derived_set_communicators(ParLevLIter pl_iter);
  void derived_free_communicators(ParLevLIter pl_iter);

  /// Performs an optimization to determine interval bounds for an entire
  /// function or interval bounds on a particular statistical estimator
  void core_run();

  const Model& algorithm_space_model() const;

protected:

  //
  //- Heading: New virtual functions
  //

  /// perform any required initialization
  virtual void initialize();
  /// set the optimization variable bounds for each cell
  virtual void set_cell_bounds();
  /// determine truthFnStar and approxFnStar
  virtual void get_best_sample(bool maximize, bool eval_approx);
  /// post-process a cell minimization/maximization result
  virtual void post_process_cell_results(bool maximize);
  /// post-process the interval computed for a response function
  virtual void post_process_response_fn_results();
  /// perform final post-processing
  virtual void post_process_final_results();

  //
  //- Heading: Convenience functions
  //

  /// post-process an optimization execution: output results,
  /// update convergence controls, and update GP approximation
  void post_process_run_results(bool maximize);
  /// evaluate the truth response at the optimal variables solution
  /// and update the GP with the new data
  void evaluate_response_star_truth();

  //
  //- Heading: Data
  //

  /// LHS iterator for constructing initial GP for all response functions
  Iterator daceIterator;
  /// GP model of response, one approximation per response function
  Model fHatModel;

  /// optimizer for solving surrogate-based subproblem: NCSU DIRECT optimizer
  /// for maximizing expected improvement or mixed EA if discrete variables.
  Iterator intervalOptimizer;
  /// recast model which formulates the surrogate-based optimization 
  /// subproblem (recasts as design problem; may assimilate mean and
  /// variance to enable max(expected improvement))
  Model intervalOptModel;

  /// approximate response corresponding to minimum/maximum truth response
  Real approxFnStar;
  /// minimum/maximum truth response function value
  Real truthFnStar;

private:

  //
  //- Heading: Convenience functions
  //

  /// static function used as the objective function in the
  /// Expected Improvement Function (EIF) for minimizing the GP
  static void EIF_objective_min(const Variables& sub_model_vars,
				const Variables& recast_vars,
				const Response& sub_model_response,
				Response& recast_response);
  /// static function used as the objective function in the
  /// Expected Improvement Function (EIF) for maximizing the GP
  static void EIF_objective_max(const Variables& sub_model_vars,
				const Variables& recast_vars,
				const Response& sub_model_response,
				Response& recast_response);

  /// static function used to extract the active objective function
  /// when optimizing for an interval lower or upper bound (non-EIF
  /// formulations).  The sense of the optimization is set separately.
  static void extract_objective(const Variables& sub_model_vars,
				const Variables& recast_vars,
				const Response& sub_model_response,
				Response& recast_response);

  //
  //- Heading: Data
  //

  /// pointer to the active object instance used within the static evaluator
  /// functions in order to avoid the need for static data
  static NonDGlobalInterval* nondGIInstance;

  const int seedSpec;   ///< the user seed specification (default is 0)     
  int       numSamples; ///< the number of samples used in the surrogate
  String    rngName;	///< name of the random number generator

  /// flag indicating use of GP surrogate emulation
  bool gpModelFlag;
  /// flag indicating use of maximized expected improvement for GP
  /// iterate selection
  bool eifFlag;
  /// counter for number of successive iterations that the iteration
  /// improvement is less than the convergenceTol
  unsigned short improvementConvergeCntr;
  /// counter for number of successive iterations that the iteration
  /// improvement is less than the convergenceTol
  unsigned short improvementConvergeLimit;
  /// tolerance for L_2 change in optimal solution
  Real distanceTol;
  /// counter for number of successive iterations that the L_2 change
  /// in optimal solution is less than the convergenceTol
  unsigned short distanceConvergeCntr;
  /// counter for number of successive iterations that the L_2 change
  /// in optimal solution is less than the convergenceTol
  unsigned short distanceConvergeLimit;
  /// stores previous optimal point for continuous variables; used for
  /// assessing convergence
  RealVector prevCVStar;
  /// stores previous optimal point for discrete integer variables;
  /// used for assessing convergence
  IntVector prevDIVStar;
  /// stores previous optimal point for discrete real variables; used
  /// for assessing convergence
  RealVector prevDRVStar;
  /// stores previous solution value for assessing convergence
  Real prevFnStar;
  /// global iteration counter for number of surrogate-based min/max solves
  size_t globalIterCntr;
  /// flag indicating convergence of a minimization or maximization cycle
  bool boundConverged;
  /// flag for maximal response extraction (all response values
  /// obtained on each function call)
  bool allResponsesPerIter;

  /// order of the data used for surrogate construction, in ActiveSet
  /// request vector 3-bit format; user may override responses spec
  short dataOrder;
};


inline const Model& NonDGlobalInterval::algorithm_space_model() const
{ return fHatModel; }

} // namespace Dakota

#endif
