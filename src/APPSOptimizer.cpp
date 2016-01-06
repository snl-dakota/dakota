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
#include "HOPSPACK_float.hpp"

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

  RealVector contVars(numContinuousVars);
  IntVector  discIntVars(numDiscreteIntVars);
  RealVector discRealVars(numDiscreteRealVars);

  const BitArray& int_set_bits = iteratedModel.discrete_int_sets();
  const IntSetArray& set_int_vars = iteratedModel.discrete_set_int_values();
  const RealSetArray& set_real_vars = iteratedModel.discrete_set_real_values();
  const StringSetArray& set_string_vars = iteratedModel.discrete_set_string_values();

  size_t i, dsi_cntr;

  for(i=0; i<numContinuousVars; i++)
    contVars[i] = bestX[i];
  bestVariablesArray.front().continuous_variables(contVars);

  for(i=0, dsi_cntr=0; i<numDiscreteIntVars; i++)
  { 
    // This active discrete int var is a set type
    // Map from index back to value.
    if (int_set_bits[i]) {
      discIntVars[i] = 
	set_index_to_value(bestX[i+numContinuousVars], set_int_vars[dsi_cntr]);
      ++dsi_cntr;
    }
    // This active discrete int var is a range type
    else
      discIntVars[i] = bestX[i+numContinuousVars];
  }

  // For real sets and strings, map from index back to value.
  bestVariablesArray.front().discrete_int_variables(discIntVars);
  for (i=0; i<numDiscreteRealVars; i++)
    discRealVars = 
      set_index_to_value(bestX[i+numContinuousVars+numDiscreteIntVars], set_real_vars[i]);
  bestVariablesArray.front().discrete_real_variables(discRealVars);

  for (i=0; i<numDiscreteStringVars; i++)
    bestVariablesArray.front().discrete_string_variable(set_index_to_value(bestX[i+numContinuousVars+numDiscreteIntVars+numDiscreteRealVars], set_string_vars[i]), i);

  // Retrieve the best responses and convert from HOPS vector to
  // DAKOTA vector.

  if (!localObjectiveRecast) {
    // else local_objective_recast_retrieve() is used in Optimizer::post_run()

    RealVector best_fns(numFunctions);
    std::vector<double> bestEqs(numNonlinearEqConstraints);
    std::vector<double> bestIneqs(constraintMapIndices.size()-numNonlinearEqConstraints);
    const BoolDeque& max_sense = iteratedModel.primary_response_fn_sense();
    best_fns[0] = (!max_sense.empty() && max_sense[0]) ?
      -optimizer.getBestF() : optimizer.getBestF();
    if (numNonlinearEqConstraints > 0) {
      optimizer.getBestNonlEqs(bestEqs);
      for (i=0; i<numNonlinearEqConstraints; i++)
	best_fns[constraintMapIndices[i]+1] = (bestEqs[i] -
					       constraintMapOffsets[i]) /
	                                       constraintMapMultipliers[i];
    }
    if (numNonlinearIneqConstraints > 0) {
      optimizer.getBestNonlIneqs(bestIneqs);
      for (i=0; i<bestIneqs.size(); i++)
	best_fns[constraintMapIndices[i+numNonlinearEqConstraints]+1] = (bestIneqs[i] -
				       constraintMapOffsets[i+numNonlinearEqConstraints]) /
                                       constraintMapMultipliers[i+numNonlinearEqConstraints];
    }
    bestResponseArray.front().function_values(best_fns);
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

  numTotalVars = numContinuousVars + numDiscreteIntVars 
             + numDiscreteRealVars + numDiscreteStringVars;

  HOPSPACK::Vector init_point(numTotalVars), tmp_vector(numTotalVars);
  HOPSPACK::Vector lower(numTotalVars), upper(numTotalVars);
  HOPSPACK::Vector lin_ineq_lower_bnds(numLinearIneqConstraints);
  HOPSPACK::Vector lin_ineq_upper_bnds(numLinearIneqConstraints);
  HOPSPACK::Vector lin_eq_targets(numLinearEqConstraints);
  HOPSPACK::Matrix lin_ineq_coeffs, lin_eq_coeffs;

  const RealVector& init_pt_cont
    = iteratedModel.continuous_variables();
  const RealVector& lower_bnds_cont
    = iteratedModel.continuous_lower_bounds();
  const RealVector& upper_bnds_cont
    = iteratedModel.continuous_upper_bounds();

  const IntVector& init_pt_int = iteratedModel.discrete_int_variables();
  const IntVector& lower_bnds_int = iteratedModel.discrete_int_lower_bounds();
  const IntVector& upper_bnds_int = iteratedModel.discrete_int_upper_bounds();

  const RealVector& init_pt_real = iteratedModel.discrete_real_variables();
  const RealVector& lower_bnds_real = iteratedModel.discrete_real_lower_bounds();
  const RealVector& upper_bnds_real = iteratedModel.discrete_real_upper_bounds();

  const StringMultiArrayConstView init_pt_string = 
    iteratedModel.discrete_string_variables();

  const BitArray& int_set_bits = iteratedModel.discrete_int_sets();
  const IntSetArray& init_pt_set_int = 
    iteratedModel.discrete_set_int_values();
  const RealSetArray& init_pt_set_real = 
    iteratedModel.discrete_set_real_values();
  const StringSetArray& init_pt_set_string = 
    iteratedModel.discrete_set_string_values();

  vector<char> variable_types;
  bool setScales = false;

  size_t i, index, dsi_cntr;

  for (i=0; i<numContinuousVars; i++) {
    init_point[i] = init_pt_cont[i];
    variable_types.push_back('C');
    if (lower_bnds_cont[i] > -bigRealBoundSize)
      lower[i] = lower_bnds_cont[i];
    else {
      lower[i] = HOPSPACK::dne();
      setScales = true;
    }
    if (upper_bnds_cont[i] < bigRealBoundSize)
      upper[i] = upper_bnds_cont[i];
    else {
      upper[i] = HOPSPACK::dne();
      setScales = true;
    }
  }
  for(i=0, dsi_cntr=0; i<numDiscreteIntVars; i++)
  {
    if (int_set_bits[i]) {
      index = set_value_to_index(init_pt_int[i], 
				 init_pt_set_int[dsi_cntr]);
      if (index == _NPOS) {
	Cerr << "\nError: failure in discrete integer set lookup within "
	     << "NomadOptimizer::load_parameters(Model &model)" << std::endl;
	abort_handler(-1);
      }
      else {
	init_point[i+numContinuousVars] = (int)index;
	variable_types.push_back('C');
      }
      lower[i+numContinuousVars] = 0;
      upper[i+numContinuousVars] = 
	init_pt_set_int[dsi_cntr].size() - 1;
      ++dsi_cntr;
    }
    else 
    {
      init_point[i+numContinuousVars] = init_pt_int[i];
      variable_types.push_back('C');
      if (lower_bnds_int[i] > -bigIntBoundSize)
	lower[i+numContinuousVars] = lower_bnds_int[i];
      else
      {
	lower[i] = HOPSPACK::dne();
	setScales = true;
      }
      if (upper_bnds_int[i] < bigIntBoundSize)
	upper[i+numContinuousVars] = upper_bnds_int[i];
      else
      {
	upper[i] = HOPSPACK::dne();
	setScales = true;
      }
    }
  }
  for (i=0; i<numDiscreteRealVars; i++) {
    index = set_value_to_index(init_pt_real[i], 
			       init_pt_set_real[i]);
    if (index == _NPOS) {
      Cerr << "\nError: failure in discrete real set lookup within "
	   << "NomadOptimizer::load_parameters(Model &model)" << std::endl;
      abort_handler(-1);
    }
    else {
      init_point[i+numContinuousVars+numDiscreteIntVars] = (int)index;
      variable_types.push_back('C');
    }
    lower[i+numContinuousVars+numDiscreteIntVars] = 0;
    upper[i+numContinuousVars+numDiscreteIntVars] = 
	init_pt_set_real[i].size() - 1;
  }
  for (i=0; i<numDiscreteStringVars; i++) {
    index = set_value_to_index(init_pt_string[i], 
			       init_pt_set_string[i]);
    if (index == _NPOS) {
      Cerr << "\nError: failure in discrete string set lookup within "
	   << "NomadOptimizer::load_parameters(Model &model)" << std::endl;
      abort_handler(-1);
    }
    else {
      init_point[i+numContinuousVars+numDiscreteIntVars+numDiscreteRealVars] =
	(int)index;
      variable_types.push_back('C');
    }
    lower[i+numContinuousVars+numDiscreteIntVars+numDiscreteRealVars] = 0;
    upper[i+numContinuousVars+numDiscreteIntVars+numDiscreteRealVars] = 
	init_pt_set_string[i].size() - 1;
  }

  problemParams->setParameter("Number Unknowns", (int) numTotalVars);
  problemParams->setParameter("Variable Types", variable_types);
  problemParams->setParameter("Initial X", init_point);
  problemParams->setParameter("Lower Bounds", lower);
  problemParams->setParameter("Upper Bounds", upper);

  // If there are no bound constraints, HOPSPACK requires that scaling be provided.

  if (setScales) {
    HOPSPACK::Vector scales(numContinuousVars);
    for (int i=0; i<numContinuousVars; i++)
      scales[i] = 1.0;
    problemParams->setParameter("Scaling", scales);
  }

  // Define linear equality and inequality constraints.

  const RealMatrix& linear_ineq_coeffs = iteratedModel.linear_ineq_constraint_coeffs();
  const RealVector& linear_ineq_lower_bnds = iteratedModel.linear_ineq_constraint_lower_bounds();
  const RealVector& linear_ineq_upper_bnds = iteratedModel.linear_ineq_constraint_upper_bounds();
  const RealMatrix& linear_eq_coeffs = iteratedModel.linear_eq_constraint_coeffs();
  const RealVector& linear_eq_targets = iteratedModel.linear_eq_constraint_targets();

  for (int i=0; i<numLinearIneqConstraints; i++) {
    for (int j=0; j<numContinuousVars; j++) 
      tmp_vector[j] = linear_ineq_coeffs(i,j);
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
      tmp_vector[j] = linear_eq_coeffs(i,j);
    lin_eq_coeffs.addRow(tmp_vector);
    lin_eq_targets[i] = linear_eq_targets[i];
  }

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
