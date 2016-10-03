/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NOWPACOptimizer
//- Description: Implementation code for the NOWPACOptimizer class
//- Owner:       Mike Eldred

#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NOWPACOptimizer.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"

static const char rcsId[]="@(#) $Id: NOWPACOptimizer.cpp 7029 2010-10-22 00:17:02Z mseldre $";

#if 0

namespace Dakota {


void NOWPACBlackBoxEvaluator::
evaluate(std::vector<double> const &x, std::vector<double> &vals, void *param)
{
  RealVector c_vars = iteratedModel.continuous_variables_view();
  copy_data(x, c_vars);
  iteratedModel.compute_response(); // no ASV control, use default
  copy_data(iteratedModel.current_response().function_values(), vals);

  // TO DO: apply optimization sense mapping...

  // TO DO: apply constraint mapping...
}

// TO DO: asnchronous evaluate() + synchronize() ???

NOWPACOptimizer::NOWPACOptimizer(ProblemDescDB& problem_db, Model& model):
  Optimizer(problem_db, model),
  nowpacSolver(numContinuousVars, "nowpac_diagnostics.dat")
{ initialize(); }


NOWPACOptimizer::NOWPACOptimizer(Model& model): Optimizer(NOWPAC_OPT, model),
  nowpacSolver(numContinuousVars, "nowpac_diagnostics.dat")
{ initialize(); }


void NOWPACOptimizer::initialize()
{
  // NOWPAC does not support internal calculation of numerical derivatives
  if (vendorNumericalGradFlag) {
    Cerr << "\nError: vendor numerical gradients not supported by NOWPAC."
	 << "\n       Please select dakota numerical instead." << std::endl;
    abort_handler(-1);
  }

  allocate_constraints();

  // plug in derived evaluator instance
  nowpacSolver.set_blackbox(nowpacEvaluator, numNowpacIneqConstr );

  // Set lower bound constraints (optional)
  RealArray l_bnds, u_bnds;
  copy_data(iteratedModel.continuous_lower_bounds(), l_bnds);
  copy_data(iteratedModel.continuous_upper_bounds(), u_bnds);
  nowpacSolver.set_lower_bounds(l_bnds);
  nowpacSolver.set_upper_bounds(u_bnds);

  // Refer to bit bucket docs: https://bitbucket.org/fmaugust/nowpac

  // Optional: note that we are overridding NOWPAC defaults with Dakota defaults
  // May want to leave NOWPAC defaults in place if there is no user spec.
  nowpacSolver.set_option("eta_0",
    probDescDB.get_real("method.sbl.trust_region.contract_threshold") );
  nowpacSolver.set_option("eta_1",
    probDescDB.get_real("method.sbl.trust_region.expand_threshold") );
  // Criticality measures:
  //nowpacSolver.set_option("eps_c"                         , 1e-6 );
  //nowpacSolver.set_option("mu"                            , 1e1  );
  // Upper bound on poisedness constant augmented with distance penalty:
  //nowpacSolver.set_option("geometry_threshold"            , 5e2  );
  nowpacSolver.set_option("gamma_inc",
    probDescDB.get_real("method.sbl.trust_region.expansion_factor") );
  nowpacSolver.set_option("gamma",
    probDescDB.get_real("method.sbl.trust_region.contraction_factor") );
  // Reduction factors:
  //nowpacSolver.set_option("omega"                         , 0.8  );
  //nowpacSolver.set_option("theta"                         , 0.8  );
  // Inner boundary path constant: (default should be good and will be adapted)
  //if ( numNowpacIneqConstr > 0)
  //  nowpacSolver.set_option("eps_b"                       , 1e1  );

  // NOWPAC output verbosity is 0 (least) to 3 (most)
  nowpacSolver.set_option("verbose", std::min(outputLevel,3) );

  //nowpacSolver.set_max_trustregion( 1e0 ); // scale dependent

  // This option activates SNOWPAC which turns on 3 features:
  // (a) links TR size to noise returned from evaluator
  // (b) feasibility restoration (on but not active in deterministic mode)
  // (c) outer Gaussian process approximation (smooths noisy evaluations)
  nowpacSolver.set_option("stochastic_optimization"       , false);
  // This is tied to the other BlackBoxBaseClass::evaluate() function redefinition.

  // Maximum number of total accepted steps
  nowpacSolver.set_option("max_nb_accepted_steps", maxIterations); // default is +inf
  // Within special context of meta-iteration like MG/Opt, ensure that we
  // have at least 2 successful steps
  //if (subIteratorFlag && ...)
  //  nowpacSolver.set_option("max_nb_accepted_steps"         , 2    );

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
  //nowpacSolver.set_trustregion(initial_tr_radius); // if only initial
  // TO DO: these are not relative to global bounds, they are absolute values
  //        this is a hyper-sphere of constant dimensional radius.
  // Therefore, it is advisable to present a scaled problem to NOWPAC in terms 
  // of these optional inputs, and then perform a descaling to user-space within
  // the BBEvaluator (scaling data can be passed by the "void* params").
  nowpacSolver.set_trustregion(
    probDescDB.get_real("method.sbl.trust_region.initial_size"),
    probDescDB.get_real("method.sbl.trust_region.minimum_size"));
  nowpacSolver.set_max_number_evaluations(maxFunctionEvals); // default is +inf

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


NOWPACOptimizer::~NOWPACOptimizer()
{
  // Virtual destructor handles referenceCount at Iterator level
}


void NOWPACOptimizer::allocate_constraints()
{
  // NOWPAC handles equality constraints = 0 and 1-sided inequalities >= 0.
  // Compute the number of equalities and 1-sided inequalities to pass to NOWPAC
  // as well as the mappings (indices, multipliers, offsets) between the DAKOTA
  // constraints and the NOWPAC constraints.
  size_t i, num_nln_ineq = iteratedModel.num_nonlinear_ineq_constraints(),
            num_lin_ineq = iteratedModel.num_linear_ineq_constraints();
  numNowpacIneqConstr = 0;
  const RealVector& nln_ineq_lwr_bnds
    = iteratedModel.nonlinear_ineq_constraint_lower_bounds();
  const RealVector& nln_ineq_upr_bnds
    = iteratedModel.nonlinear_ineq_constraint_upper_bounds();
  const RealVector& lin_ineq_lwr_bnds
    = iteratedModel.linear_ineq_constraint_lower_bounds();
  const RealVector& lin_ineq_upr_bnds
    = iteratedModel.linear_ineq_constraint_upper_bounds();

  /* NOTE: no support for linear/nonlinear equality constraints due to
     interior path construction. */

  for (i=0; i<num_nln_ineq; i++) {
    if (nln_ineq_lwr_bnds[i] > -bigRealBoundSize) {
      ++numNowpacIneqConstr;
      // nln_ineq_lower_bnd - dakota_constraint <= 0
      nonlinIneqConMappingIndices.push_back(i);
      nonlinIneqConMappingMultipliers.push_back(-1.0);
      nonlinIneqConMappingOffsets.push_back(nln_ineq_lwr_bnds[i]);
    }
    if (nln_ineq_upr_bnds[i] < bigRealBoundSize) {
      ++numNowpacIneqConstr;
      // dakota_constraint - nln_ineq_upper_bnd <= 0
      nonlinIneqConMappingIndices.push_back(i);
      nonlinIneqConMappingMultipliers.push_back(1.0);
      nonlinIneqConMappingOffsets.push_back(-nln_ineq_upr_bnds[i]);
    }
  }
  for (i=0; i<num_lin_ineq; i++) {
    if (lin_ineq_lwr_bnds[i] > -bigRealBoundSize) {
      ++numNowpacIneqConstr;
      // lin_ineq_lower_bnd - Ax <= 0
      linIneqConMappingIndices.push_back(i);
      linIneqConMappingMultipliers.push_back(-1.0);
      linIneqConMappingOffsets.push_back(lin_ineq_lwr_bnds[i]);
    }
    if (lin_ineq_upr_bnds[i] < bigRealBoundSize) {
      ++numNowpacIneqConstr;
      // Ax - lin_ineq_upper_bnd <= 0
      linIneqConMappingIndices.push_back(i);
      linIneqConMappingMultipliers.push_back(1.0);
      linIneqConMappingOffsets.push_back(-lin_ineq_upr_bnds[i]);
    }
  }
}


void NOWPACOptimizer::initialize_run()
{
  Optimizer::initialize_run();

}


void NOWPACOptimizer::core_run()
{
  // TO DO: utilize L concurrency with evaluate_nowait()/synchronize()

  const RealVector& cdv_lower_bnds
    = iteratedModel.continuous_lower_bounds();
  const RealVector& cdv_upper_bnds
    = iteratedModel.continuous_upper_bounds();
  size_t i, j, fn_eval_cntr,
    num_nln_ineq = iteratedModel.num_nonlinear_ineq_constraints(),
    num_nln_eq   = iteratedModel.num_nonlinear_eq_constraints();
  const RealMatrix& lin_ineq_coeffs
    = iteratedModel.linear_ineq_constraint_coeffs();

  // Any MOO/NLS recasting is responsible for setting the scalar min/max
  // sense within the recast.
  const BoolDeque& max_sense = iteratedModel.primary_response_fn_sense();
  bool max_flag = (!max_sense.empty() && max_sense[0]);

  //////////////////////////////////////////////////////////////////////

  // start optimization
  nowpacSolver.optimize(x_star, obj_star);

  // output ...
  std::cout << "optimal value = " << obj_star << std::endl;
  std::cout << "optimal point = [" << x_star << "]" << std::endl;
    
  double                             trustregion;
  double                             c;
  std::vector<double>                g(dim);
  std::vector< std::vector<double> > H(dim);
  for ( int i = 0; i < dim; ++i )    H[i].resize(dim);
    
  trustregion = nowpacSolver.get_trustregion();
  std::cout << std::endl << "----------------------------------------" << std::endl;
  std::cout << "tr size = " << trustregion << std::endl;
  std::cout << "----------------------------------------" << std::endl;
  // model = c + g'(x-x_c) + (x-x_c)'H(x-x_c) / 2
  for ( int i = 0; i < numFunctions; ++i) {
    fn   = nowpacSolver.get_c(i, x_star); // get model value at x_star
    grad = nowpacSolver.get_g(i, x_star); // get model gradient at x_star (center of final TR) = g + H (x-x_c)
    Hess = nowpacSolver.get_H(i);    // get model Hessian = H
    std::cout << "model number " << i << std::endl;
    std::cout << "value   = " << c << std::endl;
    std::cout << "grad    = [" << g[0] << ", " << g[1] << "]" << std::endl;
    std::cout << "hess    = [" << H[0][0] << ", " << H[0][1] << std::endl;
    std::cout << "           " << H[1][0] << ", " << H[1][1] << "]" <<std::endl;
  }

  //////////////////////////////////////////

  // Publish optimal solution
  RealVector local_cdv(N, false);
  copy_data(X, N, local_cdv); // Note: X is [NMAX,L]
  bestVariablesArray.front().continuous_variables(local_cdv);
  if (!localObjectiveRecast) { // else local_objective_recast_retrieve()
                               // is used in Optimizer::post_run()
    RealVector best_fns(numFunctions);
    best_fns[0] = (max_flag) ? -F[0] : F[0];

    StLIter i_iter;
    RLIter  m_iter, o_iter;
    size_t  cntr = numEqConstraints;
    for (i_iter  = nonlinIneqConMappingIndices.begin(),
	 m_iter  = nonlinIneqConMappingMultipliers.begin(),
	 o_iter  = nonlinIneqConMappingOffsets.begin();
	 i_iter != nonlinIneqConMappingIndices.end();
	 i_iter++, m_iter++, o_iter++)   // nonlinear ineq
      best_fns[(*i_iter)+1] = (G[cntr++] - (*o_iter))/(*m_iter);

    size_t i, 
      num_nln_ineq = iteratedModel.num_nonlinear_ineq_constraints(),
      num_nln_eq = iteratedModel.num_nonlinear_eq_constraints();
    const RealVector& nln_eq_targets
      = iteratedModel.nonlinear_eq_constraint_targets();
    for (i=0; i<num_nln_eq; i++)
      best_fns[i+num_nln_ineq+1] = G[i] + nln_eq_targets[i];

    bestResponseArray.front().function_values(best_fns);
  }
}


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

#endif
