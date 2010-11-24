/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       APPSOptimizer
//- Description: Wrapper class for APPSPACK
//- Owner:       Patty Hough
//- Checked by:
//- Version: $Id

#include "ProblemDescDB.H"
#include "APPSOptimizer.H"
#include "HOPSPACK_float.hpp"

namespace Dakota {

/// Wrapper class for APPSPACK 

/** The APPSOptimizer class provides a wrapper for APPSPACK, a
    Sandia-developed C++ library for generalized pattern search.
    APPSPACK defaults to a coordinate pattern search but also allows
    for augmented search patterns.  It can solve problems with bounds,
    linear constraints, and general nonlinear constraints.
    APPSOptimizer uses an APPSEvalMgr object to manage the function
    evaluations.

    The user input mappings are as follows: \c output \c
    max_function_evaluations, \c constraint_tol \c initial_delta, \c
    contraction_factor, \c threshold_delta, \c solution_target, \c
    synchronization, \c merit_function, \c constraint_penalty, and \c
    smoothing_factor are mapped into APPS's \c "Debug", "Maximum
    Evaluations", "Bounds Tolerance"/"Machine Epsilon"/"Constraint
    Tolerance", "Initial Step", "Contraction Factor", "Step
    Tolerance", "Function Tolerance", "Synchronous", "Method",
    "Initial Penalty Value", and "Initial Smoothing Value" data
    attributes.  Refer to the APPS web site
    (http://software.sandia.gov/appspack) for additional information
    on APPS objects and controls. */

APPSOptimizer::APPSOptimizer(Model& model): Optimizer(model)
{
  // (iteratedModel initialized in Optimizer(Model&))

  evalMgr = new APPSEvalMgr(iteratedModel);
  set_apps_parameters(); // set specification values using DB

  // The following is not performed in the Optimizer constructor since
  // maxConcurrency is updated within set_method_parameters().  The
  // matching free_communicators() appears in the Optimizer destructor.

  if (scaleFlag || multiObjFlag)
    iteratedModel.init_communicators(maxConcurrency);
}

APPSOptimizer::APPSOptimizer(NoDBBaseConstructor, Model& model):
  Optimizer(NoDBBaseConstructor(), model)
{
  // (iteratedModel initialized in Optimizer(Model&))

  evalMgr = new APPSEvalMgr(iteratedModel);
  set_apps_parameters(); // set specification values using DB

  // The following is not performed in the Optimizer constructor since
  // maxConcurrency is updated within set_method_parameters().  The
  // matching free_communicators() appears in the Optimizer destructor.

  if (scaleFlag || multiObjFlag)
    iteratedModel.init_communicators(maxConcurrency);
}

/** find_optimum redefines the Optimizer virtual function to perform
    the optimization using APPS. It first sets up the problem data,
    then executes minimize() on the APPS optimizer, and finally
    catalogues the results. */
void APPSOptimizer::find_optimum()
{
  evalMgr->set_asynch_flag(iteratedModel.asynch_flag());
  evalMgr->set_total_workers(iteratedModel.evaluation_capacity());
  initialize_variables_and_constraints();

  // Instantiate optimizer and solve.

  HOPSPACK::Hopspack optimizer(evalMgr);
  optimizer.setInputParameters(params);
  optimizer.solve();

  // Retrieve best iterate and convert from APPS vector to DAKOTA
  // vector.
  std::vector<double> bestX(numContinuousVars);
  bool state = optimizer.getBestX(bestX);
  RealVector variableHolder(numContinuousVars, false);
  for (int i=0; i<numContinuousVars; i++)
    variableHolder[i] = bestX[i];
  bestVariablesArray.front().continuous_variables(variableHolder);

  // Retrieve the best responses and convert from APPS vector to
  // DAKOTA vector.
  if (!multiObjFlag) { // else multi_objective_retrieve() is used in
                       // Optimizer::post_run()
    std::vector<double> bestEqs(numNonlinearEqConstraints);
    std::vector<double> bestIneqs(constraintMapIndices.size()-numNonlinearEqConstraints);
    RealVector best_fns(numFunctions);
    best_fns[0] = optimizer.getBestF();
    if (numNonlinearEqConstraints > 0) {
      optimizer.getBestNonlEqs(bestEqs);
      for (int i=0; i<numNonlinearEqConstraints; i++)
	best_fns[constraintMapIndices[i]+1] = (bestEqs[i] -
					       constraintMapOffsets[i]) /
	                                       constraintMapMultipliers[i];
    }
    if (numNonlinearIneqConstraints > 0) {
      optimizer.getBestNonlIneqs(bestIneqs);
      for (int i=0; i<bestIneqs.size(); i++)
	best_fns[constraintMapIndices[i+numNonlinearEqConstraints]+1] = (bestIneqs[i] -
				       constraintMapOffsets[i+numNonlinearEqConstraints]) /
                                       constraintMapMultipliers[i+numNonlinearEqConstraints];
    }
    bestResponseArray.front().function_values(best_fns);
  }
}

/** Set all of the APPS algorithmic parameters as specified in the
    DAKOTA input deck.  This is called at construction time. */
void APPSOptimizer::set_apps_parameters()
{
  // Set APPS parameters from DAKOTA input deck values.

  HOPSPACK::Vector scales(numContinuousVars);

  problemParams = &(params.getOrSetSublist("Problem Definition"));
  linearParams = &(params.getOrSetSublist("Linear Constraints"));
  mediatorParams = &(params.getOrSetSublist("Mediator"));
  citizenParams = &(params.getOrSetSublist("Citizen 1"));

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
  case VERBOSE_OUTPUT:
    problemParams->setParameter("Display", 2);
    linearParams->setParameter("Display", 2);
    mediatorParams->setParameter("Display", 4);
    if (numNonlinearConstraints > 0) {
      citizenParams->setParameter("Display", 2);
      citizenParams->setParameter("Display Subproblem", 3);
    }
    else
      citizenParams->setParameter("Display", 3);
  case NORMAL_OUTPUT:
    problemParams->setParameter("Display", 1);
    linearParams->setParameter("Display", 1);
    mediatorParams->setParameter("Display", 3);
    if (numNonlinearConstraints > 0) {
      citizenParams->setParameter("Display", 1);
      citizenParams->setParameter("Display Subproblem", 2);
    }
    else
      citizenParams->setParameter("Display", 2);
  case QUIET_OUTPUT:
    problemParams->setParameter("Display", 1);
    linearParams->setParameter("Display", 1);
    mediatorParams->setParameter("Display", 2);
    if (numNonlinearConstraints > 0) {
      citizenParams->setParameter("Display", 1);
      citizenParams->setParameter("Display Subproblem", 1);
    }
    else
      citizenParams->setParameter("Display", 1);
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

  mediatorParams->setParameter("Citizen Count", 1);
  if (numNonlinearConstraints > 0)
    citizenParams->setParameter("Type", "GSS-NLC");
  else
    citizenParams->setParameter("Type", "GSS");

  citizenParams->setParameter("Maximum Evaluations", maxFunctionEvals);
  if (numNonlinearConstraints > 0)
    citizenParams->setParameter("Maximum Evaluations", maxFunctionEvals);

  if (constraintTol > 0.0) {
    //    params.sublist("Solver").setParameter("Bounds Tolerance", constraintTol);
    linearParams->setParameter("Active Tolerance", constraintTol);
    citizenParams->setParameter("Nonlinear Active Tolerance", constraintTol);
  }

  if (probDescDB.is_null()) { // instantiate on-the-fly
    // rely on internal APPS defaults for the most part, but set any
    // default overrides (including enforcement of DAKOTA defaults) here
  }
  else {

    const Real& init_step_length
      = probDescDB.get_real("method.asynch_pattern_search.initial_delta");
    if (init_step_length >= 0.0)
      citizenParams->setParameter("Initial Step", init_step_length);

    const Real& contract_step_length
      = probDescDB.get_real("method.asynch_pattern_search.contraction_factor");
    citizenParams->setParameter("Contraction Factor", contract_step_length);

    const Real& thresh_step_length
      = probDescDB.get_real("method.asynch_pattern_search.threshold_delta");
    if (thresh_step_length >= 0.0)
      citizenParams->setParameter("Step Tolerance", thresh_step_length);

    const Real& solution_target
      = probDescDB.get_real("method.solution_target");
    if (solution_target!=-1.e+25)
      problemParams->setParameter("Objective Target", solution_target);

    // A null string is the DB default and nonblocking is the APPS default, so
    // the flag is true only for an explicit blocking user specification.

    const bool blocking_synch = (probDescDB.get_string("method.asynch_pattern_search.synchronization")
		     == "blocking") ? true : false;
    if (blocking_synch) {
      mediatorParams->setParameter("Synchronous Evaluations", true);
      evalMgr->set_blocking_synch(true);
    }
    else
      mediatorParams->setParameter("Synchronous Evaluations", false);

    //const RealVector& variable_scales = probDescDB.get_rdv("variables.continuous_design.scales");
    const RealVector& lower_bnds
      = iteratedModel.continuous_lower_bounds();
    const RealVector& upper_bnds
      = iteratedModel.continuous_upper_bounds();

    if (lower_bnds[0]<=-bigRealBoundSize || upper_bnds[0]>=bigRealBoundSize) {
      for (int i=0; i<numContinuousVars; i++)
	scales[i] = 1.0;
      problemParams->setParameter("Scaling", scales);
    }

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

    Real constr_penalty = probDescDB.get_real("method.asynch_pattern_search.constraint_penalty");
    if (constr_penalty >= 0.)
      citizenParams->setParameter("Penalty Parameter", constr_penalty);

    /*    Real smooth_factor = probDescDB.get_real("method.asynch_pattern_search.smoothing_factor");
    if (smooth_factor >= 0.)
    citizenParams->setParameter("Penalty Smoothing Value", smooth_factor); */

    maxConcurrency *= 2*numContinuousVars;

    // ----------------------------------------------------------------
    // Current APPS is hardwired for coordinate bases, no expansion,
    // and no pattern augmentation.
    //
    // See http://software.sandia.gov/appspack/pageParameters.html
    // for valid specification options.
    //
    // APPS wish list:
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
  // Initialize variables and bounds.  This is performed in find_optimum
  // in order to capture any reassignment at the strategy layer (after
  // iterator construction).  

  HOPSPACK::Vector init_point(numContinuousVars), tmp_vector(numContinuousVars);
  HOPSPACK::Vector lower(numContinuousVars), upper(numContinuousVars);
  HOPSPACK::Vector lin_ineq_lower_bnds(numLinearIneqConstraints);
  HOPSPACK::Vector lin_ineq_upper_bnds(numLinearIneqConstraints);
  HOPSPACK::Vector lin_eq_targets(numLinearEqConstraints);
  HOPSPACK::Matrix lin_ineq_coeffs, lin_eq_coeffs;

  const RealVector& init_pt
    = iteratedModel.continuous_variables();
  const RealVector& lower_bnds
    = iteratedModel.continuous_lower_bounds();
  const RealVector& upper_bnds
    = iteratedModel.continuous_upper_bounds();

  for (int i=0; i<numContinuousVars; i++) {
    init_point[i] = init_pt[i];
    if (lower_bnds[i] > -bigRealBoundSize)
      lower[i] = lower_bnds[i];
    else
      lower[i] = HOPSPACK::dne();
    if (upper_bnds[i] < bigRealBoundSize)
      upper[i] = upper_bnds[i];
    else
      upper[i] = HOPSPACK::dne();
  }

  problemParams->setParameter("Number Unknowns", (int) numContinuousVars);
  problemParams->setParameter("Initial X", init_point);
  problemParams->setParameter("Lower Bounds", lower);
  problemParams->setParameter("Upper Bounds", upper);

  const RealMatrix& linear_ineq_coeffs = iteratedModel.linear_ineq_constraint_coeffs();
  const RealVector& linear_ineq_lower_bnds = iteratedModel.linear_ineq_constraint_lower_bounds();
  const RealVector& linear_ineq_upper_bnds = iteratedModel.linear_ineq_constraint_upper_bounds();
  const RealMatrix& linear_eq_coeffs = iteratedModel.linear_eq_constraint_coeffs();
  const RealVector& linear_eq_targets = iteratedModel.linear_eq_constraint_targets();

  for (int i=0; i<numLinearIneqConstraints; i++) {
    for (int j=0; j<numContinuousVars; j++)
      tmp_vector[j] = linear_ineq_coeffs[i][j];
    lin_ineq_coeffs.addRow(tmp_vector);
    if (linear_ineq_lower_bnds[i] > -bigRealBoundSize)
      lin_ineq_lower_bnds[i] = linear_ineq_lower_bnds[i];
    else
      lin_ineq_lower_bnds[i] = HOPSPACK::dne();
    if (linear_ineq_upper_bnds[i] < bigRealBoundSize)
      lin_ineq_upper_bnds[i] = linear_ineq_upper_bnds[i];
    else
      lin_ineq_upper_bnds[i] = HOPSPACK::dne();
  }
  for (int i=0; i<numLinearEqConstraints; i++) {
    for (int j=0; j<numContinuousVars; j++)
      tmp_vector[j] = linear_eq_coeffs[i][j];
    lin_eq_coeffs.addRow(tmp_vector);
    lin_eq_targets[i] = linear_eq_targets[i];
  }

  linearParams->setParameter("Inequality Matrix", lin_ineq_coeffs);
  linearParams->setParameter("Inequality Lower", lin_ineq_lower_bnds);
  linearParams->setParameter("Inequality Upper", lin_ineq_upper_bnds);
  linearParams->setParameter("Equality Matrix", lin_eq_coeffs);
  linearParams->setParameter("Equality Bounds", lin_eq_targets);

  const RealVector& nln_ineq_lwr_bnds
    = iteratedModel.nonlinear_ineq_constraint_lower_bounds();
  const RealVector& nln_ineq_upr_bnds
    = iteratedModel.nonlinear_ineq_constraint_upper_bounds();
  const RealVector& nln_eq_targets
    = iteratedModel.nonlinear_eq_constraint_targets();

  int numAPPSNonlinearIneqConstraints = 0;

  // APPSPACK expects nonlinear equality constraints to be of the form
  // c(x) = 0 and nonlinear inequality constraints to be of the form
  // c(x) <= 0.  Compute the number of 1-sided nonlinear inequality
  // constraints to pass to APPSPACK (numAPPSNonlinearIneqConstraints)
  // as well as the mappings for both equalities and inequalities
  // (indices, multipliers, offsets) between the DAKOTA constraints
  // and the APPSPACK constraints.

  for (int i=0; i<numNonlinearEqConstraints; i++) {
    constraintMapIndices.push_back(i+numNonlinearIneqConstraints);
    constraintMapMultipliers.push_back(1.0);
    constraintMapOffsets.push_back(-nln_eq_targets[i]);
  }

  for (int i=0; i<numNonlinearIneqConstraints; i++) {
    if (nln_ineq_lwr_bnds[i] > -bigRealBoundSize) {
      numAPPSNonlinearIneqConstraints++;
      constraintMapIndices.push_back(i);
      constraintMapMultipliers.push_back(-1.0);
      constraintMapOffsets.push_back(nln_ineq_lwr_bnds[i]);
    }
    if (nln_ineq_upr_bnds[i] < bigRealBoundSize) {
      numAPPSNonlinearIneqConstraints++;
      constraintMapIndices.push_back(i);
      constraintMapMultipliers.push_back(1.0);
      constraintMapOffsets.push_back(-nln_ineq_upr_bnds[i]);
    }
  }

  evalMgr->set_constraint_map(constraintMapIndices, constraintMapMultipliers, constraintMapOffsets);

  problemParams->setParameter("Number Nonlinear Eqs", (int) numNonlinearEqConstraints);
  problemParams->setParameter("Number Nonlinear Ineqs", (int) numAPPSNonlinearIneqConstraints);
}

} // namespace Dakota
