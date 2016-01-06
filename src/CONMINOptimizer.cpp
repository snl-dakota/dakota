/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       CONMINOptimizer
//- Description: Implementation code for the CONMINOptimizer class
//- Owner:       Tony Giunta
//- Checked by:

#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "CONMINOptimizer.hpp"
#include "ProblemDescDB.hpp"

static const char rcsId[]="@(#) $Id: CONMINOptimizer.cpp 7029 2010-10-22 00:17:02Z mseldre $";

// Prototype for a direct call to the F77 CONMIN code
#define CONMIN_F77 F77_FUNC(conmin,CONMIN)
extern "C" void CONMIN_F77(double*, double*, double*, double*, double*, double*,
			   double*, double*, double*, double*, double*, double*,
			   int*, int*, int*, int&, int&, int&, int&, int&,
			   double&, double&, double&, double&, double&, double&,
			   double&, double&, double&, double&, double&, double&,
			   int&, int&, int&, int&, int&, int&, int&, int&,
			   int&, int&, int&, int&, int&, int&, int&);

using namespace std;

namespace Dakota {


CONMINOptimizer::CONMINOptimizer(ProblemDescDB& problem_db, Model& model):
  Optimizer(problem_db, model)
{
  // If speculativeFlag is set with vendor numerical_gradients, output a warning
  if (speculativeFlag && vendorNumericalGradFlag)
    Cerr << "\nWarning: speculative method specification is ignored for"
	 << "\n         vendor numerical gradients.\n\n";

  initialize(); // convenience fn for shared ctor code
}


CONMINOptimizer::CONMINOptimizer(const String& method_string, Model& model):
  Optimizer(method_string_to_enum(method_string), model)
{ initialize(); } // convenience fn for shared ctor code


void CONMINOptimizer::initialize()
{
  // Prevent nesting of an instance of a Fortran iterator within another
  // instance of the same iterator (which would result in data clashes since
  // Fortran does not support object independence).  Recurse through all
  // sub-models and test each sub-iterator for CONMIN presence.
  // Note: This check is performed for DOT, CONMIN, and SOLBase, but not
  //       for LHS since it is only active in pre-processing.
  Iterator sub_iterator = iteratedModel.subordinate_iterator();
  if (!sub_iterator.is_null() && 
       ( sub_iterator.method_name() == CONMIN_FRCG ||
	 sub_iterator.method_name() == CONMIN_MFD  ||
	 sub_iterator.uses_method() == CONMIN_FRCG ||
	 sub_iterator.uses_method() == CONMIN_MFD ) )
    sub_iterator.method_recourse();
  ModelList& sub_models = iteratedModel.subordinate_models();
  for (ModelLIter ml_iter = sub_models.begin();
       ml_iter != sub_models.end(); ml_iter++) {
    sub_iterator = ml_iter->subordinate_iterator();
    if (!sub_iterator.is_null() && 
	 ( sub_iterator.method_name() == CONMIN_FRCG ||
	   sub_iterator.method_name() == CONMIN_MFD  ||
	   sub_iterator.uses_method() == CONMIN_FRCG ||
	   sub_iterator.uses_method() == CONMIN_MFD ) )
      sub_iterator.method_recourse();
  }

  // Initialize CONMIN specific data
  NFDG   = 0;       // default finite difference flag
  IPRINT = 1;       // default flag to control amount of output info
  ITMAX  = 100;     // default max number of iterations
  FDCH   =  1.0e-5; // default relative finite difference step size
  FDCHM  =  1.0e-5; // default absolute finite difference step size
  CT     = -0.1;    // default constraint thickness tolerance
                    // (for determining active/inactive constraint status)
  CTMIN  =  0.001;  // default absolute constraint tolerance 
                    // (note: the CONMIN manual default is 0.004)
  CTL    = -0.01;   // default side constraint thickness tolerance (see CT)
  CTLMIN =  0.001;  // default absolute side constraint tolerance
  DELFUN =  1.0e-7; // default minimum relative change in the objective
                    // function needed for convergence
  DABFUN =  1.0e-7; // default minimum absolute change in the objective
                    // function needed for convergence

  conminInfo = 0; // Must be set to 0 before calling CONMIN
  ITMAX = maxIterations;

  // Set the print control flag
  if (outputLevel > NORMAL_OUTPUT) {
    IPRINT = printControl = 4;
    Cout << "CONMIN print control = " << printControl << endl;
  }
  else
    IPRINT = printControl = 2;

  // assigns a nondefault constraint tolerance if a valid value has been 
  // set in dakota.in; otherwise use CONMIN default.
  if (constraintTol > 0.0) {
    CTMIN = CTLMIN = constraintTol;
    if (outputLevel > QUIET_OUTPUT)
      Cout << "constraint violation tolerance = " << constraintTol << '\n';
  }

  // convergenceTol is an optional parameter in dakota.input.nspec, but
  // defining our own default (in the DataMethod constructor) and
  // always assigning it applies some consistency across methods.
  // Therefore, the CONMIN default is not used.
  DELFUN = DABFUN = convergenceTol; // needed in CONMIN

  // Default CONMIN gradients = numerical;forward;vendor setting.
  const String& grad_type     = iteratedModel.gradient_type();
  const String& method_src    = iteratedModel.method_source();
  const String& interval_type = iteratedModel.interval_type();
  if ( grad_type == "analytic" || grad_type == "mixed" || 
       ( grad_type == "numerical" && method_src == "dakota" ) )
    // Set NFDG=1 before calling CONMIN. This invokes the
    // user-supplied gradient mode which DAKOTA uses for analytic,
    // dakota numerical, or mixed analytic/dakota numerical gradients.
    NFDG=1; 
  else if (grad_type == "none") {
    Cerr << "\nError: gradient type = none is invalid with CONMIN.\n"
         << "Please select numerical, analytic, or mixed gradients." << endl;
    abort_handler(-1);
  }
  else if (interval_type == "central") {
    Cerr << "\nFinite Difference Type = 'central' is invalid with CONMIN.\n"
         << "Forward difference is only available internal to CONMIN." << endl;
    abort_handler(-1);
  }
  else { // Vendor numerical gradients with forward differences
    NFDG = 0; // use CONMIN's default internal forward difference method

    Real fd_grad_ss = iteratedModel.fd_gradient_step_size()[0];

    // CONMIN's forward differencing uses fdss*X_i as one would expect
    FDCH = fd_grad_ss;

    // for FDCHM (minimum delta), use 2 orders of magnitude smaller than fdss to
    // be consistent with Model::estimate_derivatives():
    FDCHM = fd_grad_ss*.01;
  }
}


CONMINOptimizer::~CONMINOptimizer()
{
  // Virtual destructor handles referenceCount at Iterator level
}


void CONMINOptimizer::allocate_constraints()
{
  // CONMIN handles all constraints as 1-sided inequalities.  Compute the number
  // of 1-sided inequalities to pass to CONMIN (numConminConstr) as well as the
  // mappings (indices, multipliers, offsets) between the DAKOTA constraints and
  // the CONMIN constraints.  TO DO: support for automatic constraint scaling.
  // 
  // ***Note: CONMIN has difficulty when handling a nonlinear equality
  // constraint as two oppositely-signed inequality constraints (CONMIN's MFD
  // algorithm cannot handle the nonlinearity created by the two inequalities).
  // This is the reason behind the Modified MFD in DOT.  See Vanderplaats' book,
  // Numer. Opt. Techniques for Eng. Design, for additional details.
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
  numConminNlnConstr = 2*num_nln_eq;
  for (i=0; i<num_nln_ineq; i++) {
    if (nln_ineq_lwr_bnds[i] > -bigRealBoundSize) {
      numConminNlnConstr++;
      // nln_ineq_lower_bnd - dakota_constraint <= 0
      constraintMappingIndices.push_back(i);
      constraintMappingMultipliers.push_back(-1.0);
      constraintMappingOffsets.push_back(nln_ineq_lwr_bnds[i]);
    }
    if (nln_ineq_upr_bnds[i] < bigRealBoundSize) {
      numConminNlnConstr++;
      // dakota_constraint - nln_ineq_upper_bnd <= 0
      constraintMappingIndices.push_back(i);
      constraintMappingMultipliers.push_back(1.0);
      constraintMappingOffsets.push_back(-nln_ineq_upr_bnds[i]);
    }
  }
  for (i=0; i<num_nln_eq; i++) {
    // nln_eq_target - dakota_constraint <= 0
    constraintMappingIndices.push_back(i+num_nln_ineq);
    constraintMappingMultipliers.push_back(-1.0);
    constraintMappingOffsets.push_back(nln_eq_targets[i]);
    // dakota_constraint - nln_eq_target <= 0
    constraintMappingIndices.push_back(i+num_nln_ineq);
    constraintMappingMultipliers.push_back(1.0);
    constraintMappingOffsets.push_back(-nln_eq_targets[i]);
  }

  numConminLinConstr = 2*num_lin_eq;
  for (i=0; i<num_lin_ineq; i++) {
    if (lin_ineq_lwr_bnds[i] > -bigRealBoundSize) {
      numConminLinConstr++;
      // lin_ineq_lower_bnd - Ax <= 0
      constraintMappingIndices.push_back(i);
      constraintMappingMultipliers.push_back(-1.0);
      constraintMappingOffsets.push_back(lin_ineq_lwr_bnds[i]);
    }
    if (lin_ineq_upr_bnds[i] < bigRealBoundSize) {
      numConminLinConstr++;
      // Ax - lin_ineq_upper_bnd <= 0
      constraintMappingIndices.push_back(i);
      constraintMappingMultipliers.push_back(1.0);
      constraintMappingOffsets.push_back(-lin_ineq_upr_bnds[i]);
    }
  }
  for (i=0; i<num_lin_eq; i++) {
    // lin_eq_target - Ax <= 0
    constraintMappingIndices.push_back(i+num_lin_ineq);
    constraintMappingMultipliers.push_back(-1.0);
    constraintMappingOffsets.push_back(lin_eq_targets[i]);
    // Ax - lin_eq_target <= 0
    constraintMappingIndices.push_back(i+num_lin_ineq);
    constraintMappingMultipliers.push_back(1.0);
    constraintMappingOffsets.push_back(-lin_eq_targets[i]);
  }

  numConminConstr = numConminNlnConstr + numConminLinConstr;

  // Check method setting versus constraints
  if (methodName == CONMIN_MFD && !numConminConstr) {
    Cerr << "\nWarning: for no constraints, conmin_mfd request will be changed"
	 << "\n         to conmin_frcg.\n\n";
    methodName = CONMIN_FRCG; // for output header/footer
  }
  else if (methodName == CONMIN_FRCG && numConminConstr) {
    Cerr << "\nWarning: for constrained optimization, conmin_frcg request will"
	 << "\n         be changed to conmin_mfd.\n\n";
    methodName = CONMIN_MFD; // for output header/footer
  }
}


void CONMINOptimizer::allocate_workspace()
{
  // Compute sizes of arrays and matrices needed for CONMIN.
  // These sizes are listed in the 1978 Addendum to the CONMIN User's Manual.
  N1 = numContinuousVars + 2;
  N2 = numContinuousVars*2 + numConminConstr;
  // N3 = 1 + an estimate of the max # of active constraints,
  // including side constraints. The largest N3 can be is
  // N3 = 1 + numConminConstr + numContinuousVars
  // (i.e., there are, at most, numContinuousVars active side constraints)
  // NOTE: with a bad user input of lowerBnd==upperBnd, this could break down.
  N3 = 1 + numConminConstr + numContinuousVars;
  N4 = (N3 >= numContinuousVars) ? N3 : numContinuousVars; // always N3
  N5 = 2*N4;

  // Declare arrays needed for CONMIN. The sizes for these arrays are
  // listed in the 1978 Addendum to the CONMIN User's Manual
  conminDesVars   = new Real[N1];
  conminLowerBnds = new Real[N1];  
  conminUpperBnds = new Real[N1];
  S               = new Real[N1];
  G1              = new Real[N2];
  G2              = new Real[N2];
  B               = new Real[N3*N3];
  C               = new Real[N4];
  MS1             = new int[N5];
  SCAL            = new Real[N1];
  DF              = new Real[N1];
  A               = new Real[N1*N3];
  ISC             = new int[N2];
  IC              = new int[N3];

  // Size the array that holds the constraint values.
  // For CONMIN the minimum size of the vector of constraints is "N2"
  constraintValues.resize(N2);
}


void CONMINOptimizer::deallocate_workspace()
{
  delete [] conminDesVars;
  delete [] conminLowerBnds;
  delete [] conminUpperBnds;
  delete [] S;
  delete [] G1;
  delete [] G2;
  delete [] B;
  delete [] C;
  delete [] MS1;
  delete [] SCAL;
  delete [] DF;
  delete [] A;
  delete [] ISC;
  delete [] IC;
}


void CONMINOptimizer::initialize_run()
{
  Optimizer::initialize_run();

  // Allocate space for CONMIN arrays
  allocate_constraints();
  allocate_workspace();

  // initialize the IC and ISC vectors
  size_t i;
  for (i=0; i<numConminConstr; i++)
    IC[i] = ISC[i] = 0;

  // Initialize CONMIN's local vars and bounds arrays
  // Note: these are different than DAKOTA's local vars and bounds arrays
  // because CONMIN uses arrays for vars and bounds that are larger than
  // used by DAKOTA, i.e., DAKOTA's local_cdv has length numContinuousVars,
  // and CONMIN's conminDesVars has length N1 = numContinuousVars+2
  //
  // copy DAKOTA arrays to CONMIN arrays and check for the existence of bounds.
  const RealVector& local_cdv  = iteratedModel.continuous_variables();
  const RealVector& lower_bnds = iteratedModel.continuous_lower_bounds();
  const RealVector& upper_bnds = iteratedModel.continuous_upper_bounds();
  for (i=0; i<numContinuousVars; i++) {
    conminDesVars[i]   = local_cdv[i];
    conminLowerBnds[i] = lower_bnds[i];
    conminUpperBnds[i] = upper_bnds[i];
  }
  // Initialize array padding (N1 = numContinuousVars + 2).
  for (i=numContinuousVars; i<N1; i++)
    conminDesVars[i] = conminLowerBnds[i] = conminUpperBnds[i] = 0.;
}


void CONMINOptimizer::core_run()
{
  size_t i, j, fn_eval_cntr;
  int num_cv = numContinuousVars;

  // Any MOO/NLS recasting is responsible for setting the scalar min/max
  // sense within the recast.
  const BoolDeque& max_sense = iteratedModel.primary_response_fn_sense();
  bool max_flag = (!max_sense.empty() && max_sense[0]);

  // Initialize variables internal to CONMIN
  int NSIDE     = 0;   // flag for upper/lower var bounds: 1=bounds, 0=no bounds
  // NSIDE must be set to 0 for unbounded since CONMIN cannot handle having
  // upper/lower bounds set to +/-inf.  Set NSIDE to 1 if bounds arrays have
  // nondefault values.
  for (i=0; i<numContinuousVars; i++) {
    if (conminLowerBnds[i] > -bigRealBoundSize ||
	conminUpperBnds[i] <  bigRealBoundSize) {
      NSIDE = 1;
      break;
    }
  }
  int ICNDIR    = numContinuousVars+1; // conjugate direction restart parameter
  int NSCAL     = 0;   // (unused) scaling flag
  //int NACMX1  = N3;  // estimate of 1+(max # of active constraints)
  int LINOBJ    = 0;   // (unused) set to 1 if obj fcn is linear
  int ITRM      = 3;   // number of consecutive iterations of less than DELFUN
                       // or DABFUN progress before optimization terminated
  double THETA  = 1.0; // mean value of "push-off factor" in mtd of feasible
                       // directions
  //double PHI  = 5.0; // "participation coefficient" to force an infeasible
                       // design to be feasible
  double ALPHAX = 0.1; // 1-D search fractional change parameter
  double ABOBJ1 = 0.1; // 1-D search fractional change parameter for 1st step

  // variables for loop that calls CONMIN
  int IGOTO     = 0;   // CONMIN internal optimization termination flag: 0=stop
  int NAC;             // number of active constraints computed in CONMIN
  int INFOG;           // (unused) flag that allows for non-essential function
                       // evaluations in FD gradients to be avoided
  int ITER;            // CONMIN internal iteration counter

  // Initialize local vars and linear constraints
  RealVector local_cdv(numContinuousVars);
  size_t num_lin_ineq = iteratedModel.num_linear_ineq_constraints(),
    num_lin_eq = iteratedModel.num_linear_eq_constraints();
  const RealMatrix& lin_ineq_coeffs
    = iteratedModel.linear_ineq_constraint_coeffs();
  const RealMatrix& lin_eq_coeffs
    = iteratedModel.linear_eq_constraint_coeffs();
  const String& grad_type = iteratedModel.gradient_type();

  // Start FOR loop to execute calls to CONMIN
  for (fn_eval_cntr=1; fn_eval_cntr<=maxFunctionEvals; fn_eval_cntr++) {

    CONMIN_F77(conminDesVars, conminLowerBnds, conminUpperBnds,
	       constraintValues.values(), SCAL, DF, A, S, G1, G2, B, C, ISC, IC,
	       MS1, N1, N2, N3, N4, N5, DELFUN, DABFUN, FDCH, FDCHM, CT, CTMIN,
	       CTL, CTLMIN, ALPHAX, ABOBJ1, THETA, objFnValue, num_cv,
	       numConminConstr, NSIDE, IPRINT, NFDG, NSCAL, LINOBJ, ITMAX, ITRM,
	       ICNDIR, IGOTO, NAC, conminInfo, INFOG, ITER);

    if (IGOTO == 0) break; // CONMIN's flag that optimization is complete

    // conminInfo=1: Line search/Finite differencing, eval obj & all constr fns.
    // conminInfo=2: Evaluate gradients of objective & active constraints
    if (conminInfo == 1) {
      if (outputLevel > NORMAL_OUTPUT) // output info on CONMIN request
	Cout << "\nCONMIN requests function values:";
      if (speculativeFlag && !vendorNumericalGradFlag) {
	if (outputLevel > NORMAL_OUTPUT)
	  Cout << "\nSpeculative optimization: evaluation augmented with "
	       << "speculative gradients.";
	activeSet.request_values(3);
      }
      else 
	activeSet.request_values(1);
    }
    else if (conminInfo == 2) {
      if (outputLevel > NORMAL_OUTPUT) { // output info on CONMIN request
	if (grad_type == "numerical")
	  Cout << "\nCONMIN requests dakota-numerical gradients:";
	else
	  Cout << "\nCONMIN requests analytic gradients:";
	if (speculativeFlag && !vendorNumericalGradFlag)
	  Cout << "\nSpeculative optimization: retrieving gradients already "
	       << "evaluated from database.";
      }
      activeSet.request_values(0);
      for (i=0; i<numObjectiveFns; i++)
	activeSet.request_value(conminInfo, i); // objective function(s)
      // CONMIN does not compute the number of active constraints and 
      // requires the user to do so.  Store this value in CONMIN's variable
      // NAC and store the indices of the constraints in array IC.
      NAC = 0;
      for (i=0; i<numConminConstr; i++)
      	if ( constraintValues[i] >= CT )
	  IC[NAC++] = i + 1; // The +1 is needed for F77 array indexing

      // for each of the active constraints, assign a value to DAKOTA's
      // active set vector
      for (i=0; i<NAC; i++) {
	// IC contains the active constraint #'s While CONMIN returns
	// 1-based constraint id's, work in 0-based id's for indexing
	// convenience.
	size_t conmin_constr = IC[i] - 1; // (0-based)
	if (conmin_constr < numConminNlnConstr) {
	  size_t dakota_constr = constraintMappingIndices[conmin_constr];
	  activeSet.request_value(conminInfo, dakota_constr + numObjectiveFns);
	  // some DAKOTA equality and 2-sided inequality constraints may
	  // have their ASV assigned multiple times depending on which of
	  // CONMIN's 1-sided inequalities are active.
	}
      }
    }

    copy_data(conminDesVars, num_cv, local_cdv);
    iteratedModel.continuous_variables(local_cdv);
    iteratedModel.evaluate(activeSet);
    const Response& local_response = iteratedModel.current_response();

    // Populate proper data for input back to CONMIN through parameter list.
    // This include gradient data, along with the number and indices of
    // the active constraints.
    if (conminInfo == 2) {
      // Populate CONMIN's obj fcn gradient vector "DF"
      const RealMatrix& local_fn_grads = local_response.function_gradients();
      const int  num_vars = local_fn_grads.numRows();
      for (j=0; j<num_vars; ++j)   // obj. fn. grad. always needed
	DF[j] = (max_flag) ? -local_fn_grads(j, 0) : local_fn_grads(j, 0);
      // Return the gradients of the active constraints in the matrix "A".
      for (i=0; i<NAC; i++) {
	// Populate CONMIN's active constraint gradient matrix "A".  For some
	// reason, CONMIN's A-matrix has a column length of N1 (and N1 =
	// numContinuousVars+2).
	size_t conmin_constr = IC[i]-1;
	size_t dakota_constr = constraintMappingIndices[conmin_constr];
        if (conmin_constr < numConminNlnConstr) { // nonlinear ineq & eq
          // gradients pick up multiplier mapping only (offsets drop out)
          for (j=0; j<num_vars; ++j)
            A[i*N1 + j] = constraintMappingMultipliers[conmin_constr] *
	      local_fn_grads(j,dakota_constr+1);
        }
        else if (dakota_constr < num_lin_ineq) { // linear ineq
	  for (j=0; j<num_vars; ++j)
	    A[i*N1 + j] = constraintMappingMultipliers[conmin_constr] *
	      lin_ineq_coeffs(dakota_constr,j);
	}
	else { // linear eq
	  size_t dakota_leq_constr = dakota_constr - num_lin_ineq;
	  for (j=0; j<num_vars; ++j)
	    A[i*N1 + j] = constraintMappingMultipliers[conmin_constr] *
	      lin_eq_coeffs(dakota_leq_constr,j);
        }
      }
    }
    else {
      // Get objective function and constraint values
      // note: no active/inactive distinction needed with constraints
      const RealVector& local_fn_vals = local_response.function_values();
      objFnValue = (max_flag) ? -local_fn_vals[0] : local_fn_vals[0];
      // CONMIN constraint requests must be mapped to DAKOTA constraints and
      // offsets/multipliers must be applied.
      size_t conmin_constr, dakota_constr;
      for (conmin_constr=0; conmin_constr<numConminConstr; conmin_constr++) {
        dakota_constr = constraintMappingIndices[conmin_constr];
        if (conmin_constr < numConminNlnConstr) // nonlinear ineq & eq
          constraintValues[conmin_constr] =
            constraintMappingOffsets[conmin_constr] +
            constraintMappingMultipliers[conmin_constr] *
            local_fn_vals[dakota_constr+1];
        else {
          Real Ax = 0.0;
          if (dakota_constr < num_lin_ineq) { // linear ineq
            for (j=0; j<numContinuousVars; j++)
              Ax += lin_ineq_coeffs(dakota_constr,j) * local_cdv[j];
          }
          else { // linear eq
            size_t dakota_leq_constr = dakota_constr - num_lin_ineq;
            for (j=0; j<numContinuousVars; j++)
              Ax += lin_eq_coeffs(dakota_leq_constr,j) * local_cdv[j];
          }
          constraintValues[conmin_constr] =
            constraintMappingOffsets[conmin_constr] +
            constraintMappingMultipliers[conmin_constr] * Ax;
        }
      }
    }

  } // end of FOR loop starting above call to CONMIN

  if (fn_eval_cntr == maxFunctionEvals+1)
    Cout << "Iteration terminated: max_function_evaluations limit has been "
         << "met.\n";

  // Set best variables and response for use by strategy level.
  // If conminInfo = 0, then optimization was complete and CONMIN
  // contains the best variables & responses data in the argument list
  // data structures.  If conminInfo is not 0, then there was an abort
  // (e.g., maxFunctionEvals) and it is not clear how to return the
  // best solution. For now, return the argument list data (as if
  // conminInfo = 0) which should be the last evaluation (?).
  copy_data(conminDesVars, num_cv, local_cdv);
  bestVariablesArray.front().continuous_variables(local_cdv);
  if (!localObjectiveRecast) { // else local_objective_recast_retrieve()
                               // used in Optimizer::post_run()
    RealVector best_fns(numFunctions);
    best_fns[0] = (max_flag) ? -objFnValue : objFnValue;
    // NOTE: best_fn_vals[i] may be recomputed multiple times, but this
    // should be OK so long as all of constraintValues is populated
    // (no active set deletions).
    for (size_t i=0; i<numConminNlnConstr; i++) {
      size_t dakota_constr = constraintMappingIndices[i];
      // back out the offset and multiplier
      best_fns[dakota_constr+1] = ( constraintValues[i] -
	constraintMappingOffsets[i] ) / constraintMappingMultipliers[i];
    }
    bestResponseArray.front().function_values(best_fns);
  }
  deallocate_workspace();
}

} // namespace Dakota
