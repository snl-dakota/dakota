/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDExpansion
//- Description: Iterator base class for polynomial expansion methods for UQ
//- Owner:       Mike Eldred, Sandia National Laboratories

#ifndef NOND_EXPANSION_H
#define NOND_EXPANSION_H

#include "DakotaNonD.hpp"


namespace Dakota {

/// Base class for polynomial chaos expansions (PCE) and stochastic
/// collocation (SC)

/** The NonDExpansion class provides a base class for methods that use
    polynomial expansions to approximate the effect of parameter
    uncertainties on response functions of interest. */

class NonDExpansion: public NonD
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDExpansion(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor
  NonDExpansion(unsigned short method_name, Model& model,
		short exp_coeffs_approach, short refine_type,
		short refine_control, short covar_control, short ml_discrep,
		short rule_nest, short rule_growth, bool piecewise_basis,
		bool use_derivs);
  /// destructor
  ~NonDExpansion();

  //
  //- Heading: Virtual function redefinitions
  //

  bool resize();
  void derived_init_communicators(ParLevLIter pl_iter);
  void derived_set_communicators(ParLevLIter pl_iter);
  void derived_free_communicators(ParLevLIter pl_iter);

  void nested_variable_mappings(const SizetArray& c_index1,
				const SizetArray& di_index1,
				const SizetArray& ds_index1,
				const SizetArray& dr_index1,
				const ShortArray& c_target2,
				const ShortArray& di_target2,
				const ShortArray& ds_target2,
				const ShortArray& dr_target2);

  /// perform a forward uncertainty propagation using PCE/SC methods
  void core_run();
  /// print the final statistics
  void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

  const Model& algorithm_space_model() const;

  //
  //- Heading: Virtual functions
  //

  /// evaluate allSamples for inclusion in the (PCE regression) approximation
  /// and retain the best set (well spaced) of size batch_size
  virtual void select_refinement_points(
    const RealVectorArray& candidate_samples, unsigned short batch_size,
    RealMatrix& best_samples);

  /// append new data to uSpaceModel and update expansion order (PCE only)
  virtual void append_expansion(const RealMatrix& samples,
				const IntResponseMap& resp_map);

  //
  //- Heading: Member functions
  //

  /// return maxRefineIterations
  int maximum_refinement_iterations() const;
  /// set maxRefineIterations
  void maximum_refinement_iterations(int max_refine_iter);

protected:

  //
  //- Heading: Virtual functions
  //

  /// perform error checks and mode overrides
  virtual void resolve_inputs(short& u_space_type, short& data_order);
  /// initialize uSpaceModel polynomial approximations with PCE/SC data
  virtual void initialize_u_space_model();
  /// initialize random variable definitions and final stats arrays
  virtual void initialize_expansion();
  /// form the expansion by calling uSpaceModel.build_approximation()
  virtual void compute_expansion();
  /// finalize mappings for the uSpaceModel
  virtual void finalize_expansion();
  /// uniformly increment the expansion order and structured/unstructured
  /// grid (PCE only)
  virtual void increment_order_and_grid();
  /// uniformly decrement the expansion order and structured/unstructured
  /// grid (PCE only)
  virtual void decrement_order_and_grid();
  /// assign the current values from the input specification sequence
  virtual void assign_specification_sequence();
  /// increment the input specification sequence and assign values
  virtual void increment_specification_sequence();
  /// update an expansion; avoids overhead in compute_expansion()
  virtual void update_expansion();
  /// combine coefficients, promote to active, and update statsType
  virtual void combined_to_active();
  /// archive expansion coefficients, as supported by derived instance
  virtual void archive_coefficients();

  /// perform any required expansion roll-ups prior to metric computation
  virtual void metric_roll_up();

  /// compute 2-norm of change in response covariance
  virtual Real compute_covariance_metric(bool revert, bool print_metric);
  /// compute 2-norm of change in final statistics
  virtual Real compute_level_mappings_metric(bool revert, bool print_metric);
  // compute 2-norm of change in final statistics
  //virtual Real compute_final_statistics_metric(bool revert,bool print_metric);

  /// calculate analytic and numerical statistics from the expansion,
  /// supporting {REFINEMENT,INTERMEDIATE,FINAL}_RESULTS modes
  virtual void compute_statistics(short results_state = FINAL_RESULTS);
  /// extract statistics from native stats arrays for a selected candidate
  virtual void pull_candidate(RealVector& stats_star);
  /// restore statistics into native stats arrays for a selected candidate
  virtual void push_candidate(const RealVector& stats_star);

  /// print global sensitivity indices
  virtual void print_sobol_indices(std::ostream& s);

  //
  //- Heading: Virtual function redefinitions
  //

  /// set covarianceControl defaults and shape respCovariance
  void initialize_response_covariance();
  /// update function values within finalStatistics
  void update_final_statistics();
  /// update function gradients within finalStatistics
  void update_final_statistics_gradients();

  //
  //- Heading: Member functions
  //

  /// helper for initializing a numerical integration grid
  void initialize_u_space_grid();

  // common constructor code for initialization of natafTransform
  //void initialize_random(short u_space_type);

  /// check length and content of dimension preference vector
  void check_dimension_preference(const RealVector& dim_pref) const;

  /// assign a NonDCubature instance within u_space_sampler
  void construct_cubature(Iterator& u_space_sampler, Model& g_u_model,
			  unsigned short cub_int_order);
  /// assign a NonDQuadrature instance within u_space_sampler based on
  /// a quad_order specification
  void construct_quadrature(Iterator& u_space_sampler, Model& g_u_model,
			    unsigned short quad_order,
			    const RealVector& dim_pref);
  /// assign a NonDQuadrature instance within u_space_sampler that
  /// generates a filtered tensor product sample set
  void construct_quadrature(Iterator& u_space_sampler, Model& g_u_model,
			    unsigned short quad_order,
			    const RealVector& dim_pref, int filtered_samples);
  /// assign a NonDQuadrature instance within u_space_sampler that
  /// samples randomly from a tensor product multi-index
  void construct_quadrature(Iterator& u_space_sampler, Model& g_u_model,
			    unsigned short quad_order,
			    const RealVector& dim_pref,
			    int random_samples, int seed);
  /// assign a NonDSparseGrid instance within u_space_sampler
  void construct_sparse_grid(Iterator& u_space_sampler, Model& g_u_model,
			     unsigned short ssg_level,
			     const RealVector& dim_pref);
  // assign a NonDIncremLHSSampling instance within u_space_sampler
  //void construct_incremental_lhs(Iterator& u_space_sampler, Model& u_model,
  //				 int num_samples, int seed, const String& rng);

  /// construct the expansionSampler for evaluating samples on uSpaceModel
  void construct_expansion_sampler(const String& import_approx_file,
    unsigned short import_approx_format = TABULAR_ANNOTATED,
    bool import_approx_active_only = false);

  /// construct a multifidelity expansion, across model forms or
  /// discretization levels
  void multifidelity_expansion(short refine_type, bool to_active = true);
  /// construct a multifidelity expansion, across model forms or
  /// discretization levels
  void greedy_multifidelity_expansion();

  /// configure fidelity/level counts from model hierarchy
  void configure_levels(size_t& num_lev, unsigned short& model_form,
			bool& multilevel, bool mf_precedence);
  /// configure fidelity/level counts from model hierarchy
  void configure_cost(size_t num_lev, bool multilevel, RealVector& cost);
  /// configure response mode and truth/surrogate model indices within
  /// hierarchical iteratedModel
  void configure_indices(unsigned short lev, unsigned short form,
			 bool multilevel);
  /// return aggregate cost (one or more models) for a level sample
  Real level_cost(unsigned short lev, const RealVector& cost);
  /// compute equivHFEvals from samples per level and cost per evaluation
  void compute_equivalent_cost(const SizetArray& N_l, const RealVector& cost);

  /// creat a model key from level index, form index, and multilevel flag
  void form_key(unsigned short lev, unsigned short form, bool multilevel,
		UShortArray& model_key);
  /// activate model key within uSpaceModel
  void activate_key(const UShortArray& model_key);
  /// activate model key within uSpaceModel
  void activate_key(unsigned short lev, unsigned short form, bool multilevel);

  /// refine the reference expansion found by compute_expansion() using
  /// uniform/adaptive p-/h-refinement strategies
  void refine_expansion();
  /// initialization of expansion refinement, if necessary
  void pre_refinement();
  /// advance the refinement strategy one step
  size_t core_refinement(Real& metric, bool revert = false,
			 bool print_metric = true);
  /// finalization of expansion refinement, if necessary
  void post_refinement(Real& metric, bool reverted = false);

  /// helper function to manage different grid increment cases
  void increment_grid(bool update_anisotropy = true);
  /// helper function to manage different grid decrement cases
  void decrement_grid();
  /// helper function to manage different grid merge cases
  void merge_grid();

  /// helper function to manage different push increment cases
  void push_increment();
  /// helper function to manage different pop increment cases
  void pop_increment();

  /// update statsType, here and in Pecos::ExpansionConfigOptions
  void statistics_type(short stats_type, bool clear_bits = true);

  /// calculate the response covariance (diagonal or full matrix) for
  /// the expansion indicated by statsType
  void compute_covariance();
  /// calculate the response covariance of the active expansion
  void compute_active_covariance();
  /// calculate the response covariance of the combined expansion
  void compute_combined_covariance();

  /// calculate the diagonal response variance of the active expansion
  void compute_active_diagonal_variance();
  /// calculate the diagonal response variance of the cmbined expansion
  void compute_combined_diagonal_variance();

  /// calculate off diagonal terms in respCovariance(i,j) for j<i for
  /// the expansion indicated by statsType
  void compute_off_diagonal_covariance();
  /// calculate off diagonal terms in respCovariance(i,j) for j<i
  /// using the active expansion coefficients
  void compute_active_off_diagonal_covariance();
  /// calculate off diagonal terms in respCovariance(i,j) for j<i
  /// using the combined expansion coefficients
  void compute_combined_off_diagonal_covariance();

  /// compute expansion moments; this uses a lightweight approach for
  /// incremental statistics (no additional moments; no finalStatistics update)
  void compute_moments();
  /// compute all analytic/numerical level mappings; this uses a lightweight
  /// approach for incremental statistics (no derivatives, no finalStatistics
  /// update)
  void compute_level_mappings();
  /// compute only numerical level mappings; this uses a lightweight approach
  /// for incremental statistics (no derivatives, no finalStatistics update)
  void compute_numerical_level_mappings();
  /// compute Sobol' indices for main, interaction, and total effects; this
  /// is intended for incremental statistics
  void compute_sobol_indices();

  /// print resp{Variance,Covariance}
  void print_covariance(std::ostream& s);
  /// print resp_var (response variance vector) using optional pre-pend
  void print_variance(std::ostream& s, const RealVector& resp_var,
		      const String& prepend = "");
  /// print resp_covar (response covariance matrix) using optional pre-pend
  void print_covariance(std::ostream& s, const RealSymMatrix& resp_covar,
			const String& prepend = "");

  /// archive the central moments (numerical and expansion) to ResultsDB
  void archive_moments();

  /// archive the Sobol' indices to the resultsDB
  void archive_sobol_indices();

  void pull_reference(RealVector& stats_ref);
  void push_reference(const RealVector& stats_ref);

  /// pull lower triangle of symmetric matrix into vector
  void pull_lower_triangle(const RealSymMatrix& mat, RealVector& vec,
			   size_t offset = 0);
  /// push vector into lower triangle of symmetric matrix
  void push_lower_triangle(const RealVector& vec, RealSymMatrix& mat,
			   size_t offset = 0);

  //
  //- Heading: Data
  //

  /// Model representing the approximate response function in u-space,
  /// after u-space recasting and polynomial data fit recursions
  Model uSpaceModel;

  /// method for collocation point generation and subsequent
  /// calculation of the expansion coefficients
  short expansionCoeffsApproach;
  /// type of expansion basis: DEFAULT_BASIS or
  /// Pecos::{NODAL,HIERARCHICAL}_INTERPOLANT for SC or
  /// Pecos::{TENSOR_PRODUCT,TOTAL_ORDER,ADAPTED}_BASIS for PCE regression
  short expansionBasisType;
  /// type of statistical metric: NO_EXPANSION_STATS,
  /// ACTIVE_EXPANSION_STATS, or COMBINED_EXPANSION_STATS
  short statsType;

  /// flag for combined variable expansions which include a
  /// non-probabilistic subset (design, epistemic, state)
  bool allVars;

  /// emulation approach for multilevel discrepancy: distinct or recursive
  short multilevDiscrepEmulation;
  /// number of samples allocated to each level of a discretization/model
  /// hierarchy within multilevel/multifidelity methods
  SizetArray NLev;
  /// equivalent number of high fidelity evaluations accumulated using samples
  /// across multiple model forms and/or discretization levels
  Real equivHFEvals;

  /// number of invocations of core_run()
  size_t numUncertainQuant;

  /// number of truth samples performed on g_u_model to form the expansion
  int numSamplesOnModel;
  /// number of approximation samples performed on the polynomial
  /// expansion in order to estimate probabilities
  int numSamplesOnExpansion;

  /// flag indicating the use of relative scaling in refinement metrics
  bool relativeMetric;

  /// flag for indicating state of \c nested and \c non_nested overrides of
  /// default rule nesting, which depends on the type of integration driver;
  /// this is defined in construct_{quadrature,sparse_grid}(), such that
  /// override attributes (defined in ctors) must be used upstream
  bool nestedRules;
  /// user override of default rule nesting: NO_NESTING_OVERRIDE,
  /// NESTED, or NON_NESTED
  short ruleNestingOverride;
  /// user override of default rule growth: NO_GROWTH_OVERRIDE,
  /// RESTRICTED, or UNRESTRICTED
  short ruleGrowthOverride;

  /// flag for \c piecewise specification, indicating usage of local
  /// basis polynomials within the stochastic expansion
  bool piecewiseBasis;

  /// flag for \c use_derivatives specification, indicating usage of derivative
  /// data (with respect to expansion variables) to enhance the calculation of
  /// the stochastic expansion.
  /** This is part of the method specification since the instantiation of the
      global data fit surrogate is implicit with no user specification.  This
      behavior is distinct from the usage of response derivatives with respect
      to auxilliary variables (design, epistemic) for computing derivatives of
      aleatory expansion statistics with respect to these variables. */
  bool useDerivs;

  /// stores the initial variables data in u-space
  RealVector initialPtU;

  /// refinement type: NO_REFINEMENT, P_REFINEMENT, or H_REFINEMENT
  short refineType;
  /// refinement control: NO_CONTROL, UNIFORM_CONTROL, LOCAL_ADAPTIVE_CONTROL,
  /// DIMENSION_ADAPTIVE_CONTROL_SOBOL, DIMENSION_ADAPTIVE_CONTROL_DECAY, or
  /// DIMENSION_ADAPTIVE_CONTROL_GENERALIZED
  short refineControl;
  /// refinement metric: NO_METRIC, COVARIANCE_METRIC, LEVEL_STATS_METRIC,
  /// or MIXED_STATS_METRIC
  short refineMetric;

  /// enumeration for controlling response covariance calculation and
  /// output: {DEFAULT,DIAGONAL,FULL}_COVARIANCE
  short covarianceControl;

  /// number of consecutive iterations within tolerance required to
  /// indicate soft convergence
  unsigned short softConvLimit;

  /// symmetric matrix of analytic response covariance (full response
  /// covariance option)
  RealSymMatrix respCovariance;
  /// vector of response variances (diagonal response covariance option)
  RealVector respVariance;

  /// stats of the best candidate for the current level
  RealVector levelStatsStar;
  /// stats of the best candidate across all levels
  RealVector statsStar;

private:

  //
  //- Heading: Convenience function definitions
  //

  /// initialize data based on variable counts
  void initialize_counts();

  /// set response mode to AGGREGATED_MODELS and recur response size updates
  void aggregated_models_mode();
  /// set response mode to BYPASS_SURROGATE and recur response size updates
  void bypass_surrogate_mode();

  /// compute average of total Sobol' indices (from VBD) across the
  /// response set for use as an anisotropy indicator
  void reduce_total_sobol_sets(RealVector& avg_sobol);
  /// compute minimum of spectral coefficient decay rates across the
  /// response set for use as an anisotropy indicator
  void reduce_decay_rate_sets(RealVector& min_decay);

  /// perform an adaptive refinement increment using generalized sparse grids
  size_t increment_sets(Real& delta_star, bool revert, bool print_metric);
  /// finalization of adaptive refinement using generalized sparse grids
  void finalize_sets(bool converged_within_tol, bool reverted = false);

  /// promote selected candidate into reference grid + expansion
  void select_candidate(size_t best_candidate);
  /// promote selected index set candidate into reference grid + expansion
  void select_index_set_candidate(
    std::set<UShortArray>::const_iterator cit_star);
  /// promote selected refinement increment candidate into reference
  /// grid + expansion
  void select_increment_candidate();

  /// analytic portion of compute_statistics() from post-processing of
  /// expansion coefficients (used for FINAL_RESULTS)
  void compute_analytic_statistics();
  /// numerical portion of compute_statistics() from sampling on the
  /// expansion (used for FINAL_RESULTS)
  void compute_numerical_statistics();
  /// refinements to numerical probability statistics from importanceSampler
  void compute_numerical_stat_refinements(RealVectorArray& imp_sampler_stats,
					  RealRealPairArray& min_max_fns);

  /// helper to define the expansionSampler's data requests when sampling on
  /// the expansion
  void define_sampler_asv(ShortArray& sampler_asv);
  /// helper to run the expansionSampler and compute its statistics
  void run_sampler(const ShortArray& sampler_asv,
		   RealVector& exp_sampler_stats);
  /// helper to refine the results from expansionSampler with importance
  /// sampling (for probability levels) or bounds post-processing (for PDFs)
  void refine_sampler(RealVectorArray& imp_sampler_stats,
		      RealRealPairArray& min_max_fns);
  
  /// print expansion and numerical moments
  void print_moments(std::ostream& s);
  /// print local sensitivities evaluated at initialPtU
  void print_local_sensitivity(std::ostream& s);

  //
  //- Heading: Data
  //

  /// Iterator used for sampling on the uSpaceModel to generate approximate
  /// probability/reliability/response level statistics.  Currently this is
  /// an LHS sampling instance, but AIS could also be used.
  Iterator expansionSampler;
  /// Iterator used to refine the approximate probability estimates 
  /// generated by the expansionSampler using importance sampling
  Iterator importanceSampler;
  
  /// derivative of the expansion with respect to the x-space variables
  /// evaluated at the means (used as uncertainty importance metrics)
  RealMatrix expGradsMeanX;

  /// maximum number of uniform/adaptive refinement iterations
  /// (specialization of maxIterations)
  int maxRefineIterations;
  /// maximum number of regression solver iterations (specialization
  /// of maxIterations)
  int maxSolverIterations;
  
  /// flag indicating the activation of variance-bsaed decomposition
  /// for computing Sobol' indices
  bool vbdFlag;
  /// limits the order of interactions within the component Sobol' indices
  unsigned short vbdOrderLimit;
  /// tolerance for omitting output of small VBD indices
  Real vbdDropTol;

  /// integration refinement for expansion sampler
  unsigned short integrationRefine;
  /// random number generator for expansion sampler
  String expansionRng;
  /// seed for expansion sampler random number generator
  int origSeed;
  /// sample type for expansion sampler
  unsigned short expansionSampleType;
  /// refinement samples for expansion sampler
  IntVector refinementSamples;
};


inline void NonDExpansion::
nested_variable_mappings(const SizetArray& c_index1,
			 const SizetArray& di_index1,
			 const SizetArray& ds_index1,
			 const SizetArray& dr_index1,
			 const ShortArray& c_target2,
			 const ShortArray& di_target2,
			 const ShortArray& ds_target2,
			 const ShortArray& dr_target2)
{
  uSpaceModel.nested_variable_mappings(c_index1, di_index1, ds_index1,
				       dr_index1, c_target2, di_target2,
				       ds_target2, dr_target2);
}


inline int NonDExpansion::maximum_refinement_iterations() const
{ return maxRefineIterations; }


inline void NonDExpansion::maximum_refinement_iterations(int max_refine_iter)
{ maxRefineIterations = max_refine_iter; }


inline const Model& NonDExpansion::algorithm_space_model() const
{ return uSpaceModel; }


inline void NonDExpansion::aggregated_models_mode()
{
  // update iteratedModel / uSpaceModel in separate calls rather than using
  // uSpaceModel.surrogate_response_mode(mode) since DFSurrModel must pass
  // mode along to iteratedModel (a HierarchSurrModel) without absorbing it
  if (iteratedModel.surrogate_response_mode() != AGGREGATED_MODELS) {
    iteratedModel.surrogate_response_mode(AGGREGATED_MODELS);//MODEL_DISCREPANCY
    uSpaceModel.resize_from_subordinate_model();// recurs until hits aggregation
  }
}


inline void NonDExpansion::bypass_surrogate_mode()
{
  // update iteratedModel / uSpaceModel in separate calls rather than using
  // uSpaceModel.surrogate_response_mode(mode) since DFSurrModel must pass
  // mode along to iteratedModel (a HierarchSurrModel) without absorbing it
  if (iteratedModel.surrogate_response_mode() != BYPASS_SURROGATE) {
    iteratedModel.surrogate_response_mode(BYPASS_SURROGATE); // single level
    uSpaceModel.resize_from_subordinate_model();// recurs until hits aggregation
  }
}


inline void NonDExpansion::
form_key(unsigned short lev, unsigned short form, bool multilevel,
	 UShortArray& model_key)
{
  if (multilevel) // model form is fixed @ HF; lev enumerates the levels
    { model_key.resize(2); model_key[0] = form; model_key[1] = lev; }
  else            // lev enumerates the model forms; levels are ignored
    { model_key.resize(1); model_key[0] = lev; } // mi_key[1] = _NPOS;
}


inline void NonDExpansion::
activate_key(const UShortArray& model_key)
{ uSpaceModel.active_model_key(model_key); }


inline void NonDExpansion::
activate_key(unsigned short lev, unsigned short form, bool multilevel)
{
  UShortArray model_key;
  form_key(lev, form, multilevel, model_key);
  activate_key(model_key); // assign key to uSpaceModel
}


inline Real NonDExpansion::
level_cost(unsigned short lev, const RealVector& cost)
{
  if (cost.empty())
    return 0.;
  else {
    Real lev_cost = cost[lev];
    if (lev && multilevDiscrepEmulation == DISTINCT_EMULATION)
      lev_cost += cost[lev-1]; // discrepancies incur 2 level costs
    return lev_cost;
  }
}


inline void NonDExpansion::metric_roll_up()
{
  // greedy_multifidelity_expansion() assesses level candidates using combined
  // stat metrics, which by default require expansion combination (overridden
  // for hierarchical SC)
  if (statsType == Pecos::COMBINED_EXPANSION_STATS)
    uSpaceModel.combine_approximation();
}


inline void NonDExpansion::compute_active_covariance()
{
  switch (covarianceControl) {
  case DIAGONAL_COVARIANCE:
    compute_active_diagonal_variance(); break;
  case FULL_COVARIANCE:
    compute_active_diagonal_variance();
    compute_active_off_diagonal_covariance(); break;
  }
}


inline void NonDExpansion::compute_combined_covariance()
{
  switch (covarianceControl) {
  case DIAGONAL_COVARIANCE:
    compute_combined_diagonal_variance(); break;
  case FULL_COVARIANCE:
    compute_combined_diagonal_variance();
    compute_combined_off_diagonal_covariance(); break;
  }
}


inline void NonDExpansion::compute_off_diagonal_covariance()
{
  if (numFunctions <= 1)
    return;

  // See also full_covar_stats logic in compute_analytic_statistics() ...

  switch (statsType) {
  case Pecos::ACTIVE_EXPANSION_STATS:
    compute_active_off_diagonal_covariance();   break;
  case Pecos::COMBINED_EXPANSION_STATS:
    compute_combined_off_diagonal_covariance(); break;
  }
}


inline void NonDExpansion::compute_covariance()
{
  switch (statsType) {
  // multifidelity_expansion() is outer loop:
  // > use of refine_expansion(): refine individually based on level covariance
  // > after combine_approx(), combined_to_active() enables use of active covar
  case Pecos::ACTIVE_EXPANSION_STATS:
    compute_active_covariance();   break;
  // greedy_multifidelity_expansion() (multifidelity_expansion() on inner loop):
  // > roll up effect of level candidate on combined multilevel covariance,
  //   avoiding combined_to_active() promotion until end
  // > limited stats support for combinedExpCoeffs: only compute_covariance()
  case Pecos::COMBINED_EXPANSION_STATS:
    compute_combined_covariance(); break;
  }
}


inline void NonDExpansion::
pull_lower_triangle(const RealSymMatrix& mat, RealVector& vec, size_t offset)
{
  size_t i, j, cntr = offset, nr = mat.numRows(),
    sm_len = (nr*nr + nr)/2, vec_len = offset + sm_len;
  if (vec.length() != vec_len) vec.resize(vec_len);
  for (i=0; i<nr; ++i)
    for (j=0; j<=i; ++j, ++cntr)
      vec[cntr] = mat(i,j); // pull from lower triangle
}


inline void NonDExpansion::
push_lower_triangle(const RealVector& vec, RealSymMatrix& mat, size_t offset)
{
  size_t i, j, cntr = offset, nr = mat.numRows();
  if (vec.length() != offset + (nr*nr + nr)/2) {
    Cerr << "Error: inconsistent vector length in NonDExpansion::"
	 << "push_lower_triangle()" << std::endl;
    abort_handler(METHOD_ERROR);
  }
  for (i=0; i<nr; ++i)
    for (j=0; j<=i; ++j, ++cntr)
      mat(i,j) = vec[cntr]; // push to lower triangle
}


inline void NonDExpansion::pull_candidate(RealVector& stats_star)
{ pull_reference(stats_star); } // default implementation: candidate same as ref


inline void NonDExpansion::push_candidate(const RealVector& stats_star)
{ push_reference(stats_star); } // default implementation: candidate same as ref


inline void NonDExpansion::update_final_statistics()
{
  // aleatory final stats & their grads are updated directly within
  // compute_statistics() (NonD::update_aleatory_final_statistics() is awkward
  // for NonDExpansion since Pecos manages the moment arrays), such that all
  // that remains are system final stats and additional gradient scaling.
  if (respLevelTargetReduce) {
    update_system_final_statistics();
    update_system_final_statistics_gradients();
  }
  update_final_statistics_gradients();
}


inline void NonDExpansion::archive_coefficients()
{ /* default is no-op */ }


inline void NonDExpansion::
check_dimension_preference(const RealVector& dim_pref) const
{
  size_t len = dim_pref.length();
  if (len) {
    if (len != numContinuousVars) {
      Cerr << "Error: length of dimension preference specification (" << len
	   << ") is inconsistent with continuous expansion variables ("
	   << numContinuousVars << ")." << std::endl;
      abort_handler(METHOD_ERROR);
    }
    else
      for (size_t i=0; i<len; ++i)
	if (dim_pref[i] < 0.) { // allow zero preference
	  Cerr << "Error: bad dimension preference value (" << dim_pref[i]
	       << ")." << std::endl;
	  abort_handler(METHOD_ERROR);
	}
  }
}

} // namespace Dakota

#endif
