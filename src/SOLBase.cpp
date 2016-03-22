/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SOLBase
//- Description: Implementation code for the SOLBase class
//- Owner:       Mike Eldred
//- Checked by:

#include "SOLBase.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "DakotaMinimizer.hpp"
#include "DataMethod.hpp"
#include <boost/lexical_cast.hpp>
#include <sstream>

static const char rcsId[]="@(#) $Id: SOLBase.cpp 7004 2010-10-04 17:55:00Z wjbohnh $";


// BMA (20160315): Changed to use Fortran 2003 ISO C bindings.
// The Fortran symbol will be lowercase with same name as if in C
//#define NPOPTN2_F77 F77_FUNC(npoptn2,NPOPTN2)
#define NPOPTN2_F77 npoptn2
extern "C" void NPOPTN2_F77( const char* option_string );


namespace Dakota {

SOLBase*   SOLBase::solInstance(NULL);
Minimizer* SOLBase::optLSqInstance(NULL);


SOLBase::SOLBase(Model& model)
{
  // Prevent nesting of an instance of a Fortran iterator within another
  // instance of the same iterator (which would result in data clashes since
  // Fortran does not support object independence).  Recurse through all
  // sub-models and test each sub-iterator for NPSOL/NLSSOL presence.
  // Note: This check is performed for DOT, CONMIN, and SOLBase, but not
  //       for LHS (since it is only active in pre-processing) or SOL
  //       user-functions mode (since there is no model in this case).
  Iterator sub_iterator = model.subordinate_iterator();
  if (!sub_iterator.is_null() && 
      ( sub_iterator.method_name() ==  NPSOL_SQP ||
	sub_iterator.method_name() == NLSSOL_SQP ||
	sub_iterator.uses_method() ==  NPSOL_SQP ||
	sub_iterator.uses_method() == NLSSOL_SQP ) )
    sub_iterator.method_recourse();
  ModelList& sub_models = model.subordinate_models();
  for (ModelLIter ml_iter = sub_models.begin();
       ml_iter != sub_models.end(); ml_iter++) {
    sub_iterator = ml_iter->subordinate_iterator();
    if (!sub_iterator.is_null() && 
	 ( sub_iterator.method_name() ==  NPSOL_SQP ||
	   sub_iterator.method_name() == NLSSOL_SQP ||
	   sub_iterator.uses_method() ==  NPSOL_SQP ||
	   sub_iterator.uses_method() == NLSSOL_SQP ) )
      sub_iterator.method_recourse();
  }

  // Use constructor to populate only the problem_db attributes inherited by 
  // NPSOL/NLSSOL from SOLBase (none currently).  For attributes inherited by
  // NPSOL/NLSSOL from the Iterator hierarchy that are needed in SOLBase,
  // it's a bit cleaner/more flexible to have them passed through member
  // function parameter lists rather than re-extracted from problem_db.
  //const ProblemDescDB& problem_db = model.problem_description_db();
}


void SOLBase::
allocate_arrays(int num_cv, size_t num_nln_con,
		const RealMatrix& lin_ineq_coeffs,
		const RealMatrix& lin_eq_coeffs)
{
  // NPSOL directly handles equality constraints and 1- or 2-sided inequalities
  size_t num_lin_ineq_con = lin_ineq_coeffs.numRows(),
         num_lin_eq_con   = lin_eq_coeffs.numRows(),
         num_lin_con      = num_lin_ineq_con + num_lin_eq_con;

  // The Fortran optimizers' need for a nonzero array size is handled with 
  // nlnConstraintArraySize & linConstraintArraySize.
  nlnConstraintArraySize = (num_nln_con) ? num_nln_con : 1;
  linConstraintArraySize = (num_lin_con) ? num_lin_con : 1;

  // Matrix memory passed to Fortran must be contiguous
  linConstraintMatrixF77 = new double[linConstraintArraySize * num_cv];
  upperFactorHessianF77  = new double[num_cv * num_cv];
  constraintJacMatrixF77 = new double[nlnConstraintArraySize * num_cv];

  // Populate linConstraintMatrixF77 with linear coefficients from PDDB. Loop
  // order is reversed (j, then i) since Fortran matrix ordering is reversed 
  // from C ordering (linConstraintMatrixF77 is column-major order: columns
  // arranged head to tail).
  size_t i, j, cntr = 0;
  for (j=0; j<num_cv; j++) { // loop over columns
    for (i=0; i<num_lin_ineq_con; i++) // loop over inequality rows
      linConstraintMatrixF77[cntr++] = lin_ineq_coeffs(i,j);
    for (i=0; i<num_lin_eq_con; i++) // loop over equality rows
      linConstraintMatrixF77[cntr++] = lin_eq_coeffs(i,j);
  }

  boundsArraySize = num_cv + num_lin_con + num_nln_con;
  cLambda.resize(boundsArraySize);          // clambda[bnd_size]
  constraintState.resize(boundsArraySize);  // istate[bnd_size]
}


void SOLBase::deallocate_arrays()
{
  // Delete double* matrix allocations
  delete [] linConstraintMatrixF77;
  delete [] upperFactorHessianF77;
  delete [] constraintJacMatrixF77;
}


void SOLBase::allocate_workspace(int num_cv, int num_nln_con,
                                 int num_lin_con, int num_lsq)
{
  // see leniw/lenw discussion in "Subroutine npsol" section of NPSOL manual
  // for workspace size requirements.
  intWorkSpaceSize  = 3*num_cv + num_lin_con + 2*num_nln_con;
  if (num_lin_con == 0 && num_nln_con == 0)
    realWorkSpaceSize = 20*num_cv;
  else if (num_nln_con == 0)
    realWorkSpaceSize = 2*num_cv*num_cv + 20*num_cv + 11*num_lin_con;
  else
    realWorkSpaceSize = 2*num_cv*num_cv + num_cv*num_lin_con 
      + 2*num_cv*num_nln_con + 20*num_cv + 11*num_lin_con + 21*num_nln_con;
 
  // BMA, 20150604: workaround for out of bounds indexing:
  // At line 375 of file packages/NPSOL/npsolsubs.f
  // Fortran runtime error: Array reference out of bounds for array 'w', upper bound of dimension 1 exceeded (99 > 98)
  // TODO: look into updated NPSOL, as this may be masking a bug
  realWorkSpaceSize += 1;

  // in subroutine nlssol() in nlssolsubs.f, subroutine nlloc() adds the
  // following to the result from nploc().
  realWorkSpaceSize += 3*num_lsq + num_lsq*num_cv;

  realWorkSpace.resize(realWorkSpaceSize);  // work[lwork]
  intWorkSpace.resize(intWorkSpaceSize);    // iwork[liwork]
}


void SOLBase::set_options(bool speculative_flag, bool vendor_num_grad_flag, 
                          short output_lev, int verify_lev, Real fn_prec,
			  Real linesrch_tol, int max_iter, Real constr_tol,
                          Real conv_tol, const std::string& grad_type,
                          const RealVector& fdss)
{
  // Set NPSOL options (see "Optional Input Parameters" section of NPSOL manual)

  // The subroutine npoptn2 in file npoptn_wrapper.f accepts a string of 
  // length 72 (the max that NPSOL accepts) which is then passed along to
  // the npoptn routine in NPSOL. Therefore, strings passed to npoptn2 need
  // to be of length 72 (thus, the use of data() rather than c_str()).
 
  // Each of NPSOL's settings is an optional parameter in dakota.input.nspec, 
  // but always assigning them is OK since they are always defined, either from
  // dakota.in or from the default specified in the DataMethod constructor.
  // However, this approach may not use the NPSOL default:

  // If speculative_flag is set and numerical_gradients are used, then check 
  // method_source for dakota setting (behaves like speculative) or vendor 
  // setting (doesn't behave like speculative approach) and output info message
  if (speculative_flag) {
    Cerr << "\nWarning: speculative setting is ignored by SOL methods.";
    if (vendor_num_grad_flag)
      Cerr << "\n         A value-based line search will be used for vendor"
	   << "\n         numerical gradients.\n";
    else
      Cerr << "\n         A gradient-based line-search will be used for "
	   << "\n         analytic and dakota numerical gradients.\n";
  }

  // For each option, set the character data width to 72.
  // Pad the right hand end of the string with white space to ensure success
  // for compilers that have problems with ios formatting (e.g., janus, Linux).
  // NOTE: returned buffers from std::string::data() are NOT Null terminated!
  std::string verify_s("Verify Level                = ");
  verify_s += boost::lexical_cast<std::string>(verify_lev);
  verify_s.resize(72, ' ');
  NPOPTN2_F77( verify_s.data() );

  // Default NPSOL function precision is frequently tighter than DAKOTA's 
  // 11-digit precision.  Scaling back NPSOL's precision prevents wasted fn. 
  // evals. which are seeking to discern between design points that appear
  // identical in DAKOTA's precision. Unfortunately, NPSOL's definition of
  // Function Precision is not as simple as number of significant figures (see
  // "Optional Input Parameters" section of NPSOL manual); appropriate precision
  // is linked to the scale of f.  Default ProblemDescDB::functionPrecision is
  // 1e-10, but this may not always be appropriate depending on the scale of f.
  std::ostringstream fnprec_stream;
  fnprec_stream << "Function Precision          = "
                << std::setiosflags(std::ios::left) << std::setw(26) << fn_prec;
  std::string fnprec_s( fnprec_stream.str() );
  fnprec_s.resize(72, ' ');
  NPOPTN2_F77( fnprec_s.data() );

  std::ostringstream lstol_stream;
  lstol_stream << "Linesearch Tolerance        = "
               << std::setiosflags(std::ios::left)
               << std::setw(26) << linesrch_tol;
  std::string lstol_s( lstol_stream.str() );
  lstol_s.resize(72, ' ');
  NPOPTN2_F77( lstol_s.data() );

  std::string maxiter_s("Major Iteration Limit       = ");
  maxiter_s += boost::lexical_cast<std::string>(max_iter);
  maxiter_s.resize(72, ' ');
  NPOPTN2_F77( maxiter_s.data() );

  if (output_lev > NORMAL_OUTPUT) {
    std::string plevel_s("Major Print Level           = 20");
    plevel_s.resize(72, ' ');
    NPOPTN2_F77( plevel_s.data() );
    Cout << "\nNPSOL option settings:\n----------------------\n" 
         << verify_s.c_str() << '\n' << "Major Print Level           = 20\n" 
         << fnprec_s.c_str() << '\n' << lstol_s.c_str() << '\n'
	 << maxiter_s.c_str() << '\n';
  }
  else {
    std::string plevel_s("Major Print Level           = 10");
    plevel_s.resize(72, ' ');
    NPOPTN2_F77( plevel_s.data() );
  }

  // assign a nondefault linear/nonlinear constraint tolerance if a valid
  // value has been set in dakota.in; otherwise utilize the NPSOL default.
  if (constr_tol > 0.0) {
    std::ostringstream ct_tol_stream;
    ct_tol_stream << "Feasibility Tolerance       = "
                  << std::setiosflags(std::ios::left)
                  << std::setw(26) << constr_tol;
    std::string ct_tol_s( ct_tol_stream.str() );
    ct_tol_s.resize(72, ' ');
    NPOPTN2_F77( ct_tol_s.data() );
    if (output_lev > NORMAL_OUTPUT)
      Cout << ct_tol_s.c_str() << '\n';
  }

  // conv_tol is an optional parameter in dakota.input.nspec, but
  // defining our own default (in the DataMethod constructor) and
  // always assigning it applies some consistency across methods.
  // Therefore, the NPSOL default is not used.
  std::ostringstream ctol_stream;
  ctol_stream << "Optimality Tolerance        = "
              << std::setiosflags(std::ios::left) << std::setw(26) << conv_tol;
  std::string ctol_s( ctol_stream.str() );
  ctol_s.resize(72, ' ');
  NPOPTN2_F77( ctol_s.data() );
  if (output_lev > NORMAL_OUTPUT)
    Cout << ctol_s.c_str() << "\nNOTE: NPSOL's convergence tolerance is not a "
	 << "relative tolerance.\n      See \"Optimality tolerance\" in "
         << "Optional Input Parameters section of \n      NPSOL manual for "
         << "description.\n";

  // Set Derivative Level = 3 for user-supplied gradients (analytic, dakota 
  // numerical, or mixed analytic/dakota numerical gradients).  This does NOT
  // take advantage of NPSOL's internal mixed gradient capability, but is 
  // simpler, parallelizable, and more consistent with the other optimizers.
  // Inactive functions have been observed in NPSOL's internal mixed gradient
  // mode, so support for NPSOL's internal mixed gradients might add efficiency.
  if ( grad_type == "analytic" || grad_type == "mixed" || 
       ( grad_type == "numerical" && !vendor_num_grad_flag ) ) {
    // user-supplied gradients: Derivative Level = 3
    std::string dlevel_s("Derivative Level            = 3");
    dlevel_s.resize(72, ' ');
    NPOPTN2_F77( dlevel_s.data() );
    if (output_lev > NORMAL_OUTPUT)
      Cout << "Derivative Level            = 3\n";
  }
  else if (grad_type == "none") {
    Cerr << "\nError: gradient type = none is invalid with SOL methods.\n"
         << "Please select numerical, analytic, or mixed gradients."
         << std::endl;
    abort_handler(-1);
  }
  else { // vendor numerical gradients: Derivative Level = 0. No forward/central
         // interval type control, since NPSOL switches automatically.
    std::string dlevel_s("Derivative Level            = 0");
    dlevel_s.resize(72, ' ');
    NPOPTN2_F77( dlevel_s.data() );

    std::ostringstream fdss_stream;
    Real fd_step_size = fdss[0]; // first entry
    fdss_stream << "Difference Interval         = "
                << std::setiosflags(std::ios::left) << std::setw(26)
		<< fd_step_size;
    std::string fdss_s( fdss_stream.str() );
    fdss_s.resize(72, ' ');
    NPOPTN2_F77( fdss_s.data() );
  
    // Set "Central Difference Interval" to fdss as well.
    // It may be desirable to set central FDSS to fdss/2. (?)
    std::ostringstream cfdss_stream;
    cfdss_stream << "Central Difference Interval = "
                 << std::setiosflags(std::ios::left) << std::setw(26)
		 << fd_step_size;
    std::string cfdss_s( cfdss_stream.str() );
    cfdss_s.resize(72, ' ');
    NPOPTN2_F77( cfdss_s.data() );
  
    if (output_lev > NORMAL_OUTPUT)
      Cout << "Derivative Level            = 0\n" << fdss_s.c_str() << '\n'
	   << cfdss_s.c_str() << "\nNOTE: NPSOL's finite difference interval "
	   << "uses a unit offset to remove the\n      need for a minimum step "
	   << "specification (see \"Difference interval\" in\n      Optional "
           << "Input Parameters section of NPSOL manual).\n"
	   << "Interval type ignored since NPSOL automatically selects\n"
	   << "and switches between forward and central differences.\n\n";
  }
}


void SOLBase::
augment_bounds(RealVector& augmented_l_bnds,
	       RealVector& augmented_u_bnds,
	       const RealVector& lin_ineq_l_bnds,
	       const RealVector& lin_ineq_u_bnds,
	       const RealVector& lin_eq_targets,
	       const RealVector& nln_ineq_l_bnds,
	       const RealVector& nln_ineq_u_bnds,
	       const RealVector& nln_eq_targets)
{
  // Construct augmented_l_bnds & augmented_u_bnds from variable bounds,
  // linear inequality bounds and equality targets, and nonlinear inequality
  // bounds and equality targets.  Arrays passed in are assumed to already 
  // contain the variable bounds and are augmented with linear and nonlinear
  // constraint bounds.  Note: bounds above or below NPSOL's "Infinite bound
  // size" (see bl/bu in "Subroutine npsol" section and Infinite bound size in
  // "Optional Input Parameters" section of NPSOL manual) are ignored. 
  // DAKOTA's default bounds for no user specification are set in
  // ProblemDescDB::responses_kwhandler for nonlinear constraints and in
  // Constraints::manage_linear_constraints for linear constraints.

  size_t num_cv       = augmented_l_bnds.length(),
         num_lin_ineq = lin_ineq_l_bnds.length(),
         num_lin_eq   = lin_eq_targets.length(),
         num_nln_ineq = nln_ineq_l_bnds.length(),
         num_nln_eq   = nln_eq_targets.length();
  if (boundsArraySize != num_cv + num_lin_ineq + num_lin_eq +
                         num_nln_ineq + num_nln_eq) {
    Cerr << "Error: bad boundsArraySize in SOLBase::augment_bounds."
         << std::endl;
    abort_handler(-1);
  }

  augmented_l_bnds.resize(boundsArraySize); // retains variables data
  augmented_u_bnds.resize(boundsArraySize); // retains variables data
  size_t i, cntr = num_cv;
  for (i=0; i<num_lin_ineq; i++) { // linear inequality
    augmented_l_bnds[cntr] = lin_ineq_l_bnds[i];
    augmented_u_bnds[cntr] = lin_ineq_u_bnds[i];
    cntr++;
  }
  for (i=0; i<num_lin_eq; i++) { // linear equality
    augmented_l_bnds[cntr] = lin_eq_targets[i];
    augmented_u_bnds[cntr] = lin_eq_targets[i];
    cntr++;
  }
  for (i=0; i<num_nln_ineq; i++) { // nonlinear inequality
    augmented_l_bnds[cntr] = nln_ineq_l_bnds[i];
    augmented_u_bnds[cntr] = nln_ineq_u_bnds[i];
    cntr++;
  }
  for (i=0; i<num_nln_eq; i++) { // nonlinear equality
    augmented_l_bnds[cntr] = nln_eq_targets[i];
    augmented_u_bnds[cntr] = nln_eq_targets[i];
    cntr++;
  }
}


void SOLBase::
constraint_eval(int& mode, int& ncnln, int& n, int& nrowj, int* needc,
                double* x, double* c, double* cjac, int& nstate)
{
  // This routine is called before objective_eval, but is only called if
  // there are nonlinear constraints (numNonlinearConstraints != 0)
  
  // Constraint requests are governed by "mode" and "needc":
  //   SOL mode=2: get active values and gradients using needc
  //   SOL mode=1: get active gradients using needc
  //   SOL mode=0: get active values using needc

  // Adjust SOL's mode requests for case of vendor numerical gradients.
  // [SOL's requests are valid since Derivative Level=0 means that SOME
  // elements of the objFunctionGradient & Jacobian are unavailable. However,
  // Dakota's numerical gradient mode currently assumes that NO gradient data
  // is available, which means that some SOL requests need to be modified.
  // If mixed _vendor_ gradients are supported in a future Dakota version,
  // these adjustments should be removed.]
  int i;
  short asv_request = mode + 1;
  //if ( !(solInstance->derivLevel & 2) && (asv_request & 2) ) { // more general
  if (optLSqInstance->vendorNumericalGradFlag && asv_request & 2) {
    asv_request -= 2; // downgrade request
    Cout << "SOL has requested user-supplied constraint gradients for case of "
         << "vendor numerical gradients.\n";
    if (asv_request)
      Cout << "Request will be downgraded to function values alone.\n"
           << std::endl;
    else
      Cout << "Request will be ignored and no evaluation performed.\n"
           << std::endl;
  }

  if (asv_request) {
    // Generate local_asv from asv_request (not mode) to accomodate special case
    // behavior. If needc[i]>0, then make asv_request request for constraint i.
    // For local_asv[0], assume the same request for the obj. fn.  NOTE: This 
    // would be wasteful in a future implementation for mixed gradients, since
    // it is possible to have requests for FD evaluation of constraint values
    // only or of the obj. fn. only (Derivative Level = 0, 1, or 2).
    ShortArray local_asv(solInstance->constrOffset + ncnln);
    for (i=0; i<solInstance->constrOffset; i++)
      local_asv[i] = asv_request;
    for (i=0; i<ncnln; i++)
      local_asv[i+solInstance->constrOffset] = (needc[i] > 0) ? asv_request : 0;
      // NOTE: SOL does not appear to use needc[i] for active set requests
      // (SOL always needs all gradients, even for constraints which are
      // inactive by a lot). So far, needc[i]=0 has only been observed for
      // partial finite difference requests in the case of mixed gradients.

    // Update model variables from x for use in evaluate()
    RealVector local_des_vars(Teuchos::Copy, x, n);
    optLSqInstance->iteratedModel.continuous_variables(local_des_vars);

    optLSqInstance->activeSet.request_vector(local_asv);
    optLSqInstance->iteratedModel.evaluate(optLSqInstance->activeSet);
    solInstance->fnEvalCntr++;
  }
  
  // Follow asv_request request (special case exceptions handled above).
  // Could follow local_asv entry-by-entry, but the design below is 
  // easier since the Response structure matches that needed by SOL.
  const Response& local_response
    = optLSqInstance->iteratedModel.current_response();
  if (asv_request & 1) {
    // Direct use of the array class assignment operator works
    // fine in DOTOptimizer, but causes major memory problems in npsol!
    // So use a more "brute force" approach to assign the constraint values.
    const RealVector& local_fn_vals = local_response.function_values();
    for (i=0; i<ncnln; i++)
      c[i] = local_fn_vals[i+solInstance->constrOffset];
  }

  if (asv_request & 2) {
    const RealMatrix& local_fn_grads = local_response.function_gradients();

    // No need to concatenate active gradients (as in DOT).  Response population
    // mechanism sets inactive constraint entries to zero which works fine with
    // SOL so long as the gradients are either exclusively user-supplied or
    // exclusively SOL-computed (with finite differences).  In the former case
    // (Der. Level = 3), *all* of cjac is specified and inactive entries are
    // ignored, and in the latter case (Der. Level = 0), this code is not used
    // and cjac is left at its special value (see "Constant Jacobian elements"
    // section in NPSOL manual). For a future implementation of mixed _vendor_
    // gradients (some user-supplied, some not), unavailable cjac elements must
    // *not* be set to zero since SOL uses detection of the special value to
    // determine which cjac entries require finite differencing.
      
    // Loop order is reversed (j, then i) since Fortran matrix ordering is
    // reversed from C ordering. Gradients are not mixed, so assign each
    // entry of cjac (inactive entries will be zero, but SOL ignores them).
    int j, cntr = 0;
    for (j=0; j<n; j++)
      for (i=solInstance->constrOffset; i<solInstance->constrOffset+ncnln; i++)
	cjac[cntr++] = local_fn_grads(j,i);
  }
}

} // namespace Dakota
