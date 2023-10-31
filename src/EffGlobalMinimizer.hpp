/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Edited by:   Anh Tran on 12/21/2019

#ifndef EGO_MINIMIZER_H
#define EGO_MINIMIZER_H

#include "SurrBasedMinimizer.hpp"


namespace Dakota {


/// Implementation of Efficient Global Optimization/Least Squares algorithms


/**
 * \brief A version of TraitsBase specialized for efficient global minimizer
 *
 */

class EffGlobalTraits: public TraitsBase
{
  public:

  /// default constructor
  EffGlobalTraits() { }

  /// destructor
  virtual ~EffGlobalTraits() { }

  /// A temporary query used in the refactor
  virtual bool is_derived() { return true; }

  /// Return the flag indicating whether method supports continuous variables
  bool supports_continuous_variables() { return true; }

  /// Return the flag indicating whether method supports nonlinear equalities
  bool supports_nonlinear_equality() { return true; }

  /// Return the flag indicating whether method supports nonlinear inequalities
  bool supports_nonlinear_inequality() { return true; }
};


/** The EffGlobalMinimizer class provides an implementation of the
    Efficient Global Optimization algorithm developed by Jones, Schonlau,
    & Welch as well as adaptation of the concept to nonlinear least squares. */

class EffGlobalMinimizer: public SurrBasedMinimizer
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  EffGlobalMinimizer(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor for instantiations "on the fly"
  EffGlobalMinimizer(Model& model, const String& approx_type, int samples,
		     int seed, bool use_derivs, size_t max_iter,
		     size_t max_eval, Real conv_tol);
  /// destructor
  ~EffGlobalMinimizer();

  //
  //- Heading: Virtual function redefinitions
  //

  //void derived_init_communicators(ParLevLIter pl_iter);
  //void derived_set_communicators(ParLevLIter pl_iter);
  //void derived_free_communicators(ParLevLIter pl_iter);

  void pre_run();
  void core_run();
  void post_run(std::ostream& s);

  const Model& algorithm_space_model() const;

  //void declare_sources();

private:

  //
  //- Heading: Convenience member functions
  //

  /// shared ctor code for initializing Models and Minimizers for solving an
  /// approximate sub-problem 
  void initialize_sub_problem(const String& approx_type, int samples, int seed,
    bool use_derivs, const String& sample_reuse,
    const String& import_build_points_file = String(),
    unsigned short import_build_format = TABULAR_ANNOTATED,
    bool import_build_active_only = false,
    const String& export_approx_points_file = String(),
    unsigned short export_approx_format = TABULAR_ANNOTATED);
  /// initialize Lagrange multipliers in the case of nonlinear constraints
  void initialize_multipliers();

  /// function that checks if model supports asynchronous parallelism
  void check_parallelism();

  /// build initial GP approximations after initial sampling
  void build_gp();

  // sequential EGO implementation: main function
  //void serial_ego();
  /// synchronous batch-sequential implementation: main function
  void batch_synchronous_ego();
  /// synchronous batch-sequential implementation: main function
  void batch_asynchronous_ego();

  /// construct a batch of points from performing acquisition cycles
  void construct_batch_acquisition(size_t new_acq,  size_t new_batch);
  /// construct a batch of points from performing exploration cycles
  void construct_batch_exploration(size_t new_expl, size_t new_batch);

  /// evaluate batch in parallel and replace liar responses
  void evaluate_batch(bool rebuild);
  /// perform nonblocking synchronization for parallel evaluation of
  /// truth responses and replace liar responses for any completions
  bool query_batch(bool rebuild);
  /// backfill any completed truth response evaluations in case of
  /// nonblocking synchronization
  void backfill_batch(size_t new_acq, size_t new_expl);

  /// launch all jobs in the variables map queues
  void launch_batch();
  /// launch a single job
  void launch_single(const Variables& vars_star);

  /// update approximation data and constraint penalties/multipliers
  /// based on new truth data
  void process_truth_response_map(const IntResponseMap& truth_resp_map,
				  bool rebuild);
  /// update variable map queues based on completed jobs
  void update_variable_maps(const IntResponseMap& truth_resp_map);

  /// check convergence indicators to assess if EGO has converged
  bool converged();

  /// post-processing: retrieve and export best samples and responses
  void retrieve_final_results();

  /// initialize counters and limits used for assessing convergence
  void initialize_counters_limits();
  /// update counters used for assessing convergence
  void update_convergence_counters(const Variables& vars_star,
				   const Response&  resp_star);
  /// update counters used for assessing convergence in variables
  void update_convergence_counters(const Variables& vars_star);
  /// update counters used for assessing convergence in response
  void update_convergence_counters(const Response&  resp_star);

  /// delete all liar responses
  void pop_liar_responses();
  // delete a particular liar response
  //void pop_liar_response(int liar_id);
  /// evaluate and append a liar response
  void append_liar(const Variables& vars_star, int liar_id, bool rebuild);

  /// manage special value when iterator has advanced to end
  int extract_id(IntVarsMCIter it, const IntVariablesMap& map);

  /// determine meritFnStar from among the GP build data for use in EIF
  void compute_best_sample();
  /// extract best solution from among the GP build data for final results
  void extract_best_sample();
  /// extra response function build data from across the set of QoI
  void extract_qoi_build_data(size_t data_index, RealVector& fn_vals);

  /// helper for evaluating the value of the augmented Lagrangian merit fn
  Real augmented_lagrangian(const RealVector& mean);
  /// update constraint penalties and multipliers for a single response
  void update_constraints(const RealVector& fn_vals);
  /// update constraint penalties and multipliers for a set of responses
  void update_constraints(const IntResponseMap& truth_resp_map);

  /// helper for checking queued jobs in vars{Acquisition,Exploration}Map
  bool empty_queues() const;

  /// probability improvement (PI) function for the EGO
  /// PI acquisition function implementation
  Real compute_probability_improvement(const RealVector& means,
				       const RealVector& variances);
  /// expected improvement (EI) function for the EGO
  /// EI acquisition function implementation
  Real compute_expected_improvement(const RealVector& means,
				    const RealVector& variances);
  /// lower confidence bound (LCB) function for the EGO
  /// LCB acquisition function implementation
  Real compute_lower_confidence_bound(const RealVector& means,
				      const RealVector& variances);
  /// variance function for the EGO
  /// MSE acquisition implementation
  Real compute_variances(const RealVector& variances);

  /// expected violation function for the constraint functions
  RealVector expected_violation(const RealVector& means,
				const RealVector& variances);

  /// initialize and update the penaltyParameter
  void update_penalty();

  /// print mean and variance if debug flag is ON
  void debug_print_values(const Variables& vars);
  /// print counters if debug flag is ON
  void debug_print_dist_counters(Real dist_cv_star);
  /// print counters if debug flag is ON
  void debug_print_eif_counters(Real eif_star);
  /// DEBUG_PLOTS conditional - output set of samples used to build the
  /// GP if problem is 2D
  void debug_plots();

  //
  //- Heading: Objective/constraint evaluators passed to RecastModel
  //

  /// static function used as the objective function in the
  /// Expected Improvement (EIF) problem formulation for EGO
  static void PIF_objective_eval(const Variables& sub_model_vars,
				 const Variables& recast_vars,
				 const Response& sub_model_response,
				 Response& recast_response);
  /// static function used as the objective function in the
  /// Expected Improvement (EIF) problem formulation for EGO
  static void EIF_objective_eval(const Variables& sub_model_vars,
				 const Variables& recast_vars,
				 const Response& sub_model_response,
				 Response& recast_response);
  /// static function used as the objective function in the
  /// Lower-Confidence Bound (LCB) problem formulation for EGO
  static void LCB_objective_eval(const Variables& sub_model_vars,
				 const Variables& recast_vars,
				 const Response& sub_model_response,
				 Response& recast_response);
  /// Variance formulation for primary
  static void Variances_objective_eval(const Variables& sub_model_vars,
				       const Variables& recast_vars,
				       const Response& sub_model_response,
				       Response& recast_response);

  //
  //- Heading: Data
  //

  /// pointer to the active object instance used within the static evaluator
  /// functions in order to avoid the need for static data
  static EffGlobalMinimizer* effGlobalInstance;

  // controls iteration mode: "model" (normal usage) or "user_functions"
  // (user-supplied functions mode for "on the fly" instantiations).
  //String setUpType;

  /// GP model of response, one approximation per response function
  Model fHatModel;

  /// recast model which assimilates either (a) mean and variance to solve
  /// the max(EIF) sub-problem (used by EIF_objective_eval()) or (b) variance
  /// alone for pure exploration (used by Variances_objective_eval())
  Model approxSubProbModel;
  /* Note: don't need a separate model for EIF vs. exploration since the
     underlying simulation model is the one that evaluates the truth data
     and the recastings are only used for the approximate sub-problem solve.
     So there is no need to segregate processing queues: the aggregate set
     of available {variables,response} updates can be pushed to the GP
     irregardless of acquisition type. */

  /// minimum penalized response from among truth build data
  Real meritFnStar;

  /// previous solution to EIF approximation sub-problem
  RealVector prevSubProbSoln;

  /// order of the data used for surrogate construction, in ActiveSet
  /// request vector 3-bit format; user may override responses spec
  short dataOrder;
  // number of points in the current GP
  //size_t numDataPts;

  /// total batch size for parallel EGO
  int batchSize;
  /// number of new sampling points defined from maximizing acquisition function
  int batchSizeAcquisition;
  /// number of new sampling points defined from maximizing posterior variance
  int batchSizeExploration;
  /// counter for incrementing evaluation ids to allow synchronization with
  /// iteratedModel ids across acquisition and exploration job queues
  int batchEvalId;

  /// variable sets for batch evaluation of truth model, accumulated by
  /// construct_batch_acquisition()
  IntVariablesMap varsAcquisitionMap;
  /// variable sets for batch evaluation of truth model, accumulated by
  /// construct_batch_exploration()
  IntVariablesMap varsExplorationMap;

  /// bool flag if model supports asynchronous parallelism
  bool parallelFlag;
  /// algorithm option for fully asynchronous batch updating of the GP
  bool batchAsynch;

  // convergence checkers

  /// tolerance on distance between consecutive iterates
  Real distanceTol;
  /// counter for consecutive iterates with variable distance < distanceTol
  unsigned short distConvergenceCntr;
  /// limit for distance convergence counter
  unsigned short distConvergenceLimit;

  /// counter for consecutive iterates with EIF value < convergenceTol
  unsigned short eifConvergenceCntr;
  /// limit for EIF convergence counter
  unsigned short eifConvergenceLimit;

  /// counter for global iteration
  unsigned short globalIterCount;
};


inline EffGlobalMinimizer::~EffGlobalMinimizer()
{ }


inline const Model& EffGlobalMinimizer::algorithm_space_model() const
{ return fHatModel; }


inline void EffGlobalMinimizer::initialize_counters_limits()
{
  // note that eif/dist conv triggers must be sequential:
  // counters are reset to 0 when condition is not met
  globalIterCount = eifConvergenceCntr = distConvergenceCntr = 0;

  // These soft conv limits should be elevated to the spec as in other
  // surrogate-based approaches.  For now, they are hard-wired, with logic
  // to soften the criteria when liar-based iterations are included.
  // > *** TO DO: could introduce a predVarConv cntr/limit, but do we ever want
  //   to terminate based only on PV separate from or integrated with EIF?
  //   Right now, PV convergence assessment is omitted / non-integrated.
  if (parallelFlag) {
    // use relevant batch size bounded between serial setting (lwr) and 5 (upr)
    eifConvergenceLimit  = std::min(5, std::max(2, batchSizeAcquisition));//3;
    distConvergenceLimit = std::min(5, std::max(1, batchSize));           //2;
  }
  else
    { eifConvergenceLimit = 2;  distConvergenceLimit = 1; }
}


inline Real EffGlobalMinimizer::augmented_lagrangian(const RealVector& mean)
{
  return augmented_lagrangian_merit(mean,
    iteratedModel.primary_response_fn_sense(),
    iteratedModel.primary_response_fn_weights(), origNonlinIneqLowerBnds,
    origNonlinIneqUpperBnds, origNonlinEqTargets);
}


inline void EffGlobalMinimizer::update_constraints(const RealVector& fn_vals)
{
  // Update the merit function parameters
  // Logic follows Conn, Gould, and Toint, section 14.4:
  Real norm_cv_star = std::sqrt(constraint_violation(fn_vals, 0.));
  if (norm_cv_star < etaSequence)
    update_augmented_lagrange_multipliers(fn_vals);
  else
    update_penalty();
}


inline void EffGlobalMinimizer::
update_constraints(const IntResponseMap& truth_resp_map)
{
  for (IntRespMCIter r_cit = truth_resp_map.begin();
       r_cit != truth_resp_map.end(); ++r_cit)
    update_constraints(r_cit->second.function_values());
}


inline void EffGlobalMinimizer::
update_convergence_counters(const Variables& vars_star,
			    const Response&  resp_star)
{
  update_convergence_counters(vars_star);
  update_convergence_counters(resp_star);
}


inline bool EffGlobalMinimizer::empty_queues() const
{ return (varsAcquisitionMap.empty() && varsExplorationMap.empty()); }


inline int EffGlobalMinimizer::
extract_id(IntVarsMCIter cit, const IntVariablesMap& map)
{ return (cit == map.end()) ? INT_MAX : cit->first; }


inline void EffGlobalMinimizer::pop_liar_responses()
{
  // Pop counts are 1 for append_approximation() calls in append_liar()
  // (only {evaluate/query}_batch() appends multiple in truth_resp_map),
  // so here we call pop_approximation() repeatedly for each liar.
  for (size_t i=0; i<batchSize; i++) {
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "\nParallel EGO: deleting liar response...\n";
    fHatModel.pop_approximation(false); // defer rebuild until truth append
  }
  //numDataPts = fHatModel.approximation_data(0).points();
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "\nParallel EGO: all liar responses deleted.\n";
}


/* Don't want to remove and then append (changes order, incurs more overhead);
   prefer to update in place.
inline void EffGlobalMinimizer::pop_liar_response(int liar_id)
{
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "\nParallel EGO: deleting liar response...\n";
  //fHatModel.pop_approximation(liar_id, false); // *** TO DO ***
  //numDataPts = fHatModel.approximation_data(0).points();
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "\nParallel EGO: liar response deleted.\n";
}
*/


inline void EffGlobalMinimizer::debug_print_values(const Variables& vars)
{
  fHatModel.active_variables(vars);
  fHatModel.evaluate();
  const RealVector& mean = fHatModel.current_response().function_values();
  RealVector variance = fHatModel.approximation_variances(vars),
    ev = expected_violation(mean, variance), stdv(numFunctions);
  for (size_t i=0; i<numFunctions; i++)
    stdv[i] = std::sqrt(variance[i]);
  Cout << "\nexpected values    =\n" << mean
       << "\nstandard deviation =\n" << stdv
       << "\nexpected violation =\n" << ev << std::endl;
}


inline void EffGlobalMinimizer::debug_print_dist_counters(Real dist_cv_star)
{
  Cout << "EGO Iteration " << globalIterCount << ": dist_cv_star = "
       << dist_cv_star << " distance convergence cntr = "
       << distConvergenceCntr << '\n';
}


inline void EffGlobalMinimizer::debug_print_eif_counters(Real eif_star)
{
  Cout << "EGO Iteration " << globalIterCount << ": EIF star = " << eif_star
       << " EIF convergence cntr = " << eifConvergenceCntr << '\n';
}

} // namespace Dakota

#endif
