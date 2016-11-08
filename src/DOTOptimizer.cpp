/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       DOTOptimizer
//- Description: Implementation code for the DOTOptimizer class
//- Owner:       Mike Eldred
//- Checked by:

#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "DOTOptimizer.hpp"
#include "ProblemDescDB.hpp"

static const char rcsId[]="@(#) $Id: DOTOptimizer.cpp 7029 2010-10-22 00:17:02Z mseldre $";

#define DOT_F77    F77_FUNC(dot,DOT)
#define DOT510_F77 F77_FUNC(dot510,DOT510)

extern "C" {

void DOT_F77( int& info, int& ngotoz, int& method, int& iprint, int& ndv,
	      int& ncon, double* x, double* xl, double* xu, double& obj,
	      int& minmax, double* g, double* rprm, int* iprm, double* wk,
	      int& nrwk, int* iwk, int& nriwk );

void DOT510_F77( int& ndv, int& ncon, int& ncola, int& method, int& nrwk,
		 int& nriwk, int& nrb, int& ngmax, double* xl, double* xu );

}

namespace Dakota {


DOTOptimizer::DOTOptimizer(ProblemDescDB& problem_db, Model& model):
  Optimizer(problem_db, model), realCntlParmArray(20, 0.0),
  intCntlParmArray(20, 0)
{
  // If speculativeFlag is set and vendor numerical_gradients are used, output
  // a warning
  if (speculativeFlag && vendorNumericalGradFlag)
    Cerr << "\nWarning: speculative method specification is ignored for"
	 << "\n         vendor numerical gradients.\n\n";

  if (outputLevel > NORMAL_OUTPUT) {
    printControl = 7;
    // BMA 20161107: dotMethod not initialized based on constraints
    // until initialize_run()
    //Cout << "DOT Method = " << dotMethod << std::endl;
    Cout << "DOT print control = " << printControl << std::endl;
  }
  else
    printControl = 3;

  initialize(); // convenience fn for shared ctor code
}


DOTOptimizer::DOTOptimizer(const String& method_string, Model& model):
  Optimizer(method_string_to_enum(method_string), model),
  realCntlParmArray(20, 0.0), intCntlParmArray(20, 0)
{
  printControl = 3;
  initialize(); // convenience fn for shared ctor code
}


#ifdef HAVE_DYNLIB_FACTORIES
DOTOptimizer* new_DOTOptimizer(ProblemDescDB& problem_db)
{
#ifdef DAKOTA_DYNLIB
  not_available("DOT");
  return 0;
#else
  return new DOTOptimizer(problem_db);
#endif // DAKOTA_DYNLIB
}

DOTOptimizer* new_DOTOptimizer(Model& model)
{
#ifdef DAKOTA_DYNLIB
  not_available("DOT");
  return 0;
#else
  return new DOTOptimizer(model);
#endif // DAKOTA_DYNLIB
}
#endif // HAVE_DYNLIB_FACTORIES


void DOTOptimizer::initialize()
{
  // Prevent nesting of an instance of a Fortran iterator within another
  // instance of the same iterator (which would result in data clashes since
  // Fortran does not support object independence).  Recurse through all
  // sub-models and test each sub-iterator for DOT presence.
  // Note: This check is performed for DOT, CONMIN, and SOLBase, but not
  //       for LHS since it is only active in pre-processing.
  Iterator sub_iterator = iteratedModel.subordinate_iterator();
  if (!sub_iterator.is_null() && 
      ( ( sub_iterator.method_name() >= DOT_BFGS &&
	  sub_iterator.method_name() <= DOT_SQP ) ||
	( sub_iterator.uses_method() >= DOT_BFGS &&
	  sub_iterator.uses_method() <= DOT_SQP ) ) )
    sub_iterator.method_recourse();
  ModelList& sub_models = iteratedModel.subordinate_models();
  for (ModelLIter ml_iter = sub_models.begin();
       ml_iter != sub_models.end(); ml_iter++) {
    sub_iterator = ml_iter->subordinate_iterator();
    if (!sub_iterator.is_null() && 
	( ( sub_iterator.method_name() >= DOT_BFGS &&
	    sub_iterator.method_name() <= DOT_SQP ) ||
	  ( sub_iterator.uses_method() >= DOT_BFGS &&
	    sub_iterator.uses_method() <= DOT_SQP ) ) )
      sub_iterator.method_recourse();
  }

  // Initialize DOT specific data

  intCntlParmArray[2] = maxIterations; // IPRM(3)=ITMAX, p.3-8 DOT V4.20

  // assign a nondefault constraint tolerance if a valid value has been 
  // set in dakota.in; otherwise utilize the DOT default.
  if (constraintTol > 0.0) {
    realCntlParmArray[1] = constraintTol;
    if (outputLevel > QUIET_OUTPUT)
      Cout << "constraint violation tolerance = " << constraintTol << '\n';
  }

  // convergenceTol is an optional parameter in dakota.input.nspec, but
  // defining our own default (in the DataMethod constructor) and
  // always assigning it applies some consistency across methods.
  // Therefore, the DOT default is not used.
  realCntlParmArray[3] = convergenceTol;

  // Default DOT gradients [ IPRM(1)=0 ] = numerical;forward;vendor setting.
  const String& grad_type     = iteratedModel.gradient_type();
  const String& method_src    = iteratedModel.method_source();
  const String& interval_type = iteratedModel.interval_type();
  if ( grad_type == "analytic" || grad_type == "mixed" || 
       ( grad_type == "numerical" && method_src == "dakota" ) )
    // p. 3-12 DOT V4.20: Set IPRM(1)=1 before calling DOT. This invokes
    // user-supplied gradient mode which DAKOTA uses for analytic, dakota 
    // numerical, or mixed analytic/dakota numerical gradients.
    intCntlParmArray[0] = 1;
  else if (grad_type == "none") {
    Cerr << "\nError: gradient type = none is invalid with DOT.\n"
         << "Please select numerical, analytic, or mixed gradients."<<std::endl;
    abort_handler(-1);
  }
  else { // Vendor numerical gradients
    Real fd_grad_ss = iteratedModel.fd_gradient_step_size()[0];
    if (interval_type == "central") {
      // p. 3-8 DOT V4.20: Set IPRM(1) = -1 before calling DOT
      intCntlParmArray[0] = -1; // central finite difference by DOT

      // DOT's central differencing uses +/- 2*fdss*X_i for some reason (why?),
      // so correct for this by sending fdss/2 to DOT
      realCntlParmArray[8] = fd_grad_ss/2; // FDCH
    }
    else 
      // DOT's forward differencing uses fdss*X_i as one would expect
      realCntlParmArray[8] = fd_grad_ss;   // FDCH

    // for FDCHM (minimum delta), use 2 orders of magnitude smaller than fdss to
    // be consistent with Model::estimate_derivatives():
    realCntlParmArray[9] = fd_grad_ss*.01; // FDCHM
  }
}


DOTOptimizer::~DOTOptimizer()
{
  // Virtual destructor handles referenceCount at Iterator level.
}


void DOTOptimizer::allocate_constraints()
{
  // DOT handles all constraints as 1-sided inequalities.  Compute the number
  // of 1-sided inequalities to pass to DOT (numDotConstr) as well as the
  // mappings (indices, multipliers, offsets) between the DAKOTA constraints
  // and the DOT constraints.  TO DO: support for automatic constraint scaling.
  size_t i, num_nln_ineq = iteratedModel.num_nonlinear_ineq_constraints(),
    num_nln_eq   = iteratedModel.num_nonlinear_eq_constraints(),
    num_lin_ineq = iteratedModel.num_linear_ineq_constraints(),
    num_lin_eq   = iteratedModel.num_linear_eq_constraints();
  const RealVector& nln_ineq_lwr_bnds
    = iteratedModel.nonlinear_ineq_constraint_lower_bounds();
  const RealVector& nln_ineq_upr_bnds
    = iteratedModel.nonlinear_ineq_constraint_upper_bounds();
  const RealVector& nln_eq_targets
    = iteratedModel.nonlinear_eq_constraint_targets();
  const RealVector& lin_ineq_lwr_bnds
    = iteratedModel.linear_ineq_constraint_lower_bounds();
  const RealVector& lin_ineq_upr_bnds
    = iteratedModel.linear_ineq_constraint_upper_bounds();
  const RealVector& lin_eq_targets
    = iteratedModel.linear_eq_constraint_targets();
  numDotNlnConstr = 2*num_nln_eq;
  for (i=0; i<num_nln_ineq; ++i) {
    if (nln_ineq_lwr_bnds[i] > -bigRealBoundSize) {
      numDotNlnConstr++;
      // nln_ineq_lower_bnd - dakota_constraint <= 0
      constraintMappingIndices.push_back(i);
      constraintMappingMultipliers.push_back(-1.0);
      constraintMappingOffsets.push_back(nln_ineq_lwr_bnds[i]);
    }
    if (nln_ineq_upr_bnds[i] < bigRealBoundSize) {
      numDotNlnConstr++;
      // dakota_constraint - nln_ineq_upper_bnd <= 0
      constraintMappingIndices.push_back(i);
      constraintMappingMultipliers.push_back(1.0);
      constraintMappingOffsets.push_back(-nln_ineq_upr_bnds[i]);
    }
  }
  for (i=0; i<num_nln_eq; ++i) {
    // nln_eq_target - dakota_constraint <= 0
    constraintMappingIndices.push_back(i+num_nln_ineq);
    constraintMappingMultipliers.push_back(-1.0);
    constraintMappingOffsets.push_back(nln_eq_targets[i]);
    // dakota_constraint - nln_eq_target <= 0
    constraintMappingIndices.push_back(i+num_nln_ineq);
    constraintMappingMultipliers.push_back(1.0);
    constraintMappingOffsets.push_back(-nln_eq_targets[i]);
  }

  numDotLinConstr = 2*num_lin_eq;
  for (i=0; i<num_lin_ineq; ++i) {
    if (lin_ineq_lwr_bnds[i] > -bigRealBoundSize) {
      numDotLinConstr++;
      // lin_ineq_lower_bnd - Ax <= 0
      constraintMappingIndices.push_back(i);
      constraintMappingMultipliers.push_back(-1.0);
      constraintMappingOffsets.push_back(lin_ineq_lwr_bnds[i]);
    }
    if (lin_ineq_upr_bnds[i] < bigRealBoundSize) {
      numDotLinConstr++;
      // Ax - lin_ineq_upper_bnd <= 0
      constraintMappingIndices.push_back(i);
      constraintMappingMultipliers.push_back(1.0);
      constraintMappingOffsets.push_back(-lin_ineq_upr_bnds[i]);
    }
  }
  for (i=0; i<num_lin_eq; ++i) {
    // lin_eq_target - Ax <= 0
    constraintMappingIndices.push_back(i+num_lin_ineq);
    constraintMappingMultipliers.push_back(-1.0);
    constraintMappingOffsets.push_back(lin_eq_targets[i]);
    // Ax - lin_eq_target <= 0
    constraintMappingIndices.push_back(i+num_lin_ineq);
    constraintMappingMultipliers.push_back(1.0);
    constraintMappingOffsets.push_back(-lin_eq_targets[i]);
  }

  numDotConstr = numDotNlnConstr + numDotLinConstr;

  // DOT's need for a nonzero size array is handled with nln_con_array_size
  size_t nln_con_array_size = (numDotConstr) ? numDotConstr : 1;
  constraintValues.resize(nln_con_array_size);

  // See DOT manual, Chapter 2, p. 4 for DOT's METHOD values
  if (methodName == DOT_MMFD) {
    if (numDotConstr)
      dotMethod = 1; // Good input.
    else {           // Recover from bad input.
      Cerr << "\nWarning: for no constraints, dot_mmfd request will be"
	   << "\n         changed to dot_frcg.\n\n";
      dotMethod = 2;
      methodName = DOT_FRCG; // for output header/footer
    }
  }
  else if (methodName == DOT_SLP) {
    dotMethod = 2; // Same dotMethod for good or bad input.
    if (numDotConstr == 0) { // Recover from bad input.
      Cerr << "\nWarning: for no constraints, dot_slp request will be"
           << "\n         changed to dot_frcg.\n\n";
      methodName = DOT_FRCG; // for output header/footer
    }
  }
  else if (methodName == DOT_SQP) {
    if (numDotConstr)
      dotMethod = 3; // Good input.
    else {           // Recover from bad input.
      Cerr << "\nWarning: for no constraints, dot_sqp request will be"
	   << "\n         changed to dot_bfgs.\n\n";
      dotMethod = 1;
      methodName = DOT_BFGS; // for output header/footer
    }
  }
  else if (methodName == DOT_BFGS) {
    if (numDotConstr == 0)
      dotMethod = 1; // Good input.
    else {           // Recover from bad input.
      Cerr << "\nWarning: for constrained optimization, dot_bfgs request"
	   << "\n         will be changed to dot_sqp.\n\n";
      dotMethod = 3;
      methodName = DOT_SQP; // for output header/footer
    }
  }
  else if (methodName == DOT_FRCG) {
    if (numDotConstr == 0)
      dotMethod = 2; // Good input.
    else {           // Recover from bad input.
      Cerr << "\nWarning: for constrained optimization, dot_frcg request"
	   << "\n         will be changed to dot_mmfd.\n\n";
      dotMethod = 1;
      methodName = DOT_MMFD; // for output header/footer
    }
  }
}


void DOTOptimizer::allocate_workspace()
{
  // Use the dot510 routine to compute work array sizes.
  int nrb, ngmax, ncola = 0, num_cv = numContinuousVars;
  const RealVector& lower_bnds = iteratedModel.continuous_lower_bounds();
  const RealVector& upper_bnds = iteratedModel.continuous_upper_bounds();
  DOT510_F77(num_cv, numDotConstr, ncola, dotMethod, realWorkSpaceSize,
	     intWorkSpaceSize, nrb, ngmax, lower_bnds.values(),
	     upper_bnds.values());

  //Cout << "dot510 sizing: realWorkSpaceSize = " << realWorkSpaceSize
  //     << " intWorkSpaceSize = " << intWorkSpaceSize << std::endl;
  
  // Size workspace (and initialize to be a good citizen)

  realWorkSpace.assign(realWorkSpaceSize, 0.); // wk[nrwk]
  intWorkSpace.assign(intWorkSpaceSize, 0);    // iwk[nriwk]
}


void DOTOptimizer::initialize_run()
{
  Optimizer::initialize_run();

  allocate_constraints();
  allocate_workspace();

  dotInfo = 0; // Initialize to 0 before calling DOT

  // initialize the optimization starting point
  copy_data(iteratedModel.continuous_variables(), designVars);
}


void DOTOptimizer::core_run()
{
  size_t i, j, fn_eval_cntr;
  int num_cv = numContinuousVars;

  // MINMAX from DOT manual.  Values of 0 or -1 (minimize) or 1 (maximize).
  // Any MOO/NLS recasting is responsible for setting the scalar min/max
  // sense within the recast.
  const BoolDeque& max_sense = iteratedModel.primary_response_fn_sense();
  int min_max = (!max_sense.empty() && max_sense[0]) ? 1 : 0;

  // Initialize local bounds and linear constraints
  const RealVector& lower_bnds = iteratedModel.continuous_lower_bounds();
  const RealVector& upper_bnds = iteratedModel.continuous_upper_bounds();
  size_t num_lin_ineq = iteratedModel.num_linear_ineq_constraints(),
         num_lin_eq   = iteratedModel.num_linear_eq_constraints();
  const RealMatrix& lin_ineq_coeffs
    = iteratedModel.linear_ineq_constraint_coeffs();
  const RealMatrix& lin_eq_coeffs
    = iteratedModel.linear_eq_constraint_coeffs();

  for (fn_eval_cntr=1; fn_eval_cntr<=maxFunctionEvals; fn_eval_cntr++) {

    // dotFDSinfo accepts the NGOTOZ parameter from DOT.  This involves a
    // change to the DOT source (adding NGOTOZ to the SUBROUTINE DOT
    // parameter list in ddot1.f).  dotFDSinfo is nonzero when DOT is
    // calculating finite difference gradients, and is zero otherwise.
    dotFDSinfo = 0; // Vanderplaats: Initialize to 0 before DOT call

    DOT_F77(dotInfo, dotFDSinfo, dotMethod, printControl, num_cv, numDotConstr,
	    designVars.values(), lower_bnds.values(), upper_bnds.values(),
	    objFnValue, min_max, constraintValues.values(),
	    &realCntlParmArray[0], &intCntlParmArray[0], &realWorkSpace[0],
	    realWorkSpaceSize, &intWorkSpace[0], intWorkSpaceSize);

    if (dotInfo == 0) break;

    if (outputLevel > NORMAL_OUTPUT) { // output info on DOT request
      if (vendorNumericalGradFlag) {
        if (dotFDSinfo==0)
          Cout << "\nDOT requests function values:";
        else
          Cout << "\nDOT requests function values for internally computed "
               << "numerical gradients:";
      }
      else {
        if (dotInfo==1)
          Cout << "\nDOT requests function values:";
        else {
          if (iteratedModel.gradient_type() == "numerical")
            Cout << "\nDOT requests dakota-numerical gradients:";
          else
            Cout << "\nDOT requests analytic gradients:";
	}
      }
    }

    // Populate Active Set Vector based on dotInfo, dotFDSinfo, ngt, & IWK
    // See p.3-12 of DOT manual
    int ngt = intCntlParmArray[19]; // # of active constraints from IPRM
    if (dotInfo==1 && dotFDSinfo==0) {
      // Initial analysis (once) or line search analyses: evaluate all fns.
      if (speculativeFlag && !vendorNumericalGradFlag) {
        // Evaluate all function values and gradients (analytic or finite 
	// difference)
        Cout << "\nSpeculative optimization: evaluation augmented "
             << "with speculative gradients.";
        activeSet.request_values(3); // set all values to 3
      }
      else
        activeSet.request_values(1); // set all values to 1
    }
    else {
      // dotInfo=1: Finite differencing, evaluate obj. & active constraint fns.
      // dotInfo=2: Evaluate gradients of objective & active constraints
      if (speculativeFlag && !vendorNumericalGradFlag)
        Cout << "\nSpeculative optimization: retrieving gradients already "
	     << "evaluated from database.";
      activeSet.request_values(0); // initialize all values to 0
      for (i=0; i<numObjectiveFns; i++)
        activeSet.request_value(dotInfo, i); // objective function(s)
      for (i=0; i<ngt; i++) {
        // IWK(1->NGT) = intWorkSpace[0->NGT-1] = active constraint #'s 
	// While DOT returns 1-based constraint id's, work in 0-based id's for
	// indexing convenience.
        size_t dot_constr = intWorkSpace[i] - 1; // (0-based)
        if (dot_constr < numDotNlnConstr) { // only nonlinear in ASV
          size_t dakota_constr = constraintMappingIndices[dot_constr];
          activeSet.request_value(dotInfo, dakota_constr+numObjectiveFns);
	  // some DAKOTA equality and 2-sided inequality constraints may have
	  // their ASV assigned multiple times depending on which of DOT's
	  // 1-sided inequalities are active.
	}
      }
    }

    iteratedModel.continuous_variables(designVars);
    iteratedModel.evaluate(activeSet);
    const Response& local_response = iteratedModel.current_response();

    // Populate proper data for input back to DOT through parameter list
    if (dotInfo==2) {
      // Populate realWorkSpace with a concatenated array of active gradients.
      // Alternatively: realWorkSpace = concatenate_sensitivity_array();
      const RealMatrix& local_fn_grads = local_response.function_gradients();
      const int num_vars = local_fn_grads.numRows();
      size_t offset = num_vars;
      for (j=0; j<num_vars; ++j) // objective fn. gradient always needed
	realWorkSpace[j] = local_fn_grads(j,0);
      // DOT constraint gradient requests must be mapped to DAKOTA constraint
      // gradients and multipliers must be applied.
      for (i=0; i<ngt; i++) {
	// work in 0-based constraint id's for indexing convenience
        size_t dot_constr = intWorkSpace[i] - 1;
        size_t dakota_constr = constraintMappingIndices[dot_constr];
        if (dot_constr < numDotNlnConstr) { // nonlinear ineq & eq
          // gradients pick up multiplier mapping only (offsets drop out)
          for (j=0; j<num_vars; ++j)
            realWorkSpace[offset+j] = constraintMappingMultipliers[dot_constr]
              * local_fn_grads(j,dakota_constr+1);
        }
        else if (dakota_constr < num_lin_ineq) { // linear ineq
	  for (j=0; j<num_vars; ++j)
	    realWorkSpace[offset+j] = constraintMappingMultipliers[dot_constr] *
	      lin_ineq_coeffs(dakota_constr,j);
	}
	else { // linear eq
	  size_t dakota_leq_constr = dakota_constr - num_lin_ineq;
	  for (j=0; j<num_vars; ++j)
	    realWorkSpace[offset+j] = constraintMappingMultipliers[dot_constr] *
	      lin_eq_coeffs(dakota_leq_constr,j);
        }
        offset += num_vars;
      }
    }
    else {
      // Get values (no active distinction needed since inactive = 0. works).
      const RealVector& local_fn_vals = local_response.function_values();
      objFnValue = local_fn_vals[0];
      // DOT constraint requests must be mapped to DAKOTA constraints and
      // offsets/multipliers must be applied.
      size_t dot_constr, dakota_constr;
      for (dot_constr=0; dot_constr<numDotConstr; ++dot_constr) {
        dakota_constr = constraintMappingIndices[dot_constr];
        if (dot_constr < numDotNlnConstr) // nonlinear ineq & eq
          constraintValues[dot_constr] = constraintMappingOffsets[dot_constr] +
            constraintMappingMultipliers[dot_constr] *
            local_fn_vals[dakota_constr+1];
        else {
          Real Ax = 0.0;
          if (dakota_constr < num_lin_ineq) { // linear ineq
            for (j=0; j<numContinuousVars; j++)
              Ax += lin_ineq_coeffs(dakota_constr,j) * designVars[j];
          }
          else { // linear eq
            size_t dakota_leq_constr = dakota_constr - num_lin_ineq;
            for (j=0; j<numContinuousVars; j++)
              Ax += lin_eq_coeffs(dakota_leq_constr,j) * designVars[j];
          }
          constraintValues[dot_constr] = constraintMappingOffsets[dot_constr] +
            constraintMappingMultipliers[dot_constr] * Ax;
        }
      }
    }
  }

  if (fn_eval_cntr==maxFunctionEvals+1)
    Cout << "Iteration terminated: max_function_evaluations limit has been "
         << "met.\n";

  // Set best variables and response for use by strategy level.  If 
  // dotInfo = 0, then optimization was complete and DOT contains the best 
  // variables & responses data in the argument list data structures.  If 
  // dotInfo is not 0, then there was an abort (e.g., maxFunctionEvals) and it
  // is not clear how to return the best solution. For now, return the argument
  // list data (as if dotInfo = 0) which should be the last evaluation (?).

  // Set best data using DAKOTA constraint values (not DOT constraints):
  bestVariablesArray.front().continuous_variables(designVars);
  if (!localObjectiveRecast) { // else local_objective_recast_retrieve()
                               // is used in Optimizer::post_run()
    RealVector best_fns(numFunctions);
    best_fns[0] = objFnValue;
    // NOTE: best_fn_vals[i] may be recomputed multiple times, but this
    // should be OK so long as all of constraintValues is populated
    // (no active set deletions).
    for (size_t i=0; i<numDotNlnConstr; ++i) {
      size_t dakota_constr = constraintMappingIndices[i];
      // back out the offset and multiplier
      best_fns[dakota_constr+1] = ( constraintValues[i] -
        constraintMappingOffsets[i] ) / constraintMappingMultipliers[i];
    }
    bestResponseArray.front().function_values(best_fns);
  }
}

} // namespace Dakota
