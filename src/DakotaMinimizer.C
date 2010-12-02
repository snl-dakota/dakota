/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Minimizer
//- Description: Implementation code for the Minimizer class
//- Owner:       Mike Eldred
//- Checked by:

#include "DakotaMinimizer.H"
#include "system_defs.h"
#include "data_io.h"
#include "DakotaModel.H"
#include "ProblemDescDB.H"
#include "ParamResponsePair.H"
#include "RecastModel.H"

static const char rcsId[]="@(#) $Id: DakotaMinimizer.C 7029 2010-10-22 00:17:02Z mseldre $";

using std::log;

namespace Dakota {

// initialization of static needed by RecastModel
Minimizer* Minimizer::minimizerInstance(NULL);

/** This constructor extracts inherited data for the optimizer and
    least squares branches and performs sanity checking on constraint
    settings. */
Minimizer::Minimizer(Model& model): Iterator(BaseConstructor(), model),
  constraintTol(probDescDB.get_real("method.constraint_tolerance")),
  bigRealBoundSize(1.e+30), bigIntBoundSize(1000000000),
  numNonlinearIneqConstraints(model.num_nonlinear_ineq_constraints()),
  numNonlinearEqConstraints(model.num_nonlinear_eq_constraints()),
  numLinearIneqConstraints(model.num_linear_ineq_constraints()), 
  numLinearEqConstraints(model.num_linear_eq_constraints()),
  numNonlinearConstraints(numNonlinearIneqConstraints +
			  numNonlinearEqConstraints),
  numLinearConstraints(numLinearIneqConstraints + numLinearEqConstraints),
  numConstraints(numNonlinearConstraints + numLinearConstraints),
  boundConstraintFlag(false),
  speculativeFlag(probDescDB.get_bool("method.speculative")),
  scaleFlag(probDescDB.get_bool("method.scaling")), varsScaleFlag(false),
  primaryRespScaleFlag(false), secondaryRespScaleFlag(false)
{
  // Re-assign Iterator defaults specialized to Minimizer branch
  if (maxIterations < 0) // DataMethod default set to -1
    maxIterations = 100;
  // Minimizer default number of final solution is 1, unless a
  // multi-objective frontier-based method
  if (!numFinalSolutions && methodName != "moga")
    numFinalSolutions = 1;

  // Check for active design variables
  size_t num_ddrv = probDescDB.get_sizet("variables.discrete_design_range"),
    num_ddsiv = probDescDB.get_sizet("variables.discrete_design_set_int"),
    num_ddsrv = probDescDB.get_sizet("variables.discrete_design_set_real");
  if (methodName == "moga" || methodName == "soga") {
    if (!numContinuousVars && !numDiscreteIntVars && !numDiscreteRealVars) {
      Cerr << "\nError: " << methodName << " requires active continuous or "
	   << "active discrete variables." << std::endl;
      abort_handler(-1);
    }
  }
  else if (methodName == "coliny_ea") {
    if (!numContinuousVars && !num_ddrv) {
      Cerr << "\nError: " << methodName << " requires active continuous or "
	   << "active discrete range variables." << std::endl;
      abort_handler(-1);
    }
  }
  else if (!numContinuousVars) {
    Cerr << "\nError: " << methodName << " requires active continuous "
	 << "variables." << std::endl;
    abort_handler(-1);
  }

  // check for discrete variable support
  if (num_ddrv && (methodName != "moga" && methodName != "soga" &&
		   methodName != "coliny_ea") )
    Cerr << "\nWarning: discrete design range variables ignored by "
	 << methodName << std::endl;
  if ( (num_ddsiv || num_ddsrv) && (methodName != "moga" &&
				    methodName != "soga") )
    Cerr << "\nWarning: discrete design set variables ignored by "
	 << methodName << std::endl;

  // Check for linear constraint support in method selection
  if ( ( numLinearIneqConstraints || numLinearEqConstraints )       &&
       !methodName.begins("surrogate_based_")                       &&
       !methodName.ends("_sqp")      && !methodName.begins("dot_")  &&
       !methodName.begins("conmin_") && !methodName.ends("_newton") &&
        methodName != "moga"         &&  methodName != "soga"       &&
        methodName != "asynch_pattern_search" ) {
    Cerr << "\nError: linear constraints not currently supported by "
	 << methodName << ".\n       Please select a different method for "
         << "generally constrained problems." << std::endl;
    abort_handler(-1);
  }

  // Check for nonlinear constraint support in method selection.  Note that
  // CONMIN and DOT swap method selections as needed for constraint support.
  if ( ( numNonlinearIneqConstraints || numNonlinearEqConstraints )       &&
       !methodName.begins("surrogate_based_")                             &&
       !methodName.ends("_sqp")      && !methodName.begins("dot_")        &&
       !methodName.begins("conmin_") && !methodName.ends("_newton")       &&
        methodName != "moga"         &&  methodName != "soga"             &&
	methodName != "dl_solver"    &&  methodName != "efficient_global" &&
       !methodName.begins("coliny_") &&  methodName != "asynch_pattern_search"){
    Cerr << "\nError: nonlinear constraints not currently supported by "
	 << methodName << ".\n       Please select a different method for "
	 << "generally constrained problems." << std::endl;
    abort_handler(-1);
  }

  // check for gradient/Hessian/optimizer match: abort with an error for cases
  // where insufficient derivative data is available (e.g., full Newton methods
  // require Hessians), but only echo warnings in other cases (e.g., if more
  // derivative data is specified than is needed --> for example, don't enforce
  // that analytic Hessians require full Newton methods).
  if ( gradientType == "none" &&
       ( methodName.ends("_sqp")      || methodName.begins("dot_") || 
	 methodName.begins("conmin_") || methodName == "optpp_cg"  || 
	 methodName.ends("_newton")   || methodName == "nl2sol"	      ) ) {
    Cerr << "\nError: gradient-based optimizers require a gradient "
         << "specification." << std::endl;
    abort_handler(-1);
  }
  if ( hessianType == "none" && methodName == "optpp_newton" ) {
    Cerr << "\nError: full Newton methods require a Hessian specification."
	 << std::endl;
    abort_handler(-1);
  }
  if ( hessianType != "none" && methodName != "optpp_newton" )
    Cerr << "\nWarning: Hessians are only utilized by full Newton methods.\n\n";
  if ( ( gradientType != "none"     || hessianType != "none")      &&
       ( methodName == "optpp_pds"  || methodName.begins("coliny_") || 
         methodName.begins("ncsu_") || methodName == "asynch_pattern_search" ||
	 methodName == "moga"       || methodName == "soga" ) )
    Cerr << "\nWarning: Gradient/Hessian specification for a nongradient-based "
	 << "optimizer is ignored.\n\n";

  // set boundConstraintFlag
  size_t i;
  const RealVector& c_l_bnds = model.continuous_lower_bounds();
  const RealVector& c_u_bnds = model.continuous_upper_bounds();
  for (i=0; i<numContinuousVars; i++)
    if (c_l_bnds[i] > -bigRealBoundSize || c_u_bnds[i] < bigRealBoundSize) {
      boundConstraintFlag = true;
      break;
    }
  bool discrete_bounds = (methodName == "moga" || methodName == "soga" ||
			  methodName == "coliny_ea");
  if (discrete_bounds) {
    const IntVector&  di_l_bnds = model.discrete_int_lower_bounds();
    const IntVector&  di_u_bnds = model.discrete_int_upper_bounds();
    const RealVector& dr_l_bnds = model.discrete_real_lower_bounds();
    const RealVector& dr_u_bnds = model.discrete_real_upper_bounds();
    for (i=0; i<numDiscreteIntVars; i++)
      if (di_l_bnds[i] > -bigIntBoundSize || di_u_bnds[i] < bigIntBoundSize) {
	boundConstraintFlag = true;  
	break;
      }
    for (i=0; i<numDiscreteRealVars; i++)
      if (dr_l_bnds[i] > -bigRealBoundSize || dr_u_bnds[i] < bigRealBoundSize) {
	boundConstraintFlag = true;  
	break;
      }
  }

  // Convenience flag for gradient-based optimization/least squares
  vendorNumericalGradFlag = 
    (gradientType == "numerical" && methodSource == "vendor") ? true : false;

  Response best_resp = model.current_response().copy();
  // Set bestResponse ASV to vector of 1's since only fnValues are used.
  // NOTE: bestResponse initialization above may allocate space for gradients
  // or Hessians (which wastes space in current usage).
  ShortArray asv(numFunctions, 1);
  best_resp.active_set_request_vector(asv);
  bestResponseArray.push_back(best_resp);
}


Minimizer::Minimizer(NoDBBaseConstructor, Model& model):
  Iterator(NoDBBaseConstructor(), model),
  constraintTol(0.), bigRealBoundSize(1.e+30), bigIntBoundSize(1000000000),
  numNonlinearIneqConstraints(model.num_nonlinear_ineq_constraints()),
  numNonlinearEqConstraints(model.num_nonlinear_eq_constraints()),
  numLinearIneqConstraints(model.num_linear_ineq_constraints()),
  numLinearEqConstraints(model.num_linear_eq_constraints()),
  numNonlinearConstraints(numNonlinearIneqConstraints +
			  numNonlinearEqConstraints),
  numLinearConstraints(numLinearIneqConstraints + numLinearEqConstraints),
  numConstraints(numNonlinearConstraints + numLinearConstraints),
  boundConstraintFlag(false), speculativeFlag(false), scaleFlag(false),
  varsScaleFlag(false), primaryRespScaleFlag(false),
  secondaryRespScaleFlag(false)
{
  iteratedModel = model;

  // set boundConstraintFlag
  size_t i;
  const RealVector&  c_l_bnds = model.continuous_lower_bounds();
  const RealVector&  c_u_bnds = model.continuous_upper_bounds();
  for (i=0; i<numContinuousVars; i++)
    if (c_l_bnds[i] > -bigRealBoundSize || c_u_bnds[i] < bigRealBoundSize) {
      boundConstraintFlag = true;
      break;
    }
  bool discrete_bounds = (methodName == "moga" || methodName == "soga" ||
			  methodName == "coliny_ea");
  if (discrete_bounds) {
    const IntVector&  di_l_bnds = model.discrete_int_lower_bounds();
    const IntVector&  di_u_bnds = model.discrete_int_upper_bounds();
    const RealVector& dr_l_bnds = model.discrete_real_lower_bounds();
    const RealVector& dr_u_bnds = model.discrete_real_upper_bounds();
    for (i=0; i<numDiscreteIntVars; i++)
      if (di_l_bnds[i] > -bigIntBoundSize || di_u_bnds[i] < bigIntBoundSize) {
	boundConstraintFlag = true;  
	break;
      }
    for (i=0; i<numDiscreteRealVars; i++)
      if (dr_l_bnds[i] > -bigRealBoundSize || dr_u_bnds[i] < bigRealBoundSize) {
	boundConstraintFlag = true;  
	break;
      }
  }

  // Convenience flag for gradient-based optimization/least squares
  vendorNumericalGradFlag = 
    (gradientType == "numerical" && methodSource == "vendor") ? true : false;

  Response best_resp = model.current_response().copy();
  // Set bestResponse ASV to vector of 1's since only fnValues are used.
  // NOTE: bestResponse initialization above may allocate space for gradients
  // or Hessians (which wastes space in current usage).
  ShortArray asv(numFunctions, 1);
  best_resp.active_set_request_vector(asv);
  bestResponseArray.push_back(best_resp);
}


Minimizer::
Minimizer(NoDBBaseConstructor, size_t num_lin_ineq, size_t num_lin_eq,
	  size_t num_nln_ineq, size_t num_nln_eq):
  Iterator(NoDBBaseConstructor()), bigRealBoundSize(1.e+30),
  bigIntBoundSize(1000000000), numNonlinearIneqConstraints(num_nln_ineq),
  numNonlinearEqConstraints(num_nln_eq), numLinearIneqConstraints(num_lin_ineq),
  numLinearEqConstraints(num_lin_eq),
  numNonlinearConstraints(num_nln_ineq + num_nln_eq),
  numLinearConstraints(num_lin_ineq + num_lin_eq),
  numConstraints(numNonlinearConstraints + numLinearConstraints),
  boundConstraintFlag(false), speculativeFlag(false), scaleFlag(false),
  varsScaleFlag(false), primaryRespScaleFlag(false),
  secondaryRespScaleFlag(false)
{ }


void Minimizer::initialize_run()
{
  Iterator::initialize_run();

  // Track any previous object instance in case of recursion.  Note that
  // optimizerInstance and minimizerInstance must be tracked separately since
  // the previous optimizer and previous minimizer could be different instances
  // (e.g., for MCUU with NL2SOL for NLS and NPSOL for MPP search, the previous
  // minimizer is NL2SOL and the previous optimizer is NULL).
  prevMinInstance   = minimizerInstance;
  minimizerInstance = this;

  if (subIteratorFlag) { // catch any updates to all inactive vars
    bestVariablesArray.front().all_continuous_variables(
      iteratedModel.all_continuous_variables());
    bestVariablesArray.front().all_discrete_int_variables(
      iteratedModel.all_discrete_int_variables());
    bestVariablesArray.front().all_discrete_real_variables(
      iteratedModel.all_discrete_real_variables());
  }
}


/** helper function used in constructors of derived classes to set up
    scaling types, multipliers and offsets when input scaling flag is
    enabled */
void Minimizer::initialize_scaling()
{
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "\nMinimizer: Scaling enabled (any scaling derived from 'auto' " 
	 << "reported as value)" << std::endl;
  else if (outputLevel == NORMAL_OUTPUT)
    Cout << "\nMinimizer: Scaling enabled" << std::endl;

  // in the scaled case, perform numerical derivatives at the RecastModel level
  // (override the RecastModel default and the subModel default)
  iteratedModel.supports_estimated_derivatives(true);
  RecastModel* recast_model_rep = (RecastModel*) iteratedModel.model_rep();
  recast_model_rep->submodel_supports_estimated_derivatives(false);

  // temporary arrays
  IntArray    tmp_types;
  RealVector  tmp_multipliers, tmp_offsets;
  RealVector lbs, ubs, targets;
  //RealMatrix linear_constraint_coeffs;

  // need to access the subModel of the RecastModel for bounds/constraints
  const Model& sub_model = iteratedModel.subordinate_model();

  // NOTE: When retrieving scaling vectors from database, excepting linear 
  //       constraints, they've already been checked at input to have length 0, 
  //       1, or number of vars, constraints, etc.

  // -----------------
  // CONTINUOUS DESIGN
  // -----------------
  const StringArray& cdv_scale_strings
    = probDescDB.get_dsa("variables.continuous_design.scale_types");
  const RealVector& cdv_scales
    = probDescDB.get_rdv("variables.continuous_design.scales");
  varsScaleFlag = !cdv_scale_strings.empty();

  copy_data(sub_model.continuous_lower_bounds(), lbs); // view->copy
  copy_data(sub_model.continuous_upper_bounds(), ubs); // view->copy

  compute_scaling(CDV, BOUNDS, numContinuousVars, lbs, ubs, targets,
		  cdv_scale_strings, cdv_scales, cvScaleTypes,
		  cvScaleMultipliers, cvScaleOffsets);

  iteratedModel.continuous_lower_bounds(lbs);
  iteratedModel.continuous_upper_bounds(ubs);
  iteratedModel.continuous_variables(
    modify_n2s(sub_model.continuous_variables(), cvScaleTypes,
	       cvScaleMultipliers, cvScaleOffsets) );

  if (outputLevel > NORMAL_OUTPUT && varsScaleFlag) {
    StringArray cv_labels;
    copy_data(iteratedModel.continuous_variable_labels(), cv_labels);
    print_scaling("Continuous design variable scales", cvScaleTypes,
		  cvScaleMultipliers, cvScaleOffsets, cv_labels);
  }

  // each responseScale* = [fnScale*, nonlinearIneqScale*, nonlinearEqScale*]
  // to make transformations faster at run time 
  // numFunctions should reflect size of user-space model
  responseScaleTypes.resize(numFunctions);
  responseScaleMultipliers.resize(numFunctions);
  responseScaleOffsets.resize(numFunctions);

  // -------------------------
  // OBJECTIVE FNS / LSQ TERMS
  // -------------------------
  const StringArray& primary_scale_strings
    = probDescDB.get_dsa("responses.primary_response_fn_scale_types");
  const RealVector& primary_scales
    = probDescDB.get_rdv("responses.primary_response_fn_scales");
  primaryRespScaleFlag = !primary_scale_strings.empty();

  lbs.size(0); ubs.size(0);
  compute_scaling(FN_LSQ, DISALLOW, numUserPrimaryFns, lbs, ubs, targets,
		  primary_scale_strings, primary_scales, tmp_types,
		  tmp_multipliers, tmp_offsets);

  for (int i=0; i<numUserPrimaryFns; i++) {
    responseScaleTypes[i]       = tmp_types[i];
    responseScaleMultipliers[i] = tmp_multipliers[i];
    responseScaleOffsets[i]     = 0;
  }

  // --------------------
  // NONLINEAR INEQUALITY
  // --------------------
  const StringArray& nln_ineq_scale_strings =
    probDescDB.get_dsa("responses.nonlinear_inequality_scale_types");
  const RealVector& nln_ineq_scales
    = probDescDB.get_rdv("responses.nonlinear_inequality_scales"); 
  secondaryRespScaleFlag = !nln_ineq_scale_strings.empty();

  lbs = sub_model.nonlinear_ineq_constraint_lower_bounds();
  ubs = sub_model.nonlinear_ineq_constraint_upper_bounds();

  compute_scaling(NONLIN, BOUNDS, numNonlinearIneqConstraints, lbs, ubs,
		  targets, nln_ineq_scale_strings, nln_ineq_scales, tmp_types,
		  tmp_multipliers, tmp_offsets);

  for (int i=0; i<numNonlinearIneqConstraints; i++) {
    responseScaleTypes[numUserPrimaryFns+i]       = tmp_types[i];
    responseScaleMultipliers[numUserPrimaryFns+i] = tmp_multipliers[i];
    responseScaleOffsets[numUserPrimaryFns+i]     = tmp_offsets[i];
  }

  iteratedModel.nonlinear_ineq_constraint_lower_bounds(lbs);
  iteratedModel.nonlinear_ineq_constraint_upper_bounds(ubs);

  // --------------------
  // NONLINEAR EQUALITY
  // --------------------
  const StringArray& nln_eq_scale_strings
    = probDescDB.get_dsa("responses.nonlinear_equality_scale_types");
  const RealVector& nln_eq_scales
    = probDescDB.get_rdv("responses.nonlinear_equality_scales");
  secondaryRespScaleFlag
    = (secondaryRespScaleFlag || !nln_eq_scale_strings.empty());

  lbs.size(0); ubs.size(0);
  targets = sub_model.nonlinear_eq_constraint_targets();
  compute_scaling(NONLIN, TARGET, numNonlinearEqConstraints,
		  lbs, ubs, targets, nln_eq_scale_strings, nln_eq_scales,
		  tmp_types, tmp_multipliers, tmp_offsets);

  for (int i=0; i<numNonlinearEqConstraints; i++) {
    responseScaleTypes[numUserPrimaryFns+numNonlinearIneqConstraints+i] 
      = tmp_types[i];
    responseScaleMultipliers[numUserPrimaryFns+numNonlinearIneqConstraints+i] 
      = tmp_multipliers[i];
    responseScaleOffsets[numUserPrimaryFns+numNonlinearIneqConstraints+i] 
      = tmp_offsets[i];
  }

  iteratedModel.nonlinear_eq_constraint_targets(targets);

  if (outputLevel > NORMAL_OUTPUT && 
      (primaryRespScaleFlag || secondaryRespScaleFlag) )
    print_scaling("Response scales", responseScaleTypes,
		  responseScaleMultipliers, responseScaleOffsets,
		  sub_model.response_labels());

  // LINEAR CONSTRAINT SCALING:
  // computed scales account for scaling in CVs x
  // updating constraint coefficient matrices now handled in derived classes
  // NOTE: cannot use offsets since would be an affine scale
  // ScaleOffsets in this case are only for applying to bounds

  // -----------------
  // LINEAR INEQUALITY
  // -----------------
  const StringArray& lin_ineq_scale_strings
    = probDescDB.get_dsa("method.linear_inequality_scale_types");
  const RealVector& lin_ineq_scales
    = probDescDB.get_rdv("method.linear_inequality_scales");

  if ( ( lin_ineq_scale_strings.size() != 0 &&
	 lin_ineq_scale_strings.size() != 1 && 
	 lin_ineq_scale_strings.size() != numLinearIneqConstraints  ) ||
       ( lin_ineq_scales.length() != 0 && lin_ineq_scales.length() != 1 && 
	 lin_ineq_scales.length() != numLinearIneqConstraints ) ) {
    Cerr << "Error: linear_inequality_scale specifications must have length 0, "
	 << "1, or " << numLinearIneqConstraints << ".\n";
    abort_handler(-1);
  }

  linearIneqScaleOffsets.resize(numLinearIneqConstraints);

  lbs = sub_model.linear_ineq_constraint_lower_bounds();
  ubs = sub_model.linear_ineq_constraint_upper_bounds();
  targets.size(0);

  const RealMatrix& lin_ineq_coeffs
    = sub_model.linear_ineq_constraint_coeffs();
  for (int i=0; i<numLinearIneqConstraints; ++i) {

    // compute A_i*cvScaleOffset for current constraint -- discrete variables
    // aren't scaled so don't contribute
    linearIneqScaleOffsets[i] = 0.0;
    for (int j=0; j<numContinuousVars; j++)
      linearIneqScaleOffsets[i] += lin_ineq_coeffs(i,j)*cvScaleOffsets[j];
    
    lbs[i] -= linearIneqScaleOffsets[i];
    ubs[i] -= linearIneqScaleOffsets[i];

  }
  compute_scaling(LINEAR, BOUNDS, numLinearIneqConstraints,
		  lbs, ubs, targets, lin_ineq_scale_strings, lin_ineq_scales,
		  linearIneqScaleTypes, linearIneqScaleMultipliers, 
		  tmp_offsets);

  iteratedModel.linear_ineq_constraint_lower_bounds(lbs);
  iteratedModel.linear_ineq_constraint_upper_bounds(ubs);
  iteratedModel.linear_ineq_constraint_coeffs(
    lin_coeffs_modify_n2s(lin_ineq_coeffs, cvScaleMultipliers, 
			  linearIneqScaleMultipliers) );

  if (outputLevel > NORMAL_OUTPUT && numLinearIneqConstraints > 0)
    print_scaling("Linear inequality scales (incl. any variable scaling)",
		  linearIneqScaleTypes, linearIneqScaleMultipliers,
		  linearIneqScaleOffsets, StringArray());

  // ---------------
  // LINEAR EQUALITY
  // ---------------
  const StringArray& lin_eq_scale_strings
    = probDescDB.get_dsa("method.linear_equality_scale_types");
  const RealVector& lin_eq_scales
    = probDescDB.get_rdv("method.linear_equality_scales");

  if ( ( lin_eq_scale_strings.size() != 0 && lin_eq_scale_strings.size() != 1 &&
	 lin_eq_scale_strings.size() != numLinearEqConstraints ) ||
       ( lin_eq_scales.length() != 0 && lin_eq_scales.length() != 1 && 
	 lin_eq_scales.length() != numLinearEqConstraints ) ) {
    Cerr << "Error: linear_equality_scale specifications must have length 0, "
	 << "1, or " << numLinearEqConstraints << ".\n";
    abort_handler(-1);
  }

  linearEqScaleOffsets.resize(numLinearEqConstraints);

  lbs.size(0); ubs.size(0);
  targets = sub_model.linear_eq_constraint_targets();

  const RealMatrix& lin_eq_coeffs
    = sub_model.linear_eq_constraint_coeffs();
  for (int i=0; i<numLinearEqConstraints; ++i) {
    // compute A_i*cvScaleOffset for current constraint
    linearEqScaleOffsets[i] = 0.0;
    for (int j=0; j<numContinuousVars; j++)
      linearEqScaleOffsets[i] += lin_eq_coeffs(i,j)*cvScaleOffsets[j];
   
    targets[i] -= linearEqScaleOffsets[i];
  }
  compute_scaling(LINEAR, TARGET, numLinearEqConstraints,
		  lbs, ubs, targets, lin_eq_scale_strings, lin_eq_scales,
		  linearEqScaleTypes, linearEqScaleMultipliers, 
		  tmp_offsets);

  iteratedModel.linear_eq_constraint_targets(targets);
  iteratedModel.linear_eq_constraint_coeffs(
    lin_coeffs_modify_n2s(lin_eq_coeffs, cvScaleMultipliers, 
			  linearEqScaleMultipliers) );

  if (outputLevel > NORMAL_OUTPUT && numLinearEqConstraints > 0)
    print_scaling("Linear equality scales (incl. any variable scaling)",
		  linearEqScaleTypes, linearEqScaleMultipliers,
		  linearEqScaleOffsets, StringArray());

  if (outputLevel > NORMAL_OUTPUT)
    Cout << std::endl;
}


// compute_scaling will potentially modify lbs, ubs, and targets; will resize
// and set class data referenced by scale_types, scale_mults, and scale_offsets
void Minimizer::
compute_scaling(int object_type, // type of object being scaled 
		int auto_type,   // option for auto scaling type
		int num_vars,    // length of object being scaled
		RealVector& lbs,     RealVector& ubs,
		RealVector& targets, const StringArray& scale_strings,
		const RealVector& scales, IntArray& scale_types,
		RealVector& scale_mults,  RealVector& scale_offsets)
{
  // temporary arrays
  String tmp_scl_str; 
  Real tmp_bound, tmp_mult, tmp_offset;
  //RealMatrix linear_constraint_coeffs;

  const int num_scale_strings = scale_strings.size();
  const int num_scales        = scales.length();
  
  scale_types.resize(num_vars);
  scale_mults.resize(num_vars);
  scale_offsets.resize(num_vars);

  for (int i=0; i<num_vars; i++) {

    //set defaults
    scale_types[i]   = SCALE_NONE;
    scale_mults[i]   = 1.0;
    scale_offsets[i] = 0.0;

    // set the string for scale_type, depending on whether user sent
    // 0, 1, or N scale_strings
    tmp_scl_str = "none";
    if (num_scale_strings == 1)
      tmp_scl_str = scale_strings[0];
    else if (num_scale_strings > 1)
      tmp_scl_str = scale_strings[i];

    if (tmp_scl_str != "none") {

      if (  object_type == CDV && numLinearConstraints > 0 &&
	    tmp_scl_str == "log" ) {
	Cerr << "Error: Continuous design variables cannot be logarithmically "
	     << "scaled when linear\nconstraints are present.\n";
	abort_handler(-1);
      } 
      else if ( num_scales > 0 ) {
	
	// process scale_values for all types of scaling 
	// indicate that scale values are active, update bounds, poss. negating
	scale_types[i] |= SCALE_VALUE;
	scale_mults[i] = (num_scales == 1) ? scales[0] : scales[i];
	if (std::fabs(scale_mults[i]) < SCALING_MIN_SCALE)
	  Cout << "Warning: abs(scale) < " << SCALING_MIN_SCALE
	       << " provided; carefully verify results.\n";
	// adjust bounds or targets
	if (!lbs.empty()) {
	  // don't scale bounds if the user intended no bound
	  if (-bigRealBoundSize < lbs[i])
	    lbs[i] /= scale_mults[i];
	  if (ubs[i] < bigRealBoundSize)
	    ubs[i] /= scale_mults[i];
	  if (scale_mults[i] < 0) {
	    tmp_bound = lbs[i];
	    lbs[i] = ubs[i];
	    ubs[i] = tmp_bound;
	  }
	} 
	else if (!targets.empty())
	  targets[i] /= scale_mults[i];
      }

    } // endif for generic scaling preprocessing

    // At this point bounds/targets are scaled with user-provided values and
    // scale_mults are set to user-provided values.
    // Now auto or log scale as relevant and allowed:
    if ( tmp_scl_str == "auto" && auto_type > DISALLOW ) {
      bool scale_flag = false; // will be true for valid auto-scaling
      if ( auto_type == TARGET ) {
	scale_flag = compute_scale_factor(targets[i], &tmp_mult);
	tmp_offset = 0.0;
      }
      else if (auto_type == BOUNDS )
	scale_flag = compute_scale_factor(lbs[i], ubs[i], 
					  &tmp_mult, &tmp_offset);
      if (scale_flag) {

	scale_types[i] |= SCALE_VALUE;
	// tmp_offset was calculated based on scaled bounds, so
	// includes the effect of user scale values, so in computing
	// the offset, need to include the effect of any user-supplied
	// characteristic value scaling, then update multipliers
	scale_offsets[i] += tmp_offset*scale_mults[i];
	scale_mults[i] *= tmp_mult;  
      
	// necessary since the initial values may have already been value scaled
	if (auto_type == BOUNDS) {
	  // don't scale bounds if the user intended no bound
	  if (-bigRealBoundSize < lbs[i])
	    lbs[i] = (lbs[i] - tmp_offset)/tmp_mult;
	  if (ubs[i] < bigRealBoundSize)
	    ubs[i] = (ubs[i] - tmp_offset)/tmp_mult;
	}
	else if (auto_type == TARGET)
	  targets[i] /= tmp_mult;

      }
    }
    else if ( tmp_scl_str == "log" ) {

      scale_types[i] |= SCALE_LOG;
      if (auto_type == BOUNDS) {
	if (-bigRealBoundSize < lbs[i]) {
	  if ( lbs[i] < SCALING_MIN_LOG )
	    Cout << "Warning: scale_type 'log' used without positive lower "
		 << "bound.\n";
	  lbs[i] = log(lbs[i])/SCALING_LN_LOGBASE;
	}
	if (ubs[i] < bigRealBoundSize) {
	  if ( ubs[i] < SCALING_MIN_LOG )
	    Cout << "Warning: scale_type 'log' used without positive upper "
		 << "bound.\n";
	  ubs[i] = log(ubs[i])/SCALING_LN_LOGBASE;
	}
      }
      else if (auto_type == TARGET) {
	targets[i] = log(targets[i])/SCALING_LN_LOGBASE;
	if ( targets[i] < SCALING_MIN_LOG )
	  Cout << "Warning: scale_type 'log' used without positive target.\n";
      }
    }

  } // end for each variable
}


/** Variables map from iterator/scaled space to user/native space
    using a RecastModel. */
void Minimizer::
variables_recast(const Variables& scaled_vars, Variables& native_vars)
{
  if (minimizerInstance->outputLevel > NORMAL_OUTPUT) {
    Cout << "\n----------------------------------";
    Cout << "\nPre-processing Function Evaluation";
    Cout << "\n----------------------------------";
    Cout << "\nVariables before scaling transformation:\n";
    write_data(Cout, scaled_vars.continuous_variables(),
      scaled_vars.continuous_variable_labels());
    Cout << std::endl;
  }
  native_vars.continuous_variables(minimizerInstance->modify_s2n(
    scaled_vars.continuous_variables(),    minimizerInstance->cvScaleTypes,
    minimizerInstance->cvScaleMultipliers, minimizerInstance->cvScaleOffsets));
}


/** Constraint function map from user/native space to iterator/scaled/combined
    space using a RecastModel. */
void Minimizer::
secondary_resp_recast(const Variables& native_vars,
		      const Variables& scaled_vars,
		      const Response& native_response,
		      Response& iterator_response)
{
  // need to scale if secondary responses are scaled or (variables are
  // scaled and grad or hess requested)
  bool scale_transform_needed = minimizerInstance->secondaryRespScaleFlag || 
    minimizerInstance->need_resp_trans_byvars(
     native_response.active_set_request_vector(),
     minimizerInstance->numUserPrimaryFns, 
     minimizerInstance->numNonlinearConstraints);

  if (scale_transform_needed)
    minimizerInstance->response_modify_n2s(native_vars, native_response, 
      iterator_response, minimizerInstance->numUserPrimaryFns, 
      minimizerInstance->numIterPrimaryFns,
      minimizerInstance->numNonlinearConstraints);
  else
    iterator_response.update_partial(minimizerInstance->numIterPrimaryFns,
      minimizerInstance->numNonlinearConstraints, native_response,
      minimizerInstance->numUserPrimaryFns);
}


/** Determine if variable transformations present and derivatives
    requested, which implies a response transformation is necessay */ 
bool Minimizer::
need_resp_trans_byvars(const ShortArray& asv, int start_index, int num_resp) 
{
  if (varsScaleFlag)
    for (size_t i=start_index; i<start_index+num_resp; i++)
      if (asv[i] & 2 || asv[i] & 4)
	return(true);
  
  return(false);
}

/** general RealVector mapping from native to scaled variables; loosely,
    in greatest generality:
    scaled_var = log( (native_var - offset) / multiplier ) */
RealVector Minimizer::
modify_n2s(const RealVector& native_vars, const IntArray& scale_types,
	   const RealVector& multipliers, const RealVector& offsets) const
{
  RealVector scaled_vars(native_vars.length(), false);
  for (int i=0; i<native_vars.length(); i++) {

    if (scale_types[i] & SCALE_VALUE)
      scaled_vars[i] = (native_vars[i] - offsets[i]) / multipliers[i];
    else 
      scaled_vars[i] = native_vars[i];

    if (scale_types[i] & SCALE_LOG)
      scaled_vars[i] = log(scaled_vars[i])/SCALING_LN_LOGBASE;

  }

  return(scaled_vars);
}

/** general RealVector mapping from scaled to native variables and/or vals;
    loosely, in greatest generality:
    scaled_var = (LOG_BASE^scaled_var) * multiplier + offset */
RealVector Minimizer::
modify_s2n(const RealVector& scaled_vars, const IntArray& scale_types,
	   const RealVector& multipliers, const RealVector& offsets) const
{
  RealVector native_vars(scaled_vars.length(), false);
  for (RealVector::ordinalType i=0; i<scaled_vars.length(); ++i) {

    if (scale_types[i] & SCALE_LOG)
      native_vars[i] = std::pow(SCALING_LOGBASE, scaled_vars[i]);
    else 
      native_vars[i] = scaled_vars[i];

    if (scale_types[i] & SCALE_VALUE)
      native_vars[i] = native_vars[i]*multipliers[i] + offsets[i];
    
  }

  return(native_vars);
}


/** scaling response mapping: modifies response from a model (user/native) for
    use in iterators (scaled) -- not including multi_objective_modify */
void Minimizer::response_modify_n2s(const Variables& native_vars,
				    const Response& native_response,
				    Response& recast_response,
				    int native_offset, int recast_offset,
				    int num_responses) const
{
  // (offsets are zero-based indices of the first response to transform)
  int i, j, k, ri;
  SizetMultiArray var_ids;
  RealVector cdv;

  // unroll the unscaled (native/user-space) response
  const ActiveSet&  set       = native_response.active_set();
  const ShortArray& asv       = native_response.active_set_request_vector();
  const SizetArray&  dvv      = native_response.active_set_derivative_vector();
  const size_t num_deriv_vars = dvv.size(); 
 
  const RealVector&      fn_vals     = native_response.function_values();
  const RealMatrix&      fn_grads    = native_response.function_gradients();
  const RealSymMatrixArray& fn_hessians = native_response.function_hessians();
  const StringArray&     new_labels  = recast_response.function_labels();

  if (dvv == native_vars.continuous_variable_ids()) {
    var_ids.resize(boost::extents[native_vars.cv()]);
    var_ids = native_vars.continuous_variable_ids();
    cdv = native_vars.continuous_variables(); // view OK
  }
  else if (dvv == native_vars.inactive_continuous_variable_ids()) {
    var_ids.resize(boost::extents[native_vars.icv()]);
    var_ids = native_vars.inactive_continuous_variable_ids();
    cdv = native_vars.inactive_continuous_variables(); // view OK
  }
  else { // general derivatives
    var_ids.resize(boost::extents[native_vars.acv()]);
    var_ids = native_vars.all_continuous_variable_ids();
    cdv = native_vars.all_continuous_variables(); // view OK
  }

  if (outputLevel > NORMAL_OUTPUT)
    if (recast_offset < numIterPrimaryFns)
      Cout << "Primary response scaling transformation:\n";
    else
      Cout << "Secondary response scaling transformation:\n";

  // scale functions and constraints
  // assumes Multipliers and Offsets are 1 and 0 when not in use
  // there's a tradeoff here between flops and logic simplicity
  // (responseScaleOffsets may be nonzero for constraints)
  // iterate over components of ASV-requested functions and scale when necessary
  Real tmp_fn;
  for (ri=0, i=native_offset; ri<num_responses; ri++,i++)
    if (asv[i] & 1) {
      // SCALE_LOG case here includes case of SCALE_LOG && SCALE_VALUE
      if (responseScaleTypes[i] & SCALE_LOG)
	tmp_fn = log( (fn_vals[i] - responseScaleOffsets[i]) / 
			      responseScaleMultipliers[i] )/SCALING_LN_LOGBASE; 
      else if (responseScaleTypes[i] & SCALE_VALUE)
	tmp_fn = (fn_vals[i] - responseScaleOffsets[i]) / 
	  responseScaleMultipliers[i]; 
      else
	tmp_fn = fn_vals[i];
      recast_response.function_value(tmp_fn,recast_offset+ri);
      if (outputLevel > NORMAL_OUTPUT)
	Cout << "                     " << std::setw(write_precision+7) 
	     << tmp_fn << ' ' << new_labels[recast_offset+ri] << '\n';
    }

  // scale gradients
  for (ri=0, i=native_offset; ri<num_responses; ri++,i++)
    if (asv[i] & 2) {

      Real fn_divisor;
      if (responseScaleTypes[i] & SCALE_LOG)
	fn_divisor = (fn_vals[i] - responseScaleOffsets[i])*SCALING_LN_LOGBASE;
      else if (responseScaleTypes[i] & SCALE_VALUE)
	fn_divisor = responseScaleMultipliers[i];
      else
	fn_divisor = 1.0;

      RealVector tmp_grad( Teuchos::Copy, (Real*)fn_grads[i],
                                fn_grads.numRows() );
      for (j=0; j<num_deriv_vars; j++) {
	size_t xj_index = find_index(var_ids, dvv[j]);

	// first multiply by d(f_scaled)/d(f) based on scaling type
	tmp_grad[xj_index] /= fn_divisor; 

	// now multiply by d(x)/d(x_scaled)
	if (cvScaleTypes[xj_index] & SCALE_LOG)
	  tmp_grad[xj_index] *= 
	    (cdv[xj_index] - cvScaleOffsets[xj_index]) * SCALING_LN_LOGBASE;
	else if (cvScaleTypes[xj_index] & SCALE_VALUE)
	  tmp_grad[xj_index] *= cvScaleMultipliers[xj_index];

      }
      recast_response.function_gradient(tmp_grad,recast_offset+ri);
      if (outputLevel > NORMAL_OUTPUT) {
	write_col_vector_trans(Cout, recast_offset+ri,
          true, true, false, recast_response.function_gradients());
	Cout << new_labels[recast_offset+ri] << " gradient\n";
      }
    }
  
  // scale hessians
  for (ri=0, i=native_offset; ri<num_responses; ri++,i++)
    if (asv[i] & 4) {
      Real offset_fn = 1.0;
      if (responseScaleTypes[i] & SCALE_LOG)
	offset_fn = fn_vals[i] - responseScaleOffsets[i];
      RealSymMatrix tmp_hess = fn_hessians[i];
      for (j=0; j<num_deriv_vars; j++) {
	size_t xj_index = find_index(var_ids, dvv[j]);
	for (k=0; k<num_deriv_vars; k++) {
	  size_t xk_index = find_index(var_ids, dvv[k]);

	  // first multiply by d(f_scaled)/d(f) based on scaling type
	  if (responseScaleTypes[i] & SCALE_LOG) {

	    tmp_hess(xj_index,xk_index) -= 
	      fn_grads(xj_index,i)*fn_grads(xk_index,i) / offset_fn;

	    tmp_hess(xj_index,xk_index) /= offset_fn*SCALING_LN_LOGBASE;

	  }
	  else if (responseScaleTypes[i] & SCALE_VALUE)
	    tmp_hess(xj_index,xk_index) /= responseScaleMultipliers[i];

	  // now multiply by d(x)/d(x_scaled) for j,k
	  if (cvScaleTypes[xj_index] & SCALE_LOG)
	    tmp_hess(xj_index,xk_index) *= 
	      (cdv[xj_index] - cvScaleOffsets[xj_index]) * SCALING_LN_LOGBASE;
	  else if (cvScaleTypes[xj_index] & SCALE_VALUE)
	    tmp_hess(xj_index,xk_index) *= cvScaleMultipliers[xj_index];

	  if (cvScaleTypes[xk_index] & SCALE_LOG)
	    tmp_hess(xj_index,xk_index) *= 
	      (cdv[xk_index] - cvScaleOffsets[xk_index]) * SCALING_LN_LOGBASE;
	  else if (cvScaleTypes[xk_index] & SCALE_VALUE)
	    tmp_hess(xj_index,xk_index) *= cvScaleMultipliers[xk_index];

	  // need gradient term only for diagonal entries
	  if (xj_index == xk_index && cvScaleTypes[xj_index] & SCALE_LOG)
	    if (responseScaleTypes[i] & SCALE_LOG)
	      tmp_hess(xj_index,xk_index) += fn_grads(xj_index,i)*
		(cdv[xj_index] - cvScaleOffsets[xj_index]) *
		SCALING_LN_LOGBASE / offset_fn;
	    else
	      tmp_hess(xj_index,xk_index) += fn_grads(xj_index,i)*
		(cdv[xj_index] - cvScaleOffsets[xj_index]) * SCALING_LN_LOGBASE
		* SCALING_LN_LOGBASE / responseScaleMultipliers[i];

	}
      }
      recast_response.function_hessian(tmp_hess,recast_offset+ri);
      if (outputLevel > NORMAL_OUTPUT) {
	write_data(Cout, tmp_hess, true, true, false);
	Cout << new_labels[recast_offset+ri] << " Hessian\n";
      }
    }
  
  if (outputLevel > NORMAL_OUTPUT)
    Cout << std::endl;

}

/** scaling response mapping: modifies response from scaled (iterator) to 
    native (user) space -- not including multi_objective_retrieve */
void Minimizer::response_modify_s2n(const Variables& native_vars,
				    const Response& scaled_response,
				    Response& native_response,
				    int scaled_offset, int native_offset,
				    int num_responses) const
{
  using std::pow;

  // (offsets are zero-based indices of the first response to transform)
  int i, j, k, ri;
  SizetMultiArray var_ids;
  RealVector cdv;

  // unroll the unscaled (native/user-space) response
  const ActiveSet&  set       = scaled_response.active_set();
  const ShortArray& asv       = scaled_response.active_set_request_vector();
  const SizetArray&  dvv      = scaled_response.active_set_derivative_vector();
  const size_t num_deriv_vars = dvv.size(); 
 
  const RealVector&      fn_vals     = scaled_response.function_values();
  const RealMatrix&      fn_grads    = scaled_response.function_gradients();
  const RealSymMatrixArray& fn_hessians = scaled_response.function_hessians();
  const StringArray&     new_labels  = native_response.function_labels();

  if (dvv == native_vars.continuous_variable_ids()) {
    var_ids.resize(boost::extents[native_vars.cv()]);
    var_ids = native_vars.continuous_variable_ids();
    cdv = native_vars.continuous_variables(); // view OK
  }
  else if (dvv == native_vars.inactive_continuous_variable_ids()) {
    var_ids.resize(boost::extents[native_vars.icv()]);
    var_ids = native_vars.inactive_continuous_variable_ids();
    cdv = native_vars.inactive_continuous_variables(); // view OK
  }
  else {    // general derivatives
    var_ids.resize(boost::extents[native_vars.acv()]);
    var_ids = native_vars.all_continuous_variable_ids();
    cdv = native_vars.all_continuous_variables(); // view OK
  }

  if (outputLevel > NORMAL_OUTPUT)
    if (native_offset < numIterPrimaryFns)
      Cout << "Primary response unscaling transformation:\n";
    else
      Cout << "Secondary response unscaling transformation:\n";

  // scale functions and constraints
  // assumes Multipliers and Offsets are 1 and 0 when not in use
  // there's a tradeoff here between flops and logic simplicity
  // (responseScaleOffsets may be nonzero for constraints)
  // iterate over components of ASV-requested functions and scale when necessary
  Real tmp_fn;
  for (ri=0, i=scaled_offset; ri<num_responses; ri++,i++)
    if (asv[i] & 1) {
      // SCALE_LOG case here includes case of SCALE_LOG && SCALE_VALUE
      if (responseScaleTypes[i] & SCALE_LOG)
	tmp_fn = pow(SCALING_LOGBASE, fn_vals[i])*responseScaleMultipliers[i] + 
	  responseScaleOffsets[i];
 
      else if (responseScaleTypes[i] & SCALE_VALUE)
	tmp_fn = fn_vals[i]*responseScaleMultipliers[i] + 
	  responseScaleOffsets[i];
      else
	tmp_fn = fn_vals[i];
      native_response.function_value(tmp_fn,native_offset+ri);
      if (outputLevel > NORMAL_OUTPUT)
	Cout << "                     " << std::setw(write_precision+7) 
	     << tmp_fn << ' ' << new_labels[native_offset+ri] << '\n';
    }

  // scale gradients
  Real df_dfscl;
  for (ri=0, i=scaled_offset; ri<num_responses; ri++,i++)
    if (asv[i] & 2) {

      if (responseScaleTypes[i] & SCALE_LOG)
	df_dfscl = pow(SCALING_LOGBASE, fn_vals[i]) * SCALING_LN_LOGBASE *
	  responseScaleMultipliers[i];	 
      else if (responseScaleTypes[i] & SCALE_VALUE)
	df_dfscl = responseScaleMultipliers[i];
      else
	df_dfscl = 1.0;

      RealVector tmp_grad( Teuchos::Copy, (Real*)fn_grads[i],
                                fn_grads.numRows() );
      for (j=0; j<num_deriv_vars; j++) {
	size_t xj_index = find_index(var_ids, dvv[j]);

	// first multiply by d(f)/d(f_scaled) based on scaling type
	tmp_grad[xj_index] *= df_dfscl; 

	// now multiply by d(x_scaled)/d(x)
	if (cvScaleTypes[xj_index] & SCALE_LOG)
	  tmp_grad[xj_index] /= (cdv[xj_index] - cvScaleOffsets[xj_index]) *
	    SCALING_LN_LOGBASE;
	else if (cvScaleTypes[xj_index] & SCALE_VALUE)
	  tmp_grad[xj_index] /= cvScaleMultipliers[xj_index];

      }
      native_response.function_gradient(tmp_grad,native_offset+ri);
      if (outputLevel > NORMAL_OUTPUT) {
	write_col_vector_trans(Cout, native_offset+ri,
          true, true, false, native_response.function_gradients());
	Cout << new_labels[native_offset+ri] << " gradient\n";
      }
    }
  
  // scale hessians
  for (ri=0, i=scaled_offset; ri<num_responses; ri++,i++)
    if (asv[i] & 4) {
 
     if (responseScaleTypes[i] & SCALE_LOG)
	df_dfscl = pow(SCALING_LOGBASE, fn_vals[i]) * SCALING_LN_LOGBASE *
	  responseScaleMultipliers[i];
      else if (responseScaleTypes[i] & SCALE_VALUE)
	df_dfscl = responseScaleMultipliers[i];
      else
	df_dfscl = 1.0;

      RealSymMatrix tmp_hess = fn_hessians[i];
      for (j=0; j<num_deriv_vars; j++) {
	size_t xj_index = find_index(var_ids, dvv[j]);
	for (k=0; k<num_deriv_vars; k++) {
	  size_t xk_index = find_index(var_ids, dvv[k]);
	  
	  // first multiply by d(f_scaled)/d(f) based on scaling type

	  if (responseScaleTypes[i] & SCALE_LOG) {

	    tmp_hess(xj_index,xk_index) += 
	      fn_grads(xj_index,i)*fn_grads(xk_index,i) * SCALING_LN_LOGBASE; 

	    tmp_hess(xj_index,xk_index) *= df_dfscl; 

	  }
	  else if (responseScaleTypes[i] & SCALE_VALUE)
	    tmp_hess(xj_index,xk_index) *= df_dfscl;

	  // now multiply by d(x_scaled)/d(x) for j,k
	  if (cvScaleTypes[xj_index] & SCALE_LOG)
	    tmp_hess(xj_index,xk_index) /= 
	      (cdv[xj_index] - cvScaleOffsets[xj_index]) * SCALING_LN_LOGBASE;
	  else if (cvScaleTypes[xj_index] & SCALE_VALUE)
	    tmp_hess(xj_index,xk_index) /= cvScaleMultipliers[xj_index];

	  if (cvScaleTypes[xk_index] & SCALE_LOG)
	    tmp_hess(xj_index,xk_index) /= 
	      (cdv[xk_index] - cvScaleOffsets[xk_index]) * SCALING_LN_LOGBASE;
	  else if (cvScaleTypes[xk_index] & SCALE_VALUE)
	    tmp_hess(xj_index,xk_index) /= cvScaleMultipliers[xk_index];

	  // need gradient term only for diagonal entries
	  if (xj_index == xk_index && cvScaleTypes[xj_index] & SCALE_LOG)
	    tmp_hess(xj_index,xk_index) -= df_dfscl * fn_grads(xj_index,i) /
	      (cdv[xj_index] - cvScaleOffsets[xj_index]) * 
	      (cdv[xj_index] - cvScaleOffsets[xj_index]) * SCALING_LN_LOGBASE;

	}
      }
      native_response.function_hessian(tmp_hess,native_offset+ri);
      if (outputLevel > NORMAL_OUTPUT) {
	write_data(Cout, tmp_hess, true, true, false);
	Cout << new_labels[native_offset+ri] << " Hessian\n";
      }
    }
  
  if (outputLevel > NORMAL_OUTPUT)
    Cout << std::endl;

}



// automatically compute scaling factor
// bounds case allows for negative multipliers
// returns true if a valid scaling factor was computed
bool Minimizer::compute_scale_factor(const Real lower_bound,
				     const Real upper_bound,
				     Real *multiplier, Real *offset)
{
  /*  Compute scaleMultipliers for each design var, fn, constr, etc.
      1. user-specified scaling was already detected at higher level
      2. check for two-sided bounds
      3. then check for one-sided bounds
      4. else resort to no scaling

      Auto-scaling is to [0,1] (affine scaling) in two sided case 
      or value of bound in one-sided case
  */

  bool lb_flag = false;
  bool ub_flag = false;

  if (-bigRealBoundSize < lower_bound)
    lb_flag = true;
  if (upper_bound < bigRealBoundSize)
    ub_flag = true;

  // process two-sided, then single-sided bounds
  if ( lb_flag && ub_flag ) {
    *multiplier = upper_bound - lower_bound;
    *offset = lower_bound;
  } 
  else if (lb_flag) {
    *multiplier = lower_bound;
    *offset = 0.0;
  } 
  else if (ub_flag) {
    *multiplier = upper_bound;
    *offset = 0.0;
  } 
  else {
    Cout << "Warning: abs(bounds) > bigRealBoundSize. Not auto-scaling "
	 << "component." << std::endl;
    *multiplier = 1.0;
    *offset = 0.0;
    return(false);
  }

  if (std::fabs(*multiplier) < SCALING_MIN_SCALE) {
    *multiplier = (*multiplier >= 0.0) ? SCALING_MIN_SCALE :
      -(SCALING_MIN_SCALE); 
    Cout << "Warning: in auto-scaling abs(computed scale) < " 
	 << SCALING_MIN_SCALE << "; resetting scale = " 
	 << *multiplier << ".\n";
  }

  return(true);
}


// automatically compute scaling factor
// target case allows for negative multipliers
// returns true if a valid scaling factor was computed
bool Minimizer::compute_scale_factor(const Real target, Real *multiplier)
{
  if ( std::fabs(target) < bigRealBoundSize ) 
    *multiplier = target;
  else {
    Cout << "Automatic Scaling Warning: abs(target) > bigRealBoundSize. "
	 << "Not scaling this component." << std::endl;
    *multiplier = 1.0;
    return(false);
  }

  if (std::fabs(*multiplier) < SCALING_MIN_SCALE) {
    *multiplier = (*multiplier >= 0.0) ? SCALING_MIN_SCALE :
      -(SCALING_MIN_SCALE); 
    Cout << "Warning: in auto-scaling abs(computed scale) < " 
	 << SCALING_MIN_SCALE << "; resetting scale = " 
	 << *multiplier << ".\n";
  }

  return(true);
}


/** compute scaled linear constraint matrix given design variable 
    multipliers and linear scaling multipliers.  Only scales components 
    corresponding to continuous variables so for src_coeffs of size MxN, 
    lin_multipliers.size() <= M, cv_multipliers.size() <= N */
RealMatrix Minimizer::
lin_coeffs_modify_n2s(const RealMatrix& src_coeffs,
		      const RealVector& cv_multipliers,
		      const RealVector& lin_multipliers) const
{
  RealMatrix dest_coeffs(src_coeffs);
  for (int i=0; i<lin_multipliers.length(); i++)
    for (int j=0; j<cv_multipliers.length(); j++)
      dest_coeffs(i,j) *= cv_multipliers[j] / lin_multipliers[i];

  return(dest_coeffs);
}


void Minimizer::print_scaling(const String& info, const IntArray& scale_types,
			      const RealVector& scale_mults,
			      const RealVector& scale_offsets,
			      const StringArray& labels)
{

  // labels will be empty for linear constraints
  Cout << "\n" << info << ":\n";
  Cout << "scale type " << std::setw(write_precision+7) << "multiplier" << " "
       << std::setw(write_precision+7) << "offset"
       << (labels.empty() ? " constraint number" : " label") << std::endl; 
  for (size_t i=0; i<scale_types.size(); i++) {

    switch (scale_types[i]) {
    case SCALE_NONE: 
      Cout << "none       ";
      break;
    case SCALE_VALUE: 
      Cout << "value      ";
      break;
    case SCALE_LOG:
      Cout << "log        ";
      break;
    case (SCALE_VALUE | SCALE_LOG): 
      Cout << "value+log  ";
      break;
    }
    Cout << std::setw(write_precision+7) << scale_mults[i]   << " " 
	 << std::setw(write_precision+7) << scale_offsets[i] << " ";
    if (labels.empty())
      Cout << i << std::endl;
    else
      Cout << labels[i] << std::endl;

  }

}

} // namespace Dakota
