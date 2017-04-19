/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       HOPSOptimizer
//- Description: Wrapper class for HOPSPACK
//- Owner:       Patty Hough
//- Checked by:
//- Version: $Id

#include "ProblemDescDB.hpp"
#include "APPSOptimizer.hpp"

namespace Dakota {


APPSOptimizer::APPSOptimizer(ProblemDescDB& problem_db, Model& model):
  Optimizer(problem_db, model)
{
  // (iteratedModel initialized in Optimizer(Model&))

  evalMgr = new APPSEvalMgr(iteratedModel);
  set_apps_parameters(); // set specification values using DB
}

APPSOptimizer::APPSOptimizer(Model& model):
  Optimizer(ASYNCH_PATTERN_SEARCH, model)
{
  // (iteratedModel initialized in Optimizer(Model&))

  evalMgr = new APPSEvalMgr(iteratedModel);
  set_apps_parameters(); // set specification values using DB
}

/** core_run redefines the Optimizer virtual function to perform
    the optimization using HOPS. It first sets up the problem data,
    then executes minimize() on the HOPS optimizer, and finally
    catalogues the results. */

void APPSOptimizer::core_run()
{
  // Tell the evalMgr whether or not to do asynchronous evaluations
  // and maximum available concurrency.

  evalMgr->set_asynch_flag(iteratedModel.asynch_flag());
  evalMgr->set_total_workers(iteratedModel.evaluation_capacity());

  // Initialize variable values and define constraints.

  initialize_variables_and_constraints();

  // Instantiate optimizer and solve.

  HOPSPACK::Hopspack optimizer(evalMgr);
  optimizer.setInputParameters(params);

  optimizer.solve();

  // Retrieve best iterate and convert from HOPS vector to DAKOTA
  // vector.

  std::vector<double> bestX(numTotalVars);
  bool state = optimizer.getBestX(bestX);

//PDH: Set final solution.
//     Think the code from here to the end of the method
//     can be greatly simplified with data adapters built
//     on top of data tranfers.
//     Would like to just do something like
//     setBestVariables(...)
//     setBestResponses(...)

  set_variables<HOPSPACK::Vector>(bestX, iteratedModel, bestVariablesArray.front());

  // Retrieve the best responses and convert from HOPS vector to
  // DAKOTA vector.

  if (!localObjectiveRecast) {
    // else local_objective_recast_retrieve() is used in Optimizer::post_run()

//PDH: std::vector<double> -> RealVector
//     Has to respect Dakota's ordering of inequality and equality constraints.
//     Has to map format of constraints.
//     Then populate bestResponseArray.

    set_best_responses<APPSOptimizerAdapter>( optimizer, iteratedModel, 
                                              constraintMapIndices, 
                                              constraintMapMultipliers, 
                                              constraintMapOffsets,
                                              bestResponseArray);
  }
}

/** Set all of the HOPS algorithmic parameters as specified in the
    DAKOTA input deck.  This is called at construction time. */

void APPSOptimizer::set_apps_parameters()
{
  // Get pointers to parameter sublists.

  problemParams = &(params.getOrSetSublist("Problem Definition"));
  linearParams = &(params.getOrSetSublist("Linear Constraints"));
  mediatorParams = &(params.getOrSetSublist("Mediator"));
  citizenParams = &(params.getOrSetSublist("Citizen 1"));

  // Set amount of output to display.

  switch (outputLevel) {
  case DEBUG_OUTPUT:
    problemParams->setParameter("Display", 2);
    linearParams->setParameter("Display", 2);
    mediatorParams->setParameter("Display", 5);
    if (numNonlinearConstraints > 0) {
      citizenParams->setParameter("Display", 2);
      citizenParams->setParameter("Display Subproblem", 3);
    }
    else
      citizenParams->setParameter("Display", 3);
    break;
  case VERBOSE_OUTPUT:
    problemParams->setParameter("Display", 1);
    linearParams->setParameter("Display", 1);
    mediatorParams->setParameter("Display", 4);
    if (numNonlinearConstraints > 0) {
      citizenParams->setParameter("Display", 1);
      citizenParams->setParameter("Display Subproblem", 2);
    }
    else
      citizenParams->setParameter("Display", 2);
    break;
  case NORMAL_OUTPUT:
    problemParams->setParameter("Display", 0);
    linearParams->setParameter("Display", 0);
    mediatorParams->setParameter("Display", 3);
    if (numNonlinearConstraints > 0) {
      citizenParams->setParameter("Display", 0);
      citizenParams->setParameter("Display Subproblem", 1);
    }
    else
      citizenParams->setParameter("Display", 1);
    break;
  case QUIET_OUTPUT:
    problemParams->setParameter("Display", 0);
    linearParams->setParameter("Display", 0);
    mediatorParams->setParameter("Display", 2);
    if (numNonlinearConstraints > 0) {
      citizenParams->setParameter("Display", 0);
      citizenParams->setParameter("Display Subproblem", 0);
    }
    else
      citizenParams->setParameter("Display", 0);
    break;
  case SILENT_OUTPUT:
    problemParams->setParameter("Display", 0);
    linearParams->setParameter("Display", 0);
    mediatorParams->setParameter("Display", 1);
    if (numNonlinearConstraints > 0) {
      citizenParams->setParameter("Display", 0);
      citizenParams->setParameter("Display Subproblem", 0);
    }
    else
      citizenParams->setParameter("Display", 0);
  }

  // Set number of citizens (i.e. algorithms) and maximum total number
  // of evaluations.  Not doing hybrid at this point, so only one
  // citizen (generalized set search, formerly APPS).

  mediatorParams->setParameter("Citizen Count", 1);
  mediatorParams->setParameter("Maximum Evaluations", maxFunctionEvals);

  // Set GSS variant based on presence or not of nonlinear
  // constraints.

  if (numNonlinearConstraints > 0)
    citizenParams->setParameter("Type", "GSS-NLC");
  else
    citizenParams->setParameter("Type", "GSS");

  // Set linear and nonlinear constraint tolerances.  DAKOTA does not
  // distinguish between the two, so we use the same value for both.

  if (constraintTol > 0.0) {
    linearParams->setParameter("Active Tolerance", constraintTol);
    citizenParams->setParameter("Nonlinear Active Tolerance", constraintTol);
  }

  if (probDescDB.is_null()) {
    // Instantiate on-the-fly.
    // Rely on internal HOPS defaults for the most part, but set any
    // default overrides (including enforcement of DAKOTA defaults).
  }
  else {

    // Set synchronous or asynchronous GSS behavior.
    // A null string is the DB default and nonblocking is the HOPS default, so
    // the flag is true only for an explicit blocking user specification.

    const bool blocking_synch
      = (probDescDB.get_string("method.pattern_search.synchronization") ==
	 "blocking");
    if (blocking_synch) {
      mediatorParams->setParameter("Synchronous Evaluations", true);
      citizenParams->setParameter("Use Random Order", false);
      evalMgr->set_blocking_synch(true);
    }
    else
      mediatorParams->setParameter("Synchronous Evaluations", false);

    // Set GSS algorithm control parameters.

    const Real& init_step_length
      = probDescDB.get_real("method.asynch_pattern_search.initial_delta");
    if (init_step_length > 0.0)
      citizenParams->setParameter("Initial Step", init_step_length);
    else
      Cout << "\nWarning: initial_delta must be greater than 0.0."
	   << "\n         Using default value of 1.0.\n\n";

    const Real& contract_step_length
      = probDescDB.get_real("method.asynch_pattern_search.contraction_factor");
    if (contract_step_length > 0.0 && contract_step_length < 1.0)
      citizenParams->setParameter("Contraction Factor", contract_step_length);
    else
      Cout << "\nWarning: contraction_factor must be between 0.0 and 1.0, noninclusive."
	   << "\n         Using default value of 0.5.\n\n";

    const Real& thresh_step_length
      = probDescDB.get_real("method.asynch_pattern_search.threshold_delta");
    if (thresh_step_length >= 4.4e-16)
      citizenParams->setParameter("Step Tolerance", thresh_step_length);
    else
      Cout << "\nWarning: threshold_delta must be between greater than or equal to 4.4e-16."
	   << "\n         Using default value of 0.01.\n\n";

    const Real& solution_target
      = probDescDB.get_real("method.solution_target");
    if (solution_target > -DBL_MAX)
      problemParams->setParameter("Objective Target", solution_target);

    // For nonlinearly constrained problems, set penalty-related parameters.

    if (numNonlinearConstraints > 0) {
      const String merit_function = probDescDB.get_string("method.asynch_pattern_search.merit_function");
      if (merit_function == "merit_max")
	citizenParams->setParameter("Penalty Function", "L_inf");
      else if (merit_function == "merit_max_smooth")
	citizenParams->setParameter("Penalty Function", "L_inf (smoothed)");
      else if (merit_function == "merit1")
	citizenParams->setParameter("Penalty Function", "L1");
      else if (merit_function == "merit1_smooth")
	citizenParams->setParameter("Penalty Function", "L1 (smoothed)");
      else if (merit_function == "merit2")
	citizenParams->setParameter("Penalty Function", "L2");
      else if (merit_function == "merit2_smooth")
	citizenParams->setParameter("Penalty Function", "L2 (smoothed)");
      else if (merit_function == "merit2_squared")
	citizenParams->setParameter("Penalty Function", "L2 Squared");
      else
	Cout << "\nWarning: merit_function invalid."
	     << "\n         Using default L2 Squared.\n\n";

      Real constr_penalty = probDescDB.get_real("method.asynch_pattern_search.constraint_penalty");
      if (constr_penalty >= 0.0)
	citizenParams->setParameter("Penalty Parameter", constr_penalty);
      else
	Cout << "\nWarning: constraint_penalty must be between greater than or equal to 0.0."
	     << "\n         Using default value of 1.0.\n\n";

      Real smooth_factor = probDescDB.get_real("method.asynch_pattern_search.smoothing_factor");
      if (smooth_factor >= 0.0 && smooth_factor <= 1.0)
	citizenParams->setParameter("Penalty Smoothing Value", smooth_factor);
      else
	Cout << "\nWarning: smoothing_factor must be between 0.0 and 1.0, inclusive."
	     << "\n         Using default value of 0.0.\n\n";
    }

    maxEvalConcurrency *= 2*numContinuousVars;

    // ----------------------------------------------------------------
    // Current HOPS is hardwired for coordinate bases, no expansion,
    // and no pattern augmentation.
    //
    // See http://software.sandia.gov/appspack/pageParameters.html
    // for valid specification options.
    //
    // HOPS wish list:
    //   basis control (only coordinate now supported due to simplified
    //     constraint management)
    //   some form of load balancing (total size preferred; Tammy says 
    //     that pattern augmentation can be done, but would not account 
    //     for cache management)
    // ----------------------------------------------------------------
  }
}

/** Set the variables and constraints as specified in the DAKOTA input
    deck.  This is done at run time. */

void APPSOptimizer::initialize_variables_and_constraints()
{
  // Initialize variables and bounds.  This is performed in core_run
  // in order to capture any reassignment at the strategy layer (after
  // iterator construction).  

//PDH: This initializes everything for HOPSPACK.
//     RealVector, IntVector -> std::vector
//     RealMatrix -> ??? (need to look this up)
//     Don't want any references to iteratedModel.
//     Need to handle discrete variable mapping.
//     Need to respect equality, inequality constraint ordering.
//     Need to handle constraint mapping.

  numTotalVars = numContinuousVars + numDiscreteIntVars 
             + numDiscreteRealVars + numDiscreteStringVars;

  HOPSPACK::Vector init_point(numTotalVars);
  HOPSPACK::Vector lower(numTotalVars), upper(numTotalVars);

  vector<char> variable_types(numTotalVars, 'C');

  // For now this requires that the target vector, eg init_point, be allocated properly.
  get_variables<HOPSPACK::Vector>(iteratedModel, init_point);

  bool setScales = !get_bounds<APPSOptimizerAdapter>
                    (iteratedModel, bigRealBoundSize, bigIntBoundSize, 
                     lower, upper);

  problemParams->setParameter("Number Unknowns", (int) numTotalVars);
  problemParams->setParameter("Variable Types", variable_types);
  problemParams->setParameter("Initial X", init_point);
  problemParams->setParameter("Lower Bounds", lower);
  problemParams->setParameter("Upper Bounds", upper);

  // If there are no bound constraints (ie if any are missing?), HOPSPACK requires that scaling be provided.

  if (setScales) {
    HOPSPACK::Vector scales(numContinuousVars);
    for (int i=0; i<numContinuousVars; i++)
      scales[i] = 1.0;
    problemParams->setParameter("Scaling", scales);
  }

  // Define linear equality and inequality constraints.

  HOPSPACK::Vector lin_ineq_lower_bnds(numLinearIneqConstraints);
  HOPSPACK::Vector lin_ineq_upper_bnds(numLinearIneqConstraints);
  HOPSPACK::Vector lin_eq_targets(numLinearEqConstraints);
  HOPSPACK::Matrix lin_ineq_coeffs, lin_eq_coeffs;

  // Need to make pre-allocation requirement consistent, eg vectors are allocated, matrices are not
  get_linear_constraints<APPSOptimizerAdapter>
                ( iteratedModel, bigRealBoundSize,
                  lin_ineq_lower_bnds,
                  lin_ineq_upper_bnds,
                  lin_eq_targets,
                  lin_ineq_coeffs,
                  lin_eq_coeffs);

  linearParams->setParameter("Inequality Matrix", lin_ineq_coeffs);
  linearParams->setParameter("Inequality Lower", lin_ineq_lower_bnds);
  linearParams->setParameter("Inequality Upper", lin_ineq_upper_bnds);
  linearParams->setParameter("Equality Matrix", lin_eq_coeffs);
  linearParams->setParameter("Equality Bounds", lin_eq_targets);

  // Define nonlinear equality and inequality constraints.

  const RealVector& nln_ineq_lwr_bnds
    = iteratedModel.nonlinear_ineq_constraint_lower_bounds();
  const RealVector& nln_ineq_upr_bnds
    = iteratedModel.nonlinear_ineq_constraint_upper_bounds();
  const RealVector& nln_eq_targets
    = iteratedModel.nonlinear_eq_constraint_targets();

  int numAPPSNonlinearIneqConstraints = 0;

  // HOPSPACK expects nonlinear equality constraints to be of the form
  // c(x) = 0 and nonlinear inequality constraints to be of the form
  // c(x) >= 0.  Compute the number of 1-sided nonlinear inequality
  // constraints to pass to HOPSPACK (numAPPSNonlinearIneqConstraints)
  // as well as the mappings for both equalities and inequalities
  // (indices, multipliers, offsets) between the DAKOTA constraints
  // and the HOPSPACK constraints.

  for (int i=0; i<numNonlinearEqConstraints; i++) {
    constraintMapIndices.push_back(i+numNonlinearIneqConstraints);
    constraintMapMultipliers.push_back(1.0);
    constraintMapOffsets.push_back(-nln_eq_targets[i]);
  }

  for (int i=0; i<numNonlinearIneqConstraints; i++) {
    if (nln_ineq_lwr_bnds[i] > -bigRealBoundSize) {
      numAPPSNonlinearIneqConstraints++;
      constraintMapIndices.push_back(i);
      constraintMapMultipliers.push_back(1.0);
      constraintMapOffsets.push_back(-nln_ineq_lwr_bnds[i]);
    }
    if (nln_ineq_upr_bnds[i] < bigRealBoundSize) {
      numAPPSNonlinearIneqConstraints++;
      constraintMapIndices.push_back(i);
      constraintMapMultipliers.push_back(-1.0);
      constraintMapOffsets.push_back(nln_ineq_upr_bnds[i]);
    }
  }

  evalMgr->set_constraint_map(constraintMapIndices, constraintMapMultipliers, constraintMapOffsets);

  problemParams->setParameter("Number Nonlinear Eqs", (int) numNonlinearEqConstraints);
  problemParams->setParameter("Number Nonlinear Ineqs", (int) numAPPSNonlinearIneqConstraints);
}

} // namespace Dakota
