/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NOWPACOptimizer
//- Description: Implementation code for the NOWPACOptimizer class
//- Owner:       Mike Eldred

#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NOWPACOptimizer.hpp"
#include "PostProcessModels.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"

static const char rcsId[]="@(#) $Id: NOWPACOptimizer.cpp 7029 2010-10-22 00:17:02Z mseldre $";

namespace Dakota {


NOWPACOptimizer::NOWPACOptimizer(ProblemDescDB& problem_db, Model& model):
  Optimizer(problem_db, model, std::shared_ptr<TraitsBase>(new NOWPACTraits())),
  nowpacSolver(numContinuousVars, "nowpac_diagnostics.dat"),
  nowpacEvaluator(iteratedModel)
{
  nowpacEvaluator.allocate_constraints();
  nowpacSolver.set_blackbox(nowpacEvaluator,
			    nowpacEvaluator.num_ineq_constraints());
  initialize_options();
}


NOWPACOptimizer::NOWPACOptimizer(Model& model):
  Optimizer(MIT_NOWPAC, model, std::shared_ptr<TraitsBase>(new NOWPACTraits())),
  nowpacSolver(numContinuousVars, "nowpac_diagnostics.dat"),
  nowpacEvaluator(iteratedModel)
{
  nowpacEvaluator.allocate_constraints();
  nowpacSolver.set_blackbox(nowpacEvaluator,
			    nowpacEvaluator.num_ineq_constraints());
  initialize_options();
}


NOWPACOptimizer::~NOWPACOptimizer()
{ }


void NOWPACOptimizer::initialize_options()
{
  // Refer to bit bucket docs: https://bitbucket.org/fmaugust/nowpac

  // Optional: note that we are overridding NOWPAC defaults with Dakota defaults
  // May want to leave NOWPAC defaults in place if there is no user spec.
  nowpacSolver.set_option("eta_1",
    probDescDB.get_real("method.trust_region.contract_threshold") );
  nowpacSolver.set_option("eta_2",
    probDescDB.get_real("method.trust_region.expand_threshold") );
  // Criticality measures:
  //nowpacSolver.set_option("eps_c"                         , 1e-6 );
  //nowpacSolver.set_option("mu"                            , 1e1  );
  // Upper bound on poisedness constant augmented with distance penalty:
  //nowpacSolver.set_option("geometry_threshold"            , 5e2  );
  nowpacSolver.set_option("gamma_inc",
    probDescDB.get_real("method.trust_region.expansion_factor") );
  nowpacSolver.set_option("gamma",
    probDescDB.get_real("method.trust_region.contraction_factor") );
  // Reduction factors:
  //nowpacSolver.set_option("omega"                         , 0.8  );
  //nowpacSolver.set_option("theta"                         , 0.8  );
  // Inner boundary path constant: (default should be good and will be adapted)
  //if (nowpacEvaluator.num_ineq_constraints() > 0)
  //  nowpacSolver.set_option("eps_b"                       , 1e1  );

  // NOWPAC output verbosity is 0 (least) to 3 (most)
  nowpacSolver.set_option("verbose", std::min(outputLevel, (short)3) );

  //nowpacSolver.set_max_trustregion( 1e0 ); // scale dependent

  // This option activates SNOWPAC which turns on 3 features:
  // (a) links TR size to noise returned from evaluator
  // (b) feasibility restoration (on but not active in deterministic mode)
  // (c) outer Gaussian process approximation (smooths noisy evaluations)
  bool stochastic = (methodName == MIT_SNOWPAC);
  nowpacSolver.set_option("stochastic_optimization", stochastic);
  // This is tied to the other BlackBoxBaseClass::evaluate() fn redefinition.
  if (stochastic) {
    // SNOWPAC picks random points in the trust region to improve the
    // distribution of the Gaussian Process regression
    int random_seed = probDescDB.get_int("method.random_seed");
    if (random_seed) // default for no user spec is zero
      nowpacSolver.set_option("seed",                random_seed);
    //else SNOWPAC uses a machine generated seed and is non-repeatable
    const int adaptionSteps = 5*numContinuousVars;
    nowpacSolver.set_option("GP_adaption_factor", adaptionSteps );
    nowpacSolver.set_option("use_analytic_smoothing", false);
  }

  // Maximum number of total accepted steps
  nowpacSolver.set_option("max_nb_accepted_steps", (int) maxIterations);// inf default
  // Within special context of meta-iteration like MG/Opt, ensure that we
  // have at least 2 successful steps
  //if (subIteratorFlag && ...)
  //  nowpacSolver.set_option("max_nb_accepted_steps", 2    );

  // This is also a termination criterion, but not one of the required ones.
  // NOWPAC stops if the Frobenius norm of the Hessian blows up, indicating
  // that the optimizer is now in the noise.
  //nowpacSolver.set_option("noise_termination"             , false);
  // This is not tied to  the other BlackBoxBaseClass::evaluate() function 
  // redefinition, but rather is tied to the Hessian of the computed local
  // surrogate.  Generally, this is only advisable in the deterministic
  // optimization case.  There are additional parameters associated with 
  // termination from noise-detection (e.g., consecutive iterations...)

  // Required:
  // Must specify one or more stopping criteria: min TR size or maxFnEvals

  nowpacSolver.set_max_number_evaluations(maxFunctionEvals); // default is +inf

  // NOWPAC trust region controls are not relative to global bounds; rather,
  // they are absolute values for a hyper-sphere of constant dimensional radius.
  // Therefore, we present a scaled problem to NOWPAC as consistent with these 
  // trust region controls (see use of {un,}scale() within this file).
  const RealVector& tr_init
    = probDescDB.get_rv("method.trust_region.initial_size");
  size_t num_factors = tr_init.length();
  Real   min_factor  = probDescDB.get_real("method.trust_region.minimum_size"),
          tr_factor  = (num_factors) ? tr_init[0] : 0.5;
  if (num_factors > 1)
    Cerr << "\nWarning: ignoring trailing trust_region initial_size content "
	 << "for NOWPACOptimizer.\n" << std::endl;
  // domain is [0,1]; default max radius is 1; {init,min}_radius are required
  // NOWPAC inputs (no defaults) --> use Dakota defaults for {init,min}_radius
  if (min_factor < 0.)        min_factor = 0.; // Dakota default is 1.e-6
  if (tr_factor < min_factor) tr_factor  = min_factor;
  else if (tr_factor > 1.)    tr_factor  = 1.;
  nowpacSolver.set_trustregion(tr_factor, min_factor);
  //nowpacSolver.set_trustregion(tr_factor); // if only initial

  // Scale the design variables since the TR size controls are absolute, not
  // relative.  Based on the default max TR size of 1., scale to [-1,1].
  RealArray l_bnds, u_bnds; size_t num_v = iteratedModel.cv();
  l_bnds.assign(num_v, -1.); u_bnds.assign(num_v, 1.);
  nowpacSolver.set_lower_bounds(l_bnds);
  nowpacSolver.set_upper_bounds(u_bnds);

  // NOTES from 7/29/15 discussion:
  // For Lagrangian minimization within MG/Opt:
  // Option 1. Minimize L s.t. dummy constraints on objective + all constraints
  //   --> NOWPAC will still carry them along, even though inactive, and allow
  //       retrieval of optimal values and derivatives.
  //   --> This will require additional logic in the BBEvaluator in terms of
  //       alternate subproblem formulation
  //   --> allow infeasibility in meta-algorithm
  // Option 2. If meta-algorithm should enforce feasibility. can change
  //   --> sub-problem to min L s.t. g <= 0.  No need for dummy constraints
  //       assuming we back out grad f from grad L, grad g, lambda.
}


//void NOWPACOptimizer::initialize_run()
//{
//  Optimizer::initialize_run();
//}


void NOWPACOptimizer::core_run()
{
  //////////////////////////////////////////////////////////////////////////
  // Set bound constraints at run time to catch late updates
  nowpacEvaluator.set_unscaled_bounds(iteratedModel.continuous_lower_bounds(), 
				      iteratedModel.continuous_upper_bounds());

  // allocate arrays passed to optimization solver
  RealArray x_star; RealArray obj_star;
  nowpacEvaluator.scale(iteratedModel.continuous_variables(), x_star);
  // create data object for nowpac output ( required for warm start )
  BlackBoxData bb_data(numFunctions, numContinuousVars);

  //////////////////////////////////////////////////////////////////////////
  // start optimization (on output: bbdata contains data that allows warmstart
  // and enables post-processing to get model values, gradients and hessians)
  nowpacSolver.optimize(x_star, obj_star, bb_data);
  if (outputLevel >= DEBUG_OUTPUT) {
    // create post-processing object to compute surrogate models
    PostProcessModels<> PPD( bb_data );
    Cout << "\n----------------------------------------"
	 << "\nSolution returned from nowpacSolver:\n  optimal value = "
	 << obj_star[0] << "\n  optimal point =\n" << x_star
	 << "\nData from PostProcessModels:\n"
	 << "  tr size = " << PPD.get_trustregion() << '\n';
    // model value    = c + g'(x-x_c) + (x-x_c)'H(x-x_c) / 2
    // model gradient = g + H (x-x_c)
    // model Hessian  = H
    for ( int i = 0; i < numFunctions; ++i)
      Cout <<    "\n  model number "  << i+1
	   <<    "\n  value    = "    << PPD.get_c(i, x_star)
	   <<    "\n  gradient = [\n" << PPD.get_g(i, x_star)
	   << "  ]\n  hessian  = [\n" << PPD.get_H(i) << "  ]\n";
    Cout << "----------------------------------------" << std::endl;
  }

  //////////////////////////////////////////////////////////////////////////
  // Publish optimal variables
  RealVector c_vars = bestVariablesArray.front().continuous_variables_view();
  nowpacEvaluator.unscale(x_star, c_vars);
  // Publish optimal response
  RealVector best_fns(bestResponseArray.front().num_functions());
  if (!localObjectiveRecast) {
    // else local_objective_recast_retrieve() used in Optimizer::post_run()
    const BoolDeque& max_sense = iteratedModel.primary_response_fn_sense();
    best_fns[0] = (!max_sense.empty() && max_sense[0]) ? -obj_star[0] : obj_star[0];
  }
  // objective and mapped nonlinear inequalities returned from optimize()
  const SizetList& nln_ineq_map_indices
    = nowpacEvaluator.nonlinear_inequality_mapping_indices();
  const RealList&  nln_ineq_map_mult
    = nowpacEvaluator.nonlinear_inequality_mapping_multipliers();
  const RealList&  nln_ineq_map_offsets
    = nowpacEvaluator.nonlinear_inequality_mapping_offsets();
  StLCIter i_iter; RLCIter m_iter, o_iter;
  size_t cntr = 0;//numEqConstraints;
  for (i_iter  = nln_ineq_map_indices.begin(),
	 m_iter  = nln_ineq_map_mult.begin(),
	 o_iter  = nln_ineq_map_offsets.begin();
       i_iter != nln_ineq_map_indices.end(); ++i_iter, ++m_iter, ++o_iter)
    best_fns[(*i_iter)+numUserPrimaryFns] =
      (obj_star[++cntr] - (*o_iter))/(*m_iter);
  /*
    size_t i, offset = iteratedModel.num_nonlinear_ineq_constraints() + 1,
    num_nln_eq = iteratedModel.num_nonlinear_eq_constraints();
    const RealVector& nln_eq_targets
    = iteratedModel.nonlinear_eq_constraint_targets();
    for (i=0; i<num_nln_eq; i++)
    best_fns[i+offset] = fn_star[++cntr] + nln_eq_targets[i];
  */

  bestResponseArray.front().function_values(best_fns);
}


void NOWPACBlackBoxEvaluator::allocate_constraints()
{
  // NOWPAC handles 1-sided inequalities <= 0.  Equalities cannot be mapped to
  // two oppositely-signed inequalities due to the interior path requirement.
  // Hard error for now...
  bool constraint_err = false;
  if (iteratedModel.num_nonlinear_eq_constraints()) {
    Cerr << "Error: NOWPAC does not support nonlinear equality constraints."
	 << std::endl;
    constraint_err = true;
  }
  if (iteratedModel.num_linear_eq_constraints()) {
    Cerr << "Error: NOWPAC does not support linear equality constraints."
	 << std::endl;
    constraint_err = true;
  }
  if (constraint_err)
    abort_handler(METHOD_ERROR);

  nonlinIneqConMappingIndices.clear();
  nonlinIneqConMappingMultipliers.clear();
  nonlinIneqConMappingOffsets.clear();

  linIneqConMappingIndices.clear();
  linIneqConMappingMultipliers.clear();
  linIneqConMappingOffsets.clear();

  // Compute number of 1-sided inequalities to pass to NOWPAC and the mappings
  // (indices, multipliers, offsets) between DAKOTA and NOWPAC constraints.
  numNowpacIneqConstr = 0;
  size_t i, num_nln_ineq = iteratedModel.num_nonlinear_ineq_constraints();
  const RealVector& nln_ineq_lwr_bnds
    = iteratedModel.nonlinear_ineq_constraint_lower_bounds();
  const RealVector& nln_ineq_upr_bnds
    = iteratedModel.nonlinear_ineq_constraint_upper_bounds();
  for (i=0; i<num_nln_ineq; i++) {
    if (nln_ineq_lwr_bnds[i] > -BIG_REAL_BOUND) {
      ++numNowpacIneqConstr;
      // nln_ineq_lower_bnd - dakota_constraint <= 0
      nonlinIneqConMappingIndices.push_back(i);
      nonlinIneqConMappingMultipliers.push_back(-1.);
      nonlinIneqConMappingOffsets.push_back(nln_ineq_lwr_bnds[i]);
    }
    if (nln_ineq_upr_bnds[i] <  BIG_REAL_BOUND) {
      ++numNowpacIneqConstr;
      // dakota_constraint - nln_ineq_upper_bnd <= 0
      nonlinIneqConMappingIndices.push_back(i);
      nonlinIneqConMappingMultipliers.push_back(1.);
      nonlinIneqConMappingOffsets.push_back(-nln_ineq_upr_bnds[i]);
    }
  }
  size_t num_lin_ineq = iteratedModel.num_linear_ineq_constraints();
  const RealVector& lin_ineq_lwr_bnds
    = iteratedModel.linear_ineq_constraint_lower_bounds();
  const RealVector& lin_ineq_upr_bnds
    = iteratedModel.linear_ineq_constraint_upper_bounds();
  for (i=0; i<num_lin_ineq; i++) {
    if (lin_ineq_lwr_bnds[i] > -BIG_REAL_BOUND) {
      ++numNowpacIneqConstr;
      // lin_ineq_lower_bnd - Ax <= 0
      linIneqConMappingIndices.push_back(i);
      linIneqConMappingMultipliers.push_back(-1.);
      linIneqConMappingOffsets.push_back(lin_ineq_lwr_bnds[i]);
    }
    if (lin_ineq_upr_bnds[i] <  BIG_REAL_BOUND) {
      ++numNowpacIneqConstr;
      // Ax - lin_ineq_upper_bnd <= 0
      linIneqConMappingIndices.push_back(i);
      linIneqConMappingMultipliers.push_back(1.);
      linIneqConMappingOffsets.push_back(-lin_ineq_upr_bnds[i]);
    }
  }
}


void NOWPACBlackBoxEvaluator::
evaluate(RealArray const &x, RealArray &vals, void *param)
{
  // NOWPACOptimizer enforces an embedded scaling: incoming x is scaled on [-1,1]
  // -->  unscale for posting to iteratedModel
  RealVector& c_vars
    = iteratedModel.current_variables().continuous_variables_view();
  unscale(x, c_vars);

  iteratedModel.evaluate(); // no ASV control, use default

  const RealVector& dakota_fns
    = iteratedModel.current_response().function_values();
  // If no mappings...
  //copy_data(dakota_fns, vals);

  // apply optimization sense mapping.  Note: Any MOO/NLS recasting is
  // responsible for setting the scalar min/max sense within the recast.
  const BoolDeque& max_sense = iteratedModel.primary_response_fn_sense();
  Real obj_fn = dakota_fns[0];
  vals[0] = (!max_sense.empty() && max_sense[0]) ? -obj_fn : obj_fn;

  // apply nonlinear inequality constraint mappings
  StLIter i_iter; RLIter m_iter, o_iter; size_t cntr = 0;
  for (i_iter  = nonlinIneqConMappingIndices.begin(),
       m_iter  = nonlinIneqConMappingMultipliers.begin(),
       o_iter  = nonlinIneqConMappingOffsets.begin();
       i_iter != nonlinIneqConMappingIndices.end();
       ++i_iter, ++m_iter, ++o_iter)   // nonlinear ineq
    vals[++cntr] = (*o_iter) + (*m_iter) * dakota_fns[(*i_iter)+1];

  // apply linear inequality constraint mappings
  const RealMatrix& lin_ineq_coeffs
    = iteratedModel.linear_ineq_constraint_coeffs();
  size_t j, num_cv = x.size();
  for (i_iter  = linIneqConMappingIndices.begin(),
       m_iter  = linIneqConMappingMultipliers.begin(),
       o_iter  = linIneqConMappingOffsets.begin();
       i_iter != linIneqConMappingIndices.end();
       ++i_iter, ++m_iter, ++o_iter) { // linear ineq
    size_t index = *i_iter;
    Real Ax = 0.;
    for (j=0; j<num_cv; ++j)
      Ax += lin_ineq_coeffs(index,j) * x[j];
    vals[++cntr] = (*o_iter) + (*m_iter) * Ax;
  }
}
// TO DO: asynchronous evaluate_nowait()/synchronize()


void NOWPACBlackBoxEvaluator::
evaluate(RealArray const &x, RealArray &vals, RealArray &noise, void *param)
{
  // NOWPACOptimizer enforces an embedded scaling: incoming x is scaled on [0,1]
  // -->  unscale for posting to iteratedModel
  RealVector& c_vars
    = iteratedModel.current_variables().continuous_variables_view();
  unscale(x, c_vars);

  iteratedModel.evaluate(); // no ASV control, use default

  const RealVector& dakota_fns
    = iteratedModel.current_response().function_values();
  // NonD implements std error estimates for selected QoI statistics (see, e.g.,
  // Harting et al. for MC error estimates).  NestedModel implements
  // sub-iterator mappings for both fn vals & std errors.
  const RealVector& errors = iteratedModel.error_estimates();

  // apply optimization sense mapping.  Note: Any MOO/NLS recasting is
  // responsible for setting the scalar min/max sense within the recast.
  const BoolDeque& max_sense = iteratedModel.primary_response_fn_sense();
  Real obj_fn = dakota_fns[0], mult;
  size_t index, cntr = 0;
  vals[cntr]  = (!max_sense.empty() && max_sense[0]) ? -obj_fn : obj_fn;
  noise[cntr] = 2. * errors[0]; // for now; TO DO: mapping of noise for MOO/NLS...
  ++cntr;

  // apply nonlinear inequality constraint mappings
  StLIter i_iter; RLIter m_iter, o_iter;
  for (i_iter  = nonlinIneqConMappingIndices.begin(),
       m_iter  = nonlinIneqConMappingMultipliers.begin(),
       o_iter  = nonlinIneqConMappingOffsets.begin();
       i_iter != nonlinIneqConMappingIndices.end();
       ++i_iter, ++m_iter, ++o_iter, ++cntr) {  // nonlinear ineq
    index       = (*i_iter)+1; // offset single objective
    mult        = (*m_iter);
    vals[cntr]  = (*o_iter) + mult * dakota_fns[index];
    noise[cntr] = 2. * std::abs(mult) * errors[index];
  }

  // apply linear inequality constraint mappings
  const RealMatrix& lin_ineq_coeffs
    = iteratedModel.linear_ineq_constraint_coeffs();
  size_t j, num_cv = x.size();
  for (i_iter  = linIneqConMappingIndices.begin(),
       m_iter  = linIneqConMappingMultipliers.begin(),
       o_iter  = linIneqConMappingOffsets.begin();
       i_iter != linIneqConMappingIndices.end();
       ++i_iter, ++m_iter, ++o_iter, ++cntr) { // linear ineq
    size_t index = *i_iter;
    Real Ax = 0.;
    for (j=0; j<num_cv; ++j)
      Ax += lin_ineq_coeffs(index,j) * x[j];
    vals[cntr]  = (*o_iter) + (*m_iter) * Ax;
    noise[cntr] = 0.; // no error in linear case
  }
}

double NOWPACBlackBoxEvaluator::evaluate_samples ( std::vector<double> const &samples, const unsigned int index, std::vector<double> const &x){
	//TO DO implement if smoothing works at some point.
	return -1;
}
// TO DO: asynchronous evaluate_nowait()/synchronize()


#ifdef HAVE_DYNLIB_FACTORIES
NOWPACOptimizer* new_NOWPACOptimizer(ProblemDescDB& problem_db, Model& model)
{
#ifdef DAKOTA_DYNLIB
  not_available("NOWPAC");
  return 0;
#else
  return new NOWPACOptimizer(problem_db, model);
#endif // DAKOTA_DYNLIB
}

NOWPACOptimizer* new_NOWPACOptimizer(Model& model)
{
#ifdef DAKOTA_DYNLIB
  not_available("NOWPAC");
  return 0;
#else
  return new NOWPACOptimizer(model);
#endif // DAKOTA_DYNLIB
}
#endif // HAVE_DYNLIB_FACTORIES

} // namespace Dakota
