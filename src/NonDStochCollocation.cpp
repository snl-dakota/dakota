/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDStochCollocation
//- Description: Implementation code for NonDStochCollocation class
//- Owner:       Mike Eldred

#include "dakota_system_defs.hpp"
#include "NonDStochCollocation.hpp"
#include "NonDSparseGrid.hpp"
#include "DakotaResponse.hpp"
#include "ProblemDescDB.hpp"
#include "DataFitSurrModel.hpp"
#include "ProbabilityTransformModel.hpp"
#include "SharedPecosApproxData.hpp"
#include "PecosApproximation.hpp"
#include "SharedInterpPolyApproxData.hpp"

//#define ALLOW_GLOBAL_HERMITE_INTERPOLATION
//#define DEBUG


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator
    instantiation using the ProblemDescDB. */
NonDStochCollocation::
NonDStochCollocation(ProblemDescDB& problem_db, Model& model):
  NonDExpansion(problem_db, model)
{
  // ----------------
  // Resolve settings
  // ----------------
  short data_order,
    u_space_type = probDescDB.get_short("method.nond.expansion_type");
  resolve_inputs(u_space_type, data_order);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  g_u_model.assign_rep(std::make_shared<ProbabilityTransformModel>(
    iteratedModel, u_space_type)); // retain dist bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  // LHS/Incremental LHS/Quadrature/SparseGrid samples in u-space
  // generated using active sampling view:
  Iterator u_space_sampler;
  config_integration(probDescDB.get_ushort("method.nond.quadrature_order"),
		     probDescDB.get_ushort("method.nond.sparse_grid_level"),
		     probDescDB.get_rv("method.nond.dimension_preference"),
		     u_space_type, u_space_sampler, g_u_model);
  String pt_reuse, approx_type;
  config_approximation_type(approx_type);

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for SCBDO with polynomials over {u}+{d}, change view to All.
  short corr_order = -1, corr_type = NO_CORRECTION;
  UShortArray approx_order; // empty
  const ActiveSet& recast_set = g_u_model.current_response().active_set();
  // DFSModel: consume any QoI aggregation; support surrogate grad evals at most
  ShortArray sc_asv(g_u_model.qoi(), 3); // for stand alone mode
  ActiveSet  sc_set(sc_asv, recast_set.derivative_vector());
  const ShortShortPair& sc_view = g_u_model.current_variables().view();
  String empty_str; // build data import not supported for structured grids
  uSpaceModel.assign_rep(std::make_shared<DataFitSurrModel>(u_space_sampler,
    g_u_model, sc_set, sc_view, approx_type, approx_order, corr_type,
    corr_order, data_order, outputLevel, pt_reuse, empty_str, TABULAR_ANNOTATED,
    false, probDescDB.get_string("method.export_approx_points_file"),
    probDescDB.get_ushort("method.export_approx_format")));
  initialize_u_space_model();

  // -------------------------------
  // Construct expSampler, if needed
  // -------------------------------
  construct_expansion_sampler(problem_db.get_ushort("method.sample_type"),
    problem_db.get_string("method.random_number_generator"),
    problem_db.get_ushort("method.nond.integration_refinement"),
    problem_db.get_iv("method.nond.refinement_samples"),
    probDescDB.get_string("method.import_approx_points_file"),
    probDescDB.get_ushort("method.import_approx_format"),
    probDescDB.get_bool("method.import_approx_active_only"));

  if (parallelLib.command_line_check())
    Cout << "\nStochastic collocation construction completed: initial grid "
	 << "size of " << numSamplesOnModel << " evaluations to be performed."
	 << std::endl;
}


/** This constructor is used for helper iterator instantiation on the fly. */
NonDStochCollocation::
NonDStochCollocation(Model& model, short exp_coeffs_approach,
		     unsigned short num_int, const RealVector& dim_pref,
		     short u_space_type, short refine_type,
		     short refine_control, short covar_control,
		     short rule_nest, short rule_growth,
		     bool piecewise_basis, bool use_derivs):
  NonDExpansion(STOCH_COLLOCATION, model, model.current_variables().view(),
		exp_coeffs_approach, dim_pref, 0, refine_type, refine_control,
		covar_control, 0., rule_nest, rule_growth, piecewise_basis,
		use_derivs)
  // Note: non-zero seed would be needed for expansionSampler, if defined
{
  // ----------------
  // Resolve settings
  // ----------------
  short data_order;
  resolve_inputs(u_space_type, data_order);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  g_u_model.assign_rep(std::make_shared<ProbabilityTransformModel>(
    iteratedModel, u_space_type)); // retain dist bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  // LHS/Incremental LHS/Quadrature/SparseGrid samples in u-space
  // generated using active sampling view:
  Iterator u_space_sampler;
  config_integration(exp_coeffs_approach, num_int, dim_pref, u_space_sampler,
		     g_u_model);
  String pt_reuse, approx_type;
  config_approximation_type(approx_type);

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for SCBDO with polynomials over {u}+{d}, change view to All.
  short corr_order = -1, corr_type = NO_CORRECTION;
  UShortArray approx_order; // empty
  const ActiveSet& recast_set = g_u_model.current_response().active_set();
  // DFSModel: consume any QoI aggregation.
  // TO DO: support surrogate Hessians in helper mode.
  ShortArray sc_asv(g_u_model.qoi(), 3); // TO DO: consider passing in data_mode
  ActiveSet  sc_set(sc_asv, recast_set.derivative_vector());
  const ShortShortPair& sc_view = g_u_model.current_variables().view();
  uSpaceModel.assign_rep(std::make_shared<DataFitSurrModel>(u_space_sampler,
    g_u_model, sc_set, sc_view, approx_type, approx_order, corr_type,
    corr_order, data_order, outputLevel, pt_reuse));
  initialize_u_space_model();

  // no expansionSampler, no numSamplesOnExpansion
}


/** This constructor is called from derived class constructors that
    customize the object construction. */
NonDStochCollocation::
NonDStochCollocation(unsigned short method_name, ProblemDescDB& problem_db,
		     Model& model):
  NonDExpansion(problem_db, model)
{
  // Logic delegated to derived class constructor...
}


/** This constructor is called from derived class constructors that
    customize the object construction. */
NonDStochCollocation::
NonDStochCollocation(unsigned short method_name, Model& model,
		     short exp_coeffs_approach, const RealVector& dim_pref,
		     short refine_type, short refine_control,
		     short covar_control, short ml_alloc_control,
		     short ml_discrep, short rule_nest, short rule_growth,
		     bool piecewise_basis, bool use_derivs):
  NonDExpansion(method_name, model, model.current_variables().view(),
		exp_coeffs_approach, dim_pref, 0, refine_type, refine_control,
		covar_control, 0., rule_nest, rule_growth, piecewise_basis,
		use_derivs)
{
  multilevAllocControl     = ml_alloc_control;
  multilevDiscrepEmulation = ml_discrep;

  // Logic delegated to derived class constructor...
}


NonDStochCollocation::~NonDStochCollocation()
{ }


void NonDStochCollocation::
config_integration(unsigned short quad_order, unsigned short ssg_level,
		   const RealVector& dim_pref, short u_space_type, 
		   Iterator& u_space_sampler, Model& g_u_model)
{
  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  if (quad_order != USHRT_MAX) {
    expansionCoeffsApproach = Pecos::QUADRATURE;
    // TensorProductDriver does not currently support a hierarchical grid via
    // increment_grid(), etc., although default rule type is set to nested
    // within NonDExpansion::construct_quadrature() for refinement cases
    expansionBasisType = Pecos::NODAL_INTERPOLANT;
    construct_quadrature(u_space_sampler, g_u_model, quad_order, dim_pref);
  }
  else if (ssg_level != USHRT_MAX) {
    switch (expansionBasisType) {
    case Pecos::HIERARCHICAL_INTERPOLANT:
      // Note: nestedRules not defined until construct_sparse_grid()
      if (ruleNestingOverride == Pecos::NON_NESTED) {
	Cerr << "Error: hierarchical interpolants currently require nested "
	     << "rules.  Please remove \"non_nested\" override." << std::endl;
	abort_handler(-1);
      }
      expansionCoeffsApproach = Pecos::HIERARCHICAL_SPARSE_GRID;
      break;
    case Pecos::NODAL_INTERPOLANT:
      expansionCoeffsApproach = (refineControl) ?
	Pecos::INCREMENTAL_SPARSE_GRID : Pecos::COMBINED_SPARSE_GRID;
      break;
    case Pecos::DEFAULT_BASIS:
      // hierarchical interpolation currently requires nested rules, which
      // are fairly limited outside of CC, F2, Gauss-Patterson --> use Nodal
      // as default unless conditions are just right.
      // Note: nestedRules not defined until construct_sparse_grid()
      if (u_space_type == STD_UNIFORM_U && refineControl &&
	  ruleNestingOverride != Pecos::NON_NESTED) {//TO DO: rm this constraint
	expansionCoeffsApproach = Pecos::HIERARCHICAL_SPARSE_GRID;
	expansionBasisType = Pecos::HIERARCHICAL_INTERPOLANT;
      }
      else {
	expansionCoeffsApproach = (refineControl) ?
	  Pecos::INCREMENTAL_SPARSE_GRID : Pecos::COMBINED_SPARSE_GRID;
	expansionBasisType = Pecos::NODAL_INTERPOLANT;
      }
      break;
    }
    /*
    if (refineControl == Pecos::LOCAL_ADAPTIVE_CONTROL) {
      if (!piecewiseBasis ||
          expansionBasisType != Pecos::HIERARCHICAL_INTERPOLANT) {
	// TO DO: promote this error check to resolve_inputs()
	PCerr << "Warning: overriding basis type to local hierarchical\n.";
	piecewiseBasis = true;
	expansionBasisType = Pecos::HIERARCHICAL_INTERPOLANT;
      }
      expansionCoeffsApproach = Pecos::HIERARCHICAL_SPARSE_GRID;
    }
    */
    construct_sparse_grid(u_space_sampler, g_u_model, ssg_level, dim_pref);
  }
}


void NonDStochCollocation::
config_integration(short exp_coeffs_approach, unsigned short num_int,
		   const RealVector& dim_pref, Iterator& u_space_sampler,
		   Model& g_u_model)
{
  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  switch (expansionCoeffsApproach) {
  case Pecos::QUADRATURE:
    expansionBasisType = Pecos::NODAL_INTERPOLANT;
    construct_quadrature(u_space_sampler, g_u_model, num_int, dim_pref);
    break;
  case Pecos::COMBINED_SPARSE_GRID: case Pecos::INCREMENTAL_SPARSE_GRID:
    expansionBasisType = Pecos::NODAL_INTERPOLANT;
    construct_sparse_grid(u_space_sampler, g_u_model, num_int, dim_pref);
    break;
  case Pecos::HIERARCHICAL_SPARSE_GRID:
    expansionBasisType = Pecos::HIERARCHICAL_INTERPOLANT;
    construct_sparse_grid(u_space_sampler, g_u_model, num_int, dim_pref);
    break;
  }
}


void NonDStochCollocation::config_approximation_type(String& approx_type)
{
  if (piecewiseBasis)
    approx_type = (expansionBasisType == Pecos::HIERARCHICAL_INTERPOLANT) ? 
      "piecewise_hierarchical_interpolation_polynomial" :
      "piecewise_nodal_interpolation_polynomial";
  else
    approx_type = (expansionBasisType == Pecos::HIERARCHICAL_INTERPOLANT) ?
      "global_hierarchical_interpolation_polynomial" :
      "global_nodal_interpolation_polynomial";
}


bool NonDStochCollocation::resize()
{
  bool parent_reinit_comms = NonDExpansion::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}


void NonDStochCollocation::
resolve_inputs(short& u_space_type, short& data_order)
{
  // perform first due to piecewiseBasis overrides
  NonDExpansion::resolve_inputs(u_space_type, data_order);

  // There are two derivative cases of interest: (1) derivatives used as
  // additional data for forming the approximation (derivatives w.r.t. the
  // expansion variables), and (2) derivatives that will be approximated 
  // separately (derivatives w.r.t. auxilliary variables).  The data_order
  // passed through the DataFitSurrModel defines Approximation::buildDataOrder,
  // which is restricted to managing the former case.  If we need to manage the
  // latter case in the future, we do not have a good way to detect this state
  // at construct time, as neither the finalStats ASV/DVV nor subIteratorFlag
  // have yet been defined.  One indicator that is defined is the presence of
  // inactive continuous vars, since the subModel inactive view is updated
  // within the NestedModel ctor prior to subIterator instantiation.
  data_order = 1;
  if (useDerivs) { // input specification
    if (iteratedModel.gradient_type()  != "none") data_order |= 2;
    //if (iteratedModel.hessian_type() != "none") data_order |= 4; // not yet
#ifdef ALLOW_GLOBAL_HERMITE_INTERPOLATION
    if (data_order == 1)
      Cerr << "\nWarning: use_derivatives option in stoch_collocation "
	   << "requires a response\n         gradient specification.  "
	   << "Option will be ignored.\n" << std::endl;
#else
    if (piecewiseBasis) {
      if (data_order == 1)
	Cerr << "\nWarning: use_derivatives option in stoch_collocation "
	     << "requires a response\n         gradient specification.  "
	     << "Option will be ignored.\n" << std::endl;
    }
    else {
      Cerr << "\nWarning: use of global gradient-enhanced interpolants is "
	   << "disallowed in production\n         executables.  To activate "
	   << "this research capability, define\n         ALLOW_GLOBAL_HERMITE_"
	   << "INTERPOLATION in Dakota::NonDStochCollocation and recompile.\n"
	   << std::endl;
      data_order = 1;
    }
#endif
  }
  useDerivs = (data_order > 1); // override input specification

  // override u_space_type to STD_UNIFORM_U for global Hermite interpolation
  if (useDerivs && !piecewiseBasis) {

    switch (u_space_type) {
    //case EXTENDED_U: // default; not user-selectable -> quiet default reassign
    //  break;
    case ASKEY_U: case PARTIAL_ASKEY_U: // non-default
      Cerr << "\nWarning: overriding transformation from ASKEY to STD_UNIFORM "
	   << "for Hermite interpolation.\n" << std::endl;
      break;
    case STD_NORMAL_U: // non-default
      Cerr << "\nWarning: overriding transformation from WIENER to STD_UNIFORM "
	   << "for Hermite interpolation.\n" << std::endl;
      break;
    }

    u_space_type = STD_UNIFORM_U;
  }
}


void NonDStochCollocation::initialize_u_space_model()
{
  NonDExpansion::initialize_u_space_model();
  configure_pecos_options(); // pulled out of base because C3 does not use it

  // initialize product accumulators with PolynomialApproximation pointers
  // used in covariance calculations
  if ( expansionBasisType == Pecos::HIERARCHICAL_INTERPOLANT &&
       refineControl && ( refineMetric == Pecos::COVARIANCE_METRIC ||
			  refineMetric == Pecos::MIXED_STATS_METRIC ) )
    initialize_covariance();

  // Precedes construct_basis() since basis is stored in Pecos driver
  SharedApproxData& shared_data = uSpaceModel.shared_approximation();
  shared_data.integration_iterator(uSpaceModel.subordinate_iterator());

  // DataFitSurrModel copies u-space mvDist from ProbabilityTransformModel
  const Pecos::MultivariateDistribution& u_mvd
    = uSpaceModel.multivariate_distribution();
  // construct the polynomial basis (shared by integration drivers)
  shared_data.construct_basis(u_mvd);
  // mainly a run-time requirement, but also needed at construct time
  // (e.g., to initialize NumericGenOrthogPolynomial::distributionType)
  //shared_data.update_basis_distribution_parameters(u_mvd);

  initialize_u_space_grid();
}


void NonDStochCollocation::initialize_covariance()
{
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  size_t i, j;
  for (i=0; i<numFunctions; ++i) {
    std::shared_ptr<PecosApproximation> pa_rep_i =
      std::static_pointer_cast<PecosApproximation>(
	poly_approxs[i].approx_rep());
    pa_rep_i->clear_covariance_pointers();
    for (j=0; j<=i; ++j)
      pa_rep_i->initialize_covariance(poly_approxs[j]);
  }
}


void NonDStochCollocation::compute_delta_mean(bool update_ref)
{
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  bool   warn_flag = false,
    combined_stats = (statsMetricMode == Pecos::COMBINED_EXPANSION_STATS);

  if (deltaRespMean.empty()) deltaRespMean.sizeUninitialized(numFunctions);
  for (size_t i=0; i<numFunctions; ++i) {
    std::shared_ptr<PecosApproximation> pa_rep_i =
      std::static_pointer_cast<PecosApproximation>(
	poly_approxs[i].approx_rep());
    if (pa_rep_i->expansion_coefficient_flag()) {
      if (combined_stats)
	// refinement assessed for impact on combined expansion from roll up
	deltaRespMean[i] = (allVars) ?
	  pa_rep_i->delta_combined_mean(initialPtU) :
	  pa_rep_i->delta_combined_mean();
      else // refinement assessed for impact on the current expansion
	deltaRespMean[i] = (allVars) ?
	  pa_rep_i->delta_mean(initialPtU) : pa_rep_i->delta_mean();

      if (update_ref) {
	if (combined_stats) {
	  Real new_mean = pa_rep_i->combined_moment(0) + deltaRespMean[i];
	  pa_rep_i->combined_moment(new_mean, 0);
	}
	else {
	  Real new_mean = pa_rep_i->moment(0) + deltaRespMean[i];
	  pa_rep_i->moment(new_mean, 0);
	}
      }
    }
    else
      { warn_flag = true; deltaRespMean[i] = 0.; }
  }

  // no current need for printing mean values by themselves:
  //if (print_metric) print_mean(Cout, deltaRespMean, "Change in");
  // mean values not tracked outside PolynomialApproximation:
  //if (update_ref)   respMean += deltaRespMean;
  if (warn_flag)
    Cerr << "Warning: expansion coefficients unavailable in NonD"
	 << "StochCollocation::compute_delta_mean().\n         "
	 << "Zeroing affected deltaRespMean terms." << std::endl;
}


void NonDStochCollocation::
compute_delta_variance(bool update_ref, bool print_metric)
{
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  bool   warn_flag = false,
    combined_stats = (statsMetricMode == Pecos::COMBINED_EXPANSION_STATS);

  if (deltaRespVariance.empty())
    deltaRespVariance.sizeUninitialized(numFunctions);
  for (size_t i=0; i<numFunctions; ++i) {
    std::shared_ptr<PecosApproximation> pa_rep_i =
      std::static_pointer_cast<PecosApproximation>
      (poly_approxs[i].approx_rep());
    Real& delta = deltaRespVariance[i];
    if (pa_rep_i->expansion_coefficient_flag()) {
      if (combined_stats)
	// refinement assessed for impact on combined expansion from roll up
	delta = (allVars) ? pa_rep_i->delta_combined_variance(initialPtU) :
	  pa_rep_i->delta_combined_variance();
      else // refinement assessed for impact on the current expansion
	delta = (allVars) ? pa_rep_i->delta_variance(initialPtU) :
	  pa_rep_i->delta_variance();

      if (update_ref) {
	respVariance[i] += delta;
	if (combined_stats) pa_rep_i->combined_moment(respVariance[i], 1);
	else                pa_rep_i->moment(respVariance[i], 1);
      }
    }
    else
      { warn_flag = true; delta = 0.; }
  }

  if (print_metric) print_variance(Cout, deltaRespVariance, "Change in");
  if (warn_flag)
    Cerr << "Warning: expansion coefficients unavailable in NonDStoch"
	 << "Collocation::compute_delta_variance().\n         "
	 << "Zeroing affected deltaRespVariance terms." << std::endl;
}


void NonDStochCollocation::
compute_delta_covariance(bool update_ref, bool print_metric)
{
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  bool   warn_flag = false,
    combined_stats = (statsMetricMode == Pecos::COMBINED_EXPANSION_STATS);
  size_t i, j;

  if (deltaRespCovariance.empty())
    deltaRespCovariance.shapeUninitialized(numFunctions);
  for (i=0; i<numFunctions; ++i) {
    std::shared_ptr<PecosApproximation> pa_rep_i =
      std::static_pointer_cast<PecosApproximation>
      (poly_approxs[i].approx_rep());
    if (pa_rep_i->expansion_coefficient_flag()) {
      for (j=0; j<=i; ++j) {
	Approximation& approx_j = poly_approxs[j];
	Real& delta = deltaRespCovariance(i,j);
	if (approx_j.expansion_coefficient_flag()) {
	  if (combined_stats)
	    // refinement assessed for impact on combined exp from roll up
	    delta = (allVars) ?
	      pa_rep_i->delta_combined_covariance(initialPtU, approx_j) :
	      pa_rep_i->delta_combined_covariance(approx_j);
	  else // refinement assessed for impact on the current expansion
	    delta = (allVars) ?
	      pa_rep_i->delta_covariance(initialPtU, approx_j) :
	      pa_rep_i->delta_covariance(approx_j);

	  if (update_ref) {
	    respCovariance(i,j) += delta;
	    if (i == j) {
	      if (combined_stats)
		pa_rep_i->combined_moment(respCovariance(i,i), 1);
	      else
		pa_rep_i->moment(respCovariance(i,i), 1);
	    }
	  }
	}
	else
	  { warn_flag = true; delta = 0.; }
      }
    }
    else {
      warn_flag = true;
      for (j=0; j<=i; ++j)
	deltaRespCovariance(i,j) = 0.;
    }
  }

  if (print_metric) print_covariance(Cout, deltaRespCovariance, "Change in");
  if (warn_flag)
    Cerr << "Warning: expansion coefficients unavailable in NonDStoch"
	 << "Collocation::compute_delta_covariance().\n         "
	 << "Zeroing affected deltaRespCovariance terms." << std::endl;
}


Real NonDStochCollocation::
compute_covariance_metric(bool revert, bool print_metric)
{
  if (expansionBasisType == Pecos::HIERARCHICAL_INTERPOLANT) {
    bool update_ref = !revert;

    // augment delta covar -> ref covar with delta_mean -> ref mean;
    // > supports base compute/print requirements without incurring overhead
    //   of metric_roll_up(), avoiding need to more broadly redefine
    //   compute_statistics(), print_results(), pull_*(), push_*() to
    //   exclude compute_moments()
    compute_delta_mean(update_ref);

    // Metric scale is determined from reference covariance.  While defining
    // the scale from an updated covariance would eliminate problems with zero
    // covariance for adaptations from level 0, different refinement candidates
    // would score equally at 1 (induced 100% of change in updated covariance)
    // in this initial set of candidates.  Therefore, use reference covariance
    // as the scale and mitigate underflow of its norm.
    Real scale, delta_norm;
    switch (covarianceControl) {
    case DIAGONAL_COVARIANCE:
      if (relativeMetric) // norm of reference variance, bounded from zero
	scale = std::max(Pecos::SMALL_NUMBER_SQ, respVariance.normFrobenius());
      compute_delta_variance(update_ref, print_metric);
      delta_norm = deltaRespVariance.normFrobenius();
      break;
    case FULL_COVARIANCE:
      if (relativeMetric) // norm of reference covariance, bounded from zero
	scale = std::max(Pecos::SMALL_NUMBER_SQ, respCovariance.normFrobenius());
      compute_delta_covariance(update_ref, print_metric);
      delta_norm = deltaRespCovariance.normFrobenius();
      break;
    }

    return (relativeMetric) ? delta_norm / scale : delta_norm;
  }
  else // use default implementation
    return NonDExpansion::compute_covariance_metric(revert, print_metric);
}


Real NonDStochCollocation::
compute_level_mappings_metric(bool revert, bool print_metric)
{
  // Focus only on level mappings and neglect moment deltas

  // combine delta_beta() and delta_z() from HierarchInterpPolyApproximation
  // with default definition of delta-{p,beta*}

  if (expansionBasisType == Pecos::HIERARCHICAL_INTERPOLANT) {

    // ensure moment updates for mixed stats:
    if (refineMetric == Pecos::MIXED_STATS_METRIC) {
      bool update_ref = !revert;
      compute_delta_mean(update_ref);
      switch (covarianceControl) {
      case DIAGONAL_COVARIANCE:
	compute_delta_variance(update_ref,   false);  break;
      case FULL_COVARIANCE:
	compute_delta_covariance(update_ref, false);  break;
      }
    }

    // Note: it would be desirable to include support for all active statistics,
    // including delta_mean() and delta_std_deviation().  With access to nested
    // response mappings passed down from an outer context, a more comprehensive
    // set of stats could be supported in the logic below.

    bool beta_map = false, numerical_map = false; size_t i, j, cntr;
    for (i=0; i<numFunctions; ++i) {
      if (!requestedRelLevels[i].empty()) beta_map = true;
      if (!requestedProbLevels[i].empty() || !requestedGenRelLevels[i].empty())
	numerical_map = true;
      if (!requestedRespLevels[i].empty()) {
	if (respLevelTarget == RELIABILITIES) beta_map = true;
	else                             numerical_map = true;
      }
    }
    if (beta_map) { // hierarchical increments in beta-bar->z and z-bar->beta

      size_t offset = 0;
      RealVector level_maps_ref, level_maps_new;
      pull_level_mappings(level_maps_ref, offset);
      if (numerical_map) { // merge in z-bar->p,beta* & p-bar,beta*-bar->z
	//metric_roll_up(REFINEMENT_RESULTS); // TO DO: support combined exp in numerical stats
	compute_numerical_level_mappings();
	pull_level_mappings(level_maps_new, offset);// analytic overlaid at end
	deltaLevelMaps = level_maps_new;  deltaLevelMaps -= level_maps_ref;
      }
      else {
        deltaLevelMaps.size(totalLevelRequests);              // init to 0
        if (!revert) level_maps_new.size(totalLevelRequests); // init to 0
      }

      bool warn_flag   = false,
	combined_stats = (statsMetricMode == Pecos::COMBINED_EXPANSION_STATS);
      std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
      Real delta, ref, sum_sq = 0., scale_sq = 0., z_bar, beta_bar;
      for (i=0, cntr=0; i<numFunctions; ++i) {
	size_t rl_len = requestedRespLevels[i].length(),
	       pl_len = requestedProbLevels[i].length(),
	       bl_len = requestedRelLevels[i].length(),
	       gl_len = requestedGenRelLevels[i].length();
	std::shared_ptr<PecosApproximation> pa_rep_i =
	  std::static_pointer_cast<PecosApproximation>
	  (poly_approxs[i].approx_rep());
	if (pa_rep_i->expansion_coefficient_flag()) {
	  if (respLevelTarget == RELIABILITIES)
	    for (j=0; j<rl_len; ++j, ++cntr) {
	      z_bar = requestedRespLevels[i][j];
	      if (combined_stats)
		delta = deltaLevelMaps[cntr] = (allVars) ?
		  pa_rep_i->delta_combined_beta(initialPtU, cdfFlag, z_bar) :
		  pa_rep_i->delta_combined_beta(cdfFlag, z_bar);
	      else
		delta = deltaLevelMaps[cntr] = (allVars) ?
		  pa_rep_i->delta_beta(initialPtU, cdfFlag, z_bar) :
		  pa_rep_i->delta_beta(cdfFlag, z_bar);
	      sum_sq += delta * delta;
	      // avoid proliferating numerical exception:
	      ref = level_maps_ref[cntr];
	      if (std::abs(ref) == Pecos::LARGE_NUMBER) {
		// ref is undefined and delta neglects term;
		// recompute new w/o delta in analytic_delta_level_mappings()

		// do not increment scale for dummy beta value --> may result
		// in SMALL_NUMBER scaling below if no meaningful refs exist
	      }
	      else if (relativeMetric)
		scale_sq += ref * ref;// ref,delta are valid --> update scale
	    }
	  else
	    for (j=0; j<rl_len; ++j, ++cntr) {
	      delta = deltaLevelMaps[cntr]; sum_sq += delta * delta;
	      if (relativeMetric)
		{ ref = level_maps_ref[cntr]; scale_sq += ref * ref; }
	    }
	  for (j=0; j<pl_len; ++j, ++cntr) {
	    delta = deltaLevelMaps[cntr]; sum_sq += delta * delta;
	    if (relativeMetric)
	      { ref = level_maps_ref[cntr]; scale_sq += ref * ref; }
	  }
	  for (j=0; j<bl_len; ++j, ++cntr) {
	    beta_bar = requestedRelLevels[i][j];
	    if (combined_stats)
	      delta = deltaLevelMaps[cntr] = (allVars) ?
		pa_rep_i->delta_combined_z(initialPtU, cdfFlag, beta_bar) :
		pa_rep_i->delta_combined_z(cdfFlag, beta_bar);
	    else
	      delta = deltaLevelMaps[cntr] = (allVars) ?
		pa_rep_i->delta_z(initialPtU, cdfFlag, beta_bar) :
		pa_rep_i->delta_z(cdfFlag, beta_bar);
	    sum_sq += delta * delta;
	    ref = level_maps_ref[cntr];
	    if (relativeMetric) scale_sq += ref * ref;
	  }
	  for (j=0; j<gl_len; ++j, ++cntr) {
	    delta = deltaLevelMaps[cntr]; sum_sq += delta * delta;
	    if (relativeMetric)
	      { ref = level_maps_ref[cntr]; scale_sq += ref * ref; }
	  }
	}
	else {
	  warn_flag = true;
	  cntr += rl_len + pl_len + bl_len + gl_len;
	}
      }
      if (warn_flag)
	Cerr << "Warning: expansion coefficients unavailable in "
	     << "NonDStochCollocation::compute_level_mappings_metric().\n"
	     << "         Omitting affected level mappings." << std::endl;
      // As for compute_delta_covariance(), print level mapping deltas
      // (without a moment offset as for final_stats):
      if (print_metric)
	print_level_mappings(Cout, deltaLevelMaps, false, "Change in");

      // Level mappings: promote to new or revert to previous (if required)
      if (!revert) { // retain updated values
        analytic_delta_level_mappings(level_maps_ref, level_maps_new);
	push_level_mappings(level_maps_new, offset);
      }
      else if (numerical_map) // restore ref values that were overwritten
	push_level_mappings(level_maps_ref, offset); // restore reference
      //else deltaLevelMaps does not impact existing level mappings

      // Metric scale is determined from reference stats, not updated stats,
      // as consistent with compute_covariance_metric() above.
      if (relativeMetric) {
	Real scale = std::max(Pecos::SMALL_NUMBER, std::sqrt(scale_sq));
	return std::sqrt(sum_sq) / scale;
      }
      else
	return std::sqrt(sum_sq);
    }
    else // use default implementation if no beta-mapping increments
      return
	NonDExpansion::compute_level_mappings_metric(revert, print_metric);
  }
  else // use default implementation for Nodal
    return NonDExpansion::compute_level_mappings_metric(revert, print_metric);
}


/*
Real NonDStochCollocation::
compute_final_statistics_metric(bool revert, bool print_metric)
{
  // Focus only on level mappings and neglect moment deltas

  // combine delta_beta() and delta_z() from HierarchInterpPolyApproximation
  // with default definition of delta-{p,beta*}

  if (expansionBasisType == Pecos::HIERARCHICAL_INTERPOLANT) {
    // Note: it would be desirable to include support for all active statistics,
    // including delta_mean() and delta_std_deviation().  With access to nested
    // response mappings passed down from an outer context, a more comprehensive
    // set of stats could be supported in the logic below.
    bool beta_map = false, numerical_map = false,
      combined_stats = (statsMetricMode == Pecos::COMBINED_EXPANSION_STATS);
    size_t i, j, cntr;
    for (i=0; i<numFunctions; ++i) {
      if (!requestedRelLevels[i].empty()) beta_map = true;
      if (!requestedProbLevels[i].empty() || !requestedGenRelLevels[i].empty())
	numerical_map = true;
      if (!requestedRespLevels[i].empty()) {
	if (respLevelTarget == RELIABILITIES) beta_map = true;
	else                             numerical_map = true;
      }
    }
    if (beta_map) { // hierarchical increments in beta-bar->z and z-bar->beta

      RealVector delta_final_stats, final_stats_new,
	final_stats_ref = finalStatistics.function_values();
      // *** Note: this requires that the reference includes FINAL_RESULTS,
      // *** which is not currently true (only INTERMEDIATE_RESULTS)
      if (numerical_map) { // merge in z-bar->p,beta* & p-bar,beta*-bar->z
	compute_statistics(FINAL_RESULTS);//no finalStats for REFINEMENT_RESULTS
	delta_final_stats = final_stats_new = finalStatistics.function_values();
	delta_final_stats -= final_stats_ref; // compute delta
      }
      else {
	size_t num_stats = finalStatistics.num_functions();
        delta_final_stats.size(num_stats);            // init to 0
        if (!revert) final_stats_new.size(num_stats); // init to 0
      }

      bool warn_flag = false;
      std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
      Real delta, ref, sum_sq = 0., scale_sq = 0., z_bar, beta_bar;
      for (i=0, cntr=0; i<numFunctions; ++i) {
	size_t rl_len = requestedRespLevels[i].length(),
	       pl_len = requestedProbLevels[i].length(),
	       bl_len = requestedRelLevels[i].length(),
	       gl_len = requestedGenRelLevels[i].length();
	std::shared_ptr<PecosApproximation> pa_rep_i =
	  std::static_pointer_cast<PecosApproximation>
	  (poly_approxs[i].approx_rep());
	cntr += moment_offset; // skip moments if present
	if (pa_rep_i->expansion_coefficient_flag()) {
	  if (respLevelTarget == RELIABILITIES)
	    for (j=0; j<rl_len; ++j, ++cntr) {
	      z_bar = requestedRespLevels[i][j];
	      if (combined_stats)
		delta = delta_final_stats[cntr] = (allVars) ?
		  pa_rep_i->delta_combined_beta(initialPtU, cdfFlag, z_bar) :
		  pa_rep_i->delta_combined_beta(cdfFlag, z_bar);
	      else
		delta = delta_final_stats[cntr] = (allVars) ?
		  pa_rep_i->delta_beta(initialPtU, cdfFlag, z_bar) :
		  pa_rep_i->delta_beta(cdfFlag, z_bar);
	      sum_sq += delta * delta;
	      ref = final_stats_ref[cntr];
	      // Note: this captures the more likely of the Pecos::
	      // HierarchInterpPolyApproximation::delta_beta_map() exceptions
	      // (sigma_ref = 0), but not rare case of sigma_new = 0 by itself.
	      if (std::abs(ref) == Pecos::LARGE_NUMBER) {
		// ref is undefined and delta neglects term; must compute new
		if (!revert) {
		  if (combined_stats)
		    final_stats_new[cntr] = (allVars) ?
		      pa_rep_i->combined_beta(initialPtU, cdfFlag, z_bar) :
		      pa_rep_i->combined_beta(cdfFlag, z_bar);
		  else
		    final_stats_new[cntr] = (allVars) ?
		      pa_rep_i->beta(initialPtU, cdfFlag, z_bar) :
		      pa_rep_i->beta(cdfFlag, z_bar);
		}
		// do not increment scale for dummy beta value --> may result
		// in SMALL_NUMBER scaling below if no meaningful refs exist
	      }
	      else { // ref and delta are valid --> update scale and new
		if (relativeMetric) scale_sq += ref * ref;
		if (!revert) final_stats_new[cntr] = ref + delta;
	      }
	    }
	  else
	    for (j=0; j<rl_len; ++j, ++cntr) {
	      delta = delta_final_stats[cntr]; sum_sq += delta * delta;
	      if (relativeMetric)
		{ ref = final_stats_ref[cntr]; scale_sq += ref * ref; }
	    }
	  for (j=0; j<pl_len; ++j, ++cntr) {
	    delta = delta_final_stats[cntr]; sum_sq += delta * delta;
	    if (relativeMetric)
	      { ref = final_stats_ref[cntr]; scale_sq += ref * ref; }
	  }
	  for (j=0; j<bl_len; ++j, ++cntr) {
	    beta_bar = requestedRelLevels[i][j];
	    if (combined_stats)
	      delta = delta_final_stats[cntr] = (allVars) ?
		pa_rep_i->delta_combined_z(initialPtU, cdfFlag, beta_bar) :
		pa_rep_i->delta_combined_z(cdfFlag, beta_bar);
	    else
	      delta = delta_final_stats[cntr] = (allVars) ?
		pa_rep_i->delta_z(initialPtU, cdfFlag, beta_bar) :
		pa_rep_i->delta_z(cdfFlag, beta_bar);
	    sum_sq += delta * delta;
	    ref = final_stats_ref[cntr];
	    if (relativeMetric) scale_sq += ref * ref;
	    if (!revert) final_stats_new[cntr] = ref + delta;
	  }
	  for (j=0; j<gl_len; ++j, ++cntr) {
	    delta = delta_final_stats[cntr]; sum_sq += delta * delta;
	    if (relativeMetric)
	      { ref = final_stats_ref[cntr]; scale_sq += ref * ref; }
	  }
	}
	else {
	  warn_flag = true;
	  cntr += rl_len + pl_len + bl_len + gl_len;
	}
      }
      if (warn_flag)
	Cerr << "Warning: expansion coefficients unavailable in "
	     << "NonDStochCollocation::compute_final_statistics_metric().\n"
	     << "         Omitting affected final statistics." << std::endl;

      // As for compute_delta_covariance(), print level mapping deltas:
      if (print_metric) {
        bool moments = (finalMomentsType > Pecos::NO_MOMENTS);
	print_level_mappings(Cout, delta_final_stats, moments, "Change in");
      }
      // Final stats: revert to previous or promote to new
      if (!revert)
	finalStatistics.function_values(final_stats_new);
      else if (numerical_map) // revert from final_stats_new to final_stats_ref
	finalStatistics.function_values(final_stats_ref);

      // Metric scale is determined from reference stats, not updated stats,
      // as consistent with compute_covariance_metric() above.
      if (relativeMetric) {
	Real scale = std::max(Pecos::SMALL_NUMBER, std::sqrt(scale_sq));
	return std::sqrt(sum_sq) / scale;
      }
      else
	return std::sqrt(sum_sq);
    }
    else // use default implementation if no beta-mapping increments
      return
	NonDExpansion::compute_final_statistics_metric(revert, print_metric);
  }
  else // use default implementation for Nodal
    return NonDExpansion::compute_final_statistics_metric(revert, print_metric);
}
*/


/* Calculate analytic and numerical statistics from the expansion and
    log results within final_stats for use in OUU.
void NonDStochCollocation::compute_statistics(short results_state)
{
  if (expansionBasisType == Pecos::HIERARCHICAL_INTERPOLANT)
    // specialize base implementation since metric_roll_up() is not used and
    // extra stats that require roll-up no longer come for free
    switch (results_state) {
    case REFINEMENT_RESULTS:
      // compute_{covariance,level_mapping,final_statistics}_metric() performs
      // only the necessary computations for resolving delta.norm()

      // augment delta covar -> ref covar with delta_mean -> ref mean;
      // > supports base compute/print requirements without incurring overhead
      //   of metric_roll_up(), avoiding need to more broadly redefine
      //   compute_statistics(), print_results(), pull_*(), push_*() to
      //   exclude compute_moments()
      compute_delta_mean(true); // update ref
      break;
    //case INTERMEDIATE_RESULTS:  case FINAL_RESULTS:  break;
    }

  NonDExpansion::compute_statistics(results_state);
}


void NonDStochCollocation::pull_candidate(RealVector& stats_star)
{
  if (expansionBasisType == Pecos::HIERARCHICAL_INTERPOLANT)
    // If pulling updated values as in NonDExpansion
    switch (refineMetric) {
    case Pecos::COVARIANCE_METRIC: {
      std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
      std::shared_ptr<PecosApproximation> poly_approx_rep;
      bool full_covar = (covarianceControl == FULL_COVARIANCE),
        combined_stats = (statsMetricMode == Pecos::COMBINED_EXPANSION_STATS);
      size_t vec_len = (full_covar) ?
	(numFunctions*(numFunctions + 3))/2 : 2*numFunctions;
      if (stats_star.length() != vec_len) stats_star.sizeUninitialized(vec_len);
      // pull means
      for (size_t i=0; i<numFunctions; ++i) {
	poly_approx_rep =
	std::static_pointer_cast<PecosApproximation>(
	  poly_approxs[i].approx_rep());
	stats_star[i] = (combined_stats) ?
	  poly_approx_rep->combined_moment(0) + deltaRespMean[i] :
	  poly_approx_rep->moment(0)          + deltaRespMean[i];
      }
      // pull resp{V,Cov}ariance
      if (full_covar) {
	RealSymMatrix new_covar(respCovariance);
	new_covar += deltaRespCovariance;
	pull_lower_triangle(new_covar, stats_star, numFunctions);
      }
      else
	for (size_t i=0; i<numFunctions; ++i)
	  stats_star[i+numFunctions] = respVariance[i] + deltaRespVariance[i];
      break;
    }
    default:
      // define an offset for MIXED_STATS_METRIC
      pull_level_mappings(stats_star, offset); // pull updated numerical stats
      analytic_delta_level_mappings(stats_star, stats_star); // update analytic
                 // (stats_star provides ref and becomes new for these entries)
      break;
    }
  else
    NonDExpansion::pull_candidate(stats_star);
}
*/


/** In this function, we leave numerical stats alone, updating
    analytic level stats either using ref+delta or, if ref is invalid,
    though recomputation. */
void NonDStochCollocation::
analytic_delta_level_mappings(const RealVector& level_maps_ref,
			      RealVector& level_maps_new)
{
  // preserve existing as this may be an overlay
  if (level_maps_new.length() != totalLevelRequests)
    level_maps_new.resize(totalLevelRequests);

  size_t i, j, cntr, rl_len, pl_len, bl_len, gl_len, pl_bl_gl_len;
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  Real delta, ref, sum_sq = 0., scale_sq = 0., z_bar, beta_bar;
  bool combined_stats = (statsMetricMode == Pecos::COMBINED_EXPANSION_STATS);
  for (i=0, cntr=0; i<numFunctions; ++i) {
    rl_len = requestedRespLevels[i].length();
    pl_len = requestedProbLevels[i].length();
    bl_len = requestedRelLevels[i].length();
    gl_len = requestedGenRelLevels[i].length();
    pl_bl_gl_len = pl_len+bl_len+gl_len;
    std::shared_ptr<PecosApproximation> pa_rep_i =
      std::static_pointer_cast<PecosApproximation>(
	poly_approxs[i].approx_rep());
    if (respLevelTarget == RELIABILITIES)
      for (j=0; j<rl_len; ++j, ++cntr) {
	// Note: this captures the more likely of the Pecos::
	// HierarchInterpPolyApproximation::delta_beta_map() exceptions
	// (sigma_ref = 0), but not rare case of sigma_new = 0 by itself.
	ref = level_maps_ref[cntr];
	if (std::abs(ref) == Pecos::LARGE_NUMBER) {
	  // ref is undefined and delta neglects term; must compute new
	  z_bar = requestedRespLevels[i][j];
	  if (combined_stats)
	    level_maps_new[cntr] = (allVars) ?
	      pa_rep_i->combined_beta(initialPtU, cdfFlag, z_bar) :
	      pa_rep_i->combined_beta(cdfFlag, z_bar);
	  else
	    level_maps_new[cntr] = (allVars) ?
	      pa_rep_i->beta(initialPtU, cdfFlag, z_bar) :
	      pa_rep_i->beta(cdfFlag, z_bar);
	}
	else // ref and delta are valid
	  level_maps_new[cntr] = ref + deltaLevelMaps[cntr];
      }
    else
      cntr += rl_len;
    cntr += pl_len;
    for (j=0; j<bl_len; ++j)
      level_maps_new[cntr] = level_maps_ref[cntr] + deltaLevelMaps[cntr];
    cntr += gl_len;
  }
}

} // namespace Dakota
