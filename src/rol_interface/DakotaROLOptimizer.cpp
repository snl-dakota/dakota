#include "DakotaROLInterface.hpp"

namespace rol_interface {

Optimizer::Optimizer( Dakota::ProblemDescDB& problem_db,
                      Dakota::Model&         model )
  : Dakota::Optimizer(problem_db, model, ROLTraits::create()),
  modelCache(model) {
  Initializer::initialize(*this);
} 



// -----------------------------------------------------------------
/** Implementation of Optimizer::Initializer class. */

void Optimizer::Initializer::initialize( Optimizer* opt ) {

  using ROL::makePtr;
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
  auto& model = opt->iteratedModel;
  auto& cache = opt->modelCache;
  auto obj = makePtr<Objective>(cache);

  // Create unconstrained problem
  opt->problem = makePtr<ROL::Problem<Dakota::Real>>(obj,obj->make_opt_vector());

  auto& problem = opt->problem;

  if( has_bound_constraint ) {
    auto bnd = Bounds::make_continuous_variable(model);
    problem->addBoundConstraint(bnd);
  }

  if( has_linear_equality ) {
    auto A = Jacobian::make_equality(cache);
    auto b = make_vector(nle);
    auto emul = b->dual().clone();
    auto& b_ref = get_vector(b);
    b_ref = model.linear_eq_constraint_targets();
    b->scale(-1.0);
    auto econ = makePtr<ROL::LinearConstraint<Dakota::Real>>(A,b);
    problem->addLinearConstraint("Linear Equality Constraint",econ,emul);
  }   

  if( has_linear_inequality ) {
    auto A = Jacobian::make_inequality(cache);
    auto z = make_vector(nli);
    z->zero();
    auto icon = makePtr<ROL::LinearConstraint<Dakota::Real>>(A,z);
    auto ibnd = Bounds::make_linear_ineq_constraint(model);
    auto imul = l->dual().clone();
    problem->addLinearConstraint("Linear Inequality Constraint",icon,imul,ibnd);
  }   

  if( has_nonlinear_equality ) {
    auto econ = Constraint::make_equality(cache);
    auto emul = econ->create_multiplier();  
    problem->addConstraint("Nonlinear Equality Constraint",econ,emul);
  }   

  if( has_nonlinear_inequality ) {
    auto icon = Constraint::make_inequality(cache);
    auto imul = icon->create_multiplier();  
    auto ibnd = Bounds::make_nonlinear_ineq_constraint(model);
    problem->addConstraint("Nonlinear Inequality Constraint",icon,imul,ibnd);
  }   

  // Set initial value of ROL::Problem optimization vector from Dakota::Model
  auto& x_opt = get_vector(problem->getPrimalOptimizationVector());
  x_opt = model.continuous_variables();

  set_default_parameters(opt);

} // Optimizer::Initializer::initialize






void Optimizer::Initializer::set_default_parameters( Optimizer* opt ) {
  
  auto& param   = opt->parList;
  auto& general = param.sublist("General");

  // If the user has specified "no_hessians", tell ROL to use its own
  // Hessian approximation.
  auto& secant = general.sublist("Secant");
  if (model.hessian_type() == "none") {
    secant.set("Type", "Limited-Memory BFGS");
    sectant.set("Use as Hessian", true);
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
  if (outputLevel >= VERBOSE_OUTPUT)
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
    if (boost::filesystem::exists(adv_opts_file)) {
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

} // Optimizer::Initializer::set_default_parameters


// Standard constructor for Optimizer.  Sets up ROL solver based on
// information from the problem database.
Optimizer::Optimizer( Dakota::ProblemDescDB& problem_db, 
                            Dakota::Model&         model) :
  Dakota::Optimizer( problem_db, 
                     model, 
                     ROLTraits::create() ) {
  Initializer::initialize(this);
} // Standard constructor  


// Alternate constructor for Iterator instantiations by name.  Sets up
// ROL solver based on information passed as arguments.
Optimizer::Optimizer( const String& method_string, 
                                  Model&  model):
  Dakota::Optimizer( Dakota::method_string_to_enum(method_string), 
                     model, 
                     ROLTraits::create() ) {
  Initializer::initialize(this);
} // alternate constructor


// core_run redefines the Optimizer virtual function to perform the
// optimization using ROL and catalogue the results.
void Optimizer::core_run() {

  { // Prepend output in this section with "ROL: "
    auto rol_cout = OutputStreamFilter(); 
    auto opt_solver = ROL::Solver<Real>( problem, parList );
    opt_solver.solve(rol_cout);
  } 

  auto& best_vars = bestVariablesArray.front();
  auto& x_opt = get_vector(problem->getPrimalOptimizationVector());
  x_opt = best_vars.continuous_variables_view();

} // core_run


} // namespace rol_interface

