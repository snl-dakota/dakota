/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDC3FunctionTrain
//- Description: Implementation code for NonDC3FunctionTrain class
//- Owner:       Alex Gorodetsky

#include "dakota_system_defs.hpp"
#include "NonDC3FunctionTrain.hpp"
#include "NonDIntegration.hpp"
#include "C3Approximation.hpp"
#include "SharedC3ApproxData.hpp"
#include "DataFitSurrModel.hpp"
#include "ProbabilityTransformModel.hpp"
#include "DakotaResponse.hpp"
#include "ProblemDescDB.hpp"
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
//#define DEBUG


namespace Dakota {

//NonDC3FunctionTrain* NonDC3FunctionTrain::c3Instance(NULL);

struct SPrintArgs
{
  std::ostream *s;
  StringMultiArrayConstView * cv_labels;
  double variance;
};


/** This constructor is called for a standard letter-envelope iterator
    instantiation using the ProblemDescDB. */
NonDC3FunctionTrain::
NonDC3FunctionTrain(ProblemDescDB& problem_db, Model& model):
  NonDExpansion(problem_db, model),
  importBuildPointsFile(
    problem_db.get_string("method.import_build_points_file")),
  startRankSpec(
    problem_db.get_sizet("method.nond.c3function_train.start_rank")),
  maxRankSpec(probDescDB.get_sizet("method.nond.c3function_train.max_rank")),
  startOrderSpec(
    problem_db.get_ushort("method.nond.c3function_train.start_order")),
  maxOrderSpec(probDescDB.get_ushort("method.nond.c3function_train.max_order")),
  collocPtsSpec(problem_db.get_sizet("method.nond.collocation_points")),
  c3RefineType(UNIFORM_MAX_RANK) // hardwired prior to XML spec option
{
  if (iteratedModel.model_type()     == "surrogate" &&
      iteratedModel.surrogate_type() == "global_function_train") {
    Cerr << "Error: use 'surrogate_based_uq' for UQ using a Model-based "
	 << "function train specification." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // ----------------
  // Resolve settings
  // ----------------
  short data_order;
  // See SharedC3ApproxData::construct_basis().  C3 won't support STD_{BETA,
  // GAMMA,EXPONENTIAL} so use PARTIAL_ASKEY_U to map to STD_{NORMAL,UNIFORM}.
  short u_space_type = PARTIAL_ASKEY_U;//probDescDB.get_short("method.nond.expansion_type");
  resolve_inputs(u_space_type, data_order);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  g_u_model.assign_rep(new ProbabilityTransformModel(iteratedModel,
    u_space_type), false); // retain dist bnds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  Iterator u_space_sampler; // evaluates truth model
  UShortArray approx_orders;
  configure_expansion_orders(startOrderSpec, dimPrefSpec, approx_orders);
  // compute initial regression size using a static helper
  // (uSpaceModel.shared_approximation() is not yet available)
  size_t rank = (refineType && c3RefineType == UNIFORM_MAX_RANK)
              ? maxRankSpec : startRankSpec;
  size_t regress_size = SharedC3ApproxData::regression_size(numContinuousVars,
    rank, maxRankSpec, approx_orders, maxOrderSpec);
  // configure u-space sampler and model
  if (!config_regression(collocPtsSpec, regress_size, randomSeed,
			 u_space_sampler, g_u_model)) {
    Cerr << "Error: incomplete configuration in NonDC3FunctionTrain "
	 << "constructor." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for SCBDO with polynomials over {u}+{d}, change view to All.
  short corr_order = -1, corr_type = NO_CORRECTION;
  String pt_reuse = probDescDB.get_string("method.nond.point_reuse");
  if (!importBuildPointsFile.empty() && pt_reuse.empty())
    pt_reuse = "all"; // reassign default if data import
  String approx_type = "global_function_train";
  ActiveSet ft_set = g_u_model.current_response().active_set(); // copy
  ft_set.request_values(3); // stand-alone mode: surrogate grad evals at most
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
    ft_set, approx_type, approx_orders, corr_type, corr_order, data_order,
    outputLevel, pt_reuse, importBuildPointsFile,
    probDescDB.get_ushort("method.import_build_format"),
    probDescDB.get_bool("method.import_build_active_only"),
    probDescDB.get_string("method.export_approx_points_file"),
    probDescDB.get_ushort("method.export_approx_format")), false);
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
}


/** This constructor is called by derived class constructors. */
NonDC3FunctionTrain::
NonDC3FunctionTrain(unsigned short method_name, ProblemDescDB& problem_db,
		    Model& model):
  NonDExpansion(problem_db, model),
  importBuildPointsFile(
    problem_db.get_string("method.import_build_points_file")),
  startRankSpec(
    problem_db.get_sizet("method.nond.c3function_train.start_rank")),
  maxRankSpec(probDescDB.get_sizet("method.nond.c3function_train.max_rank")),
  startOrderSpec(
    problem_db.get_ushort("method.nond.c3function_train.start_order")),
  maxOrderSpec(probDescDB.get_ushort("method.nond.c3function_train.max_order")),
  collocPtsSpec(0), // in lieu of sequence specification
  c3RefineType(UNIFORM_MAX_RANK) // hardwired prior to XML spec option
{
  if (iteratedModel.model_type()     == "surrogate" &&
      iteratedModel.surrogate_type() == "global_function_train") {
    Cerr << "Error: use 'surrogate_based_uq' for UQ using a Model-based "
	 << "function train specification." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // Rest is in derived class...
}


NonDC3FunctionTrain::~NonDC3FunctionTrain()
{ }


void NonDC3FunctionTrain::
resolve_inputs(short& u_space_type, short& data_order)
{
  NonDExpansion::resolve_inputs(u_space_type, data_order);

  // May eventually manage different transformation options, but hardwired
  // to PARTIAL_ASKEY_U for now

  data_order = 1; // no deriv enhancement for now...
}


bool NonDC3FunctionTrain::
config_regression(size_t colloc_pts, size_t regress_size, int seed,
		  Iterator& u_space_sampler, Model& g_u_model)
{
  // Adapted from NonDPolynomialChaos::config_regression()

  // Note: colloc_pts and regress_size are passed so that they can be defined
  // either from scalar (this class) or sequence (derived class) specifications

  // given regression size, either compute numSamplesOnModel from collocRatio
  // or vice versa
  if (colloc_pts != std::numeric_limits<size_t>::max()) {
    numSamplesOnModel = colloc_pts;
    if (collocRatio == 0.) // default (no user spec)
      collocRatio = 2.; // assign method-specific default for sample refinement
    // Another option: infer from pilot (terms_samples_to_ratio()) & fix for
    // iters to follow (may require averaging for shaped pilot/order/rank)
  }
  else if (collocRatio > 0.) // define colloc pts from collocRatio
    numSamplesOnModel = terms_ratio_to_samples(regress_size, collocRatio);
  else
    return false;

  // given numSamplesOnModel, configure u_space_sampler
  if (probDescDB.get_bool("method.nond.tensor_grid")) {
    // structured grid: uniform sub-sampling of TPQ
    UShortArray dim_quad_order
      = probDescDB.get_usa("method.nond.tensor_grid_order"); // copy
    Pecos::inflate_scalar(dim_quad_order, numContinuousVars);
    // convert aniso vector to scalar + dim_pref.  If iso, dim_pref is
    // empty; if aniso, it differs from exp_order aniso due to offset.
    unsigned short quad_order; RealVector dim_pref;
    NonDIntegration::anisotropic_order_to_dimension_preference(dim_quad_order,
      quad_order, dim_pref);
    // use alternate NonDQuad ctor to filter (deprecated) or sub-sample
    // quadrature points (uSpaceModel.build_approximation() invokes
    // daceIterator.run()).  The quad order inputs are updated within
    // NonDQuadrature as needed to satisfy min order constraints (but
    // not nested constraints: nestedRules is false to retain m >= p+1).
    construct_quadrature(u_space_sampler, g_u_model, quad_order, dim_pref,
			 numSamplesOnModel, seed);
  }
  else { // unstructured grid: LHS samples
    // if reusing samples within a refinement strategy, we prefer generating
    // different random numbers for new points within the grid (even if the
    // number of samples differs)
    // Note: uniform refinement uses DFSModel::rebuild_approximation()
    // which directly computes sample increment
    // *** TO DO: would be good to disntinguish top-level seed fixing for OUU
    //            from lower-level seed fixing across levels or refine iters.
    if (refineType && fixedSeed)
      Cerr << "Warning: combining sample refinement with fixed_seed is more "
	   << "likely to cause sample redundancy." << std::endl;
    // reuse type/seed/rng settings intended for the expansion_sampler.
    // Unlike expansion_sampler, allow sampling pattern to vary under
    // unstructured grid refinement/replacement/augmentation.  Also
    // unlike expansion_sampler, we use an ACTIVE sampler mode for
    // forming the PCE over all active variables.
    construct_lhs(u_space_sampler, g_u_model,
		  probDescDB.get_ushort("method.sample_type"),
		  numSamplesOnModel, seed,
		  probDescDB.get_string("method.random_number_generator"),
		  !fixedSeed, ACTIVE);
  }

  // maxEvalConcurrency updated here for expansion samples and regression
  // and in initialize_u_space_model() for sparse/quad/cub
  maxEvalConcurrency *= numSamplesOnModel;
  return true;
}


void NonDC3FunctionTrain::initialize_u_space_model()
{
  NonDExpansion::initialize_u_space_model();
  //configure_pecos_options(); // C3 does not use Pecos options

  // needs to precede construct_basis()
  push_c3_core_rank(startRankSpec);
  UShortArray approx_orders;
  configure_expansion_orders(startOrderSpec, dimPrefSpec, approx_orders);
  push_c3_core_orders(approx_orders);
  push_c3_seed(randomSeed);
  push_c3_db_options();

  // SharedC3ApproxData invokes ope_opts_alloc() to construct basis
  const Pecos::MultivariateDistribution& u_dist
    = uSpaceModel.truth_model().multivariate_distribution();
  uSpaceModel.shared_approximation().construct_basis(u_dist);
}


void NonDC3FunctionTrain::push_c3_core_rank(size_t start_rank)
{
  // rank is passed in since they may be a scalar or part of a sequence:
  SharedC3ApproxData* shared_data_rep = (SharedC3ApproxData*)
    uSpaceModel.shared_approximation().data_rep();
  shared_data_rep->set_parameter("start_rank", start_rank);
}


void NonDC3FunctionTrain::push_c3_core_orders(const UShortArray& start_orders)
{
  // These are passed in since they may be a scalar or part of a sequence:
  SharedC3ApproxData* shared_data_rep = (SharedC3ApproxData*)
    uSpaceModel.shared_approximation().data_rep();
  shared_data_rep->set_parameter("start_poly_order", start_orders);
}


void NonDC3FunctionTrain::push_c3_seed(int seed)
{
  // These are passed in since they may be a scalar or part of a sequence:
  SharedC3ApproxData* shared_data_rep = (SharedC3ApproxData*)
    uSpaceModel.shared_approximation().data_rep();
  shared_data_rep->set_parameter("random_seed", seed);
}


void NonDC3FunctionTrain::push_c3_db_options()
{
  // Commonly used approx settings (e.g., basis orders, outputLevel, useDerivs)
  // are passed through the DataFitSurrModel ctor chain.  Additional options
  // are passed here.

  SharedC3ApproxData* shared_data_rep = (SharedC3ApproxData*)
    uSpaceModel.shared_approximation().data_rep();

  // These are pulled from the DB as they are always scalars:
  shared_data_rep->set_parameter("max_poly_order", maxOrderSpec);
  shared_data_rep->set_parameter("kick_rank",
    probDescDB.get_sizet("method.nond.c3function_train.kick_rank"));
  shared_data_rep->set_parameter("max_rank", maxRankSpec);
  shared_data_rep->set_parameter("adapt_rank",
    probDescDB.get_bool("method.nond.c3function_train.adapt_rank"));
  shared_data_rep->set_parameter("regress_type",
    probDescDB.get_short("method.nond.regression_type"));
  shared_data_rep->set_parameter("regularization_parameter",
    probDescDB.get_real("method.nond.regression_penalty"));
  shared_data_rep->set_parameter("solver_tol",
    probDescDB.get_real("method.nond.c3function_train.solver_tolerance"));
  shared_data_rep->set_parameter("rounding_tol",
    probDescDB.get_real("method.nond.c3function_train.rounding_tolerance"));
  shared_data_rep->set_parameter("arithmetic_tol",
    probDescDB.get_real("method.nond.c3function_train.arithmetic_tolerance"));
  shared_data_rep->set_parameter("max_cross_iterations",
    probDescDB.get_int("method.nond.c3function_train.max_cross_iterations"));
  shared_data_rep->set_parameter("max_solver_iterations",
    probDescDB.get_int("method.nond.max_solver_iterations"));

  short comb_type = Pecos::ADD_COMBINE;
  int   verbosity = (outputLevel > NORMAL_OUTPUT) ? 1 : 0;
  shared_data_rep->set_parameter("combine_type",     comb_type);
  shared_data_rep->set_parameter("verbosity",        verbosity);

  shared_data_rep->set_parameter("discrepancy_type", multilevDiscrepEmulation);
  shared_data_rep->set_parameter("alloc_control",    multilevAllocControl);

  shared_data_rep->set_parameter("refinement_type",  c3RefineType); 
}


bool NonDC3FunctionTrain::advancement_available()
{
  // if there are new samples, then exp coeff updates are always performed
  // > Need to query the DataFitSurrModel for increment computed by
  //   rebuild_global(), or perhaps query the state of the approxData
  //if (numSamplesOnModel) // Note: this is updated total
  //  return true;

  bool refine = false;
  SharedC3ApproxData* shared_data_rep = (SharedC3ApproxData*)
    uSpaceModel.shared_approximation().data_rep();
  switch (c3RefineType) {
  // these two options only require the shared data config:
  case UNIFORM_START_ORDER: {
    const UShortArray& s_ord = shared_data_rep->start_orders();// adapted orders
    size_t i, num_ord = s_ord.size();
    for (size_t i=0; i<num_ord; ++i)
      if (s_ord[i] < maxOrderSpec)
	{ refine = true; break; }
    break;
  }
  case UNIFORM_START_RANK:
    if (shared_data_rep->start_rank() < maxRankSpec)
      refine = true;
    break;
  // this option must query recovered ranks for each of the C3Approximations:
  case UNIFORM_MAX_RANK:
    if (!shared_data_rep->adapt_rank()) {
      Cerr << "Error: adapt_rank required for UNIFORM_MAX_RANK refinement."
	   << std::endl;
      abort_handler(METHOD_ERROR);
    }
    size_t v, max_rank = shared_data_rep->max_rank(); // adapted value
    std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
    for (size_t qoi=0; qoi<numFunctions; ++qoi) {
      C3Approximation* poly_approx_q
	= (C3Approximation*)poly_approxs[qoi].approx_rep();
      // check adapted FT ranks against maxRank.  Notes:
      // > logic is inverted when advancing the bound i/o the bounded quantity
      // > logic must be augmented with an outer check on numSamples:
      //   refinement is possible for either a sample increment or an advanced
      //   bound that could admit a different adapt_rank soln for fixed data
      SizetVector ft_ranks = poly_approx_q->function_train_ranks();
      for (v=1; v<numContinuousVars; ++v) // ranks len = num_v+1 with 1's @ ends
	if (ft_ranks[v] == max_rank) // recovery potentially limited by bound
	  { refine = true; break; }
      if (refine) break;
    }
    break;
  }

  return refine;
}


void NonDC3FunctionTrain::push_increment()
{
  // Reverse order relative to NonDExpansion base implementation since
  // required sample resolution is inferred from the state of the FT

  uSpaceModel.push_approximation(); // uses reference in append_tensor_exp
  increment_grid(false); // don't recompute anisotropy
}


void NonDC3FunctionTrain::update_samples_from_order_increment()
{
  // Given a candidate advancement since the last solve (e.g., NonDExpansion::
  // increment_order_and_grid()), we can't just rely on size of the last solve
  // > sample_allocation_metric() uses recovered QoI ranks + order advancements
  // > collocRatio is then applied to number of unknowns within floor() below
  Real pow_mean_qoi_regress;
  sample_allocation_metric(pow_mean_qoi_regress, 2.);//DBL_MAX);

  // This function computes an update to the total points.  The increment
  // induced relative to the current data set is managed in DataFitSurrModel::
  // rebuild_global())
  //prevSamplesOnModel = numSamplesOnModel;//requires level mgmt for persistence
  numSamplesOnModel = (int)std::floor(collocRatio * pow_mean_qoi_regress + .5);
}


/** inconvenient to recompute: store previous samples rather than
    previous ranks
void NonDC3FunctionTrain::update_samples_from_order_decrement()
{ numSamplesOnModel = prevSamplesOnModel; }//requires level mgmt for persistence
*/


/* Compute power mean of regression size (common power values: 1 = average size,
   2 = root mean square size, DBL_MAX = max size). */
void NonDC3FunctionTrain::
sample_allocation_metric(Real& regress_metric, Real power)
{
  // case RANK_SAMPLING in NonDExpansion::multilevel_regression() as well as
  // uniform refinements:

  // > sample requirements scale as O(p r^2 d) -- see regression_size()
  //   implementations in SharedC3ApproxData (scalar spec) and C3Approximation
  //   (adapted ranks per dimension along with latest startOrder increment)
  // > The function function_train_get_nparams(const struct FunctionTrain*),
  //   which is one implementation of C3Approximation::regression_size(),
  //   returns the number of unknowns from the most recent FT regression
  //   (per QoI, per model level) which provides the sample requirements
  //   prior to over-sampling/collocRatio.  Given this, only need to compute
  //   power mean over numFunctions (below) and then add any over-sampling
  //   factor (applied in compute_sample_increment())

  SharedC3ApproxData* shared_data_rep = (SharedC3ApproxData*)
    uSpaceModel.shared_approximation().data_rep();
  switch (c3RefineType) {
  case UNIFORM_MAX_RANK:
    // scale samples based on max rank to avoid challenges from sample and
    // rank advancements not being synchronized.  This simplification is
    // consistent with the corresponding advancement_available() logic:
    // refinement candidates are generated when max rank is an active bound.
    regress_metric = shared_data_rep->max_rank_regression_size();
    break;
  default: {
    std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
    Real sum = 0., max = 0.;
    bool pow_1   = (power == 1.), // detect special cases
         pow_inf = (power == std::numeric_limits<Real>::max());
    for (size_t qoi=0; qoi<numFunctions; ++qoi) {
      C3Approximation* poly_approx_q
	= (C3Approximation*)poly_approxs[qoi].approx_rep();
      Real regress_q = poly_approx_q->regression_size(); // number of unknowns
      if (outputLevel >= DEBUG_OUTPUT)
	Cout << "System size(" /*lev " << lev << ", "*/ << "qoi " << qoi
	  /* << ", iter " << iter */ << ") = " << regress_q << '\n';
      if (pow_inf) {
	if (regress_q > max)
	  max = regress_q;
      }
      else
	sum += (pow_1) ? regress_q : std::pow(regress_q, power);
    }
    if (pow_inf)
      regress_metric = max;
    else {
      sum /= numFunctions;
      regress_metric = (pow_1) ? sum : std::pow(sum, 1. / power);
    }
    break;
  }
  }
}


/* No override appears to be required (NonDExp is sufficient)
   > initialize_expansion()
   > compute_expansion()
   Overrides needed:
   > refine_expansion() --> {pre,core,post}_refinement()

This function was used to perform the Model samples for purposes of 
adaptation (as opposed to the regression case).

Rather than a grid increment inducing a batch of samples, the cross
option checks new evaluation points, one at a time.

TO DO: will want to reactivate something like this for "cross | regress"
This fn would be passed into the C3 library for its adaptation scheme
at the bottom of C3Approximation::build() -- see Warning there.

Flag for "cross | regress" is being added to the FT Model specification.

int NonDC3FunctionTrain::
qoi_eval(size_t num_samp, const double* var_sets, double* qoi_sets, void* args)
{
  size_t num_cv  = c3Instance->numContinuousVars,
         num_fns = c3Instance->numFunctions;

  bool asynch_flag = c3Instance->iteratedModel.asynch_flag();

  RealVector cv_i(num_cv, false);
  for (size_t i=0; i<num_samp; ++i) {
    copy_data(var_sets+num_cv*i, num_cv, cv_i);
    c3Instance->iteratedModel.continuous_variables(cv_i);
    if (asynch_flag)
      c3Instance->iteratedModel.evaluate_nowait();
    else {
      c3Instance->iteratedModel.evaluate();
      // pack Dakota resp data into qoi_sets...
      const RealVector& fns_i
        = c3Instance->iteratedModel.current_response().function_values();
      copy_data(fns_i, qoi_sets+num_fns*i, num_fns);
    }
  }

  if (asynch_flag) {
    const IntResponseMap& resp_map = c3Instance->iteratedModel.synchronize();
    // pack Dakota resp data into qoi_sets...
    IntRespMCIter r_cit; size_t i;
    for (r_cit=resp_map.begin(), i=0; r_cit!=resp_map.end(); ++r_cit, ++i) {
      const RealVector& fns_i = r_cit->second.function_values();
      copy_data(fns_i, qoi_sets+num_fns*i, num_fns);
    }
  }

  return 0;
}
*/


void NonDC3FunctionTrain::print_moments(std::ostream& s)
{
  s << std::scientific << std::setprecision(write_precision);

  // std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  const StringArray& fn_labels = iteratedModel.response_labels();
  size_t i, j, width = write_precision+7;

  s << "\nMoment statistics for each response function:\n";
  // Handle cases of both expansion/numerical moments or only one or the other:
  //   both exp/num: SC and PCE with numerical integration
  //   exp only:     PCE with unstructured grids (regression, exp sampling)
  // Also handle numerical exception of negative variance in either exp or num
  bool exception = false, prev_exception = false;

  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();  
  for (size_t i=0; i<numFunctions; ++i) {
      C3Approximation* poly_approx_rep_i
	= (C3Approximation*)poly_approxs[i].approx_rep();
       if (i==0 || !prev_exception)
	 s << std::setw(width+15) << "Mean" << std::setw(width+1) << "Std Dev"
	   << std::setw(width+1)  << "Skewness" << std::setw(width+2)
	   << "Kurtosis\n";
       RealVector moments = poly_approx_rep_i->moments();
       s <<  fn_labels[i] << '\n' << std::setw(14) << "analytical: ";
       s << ' ' << std::setw(width) << moments[0]
         << ' ' << std::setw(width) << sqrt(moments[1])
         << ' ' << std::setw(width) << poly_approx_rep_i->skewness()
         << ' ' << std::setw(width) << poly_approx_rep_i->kurtosis()-3;
      s << '\n';
  }
}


void print_c3_sobol_indices(double value, size_t ninteract,
			    size_t * interactions, void * arg)
{
  if (ninteract > 1){
    struct SPrintArgs * pa = (struct SPrintArgs *)arg;

    StringMultiArrayConstView cv_label = *(pa->cv_labels);
    
    String label;
    for (size_t jj = 0; jj < ninteract; jj++)
      label += cv_label[interactions[jj]] + " ";
    *(pa->s) << "                     " << std::setw(write_precision+7)
	     << value/pa->variance << ' ' << label << '\n';
  } 
}


void NonDC3FunctionTrain::print_sobol_indices(std::ostream& s)
{
  s << "\nGlobal sensitivity indices for each response function:\n";

  const StringArray& fn_labels = iteratedModel.response_labels();

  StringMultiArrayConstView cv_labels
    = iteratedModel.continuous_variable_labels();

  // print sobol indices per response function
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  size_t wpp7 = write_precision+7;
  for (size_t i=0; i<numFunctions; ++i) {
    C3Approximation* poly_approx_rep_i
      = (C3Approximation*)poly_approxs[i].approx_rep();
        
    // Print Main and Total effects
    s << fn_labels[i] << " Sobol' indices:\n" << std::setw(38) << "Main"
      << std::setw(19) << "Total\n";
        
    RealVector moments = poly_approx_rep_i->moments();
    Real var = moments(1);
    for (size_t j=0; j<numContinuousVars; ++j)
      s << "                     "   <<        std::setw(wpp7)
	<< poly_approx_rep_i->main_sobol_index(j)/var << ' '
	<< std::setw(wpp7) << poly_approx_rep_i->total_sobol_index(j)
	<< ' ' << cv_labels[j]<<'\n';

    // *** TO DO: integrate this into std NonDExpansion VBD workflow

    // Print Interaction effects
    //if (vbdOrderLimit != 1) { 
      s << std::setw(39) << "Interaction\n";
      StringMultiArrayConstView cv_labels
        = iteratedModel.continuous_variable_labels();
            
      struct SPrintArgs pa;
      pa.s = &s;
      pa.cv_labels = &cv_labels;
      pa.variance = var;

      poly_approx_rep_i->sobol_iterate_apply(print_c3_sobol_indices,&pa);
    //}
  }
}

} // namespace Dakota
