#include "DakotaROLInterface.hpp"

namespace rol_interface {

// ModelInterface::ModelInterface( Dakota::Model& model ) 
ModelInterface::ModelInterface( Optimizer* opt ) 
  : optPtr(opt),
    dakotaModel(opt->iteratedModel),
    evalSet(dakotaModel.current_response().active_set()) { 

  using ROL::makePtr;

  auto grad_type     = dakotaModel.gradient_type();
  auto hess_type     = dakotaModel.hessian_type();
  auto method_src    = dakotaModel.method_source();
  auto interval_type = dakotaModel.interval_type();  

  // initialize ActiveSet request values
  if( grad_type == "numerical" && method_src == "vendor" ) 
    evalSet.request(asVal);
  else if( dakotaModel.hessian_type() == "none" ) 
    evalSet.request(asValGrad);
  else
    evalSet.request(asValGradHess);
  
  if( grad_type == "analytic" || grad_type == "mixed" ||
    ( grad_type == "numerical" && method_src == "dakota" ) ){
    useDefaultDeriv1 = true;
    useDefaultDeriv2 = ( hess_type != "none" );
  } 

  useCenteredDifferences = (interval_type == "central");

  // Initialize the Optimizer
  auto nx = opt->numContinuousVars;
  auto nli = opt->numLinearIneqConstraints;
  auto nle = opt->numLinearEqConstraints;
  auto nni = opt->numNonlinearIneqConstraints;
  auto nne = opt->numNonlinearEqConstraints;
  bool has_bound_constraint = opt->boundConstraintFlag;
  bool has_linear_equality = nle > 0;
  bool has_linear_inequality = nli > 0;
  bool has_nonlinear_equality = nne > 0;
  bool has_nonlinear_inequality = nni > 0;

  auto obj = makePtr<Objective>(modelInterface);

  // Create unconstrained problem
  opt->problem = makePtr<ROL::Problem<Dakota::Real>>(obj,obj->make_opt_vector());

  auto& problem = opt->problem;

  using Bounds = ROL::Bounds<Dakota::Real>;

  if( has_bound_constraint ) {
    auto l = Vector::make_from(dakotaModel.continuous_lower_bounds());
    auto u = Vector::make_from(dakotaModel.continuous_upper_bounds());
    auto bnd = ROL::makePtr<Bounds>(l,u);
    problem->addBoundConstraint(bnd);
  }

  if( has_linear_equality ) {
    auto econ = makePtr<LinearEqualityConstraint>(this);
    problem->addLinearConstraint(econ->get_name(),econ,econ->make_lagrange_multiplier());
  }

  if( has_linear_inequality ) {
    auto icon = makePtr<LinearInequalityConstraint>(this);
    auto l = Vector::make_from(dakotaModel.linear_ineq_constraint_lower_bounds());
    auto u = Vector::make_from(dakotaModel.linear_ineq_constraint_upper_bounds());
    auto ibnd = ROL::makePtr<Bounds>(l,u);
    problem->addLinearConstraint(icon->get_name(),icon,icon->make_lagrange_multiplier(),ibnd);
  }

  if( has_nonlinear_equality ) {
    auto econ = makePtr<NonlinearEqualityConstraint>(this);
    problem->addConstraint(econ->get_name(),econ,econ->make_lagrange_multiplier());
  }

  if( has_nonlinear_inequality ) {
    auto icon = makePtr<NonlinearInequalityConstraint>(this);
    auto l = Vector::make_from(dakotaModel.nonlinear_ineq_constraint_lower_bounds());
    auto u = Vector::make_from(dakotaModel.nonlinear_ineq_constraint_upper_bounds());
    auto ibnd = ROL::makePtr<Bounds>(l,u);
    problem->addConstraint(icon->get_name(),icon,icon->make_lagrange_multiplier(),ibnd);
  }

  // Set initial value of ROL::Problem optimization vector from Dakota::Model
  auto& x_opt = get_vector(problem->getPrimalOptimizationVector());
  x_opt = dakotaModel.continuous_variables();

  set_default_parameters(opt);

}

void ModelInterface::set_default_parameters( Optimizer* opt ) { 
  auto& param   = opt->parList;
  auto& general = param.sublist("General");

  // If the user has specified "no_hessians", tell ROL to use its own
  // Hessian approximation.
  auto& secant = general.sublist("Secant");
  if (dakotaModel.hessian_type() == "none") {
    secant.set("Type", "Limited-Memory BFGS");
    secant.set("Use as Hessian", true);
  }

  auto& auglag = params.sublist("Step").sublist("Augmented Lagrangian");

  // Turns off adaptively choosing initial penalty parameters
  // New ROL capabaility that results in slower convergence overall
  auglag.set("Use Default Initial Penalty Parameter",false);

  // Turns off automatic constraint and objective scaling
  // New ROL capabaility that results in slower convergence overall
  auglag.set("Use Default Problem Scaling",false);

  // QUESTION: Is there a reason this is only for problem TYPE_EB?
  // Set the verbosity level.
  if (outputLevel >= 2) {
    auglag.set("Print Intermediate Optimization History",true);
  }

  // PRECEDENCE 2: Dakota input file settings

  // Set the verbosity level.
  bool has_verbose_output = Dakota::Iterator::outputLevel >= Dakota::VERBOSE_OUTPUT;
  bool has_normal_output  = Dakota::Iterator::outputLevel >= Dakota::NORMAL_OUTPUT;

  general.set("Print Verbosity", has_verbose_output );

  auto& prob_desc_db = opt->probDescDB;

  auto& status = param.sublist("Status Test");
  status.set("Gradient Tolerance",   prob_desc_db.get_real("method.gradient_tolerance")  );
  status.set("Constraint Tolerance", prob_desc_db.get_real("method.constraint_tolerance"));
  status.set("Step Tolerance",       prob_desc_db.get_real("method.variable_tolerance")  );
  // ROL enforces an int; cast is Ok since SZ_MAX default removed at Minimizer
  status.set("Iteration Limit", static_cast<int>(maxIterations));

  // PRECEDENCE 3: power-user advanced options

  // Check for ROL XML input file.
  String adv_opts_file = prob_desc_db.get_string("method.advanced_options_file");
  if (!adv_opts_file.empty()) {
    if (std::filesystem::exists(adv_opts_file)) {
      if( has_normal_output ) {
               Cout << "Any ROL options in file '" << adv_opts_file
                 << "' will override Dakota options." << std::endl;
    }
    else {
      Cerr << "\nError: ROL options_file '" << adv_opts_file
           << "' specified, but file not found.\n";
      abort_handler(METHOD_ERROR);
    }
    // Set ROL solver parameters based on the XML input.  Overrides
    // anything previously set.
    bool success;
    try {
      ROL::Ptr<ROL::ParameterList> osp_ptr(&optSolverParams);
      ROL::updateParametersFromXmlFile(adv_opts_file, osp_ptr);
      if( has_verbose_output ) {
        Cout << "ROL OptimizationSolver parameters:\n";
           optSolverParams.print(Cout, 2, true, true);
      }
    }
    TEUCHOS_STANDARD_CATCH_STATEMENTS(has_verbose_output, Cerr, success);
  }

} // ModelInterface::set_default_parameters


void ModelInterface::set_value( NonlinearInequalityConstraint* con ) {
  const auto& resp = dakotaModel.current_response();
  int num_opt = dakotaModel.cv();
  const Dakota::RealVector& f = resp.function_values();
  auto fvalues = f.values() + 1;
  con->set_value(fvalues);  
}

void ModelInterface::set_value( NonlinearEqualityConstraint* con ) {
  const auto& resp = dakotaModel.current_response();
  int num_opt = dakotaModel.cv();
  int num_ineq_con = dakotaModel.num_nonlinear_ineq_constraints();
  const Dakota::RealVector& f = resp.function_values();
  auto fvalues = f.values() + 1 + num_ineq_con;
  con->set_value(fvalues);  
}

void ModelInterface::set_jacobian( LinearInequalityConstraint* con ) {
  const Dakota::RealMatrix& J = dakotaModel.linear_ineq_constraint_coeffs();
  con->set_jacobian(J.values());  
}

void ModelInterface::set_jacobian( LinearEqualityConstraint* con ) {
  const Dakota::RealMatrix& J = dakotaModel.linear_eq_constraint_coeffs();
  con->set_jacobian(J.values());  
}

void ModelInterface::set_jacobian( NonlinearInequalityConstraint* con ) {
  const auto& resp = dakotaModel.current_response();
  int num_opt = dakotaModel.cv();
  const Dakota::RealMatrix& J = resp.function_gradients();
  // The inequality constraint jacobian data begins after the objective gradient data
  auto jvalues = J.values() + num_opt;
  con->set_jacobian(jvalues);  
}

void ModelInterface::set_jacobian( NonlinearEqualityConstraint* con ) {
  const auto& resp = dakotaModel->current_response();
  int num_opt = dakotaModel.cv();
  int num_ineq_con = dakotaModel.num_nonlinear_ineq_constraints();
  const Dakota::RealMatrix& J = resp.function_gradients();
  // The equality constraint jacobian data begins after the objective gradient data and 
  // the inequality constraint jacobian data (if it exists)
  auto jvalues = J.values() + num_opt*(1+num_ineq_con);
  con->set_jacobian(jvalues);  
}



ModelInterface::update( const Dakota::RealVector& x,
                              UpdateType          type,
                              int                 iter ) {
           
//  switch(type) { 
//    case UpdateType::Initial:
//      break;
//
//    case UpdateType::Accept:
//      break;
//
//    case UpdateType::Revert:
//      break;
//
//    case UpdateType::Trial:
//      break;
//
//    case UpdateType::Temp:
//    default:
//      break;
//  }
  evalSet = dakotaModel.current_response().active_set();
  dakotaModel.continuous_variables(x);
  dakotaModel.evaluate(evalSet);
} // ModelInterface::update

void ModelInterface::set_dimension( Objective* obj ) {
  obj->set_dimension(dakotaModel.cv());
} 

void ModelInterface::set_dimensions( LinearInequalityConstraint* con ) {
  con->set_dimensions(dakotaModel.cv(), dakotaModel.num_linear_ineq_constraints());
}

void ModelInterface::set_dimensions( LinearEqualityConstraint* con ) {
  con->set_dimensions(dakotaModel.cv(), dakotaModel.num_linear_eq_constraints());
}

void ModelInterface::set_dimensions( NonlinearInequalityConstraint* con ) {
  con->set_dimensions(dakotaModel.cv(), dakotaModel.num_nonlinear_ineq_constraints());
}

void ModelInterface::set_dimensions( NonlinearEqualityConstraint* con ) {
  con->set_dimensions(dakotaModel.cv(), dakotaModel.num_nonlinear_eq_constraints());
}


const Dakota::RealVector& ModelInterface::get_values() const noexcept {
  return dakotaModel.current_response().function_values();
}

const Dakota::RealMatrix& ModelInterface::get_gradients() const noexcept {
  return dakotaModel.current_response().function_gradients();
}

const Dakota::RealSymMatrixArray& ModelInterface::get_hessians() const noexcept {
  return dakotaModel.current_response().function_hessians();
}

} // namespace rol_interface
