/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       OptTraits
//- Description: Implementation code for the OptTraits class
//- Owner:       Moe Khalil
//- Checked by:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "DakotaModel.hpp"
#include "DataTransformModel.hpp"
#include "ScalingModel.hpp"
#include "DakotaOptTraits.hpp"
#include "ParamResponsePair.hpp"
#include "PRPMultiIndex.hpp"
#include "ProblemDescDB.hpp"
#include "RecastModel.hpp"

static const char rcsId[]="@(#) $Id: DakotaOptTraits.hpp 0001 2017-02-21 10:35:14Z mkhalil $";


namespace Dakota {

extern PRPCache data_pairs; // global container

// initialization of static needed by RecastModel
OptTraits* OptTraits::optTraitsInstance(NULL);


OptTraits::OptTraits(ProblemDescDB& problem_db, Model& model):
  Minimizer(problem_db, model),
  supportsMultiobjectives(false),
  // initial value from Minimizer as accounts for fields and transformations
  numObjectiveFns(numUserPrimaryFns), localObjectiveRecast(false)
{
  optimizationFlag = true; // default; may be overridden below

  bool err_flag = false;
  // Check for correct bit associated within methodName
  if ( !(methodName & OPTIMIZER_BIT) ) {
    Cerr << "\nError: optimizer bit not activated for method instantiation "
	 << "within Optimizer branch." << std::endl;
    err_flag = true;
  }
  // Check for bound constraint support/requirement in method selection
  if ( boundConstraintFlag && methodName == OPTPP_CG ) {
    Cerr << "\nError: bound constraints not currently supported by "
         << method_enum_to_string(methodName) << ".\n       Please select a "
	 << "different method for bound constrained problems." << std::endl;
    err_flag = true;
  }
  // RWH: Check for finite bound constraints for Coliny global optimizers
  if ( (methodName == COLINY_DIRECT) || (methodName == COLINY_EA) ) {
    const RealVector& c_l_bnds = model.continuous_lower_bounds();
    const RealVector& c_u_bnds = model.continuous_upper_bounds();
    for (size_t i=0; i<numContinuousVars; ++i)
      if (c_l_bnds[i] <= -bigRealBoundSize || c_u_bnds[i] >= bigRealBoundSize) {
        Cerr << "\nError: finite bound constraints are required for global optimizer "
          << method_enum_to_string(methodName) << ".\n       Please specify both "
          << "lower_bounds and upper_bounds." << std::endl;
        err_flag = true;
        break;
      }
  }
 
  // Check for nongradient method with speculative flag set
  if ( speculativeFlag && methodName < NONLINEAR_CG ) {
    Cerr << "\nWarning: Speculative specification for a nongradient-based "
	 << "optimizer is ignored.\n";
    speculativeFlag = false;
  }

  // Check for full Newton method w/o Hessian support (direct or via recast)
  bool require_hessians = false;
  bool have_lsq = (model.primary_fn_type() == CALIB_TERMS);
  if (methodName == OPTPP_NEWTON) { // || ...) {
    require_hessians = true;
    if ( ( !have_lsq && iteratedModel.hessian_type()  == "none" ) ||
	 (  have_lsq && iteratedModel.gradient_type() == "none" ) ) {
      Cerr << "\nError: full Newton optimization requires objective Hessians."
	   << std::endl;
      err_flag = true;
    }
  }

  // Initialize a best variables instance; bestVariablesArray should
  // be in calling context; so initialized before any recasts
  bestVariablesArray.push_back(iteratedModel.current_variables().copy());

  // Check for proper response function definition (optimization or
  // calibration) and manage local recasting (necessary if inbound
  // model contains calibration terms)
  if (have_lsq) {
    // use local recast to solve NLS problems as single-objective optimization
    Cerr << "Warning: coercing least squares data set into optimization data "
	 << "set." << std::endl;
    optimizationFlag = false; // used to distinguish NLS from MOO
    localObjectiveRecast = true;
  }
  else if (model.primary_fn_type() == OBJECTIVE_FNS) {
    // we allow SOGA to manage weighted multiple objectives where
    // possible, so we can better retrieve final results
    if (numUserPrimaryFns > 1 && 
	(methodName != MOGA && methodName != SOGA))
      localObjectiveRecast = true;
  }
  else {
    Cerr << "\nError: responses specification is incompatible with "
	 << "optimization methods." << std::endl;
    err_flag = true;
  }

  if (err_flag)
    abort_handler(-1);

  // TODO: can't allow experimental data with SBNLS for now!  Can we
  // data transform, scale, weight?

  // Wrap the iteratedModel in 0 -- 3 RecastModels, potentially resulting
  // in reduce(scale(data(model)))
  if (calibrationDataFlag)
    data_transform_model();
  if (scaleFlag)
    scale_model();
}


OptTraits::OptTraits(unsigned short method_name, Model& model):
  Minimizer(method_name, model),
  supportsMultiobjectives(false),
  numObjectiveFns(numUserPrimaryFns),localObjectiveRecast(false)
{
  if (numObjectiveFns > 1) {
    Cerr << "Error: on-the-fly Optimizer instantiations do not currently "
	 << "support multiple objective functions." << std::endl;
    abort_handler(-1);
  }

  optimizationFlag = true;

  // Initialize a best variables instance
  bestVariablesArray.push_back(model.current_variables().copy());
}


OptTraits::
OptTraits(unsigned short method_name, size_t num_cv, size_t num_div,
	  size_t num_dsv, size_t num_drv, size_t num_lin_ineq,
	  size_t num_lin_eq, size_t num_nln_ineq, size_t num_nln_eq):
  Minimizer(method_name, num_lin_ineq, num_lin_eq, num_nln_ineq, num_nln_eq),
  supportsMultiobjectives(false),
  numObjectiveFns(1), localObjectiveRecast(false)
{
  numContinuousVars     = num_cv;
  numDiscreteIntVars    = num_div;
  numDiscreteStringVars = num_dsv;
  numDiscreteRealVars   = num_drv;
  numFunctions          = numUserPrimaryFns + numNonlinearConstraints;
  optimizationFlag      = true;

  // The following "best" initializations are done here instead of in
  // Minimizer for this lightweight case
  std::pair<short,short> view(MIXED_DESIGN, EMPTY_VIEW);
  SizetArray vc_totals(NUM_VC_TOTALS, 0);
  vc_totals[TOTAL_CDV] = num_cv;  vc_totals[TOTAL_DDIV] = num_div;
  vc_totals[TOTAL_DDSV] = num_dsv; vc_totals[TOTAL_DDRV] = num_drv;
  BitArray all_relax_di, all_relax_dr; // empty: no relaxation of discrete
  SharedVariablesData svd(view, vc_totals, all_relax_di, all_relax_dr);
  bestVariablesArray.push_back(Variables(svd));

  activeSet.reshape(numFunctions, numContinuousVars);
  activeSet.request_values(1); activeSet.derivative_start_value(1);
  bestResponseArray.push_back(Response(SIMULATION_RESPONSE, activeSet));
}


/** Sets supportsMultiobjectives to true */
void OptTraits::set_supports_multiobjectives()
{
  supportsMultiobjectives = true;
}


/** Return the value of supportsMultiobjectives */
void OptTraits::get_supports_multiobjectives(bool supports_multiobjectives)
{
  supports_multiobjectives = supportsMultiobjectives;
}

} // namespace Dakota
