/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SNLLBase
//- Description: Implementation code for the SNLLBase class
//- Owner:       Pam Williams
//- Checked by:
//- Change Log:  

#include "SNLLBase.H"
#include "DakotaModel.H"
#include "DakotaMinimizer.H"
#include "ProblemDescDB.H"
#include "OptNewtonLike.h"
#include "NLF.h"
#include "NLP.h"
#include "CompoundConstraint.h"
#include "Constraint.h"
#include "BoundConstraint.h"
#include "LinearInequality.h"
#include "LinearEquation.h"
#include "NonLinearEquation.h"
#include "NonLinearInequality.h"
#include "OptppArray.h"
#include "data_util.h"

using namespace OPTPP;

static const char rcsId[]="@(#) $Id: SNLLBase.C 7029 2010-10-22 00:17:02Z mseldre $";

namespace Dakota {

Minimizer*      SNLLBase::optLSqInstance(NULL);
bool            SNLLBase::modeOverrideFlag(false);
EvalType        SNLLBase::lastFnEvalLocn(NLFEvaluator);
int             SNLLBase::lastEvalMode(1);
RealVector SNLLBase::lastEvalVars;


SNLLBase::SNLLBase(Model& model)
{
  // Use constructor only to populate problem_db attributes inherited by 
  // SNLLOptimizer/SNLLLeastSq from SNLLBase.  For attributes inherited by
  // SNLLOptimizer/SNLLLeastSq from the Iterator hierarchy that are
  // needed in SNLLBase, it's a bit cleaner/more flexible to have them passed
  // through member function parameter lists rather than re-extracted from
  // problem_db.
  const ProblemDescDB& problem_db = model.problem_description_db();
  searchMethod    =  problem_db.get_string("method.optpp.search_method");
  constantASVFlag = !problem_db.get_bool("interface.active_set_vector");
}


void SNLLBase::
snll_pre_instantiate(const String& merit_fn, bool bound_constr_flag,
		     const int& num_constr)
{
  // OPT++ options are set here, in post_instantiate() below, and in the
  // SNLLOptimizer/SNLLLeastSq constructors to circumvent the "opt.input" file.

  if (merit_fn == "el_bakry")
    meritFn = NormFmu;
  else if (merit_fn == "argaez_tapia")
    meritFn = ArgaezTapia;
  else
    meritFn = VanShanno;

  // Constraints are not handled properly with the trust region search strategy.
  // Therefore, make LineSearch the default for bound-constrained optimizers and
  // TrustRegion the default for unconstrained optimizers.  Note that
  // setSearchStrategy is not supported/called for CG, PDS, or NIPS methods, so
  // the setting is not important for these methods.
  if (searchMethod == "value_based_line_search" || 
      searchMethod == "gradient_based_line_search")
    searchStrat = LineSearch; // value-/gradient-based set by setIsExpensive
  else if (searchMethod == "tr_pds") {
    if (bound_constr_flag || num_constr) {
      Cerr << "Warning: tr_pds is only available for unconstrained problems.\n"
           << "         search_method will be set to trust_region." 
	   << std::endl;
      searchStrat = TrustRegion;
    }
    else
      searchStrat = TrustPDS;
  }
  else if (searchMethod.empty() && bound_constr_flag)
    searchStrat = LineSearch; // default for BC optimizers
  else if (!searchMethod.empty() && num_constr)
    Cerr << "\nWarning: nonlinear interior-point optimizers do not support a "
         << "search_method specification.\n\n";
    //searchStrat = LineSearch; // default for NIPS optimizers
  else
    searchStrat = TrustRegion; // all other cases, including unconstrained opt.
}


void SNLLBase::
snll_post_instantiate(const int& num_cv, bool vendor_num_grad_flag,
		      const String& finite_diff_type, const Real& fdss,
		      const int& max_iter, const int& max_fn_evals,
		      const Real& conv_tol, const Real& grad_tol,
		      const Real& max_step, bool bound_constr_flag,
		      const int& num_constr, short output_lev,
		      OPTPP::OptimizeClass* the_optimizer, 
		      OPTPP::NLP0* nlf_objective,
		      OPTPP::FDNLF1* fd_nlf1, OPTPP::FDNLF1* fd_nlf1_con)
{
  // These options belong to the abstract base classes NLP0 and OptimizeClass
  // and are set after the pointer assignments. See bottom of
  // libopt/optnewtonlike.C in the OPT++ repository for options.

  // fcn_acc = fdss*fdss (forward) || fdss*fdss*fdss (central)
  if ( vendor_num_grad_flag ) {
    // OPT++'s internal finite differencing in use.
    Real fcn_acc, mcheps = NEWMAT::FloatingPointPrecision::Epsilon();
    if (finite_diff_type == "central") {
      fd_nlf1->setDerivOption(CentralDiff); // See libopt/globals.h for enum
      if (num_constr)
        fd_nlf1_con->setDerivOption(CentralDiff);
      fcn_acc = std::pow(fdss, 3);
    }
    else {
      fd_nlf1->setDerivOption(ForwardDiff); // See libopt/globals.h for enum
      if (num_constr)
        fd_nlf1_con->setDerivOption(ForwardDiff);
      fcn_acc = std::pow(fdss, 2);
    }
    fcn_acc = std::max(mcheps,fcn_acc);
    NEWMAT::ColumnVector fcn_accrcy(num_cv);
    fcn_accrcy = fcn_acc;
    fd_nlf1->setFcnAccrcy(fcn_accrcy);
    if (num_constr)
      fd_nlf1_con->setFcnAccrcy(fcn_accrcy);
  }

  // LineSearch strategy is value-based if expensive, gradient-based if not.
  // Default strategy for BC & NIPS optimizers is value_based_line_search.
  if ( searchMethod == "value_based_line_search" ||
       ( searchMethod.empty() && ( bound_constr_flag || num_constr ) ) )
    nlf_objective->setIsExpensive(1);

  // conv. tolerances for change in fn. value and magnitude of grad. norm
  the_optimizer->setFcnTol(conv_tol);  
  the_optimizer->setGradTol(grad_tol); 

  the_optimizer->setMaxStep(max_step);
  the_optimizer->setMaxFeval(max_fn_evals);
  the_optimizer->setMaxIter(max_iter);

  if (output_lev == DEBUG_OUTPUT)
    the_optimizer->setDebug();

  // InitFcn() calls init_fn, which is currently empty, so remove the
  // function call overhead by commenting it out for now.
  //nlf_objective->InitFcn();
}


void SNLLBase::init_fn(int n, NEWMAT::ColumnVector& x)
{
  // This routine was previously called initial_guess which was misleading.
  // This is a mechanism provided by OPT++ to perform initialization functions.
  // A pointer to this function is passed in the parameter list of the NLF1,
  // FDNLF1, and NLF2 constructors.  DAKOTA performs initialization for OPT++
  // in the SNLLOptimizer constructor, so this routine is currently empty.
  // Moen, Plantenga, et al. have used this routine for warm analysis restart
  // by performing an initial analysis here whose restart file will be used for
  // all subsequent analyses, in order to improve efficiency.
}


void SNLLBase::
snll_initialize_run(OPTPP::NLP0* nlf_objective, OPTPP::NLP* nlp_constraint,
		    const RealVector& init_pt, bool bound_constr_flag,
		    const RealVector& lower_bounds,
		    const RealVector& upper_bounds,
		    const RealMatrix& lin_ineq_coeffs,
		    const RealVector& lin_ineq_l_bnds,
		    const RealVector& lin_ineq_u_bnds,
		    const RealMatrix& lin_eq_coeffs,
		    const RealVector& lin_eq_targets, 
		    const RealVector& nln_ineq_l_bnds,
		    const RealVector& nln_ineq_u_bnds,
		    const RealVector& nln_eq_targets)
{
  // While it is not necessary to pass all of this data through the parameter
  // list (it could be accessed with optLSqInstance), it allows different 
  // behavior between the find optimum on model (with late updating from the
  // model) and find optimum on user-functions (no late updating) modes.

  // create a ColumnVector x, copy current model variables to it, and then setX
  // within opt++.  This occurs within the context of the run function so that
  // any variable reassignment at the strategy layer (after iterator
  // construction) is captured with setX.
  NEWMAT::ColumnVector x;
  copy_data(init_pt, x);
  nlf_objective->setX(x);  // setX accepts a ColumnVector
  size_t num_cv = init_pt.length();

  // Instantiate bound, linear, and nonlinear constraints and append them to
  // constraint_array.
  OptppArray<Constraint> constraint_array(0);

  // Initialize bound constraints.
  // get current model bounds, copy them to ColumnVectors, and then set them
  // for bound constrained opt++ methods.  This was moved from the constructor
  // so that any bounds modifications at the strategy layer (e.g., 
  // BranchBndStrategy, SurrBasedOptStrategy) are properly captured.
  if (bound_constr_flag) {
    NEWMAT::ColumnVector bc_lower, bc_upper;
    copy_data(lower_bounds, bc_lower);
    copy_data(upper_bounds, bc_upper);

    Constraint bc = new BoundConstraint(num_cv, bc_lower, bc_upper);
    constraint_array.append(bc);
  }

  size_t num_lin_ineq_con = lin_ineq_l_bnds.length(),
         num_lin_eq_con   = lin_eq_targets.length(),
         num_lin_con      = num_lin_ineq_con + num_lin_eq_con,
         num_nln_ineq_con = nln_ineq_l_bnds.length(),
         num_nln_eq_con   = nln_eq_targets.length(),
         num_nln_con      = num_nln_ineq_con + num_nln_eq_con;

  // Initialize linear inequalities 
  // Assumes that numLinearConstraints = linear_ineqs + linear_eqs
  if (num_lin_con) {

    if (num_lin_ineq_con){
      NEWMAT::Matrix A_i;
      copy_data(lin_ineq_coeffs, A_i);
      NEWMAT::ColumnVector b_l, b_u;
      copy_data(lin_ineq_l_bnds, b_l);
      copy_data(lin_ineq_u_bnds, b_u);

      Constraint li = new LinearInequality(A_i, b_l, b_u);
      constraint_array.append(li);
    }

    if (num_lin_eq_con) {
      NEWMAT::Matrix A_e;
      copy_data(lin_eq_coeffs,  A_e);
      NEWMAT::ColumnVector b_e;
      copy_data(lin_eq_targets, b_e);

      Constraint le = new LinearEquation(A_e, b_e);
      constraint_array.append(le);
    }
  }

  if (num_nln_con) {

    NEWMAT::ColumnVector augmented_lower_bnds(num_nln_con);
    NEWMAT::ColumnVector augmented_upper_bnds(num_nln_con);

    // Unlike Dakota, opt++ expects nonlinear equality constraints
    // followed by nonlinear inequality constraints.
    int i;
    if (num_nln_eq_con) {
      //NEWMAT::ColumnVector ne_targets(num_nln_eq_con);
      for (i=1; i<=num_nln_eq_con; i++) {
	augmented_lower_bnds(i) = nln_eq_targets[i-1];
	augmented_upper_bnds(i) = nln_eq_targets[i-1];
      }
    }
    if (num_nln_ineq_con) {
      //NEWMAT::ColumnVector ni_lower_bnds, ni_upper_bnds;
      for (i=1; i<=num_nln_ineq_con; i++) {
	augmented_lower_bnds(i+num_nln_eq_con) = nln_ineq_l_bnds[i-1];
	augmented_upper_bnds(i+num_nln_eq_con) = nln_ineq_u_bnds[i-1];
      }
    }
    Constraint nc = new NonLinearConstraint(nlp_constraint,
                                            augmented_lower_bnds,
                                            augmented_upper_bnds,
                                            num_nln_eq_con, num_nln_ineq_con);
    constraint_array.append(nc);
  }

  CompoundConstraint* cc = new CompoundConstraint(constraint_array);
  nlf_objective->setConstraints(cc);

  // R. Lee/C. Moen: This initial Eval is required in order for bound 
  // constrained and barrier methods to work properly.
  // MSE, 7/23/97: My testing shows no difference with or without this initial 
  // eval, so comment out for now.
  //nlf_objective->eval();
}


void SNLLBase::snll_post_run(OPTPP::NLP0* nlf_objective)
{
  // the best response update is specialized in the derived classes,
  // but the best variables update is not

  RealVector local_des_vars;
  copy_data(nlf_objective->getXc(), local_des_vars); // see opt++/libopt/nlp.h
  optLSqInstance->
    bestVariablesArray.front().continuous_variables(local_des_vars);

  // TO DO: Deallocate local memory allocations (is this needed w/ SmartPtr?).
  //if (bc) delete bc;
  //if (li) delete li;
  //if (le) delete le;
  //if (ni) delete ni;
  //if (ne) delete ne;
  //delete cc;
}


void SNLLBase::
copy_con_vals(const RealVector& local_fn_vals, NEWMAT::ColumnVector& g,
              const size_t& offset)
{
  // Unlike DAKOTA, OPT++ expects nonlinear equations followed by nonlinear
  // inequalities.  Therefore, we have to reorder the constraint values.
  size_t i, num_nln_eq_con = optLSqInstance->numNonlinearEqConstraints,
    num_nln_ineq_con = optLSqInstance->numNonlinearIneqConstraints;
  for (i=0; i<num_nln_eq_con; i++)
    g(i+1) = local_fn_vals[offset+num_nln_ineq_con+i];
  for (i=0; i<num_nln_ineq_con; i++)
    g(i+num_nln_eq_con+1) = local_fn_vals[offset+i];
}


void SNLLBase::
copy_con_vals(const NEWMAT::ColumnVector& g, RealVector& local_fn_vals,
	      const size_t& offset)
{
  // Unlike DAKOTA, OPT++ expects nonlinear equations followed by nonlinear
  // inequalities.  Therefore, we have to reorder the constraint values.
  size_t i, num_nln_eq_con = optLSqInstance->numNonlinearEqConstraints,
    num_nln_ineq_con = optLSqInstance->numNonlinearIneqConstraints;
  for (i=0; i<num_nln_ineq_con; i++)
    local_fn_vals[offset+i] = g(i+num_nln_eq_con+1);
  for (i=0; i<num_nln_eq_con; i++)
    local_fn_vals[offset+num_nln_ineq_con+i] = g(i+1);
}


void SNLLBase::
copy_con_grad(const RealMatrix& local_fn_grads, NEWMAT::Matrix& grad_g,
              const size_t& offset)
{
  // Unlike DAKOTA, OPT++ expects nonlinear equations followed by nonlinear
  // inequalities.  Therefore, we have to reorder the constraint gradients.

  // Assign the gradients of the nonlinear constraints.  Let g: R^n -> R^m.
  // The gradient of g, grad_g, is the n x m matrix whose ith column is the
  // gradient of g_i.  The transpose of grad_g is called the Jacobian of g.
  // WJB: candidate for more efficient rewrite?? (both are ColumnMajor grads!)
  size_t i, j, n = local_fn_grads.numRows(),
    num_nln_eq_con   = optLSqInstance->numNonlinearEqConstraints,
    num_nln_ineq_con = optLSqInstance->numNonlinearIneqConstraints;
  for (i=0; i<n; i++)
    for (j=0; j<num_nln_eq_con; j++)
      grad_g(i+1, j+1) = local_fn_grads(i,offset+num_nln_ineq_con+j);
  for (i=0; i<n; i++)
    for (j=0; j<num_nln_ineq_con; j++)
      grad_g(i+1, j+num_nln_eq_con+1) = local_fn_grads(i,offset+j);
}


void SNLLBase::
copy_con_hess(const RealSymMatrixArray& local_fn_hessians,
              OPTPP::OptppArray<NEWMAT::SymmetricMatrix>& hess_g,
	      const size_t& offset)
{
  // Unlike DAKOTA, OPT++ expects nonlinear equations followed by nonlinear
  // inequalities.  Therefore, we have to reorder the constraint Hessians.
  size_t i, num_nln_eq_con = optLSqInstance->numNonlinearEqConstraints,
    num_nln_ineq_con = optLSqInstance->numNonlinearIneqConstraints;
  for (i=0; i<num_nln_eq_con; i++)
    copy_data(local_fn_hessians[offset+num_nln_ineq_con+i], hess_g[i]);
  for (i=0; i<num_nln_ineq_con; i++)
    copy_data(local_fn_hessians[offset+i], hess_g[num_nln_eq_con+i]);
}

} // namespace Dakota
