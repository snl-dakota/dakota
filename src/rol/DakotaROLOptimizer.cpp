namespace Dakota {
namespace rol_interface {


ROLOptimizer::ROLOptimizer( Dakota::ProblemDescDB& problem_db,
                            Dakota::Model&         model )
  : Dakota::Optimzizer(problem_db, model, ROLTraits::create()),
  modelCache(model) {
  Initializer::initialize(*this);
} 



// -----------------------------------------------------------------
/** Implementation of ROLOptimizer::Initializer class. */

void ROLOptimizer::Initializer::initialize( ROLOptimizer& opt ) {

  using ROL::makePtr;

  auto num_econ = opt->numLinearEqConstraints   
                + opt->numNonlinearEqConstraints;

  auto num_icon = opt->numLinearIneqConstraints 
                + opt->numNonlinearIneqConstraints;

  auto make_vector = []( size_t dim ) {
    return makePtr<vector_type>(dim,0.0);
  };

  opt->xSol = make_vector(opt->numContinuousVars);


  opt->objFun = makePtr<

  if( opt->boundConstraintFlag ) {
    opt->lBnd = make_vector(opt->numContinuousVars);
    opt->uBnd = make_vector(opt->numContinuousVars);
    opt->bndCon = ROL::makePtr<ROL::Bounds>(
  }

  opt->eMul = make_vector(num_econ);

  opt->iMul = make_vector(num_icon);


  auto& x_ref = get_vector(opt->xSol);
  Optimizer::get_initial_values(Iterator::iteratedModel,x_ref );




























1

} // ROLOptimizer::Initializer::initialize

// -----------------------------------------------------------------
/** Implementation of ROLOptimizer class. */


// Standard constructor for ROLOptimizer.  Sets up ROL solver based on
// information from the problem database.
ROLOptimizer::ROLOptimizer( ProblemDescDB& problem_db, 
                            Model&         model) :
  Optimizer( problem_db, 
             model, 
             ROLTraits::create() ),
  optSolverParams("Dakota::ROL"), 
  problemType(TYPE_B) {
    // Populate ROL data with user-provided problem dimensions and
    // initial values, and set ROL solver parameters.  These calls are
    // order-dependent in that the solver settings depend on
    // problemType, which is set with the problem data.
  
    set_problem();
    set_rol_parameters();
} //  


// Alternate constructor for Iterator instantiations by name.  Sets up
// ROL solver based on information passed as arguments.
ROLOptimizer::ROLOptimizer( const String& method_string, 
                                  Model&  model):
  Optimizer( Dakota::method_string_to_enum(method_string), 
             model, 
             ROLTraits::create() ),
  optSolverParams("Dakota::ROL"), 
  problemType(TYPE_B) {
  // Populate ROL data with user-provided problem dimensions and
  // initial values, and set ROL solver parameters.  These calls are
  // order-dependent in that the solver settings depend on
  // problemType, which is set with the problem data.

  set_problem();
  set_rol_parameters();

} // alternate constructor


// core_run redefines the Optimizer virtual function to perform the
// optimization using ROL and catalogue the results.
void ROLOptimizer::core_run() {
  // ostream that will prefix lines with ROL identifier to distinguish
  // ROL output from Dakota output.
  boost::iostreams::filtering_ostream rol_cout;
  rol_cout.push(PrefixingLineFilter("ROL: "));
  rol_cout.push(Cout);

  // Should we attempt to remove this call from the constructor to avoid redundancy? - RWH
//  set_problem();

  // Instantiate and call simplified interface solver object
//  ROL::Solver<Real> opt_solver( optProblem, optSolverParams );
//  opt_solver.solve(rol_cout);
//  rol_cout.flush();
//  opt_solver.reset(true);
//
//  // Copy ROL solution to Dakota bestVariablesArray
//  Variables& best_vars = bestVariablesArray.front();
//  RealVector& cont_vars = best_vars.continuous_variables_view();
//  copy_data(*rolX, cont_vars);
//
//  // ROL does not currently provide access to the final solution, so
//  // attempt a model database lookup directly into best.
//  if(!localObjectiveRecast) {
//    auto best_resp = bestResponseArray.front();
//    auto search_set = ActiveSet(best_resp.active_set());
//    search_set.request_values(AS_FUNC);
//    best_resp.active_set(search_set);
//    bool db_found = iteratedModel.db_lookup(best_vars, search_set, best_resp);
//
//    // Fall back on re-evaluation if not found.
//    if( db_found )
//      Cout << "INFO: ROL retrieved best response from cache." << std::endl;
//    else {
//      Cout << "INFO: ROL re-evaluating model to retrieve best response."
//        << std::endl;
//
//      // Evaluate model for responses at best parameters and set Dakota
//      // bestResponseArray.
//      iteratedModel.continuous_variables(cont_vars);
//      iteratedModel.evaluate();
//      auto best_fns = iteratedModel.current_response().function_values();
//      best_resp.function_values(best_fns);
//    }
//  }
} // core_run


// Helper function to populate ROL data with user-provided problem
// dimensions and initial values.
void ROLOptimizer::set_problem() {

  // Needed in redefining upper and lower bounds for variables and
  // inequality constraints as required by ROL for good performance.
  Real rol_inf  = ROL::ROL_INF<Real>();
  Real rol_ninf = ROL::ROL_NINF<Real>();

  size_t num_econ = numLinearEqConstraints + numNonlinearEqConstraints;
  size_t num_icon = numLinearIneqConstraints + 
                          numNonlinearIneqConstraints;

  // Set the ROL problem type.  Default is TYPE_B (bound constrained),
  // so overwrite for other constraint scenarios.
  if( (num_icon > 0) ||
      ((num_econ > 0) && (boundConstraintFlag)) )
    problemType = TYPE_EB;
  else {
    if (!boundConstraintFlag)
      problemType = (num_econ > 0) ? TYPE_E : TYPE_U;
  }

  // Instantiate null defaults for ROL problem data.  They will be
  // dimensioned and populated as appropriate later.
  auto obj = ROL::makePtr<DakotaROLObjective>(model);
  auto x   = ROL::makePtr<vector_type>(numContinuousVars,0.0);


//  ROL::Ptr<DakotaROLObjective>         obj      = ROL::nullPtr;
//  ROL::Ptr<ROL::Vector<Real>>          x        = ROL::nullPtr;
//  ROL::Ptr<ROL::BoundConstraint<Real>> bnd      = ROL::nullPtr;
//  ROL::Ptr<DakotaROLEqConstraints>     econ     = ROL::nullPtr;
//  ROL::Ptr<ROL::Vector<Real>>          emul     = ROL::nullPtr;
//  ROL::Ptr<DakotaROLIneqConstraints>   icon     = ROL::nullPtr;
//  ROL::Ptr<ROL::Vector<Real>>          imul     = ROL::nullPtr;
//  ROL::Ptr<ROL::BoundConstraint<Real>> ineq_bnd = ROL::nullPtr;

  // Dimension ROL variable vector and set initial values.
//  rolX.reset( new container_type(numContinuousVars, 0.0) );
  Optimizer::get_initial_values(iteratedModel, *rolX);
//  x.reset( new vector_type(rolX) );

  // For TYPE_B and TYPE_EB problems, dimension bounds variable
  // vectors and set initial values.  Map any Dakota infinite bounds
  // to ROL_INF for best performance.
  if ( (problemType == TYPE_B) || (problemType == TYPE_EB) ) {

    Teuchos::RCP<container_type >
      l_rcp(new container_type(numContinuousVars, 0.0));
    Teuchos::RCP<container_type >
      u_rcp(new container_type(numContinuousVars, 0.0));
    get_bounds(iteratedModel, *l_rcp, *u_rcp);

    // Set bounds greater (less) than ROL_INF (ROL_NINF) to ROL_INF
    // (ROL_NINF)
    for (size_t i = 0; i < numContinuousVars; i++) {
      if ((*l_rcp)[i] < rol_ninf)
        (*l_rcp)[i] = rol_ninf;
      if ((*u_rcp)[i] > rol_inf)
        (*u_rcp)[i] = rol_inf;
    }

    // Set bounds values in ROL::BoundConstraint object.
    lowerBounds = Teuchos::rcp( new vector_type( l_rcp ) );
    upperBounds = Teuchos::rcp( new vector_type( u_rcp ) );
    bnd.reset( new ROL::Bounds<Real>(lowerBounds, upperBounds) );
  }

  // Extract gradient type and method for finite-differencing
  const String& grad_type     = iteratedModel.gradient_type();
  const String& method_src    = iteratedModel.method_source();
  const String& interval_type = iteratedModel.interval_type();

  if (grad_type == "none") {
    Cerr << "\nError: gradient type = none is invalid with ROL.\n"
         << "Please select numerical, analytic, or mixed gradients.\n";
    abort_handler(-1);
  }
  if (grad_type == "numerical" && method_src == "vendor") {
    // ROL uses one-sided differences for gradient computations
    if (interval_type == "central") {
    Cerr << "\nFinite Difference Type = 'central' is invalid with ROL.\n"
      << "ROL only provides internal support for forward differences.\n";
    abort_handler(-1);
    }
    // Can not control ROL's parameters for finite-differencing
    Cerr << "\nWarning: ROL's finite difference step size can not be "
      << "controlled via Dakota.\nThe user-provided (or otherwise Dakota "
      << "default) step size will be ignored.\n";
  }

  // Create objective function object and give it access to Dakota
  // model.  If there is a Dakota/user-provided Hessian, need to
  // instantiate the "Hess" version to enable support for it. If 
  // vendor numerical gradients are preferred, instantiate the
  // parent version
  if ( grad_type == "analytic" || grad_type == "mixed" || 
       ( grad_type == "numerical" && method_src == "dakota" ) ){
      if (iteratedModel.hessian_type() == "none")
        obj.reset(new DakotaROLObjectiveGrad(iteratedModel));
      else
        obj.reset(new DakotaROLObjectiveHess(iteratedModel));
  }
  else {
    // Vendor numerical gradients
    obj.reset(new DakotaROLObjective(iteratedModel));
  }

  // If there are equality constraints, create the object and provide
  // the Dakota model. If there is a Dakota/user-provided Hessian, need
  // to instantiate the "Hess" version to enable support for it. If 
  // vendor numerical gradients are preferred, instantiate the
  // parent version
  if (num_econ > 0){
    // create appropriate equality constraint object and give it
    // access to Dakota model
    if ( grad_type == "analytic" || grad_type == "mixed" || 
         ( grad_type == "numerical" && method_src == "dakota" ) ){
        if (iteratedModel.hessian_type() == "none")
          econ.reset(new DakotaROLEqConstraintsGrad(iteratedModel));
        else
          econ.reset(new DakotaROLEqConstraintsHess(iteratedModel));
    }
    else {
      // Vendor numerical gradients
      econ.reset(new DakotaROLEqConstraints(iteratedModel));
    }

    // Initialize Lagrange multipliers for equality constraints.
    Teuchos::RCP<container_type > emul_rcp = 
      Teuchos::rcp( new container_type(num_econ, 0.0) );
    emul.reset(new vector_type(emul_rcp) );
  }

  // If there are inequality constraints, create the object and provide
  // the Dakota model. (Order: [linear_ineq, nonlinear_ineq])
  // If there is a Dakota/user-provided Hessian, need to instantiate
  // the "Hess" version to enable support for it. If vendor numerical
  // gradients are preferred, instantiate the parent version
  if (num_icon > 0){
    // create appropriate inequality constraint object and give it
    // access to Dakota model
    if ( grad_type == "analytic" || grad_type == "mixed" || 
         ( grad_type == "numerical" && method_src == "dakota" ) ){
        if (iteratedModel.hessian_type() == "none")
          icon.reset(new DakotaROLIneqConstraintsGrad(iteratedModel));
        else
          icon.reset(new DakotaROLIneqConstraintsHess(iteratedModel));
    }
    else {
      // Vendor numerical gradients
      icon.reset(new DakotaROLIneqConstraints(iteratedModel));
    }

    // Initial Lagrange multipliers for inequality constraints.
    Teuchos::RCP<container_type > imul_rcp = 
      Teuchos::rcp( new container_type(num_icon, 0.0) );
    imul.reset(new vector_type(imul_rcp) );
  
    // Create ROL inequality constraint bound vectors.
    Teuchos::RCP<container_type >
      ineq_l_rcp(new container_type(num_icon, 0.0));
    Teuchos::RCP<container_type >
      ineq_u_rcp(new container_type(num_icon, 0.0));

    // Get the inequality bounds from Dakota and transfer them into
    // the ROL vectors. 
    copy_data_partial(iteratedModel.linear_iconraint_lower_bounds(),
		      *ineq_l_rcp, 0);
    copy_data_partial(iteratedModel.linear_iconraint_upper_bounds(),
		      *ineq_u_rcp, 0);
    copy_data_partial(iteratedModel.nonlinear_iconraint_lower_bounds(),
		      *ineq_l_rcp, numLinearIneqConstraints);
    copy_data_partial(iteratedModel.nonlinear_iconraint_upper_bounds(),
		      *ineq_u_rcp, numLinearIneqConstraints);

    // Set bounds greater (less) than ROL_INF (ROL_NINF) to ROL_INF
    // (ROL_NINF)
    for (size_t i = 0; i < num_icon; i++) {
      if ((*ineq_l_rcp)[i] < rol_ninf)
        (*ineq_l_rcp)[i] = rol_ninf;
      if ((*ineq_u_rcp)[i] > rol_inf)
        (*ineq_u_rcp)[i] = rol_inf;
    }

    // Set the ROL::BoundConstraint object.
    Teuchos::RCP<ROL::Vector<Real> >
      ineq_lower_bounds( new vector_type( ineq_l_rcp ) );
    Teuchos::RCP<ROL::Vector<Real> >
      ineq_upper_bounds( new vector_type( ineq_u_rcp ) );
    ineq_bnd.reset( new ROL::Bounds<Real>(ineq_lower_bounds,ineq_upper_bounds) );
  }

  // Instantiate ROL problem and populate it with relevant data.
  optProblem = ROL::OptimizationProblem<Real> (obj, x, bnd, econ, emul,
					       icon, imul, ineq_bnd);

} // set_problem

void ROLOptimizer::initialize_run()
{
  Optimizer::initialize_run();

  // Needed to make ROL output to tabular data consistent with other Opt TPLs
  // NOTE: This cannot be set in the consructor (or helper functions associated
  //       with the contructor) because some variables get assigned by base 
  //       class constructors and are not available until all have completed.
  orig_auto_graphics_flag = iteratedModel.auto_graphics();
}


// Helper function to set ROL solver parameters.  This function uses
// ProblemDescDB and therefore should be called at construct time.
void ROLOptimizer::set_rol_parameters()
{
  // PRECEDENCE 1: hard-wired default settings per ROL developers'
  // suggestions

  // If the user has specified "no_hessians", tell ROL to use its own
  // Hessian approximation.
  if (iteratedModel.hessian_type() == "none") {
    optSolverParams.sublist("General").sublist("Secant").
      set("Type", "Limited-Memory BFGS");
    optSolverParams.sublist("General").sublist("Secant").
      set("Use as Hessian", true);
  }

  // Set the solver based on the type of problem.
  if (problemType == TYPE_U){
    optSolverParams.sublist("Step").set("Type","Trust Region");
    optSolverParams.sublist("Step").sublist("Trust Region").
      set("Subproblem Solver", "Truncated CG");
  }
  else if (problemType == TYPE_B){
    optSolverParams.sublist("Step").set("Type","Trust Region");
    optSolverParams.sublist("Step").sublist("Trust Region").
      set("Subproblem Solver", "Truncated CG");
  }
  else if (problemType == TYPE_E){
    optSolverParams.sublist("Step").set("Type","Composite Step");
  }
  else if (problemType == TYPE_EB){
    optSolverParams.sublist("Step").set("Type","Augmented Lagrangian");
    optSolverParams.sublist("Step").sublist("Trust Region").
      set("Subproblem Solver", "Truncated CG");
    // The default choice of Kelley-Sachs was performing lots of fn
    // evals for smoothing, so ROL developers recommend Coleman-Li.
    optSolverParams.sublist("Step").sublist("Trust Region").
      set("Subproblem Model", "Coleman-Li");

    // Turns off adaptively choosing initial penalty parameters
    // New ROL capabaility that results in slower convergence overall
    optSolverParams.sublist("Step").sublist("Augmented Lagrangian").
        set("Use Default Initial Penalty Parameter",false);

    // Turns off automatic constraint and objective scaling
    // New ROL capabaility that results in slower convergence overall
    optSolverParams.sublist("Step").sublist("Augmented Lagrangian").
        set("Use Default Problem Scaling",false);

    // QUESTION: Is there a reason this is only for problem TYPE_EB?
    // Set the verbosity level.
    if (outputLevel >= VERBOSE_OUTPUT)
      optSolverParams.sublist("Step").sublist("Augmented Lagrangian").
        set("Print Intermediate Optimization History",true);
  }

  // PRECEDENCE 2: Dakota input file settings

  // Set the verbosity level.
  optSolverParams.sublist("General").
    set("Print Verbosity", outputLevel < VERBOSE_OUTPUT ? 0 : 1);

  // Set the stopping criteria.
  optSolverParams.sublist("Status Test").
    set("Gradient Tolerance", probDescDB.get_real("method.gradient_tolerance"));
  optSolverParams.sublist("Status Test").
    set("Constraint Tolerance",
	probDescDB.get_real("method.constraint_tolerance")
	);
  optSolverParams.sublist("Status Test").
    set("Step Tolerance", probDescDB.get_real("method.variable_tolerance"));
  // ROL enforces an int; cast is Ok since SZ_MAX default removed at Minimizer
  optSolverParams.sublist("Status Test").
    set("Iteration Limit", (int)maxIterations);

  // PRECEDENCE 3: power-user advanced options

  // Check for ROL XML input file.
  String adv_opts_file = probDescDB.get_string("method.advanced_options_file");
  if (!adv_opts_file.empty()) {
    if (boost::filesystem::exists(adv_opts_file)) {
      if (outputLevel >= NORMAL_OUTPUT)
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
      Teuchos::Ptr<Teuchos::ParameterList> osp_ptr(&optSolverParams);
      Teuchos::updateParametersFromXmlFile(adv_opts_file, osp_ptr);
      if (outputLevel >= VERBOSE_OUTPUT) {
      	Cout << "ROL OptimizationSolver parameters:\n";
      	   optSolverParams.print(Cout, 2, true, true);
      }
    }
    TEUCHOS_STANDARD_CATCH_STATEMENTS(outputLevel >= VERBOSE_OUTPUT, Cerr,
				      success);
  }

} // set_rol_parameters


// --------------------------------------------------------------
//    These classes could go into a new file for evaluators along
//    the lines of APPS and COLIN if desirable.
// --------------------------------------------------------------


namespace {
  // Helper function to manage response evaluations, both objectives
  // and nonlinear constraints.  The response and all derivatives are
  // evaluated on first call and need only be retrieved from the
  // database upon subsequent calls.  This allows for time-saving
  // speculative derivative computation, which is not natively
  // supported by ROL.

  void update_model(Model & model, const container_type & x)
  {
    static container_type prev_x = x;
    static bool first_step = true;

    // throttle consequetive duplicate output
    bool is_seq_dup = !first_step && (prev_x == x);
    if( is_seq_dup )
      model.auto_graphics(false);
    else
      prev_x = x;

    // Set the model variables to the current values.
    size_t num_cv = model.cv();
    for(size_t i=0; i<num_cv; ++i)
      model.continuous_variable(x[i], i);

    // Evaluate response, gradient, and Hessian, depending on 
    // what is provided by Dakota/user.
    ActiveSet eval_set(model.current_response().active_set());
    if (model.gradient_type() == "numerical" && model.method_source() == "vendor")
      eval_set.request_values(AS_FUNC);
    else if (model.hessian_type() == "none")
      eval_set.request_values(AS_FUNC+AS_GRAD);
    else
      eval_set.request_values(AS_FUNC+AS_GRAD+AS_HESS);
    model.evaluate(eval_set);

    // Restore tabular data output state
    if( is_seq_dup )
      model.auto_graphics(orig_auto_graphics_flag);

    first_step = false;

  } // update_model

} // namespace anonymous


// -----------------------------------------------------------------
/** Implementation of the DakotaROLObjective class. */


// Constructor.
DakotaROLObjective::DakotaROLObjective(Model & model) :
  dakotaModel(model)
{ }

// Compute objective value and return to ROL.
Real
DakotaROLObjective::value(const container_type &x, Real &tol)
{
  update_model(dakotaModel, x);
  return dakotaModel.current_response().function_value(0);

} // objective value


// -----------------------------------------------------------------
/** Implementation of the DakotaROLObjectiveGrad class. */


// Constructor.
DakotaROLObjectiveGrad::DakotaROLObjectiveGrad(Model & model) :
  DakotaROLObjective(model)
{ }


// Compute objective gradient and return to ROL.
void
DakotaROLObjectiveGrad::gradient( container_type &g, const container_type &x, Real &tol )
{
  update_model(dakotaModel, x);
  copy_column_vector(dakotaModel.current_response().function_gradients(), 0, g);

} // objective gradient


// -----------------------------------------------------------------
/** Implementation of the DakotaROLObjectiveHess class. */


// Constructor.
DakotaROLObjectiveHess::DakotaROLObjectiveHess(Model & model) :
  DakotaROLObjectiveGrad(model)
{ }


// Compute objective Hessian-vector product and return to ROL.
void
DakotaROLObjectiveHess::hessVec(container_type &hv,
        const container_type &v,
        const container_type &x,
        Real &tol)
{
  // Make sure the Hessian has been evaluated and get it from Dakota
  // Response.
  update_model(dakotaModel, x);
  const RealSymMatrix& hess_f = dakotaModel.current_response().function_hessian(0);

  // Multiply the objective Hessian matrix by the vector.
  apply_matrix_partial(hess_f, v, hv);

} // objective hessVec


// Compute product of objective Hessian and a vector.  Not implemented
// since no ROL methods currently exposed use it.
void
DakotaROLObjectiveHess::invHessVec(container_type &h,
           const container_type &v,
           const container_type &x,
           Real &tol)
{
  Cerr << "\nError: DakotaROLObjectiveHess::invHessVec is not currently supported."
       << "  logic error.\n";
  abort_handler(METHOD_ERROR);

} // objective invHessVec


// -----------------------------------------------------------------
/** Implementation of the DakotaROLIneqConstraints class. */


// Constructor.
DakotaROLIneqConstraints::DakotaROLIneqConstraints(Model & model) :
  dakotaModel(model)
{
  haveNlnConst = model.num_nonlinear_iconraints() > 0;

} // ineqConstraints constructor


// Compute inequality constraint values and return to ROL.
void
DakotaROLIneqConstraints::value(container_type &c, const container_type &x, Real &tol)
{
  // Evaluate nonlinear constraints.
  update_model(dakotaModel, x);

  // Matrix-vector multiply to get linear constraint values.
  apply_linear_constraints( dakotaModel, CONSTRAINT_EQUALITY_TYPE::INEQUALITY, x, c );

  get_nonlinear_iconraints( dakotaModel, c );

} // ineqConstraints value


// -----------------------------------------------------------------
/** Implementation of the DakotaROLIneqConstraintsGrad class. */


// Constructor.
DakotaROLIneqConstraintsGrad::DakotaROLIneqConstraintsGrad(Model & model) :
  DakotaROLIneqConstraints(model)
{ }


// Multiply a vector by the inequality constraint Jacobian and return
// to ROL.
void 
DakotaROLIneqConstraintsGrad::applyJacobian(container_type &jv,
    const container_type &v, const container_type &x, Real &tol)
{
  // Matrix-vector multiply to apply linear constraint Jacobian.
  const RealMatrix & lin_ineq_coeffs = dakotaModel.linear_iconraint_coeffs();
  apply_matrix_partial(lin_ineq_coeffs, v, jv);

  // Apply nonlinear constraint Jacobian.
  if( haveNlnConst ) {
    update_model(dakotaModel, x);
    apply_nonlinear_constraints(dakotaModel, CONSTRAINT_EQUALITY_TYPE::INEQUALITY, v, jv);
  }

} // ineqConstraints applyJacobian


// Multiply a vector by the transpose of the inequality constraint
// Jacobian and return to ROL.
void
DakotaROLIneqConstraintsGrad::applyAdjointJacobian(container_type &ajv,
    const container_type &v, const container_type &x, Real &tol)
{
  // Must init since are merging the effect of linear and nonlinear
  // might get skipped.
  ajv.assign(ajv.size(), 0.0);

  // Matrix-vector multiply to apply transpose of linear constraint
  // Jacobian.
  const RealMatrix & lin_ineq_coeffs = dakotaModel.linear_iconraint_coeffs();
  apply_matrix_transpose_partial(lin_ineq_coeffs, v, ajv);

  // Apply transpose of nonlinear constraint Jacobian.
  if (haveNlnConst) {
    update_model(dakotaModel, x);
    apply_nonlinear_constraints(dakotaModel, CONSTRAINT_EQUALITY_TYPE::INEQUALITY, v, ajv, true);
  }

} // ineqConstraints applyAdjointJacobian


// -----------------------------------------------------------------
/** Implementation of the DakotaROLIneqConstraintsGrad class. */


// Constructor.
DakotaROLIneqConstraintsHess::DakotaROLIneqConstraintsHess(Model & model) :
  DakotaROLIneqConstraintsGrad(model)
{ }


// Multiply a vector by the transpose of the inequality constraint
// Hessian and return to ROL.
void
DakotaROLIneqConstraintsHess::applyAdjointHessian( container_type & ahuv,
                                                   const container_type & u, 
                                                   const container_type & v,
                                                   const container_type & x,
                                                   Real &tol )
{
  // Must init since linear info is 0.0 and we are appending 
  // the effect of nonlinear
  ahuv.assign(ahuv.size(), 0.0);

  // apply nonlinear constraint Hessian (might be empty)
  if (haveNlnConst) {
    // make sure that model is current
    update_model(dakotaModel, x);

    RealSymMatrix hu(dakotaModel.current_response().function_hessian(1));
    hu *= u[0];

    for( size_t i=1; i<dakotaModel.num_nonlinear_iconraints(); ++i )
    {
      RealSymMatrix temp_hu(dakotaModel.current_response().function_hessian(1+i));
      temp_hu *= u[i];
      hu += temp_hu;
    }

    apply_matrix_partial(hu, v, ahuv);
  }

} // ineqConstraintsHess applyAdjointHessian


// -----------------------------------------------------------------
/** Implementation of the DakotaROLEqConstraints class. */


// Constructor.
DakotaROLEqConstraints::DakotaROLEqConstraints(Model & model) :
  dakotaModel(model)
{
  haveNlnConst = model.num_nonlinear_econraints() > 0;

} // eqContraints constructor


// Compute equality constraint values and return to ROL.
void
DakotaROLEqConstraints::value(container_type &c, const container_type &x, Real &tol)
{
  // Evaluate nonlinear constraints.
  update_model(dakotaModel, x);

  // Matrix-vector multiply to get linear constraint values.
  apply_linear_constraints( dakotaModel, CONSTRAINT_EQUALITY_TYPE::EQUALITY, x, c );

  // QUESTION: What does this do?  Also, why the -1.0?
  get_nonlinear_econraints( dakotaModel, c, -1.0 );

} // eqConstraints value



// -----------------------------------------------------------------
/** Implementation of the DakotaROLEqConstraintsGrad class. */


// Constructor.
DakotaROLEqConstraintsGrad::DakotaROLEqConstraintsGrad(Model & model) :
  DakotaROLEqConstraints(model)
{ }


// Multiply a vector by the equality constraint Jacobian and return
// to ROL.
void
DakotaROLEqConstraintsGrad::applyJacobian(container_type &jv,
    const container_type &v, const container_type &x, Real &tol)
{
  // Matrix-vector multiply to apply linear constraint Jacobian.
  const RealMatrix & lin_eq_coeffs = dakotaModel.linear_econraint_coeffs();
  apply_matrix_partial(lin_eq_coeffs, v, jv);

  // Apply nonlinear constraint Jacobian.
  if( haveNlnConst ) {
    update_model(dakotaModel, x);
    apply_nonlinear_constraints(dakotaModel, CONSTRAINT_EQUALITY_TYPE::EQUALITY, v, jv);
  }

} // eqConstraints applyJacobian


// Multiply a vector by the transpose of the equality constraint
// Jacobian and return to ROL.
void
DakotaROLEqConstraintsGrad::applyAdjointJacobian(container_type &ajv,
    const container_type &v, const container_type &x, Real &tol)
{
  // Must init since are merging the effect of linear and nonlinear
  // and linear might get skipped
  ajv.assign(ajv.size(), 0.0);

  // Matrix-vector multiply to apply transpose of linear constraint
  // Jacobian.
  const RealMatrix & lin_eq_coeffs = dakotaModel.linear_econraint_coeffs();
  apply_matrix_transpose_partial(lin_eq_coeffs, v, ajv);

  // Apply transpose of nonlinear constraint Jacobian.
  if (haveNlnConst) {
    update_model(dakotaModel, x);
    apply_nonlinear_constraints(dakotaModel, CONSTRAINT_EQUALITY_TYPE::EQUALITY, v, ajv, true);
  }

} // eqConstraints applyAdjointJacobian



// -----------------------------------------------------------------
/** Implementation of the DakotaROLEqConstraintsHess class. */


// Constructor.
DakotaROLEqConstraintsHess::DakotaROLEqConstraintsHess(Model & model) :
  DakotaROLEqConstraintsGrad(model)
{ }


// Multiply a vector by the transpose of the inequality constraint
// Hessian and return to ROL.
void
DakotaROLEqConstraintsHess::applyAdjointHessian( container_type & ahuv,
                                                   const container_type & u, 
                                                   const container_type & v,
                                                   const container_type & x,
                                                   Real &tol )
{
  // Must init since linear info is 0.0 and we are appending 
  // the effect of nonlinear
  ahuv.assign(ahuv.size(), 0.0);

  // apply nonlinear constraint Hessian (might be empty)
  if (haveNlnConst) {

    size_t num_nln_iconraints = dakotaModel.num_nonlinear_iconraints();
    // make sure that model is current
    update_model(dakotaModel, x);

    RealSymMatrix hu(dakotaModel.current_response().function_hessian(1+num_nln_iconraints));
    hu *= u[0];

    for( size_t i=1; i<dakotaModel.num_nonlinear_econraints(); ++i )
    {
      RealSymMatrix temp_hu(dakotaModel.current_response().function_hessian(1+num_nln_iconraints+i));
      temp_hu *= u[i];
      hu += temp_hu;
    }

    apply_matrix_partial(hu, v, ahuv);
  }

} // eqConstraintsHess applyAdjointHessian

} // namespace rol_interface


} // namespace Dakota
