/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaMinimizer.hpp"
#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
#include "DakotaModel.hpp"
#include "ProblemDescDB.hpp"
#include "IteratorScheduler.hpp"
#include "ParamResponsePair.hpp"
#include "PRPMultiIndex.hpp"
#include "RecastModel.hpp"
#include "DataTransformModel.hpp"
#include "ScalingModel.hpp"
#include "Teuchos_SerialDenseHelpers.hpp"
#include "ExperimentData.hpp"

#ifdef __SUNPRO_CC
#include <math.h>  // for std::log
#endif // __SUNPRO_CC

static const char rcsId[]="@(#) $Id: DakotaMinimizer.cpp 7029 2010-10-22 00:17:02Z mseldre $";


namespace Dakota {

extern PRPCache data_pairs; // global container

// initialization of static needed by RecastModel
Minimizer* Minimizer::minimizerInstance(NULL);


/** This constructor extracts inherited data for the optimizer and least
    squares branches and performs sanity checking on constraint settings. */
Minimizer::
Minimizer(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib, std::shared_ptr<Model> model,
	  std::shared_ptr<TraitsBase> traits): 
  Iterator(problem_db, parallel_lib, traits),
  constraintTol(probDescDB.get_real("method.constraint_tolerance")),
  bigRealBoundSize(BIG_REAL_BOUND), bigIntBoundSize(1000000000),
  boundConstraintFlag(false),
  speculativeFlag(probDescDB.get_bool("method.speculative")),
  optimizationFlag(true),
  calibrationDataFlag(probDescDB.get_bool("responses.calibration_data") ||
    !probDescDB.get_string("responses.scalar_data_filename").empty()),
  expData(probDescDB, model->current_response().shared_data(), outputLevel),
  numExperiments(0), numTotalCalibTerms(0),
  scaleFlag(probDescDB.get_bool("method.scaling"))
{
  iteratedModel = model;
  update_from_model(*iteratedModel); // variable,response counts & checks

  // Re-assign Iterator defaults specialized to Minimizer branch
  // DataMethod defaults are assigned a special value of SZ_MAX, for
  // reassignment in different contexts
  if (maxIterations    == SZ_MAX)    maxIterations =  100;
  if (maxFunctionEvals == SZ_MAX) maxFunctionEvals = 1000;
  // Minimizer default number of final solution is 1, unless a
  // multi-objective frontier-based method
  if (!numFinalSolutions && methodName != MOGA)
    numFinalSolutions = 1;
}


Minimizer::
Minimizer(unsigned short method_name, std::shared_ptr<Model> model,
	  std::shared_ptr<TraitsBase> traits):
  Iterator(method_name, model, traits),
  constraintTol(0.), bigRealBoundSize(1.e+30), bigIntBoundSize(1000000000),
  boundConstraintFlag(false), speculativeFlag(false), optimizationFlag(true),
  calibrationDataFlag(false), numExperiments(0), numTotalCalibTerms(0),
  scaleFlag(false)
{
  update_from_model(*iteratedModel); // variable,constraint counts & checks
}


Minimizer::
Minimizer(std::shared_ptr<Model> model, size_t max_iter, size_t max_eval, Real conv_tol,
	  std::shared_ptr<TraitsBase> traits):
  Iterator(model, max_iter, max_eval, conv_tol, traits),
  constraintTol(0.), bigRealBoundSize(1.e+30), bigIntBoundSize(1000000000),
  boundConstraintFlag(false), speculativeFlag(false), optimizationFlag(true),
  calibrationDataFlag(false), numExperiments(0), numTotalCalibTerms(0),
  scaleFlag(false)
{
  update_from_model(*iteratedModel); // variable,constraint counts & checks
}


Minimizer::
Minimizer(unsigned short method_name, size_t num_lin_ineq, size_t num_lin_eq,
	  size_t num_nln_ineq, size_t num_nln_eq,
	  std::shared_ptr<TraitsBase> traits):
  Iterator(method_name, traits),
  bigRealBoundSize(1.e+30), bigIntBoundSize(1000000000),
  numNonlinearIneqConstraints(num_nln_ineq),
  numNonlinearEqConstraints(num_nln_eq), numLinearIneqConstraints(num_lin_ineq),
  numLinearEqConstraints(num_lin_eq),
  numNonlinearConstraints(num_nln_ineq + num_nln_eq),
  numLinearConstraints(num_lin_ineq + num_lin_eq),
  numConstraints(numNonlinearConstraints + numLinearConstraints),
  numUserPrimaryFns(1), numIterPrimaryFns(1), boundConstraintFlag(false),
  speculativeFlag(false), optimizationFlag(true), 
  calibrationDataFlag(false), numExperiments(0), numTotalCalibTerms(0),
  scaleFlag(false)
{ }


bool Minimizer::resize()
{
  bool parent_reinit_comms = Iterator::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}


void Minimizer::update_from_model(const Model& model)
{
  Iterator::update_from_model(model);

  numContinuousVars     = ModelUtils::cv(model);  numDiscreteIntVars  = ModelUtils::div(model);
  numDiscreteStringVars = ModelUtils::dsv(model); numDiscreteRealVars = ModelUtils::drv(model);
  numFunctions          = ModelUtils::response_size(model);

  bool err_flag = false;
  // Check for correct bit associated within methodName
  if ( !(methodName & MINIMIZER_BIT) ) {
    Cerr << "\nError: minimizer bit not activated for method instantiation "
	 << "within Minimizer branch." << std::endl;
    err_flag = true;
  }
  
  // Check for active design variables and discrete variable support.
  // Include explicit checking for COLINOptimizer methods that are not
  // representative of the majority (i.e. other COLINOptimizer methods)
  if(( traits()->supports_continuous_variables() && 
      traits()->supports_discrete_variables()) ||
    (methodName == COLINY_EA  ||  methodName == COLINY_BETA ))
  {
    if (!numContinuousVars && !numDiscreteIntVars && !numDiscreteStringVars &&
	!numDiscreteRealVars) {
      Cerr << "\nError: " << method_enum_to_string(methodName)
	   << " requires active variables." << std::endl;
      err_flag = true;
    }
  }
  else { // methods supporting only continuous design variables
    if (!numContinuousVars) {
      Cerr << "\nError: " << method_enum_to_string(methodName)
	   << " requires active continuous variables." << std::endl;
      err_flag = true;
    }
    if (numDiscreteIntVars || numDiscreteStringVars || numDiscreteRealVars)
      Cerr << "\nWarning: discrete design variables ignored by "
	   << method_enum_to_string(methodName) << std::endl;
  }

  // Check for response functions
  if ( numFunctions <= 0 ) {
    Cerr << "\nError: number of response functions must be greater than zero."
	 << std::endl;
    err_flag = true;
  }

  // check for gradient/Hessian/minimizer match: abort with an error for cases
  // where insufficient derivative data is available (e.g., full Newton methods
  // require Hessians), but only echo warnings in other cases (e.g., if more
  // derivative data is specified than is needed --> for example, don't enforce
  // that analytic Hessians require full Newton methods).
  const String& grad_type = model.gradient_type();
  const String& hess_type = model.hessian_type();
  if (outputLevel >= VERBOSE_OUTPUT)
    Cout << "Gradient type = " << grad_type << " Hessian type = " << hess_type
	 << '\n';
  if ( grad_type == "none" && ( ( methodName & LEASTSQ_BIT ) ||
       ( ( methodName & OPTIMIZER_BIT ) && methodName >= NONLINEAR_CG ) ) ) {
    Cerr << "\nError: gradient-based minimizers require a gradient "
         << "specification." << std::endl;
    err_flag = true;
  }
  if ( hess_type != "none" && methodName != OPTPP_NEWTON )
    Cerr << "\nWarning: Hessians are only utilized by full Newton methods.\n\n";
  if ( ( grad_type != "none" || hess_type != "none") &&
       ( ( methodName & OPTIMIZER_BIT ) && methodName < NONLINEAR_CG ) )
    Cerr << "\nWarning: Gradient/Hessian specification for a nongradient-based "
	 << "optimizer is ignored.\n\n";
  // TO DO: verify vendor finite differencing support
  vendorNumericalGradFlag
    = (grad_type == "numerical" && model.method_source() == "vendor");

  numNonlinearIneqConstraints = ModelUtils::num_nonlinear_ineq_constraints(model);
  numNonlinearEqConstraints   = ModelUtils::num_nonlinear_eq_constraints(model);
  numLinearIneqConstraints    = ModelUtils::num_linear_ineq_constraints(model); 
  numLinearEqConstraints      = ModelUtils::num_linear_eq_constraints(model);
  numNonlinearConstraints     = numNonlinearIneqConstraints
                              + numNonlinearEqConstraints;
  numLinearConstraints = numLinearIneqConstraints + numLinearEqConstraints;
  numConstraints       = numNonlinearConstraints + numLinearConstraints;
  numIterPrimaryFns    = numUserPrimaryFns = model.num_primary_fns();
  if (model.primary_fn_type() == CALIB_TERMS)
    numTotalCalibTerms = numUserPrimaryFns;  // default value

  // TO DO: hard error if not supported; warning if promoted;
  //        quiet if natively supported


  // Check for linear constraint support in method selection
  // Include explicit checking for COLINOptimizer and SNLLOptimizer methods
  // that are not representative of the respective majority
  // (i.e. other COLINOptimizer or SNLLOptimizer methods)
  if ( numLinearEqConstraints && (!traits()->supports_linear_equality() ||
      ( methodName == OPTPP_CG || methodName == OPTPP_PDS ||
        methodName == COLINY_SOLIS_WETS ))) {
    Cerr << "\nError: linear equality constraints not currently supported by "
	 << method_enum_to_string(methodName) << ".\n       Please select a "
	 << "different method." << std::endl;
    err_flag = true;
  }
  if ( numLinearIneqConstraints && (!traits()->supports_linear_inequality() ||
      ( methodName == OPTPP_CG || methodName == OPTPP_PDS ||
        methodName == COLINY_SOLIS_WETS ))) {
    Cerr << "\nError: linear inequality constraints not currently supported by "
	 << method_enum_to_string(methodName) << ".\n       Please select a "
	 << "different method." << std::endl;
    err_flag = true;
  }

  // Check for nonlinear constraint support in method selection
  // Include explicit checking for SNLLOptimizer methods that are not
  // representative of the majority (i.e. other SNLLOptimizer methods)
  if ( numNonlinearEqConstraints && (!traits()->supports_nonlinear_equality() ||
      ( methodName == OPTPP_CG || methodName == OPTPP_PDS))) {
    Cerr << "\nError: nonlinear equality constraints not currently supported by "
	 << method_enum_to_string(methodName) << ".\n       Please select a "
	 << "different method." << std::endl;
    err_flag = true;
  }
  if ( numNonlinearIneqConstraints &&
       (!traits()->supports_nonlinear_inequality() ||
      ( methodName == OPTPP_CG || methodName == OPTPP_PDS))) {
    Cerr << "\nError: nonlinear inequality constraints not currently supported by "
	 << method_enum_to_string(methodName) << ".\n       Please select a "
	 << "different method." << std::endl;
    err_flag = true;
  }

  if (err_flag)
    abort_handler(-1);

  // set boundConstraintFlag
  size_t i;
  const RealVector& c_l_bnds = ModelUtils::continuous_lower_bounds(model);
  const RealVector& c_u_bnds = ModelUtils::continuous_upper_bounds(model);
  //Cout << "Continuous lower bounds:\n" << c_l_bnds
  //     << "Continuous upper bounds:\n" << c_u_bnds;
  for (i=0; i<numContinuousVars; ++i)
    if (c_l_bnds[i] > -bigRealBoundSize || c_u_bnds[i] < bigRealBoundSize)
      { boundConstraintFlag = true; break; }
  bool discrete_bounds = (methodName == MOGA || methodName == SOGA ||
			  methodName == COLINY_EA);
  if (discrete_bounds) {
    const IntVector&  di_l_bnds = ModelUtils::discrete_int_lower_bounds(model);
    const IntVector&  di_u_bnds = ModelUtils::discrete_int_upper_bounds(model);
    const RealVector& dr_l_bnds = ModelUtils::discrete_real_lower_bounds(model);
    const RealVector& dr_u_bnds = ModelUtils::discrete_real_upper_bounds(model);
    for (i=0; i<numDiscreteIntVars; ++i)
      if (di_l_bnds[i] > -bigIntBoundSize || di_u_bnds[i] < bigIntBoundSize)
	{ boundConstraintFlag = true; break; }
    for (i=0; i<numDiscreteRealVars; ++i)
      if (dr_l_bnds[i] > -bigRealBoundSize || dr_u_bnds[i] < bigRealBoundSize)
	{ boundConstraintFlag = true; break; }
  }

  // Configure the TPL data Transfer helper
  dataTransferHandler.reset(new TPLDataTransfer()); 
  dataTransferHandler->configure_data_adapters( methodTraits, model );
}


void Minimizer::initialize_run()
{
  // Verify that iteratedModel is not null (default ctor and some
  // NoDBBaseConstructor ctors leave iteratedModel uninitialized).
  if (iteratedModel) {
    // update context data that is outside scope of local DB specifications.
    // This is needed for reused objects.
    //iteratedModel.db_scope_reset(); // TO DO: need better name?

    // This is to catch un-initialized models used by local iterators that
    // are not called through IteratorScheduler::run_iterator().  Within a
    // recursion, it will correspond to the first initialize_run() with an
    // uninitialized mapping, such as the outer-iterator on the first pass
    // of a recursion.  On subsequent passes, it may correspond to the inner
    // iterator.  The Iterator scope should not matter for the iteratedModel
    // mapping initialize/finalize.
    if (!iteratedModel->mapping_initialized()) {
      ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator();
      bool var_size_changed = iteratedModel->initialize_mapping(pl_iter);
      if (var_size_changed)
        /*bool reinit_comms =*/ resize(); // ignore return value
    }

    // Do not reset the evaluation reference for sub-iterators
    // (previously managed via presence/absence of ostream)
    //if (!subIteratorFlag)
    if (summaryOutputFlag)
      iteratedModel->set_evaluation_reference();
  }

  // Track any previous object instance in case of recursion.  Note that
  // optimizerInstance and minimizerInstance must be tracked separately since
  // the previous optimizer and previous minimizer could be different instances
  // (e.g., for MCUU with NL2SOL for NLS and NPSOL for MPP search, the previous
  // minimizer is NL2SOL and the previous optimizer is NULL).
  prevMinInstance   = minimizerInstance;
  minimizerInstance = this;

  if (subIteratorFlag) { 

    // Catch any updates to all inactive vars.  For now, do this in
    // initialize because derived classes may elect to reimplement
    // post_run.  Needs to happen before derived solvers update their
    // best points, so it doesn't trample their active variables data.

    // Dive into the originally passed model (could keep a shallow copy of it)
    // Don't use a reference here as want a shallow copy, not the instance
    auto usermodel = iteratedModel;
    for (unsigned short i=1; i<=myModelLayers; ++i) {
      usermodel = usermodel->subordinate_model();
    }
    
    // Could be lighter weight, but don't have a way to update only inactive
    bestVariablesArray.front().all_continuous_variables(
      ModelUtils::all_continuous_variables(*usermodel));
    bestVariablesArray.front().all_discrete_int_variables(
      ModelUtils::all_discrete_int_variables(*usermodel));
    bestVariablesArray.front().all_discrete_real_variables(
      ModelUtils::all_discrete_real_variables(*usermodel));
  }
}


void Minimizer::post_run(std::ostream& s)
{
  archive_best_results();
  if (summaryOutputFlag) {
    // Print the function evaluation summary for all Iterators
    if (iteratedModel)
      iteratedModel->print_evaluation_summary(s); // full hdr, relative counts

    // The remaining final results output varies by iterator branch
    print_results(s);
  }
}


void Minimizer::finalize_run()
{
  // Restore previous object instance in case of recursion.
  minimizerInstance = prevMinInstance;

  // Finalize an initialized mapping.  This will correspond to the first
  // finalize_run() with an uninitialized mapping, such as the inner-iterator
  // in a recursion.
  if (iteratedModel && iteratedModel->mapping_initialized()) {
    // paired to matching call to Model.initialize_mapping() in
    // initialize_run() above
    bool var_size_changed = iteratedModel->finalize_mapping();
    if (var_size_changed)
      /*bool reinit_comms =*/ resize(); // ignore return value
  }
}


std::shared_ptr<Model> Minimizer::original_model(unsigned short recasts_left) const
{
  // Dive into the originally passed model (could keep a shallow copy of it)
  // Don't use a reference here as want a shallow copy, not the instance
  auto usermodel = iteratedModel;
  for (unsigned short i=1; i<=myModelLayers - recasts_left; ++i) {
    usermodel = usermodel->subordinate_model();
  }

  return usermodel;
}


/** Reads observation data to compute least squares residuals and
    expands residuals for multiple experiments. */
void Minimizer::data_transform_model()
{
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Initializing calibration data transformation" << std::endl;
  
  // TODO: need better validation of these sizes and data with error msgs
  numExperiments = probDescDB.get_sizet("responses.num_experiments");
  if (numExperiments < 1) {
      Cerr << "Error in number of experiments" << std::endl;
      abort_handler(-1);
  }
  // TODO: verify: we don't want to weight by missing sigma: all = 1.0
  expData.load_data("Least Squares", iteratedModel->current_variables());

  if (numNonlinearConstraints > 0 && numExperiments > 1 &&
      expData.num_config_vars() > 0)
    Cout << "\nWarning: When using nonlinear constraints with multiple "
	 << "experiment\nconfigurations, the returned constraint values must be"
	 << " the same across\nconfigurations." << std::endl;

  iteratedModel = std::make_shared<DataTransformModel>(
    iteratedModel, expData, iteratedModel->current_variables().view());
  ++myModelLayers;
  dataTransformModel = iteratedModel;

  // update sizes in Iterator view from the RecastModel
  numIterPrimaryFns = numTotalCalibTerms = iteratedModel->num_primary_fns();
  numFunctions = ModelUtils::response_size(*iteratedModel);
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "Adjusted number of calibration terms: " << numTotalCalibTerms 
	 << std::endl;

  // adjust active set vector to reflect new residual size
  ShortArray asv(numFunctions, 1);
  activeSet.request_vector(asv);

  // TODO: review where should derivatives happen in data transform
  // case: where derivatives are computed should be tied to whether
  // interpolation is present

  // Need to correct a flaw in doing this: estimate derivatives uses a
  // mix of DB lookups and new evals, but the DB lookups aren't
  // transformed!

  // In the data transform case, perform numerical derivatives at the
  // RecastModel level (override the RecastModel default and the
  // subModel default)
  // BMA: Disabled until debugged...
  //  iteratedModel.supports_derivative_estimation(true);
  //  RecastModel* recast_model_rep = (RecastModel*) iteratedModel.model_rep();
  //  recast_model_rep->submodel_supports_derivative_estimation(false);
}



/** Wrap the iteratedModel in a scaling transformation, such that
    iteratedModel now contains a scaling recast model. Potentially
    affects variables, primary, and secondary responses */
void Minimizer::scale_model()
{
  // iteratedModel becomes the sub-model of a RecastModel:
  iteratedModel = std::make_shared<ScalingModel>(iteratedModel);
  scalingModel = iteratedModel;
  ++myModelLayers;

}


/** The composite objective computation sums up the contributions from
    one of more primary functions using the primary response fn weights. */
Real Minimizer::
objective(const RealVector& fn_vals, const BoolDeque& max_sense,
	  const RealVector& primary_wts) const
{
  return objective(fn_vals, numUserPrimaryFns, max_sense, primary_wts);
}


/** This "composite" objective is a more general case of the previous
    objective(), but doesn't presume a reduction map from user to
    iterated space.  Used to apply weights and sense in COLIN results
    sorting.  Leaving as a duplicate implementation pending resolution
    of COLIN lookups. */
Real Minimizer::
objective(const RealVector& fn_vals, size_t num_fns,
	  const BoolDeque& max_sense,
	  const RealVector& primary_wts) const
{
  Real obj_fn = 0.0;
  if (optimizationFlag) { // MOO
    bool use_sense = !max_sense.empty();
    if (primary_wts.empty()) {
      for (size_t i=0; i<num_fns; ++i)
	if (use_sense && max_sense[i]) obj_fn -= fn_vals[i];
	else                           obj_fn += fn_vals[i];
      if (num_fns > 1)
	obj_fn /= (Real)num_fns; // default weight = 1/n
    }
    else {
      for (size_t i=0; i<num_fns; ++i)
	if (use_sense && max_sense[i]) obj_fn -= primary_wts[i] * fn_vals[i];
	else                           obj_fn += primary_wts[i] * fn_vals[i];
    }
  }
  else { // NLS
    if (primary_wts.empty())
      for (size_t i=0; i<num_fns; ++i)
	obj_fn += std::pow(fn_vals[i], 2); // default weight = 1
    else
      for (size_t i=0; i<num_fns; ++i)
    	obj_fn += primary_wts[i] * std::pow(fn_vals[i], 2);
  }
  return obj_fn;
}


void Minimizer::
objective_gradient(const RealVector& fn_vals, const RealMatrix& fn_grads, 
		   const BoolDeque& max_sense, const RealVector& primary_wts,
		   RealVector& obj_grad) const
{
  objective_gradient(fn_vals, numUserPrimaryFns, fn_grads, max_sense,
		     primary_wts, obj_grad);
}


/** The composite objective gradient computation combines the
    contributions from one of more primary function gradients,
    including the effect of any primary function weights.  In the case
    of a linear mapping (MOO), only the primary function gradients are
    required, but in the case of a nonlinear mapping (NLS), primary
    function values are also needed.  Within RecastModel::set_mapping(),
    the active set requests are automatically augmented to make values
    available when needed, based on nonlinearRespMapping settings. */
void Minimizer::
objective_gradient(const RealVector& fn_vals, size_t num_fns,
		   const RealMatrix& fn_grads, const BoolDeque& max_sense, 
		   const RealVector& primary_wts, RealVector& obj_grad) const
{
  if (obj_grad.length() != numContinuousVars)
    obj_grad.sizeUninitialized(numContinuousVars);
  obj_grad = 0.;
  if (optimizationFlag) { // MOO
    bool use_sense = !max_sense.empty();
    if (primary_wts.empty()) {
      for (size_t i=0; i<num_fns; ++i) {
	const Real* fn_grad_i = fn_grads[i];
	if (use_sense && max_sense[i])
	  for (size_t j=0; j<numContinuousVars; ++j)
	    obj_grad[j] -= fn_grad_i[j];
	else
	  for (size_t j=0; j<numContinuousVars; ++j)
	    obj_grad[j] += fn_grad_i[j];
      }
      if (num_fns > 1)
	obj_grad.scale(1./(Real)num_fns); // default weight = 1/n
    }
    else {
      for (size_t i=0; i<num_fns; ++i) {
	const Real& wt_i      = primary_wts[i];
	const Real* fn_grad_i = fn_grads[i];
	if (use_sense && max_sense[i])
	  for (size_t j=0; j<numContinuousVars; ++j)
	    obj_grad[j] -= wt_i * fn_grad_i[j];
	else
	  for (size_t j=0; j<numContinuousVars; ++j)
	    obj_grad[j] += wt_i * fn_grad_i[j];
      }
    }
  }
  else { // NLS
    for (size_t i=0; i<num_fns; ++i) {
      Real wt_2_fn_val = 2. * fn_vals[i]; // default weight = 1
      if (!primary_wts.empty()) wt_2_fn_val *= primary_wts[i];
      const Real* fn_grad_i = fn_grads[i];
      for (size_t j=0; j<numContinuousVars; ++j)
	obj_grad[j] += wt_2_fn_val * fn_grad_i[j];
    }
  }
}


void Minimizer::
objective_hessian(const RealVector& fn_vals, const RealMatrix& fn_grads, 
		  const RealSymMatrixArray& fn_hessians, 
		  const BoolDeque& max_sense, const RealVector& primary_wts,
		  RealSymMatrix& obj_hess) const
{
  objective_hessian(fn_vals, numUserPrimaryFns, fn_grads, fn_hessians, 
		    max_sense, primary_wts, obj_hess);
}


/** The composite objective Hessian computation combines the
    contributions from one of more primary function Hessians,
    including the effect of any primary function weights.  In the case
    of a linear mapping (MOO), only the primary function Hessians are
    required, but in the case of a nonlinear mapping (NLS), primary
    function values and gradients are also needed in general
    (gradients only in the case of a Gauss-Newton approximation).
    Within the default RecastModel::set_mapping(), the active set
    requests are automatically augmented to make values and gradients
    available when needed, based on nonlinearRespMapping settings. */
void Minimizer::
objective_hessian(const RealVector& fn_vals, size_t num_fns,
		  const RealMatrix& fn_grads, 
		  const RealSymMatrixArray& fn_hessians, 
		  const BoolDeque& max_sense, const RealVector& primary_wts,
		  RealSymMatrix& obj_hess) const
{
  if (obj_hess.numRows() != numContinuousVars)
    obj_hess.shapeUninitialized(numContinuousVars);
  obj_hess = 0.;
  size_t i, j, k;
  if (optimizationFlag) { // MOO
    bool use_sense = !max_sense.empty();
    if (primary_wts.empty()) {
      for (i=0; i<num_fns; ++i) {
	const RealSymMatrix& fn_hess_i = fn_hessians[i];
	if (use_sense && max_sense[i])
	  for (j=0; j<numContinuousVars; ++j)
	    for (k=0; k<=j; ++k)
	      obj_hess(j,k) -= fn_hess_i(j,k);
	else
	  for (j=0; j<numContinuousVars; ++j)
	    for (k=0; k<=j; ++k)
	      obj_hess(j,k) += fn_hess_i(j,k);
      }
      if (num_fns > 1)
	obj_hess *= 1./(Real)num_fns; // default weight = 1/n
    }
    else
      for (i=0; i<num_fns; ++i) {
	const RealSymMatrix& fn_hess_i = fn_hessians[i];
	const Real&               wt_i = primary_wts[i];
	if (use_sense && max_sense[i])
	  for (j=0; j<numContinuousVars; ++j)
	    for (k=0; k<=j; ++k)
	      obj_hess(j,k) -= wt_i * fn_hess_i(j,k);
	else
	  for (j=0; j<numContinuousVars; ++j)
	    for (k=0; k<=j; ++k)
	      obj_hess(j,k) += wt_i * fn_hess_i(j,k);
      }
  }
  else { // NLS
    if (fn_grads.empty()) {
      Cerr << "Error: Hessian reduction for NLS requires a minimum of least "
	   << "squares gradients (for Gauss-Newton)." << std::endl;
      abort_handler(-1);
    }
    // Gauss_Newton approx Hessian = J^T J (neglect f*H term)
    if (fn_hessians.empty() || fn_vals.empty()) {
      if (primary_wts.empty())
	for (j=0; j<numContinuousVars; ++j)
	  for (k=0; k<=j; ++k) {
	    Real& sum = obj_hess(j,k); sum = 0.;
	    for (i=0; i<num_fns; ++i)
	      sum += fn_grads(j,i) * fn_grads(k,i); // default weight = 1
	    sum *= 2.;
	  }
      else
	for (j=0; j<numContinuousVars; ++j)
	  for (k=0; k<=j; ++k) {
	    Real& sum = obj_hess(j,k); sum = 0.;
	    for (i=0; i<num_fns; ++i)
	      sum += primary_wts[i] * fn_grads(j,i) * fn_grads(k,i);
	    sum *= 2.;
	  }
    }
    // full Hessian = f H + J^T J
    else {
      if (primary_wts.empty())
	for (j=0; j<numContinuousVars; ++j)
	  for (k=0; k<=j; ++k) {
	    Real& sum = obj_hess(j,k); sum = 0.;
	    for (i=0; i<num_fns; ++i)
	      sum += fn_grads(j,i) * fn_grads(k,i) +
		fn_vals[i] * fn_hessians[i](j,k);
	    sum *= 2.;
	  }
      else
	for (j=0; j<numContinuousVars; ++j)
	  for (k=0; k<=j; ++k) {
	    Real& sum = obj_hess(j,k); sum = 0.;
	    for (i=0; i<num_fns; ++i)
	      sum += primary_wts[i] * (fn_grads(j,i) * fn_grads(k,i) +
				       fn_vals[i] * fn_hessians[i](j,k));
	    sum *= 2.;
	  }
    }
  }
}


/** Lookup evaluation id where best occurred.  This cannot be
    catalogued directly because the optimizers track the best iterate
    internally and return the best results after iteration completion.
    Therfore, perform a search in data_pairs to extract the evalId for
    the best fn eval. */
void Minimizer::print_best_eval_ids(const String& search_interface_id,
				    const Variables& search_vars,
				    const ActiveSet& search_set,
				    std::ostream& s)
{
  const String
    best_id = "<<<<< Best evaluation ID: ",
    best_id_restart = "<<<<< Best evaluation ID not found among current execution's evaluations, but\nretrieved from restart file evaluation ID: ",
    best_id_partial = "<<<<< Best evaluation ID (partial match): ",
    best_ids_partial = "<<<<< Best evaluation IDs (partial matches): ",
    id_na = "<<<<< Best evaluation ID not available\n",
    id_full_na = "<<<<< Best evaluation ID (full match) not available\n",
    id_warning = "(This warning may occur when the best iterate is comprised of multiple interface\nevaluations or arises from a composite, surrogate, or transformation model.)\n";

  PRPCacheHIter cache_it =
    lookup_by_val(data_pairs, search_interface_id, search_vars, search_set);
  if (cache_it == data_pairs.get<hashed>().end()) {

    // no exact match; try to match only vars/interface ID via hash
    // (don't check search_set)
    Response search_resp(SIMULATION_RESPONSE, search_set);
    ParamResponsePair search_pr(search_vars, search_interface_id, search_resp);
    PRPCacheHIter prp_hash_it0, prp_hash_it1;
    boost::tuples::tie(prp_hash_it0, prp_hash_it1)
      = data_pairs.get<hashed>().equal_range(search_pr);

    std::set<int> sorted_eval_ids; // in case hash isn't in eval ID order
    while (prp_hash_it0 != prp_hash_it1) {
      sorted_eval_ids.insert(prp_hash_it0->eval_id());
      ++prp_hash_it0;
    }

    if (sorted_eval_ids.empty())
      s << id_na << id_warning;
    else {
      s << id_full_na << id_warning;
      s << ((sorted_eval_ids.size() == 1) ? best_id_partial : best_ids_partial);
      // gymnastics due to use of set, which lacks .back():
      auto it = sorted_eval_ids.begin();
      while (it != sorted_eval_ids.end()) {
	s << *(it++);
	if (it != sorted_eval_ids.end())
	  s << ", ";
      }
      s << '\n';
    }
  }
  else {
    int eval_id = cache_it->eval_id();
    if (eval_id > 0)
      s << best_id << eval_id << '\n';
    else // should not occur
      s << best_id_restart << -eval_id << '\n';
  }
  s << std::endl;
}


void Minimizer::archive_best_variables(const bool active_only) const
{
  if(!resultsDB.active()) return;
  // archive the best point in the iterator database
  const StrStrSizet &iterator_id = run_identifier();
  const size_t num_points = bestVariablesArray.size();

  
  const auto & cv_labels = (active_only) ? 
                           variables_results().continuous_variable_labels() :
                           variables_results().all_continuous_variable_labels();
  const auto & div_labels = (active_only) ?
                           variables_results().discrete_int_variable_labels() :
                           variables_results().all_discrete_int_variable_labels();
  const auto & dsv_labels = (active_only) ?
                           variables_results().discrete_string_variable_labels() :
                           variables_results().all_discrete_string_variable_labels();
  const auto & drv_labels = (active_only) ? 
                           variables_results().discrete_real_variable_labels() :
                           variables_results().all_discrete_real_variable_labels();
  // ##  legacy text output ##
  if(numContinuousVars) {
    // labels
    resultsDB.insert
      (iterator_id, resultsNames.cv_labels, cv_labels);
    // best variables, with labels in metadata
    MetaDataType md;
    md["Array Spans"] = make_metadatavalue("Best Sets");
    md["Row Labels"] = 
      make_metadatavalue(cv_labels); 
    resultsDB.array_allocate<RealVector>
      (iterator_id, resultsNames.best_cv, num_points, md);
  } 
  if (numDiscreteIntVars) {
     // labels
    resultsDB.insert
      (iterator_id, resultsNames.div_labels, div_labels);
    // best variables, with labels in metadata
    MetaDataType md;
    md["Array Spans"] = make_metadatavalue("Best Sets");
    md["Row Labels"] = 
      make_metadatavalue(div_labels); 
    resultsDB.array_allocate<IntVector>
      (iterator_id, resultsNames.best_div, num_points, md);
  }
  if (numDiscreteStringVars) {
    // labels
    resultsDB.insert
      (iterator_id, resultsNames.dsv_labels, dsv_labels);
    // best variables, with labels in metadata
    MetaDataType md;
    md["Array Spans"] = make_metadatavalue("Best Sets");
    md["Row Labels"] = 
      make_metadatavalue(dsv_labels); 
    resultsDB.array_allocate<StringArray>
      (iterator_id, resultsNames.best_dsv, num_points, md);
  }
  if (numDiscreteRealVars) {
    // labels
    resultsDB.insert
      (iterator_id, resultsNames.drv_labels, drv_labels); 
    // best variables, with labels in metadata
    MetaDataType md;
    md["Array Spans"] = make_metadatavalue("Best Sets");
    md["Row Labels"] = 
      make_metadatavalue(drv_labels);
    resultsDB.array_allocate<RealVector>
      (iterator_id, resultsNames.best_drv, num_points, md);
  }

  DimScaleMap cv_scale;
  cv_scale.emplace(0, StringScale("variables", cv_labels)); 
  DimScaleMap div_scale;
  div_scale.emplace(0, StringScale("variables", div_labels));
  DimScaleMap dsv_scale;
  dsv_scale.emplace(0, StringScale("variables", dsv_labels)); 
  DimScaleMap drv_scale;
  drv_scale.emplace(0, StringScale("variables", drv_labels));

  StringArray location;
  size_t r_index = 1; // index in location of variable type (e.g. "continuous")
  if(num_points > 1) {
    location.push_back("");
    r_index = 2;
  }
  location.push_back("best_parameters");
  location.push_back("");
  size_t point_index = 0;
  for(const auto & best_vars : bestVariablesArray) {
    if(num_points > 1)
      location[0] = String("set:") + std::to_string(point_index+1);
    if (numContinuousVars) {
      // coreDB backend which will likely be removed in the future - RWH
      resultsDB.array_insert<RealVector>
        (run_identifier(), resultsNames.best_cv, point_index,
         best_vars.continuous_variables());

      // hdf5DB backend
      location[r_index] = "continuous";
      if(active_only)
        resultsDB.insert(iterator_id, location, best_vars.continuous_variables(), cv_scale);
      else
        resultsDB.insert(iterator_id, location, best_vars.all_continuous_variables(), cv_scale);
    }

    if (numDiscreteIntVars) {
      // coreDB backend which will likely be removed in the future - RWH
      resultsDB.array_insert<IntVector>
        (run_identifier(), resultsNames.best_div, point_index,
         best_vars.discrete_int_variables());

      // hdf5DB backend
      location[r_index] = "discrete_integer";
      if(active_only)
        resultsDB.insert(iterator_id, location, best_vars.discrete_int_variables(), div_scale);
      else
        resultsDB.insert(iterator_id, location, best_vars.all_discrete_int_variables(), div_scale);
    }

    if (numDiscreteStringVars) {
      // coreDB backend which will likely be removed in the future - RWH
      resultsDB.array_insert<StringArray>
        (run_identifier(), resultsNames.best_dsv, point_index,
         best_vars.discrete_string_variables());

      // hdf5DB backend
      location[r_index] = "discrete_string";
      if(active_only)
        resultsDB.insert(iterator_id, location, best_vars.discrete_string_variables(), dsv_scale);
      else
        resultsDB.insert(iterator_id, location, best_vars.all_discrete_string_variables(), dsv_scale);
    }

    if (numDiscreteRealVars) {
      // coreDB backend which will likely be removed in the future - RWH
      resultsDB.array_insert<RealVector>
        (run_identifier(), resultsNames.best_drv, point_index,
         best_vars.discrete_real_variables());

      // hdf5DB backend
      location[r_index] = "discrete_real";
      if(active_only)
        resultsDB.insert(iterator_id, location, best_vars.discrete_real_variables(), drv_scale);
      else
        resultsDB.insert(iterator_id, location, best_vars.all_discrete_real_variables(), drv_scale);
    }
    point_index++;
  }
}


void Minimizer::
archive_best_objective_functions() const
{
  const size_t num_points = bestResponseArray.size();
  StrStrSizet iterator_id = run_identifier();
  // ##  legacy text output ##
  // labels
  resultsDB.insert(iterator_id, resultsNames.fn_labels,
     response_results().function_labels());
  // best functions, with labels in metadata
  MetaDataType md;
  md["Array Spans"] = make_metadatavalue("Best Sets");
  md["Row Labels"] = 
    make_metadatavalue(response_results().function_labels());
  resultsDB.array_allocate<RealVector>
    (iterator_id, resultsNames.best_fns, num_points, md);

  size_t point_index = 0;
  StringArray location;
  if(num_points > 1)
    location.push_back("");
  location.push_back("best_objective_functions");
  DimScaleMap scale;
  scale.emplace(0, StringScale("responses", response_results().function_labels())); 
  for(const auto & best_resp : bestResponseArray) {
    if(num_points > 1)
      location[0] = String("set:") + std::to_string(point_index + 1);
    // coreDB-based API Results output
    resultsDB.array_insert<RealVector>
      (iterator_id, resultsNames.best_fns, point_index, best_resp.function_values());

    // hdf5DB-based API Results output
    const RealVector &fvals = best_resp.function_values();
      Teuchos::SerialDenseVector<int, Real> primary(Teuchos::View, const_cast<Real*>(&fvals[0]), numUserPrimaryFns);
      resultsDB.insert(iterator_id,location, primary, scale);
    point_index++;
  }
} 


/// Archive residuals when calibration terms are used
void Minimizer::
archive_best_residuals() const
{
  if(!resultsDB.active()) return;
  
  const RealVector& lsq_weights 
      = original_model()->primary_response_fn_weights();
  const StrStrSizet &iterator_id = run_identifier();
  size_t num_points = bestResponseArray.size();

  // ##  legacy text output ##
  // labels
  resultsDB.insert(iterator_id, resultsNames.fn_labels,
     response_results().function_labels());
  // best functions, with labels in metadata
  MetaDataType md;
  md["Array Spans"] = make_metadatavalue("Best Sets");
  md["Row Labels"] = 
    make_metadatavalue(response_results().function_labels());
  resultsDB.array_allocate<RealVector>
    (iterator_id, resultsNames.best_fns, num_points, md);

  // HDF5 setup
  StringArray residuals_location;
  StringArray norm_location;
  if(num_points > 1) {
    residuals_location.push_back("");
    norm_location.push_back("");
  }
  residuals_location.push_back("best_residuals");
  norm_location.push_back("best_norm");
  size_t point_index = 0;
  for(const auto &resp : bestResponseArray) {
    if(num_points > 1) {
      String set_string = String("set:") + std::to_string(point_index + 1);
      residuals_location[0] = set_string;
      norm_location[0] = set_string;
    }
    const RealVector &best_terms = resp.function_values();
    Real wssr =  std::sqrt(sum_squared_residuals(numUserPrimaryFns, best_terms, lsq_weights));
    Teuchos::SerialDenseVector<int, Real> residuals(Teuchos::View, 
                                const_cast<Real*>(&best_terms[0]), 
                                numUserPrimaryFns);
    resultsDB.insert(iterator_id, residuals_location, residuals);
    resultsDB.insert(iterator_id, norm_location, wssr);
    // coreDB
    resultsDB.array_insert<RealVector>
            (iterator_id, resultsNames.best_fns, point_index, resp.function_values());
    point_index++;
  }
}


void Minimizer::
archive_best_constraints() const {
  if(!resultsDB.active() || !numNonlinearConstraints) return;
  const size_t num_points = bestResponseArray.size();
  StrStrSizet iterator_id = run_identifier();
  StringArray location;
  if(num_points > 1) 
    location.push_back("");
  location.push_back("best_constraints");
  DimScaleMap scales;
  scales.emplace(0, StringScale(String("nonlinear_constraints"),
          response_results().function_labels(), numUserPrimaryFns, numNonlinearConstraints));
  size_t point_index = 0;
  for(const auto & best_resp : bestResponseArray) {
    if(num_points > 1)
      location[0] = String("set:") + std::to_string(point_index+1);
    const RealVector &fvals = best_resp.function_values();
    Teuchos::SerialDenseVector<int, Real> secondary(Teuchos::View, 
                                const_cast<Real*>(&fvals[numUserPrimaryFns]), 
                                numNonlinearConstraints);
    resultsDB.insert(iterator_id, location, secondary, scales);
    point_index++;
  }
}


/** Uses data from the innermost model, should any Minimizer recasts be active.
    Called by multipoint return solvers. Do not directly call resize on the 
    bestVariablesArray object unless you intend to share the internal content 
    (letter) with other objects after assignment. */
void Minimizer::resize_best_vars_array(size_t newsize) 
{
  size_t curr_size = bestVariablesArray.size();

  if(newsize < curr_size) {
    // If reduction in size, use the standard resize
    bestVariablesArray.resize(newsize);
  }
  else if(newsize > curr_size) {

    // Otherwise, we have to do the iteration ourselves so that we make use
    // of the model's current variables for envelope-letter requirements.

    // Best point arrays have be sized and scaled in the original user space.  
    auto usermodel = original_model();
    bestVariablesArray.reserve(newsize);
    for(size_t i=curr_size; i<newsize; ++i)
      bestVariablesArray.push_back(usermodel->current_variables().copy());
  }
  // else no size change
}


/** Uses data from the innermost model, should any Minimizer recasts be active.
    Called by multipoint return solvers. Do not directly call resize on the 
    bestResponseArray object unless you intend to share the internal content 
    (letter) with other objects after assignment. */
void Minimizer::resize_best_resp_array(size_t newsize) 
{
  size_t curr_size = bestResponseArray.size();

  if (newsize < curr_size) {
    // If reduction in size, use the standard resize
    bestResponseArray.resize(newsize);
  }
  else if (newsize > curr_size) {

    // Otherwise, we have to do the iteration ourselves so that we make use
    // of the model's current response for envelope-letter requirements.

    // Best point arrays have be sized and scaled in the original user space.  
    auto usermodel = original_model();
    bestResponseArray.reserve(newsize);
    for(size_t i=curr_size; i<newsize; ++i)
      bestResponseArray.push_back(usermodel->current_response().copy());
  }
  // else no size change
}


Real Minimizer::
sum_squared_residuals(size_t num_pri_fns, const RealVector& residuals, 
		      const RealVector& weights)
{
  if (!weights.empty() && num_pri_fns != weights.length()) {
    Cerr << "\nError (sum_squared_residuals): incompatible residual and weight "
	 << "lengths." << std::endl;
    abort_handler(-1);
  }

  // TODO: Just call reduce/objective on the data?
  Real t = 0.;
  for(size_t j=0; j<num_pri_fns; ++j) {
    const Real& t1 = residuals[j];
    if (weights.empty())
      t += t1*t1;
    else
      t += t1*t1*weights[j];
  }

  return t;
}


void Minimizer::print_model_resp(size_t num_pri_fns, const RealVector& best_fns,
				 size_t num_best, size_t best_index,
				 std::ostream& s)
{
  if (num_pri_fns > 1) s << "<<<<< Best model responses "; 
  else                 s << "<<<<< Best model response "; 
  if (num_best > 1) s << "(set " << best_index+1 << ") "; s << "=\n"; 
  write_data_partial(s, (size_t)0, num_pri_fns, best_fns); 
}


void Minimizer::print_residuals(size_t num_terms, const RealVector& best_terms, 
				const RealVector& weights, 
				size_t num_best, size_t best_index,
				std::ostream& s)
{
  // Print best response functions
  if (num_terms > 1) s << "<<<<< Best residual terms ";
  else               s << "<<<<< Best residual term  ";
  if (num_best > 1) s << "(set " << best_index+1 << ") "; s << "=\n";
  write_data_partial(s, (size_t)0, num_terms, best_terms);

  // BMA TODO: if data and scaling are present, this won't print
  // correct weighted residual norms

  Real wssr = sum_squared_residuals(num_terms, best_terms, weights);

  s << "<<<<< Best residual norm ";
  if (num_best > 1) s << "(set " << best_index+1 << ") ";
  s << "= " << std::setw(write_precision+7)
    << std::sqrt(wssr) << "; 0.5 * norm^2 = " 
    << std::setw(write_precision+7) << 0.5*wssr << '\n';
}


void Minimizer::archive_best_results()
{
  if(!resultsDB.active()) return;
  size_t i, num_best = bestVariablesArray.size();
  if (num_best != bestResponseArray.size()) {
    Cerr << "\nError: mismatch in lengths of bestVariables and bestResponses."
         << std::endl;
    abort_handler(-1);
  }

  StrStrSizet iterator_id = run_identifier();
  // must search in the inbound Model's space (and even that may not
  // suffice if there are additional recastings underlying this
  // Optimizer's Model) to find the function evaluation ID number
  auto orig_model = original_model();
  const String& interface_id = orig_model->interface_id();
  // use asv = 1's
  ActiveSet search_set(ModelUtils::response_size(*orig_model), numContinuousVars);
  int eval_id;

  if(numNonlinearConstraints)
    archive_best_constraints();
  if(optimizationFlag) {
    archive_best_objective_functions();
    archive_best_variables();
  } else if(!calibrationDataFlag) {
    // the original model had least squares terms
    archive_best_residuals();
    archive_best_variables();
  } else { //calibration with data
    std::shared_ptr<DataTransformModel> dt_model_rep =
      std::static_pointer_cast<DataTransformModel>(dataTransformModel);
    if(dt_model_rep->num_config_vars())
      archive_best_variables(true);
    else
      archive_best_variables();
    for (i=0; i<num_best; ++i) {
      const Variables& best_vars = bestVariablesArray[i];
      // output best response
      const RealVector& best_fns = bestResponseArray[i].function_values(); 
      if (!optimizationFlag && calibrationDataFlag) {
          dt_model_rep->archive_best_responses(resultsDB, iterator_id, 
                                             best_vars, bestResponseArray[i],
                                             num_best, i);
      }
    }
  }
  // Associate evaluation ids as metadata
  String set_string = "set:";
  StringArray location(1);
  for(i=0; i < num_best; ++i) {
    // lookup evaluation id where best occurred.  This cannot be catalogued
    // directly because the optimizers track the best iterate internally and
    // return the best results after iteration completion.  Therfore, perform a
    // search in data_pairs to extract the evalId for the best fn eval.
    const Variables& best_vars = bestVariablesArray[i];
    PRPCacheHIter cache_it = lookup_by_val(data_pairs, interface_id,
                                           best_vars, search_set);
    if (cache_it == data_pairs.get<hashed>().end()) 
      eval_id = 0;
    else 
      eval_id = cache_it->eval_id();
    AttributeArray attrs = {ResultAttribute<int>("evaluation_id", eval_id)};
    if(num_best > 1) {
      location[0] = set_string+std::to_string(i+1);
      resultsDB.add_metadata_to_object(iterator_id,location, attrs);
    } else
      resultsDB.add_metadata_to_execution(iterator_id, attrs);
  }
}


/** Retrieve a MOO/NLS response based on the data returned by a single
    objective optimizer by performing a data_pairs search. This may
    get called even for a single user-specified function, since we may
    be recasting a single NLS residual into a squared
    objective. Always returns best data in the space of the original
    inbound Model. */
bool Minimizer::
local_recast_retrieve(const Variables& vars, Response& response) const
{
  // TODO: could omit constraints for solvers populating them (there
  // may not exist a single DB eval with both functions, constraints)
  ActiveSet lookup_set(response.active_set());
  PRPCacheHIter cache_it
    = lookup_by_val(data_pairs, iteratedModel->interface_id(), vars, lookup_set);
  if (cache_it == data_pairs.get<hashed>().end()) {
    Cerr << "Warning: failure in recovery of final values for locally recast "
	 << "optimization." << std::endl;
    return false;
  }    
  response.update(cache_it->response());
  return true;
}

} // namespace Dakota
