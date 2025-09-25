#include "DakotaROLInterface.hpp"
#include "model_utils.hpp"
#include <stdexcept>
#include <filesystem>
#include "Teuchos_XMLParameterListHelpers.hpp" 

namespace rol_interface {

// Standard constructor for Optimizer.  Sets up ROL solver based on
// information from the problem database.
Optimizer::Optimizer(       Dakota::ProblemDescDB&          problem_db, 
                      const std::shared_ptr<Dakota::Model>& model ) :
  Dakota::Optimizer( Traits::create() )
  {
} // Standard constructor  


// Alternate constructor for Iterator instantiations by name.  Sets up
// ROL solver based on information passed as arguments.
Optimizer::Optimizer( const Dakota::String&                 method_string, 
                      const std::shared_ptr<Dakota::Model>& model ):
  Dakota::Optimizer( Traits::create() ) {
} // alternate constructor


// core_run redefines the Optimizer virtual function to perform the
// optimization using ROL and catalogue the results.
void Optimizer::core_run() {

  // Validate gradient settings (following old ROLOptimizer pattern)
  const Dakota::String& grad_type = iteratedModel->gradient_type();
  const Dakota::String& method_src = iteratedModel->method_source();
  
  if (grad_type == "none") {
    Cerr << "\nError: gradient type = none is invalid with ROL.\n"
         << "Please select numerical, analytic, or mixed gradients.\n";
    throw std::runtime_error("Invalid gradient type for ROL optimizer");
  }
  
  // Calculate constraint counts
  size_t num_eq_const = numLinearEqConstraints + numNonlinearEqConstraints;
  size_t num_ineq_const = numLinearIneqConstraints + numNonlinearIneqConstraints;
  
  // Create ROL objective and constraints from the Dakota model
  auto obj = rol_interface::Objective::createFromModel(*iteratedModel);
  auto constraints = rol_interface::Constraint::createSetFromModel(*iteratedModel);
  
  // Create initial optimization vector
  auto x = rol_interface::make_vector(iteratedModel->current_variables().cv());
  auto& x_vec = rol_interface::as_dakota_vector(*x);
  x_vec = iteratedModel->current_variables().continuous_variables();
  
  // Set up the ROL problem
  auto problem = ROL::makePtr<ROL::Problem<Dakota::Real>>(obj, x);
  
  // Add constraints with proper sizing
  if (constraints.linearEquality && numLinearEqConstraints > 0) {
    auto leq_mul = rol_interface::make_vector(numLinearEqConstraints);
    auto leq_res = rol_interface::make_vector(numLinearEqConstraints);
    problem->addConstraint("linear_equality", constraints.linearEquality, leq_mul, leq_res);
  }
  
  if (constraints.nonlinearEquality && numNonlinearEqConstraints > 0) {
    auto nleq_mul = rol_interface::make_vector(numNonlinearEqConstraints);
    auto nleq_res = rol_interface::make_vector(numNonlinearEqConstraints);
    problem->addConstraint("nonlinear_equality", constraints.nonlinearEquality, nleq_mul, nleq_res);
  }
  
  if (constraints.linearInequality && numLinearIneqConstraints > 0) {
    auto li_mul = rol_interface::make_vector(numLinearIneqConstraints);
    auto li_res = rol_interface::make_vector(numLinearIneqConstraints);
    
    // Create appropriate bounds for inequality constraint
    auto li_lower = rol_interface::make_vector(numLinearIneqConstraints);
    auto li_upper = rol_interface::make_vector(numLinearIneqConstraints);
    auto& li_l = rol_interface::as_dakota_vector(*li_lower);
    auto& li_u = rol_interface::as_dakota_vector(*li_upper);
    
    // Set bounds from Dakota constraint bounds (using ModelUtils)
    const Dakota::RealVector& li_lower_bnds = Dakota::ModelUtils::linear_ineq_constraint_lower_bounds(*iteratedModel);
    const Dakota::RealVector& li_upper_bnds = Dakota::ModelUtils::linear_ineq_constraint_upper_bounds(*iteratedModel);
    for (size_t i = 0; i < numLinearIneqConstraints; ++i) {
      li_l[i] = li_lower_bnds[i];
      li_u[i] = li_upper_bnds[i];
    }
    
    auto li_bnd = ROL::makePtr<ROL::Bounds<Dakota::Real>>(li_lower, li_upper);
    problem->addConstraint("linear_inequality", constraints.linearInequality, li_mul, li_bnd, li_res);
  }
  
  if (constraints.nonlinearInequality && numNonlinearIneqConstraints > 0) {
    auto nli_mul = rol_interface::make_vector(numNonlinearIneqConstraints);
    auto nli_res = rol_interface::make_vector(numNonlinearIneqConstraints);
    
    // Create appropriate bounds for inequality constraint
    auto nli_lower = rol_interface::make_vector(numNonlinearIneqConstraints);
    auto nli_upper = rol_interface::make_vector(numNonlinearIneqConstraints);
    auto& nli_l = rol_interface::as_dakota_vector(*nli_lower);
    auto& nli_u = rol_interface::as_dakota_vector(*nli_upper);
    
    // Set bounds from Dakota constraint bounds (using ModelUtils)
    const Dakota::RealVector& nli_lower_bnds = Dakota::ModelUtils::nonlinear_ineq_constraint_lower_bounds(*iteratedModel);
    const Dakota::RealVector& nli_upper_bnds = Dakota::ModelUtils::nonlinear_ineq_constraint_upper_bounds(*iteratedModel);
    for (size_t i = 0; i < numNonlinearIneqConstraints; ++i) {
      nli_l[i] = nli_lower_bnds[i];
      nli_u[i] = nli_upper_bnds[i];
    }
    
    auto nli_bnd = ROL::makePtr<ROL::Bounds<Dakota::Real>>(nli_lower, nli_upper);
    problem->addConstraint("nonlinear_inequality", constraints.nonlinearInequality, nli_mul, nli_bnd, nli_res);
  }
  
  // Set ROL parameters (including XML file if specified)
  set_rol_parameters();
  
  // Prepare output stream with ROL prefix
  { // Scoped section for output filtering
    rol_interface::OutputStreamFilter rol_cout(Cout);
    
    // Finalize the problem
    problem->finalize(false, true, rol_cout.stream());
    
    // Create and run the ROL solver using proper parameter list
    ROL::Solver<Dakota::Real> solver(problem, parList);
    solver.solve(rol_cout.stream());
  }
  
  // Extract the optimized variables and store in Dakota's result arrays
  Dakota::Variables& best_vars = bestVariablesArray.front();
  Dakota::Response& best_resp = bestResponseArray.front();
  
  // Copy optimized variables back to Dakota
  const auto& x_opt = rol_interface::as_dakota_vector(*x);
  best_vars.continuous_variables(x_opt);
  
  // Try database lookup first (optimization from old ROLOptimizer)
  Dakota::ActiveSet search_set(best_resp.active_set());
  search_set.request_values(1); // Function values only
  best_resp.active_set(search_set);
  bool db_found = iteratedModel->db_lookup(best_vars, search_set, best_resp);
  
  if (db_found) {
    Cout << "INFO: ROL retrieved best response from cache." << std::endl;
  } else {
    Cout << "INFO: ROL re-evaluating model to retrieve best response." << std::endl;
    
    // Evaluate the model at the optimal point to get the final response
    Dakota::ModelUtils::continuous_variables(*iteratedModel, x_opt);
    iteratedModel->evaluate();
    const Dakota::RealVector& best_fns = iteratedModel->current_response().function_values();
    best_resp.function_values(best_fns);
  }

} // core_run


void Optimizer::set_rol_parameters() {
  // Set default ROL parameters first
  
  // Set the solver based on the type of problem
  size_t num_eq_const = numLinearEqConstraints + numNonlinearEqConstraints;
  size_t num_ineq_const = numLinearIneqConstraints + numNonlinearIneqConstraints;
  
  if (num_eq_const == 0 && num_ineq_const == 0) {
    // Unconstrained
    parList.sublist("Step").set("Type","Trust Region");
    parList.sublist("Step").sublist("Trust Region").set("Subproblem Solver", "Truncated CG");
  }
  else if (num_eq_const > 0 && num_ineq_const == 0) {
    // Equality constrained
    parList.sublist("Step").set("Type","Composite Step");
  }
  else {
    // General constrained (has inequalities)
    parList.sublist("Step").set("Type","Augmented Lagrangian");
    parList.sublist("Step").sublist("Augmented Lagrangian").set("Subproblem Solver", "Trust Region");
  }

  // Set stopping criteria from Dakota
  parList.sublist("Status Test").set("Gradient Tolerance", convergenceTol);
  parList.sublist("Status Test").set("Constraint Tolerance", constraintTol);  
  parList.sublist("Status Test").set("Iteration Limit", (int)maxIterations);
  
  // Set verbosity
  parList.sublist("General").set("Print Verbosity", outputLevel < Dakota::VERBOSE_OUTPUT ? 0 : 1);

  // Check for ROL XML input file (following old ROLOptimizer pattern)
  Dakota::String adv_opts_file = probDescDB.get_string("method.advanced_options_file");
  if (!adv_opts_file.empty()) {
    if (std::filesystem::exists(adv_opts_file)) {
      if (outputLevel >= Dakota::NORMAL_OUTPUT) {
        Cout << "Any ROL options in file '" << adv_opts_file
             << "' will override Dakota options." << std::endl;
      }
    }
    else {
      Cerr << "\nError: ROL options_file '" << adv_opts_file
           << "' specified, but file not found.\n";
      abort_handler(Dakota::METHOD_ERROR);
    }

    // Set ROL solver parameters based on the XML input. Overrides anything previously set.
    try {
      Teuchos::Ptr<Teuchos::ParameterList> parList_ptr(&parList);
      Teuchos::updateParametersFromXmlFile(adv_opts_file, parList_ptr);
      if (outputLevel >= Dakota::VERBOSE_OUTPUT) {
        Cout << "ROL optimization parameters:\n";
        parList.print(Cout, 2, true, true);
      }
    }
    catch (const std::exception& e) {
      Cerr << "\nError reading ROL XML file '" << adv_opts_file << "': " 
           << e.what() << std::endl;
      abort_handler(Dakota::METHOD_ERROR);
    }
  }
}


} // namespace rol_interface

