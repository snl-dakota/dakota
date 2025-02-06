/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "SOLBase.hpp"
#include "DakotaResponse.hpp"
#include "DakotaMinimizer.hpp"
#include "DataMethod.hpp"
#include <sstream>

static const char rcsId[]="@(#) $Id: SOLBase.cpp 7004 2010-10-04 17:55:00Z wjbohnh $";


namespace Dakota {

SOLBase*   SOLBase::solInstance(NULL);
Minimizer* SOLBase::optLSqInstance(NULL);

size_t SOLBase::numInstances = 0;

SOLBase::SOLBase(std::shared_ptr<Model> model):
  boundsArraySize(0), linConstraintMatrixF77(NULL),
  upperFactorHessianF77(NULL), constraintJacMatrixF77(NULL)
{
  // Use constructor to populate only the problem_db attributes inherited by 
  // NPSOL/NLSSOL from SOLBase (none currently).  For attributes inherited by
  // NPSOL/NLSSOL from the Iterator hierarchy that are needed in SOLBase,
  // it's a bit cleaner/more flexible to have them passed through member
  // function parameter lists rather than re-extracted from problem_db.
  //const ProblemDescDB& problem_db = model.problem_description_db();
  
  numInstances++;
}


void SOLBase::
check_sub_iterator_conflict(Model& model, unsigned short method_name)
{
  // Prevent nesting of an instance of a Fortran iterator within another
  // instance of the same iterator (which would result in data clashes since
  // Fortran does not support object independence).  Recurse through all
  // sub-models and test each sub-iterator for NPSOL/NLSSOL presence.
  // Note: This check is performed for DOT, CONMIN, and SOLBase, but not
  //       for LHS (since it is only active in pre-processing) or SOL
  //       user-functions mode (since there is no model in this case).
  std::shared_ptr<Iterator> sub_iterator;
  sub_iterator = model.subordinate_iterator();
  if (sub_iterator && 
      ( sub_iterator->method_name() ==     NPSOL_SQP   ||
	sub_iterator->method_name() ==    NLSSOL_SQP   ||
	sub_iterator->uses_method() == SUBMETHOD_NPSOL ||
	sub_iterator->uses_method() == SUBMETHOD_NPSOL_OPTPP  ||
	sub_iterator->uses_method() == SUBMETHOD_DIRECT_NPSOL ||
	sub_iterator->uses_method() == SUBMETHOD_DIRECT_NPSOL_OPTPP ) )
    sub_iterator->method_recourse(method_name);
  ModelList& sub_models = model.subordinate_models();
  for (auto& sm : sub_models) {
    sub_iterator = sm->subordinate_iterator();
    if (sub_iterator && 
	 ( sub_iterator->method_name() ==     NPSOL_SQP   ||
	   sub_iterator->method_name() ==    NLSSOL_SQP   ||
	   sub_iterator->uses_method() == SUBMETHOD_NPSOL ||
	   sub_iterator->uses_method() == SUBMETHOD_NPSOL_OPTPP  ||
	   sub_iterator->uses_method() == SUBMETHOD_DIRECT_NPSOL ||
	   sub_iterator->uses_method() == SUBMETHOD_DIRECT_NPSOL_OPTPP ) )
      sub_iterator->method_recourse(method_name);
  }
}


void SOLBase::
allocate_linear_arrays(int num_cv, const RealMatrix& lin_ineq_coeffs,
		       const RealMatrix& lin_eq_coeffs)
{
  // NPSOL directly handles equality constraints and 1- or 2-sided inequalities
  size_t num_lin_ineq_con = lin_ineq_coeffs.numRows(),
         num_lin_eq_con   =   lin_eq_coeffs.numRows(),
         num_lin_con      = num_lin_ineq_con + num_lin_eq_con;

  // The Fortran optimizers' need for a nonzero array size is handled with 
  // nlnConstraintArraySize & linConstraintArraySize.
  linConstraintArraySize = (num_lin_con) ? num_lin_con : 1;

  // Matrix memory passed to Fortran must be contiguous
  if (linConstraintMatrixF77) delete [] linConstraintMatrixF77;
  linConstraintMatrixF77 = new double[linConstraintArraySize * num_cv];

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
}


void SOLBase::
allocate_nonlinear_arrays(int num_cv, size_t num_nln_con)
{
  // The Fortran optimizers' need for a nonzero array size is handled with 
  // nlnConstraintArraySize & linConstraintArraySize.
  nlnConstraintArraySize = (num_nln_con) ? num_nln_con : 1;

  // Matrix memory passed to Fortran must be contiguous
  if (constraintJacMatrixF77) delete [] constraintJacMatrixF77;
  constraintJacMatrixF77 = new double[nlnConstraintArraySize * num_cv];
}


void SOLBase::
allocate_arrays(int num_cv, size_t num_nln_con,
		const RealMatrix& lin_ineq_coeffs,
		const RealMatrix& lin_eq_coeffs)
{
  allocate_linear_arrays(num_cv, lin_ineq_coeffs, lin_eq_coeffs);
  allocate_nonlinear_arrays(num_cv, num_nln_con);

  // Matrix memory passed to Fortran must be contiguous
  if (upperFactorHessianF77)  delete [] upperFactorHessianF77;
  upperFactorHessianF77  = new double[num_cv * num_cv];

  size_bounds_array(num_cv + lin_ineq_coeffs.numRows() +
		    lin_eq_coeffs.numRows() + num_nln_con);
}


void SOLBase::deallocate_arrays()
{
  // Delete double* matrix allocations
  delete [] linConstraintMatrixF77;  linConstraintMatrixF77 = NULL;
  delete [] upperFactorHessianF77;   upperFactorHessianF77  = NULL;
  delete [] constraintJacMatrixF77;  constraintJacMatrixF77 = NULL;
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
			  Real linesrch_tol, size_t max_iter, Real constr_tol,
                          Real conv_tol, const std::string& grad_type,
                          const RealVector& fdss)
{
  // Set NPSOL options (see "Optional Input Parameters" section of NPSOL manual)

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
  verify_s += std::to_string(verify_lev);
  send_sol_option(verify_s);

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
  send_sol_option(fnprec_s);

  std::ostringstream lstol_stream;
  lstol_stream << "Linesearch Tolerance        = "
               << std::setiosflags(std::ios::left)
               << std::setw(26) << linesrch_tol;
  std::string lstol_s( lstol_stream.str() );
  send_sol_option(lstol_s);

  std::string maxiter_s("Major Iteration Limit       = ");
  maxiter_s += std::to_string(max_iter);
  send_sol_option(maxiter_s);

  if (output_lev > NORMAL_OUTPUT) {
    std::string plevel_s("Major Print Level           = 20");
    send_sol_option(plevel_s);
    Cout << "\nNPSOL option settings:\n----------------------\n" 
         << verify_s << '\n' << "Major Print Level           = 20\n" 
         << fnprec_s << '\n' << lstol_s << '\n'
	 << maxiter_s << '\n';
  }
  else {
    std::string plevel_s("Major Print Level           = 10");
    send_sol_option(plevel_s);
  }

  // assign a nondefault linear/nonlinear constraint tolerance if a valid
  // value has been set in dakota.in; otherwise utilize the NPSOL default.
  if (constr_tol > 0.0) {
    std::ostringstream ct_tol_stream;
    ct_tol_stream << "Feasibility Tolerance       = "
                  << std::setiosflags(std::ios::left)
                  << std::setw(26) << constr_tol;
    std::string ct_tol_s( ct_tol_stream.str() );
    send_sol_option(ct_tol_s);
    if (output_lev > NORMAL_OUTPUT)
      Cout << ct_tol_s << '\n';
  }

  // conv_tol is an optional parameter in dakota.input.nspec, but
  // defining our own default (in the DataMethod constructor) and
  // always assigning it applies some consistency across methods.
  // Therefore, the NPSOL default is not used.
  std::ostringstream ctol_stream;
  ctol_stream << "Optimality Tolerance        = "
              << std::setiosflags(std::ios::left) << std::setw(26) << conv_tol;
  std::string ctol_s( ctol_stream.str() );
  send_sol_option(ctol_s);
  if (output_lev > NORMAL_OUTPUT)
    Cout << ctol_s << "\nNOTE: NPSOL's convergence tolerance is not a "
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
    send_sol_option(dlevel_s);
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
    send_sol_option(dlevel_s);

    std::ostringstream fdss_stream;
    Real fd_step_size = fdss[0]; // first entry
    fdss_stream << "Difference Interval         = "
                << std::setiosflags(std::ios::left) << std::setw(26)
		<< fd_step_size;
    std::string fdss_s( fdss_stream.str() );
    send_sol_option(fdss_s);
  
    // Set "Central Difference Interval" to fdss as well.
    // It may be desirable to set central FDSS to fdss/2. (?)
    std::ostringstream cfdss_stream;
    cfdss_stream << "Central Difference Interval = "
                 << std::setiosflags(std::ios::left) << std::setw(26)
		 << fd_step_size;
    std::string cfdss_s( cfdss_stream.str() );
    send_sol_option(cfdss_s);
  
    if (output_lev > NORMAL_OUTPUT)
      Cout << "Derivative Level            = 0\n" << fdss_s << '\n'
	   << cfdss_s << "\nNOTE: NPSOL's finite difference interval "
	   << "uses a unit offset to remove the\n      need for a minimum step "
	   << "specification (see \"Difference interval\" in\n      Optional "
           << "Input Parameters section of NPSOL manual).\n"
	   << "Interval type ignored since NPSOL automatically selects\n"
	   << "and switches between forward and central differences.\n\n";
  }
}


void SOLBase::
aggregate_bounds(const RealVector& cv_l_bnds, const RealVector& cv_u_bnds,
		 const RealVector& lin_ineq_l_bnds,
		 const RealVector& lin_ineq_u_bnds,
		 const RealVector& lin_eq_targets,
		 const RealVector& nln_ineq_l_bnds,
		 const RealVector& nln_ineq_u_bnds,
		 const RealVector& nln_eq_targets,
		 RealVector& aggregate_l_bnds, RealVector& aggregate_u_bnds)
{
  // Construct aggregate_l_bnds & aggregate_u_bnds from variable bounds,
  // linear inequality bounds and equality targets, and nonlinear inequality
  // bounds and equality targets.

  size_t num_cv       = cv_l_bnds.length(),
         num_lin_ineq = lin_ineq_l_bnds.length(),
         num_lin_eq   = lin_eq_targets.length(),
         num_nln_ineq = nln_ineq_l_bnds.length(),
         num_nln_eq   = nln_eq_targets.length(),
         bnds_size    = num_cv + num_lin_ineq + num_lin_eq
                      + num_nln_ineq + num_nln_eq;
  if (aggregate_l_bnds.length() != bnds_size ||
      aggregate_u_bnds.length() != bnds_size) {
    aggregate_l_bnds.sizeUninitialized(bnds_size);
    aggregate_u_bnds.sizeUninitialized(bnds_size);
  }
  size_t cntr = 0;
  copy_data_partial(cv_l_bnds,       0, aggregate_l_bnds, cntr, num_cv );
  copy_data_partial(cv_u_bnds,       0, aggregate_u_bnds, cntr, num_cv );
  cntr += num_cv;
  copy_data_partial(lin_ineq_l_bnds, 0, aggregate_l_bnds, cntr, num_lin_ineq );
  copy_data_partial(lin_ineq_u_bnds, 0, aggregate_u_bnds, cntr, num_lin_ineq );
  cntr += num_lin_ineq;
  copy_data_partial(lin_eq_targets,  0, aggregate_l_bnds, cntr, num_lin_eq );
  copy_data_partial(lin_eq_targets,  0, aggregate_u_bnds, cntr, num_lin_eq );
  cntr += num_lin_eq;
  copy_data_partial(nln_ineq_l_bnds, 0, aggregate_l_bnds, cntr, num_nln_ineq );
  copy_data_partial(nln_ineq_u_bnds, 0, aggregate_u_bnds, cntr, num_nln_ineq );
  cntr += num_nln_ineq;
  copy_data_partial(nln_eq_targets,  0, aggregate_l_bnds, cntr, num_nln_eq );
  copy_data_partial(nln_eq_targets,  0, aggregate_u_bnds, cntr, num_nln_eq );
}


/*
void SOLBase::
augment_bounds(RealVector& aggregate_l_bnds, RealVector& aggregate_u_bnds,
	       const RealVector& lin_ineq_l_bnds,
	       const RealVector& lin_ineq_u_bnds,
	       const RealVector& lin_eq_targets,
	       const RealVector& nln_ineq_l_bnds,
	       const RealVector& nln_ineq_u_bnds,
	       const RealVector& nln_eq_targets)
{
  // Construct aggregate_l_bnds & aggregate_u_bnds from variable bounds,
  // linear inequality bounds and equality targets, and nonlinear inequality
  // bounds and equality targets.  Arrays passed in are assumed to already 
  // contain the variable bounds and are aggregated with linear and nonlinear
  // constraint bounds.  Note: bounds above or below NPSOL's "Infinite bound
  // size" (see bl/bu in "Subroutine npsol" section and Infinite bound size in
  // "Optional Input Parameters" section of NPSOL manual) are ignored. 
  // DAKOTA's default bounds for no user specification are set in
  // ProblemDescDB::responses_kwhandler for nonlinear constraints and in
  // Constraints::manage_linear_constraints for linear constraints.

  size_t num_cv       = aggregate_l_bnds.length(),
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

  aggregate_l_bnds.resize(boundsArraySize); // retains variables data
  aggregate_u_bnds.resize(boundsArraySize); // retains variables data
  size_t cntr = num_cv;
  copy_data_partial(lin_ineq_l_bnds, 0, aggregate_l_bnds, cntr, num_lin_ineq );
  copy_data_partial(lin_ineq_u_bnds, 0, aggregate_u_bnds, cntr, num_lin_ineq );
  cntr += num_lin_ineq;
  copy_data_partial(lin_eq_targets, 0, aggregate_l_bnds, cntr, num_lin_eq );
  copy_data_partial(lin_eq_targets, 0, aggregate_u_bnds, cntr, num_lin_eq );
  cntr += num_lin_eq;
  copy_data_partial(nln_ineq_l_bnds, 0, aggregate_l_bnds, cntr, num_nln_ineq );
  copy_data_partial(nln_ineq_u_bnds, 0, aggregate_u_bnds, cntr, num_nln_ineq );
  cntr += num_nln_ineq;
  copy_data_partial(nln_eq_targets, 0, aggregate_l_bnds, cntr, num_nln_eq );
  copy_data_partial(nln_eq_targets, 0, aggregate_u_bnds, cntr, num_nln_eq );
}


void SOLBase::
replace_variable_bounds(size_t num_lin_con, size_t num_nln_con,
			RealVector& aggregate_l_bnds,
			RealVector& aggregate_u_bnds,
			const RealVector& cv_lower_bnds,
			const RealVector& cv_upper_bnds)
{
  size_t num_cv = cv_lower_bnds.length(), num_con = num_lin_con + num_nln_con,
    old_cv, old_bnds_size = aggregate_l_bnds.length(),
    new_bnds_size = num_cv + num_con;

  size_bounds_array(new_bnds_size);
  if (old_bnds_size != new_bnds_size) {
    RealVector old_l_bnds(aggregate_l_bnds), old_u_bnds(aggregate_u_bnds);
    aggregate_l_bnds.resize(new_bnds_size);
    aggregate_u_bnds.resize(new_bnds_size);
    // migrate linear/nonlinear bnds/targets:
    old_cv = old_bnds_size - num_con;
    copy_data_partial(old_l_bnds, old_cv, aggregate_l_bnds, num_cv, num_con);
    copy_data_partial(old_u_bnds, old_cv, aggregate_u_bnds, num_cv, num_con);
  }
  // assign new variable bnds:
  copy_data_partial(cv_lower_bnds, 0, aggregate_l_bnds, 0, num_cv);
  copy_data_partial(cv_upper_bnds, 0, aggregate_u_bnds, 0, num_cv);
}


void SOLBase::
replace_linear_bounds(size_t num_cv, size_t num_nln_con,
		      RealVector& aggregate_l_bnds,
		      RealVector& aggregate_u_bnds,
		      const RealVector& lin_ineq_l_bnds,
		      const RealVector& lin_ineq_u_bnds,
		      const RealVector& lin_eq_targets)
{
  size_t num_lin_ineq  = lin_ineq_l_bnds.length(),
         num_lin_eq    = lin_eq_targets.length(),
         num_lin_con   = num_lin_ineq + num_lin_eq, new_offset, old_offset,
         old_bnds_size = aggregate_l_bnds.length(),
         new_bnds_size = num_cv + num_lin_con + num_nln_con;

  size_bounds_array(new_bnds_size);
  if (old_bnds_size != new_bnds_size) {
    RealVector old_l_bnds(aggregate_l_bnds), old_u_bnds(aggregate_u_bnds);
    aggregate_l_bnds.resize(new_bnds_size); // retains variables data
    aggregate_u_bnds.resize(new_bnds_size); // retains variables data
    // migrate nonlinear bnds/targets:
    old_offset = old_bnds_size - num_nln_con;
    new_offset = num_cv + num_lin_con;
    copy_data_partial(old_l_bnds, old_offset, aggregate_l_bnds,
		      new_offset, num_nln_con);
    copy_data_partial(old_u_bnds, old_offset, aggregate_u_bnds,
		      new_offset, num_nln_con);
  }
  // assign new linear bnds/targets:
  new_offset = num_cv;
  copy_data_partial(lin_ineq_l_bnds, 0, aggregate_l_bnds,
		    new_offset, num_lin_ineq);
  copy_data_partial(lin_ineq_u_bnds, 0, aggregate_u_bnds,
		    new_offset, num_lin_ineq);
  new_offset += num_lin_ineq;
  copy_data_partial(lin_eq_targets, 0, aggregate_l_bnds,
		    new_offset, num_lin_eq);
  copy_data_partial(lin_eq_targets, 0, aggregate_u_bnds,
		    new_offset, num_lin_eq);
}


void SOLBase::
replace_nonlinear_bounds(size_t num_cv, size_t num_lin_con,
			 RealVector& aggregate_l_bnds,
			 RealVector& aggregate_u_bnds,
			 const RealVector& nln_ineq_l_bnds,
			 const RealVector& nln_ineq_u_bnds,
			 const RealVector& nln_eq_targets)
{
  size_t num_nln_ineq  = nln_ineq_l_bnds.length(),
         num_nln_eq    = nln_eq_targets.length(),
         num_nln_con   = num_nln_ineq + num_nln_eq, offset,
         old_bnds_size = aggregate_l_bnds.length(),
         new_bnds_size = num_cv + num_lin_con + num_nln_con;

  size_bounds_array(new_bnds_size);
  if (old_bnds_size != new_bnds_size) {
    aggregate_l_bnds.resize(new_bnds_size); // retains vars, lin cons data
    aggregate_u_bnds.resize(new_bnds_size); // retains vars, lin cons data
  }
  // assign new nonlinear bnds/targets:
  offset = num_cv + num_lin_con;
  copy_data_partial(nln_ineq_l_bnds, 0, aggregate_l_bnds, offset, num_nln_ineq);
  copy_data_partial(nln_ineq_u_bnds, 0, aggregate_u_bnds, offset, num_nln_ineq);
  offset += num_nln_ineq;
  copy_data_partial(nln_eq_targets,  0, aggregate_l_bnds, offset, num_nln_eq);
  copy_data_partial(nln_eq_targets,  0, aggregate_u_bnds, offset, num_nln_eq);
}
*/


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
    ModelUtils::continuous_variables(*optLSqInstance->iteratedModel, local_des_vars);

    optLSqInstance->activeSet.request_vector(local_asv);
    optLSqInstance->iteratedModel->evaluate(optLSqInstance->activeSet);
    solInstance->fnEvalCntr++;
  }
  
  // Follow asv_request request (special case exceptions handled above).
  // Could follow local_asv entry-by-entry, but the design below is 
  // easier since the Response structure matches that needed by SOL.
  const Response& local_response
    = optLSqInstance->iteratedModel->current_response();
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
