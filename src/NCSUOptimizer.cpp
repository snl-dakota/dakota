/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NCSUOptimizer
//- Description: Implementation code for the NCSUOptimizer class
//- Owner:       Barron J Bichon, Vanderbilt University

#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NCSUOptimizer.hpp"
#include "ProblemDescDB.hpp"

#define NCSU_DIRECT_F77 F77_FUNC_(ncsuopt_direct,NCSUOPT_DIRECT)

extern "C" {

void NCSU_DIRECT_F77(int (*objfun)(int *n, double c[], double l[], double u[],
				   int point[], int *maxI, int *start,
				   int *maxfunc, double fvec[], int iidata[],
				   int *iisize, double ddata[], int *idsize,
				   char cdata[], int *icsize),
		     double* x, int& n, double& eps, int& maxf, int& maxT,
		     double& fmin, double* l, double* u, int& algmethod,
		     int& ierror, int& logfile, double& fglobal, double& fglper,
		     double& volper, double& sigmaper, int* idata, int& isize, 
		     double* ddata, int& dsize, char* cdata, int& csize,
		     int& quiet_flag);

}

namespace Dakota {

// define values for setup type (DAKOTA Model or user-supplied function)
enum { SETUP_MODEL, SETUP_USERFUNC };

// define array limits hard-wired in DIRECT
// maxdim (same as maxor)
#define NCSU_DIRECT_MAXDIM 64
// maxfunc = 90000-20
#define NCSU_DIRECT_MAXFUNC 89980

NCSUOptimizer* NCSUOptimizer::ncsudirectInstance(NULL);


/** This is the standard constructor with method specification support. */ 
NCSUOptimizer::NCSUOptimizer(ProblemDescDB& problem_db, Model& model):
  Optimizer(problem_db, model), setUpType(SETUP_MODEL),
  minBoxSize(probDescDB.get_real("method.min_boxsize_limit")), 
  volBoxSize(probDescDB.get_real("method.volume_boxsize_limit")),
  solutionTarget(probDescDB.get_real("method.solution_target")),
  userObjectiveEval(NULL)
{ 
  initialize(); 
  check_inputs();
}


/** This is an alternate constructor for instantiations on the fly
    using a Model but no ProblemDescDB. */
NCSUOptimizer::
NCSUOptimizer(Model& model, const int& max_iter, const int& max_eval,
	      double min_box_size, double vol_box_size, double solution_target):
  Optimizer(NCSU_DIRECT, model), setUpType(SETUP_MODEL),
  minBoxSize(min_box_size), volBoxSize(vol_box_size),
  solutionTarget(solution_target), userObjectiveEval(NULL)
{ 
  maxIterations = max_iter; maxFunctionEvals = max_eval;
  initialize(); 
  check_inputs();
}


/** This is an alternate constructor for Iterator instantiations by name
    using a Model but no ProblemDescDB. */
NCSUOptimizer::NCSUOptimizer(Model& model):
  Optimizer(NCSU_DIRECT, model), setUpType(SETUP_MODEL), minBoxSize(-1.),
  volBoxSize(-1.), solutionTarget(-DBL_MAX), userObjectiveEval(NULL)
{ 
  initialize(); 
  check_inputs();
}


/** This is an alternate constructor for performing an optimization using
    the passed in objective function pointer. */
NCSUOptimizer::
NCSUOptimizer(const RealVector& var_l_bnds,
	      const RealVector& var_u_bnds, const int& max_iter,
	      const int& max_eval,
	      double (*user_obj_eval) (const RealVector &x),
	      double min_box_size, double vol_box_size, double solution_target):
  Optimizer(NCSU_DIRECT, var_l_bnds.length(), 0, 0, 0, 0, 0, 0, 0),
  setUpType(SETUP_USERFUNC), minBoxSize(min_box_size), volBoxSize(vol_box_size),
  solutionTarget(solution_target), lowerBounds(var_l_bnds), 
  upperBounds(var_u_bnds), userObjectiveEval(user_obj_eval)
{ 
  maxIterations = max_iter; maxFunctionEvals = max_eval; 
  check_inputs();
}


void NCSUOptimizer::initialize()
{
  // Prevent nesting of an instance of a Fortran iterator within another
  // instance of the same iterator (which would result in data clashes since
  // Fortran does not support object independence).  Recurse through all
  // sub-models and test each sub-iterator for NCSU presence.
  Iterator sub_iterator = iteratedModel.subordinate_iterator();
  if (!sub_iterator.is_null() && 
       ( sub_iterator.method_name() == NCSU_DIRECT  ||
	 sub_iterator.uses_method() == NCSU_DIRECT ) )
    sub_iterator.method_recourse();
  ModelList& sub_models = iteratedModel.subordinate_models();
  for (ModelLIter ml_iter = sub_models.begin();
       ml_iter != sub_models.end(); ml_iter++) {
    sub_iterator = ml_iter->subordinate_iterator();
    if (!sub_iterator.is_null() && 
	 ( sub_iterator.method_name() == NCSU_DIRECT  ||
	   sub_iterator.uses_method() == NCSU_DIRECT ) )
      sub_iterator.method_recourse();
  }
}


void NCSUOptimizer::check_inputs()
{
  // Check limits against DIRECT hard-wired parameters. Note that
  // maxIterations could be a problem with constants maxdeep and
  // maxdiv, but there's no way to know a priori.
  if (numContinuousVars > NCSU_DIRECT_MAXDIM || 
      maxFunctionEvals > NCSU_DIRECT_MAXFUNC) {
    if (numContinuousVars > NCSU_DIRECT_MAXDIM) 
      Cerr << "Error (NCSUOptimizer): " << numContinuousVars << " variables "
	   << "specified exceeds NCSU DIRECT limit\n                       of "
	   << NCSU_DIRECT_MAXDIM << " variables.\n";
    if (maxFunctionEvals > NCSU_DIRECT_MAXFUNC)
      Cerr << "Error (NCSUOptimizer): max function evaluations " 
	   << maxFunctionEvals << " specified exceeds\n                       "
	   << "NCSU DIRECT limit of " << NCSU_DIRECT_MAXFUNC << ".\n";
    Cerr << std::endl;
    abort_handler(-1);
  }
}


NCSUOptimizer::~NCSUOptimizer() 
{ }


/// Modified batch evaluator that accepts multiple points and returns
/// corresponding vector of functions in fvec.  Must be used with modified
/// DIRECT src (DIRbatch.f).
int NCSUOptimizer::
objective_eval(int *n, double c[], double l[], double u[], int point[],
	       int *maxI, int *start, int *maxfunc, double fvec[], int iidata[],
	       int *iisize, double ddata[], int *idsize, char cdata[],
	       int *icsize)
{
  // TODO: set to 1 if a constraint violated 
  int feasible = 0;  // 0: no violation of hidden constraints

  int cnt = *start-1; // starting index into fvec
  int nx  = *n;       // dimension of design vector x.
  
  // number of trial points to evaluate
  // if initial point, we have a single point to evaluate
  int np = (*start == 1) ? 1 : *maxI*2;

  // Any MOO/NLS recasting is responsible for setting the scalar min/max
  // sense within the recast.
  const BoolDeque& max_sense
    = ncsudirectInstance->iteratedModel.primary_response_fn_sense();
  bool max_flag = (!max_sense.empty() && max_sense[0]);

//PDH: double * to RealVector
//     Note that there's some re-scaling going on here because the
//     DIRECT internal function that does it is bypassed.
//     Also, c (and u and l) contains multiple points to be
//     evaluated.  NOMAD can also do this, though the wrapper doesn't
//     currently take advantage of it.  It should in future
//     iterations.

  // loop over trial points, lift internal DIRECT scaling (mimics
  // DIRinfcn in DIRsubrout.f), and either submit for asynch
  // evaluation or compute synchronously
  RealVector local_des_vars(nx, false);
  int  pos = *start-1; // only used for second eval and beyond
  for (int j=0; j<np; j++) {

    if (*start == 1)
      for (int i=0; i<nx; i++)
	local_des_vars[i] = (c[i]+u[i])*l[i];
    else {
      for (int i=0; i<nx; i++) {
	// c[pos+i*maxfunc] = c(pos,i) in Fortran.
	// we believe c should be sized maxfunc by nx
	double ci=c[pos+i*(*maxfunc)];
	local_des_vars[i] = (ci + u[i])*l[i];
      }
      pos = point[pos]-1;
    }

    // below, use default ASV (function values, no gradients or
    // hessians); we assume fvec is sized maxfunc by 2 with a column
    // for function values and a column for constraints

    if (ncsudirectInstance->setUpType == SETUP_MODEL) {

      ncsudirectInstance->iteratedModel.continuous_variables(local_des_vars);

      // request the evaluation in synchronous or asynchronous mode
      if (ncsudirectInstance->iteratedModel.asynch_flag())
	ncsudirectInstance->iteratedModel.evaluate_nowait();
      else {
	ncsudirectInstance->iteratedModel.evaluate();
	// record the response in the function vector
	Real fn_val = ncsudirectInstance->
	  iteratedModel.current_response().function_value(0);
	fvec[cnt+j] = (max_flag) ? -fn_val : fn_val;
	fvec[cnt+j+(*maxfunc)] = feasible;
      }

    }
    else {
      fvec[cnt+j] = ncsudirectInstance->userObjectiveEval(local_des_vars);
      fvec[cnt+j+(*maxfunc)] = feasible;
    }

  } // end evaluation loop over points

  // If using model and evaluations performed asynchronously, need to record
  // the results now, after blocking until evaluation completion 
  if (ncsudirectInstance->setUpType == SETUP_MODEL &&
      ncsudirectInstance->iteratedModel.asynch_flag()) { 
      
    // block and wait for the responses
    const IntResponseMap& response_map
      = ncsudirectInstance->iteratedModel.synchronize();
    // record the responses in the function vector
    IntRespMCIter r_cit = response_map.begin();
    for (int j=0; j<np; ++j, ++r_cit) {
      fvec[cnt+j] = (max_flag) ? -r_cit->second.function_value(0) :
	                          r_cit->second.function_value(0);
      fvec[cnt+j+(*maxfunc)] = feasible;
    }
  }

  return 0;
}


void NCSUOptimizer::core_run()
{
  //------------------------------------------------------------------
  //     Solve the problem.
  //------------------------------------------------------------------

  // set the object instance pointers for use within the static member fns
  NCSUOptimizer* prev_instance = ncsudirectInstance;
  ncsudirectInstance = this;

  int ierror, num_cv = numContinuousVars, algmethod = 1, logfile = 13,
      quiet_flag  = 1;
  double fmin = 0., eps = 1.e-4;

  // terminate when size of box  w/ f_min < sigmaper*size of orig box
  double sigmaper = (minBoxSize >= 0.) ? minBoxSize : 1.e-4;
  // terminate when volume of box w/ f_min < volper*volume of orig box
  double volper   = (volBoxSize >= 0.) ? volBoxSize : 1.e-6;
  // convergence tolerance for target solution (DIRECT wants 0. when inactive)
  double fglper   = (solutionTarget > -DBL_MAX) ? convergenceTol : 0.;

  // for passing additional data to objective_eval()
  int isize = 0, dsize = 0, csize = 0;
  int*    idata = NULL;
  double* ddata = NULL;
  char*   cdata = NULL;

//PDH: RealVector to double *
//     copy_data and .values() already used here.
//     do something to make it consistent with other wrappers using
//     new transfers/adapters.

  // Here local_des_vars is in the space of the original model
  RealVector local_des_vars;
  if (setUpType == SETUP_MODEL) {
    // initialize local_des_vars with DB initial point.  Variables are updated 
    // in constraint_eval/objective_eval
    copy_data(iteratedModel.continuous_variables(), local_des_vars);
    copy_data(iteratedModel.continuous_lower_bounds(), lowerBounds);
    copy_data(iteratedModel.continuous_upper_bounds(), upperBounds);
  } 
  else
    local_des_vars.size(num_cv);

  NCSU_DIRECT_F77(objective_eval, local_des_vars.values(), num_cv, eps,
		  maxFunctionEvals, maxIterations, fmin, lowerBounds.values(),
		  upperBounds.values(), algmethod, ierror, logfile,
		  solutionTarget, fglper, volper, sigmaper, idata, isize,
		  ddata, dsize, cdata, csize, quiet_flag);

  if (ierror < 0) {
    Cerr << "NCSU DIRECT failed with fatal error code " << ierror << "\n";
    switch (ierror) {
    case -1:
      Cerr << "(variable lower bounds must be strictly less than upper bounds)";
      break;
    case -2:
      Cerr << "(maximum function evaluations is too large)";
      break;
    case -3:
      Cerr << "(initialization in DIRpreprc failed)";
      break;
    case -4:
      Cerr << "(error in creation of the sample points)";
      break;
    case -5:
      Cerr << "(error occurred in sampling the function)";
      break;
    case -6:
      Cerr << "(maximum iterations is too large)";
      break;
    default:
      Cerr << "(unknown error code)";
    }
    Cerr << "\nSee \"Calling DIRECT\" section in DIRECT Version 2.0 User Guide"
	 << ".\n" << std::endl;
    abort_handler(-1);
  }
  else if (outputLevel >= QUIET_OUTPUT) {
    Cout << "NCSU DIRECT succeeded with code " << ierror << "\n";
    switch (ierror) {
    case 1:
      Cout << "(maximum function evaluations exceeded)";
      break;;
    case 2:
      Cout << "(maximum iterations reached)";
      break;;
    case 3:
      Cout << "(prescribed global minimum reached within tolerance)";
      break;;
    case 4:
      Cout << "(best rectangle reduced from original volume by prescribed "
	   << "fraction)";
      break;;
    case 5:
      Cout << "(best rectangle measure is less than prescribed min box size)";
      break;;
    default:
      Cout << "(unknown code)";
      break;;
    }
    Cout << std::endl;
  }

//PDH: Pretty clean due to use of .values() above, but do something to
//make this consistent with other wrappers using new transfers/adapters.

  // Set best variables and response for use by strategy level.
  // local_des_vars, fmin contain the optimal design 
  bestVariablesArray.front().continuous_variables(local_des_vars);
  if (!localObjectiveRecast) { // else local_objective_recast_retrieve()
                               // is used in Optimizer::post_run()
    RealVector best_fns(numFunctions);
    const BoolDeque& max_sense = iteratedModel.primary_response_fn_sense();
    best_fns[0] = (!max_sense.empty() && max_sense[0]) ? -fmin : fmin;
    bestResponseArray.front().function_values(best_fns);
  }

  // restore in case of recursion
  ncsudirectInstance = prev_instance;
}

} // namespace Dakota
