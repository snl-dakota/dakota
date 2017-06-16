/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Model
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "dakota_system_defs.hpp"
#include "DakotaModel.hpp"
#include "ParamResponsePair.hpp"
#include "PRPMultiIndex.hpp"
#include "ParallelLibrary.hpp"
#include "ProblemDescDB.hpp"
#include "SimulationModel.hpp"
#include "NestedModel.hpp"
#include "DataFitSurrModel.hpp"
#include "HierarchSurrModel.hpp"
#include "ActiveSubspaceModel.hpp"
#include "AdaptedBasisModel.hpp"
#include "RandomFieldModel.hpp"
#include "DakotaGraphics.hpp"
#include "pecos_stat_util.hpp"

//#define REFCOUNT_DEBUG

static const char rcsId[]="@(#) $Id: DakotaModel.cpp 7029 2010-10-22 00:17:02Z mseldre $";


namespace Dakota 
{
extern PRPCache        data_pairs;
extern ParallelLibrary dummy_lib;       // defined in dakota_global_defs.cpp
extern ProblemDescDB   dummy_db;        // defined in dakota_global_defs.cpp

// These globals defined here rather than in dakota_global_defs.cpp in order to
// minimize dakota_restart_util object file dependencies
Interface dummy_interface; ///< dummy Interface object used for mandatory
                           ///< reference initialization or default virtual
                           ///< function return by reference when a real
                           ///< Interface instance is unavailable
Model     dummy_model;     ///< dummy Model object used for mandatory reference
                           ///< initialization or default virtual function
                           ///< return by reference when a real Model instance
                           ///< is unavailable
Iterator  dummy_iterator;  ///< dummy Iterator object used for mandatory
                           ///< reference initialization or default virtual
                           ///< function return by reference when a real
                           ///< Iterator instance is unavailable


/** This constructor builds the base class data for all inherited
    models.  get_model() instantiates a derived class and the derived
    class selects this base class constructor in its initialization
    list (to avoid the recursion of the base class constructor calling
    get_model() again).  Since the letter IS the representation, its
    representation pointer is set to NULL (an uninitialized pointer
    causes problems in ~Model). */
Model::Model(BaseConstructor, ProblemDescDB& problem_db):
  currentVariables(problem_db.get_variables()),
  numDerivVars(currentVariables.cv()),
  currentResponse(
    problem_db.get_response(SIMULATION_RESPONSE, currentVariables)),
  numFns(currentResponse.num_functions()),
  userDefinedConstraints(problem_db, currentVariables.shared_data()),
  modelType(problem_db.get_string("model.type")),
  surrogateType(problem_db.get_string("model.surrogate.type")),
  gradientType(problem_db.get_string("responses.gradient_type")),
  methodSource(problem_db.get_string("responses.method_source")),
  intervalType(problem_db.get_string("responses.interval_type")),
  fdGradStepSize(problem_db.get_rv("responses.fd_gradient_step_size")),
  fdGradStepType(problem_db.get_string("responses.fd_gradient_step_type")),
  gradIdAnalytic(problem_db.get_is("responses.gradients.mixed.id_analytic")),
  gradIdNumerical(problem_db.get_is("responses.gradients.mixed.id_numerical")),
  hessianType(problem_db.get_string("responses.hessian_type")),
  quasiHessType(problem_db.get_string("responses.quasi_hessian_type")),
  fdHessByFnStepSize(problem_db.get_rv("responses.fd_hessian_step_size")),
  fdHessByGradStepSize(problem_db.get_rv("responses.fd_hessian_step_size")),
  fdHessStepType(problem_db.get_string("responses.fd_hessian_step_type")),
  hessIdAnalytic(problem_db.get_is("responses.hessians.mixed.id_analytic")),
  hessIdNumerical(problem_db.get_is("responses.hessians.mixed.id_numerical")),
  hessIdQuasi(problem_db.get_is("responses.hessians.mixed.id_quasi")),
  warmStartFlag(false), supportsEstimDerivs(true),
  probDescDB(problem_db), parallelLib(problem_db.parallel_library()),
  modelPCIter(parallelLib.parallel_configuration_iterator()),
  componentParallelMode(0), asynchEvalFlag(false), evaluationCapacity(1), 
  // See base constructor in DakotaIterator.cpp for full discussion of output
  // verbosity.  For models, QUIET_OUTPUT turns off response reporting and
  // SILENT_OUTPUT additionally turns off fd_gradient parameter set reporting.
  outputLevel(problem_db.get_short("method.output")),
  discreteDesignSetIntValues(
    problem_db.get_isa("variables.discrete_design_set_int.values")),
  discreteDesignSetStringValues(
    problem_db.get_ssa("variables.discrete_design_set_string.values")),
  discreteDesignSetRealValues(
    problem_db.get_rsa("variables.discrete_design_set_real.values")),
  discreteStateSetIntValues(
    problem_db.get_isa("variables.discrete_state_set_int.values")),
  discreteStateSetStringValues(
    problem_db.get_ssa("variables.discrete_state_set_string.values")),
  discreteStateSetRealValues(
    problem_db.get_rsa("variables.discrete_state_set_real.values")),
  aleatDistParams(problem_db.get_rv("variables.normal_uncertain.means"),
    problem_db.get_rv("variables.normal_uncertain.std_deviations"),
    problem_db.get_rv("variables.normal_uncertain.lower_bounds"),
    problem_db.get_rv("variables.normal_uncertain.upper_bounds"),
    problem_db.get_rv("variables.lognormal_uncertain.means"),
    problem_db.get_rv("variables.lognormal_uncertain.std_deviations"),
    problem_db.get_rv("variables.lognormal_uncertain.lambdas"),
    problem_db.get_rv("variables.lognormal_uncertain.zetas"),
    problem_db.get_rv("variables.lognormal_uncertain.error_factors"),
    problem_db.get_rv("variables.lognormal_uncertain.lower_bounds"),
    problem_db.get_rv("variables.lognormal_uncertain.upper_bounds"),
    problem_db.get_rv("variables.uniform_uncertain.lower_bounds"),
    problem_db.get_rv("variables.uniform_uncertain.upper_bounds"),
    problem_db.get_rv("variables.loguniform_uncertain.lower_bounds"),
    problem_db.get_rv("variables.loguniform_uncertain.upper_bounds"),
    problem_db.get_rv("variables.triangular_uncertain.modes"),
    problem_db.get_rv("variables.triangular_uncertain.lower_bounds"),
    problem_db.get_rv("variables.triangular_uncertain.upper_bounds"),
    problem_db.get_rv("variables.exponential_uncertain.betas"),
    problem_db.get_rv("variables.beta_uncertain.alphas"),
    problem_db.get_rv("variables.beta_uncertain.betas"),
    problem_db.get_rv("variables.beta_uncertain.lower_bounds"),
    problem_db.get_rv("variables.beta_uncertain.upper_bounds"),
    problem_db.get_rv("variables.gamma_uncertain.alphas"),
    problem_db.get_rv("variables.gamma_uncertain.betas"),
    problem_db.get_rv("variables.gumbel_uncertain.alphas"),
    problem_db.get_rv("variables.gumbel_uncertain.betas"),
    problem_db.get_rv("variables.frechet_uncertain.alphas"),
    problem_db.get_rv("variables.frechet_uncertain.betas"),
    problem_db.get_rv("variables.weibull_uncertain.alphas"),
    problem_db.get_rv("variables.weibull_uncertain.betas"),
    problem_db.get_rrma("variables.histogram_uncertain.bin_pairs"),
    problem_db.get_rv("variables.poisson_uncertain.lambdas"),
    problem_db.get_rv("variables.binomial_uncertain.prob_per_trial"),
    problem_db.get_iv("variables.binomial_uncertain.num_trials"),
    problem_db.get_rv("variables.negative_binomial_uncertain.prob_per_trial"),
    problem_db.get_iv("variables.negative_binomial_uncertain.num_trials"),
    problem_db.get_rv("variables.geometric_uncertain.prob_per_trial"),
    problem_db.get_iv("variables.hypergeometric_uncertain.total_population"),
    problem_db.get_iv(
      "variables.hypergeometric_uncertain.selected_population"),
    problem_db.get_iv("variables.hypergeometric_uncertain.num_drawn"),
    problem_db.get_irma("variables.histogram_uncertain.point_int_pairs"),
    problem_db.get_srma("variables.histogram_uncertain.point_string_pairs"),
    problem_db.get_rrma("variables.histogram_uncertain.point_real_pairs"),
    problem_db.get_rsm("variables.uncertain.correlation_matrix")),
  epistDistParams(
    problem_db.get_rrrma("variables.continuous_interval_uncertain.basic_probs"),
    problem_db.get_iirma("variables.discrete_interval_uncertain.basic_probs"),
    problem_db.get_irma("variables.discrete_uncertain_set_int.values_probs"),
    problem_db.get_srma("variables.discrete_uncertain_set_string.values_probs"),
    problem_db.get_rrma("variables.discrete_uncertain_set_real.values_probs")),
  primaryRespFnWts(probDescDB.get_rv("responses.primary_response_fn_weights")),
  hierarchicalTagging(probDescDB.get_bool("model.hierarchical_tags")),
  scalingOpts(probDescDB.get_sa("variables.continuous_design.scale_types"),
              probDescDB.get_rv("variables.continuous_design.scales"),
              probDescDB.get_sa("responses.primary_response_fn_scale_types"),
              probDescDB.get_rv("responses.primary_response_fn_scales"),
              probDescDB.get_sa("responses.nonlinear_inequality_scale_types"),
              probDescDB.get_rv("responses.nonlinear_inequality_scales"),
              probDescDB.get_sa("responses.nonlinear_equality_scale_types"),
              probDescDB.get_rv("responses.nonlinear_equality_scales"),
              probDescDB.get_sa("variables.linear_inequality_scale_types"),
              probDescDB.get_rv("variables.linear_inequality_scales"),
              probDescDB.get_sa("variables.linear_equality_scale_types"),
              probDescDB.get_rv("variables.linear_equality_scales")),
  modelId(problem_db.get_string("model.id")), modelEvalCntr(0),
  estDerivsFlag(false), initCommsBcastFlag(false),
  modelAutoGraphicsFlag(false), modelRep(NULL), referenceCount(1)
{
  // Define primaryRespFnSense BoolDeque from DB StringArray
  StringArray db_sense
    = problem_db.get_sa("responses.primary_response_fn_sense");
  if (!db_sense.empty()) {
    size_t i, num_sense = db_sense.size(),
      num_primary = currentResponse.num_functions()
                  - userDefinedConstraints.num_nonlinear_ineq_constraints()
                  - userDefinedConstraints.num_nonlinear_eq_constraints();
    primaryRespFnSense.resize(num_primary);
    if (num_sense == num_primary)
      for (i=0; i<num_primary; ++i)
	primaryRespFnSense[i] = strbegins(strtolower(db_sense[i]), "max");
    else if (num_sense == 1)
      primaryRespFnSense.assign(num_primary, 
				strbegins(strtolower(db_sense[0]), "max"));
    else {
      Cerr << "Error: wrong length in sense array.  Expected 0, 1, or "
	   << num_primary << " but saw " << num_sense << "." << std::endl;
      abort_handler(MODEL_ERROR);
    }
  }

  // Promote fdGradStepSize/fdHessByFnStepSize/fdHessByGradStepSize to defaults
  // if needed.  Note: the fdStepSize arrays specialize by variable, whereas
  // mixed grads/Hessians specialize by function.
  if ( gradientType == "numerical" ||
       ( gradientType == "mixed" && !gradIdNumerical.empty() ) ) {
    if (fdGradStepSize.empty()) {
      fdGradStepSize.resize(1);
      fdGradStepSize[0] = 0.001;
    }
  }
  if ( hessianType == "numerical" ||
       ( hessianType == "mixed" && !hessIdNumerical.empty() ) ) {
    // fdHessByFnStepSize and fdHessByGradStepSize can only differ currently
    // in the case of assignment of default values, since the same
    // fd_hessian_step_size input is reused for both first- and second-order
    // differencing.  If needed in the future (numerical Hessians with mixed
    // gradients require both first- and second-order step sizes), separate
    // inputs could be added and easily accomodated here.
    if (fdHessByFnStepSize.empty()) {
      fdHessByFnStepSize.resize(1);
      fdHessByFnStepSize[0] = 0.002;
    }
    if (fdHessByGradStepSize.empty()) {
      fdHessByGradStepSize.resize(1);
      fdHessByGradStepSize[0] = 0.001;
    }
  }

  /*
  // Populate gradient/Hessian attributes for use within the iterator hierarchy.
  // Note: the fd step size arrays specialize by variable, whereas the mixed
  // grads/Hessians specialize by function.
  if (outputLevel >= VERBOSE_OUTPUT)
    Cout << "gradientType = " << gradientType << '\n';
  if (gradientType == "numerical") {
    if (methodSource == "vendor") {
      const RealVector& fdgss
	= probDescDB.get_rv("responses.fd_gradient_step_size");
      if (fdgss.length()) // else use default from initializer list
	fdGradStepSize = fdgss[0];
    }
    if (outputLevel >= VERBOSE_OUTPUT)
      Cout << "Numerical gradients using " << intervalType
	   << " differences\nto be calculated by the " << methodSource
	   << " finite difference routine.\n";
  }
  else if (gradientType == "mixed" && outputLevel >= VERBOSE_OUTPUT) {
    // Vendor numerical is no good in mixed mode except maybe for NPSOL/NLSSOL
    if (methodSource == "vendor") {
      Cerr << "Error: Mixed gradient specification not currently valid with "
           << "vendor numerical.\nSelect dakota as method_source instead."
	   << std::endl;
      abort_handler(MODEL_ERROR);
    }
    Cout << "Mixed gradients: analytic gradients for functions { ";
    for (ILCIter cit=mixed_grad_analytic_ids.begin();
	 cit!=mixed_grad_analytic_ids.end(); cit++)
      Cout << *cit << ' ';
    Cout << "} and\nnumerical gradients for functions { ";
    for (ILCIter cit=mixed_grad_numerical_ids.begin();
	 cit!=mixed_grad_numerical_ids.end(); cit++)
      Cout << *cit << ' ';
    Cout << "} using " << intervalType << " differences\ncalculated by the "
	 << methodSource << " routine.\n";
  }
  Cout << "hessianType = " << hessianType << '\n';
  if ( hessianType == "numerical" || ( hessianType == "mixed" &&
      !probDescDB.get_is("responses.hessians.mixed.id_numerical").empty() ) ) {
    const RealVector& fdhss
      = probDescDB.get_rv("responses.fd_hessian_step_size");
    if (fdhss.length()) // else use defaults from initializer list
      fdHessByGradStepSize = fdHessByFnStepSize = fdhss[0];
  }
  */

#ifdef REFCOUNT_DEBUG
  Cout << "Model::Model(BaseConstructor, ProblemDescDB&) called "
       << "to build letter base class\n";
#endif
}


Model::
Model(LightWtBaseConstructor, ProblemDescDB& problem_db,
      ParallelLibrary& parallel_lib, const SharedVariablesData& svd,
      const SharedResponseData& srd, const ActiveSet& set, short output_level):
  currentVariables(svd), numDerivVars(set.derivative_vector().size()),
  currentResponse(srd, set), numFns(set.request_vector().size()),
  userDefinedConstraints(svd), fdGradStepType("relative"),
  fdHessStepType("relative"), warmStartFlag(false), supportsEstimDerivs(true),
  probDescDB(problem_db), parallelLib(parallel_lib),
  modelPCIter(parallel_lib.parallel_configuration_iterator()),
  componentParallelMode(0), asynchEvalFlag(false), evaluationCapacity(1),
  outputLevel(output_level), hierarchicalTagging(false),
  modelId("NO_SPECIFICATION"), modelEvalCntr(0), estDerivsFlag(false),
  initCommsBcastFlag(false), modelAutoGraphicsFlag(false),
  modelRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Model::Model(NoDBBaseConstructor, ParallelLibrary&, "
       << "SharedVariablesData&, ActiveSet&, short) called to build letter "
       << "base class\n";
#endif
}


/** This constructor also builds the base class data for inherited
    models.  However, it is used for recast models which are
    instantiated on the fly.  Therefore it only initializes a small
    subset of attributes. */
Model::
Model(LightWtBaseConstructor, ProblemDescDB& problem_db,
      ParallelLibrary& parallel_lib):
  warmStartFlag(false), supportsEstimDerivs(true),
  probDescDB(problem_db), parallelLib(parallel_lib),
  modelPCIter(parallel_lib.parallel_configuration_iterator()),
  componentParallelMode(0), asynchEvalFlag(false), evaluationCapacity(1),
  outputLevel(NORMAL_OUTPUT), hierarchicalTagging(false),
  modelId("NO_SPECIFICATION"), modelEvalCntr(0), estDerivsFlag(false),
  initCommsBcastFlag(false), modelAutoGraphicsFlag(false),
  modelRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Model::Model(LightWtBaseConstructor, ProblemDescDB&, "
       << "ParallelLibrary&) called to build letter base class\n";
#endif
}


/** The default constructor is used in vector<Model> instantiations
    and for initialization of Model objects contained in Iterator and
    derived Strategy classes.  modelRep is NULL in this case (a
    populated problem_db is needed to build a meaningful Model
    object).  This makes it necessary to check for NULL in the copy
    constructor, assignment operator, and destructor. */
Model::Model():
  modelRep(NULL), referenceCount(1), probDescDB(dummy_db),
  parallelLib(dummy_lib)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Model::Model(), modelRep = NULL" << std::endl;
#endif
}


/** Used in model instantiations within strategy constructors.
    Envelope constructor only needs to extract enough data to properly
    execute get_model, since Model(BaseConstructor, problem_db)
    builds the actual base class data for the derived models. */
Model::Model(ProblemDescDB& problem_db): probDescDB(problem_db),
  parallelLib(problem_db.parallel_library()), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Model::Model(ProblemDescDB&) called to instantiate envelope."
       << std::endl;
#endif

  modelRep = get_model(problem_db);
  if ( !modelRep ) // bad type or insufficient memory
    abort_handler(MODEL_ERROR);
}


/** Used only by the envelope constructor to initialize modelRep to the
    appropriate derived type, as given by the modelType attribute. */
Model* Model::get_model(ProblemDescDB& problem_db)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Envelope instantiating letter: Getting model " << modelType
       << std::endl;
#endif

  // These instantiations will NOT recurse on the Model(problem_db)
  // constructor due to the use of BaseConstructor.

  const String& model_type = problem_db.get_string("model.type");
  if ( model_type == "simulation" )
    return new SimulationModel(problem_db);
  else if ( model_type == "nested")
    return new NestedModel(problem_db);
  else if ( model_type == "surrogate") {
    if (problem_db.get_string("model.surrogate.type") == "hierarchical")
      return new HierarchSurrModel(problem_db); // hierarchical approx
    else
      return new DataFitSurrModel(problem_db);  // local/multipt/global approx
  }
  else if ( model_type == "active_subspace" )
    return new ActiveSubspaceModel(problem_db);
  else if ( model_type == "adapted_basis" )
    return new AdaptedBasisModel(problem_db);
  else if ( model_type == "random_field" )
    return new RandomFieldModel(problem_db);
  else {
    Cerr << "Invalid model type: " << model_type << std::endl;
    return NULL;
  }
}


/** Copy constructor manages sharing of modelRep and incrementing
    of referenceCount. */
Model::Model(const Model& model): probDescDB(model.problem_description_db()),
  parallelLib(probDescDB.parallel_library())
{
  // Increment new (no old to decrement)
  modelRep = model.modelRep;
  if (modelRep) // Check for an assignment of NULL
    ++modelRep->referenceCount;

#ifdef REFCOUNT_DEBUG
  Cout << "Model::Model(Model&)" << std::endl;
  if (modelRep)
    Cout << "modelRep referenceCount = " << modelRep->referenceCount
	 << std::endl;
#endif
}


/** Assignment operator decrements referenceCount for old modelRep, assigns
    new modelRep, and increments referenceCount for new modelRep. */
Model Model::operator=(const Model& model)
{
  if (modelRep != model.modelRep) { // normal case: old != new
    // Decrement old
    if (modelRep) // Check for NULL
      if ( --modelRep->referenceCount == 0 )
	delete modelRep;
    // Assign and increment new
    modelRep = model.modelRep;
    if (modelRep) // Check for NULL
      ++modelRep->referenceCount;
  }
  // else if assigning same rep, then do nothing since referenceCount
  // should already be correct

#ifdef REFCOUNT_DEBUG
  Cout << "Model::operator=(Model&)" << std::endl;
  if (modelRep)
    Cout << "modelRep referenceCount = " << modelRep->referenceCount
	 << std::endl;
#endif

  return *this;
}


/** Destructor decrements referenceCount and only deletes modelRep
    when referenceCount reaches zero. */
Model::~Model()
{
  if (modelRep) { // Check for NULL
    --modelRep->referenceCount; // decrement
#ifdef REFCOUNT_DEBUG
    Cout << "modelRep referenceCount decremented to "
         << modelRep->referenceCount << std::endl;
#endif
    if (modelRep->referenceCount == 0) {
#ifdef REFCOUNT_DEBUG
      Cout << "deleting modelRep" << std::endl;
#endif
      delete modelRep;
    }
  }
}


/** Similar to the assignment operator, the assign_rep() function
    decrements referenceCount for the old modelRep and assigns the new
    modelRep.  It is different in that it is used for publishing
    derived class letters to existing envelopes, as opposed to sharing
    representations among multiple envelopes (in particular,
    assign_rep is passed a letter object and operator= is passed an
    envelope object).  Letter assignment supports two models as
    governed by ref_count_incr:

    \li ref_count_incr = true (default): the incoming letter belongs to
    another envelope.  In this case, increment the reference count in the
    normal manner so that deallocation of the letter is handled properly.

    \li ref_count_incr = false: the incoming letter is instantiated on the
    fly and has no envelope.  This case is modeled after get_model():
    a letter is dynamically allocated using new and passed into assign_rep,
    the letter's reference count is not incremented, and the letter is not
    remotely deleted (its memory management is passed over to the envelope). */
void Model::assign_rep(Model* model_rep, bool ref_count_incr)
{
  if (modelRep == model_rep) {
    // if ref_count_incr = true (rep from another envelope), do nothing as
    // referenceCount should already be correct (see also operator= logic).
    // if ref_count_incr = false (rep from on the fly), then this is an error.
    if (!ref_count_incr) {
      Cerr << "Error: duplicated model_rep pointer assignment without "
	   << "reference count increment in Model::assign_rep()." << std::endl;
      abort_handler(MODEL_ERROR);
    }
  }
  else { // normal case: old != new
    // Decrement old
    if (modelRep) // Check for NULL
      if ( --modelRep->referenceCount == 0 )
	delete modelRep;
    // Assign new
    modelRep = model_rep;
    // Increment new
    if (modelRep && ref_count_incr) // Check for NULL and honor ref_count_incr
      modelRep->referenceCount++;
  }

#ifdef REFCOUNT_DEBUG
  Cout << "Model::assign_rep(Model*)" << std::endl;
  if (modelRep)
    Cout << "modelRep referenceCount = " << modelRep->referenceCount
	 << std::endl;
#endif
}


SizetMultiArrayConstView
Model::initialize_x0_bounds(const SizetArray& original_dvv, 
			    bool& active_derivs, bool& inactive_derivs, 
			    RealVector& x0, 
			    RealVector& fd_lb, RealVector& fd_ub) const
{
  // Are derivatives w.r.t. active or inactive variables?
  if (original_dvv == currentVariables.continuous_variable_ids()) {
    active_derivs = true;
    copy_data(currentVariables.continuous_variables(), x0); // view->copy
  }
  else if (original_dvv ==
	   currentVariables.inactive_continuous_variable_ids()) {
    inactive_derivs = true;
    copy_data(currentVariables.inactive_continuous_variables(), x0);// vw->cpy
  }
  else // general derivatives
    copy_data(currentVariables.all_continuous_variables(), x0); // view->copy

  // define c_l_bnds, c_u_bnds, cv_ids, cv_types
  const RealVector& c_l_bnds = (active_derivs) ? continuous_lower_bounds() :
    ( (inactive_derivs) ? inactive_continuous_lower_bounds() :
      all_continuous_lower_bounds() );
  const RealVector& c_u_bnds = (active_derivs) ? continuous_upper_bounds() :
    ( (inactive_derivs) ? inactive_continuous_upper_bounds() :
      all_continuous_upper_bounds() );
  SizetMultiArrayConstView cv_ids = (active_derivs) ?
    continuous_variable_ids() :
    ( (inactive_derivs) ? inactive_continuous_variable_ids() : 
      all_continuous_variable_ids() );
  UShortMultiArrayConstView cv_types = (active_derivs) ? 
    continuous_variable_types() : 
    ( (inactive_derivs) ? inactive_continuous_variable_types() : 
      all_continuous_variable_types() );

  // if not respecting bounds, leave at +/- infinity
  Real dbl_inf = std::numeric_limits<Real>::infinity();
  fd_lb = -dbl_inf;
  fd_ub =  dbl_inf;
  if (!ignoreBounds) { // manage global/inferred vs. distribution bounds
    size_t num_deriv_vars = original_dvv.size();
    for (size_t j=0; j<num_deriv_vars; j++) {
      size_t xj_index = find_index(cv_ids, original_dvv[j]);
      fd_lb[j] = finite_difference_lower_bound(cv_types, c_l_bnds, xj_index);
      fd_ub[j] = finite_difference_upper_bound(cv_types, c_u_bnds, xj_index);
    }
  }
  
  return cv_ids;
}


// compute a forward step for fd gradients; can't be const
Real Model::forward_grad_step(size_t num_deriv_vars, size_t xj_index,
			      Real x0_j, Real lb_j, Real ub_j)
{
  // Compute the offset for the ith gradient variable.
  // Enforce a minimum delta of fdgss*.01
  Real fdgss = (fdGradStepSize.length() == num_deriv_vars)
    ? fdGradStepSize[xj_index] : fdGradStepSize[0];
  //Real h = FDstep1(x0_j, lb_j, ub_j, fdgss*std::max(std::fabs(x0_j),.01));
  Real h = FDstep1(x0_j, lb_j, ub_j,
		   initialize_h(x0_j, lb_j, ub_j, fdgss, fdGradStepType));
  return h;
}



void Model::evaluate()
{
  if (modelRep) // envelope fwd to letter
    modelRep->evaluate();
  else { // letter
    ++modelEvalCntr;

    // Define default ActiveSet for iterators which don't pass one
    ActiveSet temp_set = currentResponse.active_set(); // copy
    temp_set.request_values(1); // function values only

    if (derived_master_overload()) {
      // prevents error of trying to run a multiproc. direct job on the master
      derived_evaluate_nowait(temp_set);
      currentResponse = derived_synchronize().begin()->second;
    }
    else // perform a normal synchronous map
      derived_evaluate(temp_set);

    if (modelAutoGraphicsFlag) {
      OutputManager& output_mgr = parallelLib.output_manager();
      output_mgr.add_datapoint(currentVariables, interface_id(), 
			       currentResponse);
    }
  }
}


void Model::evaluate(const ActiveSet& set)
{
  if (modelRep) // envelope fwd to letter
    modelRep->evaluate(set);
  else { // letter
    ++modelEvalCntr;

    // Derivative estimation support goes here and is not replicated in the
    // default asv version of evaluate -> a good reason for using an
    // overloaded function design rather than a default parameter design.
    ShortArray map_asv(numFns, 0), fd_grad_asv(numFns, 0),
      fd_hess_asv(numFns, 0), quasi_hess_asv(numFns, 0);
    // Manage map/estimate_derivs for a particular asv based on responses spec.
    bool use_est_deriv = manage_asv(set, map_asv, fd_grad_asv,
				    fd_hess_asv, quasi_hess_asv);

    if (use_est_deriv) {
      // Compute requested derivatives not available from the simulation (also
      // perform initial map for parallel load balance).  estimate_derivatives()
      // may involve asynch evals depending on asynchEvalFlag.
      estimate_derivatives(map_asv, fd_grad_asv, fd_hess_asv, quasi_hess_asv,
			   set, asynchEvalFlag);
      if (asynchEvalFlag) { // concatenate asynch map calls into 1 response
        const IntResponseMap& fd_responses = derived_synchronize();
        synchronize_derivatives(currentVariables, fd_responses, currentResponse,
				fd_grad_asv, fd_hess_asv, quasi_hess_asv, set);
      }
    }
    else if (derived_master_overload()) {
      // This map must be asynchronous since it prevents the error of trying
      // to run a multiprocessor direct job on the master.
      derived_evaluate_nowait(set);
      currentResponse = derived_synchronize().begin()->second;
    }
    else
      // Perform synchronous eval
      derived_evaluate(set);

    if (modelAutoGraphicsFlag) {
      OutputManager& output_mgr = parallelLib.output_manager();
      output_mgr.add_datapoint(currentVariables, interface_id(), 
			       currentResponse);
    }
  }
}


void Model::evaluate_nowait()
{
  if (modelRep) // envelope fwd to letter
    modelRep->evaluate_nowait();
  else { // letter
    ++modelEvalCntr;

    // Define default ActiveSet for iterators which don't pass one
    ActiveSet temp_set = currentResponse.active_set(); // copy
    temp_set.request_values(1); // function values only

    // perform an asynchronous parameter-to-response mapping
    derived_evaluate_nowait(temp_set);

    rawEvalIdMap[derived_evaluation_id()] = modelEvalCntr;
    numFDEvalsMap[modelEvalCntr] = -1;//no deriv est; distinguish from QN update

    // history of vars must be catalogued for use in synchronize()
    if (modelAutoGraphicsFlag)
      varsMap[modelEvalCntr] = currentVariables.copy();
  }
}


void Model::evaluate_nowait(const ActiveSet& set)
{
  if (modelRep) // envelope fwd to letter
    modelRep->evaluate_nowait(set);
  else { // letter
    ++modelEvalCntr;
    // derived evaluation_id() not yet incremented (for first of several if est
    // derivs); want the key for id map to be the first raw eval of the set
    rawEvalIdMap[derived_evaluation_id() + 1] = modelEvalCntr;

    // Manage use of estimate_derivatives() for a particular asv based on
    // the user's gradients/Hessians spec.
    ShortArray map_asv(numFns, 0),    fd_grad_asv(numFns, 0),
           fd_hess_asv(numFns, 0), quasi_hess_asv(numFns, 0);
    bool use_est_deriv = manage_asv(set, map_asv, fd_grad_asv,
				    fd_hess_asv, quasi_hess_asv);
    int num_fd_evals;
    if (use_est_deriv) {
      // Compute requested derivatives not available from the simulation.
      // Since we expect multiple evaluate_nowait()/estimate_derivatives()
      // calls prior to synchronize()/synchronize_derivatives(), we must perform
      // some additional bookkeeping so that the response arrays can be properly
      // recombined into estimated gradients/Hessians.
      estDerivsFlag = true; // flipped once per set of asynch evals
      asvList.push_back(fd_grad_asv);     asvList.push_back(fd_hess_asv);
      asvList.push_back(quasi_hess_asv);  setList.push_back(set);
      num_fd_evals
	= estimate_derivatives(map_asv, fd_grad_asv, fd_hess_asv,
			       quasi_hess_asv, set, true); // always asynch
    }
    else {
      derived_evaluate_nowait(set);
      num_fd_evals = -1; // no deriv est; distinguish from QN update
    }
    numFDEvalsMap[modelEvalCntr] = num_fd_evals;

    // history of vars must be catalogued for use in synchronize
    if (modelAutoGraphicsFlag || num_fd_evals >= 0)
      varsMap[modelEvalCntr] = currentVariables.copy();
  }
}


const IntResponseMap& Model::synchronize()
{
  if (modelRep) // envelope fwd to letter
    return modelRep->synchronize();
  else { // letter
    responseMap.clear();

    const IntResponseMap& raw_resp_map = derived_synchronize();
    IntVarsMIter v_it; IntRespMCIter r_cit; IntIntMIter id_it;

    if (estDerivsFlag) { // merge several responses into response gradients
      if (outputLevel > QUIET_OUTPUT)
        Cout <<"-----------------------------------------\n"
             << "Raw asynchronous response data captured.\n"
	     << "Merging data to estimate derivatives:\n"
	     << "----------------------------------------\n\n";
      id_it = rawEvalIdMap.begin(); IntIntMIter fd_it = numFDEvalsMap.begin();
      while (id_it != rawEvalIdMap.end() && fd_it != numFDEvalsMap.end()) {
	int raw_id = id_it->first;
	r_cit = raw_resp_map.find(raw_id);
	if (r_cit != raw_resp_map.end()) {
	  int model_id = fd_it->first, num_fd_evals = fd_it->second;
	  if (num_fd_evals >= 0) {
	    // estimate_derivatives() was used: merge raw FD responses into 1
	    // response or augment response with quasi-Hessian updating
	    if (outputLevel > QUIET_OUTPUT) {
	      //if (num_fd_evals > 1) // inconclusive due to initial_map lookup
		Cout << "Merging asynchronous responses " << raw_id
		     << " through " << raw_id + num_fd_evals - 1 << '\n';
	      //else
	      //  Cout << "Augmenting asynchronous response " << raw_id
	      //       << " with quasi-Hessian updating\n";
	    }
	    v_it = varsMap.find(model_id);
	    IntRespMCIter re = r_cit; std::advance(re, num_fd_evals);
	    IntResponseMap tmp_response_map(r_cit, re);
	    // Recover fd_grad/fd_hess/quasi_hess asv's from asvList and
	    // orig_set from setList
	    ShortArray fd_grad_asv    = asvList.front(); asvList.pop_front();
	    ShortArray fd_hess_asv    = asvList.front(); asvList.pop_front();
	    ShortArray quasi_hess_asv = asvList.front(); asvList.pop_front();
	    ActiveSet  orig_set       = setList.front(); setList.pop_front();
	    synchronize_derivatives(v_it->second, tmp_response_map,
				    responseMap[model_id], fd_grad_asv,
				    fd_hess_asv, quasi_hess_asv, orig_set);
	    // cleanup
	    if (!modelAutoGraphicsFlag) varsMap.erase(v_it);
	  }
	  else { // number of maps==1, derivs not estimated
	    if (outputLevel > QUIET_OUTPUT)
	      Cout << "Asynchronous response " << raw_id
		   << " does not require merging.\n";
	    responseMap[model_id] = r_cit->second;
	  }
	  // cleanup: postfix increment manages iterator invalidation
	  numFDEvalsMap.erase(fd_it++); rawEvalIdMap.erase(id_it++);
	}
	else // preserve bookkeeping for a subsequent synchronization pass
	  { ++fd_it; ++id_it; }
      }
      // reset flags
      estDerivsFlag = false;
    }
    else // no calls to estimate_derivatives()
      // rekey the raw response map (lower level evaluation ids may be offset
      // from modelEvalCntr if previous finite differencing occurred)
      //rekey_response_map(raw_resp_map, rawEvalIdMap, responseMap);
      for (r_cit = raw_resp_map.begin(); r_cit != raw_resp_map.end(); ++r_cit) {
	id_it = rawEvalIdMap.find(r_cit->first);
	if (id_it != rawEvalIdMap.end()) {
	  int model_id = id_it->second;
	  responseMap[model_id] = r_cit->second;
	  rawEvalIdMap.erase(id_it);
	  numFDEvalsMap.erase(model_id);
	}
      }

    // update graphics
    if (modelAutoGraphicsFlag) {
      OutputManager& output_mgr = parallelLib.output_manager();
      for (r_cit = responseMap.begin(); r_cit != responseMap.end(); ++r_cit) {
	v_it = varsMap.find(r_cit->first);
	output_mgr.add_datapoint(v_it->second, interface_id(), r_cit->second);
	varsMap.erase(v_it);
      }
    }

    // Now augment rekeyed response map with locally cached evals.  If
    // these are not matched in a higher-level rekey process used by the
    // calling context, then they are returned to cachedResponseMap
    // using Model::cache_unmatched_response().
    responseMap.insert(cachedResponseMap.begin(), cachedResponseMap.end());
    cachedResponseMap.clear();

    // return final map
    return responseMap;
  }
}


const IntResponseMap& Model::synchronize_nowait()
{
  if (modelRep) // envelope fwd to letter
    return modelRep->synchronize_nowait();
  else { // letter
    responseMap.clear();

    if (estDerivsFlag) {
      // This will require caching of evals until a merging set is complete.
      // While this would be straightforward to implement (using similar code
      // to the graphics caching below), there are no current use cases since
      // all gradient-based methods use blocking synchronization (you'd need
      // something like a parallel greedy gradient-based line search).
      Cerr << "Error: finite differencing within asynch evaluations not "
	   << "currently supported by Model::synchronize_nowait()" << std::endl;
      abort_handler(MODEL_ERROR);
    }

    const IntResponseMap& raw_resp_map = derived_synchronize_nowait();

    // rekey and cleanup.
    // Note 1: rekeying is needed for the case of mixed usage of synchronize()
    // and synchronize_nowait(), since the former can introduce offsets.
    // Note 2: if estimate_derivatives() support is added, then rawEvalIdMap
    // data input must be expanded to include FD evals.
    IntRespMCIter r_cit; IntIntMIter id_it;
    for (r_cit = raw_resp_map.begin(); r_cit != raw_resp_map.end(); ++r_cit) {
      id_it = rawEvalIdMap.find(r_cit->first);
      if (id_it != rawEvalIdMap.end()) {
	int model_id = id_it->second;
	responseMap[model_id] = r_cit->second;
	rawEvalIdMap.erase(id_it);
	numFDEvalsMap.erase(model_id);
      }
    }

    // Update graphics.  There are two possible ways to do this:
    // (1) propagate separate eval id bookkeeping to Model level and extend
    //     Dakota::Graphics to allow nonsequential input with id's (where data
    //     is reordered such that lines connect properly).
    // (2) cache data here and input sequentially when enough data has been
    //     returned to allow an unbroken sequence.
    // Since SciPlot will not directly support (1), approach (2) is taken.
    if (modelAutoGraphicsFlag) {
      // add new completions to graphics cache.
      graphicsRespMap.insert(responseMap.begin(), responseMap.end());
      // search for next response set(s) in sequence
      bool found = true;
      while (found) {
	OutputManager& output_mgr = parallelLib.output_manager();
	int graphics_cntr = output_mgr.graphics_counter();
	// find() is not really necessary due to Map ordering
	//g_it = graphicsRespMap.begin();
	//if (g_it == graphicsRespMap.end() || g_it->first != graphics_cntr)
	IntRespMIter g_it = graphicsRespMap.find(graphics_cntr);
	if (g_it == graphicsRespMap.end())
	  found = false;
	else {
	  IntVarsMIter v_it = varsMap.find(graphics_cntr);
	  output_mgr.add_datapoint(v_it->second, interface_id(), g_it->second);
	  varsMap.erase(v_it); graphicsRespMap.erase(g_it);
	}
      }
    }

    // Now augment rekeyed response map with locally cached evals.  If
    // these are not matched in a higher-level rekey process used by the
    // calling context, then they are returned to cachedResponseMap
    // using Model::cache_unmatched_response().
    responseMap.insert(cachedResponseMap.begin(), cachedResponseMap.end());
    cachedResponseMap.clear();

    return responseMap;
  }
}


/** Auxiliary function to determine initial finite difference h
    (before step length adjustment) based on type of step desired. */
Real Model::initialize_h(Real x_j, Real lb_j, Real ub_j, Real step_size, 
			 String step_type) const
{
  Real h;
  if (step_type == "absolute")
    h = std::max(step_size, std::sqrt(DBL_MIN));
  else if (step_type == "bounds")
    h = step_size*std::max((ub_j-lb_j), std::sqrt(DBL_MIN));
  else     // relative
    h = step_size*std::max(std::fabs(x_j),.01);

  return h;
}


/** Auxiliary function to compute forward or first central-difference
    step size, honoring bounds.  The first step is away from zero,
    when possible.  Flips the direction or updates shortStep if can't
    take the full requested step h_mag. */
Real Model::FDstep1(Real x0_j, Real lb_j, Real ub_j, Real h_mag)
{
  Real h;
  shortStep = false;
  if (x0_j < 0.) {
    h = -h_mag;
    if (!ignoreBounds && x0_j + h < lb_j) {
      // step would exceed lower bound; try flipping, else shorten
      if (x0_j + h_mag <= ub_j)
        h = h_mag;
      else
        shortStep = true;
    }
  }
  else {
    h = h_mag;
    if (!ignoreBounds && x0_j + h > ub_j) {
      // step would exceed upper bound; try flipping, else shorten
      if (x0_j - h_mag >= lb_j)
        h = -h_mag;
      else {
        shortStep = true;
      }
    }
  }
  
  if (shortStep) {
    // take the step to the furthest boundary
    Real h1 = x0_j - lb_j;
    Real h2 = ub_j - x0_j;
    if (h1 < h2)
      h = h2;
    else
      h = -h1;
  }

  return h;
}


/** Auxiliary function to compute the second central-difference step size,
    honoring bounds. */
Real Model::FDstep2(Real x0_j, Real lb_j, Real ub_j, Real h)
{
  Real h2 = -h;  // default is to take same size step in opposite direction

  // if taking a shorter step to a boundary, the second step is half of it
  if (shortStep)
    h2 = 0.5*h;
  else if (!ignoreBounds) {
    Real h1;
    if (h2 < 0.) {
      if (x0_j + h2 < lb_j) {
        // step would exceed lower bound; try full step in opposite
        // direction; contracting until in-bounds
        shortStep = true;
        h1 = h + h;
        if (x0_j + h1 <= ub_j)
          h2 = h1;
        else {
          h1 = 1.5*h;
          if (x0_j + h1 <= ub_j)
            h2 = h1;
          else
            h2 = 0.5*h;
        }
      }
    }
    else {
      if (x0_j + h2 > ub_j) {
        // step would exceed upper bound; try full step in opposite
        // direction; contracting until in-bounds
        shortStep = true;
        h1 = h + h;
        if (x0_j + h1 >= lb_j)
          h2 = h1;
        else {
          h1 = 1.5*h;
          if (x0_j + h1 >= lb_j)
            h2 = h1;
          else
            h2 = 0.5*h;
        }
      }
    }
  }
  return h2;
}


/** Estimate derivatives by computing finite difference gradients, finite
    difference Hessians, and/or quasi-Newton Hessians.  The total number of
    finite difference evaluations is returned for use by synchronize() to
    track response arrays, and it could be used to improve management of
    max_function_evaluations within the iterators. */
int Model::
estimate_derivatives(const ShortArray& map_asv, const ShortArray& fd_grad_asv,
		     const ShortArray& fd_hess_asv,
		     const ShortArray& quasi_hess_asv,
		     const ActiveSet& original_set, const bool asynch_flag)
{
  if (outputLevel > SILENT_OUTPUT)
    Cout << "\n------------------------------------------\n"
         <<   "Begin Dakota derivative estimation routine\n"
         <<   "------------------------------------------\n\n";

  // -----------------------------------------------
  // Retrieve/evaluate fn_vals_x0 and/or fn_grads_x0
  // -----------------------------------------------

  // Perform initial map at x0 to calculate (1) non-finite-differenced portions
  // of the response, (2) fn_vals_x0 for forward difference gradients or
  // second-order function-difference Hessians, and/or (3) fn_grads_x0 for
  // first-order gradient-difference Hessians.  The fn_vals_x0/fn_grads_x0 data
  // may already be available from preceding function evaluations (e.g., an
  // iterator such as OPT++ requests fn. values in one evaluate call
  // followed by a gradient request, followed by a Hessian request), so perform
  // a database search when appropriate to retrieve the data instead of relying
  // solely on duplication detection.
  bool initial_map = false, augmented_data_flag = false, db_capture = false,
    fd_grad_flag = false, fd_hess_flag = false, fd_hess_by_fn_flag = false,
    fd_hess_by_grad_flag = false;
  const ShortArray& original_asv = original_set.request_vector();
  const SizetArray& original_dvv = original_set.derivative_vector();
  size_t i, j, k, map_counter = 0, num_deriv_vars = original_dvv.size();
  size_t ifg, nfg = 0;

  for (i=0; i<numFns; i++) {
    if (map_asv[i]) {
      initial_map = true;
      if ( ( (map_asv[i] & 1) && !(original_asv[i] & 1) ) ||
           ( (map_asv[i] & 2) && !(original_asv[i] & 2) ) )
        augmented_data_flag = true; // original_asv val/grad requests augmented
    }
    if (fd_grad_asv[i])
      fd_grad_flag = true;           // gradient finite differencing needed
    if (fd_hess_asv[i]) {
      fd_hess_flag = true;           // Hessian finite differencing needed ...
      if (fd_hess_asv[i] & 1)
        fd_hess_by_fn_flag = true;   // ... by 2nd-order function differences
      if (fd_hess_asv[i] & 2)
        ++nfg;                       // ... by 1st-order gradient differences
    }
  }
  if (nfg)
    fd_hess_by_grad_flag = true;

  ActiveSet new_set(map_asv, original_set.derivative_vector());
  Response initial_map_response(currentResponse.shared_data(), new_set);

  // The logic for incurring an additional data_pairs search (beyond the
  // existing duplicate detection) is that a data request contained in
  // original_asv is most likely not a duplicate, but there is a good chance
  // that an augmented data reqmt (appears in map_asv but not in original_asv)
  // has been evaluated previously.  The additional search allows us to trap
  // this common case more gracefully (special header, no evaluation echo).
  if (augmented_data_flag) {
    if (db_lookup(currentVariables, new_set, initial_map_response)) {
      if (outputLevel > SILENT_OUTPUT)
        Cout << ">>>>> map at X performed previously and results retrieved\n\n";
      initial_map = false; // reset
      if (asynch_flag) {
        db_capture = true;
        dbResponseList.push_back(initial_map_response);
      }
    }
  }
  if (asynch_flag) { // communicate settings to synchronize_derivatives()
    initialMapList.push_back(initial_map);
    dbCaptureList.push_back(db_capture);
  }

  if (initial_map) {
    if (outputLevel > SILENT_OUTPUT) {
      Cout << ">>>>> Initial map for analytic portion of response";
      if (augmented_data_flag)
	Cout << "\n      augmented with data requirements for differencing";
      Cout << ":\n";
    }
    if (asynch_flag) {
      derived_evaluate_nowait(new_set);
      if (outputLevel > SILENT_OUTPUT)
	Cout << "\n\n";
    }
    else {
      derived_evaluate(new_set);
      initial_map_response.update(currentResponse);
    }
    ++map_counter;
  }

  // ------------------------------
  // Estimate numerical derivatives
  // ------------------------------
  RealVector dx;
  RealVectorArray fg, fx;
  RealMatrix new_fn_grads;
  if (fd_grad_flag && !asynch_flag) {
    new_fn_grads.shapeUninitialized(num_deriv_vars, numFns);
    new_fn_grads = 0.;
  }
  RealSymMatrixArray new_fn_hessians;
  if (fd_hess_flag) {
    if (fd_hess_by_fn_flag && !centralHess)
      dx.resize(num_deriv_vars);
    if (!asynch_flag) {
      new_fn_hessians.resize(numFns);
      for (i=0; i<numFns; i++) {
        new_fn_hessians[i].reshape(num_deriv_vars);
        new_fn_hessians[i] = 0.;
      }
      if (fd_hess_by_fn_flag && !centralHess)
        fx.resize(num_deriv_vars);
      if (fd_hess_by_grad_flag) {
        fg.resize(ifg = nfg*num_deriv_vars);
        while(ifg > 0)
          fg[--ifg].resize(num_deriv_vars);
      }
    }
  }
  if (fd_grad_flag || fd_hess_flag) {

    // define x0 and mode flags
    bool active_derivs = false;    // derivatives w.r.t. active vars
    bool inactive_derivs = false;  // derivs w.r.t. inactive vars
    RealVector x0;

    // define lower/upper bounds for finite differencing and cv_ids
    RealVector fd_lb(num_deriv_vars), fd_ub(num_deriv_vars);
    SizetMultiArrayConstView cv_ids = 
      initialize_x0_bounds(original_dvv, active_derivs, inactive_derivs, 
                           x0, fd_lb, fd_ub);

    const RealVector& fn_vals_x0  = initial_map_response.function_values();
    const RealMatrix& fn_grads_x0 = initial_map_response.function_gradients();

    // ------------------------
    // Loop over num_deriv_vars
    // ------------------------
    RealVector x = x0; 
    for (j=0; j<num_deriv_vars; j++) { // difference the 1st num_deriv_vars vars

      size_t xj_index = find_index(cv_ids, original_dvv[j]);
      Real x0_j = x0[xj_index], lb_j = fd_lb[j], ub_j = fd_ub[j];

      if (fd_grad_flag) {
        if (!ignoreBounds && lb_j >= ub_j) {
          if (asynch_flag)
            deltaList.push_back(0.);
          else
            for (i=0; i<numFns; i++)
              if (fd_grad_asv[i])
                new_fn_grads(j,i) = 0.;
          continue;
        }
        new_set.request_vector(fd_grad_asv);

        // Compute the offset for the ith gradient variable.
        Real h = forward_grad_step(num_deriv_vars, xj_index, x0_j, lb_j, ub_j);

        if (asynch_flag) // communicate settings to synchronize_derivatives()
          deltaList.push_back(h);

        // -------------------------
        // Evaluate fn_vals_x_plus_h
        // -------------------------
        RealVector fn_vals_x_plus_h;
        x[xj_index] = x0_j + h;
        if (outputLevel > SILENT_OUTPUT)
          Cout << ">>>>> Dakota finite difference gradient evaluation for x["
               << j+1 << "] + h:\n";
        if (active_derivs)
          currentVariables.continuous_variables(x);
        else if (inactive_derivs)
          currentVariables.inactive_continuous_variables(x);
        else
          currentVariables.all_continuous_variables(x);
        if (asynch_flag) {
          derived_evaluate_nowait(new_set);
          if (outputLevel > SILENT_OUTPUT)
            Cout << "\n\n";
        }
        else {
          derived_evaluate(new_set);
          fn_vals_x_plus_h = currentResponse.function_values();
          if (intervalType == "forward") {
            for (i=0; i<numFns; i++)
              // prevent erroneous difference of vals present in fn_vals_x0 but
              // not in fn_vals_x_plus_h because of map/fd_grad asv differences
              if (fd_grad_asv[i])
                new_fn_grads(j,i) = (fn_vals_x_plus_h[i] - fn_vals_x0[i])/h;
          }
        }
        ++map_counter;

        // --------------------------
        // Evaluate fn_vals_x_minus_h
        // --------------------------
        if (intervalType == "central") {
          Real h1, h2 = FDstep2(x0_j, lb_j, ub_j, h);
          x[xj_index] = x0_j + h2;
          if (outputLevel > SILENT_OUTPUT)
            Cout << ">>>>> Dakota finite difference gradient evaluation for x["
                 << j+1 << "] - h:\n";
          if (active_derivs)
            currentVariables.continuous_variables(x);
          else if (inactive_derivs)
            currentVariables.inactive_continuous_variables(x);
          else
            currentVariables.all_continuous_variables(x);
          if (asynch_flag) {
            deltaList.push_back(h2);
            derived_evaluate_nowait(new_set);
            if (outputLevel > SILENT_OUTPUT)
              Cout << "\n\n";
          }
          else {
            derived_evaluate(new_set);
            const RealVector& fn_vals_x_minus_h
              = currentResponse.function_values();
            // no need to check fd_grad_asv since it was used for both evals
            if (shortStep) {
              Real h12 = h*h, h22 = h2*h2;
              h1 = h*h2*(h2-h);
              for(i = 0; i < numFns; ++i)
                new_fn_grads(j,i)
                  = ( h22*(fn_vals_x_plus_h[i]  - fn_vals_x0[i]) -
		      h12*(fn_vals_x_minus_h[i] - fn_vals_x0[i]) ) / h1;
            }
            else {
              h1 = h - h2;
              for (i=0; i<numFns; i++)
                new_fn_grads(j,i)
                  = (fn_vals_x_plus_h[i] - fn_vals_x_minus_h[i]) / h1;
            }
          }
          ++map_counter;
        }
      }

      if (fd_hess_flag) {
        new_set.request_vector(fd_hess_asv);

        // If analytic grads, then 1st-order gradient differences
        // > no interval type control (uses only forward diff of analytic
        //   grads), separate finite diff step size.
        // If numerical grads, then 2nd-order function differences
        // > no interval type control (uses only central diffs of numerical
        //   grads from central fn diffs), separate finite diff step size.
        //   Could get some eval reuse for diagonal Hessian terms by setting
        //   fdHessStepSize to half of fdGradStepSize, but this is not
        //   hard-wired since generally want fdHessStepSize > fdGradStepSize
        //   (if desired, the user can set fdHessStepSize to fdGradStepSize/2
        //   to get reuse).
        // If mixed grads, then mixed 1st/2nd-order diffs for numerical Hessians

        if (fd_hess_by_fn_flag) {
          if (centralHess) {
            RealVector fn_vals_x_plus_2h, fn_vals_x_minus_2h;

            // Compute the 2nd-order Hessian offset for the ith variable.
            // Enforce a minimum delta of fdhss*.01
            Real fdhbfss = (fdHessByFnStepSize.length() == num_deriv_vars)
              ? fdHessByFnStepSize[xj_index] : fdHessByFnStepSize[0];
            Real h_mag = fdhbfss * std::max(std::fabs(x0_j), .01);
            Real h = (x0_j < 0.) ? -h_mag : h_mag; // h has same sign as x0_j
            if (asynch_flag)// communicate settings to synchronize_derivatives()
              deltaList.push_back(h);

            // evaluate diagonal term

            // --------------------------
            // Evaluate fn_vals_x_plus_2h
            // --------------------------
            x[xj_index] = x0[xj_index] + 2.*h;
            if (outputLevel > SILENT_OUTPUT)
              Cout << ">>>>> Dakota finite difference Hessian evaluation for x["
                   << j+1 << "] + 2h:\n";
            if (active_derivs)
              currentVariables.continuous_variables(x);
            else if (inactive_derivs)
              currentVariables.inactive_continuous_variables(x);
            else
              currentVariables.all_continuous_variables(x);
            if (asynch_flag) {
              derived_evaluate_nowait(new_set);
              if (outputLevel > SILENT_OUTPUT)
                Cout << "\n\n";
            }
            else {
              derived_evaluate(new_set);
              fn_vals_x_plus_2h = currentResponse.function_values();
            }

            // ---------------------------
            // Evaluate fn_vals_x_minus_2h
            // ---------------------------
            x[xj_index] = x0[xj_index] - 2.*h;
            if (outputLevel > SILENT_OUTPUT)
              Cout << ">>>>> Dakota finite difference Hessian evaluation for x["
                   << j+1 << "] - 2h:\n";
            if (active_derivs)
              currentVariables.continuous_variables(x);
            else if (inactive_derivs)
              currentVariables.inactive_continuous_variables(x);
            else
              currentVariables.all_continuous_variables(x);
            if (asynch_flag) {
              derived_evaluate_nowait(new_set);
              if (outputLevel > SILENT_OUTPUT)
                Cout << "\n\n";
            }
            else {
              derived_evaluate(new_set);
              fn_vals_x_minus_2h = currentResponse.function_values();
            }

            map_counter += 2;
            if (!asynch_flag) {
              for (i=0; i<numFns; i++)
                // prevent error in differencing vals present in fn_vals_x0 but
                // not in fn_vals_x_(plus/minus)_2h due to map/fd_hess asv diffs
                if (fd_hess_asv[i] & 1)
                  new_fn_hessians[i](j,j) = (fn_vals_x_plus_2h[i]
                     - 2.*fn_vals_x0[i] +  fn_vals_x_minus_2h[i])/(4.*h*h);
            }

            // evaluate off-diagonal terms

            for (k=j+1; k<num_deriv_vars; k++) {
              size_t xk_index = find_index(cv_ids, original_dvv[k]);
              RealVector fn_vals_x_plus_h_plus_h,  fn_vals_x_plus_h_minus_h,
                fn_vals_x_minus_h_plus_h, fn_vals_x_minus_h_minus_h;

              // --------------------------------
              // Evaluate fn_vals_x_plus_h_plus_h
              // --------------------------------
              x[xj_index] = x0[xj_index] + h;
              x[xk_index] = x0[xk_index] + h;
              if (outputLevel > SILENT_OUTPUT)
                Cout << ">>>>> Dakota finite difference Hessian evaluation for "
                     << "x[" << j+1 << "] + h, x[" << k+1 << "] + h:\n";
              if (active_derivs)
                currentVariables.continuous_variables(x);
              else if (inactive_derivs)
                currentVariables.inactive_continuous_variables(x);
              else
                currentVariables.all_continuous_variables(x);
              if (asynch_flag) {
                derived_evaluate_nowait(new_set);
                if (outputLevel > SILENT_OUTPUT)
                  Cout << "\n\n";
              }
              else {
                derived_evaluate(new_set);
                fn_vals_x_plus_h_plus_h = currentResponse.function_values();
              }
              // ---------------------------------
              // Evaluate fn_vals_x_plus_h_minus_h
              // ---------------------------------
              //x[xj_index] = x0[xj_index] + h;
              x[xk_index] = x0[xk_index] - h;
              if (outputLevel > SILENT_OUTPUT)
                Cout << ">>>>> Dakota finite difference Hessian evaluation for "
                     << "x[" << j+1 << "] + h, x[" << k+1 << "] - h:\n";
              if (active_derivs)
                currentVariables.continuous_variables(x);
              else if (inactive_derivs)
                currentVariables.inactive_continuous_variables(x);
              else
                currentVariables.all_continuous_variables(x);
              if (asynch_flag) {
                derived_evaluate_nowait(new_set);
                if (outputLevel > SILENT_OUTPUT)
                  Cout << "\n\n";
              }
              else {
                derived_evaluate(new_set);
                fn_vals_x_plus_h_minus_h = currentResponse.function_values();
              }
              // ---------------------------------
              // Evaluate fn_vals_x_minus_h_plus_h
              // ---------------------------------
              x[xj_index] = x0[xj_index] - h;
              x[xk_index] = x0[xk_index] + h;
              if (outputLevel > SILENT_OUTPUT)
                Cout << ">>>>> Dakota finite difference Hessian evaluation for "
                     << "x[" << j+1 << "] - h, x[" << k+1 << "] + h:\n";
              if (active_derivs)
                currentVariables.continuous_variables(x);
              else if (inactive_derivs)
                currentVariables.inactive_continuous_variables(x);
              else
                currentVariables.all_continuous_variables(x);
              if (asynch_flag) {
                derived_evaluate_nowait(new_set);
                if (outputLevel > SILENT_OUTPUT)
                  Cout << "\n\n";
              }
              else {
                derived_evaluate(new_set);
                fn_vals_x_minus_h_plus_h = currentResponse.function_values();
              }
              // ----------------------------------
              // Evaluate fn_vals_x_minus_h_minus_h
              // ----------------------------------
              //x[xj_index] = x0[xj_index] - h;
              x[xk_index] = x0[xk_index] - h;
              if (outputLevel > SILENT_OUTPUT)
                Cout << ">>>>> Dakota finite difference Hessian evaluation for "
                     << "x[" << j+1 << "] - h, x[" << k+1 << "] - h:\n";
              if (active_derivs)
                currentVariables.continuous_variables(x);
              else if (inactive_derivs)
                currentVariables.inactive_continuous_variables(x);
              else
                currentVariables.all_continuous_variables(x);
              if (asynch_flag) {
                derived_evaluate_nowait(new_set);
                if (outputLevel > SILENT_OUTPUT)
                  Cout << "\n\n";
              }
              else {
                derived_evaluate(new_set);
                fn_vals_x_minus_h_minus_h = currentResponse.function_values();
              }

              map_counter += 4;
              if (!asynch_flag)
                for (i=0; i<numFns; i++)
                  // no need to check fd_hess_asv since used for each eval
                  // NOTE: symmetry is naturally satisfied.  Teuchos maps
                  // new_fn_hessians[i](j,k) and new_fn_hessians[i](k,j)
                  // to the same memory cell.
                  new_fn_hessians[i](j,k)
                    = (fn_vals_x_plus_h_plus_h[i] - fn_vals_x_plus_h_minus_h[i]
                       -  fn_vals_x_minus_h_plus_h[i]
                       +  fn_vals_x_minus_h_minus_h[i] ) / (4.*h*h);

              x[xk_index] = x0[xk_index];
            }
          }
          else { //!! new logic
            RealVector fn_vals_x1, fn_vals_x12, fn_vals_x2;

            // Compute the 2nd-order Hessian offset for the ith variable.
            // Enforce a minimum delta of fdhss*.01
            Real fdhbfss = (fdHessByFnStepSize.length() == num_deriv_vars)
              ? fdHessByFnStepSize[xj_index] : fdHessByFnStepSize[0];
            //	    Real h1 = FDstep1(x0_j, lb_j, ub_j, 2. * fdhbfss *
            //			      std::max(std::fabs(x0_j), .01));
            Real h1 = FDstep1(x0_j, lb_j, ub_j,
	      initialize_h(x0_j, lb_j, ub_j, 2.*fdhbfss, fdHessStepType));
            Real h2 = FDstep2(x0_j, lb_j, ub_j, h1);
            Real denom, hdiff;
            if (asynch_flag) { // transfer settings to synchronize_derivatives()
              deltaList.push_back(h1);
              deltaList.push_back(h2);
            }
            dx[j] = h1;

            // evaluate diagonal term

            // --------------------------
            // Evaluate fn_vals_x1
            // --------------------------
            x[xj_index] = x0[xj_index] + h1;
            if (outputLevel > SILENT_OUTPUT)
              Cout << ">>>>> Dakota finite difference Hessian evaluation for x["
                   << j+1 << "] + 2h:\n";
            if (active_derivs)
              currentVariables.continuous_variables(x);
            else if (inactive_derivs)
              currentVariables.inactive_continuous_variables(x);
            else
              currentVariables.all_continuous_variables(x);
            if (asynch_flag) {
              derived_evaluate_nowait(new_set);
              if (outputLevel > SILENT_OUTPUT)
                Cout << "\n\n";
            }
            else {
              derived_evaluate(new_set);
              fx[j] = fn_vals_x1 = currentResponse.function_values();
            }

            // ---------------------------
            // Evaluate fn_vals_x2
            // ---------------------------
            x[xj_index] = x0[xj_index] + h2;
            if (outputLevel > SILENT_OUTPUT)
              Cout << ">>>>> Dakota finite difference Hessian evaluation for x["
                   << j+1 << "] - 2h:\n";
            if (active_derivs)
              currentVariables.continuous_variables(x);
            else if (inactive_derivs)
              currentVariables.inactive_continuous_variables(x);
            else
              currentVariables.all_continuous_variables(x);
            if (asynch_flag) {
              derived_evaluate_nowait(new_set);
              if (outputLevel > SILENT_OUTPUT)
                Cout << "\n\n";
            }
            else {
              derived_evaluate(new_set);
              fn_vals_x2 = currentResponse.function_values();
            }

            map_counter += 2;
            if (!asynch_flag) {
              if (h1 + h2 == 0.) {
                denom = h1*h1;
                for (i = 0; i < numFns; i++)
                  // prevent erroneous difference of vals in fn_vals_x0 but not
                  // in fn_vals_x_(plus/minus)_2h due to map/fd_hess asv diffs
                  if (fd_hess_asv[i] & 1)
                    new_fn_hessians[i](j,j)
                      = (fn_vals_x1[i] - 2.*fn_vals_x0[i] + fn_vals_x2[i])
                      / denom;
              }
              else {
                hdiff = h1 - h2;
                denom = 0.5*h1*h2*hdiff;
                for (i = 0; i < numFns; i++)
                  if (fd_hess_asv[i] & 1)
                    new_fn_hessians[i](j,j)
                      = (h2*fn_vals_x1[i] + hdiff*fn_vals_x0[i] -
                         h1*fn_vals_x2[i])/denom;
              }
            }

            // evaluate off-diagonal terms

            for (k = 0; k < j; k++) {
              size_t xk_index = find_index(cv_ids, original_dvv[k]);

              // --------------------------------
              // Evaluate fn_vals_x12
              // --------------------------------
              h2 = dx[k];
              x[xj_index] = x0[xj_index] + h1;
              x[xk_index] = x0[xk_index] + h2;
              if (outputLevel > SILENT_OUTPUT)
                Cout << ">>>>> Dakota finite difference Hessian evaluation for "
                     << "x[" << j+1 << "] + h, x[" << k+1 << "] + h:\n";
              if (active_derivs)
                currentVariables.continuous_variables(x);
              else if (inactive_derivs)
                currentVariables.inactive_continuous_variables(x);
              else
                currentVariables.all_continuous_variables(x);
              if (asynch_flag) {
                derived_evaluate_nowait(new_set);
                if (outputLevel > SILENT_OUTPUT)
                  Cout << "\n\n";
              }
              else {
                derived_evaluate(new_set);
                fn_vals_x12 = currentResponse.function_values();
                denom = h1*h2;
                const RealVector& f2 = fx[k];
                for (i=0; i<numFns; ++i)
                  new_fn_hessians[i](j,k) = (fn_vals_x12[i] - fn_vals_x1[i] -
                                             f2[i] + fn_vals_x0[i]) / denom;
              }

              ++map_counter;
              x[xk_index] = x0[xk_index];
            }
          }
        }

        if (fd_hess_by_grad_flag) {

          // Compute the 1st-order Hessian offset for the ith variable.
          // Enforce a minimum delta of fdhss*.01
          Real fdhbgss = (fdHessByGradStepSize.length() == num_deriv_vars)
            ? fdHessByGradStepSize[xj_index] : fdHessByGradStepSize[0];
          //	  Real h = FDstep1(x0_j, lb_j, ub_j, fdhbgss *
          //			   std::max(std::fabs(x0_j), .01));
          Real h = FDstep1(x0_j, lb_j, ub_j,
            initialize_h(x0_j, lb_j, ub_j, fdhbgss, fdHessStepType));
          if (asynch_flag) // communicate settings to synchronize_derivatives()
            deltaList.push_back(h);

          // --------------------------
          // Evaluate fn_grads_x_plus_h
          // --------------------------
          x[xj_index] = x0[xj_index] + h;
          if (outputLevel > SILENT_OUTPUT)
            Cout << ">>>>> Dakota finite difference Hessian evaluation for x["
                 << j+1 << "] + h:\n";
          if (active_derivs)
            currentVariables.continuous_variables(x);
          else if (inactive_derivs)
            currentVariables.inactive_continuous_variables(x);
          else
            currentVariables.all_continuous_variables(x);
          if (asynch_flag) {
            derived_evaluate_nowait(new_set);
            if (outputLevel > SILENT_OUTPUT)
              Cout << "\n\n";
          }
          else {
            derived_evaluate(new_set);
            const RealMatrix& fn_grads_x_plus_h
              = currentResponse.function_gradients();
            ifg = j;
            for (i=0; i<numFns; i++)
              // prevent erroneous difference of grads present in fn_grads_x0
              // but not in fn_grads_x_plus_h due to map_asv & fd_hess_asv diffs
              // NOTE: symmetry NOT enforced [could replace with 1/2 (H + H^T)]

              if (fd_hess_asv[i] & 2) {
                //fg[ifg] = (fn_grads_x_plus_h[i] - fn_grads_x0[i]) / h;
                for(k = 0; k < num_deriv_vars; ++k)
                  fg[ifg][k] = (fn_grads_x_plus_h[i][k] - fn_grads_x0[i][k])/ h;
                ifg += num_deriv_vars;
              }
          }
          ++map_counter;
        }
      }
      x[xj_index] = x0[xj_index];
    }

    // Reset currentVariables to x0 (for graphics, etc.)
    if (active_derivs)
      currentVariables.continuous_variables(x0);
    else if (inactive_derivs)
      currentVariables.inactive_continuous_variables(x0);
    else
      currentVariables.all_continuous_variables(x0);
  }

  // If asynchronous, then synchronize_derivatives() will postprocess and
  // update the response.  If synchronous, then update the response now.
  if (!asynch_flag) {
    // Enforce symmetry in the case of FD Hessians from 1st-order gradient
    // differences by averaging off-diagonal terms: H' = 1/2 (H + H^T)
    if (fd_hess_by_grad_flag)
      for (i = ifg = 0; i < numFns; i++)
        if (fd_hess_asv[i] & 2) {
          for (j=0; j<num_deriv_vars; j++) {
            for (k = 0; k < j; k++)
              new_fn_hessians[i](j,k) = 0.5 * (fg[ifg+j][k] + fg[ifg+k][j]);
            new_fn_hessians[i](j,j) = fg[ifg+j][j];
          }
          ifg += num_deriv_vars;
        }

    update_response(currentVariables, currentResponse, fd_grad_asv, fd_hess_asv,
                    quasi_hess_asv, original_set, initial_map_response,
                    new_fn_grads, new_fn_hessians);
  }

  return map_counter;
}


/** Merge an array of fd_responses into a single new_response.  This
    function is used both by synchronous evaluate() for the
    case of asynchronous estimate_derivatives() and by synchronize()
    for the case where one or more evaluate_nowait() calls has
    employed asynchronous estimate_derivatives(). */
void Model::
synchronize_derivatives(const Variables& vars,
			const IntResponseMap& fd_responses,
			Response& new_response, const ShortArray& fd_grad_asv,
			const ShortArray& fd_hess_asv,
			const ShortArray& quasi_hess_asv,
			const ActiveSet& original_set)
{
  const SizetArray& original_dvv = original_set.derivative_vector();
  size_t i, j, k, num_deriv_vars = original_dvv.size();
  bool fd_grad_flag = false, fd_hess_flag = false, fd_hess_by_fn_flag = false,
    fd_hess_by_grad_flag = false;
  RealVector dx;
  std::vector<const RealVector*> fx;
  size_t ifg, nfg = 0;

  for (i=0; i<numFns; i++) {
    if (fd_grad_asv[i])
      fd_grad_flag = true;           // gradient finite differencing used
    if (fd_hess_asv[i]) {
      fd_hess_flag = true;           // Hessian finite differencing used ...
      if (fd_hess_asv[i] & 1)
        fd_hess_by_fn_flag = true;   // ... with 2nd-order function differences
      if (fd_hess_asv[i] & 2)
        ++nfg;                       // ... with 1st-order gradient differences
    }
  }
  if (nfg)
    fd_hess_by_grad_flag = true;

  RealMatrix new_fn_grads;
  if (fd_grad_flag) {
    new_fn_grads.shapeUninitialized(num_deriv_vars, numFns);
    new_fn_grads = 0.;
  }
  RealSymMatrixArray new_fn_hessians;
  RealVectorArray fg;
  if (fd_hess_flag) {
    if (fd_hess_by_grad_flag) {
      fg.resize(ifg = nfg*num_deriv_vars);
      while(ifg > 0)
        fg[--ifg].resize(num_deriv_vars);
    }
    new_fn_hessians.resize(numFns);
    for (i=0; i<numFns; i++) {
      new_fn_hessians[i].reshape(num_deriv_vars);
      new_fn_hessians[i] = 0.;
    }
  }

  // Get non-finite-diff. portion of the response from 1st fd_responses
  // or from a DB capture in estimate_derivatives()
  bool initial_map = initialMapList.front(); initialMapList.pop_front();
  bool db_capture  = dbCaptureList.front();  dbCaptureList.pop_front();
  Response initial_map_response;
  IntRespMCIter fd_resp_cit = fd_responses.begin();
  if (initial_map) {
    initial_map_response = fd_resp_cit->second; 
    ++fd_resp_cit;
  }
  else if (db_capture) {
    initial_map_response = dbResponseList.front(); 
    dbResponseList.pop_front();
  }
  else { // construct an empty initial_map_response
    ShortArray asv(numFns, 0);
    ActiveSet initial_map_set(asv, original_dvv);
    initial_map_response
      = Response(currentResponse.shared_data(), initial_map_set);
  }

  if (fd_hess_flag && fd_hess_by_fn_flag && !centralHess) {
    dx.resize(num_deriv_vars);
    fx.resize(num_deriv_vars);
  }

  // Postprocess the finite difference responses
  if (fd_grad_flag || fd_hess_flag) {
    SizetMultiArray cv_ids;
    if (original_dvv == currentVariables.continuous_variable_ids()) {
      cv_ids.resize(boost::extents[cv()]);
      cv_ids = currentVariables.continuous_variable_ids();
    }
    else if (original_dvv ==
             currentVariables.inactive_continuous_variable_ids()) {
      cv_ids.resize(boost::extents[icv()]);
      cv_ids = currentVariables.inactive_continuous_variable_ids();
    }
    else { // general derivatives
      cv_ids.resize(boost::extents[acv()]);
      cv_ids = currentVariables.all_continuous_variable_ids();
    }
    const RealVector& fn_vals_x0  = initial_map_response.function_values();
    const RealMatrix& fn_grads_x0 = initial_map_response.function_gradients();
    for (j=0; j<num_deriv_vars; j++) {
      size_t xj_index = find_index(cv_ids, original_dvv[j]);

      if (fd_grad_flag) { // numerical gradients
        Real h = deltaList.front(); deltaList.pop_front();// first in, first out

        if (h == 0.) // lower bound == upper bound; report 0 gradient
          for (i=0; i<numFns; i++)
            new_fn_grads(j,i) = 0.;
        else {
          const RealVector& fn_vals_x_plus_h
            = fd_resp_cit->second.function_values();
          ++fd_resp_cit;
          if (intervalType == "central") {
            Real h1, h12, h2, h22;
            const RealVector& fn_vals_x_minus_h
              = fd_resp_cit->second.function_values();
            ++fd_resp_cit;
            h2 = deltaList.front(); deltaList.pop_front();
            // no need to check fd_grad_asv since it was used for both map calls
            if (h + h2 == 0.) {
              h1 = h - h2;
              for (i=0; i<numFns; ++i)
                new_fn_grads(j,i)
                  = (fn_vals_x_plus_h[i] - fn_vals_x_minus_h[i])/h1;
            }
            else {
              h12 = h*h;
              h22 = h2*h2;
              h1  = h*h2*(h2-h);
              for (i=0; i<numFns; ++i)
                new_fn_grads(j,i)
                  = ( h22*(fn_vals_x_plus_h[i]  - fn_vals_x0[i]) -
                      h12*(fn_vals_x_minus_h[i] - fn_vals_x0[i]) ) / h1;
            }
          }
          else {
            for (i=0; i<numFns; i++)
              // prevent erroneous difference of vals present in fn_vals_x0 but
              // not in fn_vals_x_plus_h due to map_asv & fd_grad_asv diffs
              if (fd_grad_asv[i])
                new_fn_grads(j,i) = (fn_vals_x_plus_h[i] - fn_vals_x0[i])/h;
          }
        }
      }

      if (fd_hess_flag) { // numerical Hessians

        if (fd_hess_by_fn_flag) { // 2nd-order function differences
          if (centralHess) {
            Real h = deltaList.front(); deltaList.pop_front();// 1st in, 1st out

            // diagonal term

            const RealVector& fn_vals_x_plus_2h
              = fd_resp_cit->second.function_values();
            ++fd_resp_cit;
            const RealVector& fn_vals_x_minus_2h
              = fd_resp_cit->second.function_values();
            ++fd_resp_cit;
            for (i=0; i<numFns; i++)
              // prevent erroneous difference of vals present in fn_vals_x0 but
              // not in fn_vals_x_(plus/minus)_2h due to map/fd_hess asv diffs
              if (fd_hess_asv[i] & 1)
                new_fn_hessians[i](j,j) = 
                  (fn_vals_x_plus_2h[i] - 2.*fn_vals_x0[i] + 
                   fn_vals_x_minus_2h[i])/(4.*h*h);

            // off-diagonal terms

            for (k=j+1; k<num_deriv_vars; k++) {
              size_t xk_index = find_index(cv_ids, original_dvv[k]);
              const RealVector& fn_vals_x_plus_h_plus_h
                = fd_resp_cit->second.function_values();
              ++fd_resp_cit;
              const RealVector& fn_vals_x_plus_h_minus_h
                = fd_resp_cit->second.function_values();
              ++fd_resp_cit;
              const RealVector& fn_vals_x_minus_h_plus_h
                = fd_resp_cit->second.function_values();
              ++fd_resp_cit;
              const RealVector& fn_vals_x_minus_h_minus_h
                = fd_resp_cit->second.function_values();
              ++fd_resp_cit;
              for (i=0; i<numFns; i++)
                // no need to check fd_hess_asv since it was used for each eval
                // NOTE: symmetry is naturally satisfied.
                new_fn_hessians[i](j,k)
                  = new_fn_hessians[i](k,j)
                  = (fn_vals_x_plus_h_plus_h[i]  - fn_vals_x_plus_h_minus_h[i]
                  -  fn_vals_x_minus_h_plus_h[i] + fn_vals_x_minus_h_minus_h[i])
                  / (4.*h*h);
            }
          }
          else { //!!
            Real denom, h1, h2, hdiff;
            const RealVector *fx1, *fx12, *fx2;

            dx[j] = h1 = deltaList.front(); deltaList.pop_front();
            h2 = deltaList.front(); deltaList.pop_front();
            fx[j] = fx1 = &fd_resp_cit->second.function_values();
            ++fd_resp_cit;
            fx2 = &fd_resp_cit->second.function_values();
            ++fd_resp_cit;
            if (h1 + h2 == 0.) {
              denom = h1*h1;
              for(i = 0; i < numFns; ++i)
                if (fd_hess_asv[i] & 1)
                  new_fn_hessians[i](j,j)
                    = ((*fx1)[i] - 2.*fn_vals_x0[i] + (*fx2)[i]) / denom;
            }
            else {
              hdiff = h1 - h2;
              denom = 0.5*h1*h2*hdiff;
              for (i = 0; i < numFns; i++)
                if (fd_hess_asv[i] & 1)
                  new_fn_hessians[i](j,j)
                    = (h2*(*fx1)[i] + hdiff*fn_vals_x0[i] - h1*(*fx2)[i])/denom;
            }

            // off-diagonal terms

            for(k = 0; k < j; ++k) {
              size_t xk_index = find_index(cv_ids, original_dvv[k]);
              h2 = dx[k];
              denom = h1*h2;
              fx2 = fx[k];
              fx12 = &fd_resp_cit->second.function_values();
              ++fd_resp_cit;
              for (i = 0; i < numFns; i++)
                new_fn_hessians[i](j,k) =
                  ((*fx12)[i] - (*fx1)[i] - (*fx2)[i] + fn_vals_x0[i]) / denom;
            }
          }
        }
        if (fd_hess_by_grad_flag) { // 1st-order gradient differences
          Real h = deltaList.front(); deltaList.pop_front(); // 1st in, 1st out

          const RealMatrix& fn_grads_x_plus_h
            = fd_resp_cit->second.function_gradients();
          ++fd_resp_cit;
          ifg = j;
          for (i=0; i<numFns; i++)
            // prevent erroneous difference of grads present in fn_grads_x0 but
            // not in fn_grads_x_plus_h due to map_asv & fd_hess_asv diffs
            // NOTE: symmetry must be enforced below.
            if (fd_hess_asv[i] & 2) {
              //fg[ifg] = (fn_grads_x_plus_h[i] - fn_grads_x0[i]) / h;
              for(k = 0; k < num_deriv_vars; ++k)
                fg[ifg][k] = (fn_grads_x_plus_h[i][k] - fn_grads_x0[i][k]) / h;
              ifg += num_deriv_vars;
            }
        }
      }
    }
  }

  // Enforce symmetry in the case of FD Hessians from 1st-order gradient
  // differences by averaging off-diagonal terms: H' = 1/2 (H + H^T)
  if (fd_hess_by_grad_flag)
    for (i=0; i<numFns; i++)
      if (fd_hess_asv[i] & 2)
        for (i = ifg = 0; i < numFns; i++)
          if (fd_hess_asv[i] & 2) {
            for (j=0; j<num_deriv_vars; j++) {
              for (k = 0; k < j; k++)
                new_fn_hessians[i](j,k) = 0.5 * (fg[ifg+j][k] + fg[ifg+k][j]);
              new_fn_hessians[i](j,j) = fg[ifg+j][j];
            }
            ifg += num_deriv_vars;
          }

  update_response(vars, new_response, fd_grad_asv, fd_hess_asv, quasi_hess_asv,
                  original_set, initial_map_response, new_fn_grads,
                  new_fn_hessians);
}


/** Overlay the initial_map_response with numerically estimated new_fn_grads
    and new_fn_hessians to populate new_response as governed by asv vectors.
    Quasi-Newton secant Hessian updates are also performed here, since this
    is where the gradient data needed for the updates is first consolidated.
    Convenience function used by estimate_derivatives() for the synchronous
    case and by synchronize_derivatives() for the asynchronous case. */
void Model::
update_response(const Variables& vars, Response& new_response,
		const ShortArray& fd_grad_asv, const ShortArray& fd_hess_asv,
		const ShortArray& quasi_hess_asv, const ActiveSet& original_set,
		Response& initial_map_response, const RealMatrix& new_fn_grads,
		const RealSymMatrixArray& new_fn_hessians)
{
  // ----------
  // Initialize
  // ----------

  // If estDerivsFlag is set, then new_response was initially built with its
  // default constructor and has a NULL rep.  In this case, allocate a new
  // response by copying currentResponse.  This is a convenient way to carry
  // over fnTags and interfaceId to the new response.
  if (new_response.is_null())
    new_response = currentResponse.copy();

  size_t i;
  bool initial_map = false, initial_map_fn_flag = false,
       initial_map_grad_flag = false, initial_map_hess_flag = false,
       fd_grad_flag = false, fd_hess_flag = false, quasi_hess_flag = false;
  const ShortArray& initial_map_asv
    = initial_map_response.active_set_request_vector();
  for (i=0; i<numFns; i++) {
    if (initial_map_asv[i]) {
      initial_map = true;
      if (initial_map_asv[i] & 1)
	initial_map_fn_flag = true;
      if (initial_map_asv[i] & 2)
	initial_map_grad_flag = true;
      if (initial_map_asv[i] & 4)
	initial_map_hess_flag = true;
    }
    if (fd_grad_asv[i])
      fd_grad_flag = true; // gradient finite differencing used
    if (fd_hess_asv[i])
      fd_hess_flag = true; // Hessian finite differencing used
    if (quasi_hess_asv[i])
      quasi_hess_flag = true; // quasi-Hessians needed in new_response
  }

  // ----------------------
  // Update function values
  // ----------------------

  if (initial_map_fn_flag)
    new_response.function_values(initial_map_response.function_values());
  // else reset_inactive() zero's out all of the function values

  // -------------------------
  // Update function gradients
  // -------------------------

  if (initial_map) {
    if (fd_grad_flag) { // merge new_fn_grads with initial map grads
      RealMatrix partial_fn_grads;
      if (initial_map_grad_flag)
	partial_fn_grads = initial_map_response.function_gradients();
      else
	partial_fn_grads.shape(new_fn_grads.numRows(), new_fn_grads.numCols());
      for (i=0; i<numFns; ++i) {
	// overwrite partial_fn_grads with new_fn_grads based on fd_grad_asv
	// (needed for case of mixed gradients)
	if (fd_grad_asv[i]) {
	  RealVector new_fn_gradi_col_vector = Teuchos::getCol(Teuchos::View,
	    const_cast<RealMatrix&>(new_fn_grads), (int)i);
	  Teuchos::setCol(new_fn_gradi_col_vector, (int)i, partial_fn_grads);
        }
      }
      new_response.function_gradients(partial_fn_grads);
    }
    else if (initial_map_grad_flag) // no merge: initial map grads are complete
      new_response.function_gradients(
        initial_map_response.function_gradients());
  }
  else if (fd_grad_flag) // no merge: new_fn_grads are complete
    new_response.function_gradients(new_fn_grads);

  // ------------------------
  // Update function Hessians
  // ------------------------

  // perform quasi-Newton updates if quasi_hessians have been specified by the
  // user, the response data is uncorrected, and the DVV is set to the active
  // continuous variables (the default).
  if ( supportsEstimDerivs && 
       surrogate_response_mode() != AUTO_CORRECTED_SURROGATE &&
       original_set.derivative_vector() ==
       currentVariables.continuous_variable_ids() &&
       ( hessianType == "quasi" ||
	 ( hessianType == "mixed" && !hessIdQuasi.empty() ) ) )
    update_quasi_hessians(vars, new_response, original_set);

  // overlay Hessian data as needed
  if (initial_map || hessianType == "mixed") {
    // merge initial map Hessians, new_fn_hessians, and quasiHessians
    if (fd_hess_flag || quasi_hess_flag) {
      RealSymMatrixArray partial_fn_hessians;
      if (initial_map_hess_flag)
	partial_fn_hessians = initial_map_response.function_hessians();
      else
	partial_fn_hessians.resize(numFns);
      for (i=0; i<numFns; i++) {
	if (fd_hess_asv[i])    // overwrite with FD Hessians
	  partial_fn_hessians[i] = new_fn_hessians[i]; // full matrix
	if (quasi_hess_asv[i]) // overwrite with quasi-Hessians
	  partial_fn_hessians[i] = quasiHessians[i];   // full matrix
      }
      new_response.function_hessians(partial_fn_hessians);
    }
    else if (initial_map_hess_flag)
      new_response.function_hessians(initial_map_response.function_hessians());
  }
  else if (fd_hess_flag)    // no merge necessary
    new_response.function_hessians(new_fn_hessians);
  else if (quasi_hess_flag) // no merge necessary
    new_response.function_hessians(quasiHessians);

  // --------
  // Finalize
  // --------

  // update newly rebuilt new_response with the original asv request
  new_response.active_set_request_vector(original_set.request_vector());
  new_response.reset_inactive(); // clear out any left overs

  // Output header for the rebuilt new_response
  if (outputLevel > QUIET_OUTPUT) {
    if (initial_map)
      Cout << ">>>>> Total response returned to iterator:\n\n";
    else
      Cout << ">>>>> Gradients returned to iterator:\n\n";
    Cout << new_response << std::endl;
  }
}


/** quasi-Newton updates are performed for approximating response
    function Hessians using BFGS or SR1 formulations.  These Hessians
    are supported only for the active continuous variables, and a
    check is performed on the DVV prior to invoking the function. */
void Model::
update_quasi_hessians(const Variables& vars, Response& new_response,
		      const ActiveSet& original_set)
{
  size_t i, j, k;
  const RealVector& x       = vars.continuous_variables(); // view
  const ShortArray& original_asv = original_set.request_vector();
  const RealMatrix& fn_grads     = new_response.function_gradients();

  // if necessary, initialize quasi-Hessians before populating
  if ( numQuasiUpdates.empty() ) {
    if (outputLevel == DEBUG_OUTPUT)
      Cout << "Reshaping quasiHessians in modelType = " << modelType
	   << std::endl;
    xPrev.resize(numFns);
    fnGradsPrev.reshape(numDerivVars, numFns);
    quasiHessians.resize(numFns);
    for (size_t i=0; i<numFns; i++) {
      if ( hessianType == "quasi" ||
	   ( hessianType == "mixed" && contains(hessIdQuasi, i+1) ) ) {
	quasiHessians[i].reshape(numDerivVars);
	quasiHessians[i] = 0.;
	// leave as zero matrix so that any early use of quasi-Hessian has
	// no effect.  The initial scaling updates to nonzero values at the
	// appropriate time.
	//for (size_t j=0; j<numDerivVars; j++)
	//  quasiHessians[i][j][j] = 1.; // initialize to identity
      }
    }
    numQuasiUpdates.assign(numFns, 0);
  }

  for (i=0; i<numFns; ++i) {

    // perform i-th quasi-Hessian update if a new grad vector is present,
    // regardless of whether the quasi-Hessian is active on this eval.
    if ( !quasiHessians[i].empty() && (original_asv[i] & 2) ) {

      // quasi-Hessian updates require a history of at least 2 gradient evals
      Real norm_s = 0.;
      if (numQuasiUpdates[i]) {

	RealVector s(numDerivVars), y(numDerivVars);
	Real sj, yj, norm_s_sq = 0., norm_y_sq = 0.;
	for (j=0; j<numDerivVars; ++j) {
	  s[j] = sj = x[j]           - xPrev[i][j];       // s = step
	  y[j] = yj = fn_grads[i][j] - fnGradsPrev[i][j]; // y = yield
	  norm_s_sq += sj*sj; // avoid repeated indexing
	  norm_y_sq += yj*yj; // avoid repeated indexing
	}
	norm_s = std::sqrt(norm_s_sq);
	Real norm_y = std::sqrt(norm_y_sq);
	if (outputLevel == DEBUG_OUTPUT)
	  Cout << "Quasi-Hessian step/yield:\ns =\n" << s << "y =\n" << y
	       << "norm_s = " << norm_s << " norm_y = " << norm_y << '\n';

	// Verify that there's a non-zero step (zero yield is acceptable).
	// Don't update anything (including history) if step is zero.
	if (norm_s > Pecos::SMALL_NUMBER) {

	  // -------------------------------------------
	  // Apply initial scaling prior to first update
	  // -------------------------------------------
	  //   Gay:             y's/s's I  (scaling1/norm_s_sq)
	  //   Dennis/Schnabel: y's/s'Hs I (same as Gay if initial H = I)
	  //   Shanno/Phua:     y'y/y's I  (current selection)
	  if (numQuasiUpdates[i] == 1) {
	    Real scaling1 = 0.;
	    for (j=0; j<numDerivVars; j++)
	      scaling1 += y[j]*s[j];
	    // step is nonzero but yield may reasonably be zero, so safeguard
	    // numerics.  In the case of no yield in gradients (no observed
	    // curvature), 0 is assigned as the initial scaling.
	    Real scaling2 = 0.;
	    if (norm_y > Pecos::SMALL_NUMBER)
	      scaling2 = (std::sqrt(std::fabs(scaling1)) > Pecos::SMALL_NUMBER)
                       ? norm_y_sq/scaling1 : 1.;
	    for (j=0; j<numDerivVars; j++)
	      quasiHessians[i](j,j) = scaling2;
	    if (outputLevel == DEBUG_OUTPUT)
	      Cout << "initial scaling =\n" << quasiHessians[i] << '\n';
	  }

	  // ----------------------
	  // Symmetric Rank 1 (SR1)
	  // ----------------------
	  if (quasiHessType == "sr1") {
	    RealVector ymBs(numDerivVars);
	    Real scaling = 0.;
	    for (j=0; j<numDerivVars; j++) {
	      Real Bs = 0.;
	      for (k=0; k<numDerivVars; k++)
		Bs += quasiHessians[i](j,k)*s[k];
	      ymBs[j] = y[j] - Bs;
	      scaling += ymBs[j]*s[j];
	    }
	    // use standard safeguard against orthogonality in denominator.
	    // skip update if denominator magnitude is insufficient.  Guanghui
	    // Liu thesis uses 1.e-4 (p. 57) and Nocedal and Wright uses 1.e-8
	    // (pp. 203-4).  Here we split the difference with 1.e-6.
	    Real norm_ymBs = 0.;
	    for (j=0; j<numDerivVars; j++)
	      norm_ymBs += ymBs[j]*ymBs[j];
	    norm_ymBs = std::sqrt(norm_ymBs);
	    // perform update
	    if (std::fabs(scaling) > 1.e-6*norm_s*norm_ymBs) {
	      for (j=0; j<numDerivVars; j++) {
		// exploit hereditary symmetry: compute upper half of matrix
		quasiHessians[i](j,j) += ymBs[j]*ymBs[j]/scaling;
		for (k=j+1; k<numDerivVars; k++)
		  quasiHessians[i](k,j) = quasiHessians[i](j,k) +=
		    ymBs[j]*ymBs[k]/scaling;
	      }
	      if (outputLevel == DEBUG_OUTPUT)
		Cout << "SR1: B = " << quasiHessians[i] << '\n';
	    }
	    else if (outputLevel == DEBUG_OUTPUT)
	      Cout << "SR1: update skipped\n";
	  }
	  // ---------------------------------------
	  // Broyden-Fletcher-Goldfarb-Shanno (BFGS)
	  // ---------------------------------------
	  else {
	    bool damped_bfgs = (quasiHessType == "damped_bfgs") ? true : false;
	    RealVector Bs(numDerivVars, true);
	    Real scaling1 = 0., scaling2 = 0.;
	    for (j=0; j<numDerivVars; j++) {
	      scaling1 += y[j]*s[j];
	      for (k=0; k<numDerivVars; k++)
		Bs[j] += quasiHessians[i](j,k)*s[k];
	      scaling2 += s[j]*Bs[j];
	    }
	    // These two approaches appear in the literature for enforcing the
	    // curvature condition when the steps are generated by a Newton
	    // optimizer:
	    //   damp: if (scaling1 >=  0.2*scaling2) { update } else { damp }
	    //   skip: if (scaling1 >= 1e-6*scaling2) { update }
	    // These approaches appear to be inappropriate when the steps are
	    // not directly Newton-generated (e.g., from trust-region SBO) since
	    // the curvature condition is violated too frequently.  In this
	    // case, a minimal approach can be used which only protects the
	    // denominator magnitude.  The BFGS update can lose positive
	    // definiteness in this case -> use fabs on scaling2 triple product.
	    using std::fabs;
	    if ( ( !damped_bfgs && fabs(scaling1) > 1.e-6*fabs(scaling2) ) ||
		 (  damped_bfgs && scaling1 >= 0.2*scaling2 ) ) {
	      // standard BFGS (in damped formulation: theta=1 and r=y)
	      for (j=0; j<numDerivVars; j++) {
		// exploit hereditary symmetry: compute upper half of matrix
		quasiHessians[i](j,j) +=  y[j]* y[j]/scaling1
                                       -  Bs[j]*Bs[j]/scaling2;
		for (k=j+1; k<numDerivVars; k++)
		  quasiHessians[i](k,j) = quasiHessians[i](j,k) +=
		    y[j]*y[k]/scaling1 - Bs[j]*Bs[k]/scaling2;
	      }
	      if (outputLevel == DEBUG_OUTPUT)
		Cout << "BFGS: y's = " << scaling1 << " s'Bs = " << scaling2
		     << "\nB = " << quasiHessians[i] << '\n';
	    }
	    else if (damped_bfgs) {
	      // damped BFGS is a standard safeguard against violation of the
	      // curvature condition (y's should be > 0).  See Nocedal & Wright
	      // (pp. 540-1) or Guanghui Liu thesis (p. 57), among others.
	      RealVector r(numDerivVars);
	      Real theta = 0.8*scaling2/(scaling2 - scaling1);
	      //Cout << "Damped BFGS:   y's = " << scaling1 << " s'Bs = "
              //     << scaling2 << " theta = " << theta;
	      scaling1 = 0.;
	      for (j=0; j<numDerivVars; j++) {
		r[j] = theta*y[j] + (1.-theta)*Bs[j];
		scaling1 += r[j]*s[j];
	      }
	      for (j=0; j<numDerivVars; j++) {
		// exploit hereditary symmetry: compute upper half of matrix
		quasiHessians[i](j,j) +=  r[j]* r[j]/scaling1
                                       -  Bs[j]*Bs[j]/scaling2;
		for (k=j+1; k<numDerivVars; k++)
		  quasiHessians[i](k,j) = quasiHessians[i](j,k) +=
		    r[j]*r[k]/scaling1 - Bs[j]*Bs[k]/scaling2;
	      }
	      if (outputLevel == DEBUG_OUTPUT)
		Cout << "Damped BFGS: r's = " << scaling1 << "\nB = "
		     << quasiHessians[i] << '\n';
	    }
	    else if (outputLevel == DEBUG_OUTPUT)
	      Cout << "Undamped BFGS: update skipped\n";
	  }
	}
      }

      // history updates for next iteration
      if ( numQuasiUpdates[i] == 0 || norm_s > Pecos::SMALL_NUMBER ) {
	// store previous data independently for each response fn.  So long
	// as at least one previous data pt has been stored, we do not need
	// to track the presence of active grads in particular responses.
	copy_data(x, xPrev[i]); // view->copy
	RealVector tmp_col_vector = Teuchos::getCol(Teuchos::View,
	  const_cast<RealMatrix&>(fn_grads), (int)i);
	Teuchos::setCol(tmp_col_vector, (int)i, fnGradsPrev);
	++numQuasiUpdates[i];
      }
    }
  }
}


Real Model::
finite_difference_lower_bound(UShortMultiArrayConstView cv_types,
			      const RealVector& global_c_l_bnds,
			      size_t cv_index) const
{
  // replace inferred lower bounds for unbounded distributions
  switch (cv_types[cv_index]) {
  case NORMAL_UNCERTAIN: {    // -infinity or user-specified
    size_t n_index = cv_index -
      find_index(cv_types, (unsigned short)NORMAL_UNCERTAIN);
    return aleatDistParams.normal_lower_bound(n_index);    break;
  }
  case LOGNORMAL_UNCERTAIN: { // 0 or user-specified
    size_t ln_index = cv_index -
      find_index(cv_types, (unsigned short)LOGNORMAL_UNCERTAIN);
    return aleatDistParams.lognormal_lower_bound(ln_index); break;
  }
  case GUMBEL_UNCERTAIN:      // -infinity
    return -std::numeric_limits<Real>::infinity();  break;
  default:
    return global_c_l_bnds[cv_index];               break;
  }
}


Real Model::
finite_difference_upper_bound(UShortMultiArrayConstView cv_types,
			      const RealVector& global_c_u_bnds,
			      size_t cv_index) const
{
  // replace inferred upper bounds for unbounded/semi-bounded distributions
  switch (cv_types[cv_index]) {
  case NORMAL_UNCERTAIN: {    // infinity or user-specified
    size_t n_index = cv_index -
      find_index(cv_types, (unsigned short)NORMAL_UNCERTAIN);
    return aleatDistParams.normal_upper_bound(n_index);    break;
  }
  case LOGNORMAL_UNCERTAIN: { // infinity or user-specified
    size_t ln_index = cv_index -
      find_index(cv_types, (unsigned short)LOGNORMAL_UNCERTAIN);
    return aleatDistParams.lognormal_upper_bound(ln_index); break;
  }
  case EXPONENTIAL_UNCERTAIN: case GAMMA_UNCERTAIN: // infinity
  case GUMBEL_UNCERTAIN:      case FRECHET_UNCERTAIN: case WEIBULL_UNCERTAIN:
    return std::numeric_limits<Real>::infinity();   break;
  default:
    return global_c_u_bnds[cv_index];               break;
  }
}


/** Splits asv_in total request into map_asv_out, fd_grad_asv_out,
    fd_hess_asv_out, and quasi_hess_asv_out as governed by the
    responses specification.  If the returned use_est_deriv is true,
    then these asv outputs are used by estimate_derivatives() for the
    initial map, finite difference gradient evals, finite difference
    Hessian evals, and quasi-Hessian updates, respectively.  If the
    returned use_est_deriv is false, then only map_asv_out is used. */
bool Model::manage_asv(const ActiveSet& original_set, ShortArray& map_asv_out,
		       ShortArray& fd_grad_asv_out, ShortArray& fd_hess_asv_out,
		       ShortArray& quasi_hess_asv_out)
{
  const ShortArray& asv_in = original_set.request_vector();
  const SizetArray& original_dvv = original_set.derivative_vector();
  
  // *_asv_out[i] have all been initialized to zero

  // For HierarchSurr and Recast models with no scaling (which contain no
  // interface object, only subordinate models), pass the ActiveSet through to
  // the sub-models in one piece and do not break it apart here. This preserves
  // sub-model parallelism.
  if (!supportsEstimDerivs)
    return false;

  bool use_est_deriv = false, fd_grad_flag = false;
  size_t i, asv_len = asv_in.size();
  for (i=0; i<asv_len; ++i) {

    // Function value requests
    if (asv_in[i] & 1)
      map_asv_out[i] = 1;

    // Function gradient requests
    if (asv_in[i] & 2) {
      if ( gradientType == "analytic" ||
           ( gradientType == "mixed" && contains(gradIdAnalytic, i+1) ) )
        map_asv_out[i] |= 2; // activate 2nd bit
      else if ( methodSource == "dakota" && ( gradientType == "numerical" ||
		( gradientType == "mixed" && contains(gradIdNumerical, i+1)))) {
        fd_grad_asv_out[i] = 1;
        fd_grad_flag = true;
        if (intervalType == "forward")
          map_asv_out[i] |= 1; // activate 1st bit
        use_est_deriv = true;
      }
      else { // could happen if an iterator requiring gradients is selected
        // with no_gradients or unsupported vendor numerical gradients
        // and lacks a separate error check.
        Cerr << "Error: unsupported asv gradient request in Model::manage_asv."
             << std::endl;
        abort_handler(MODEL_ERROR);
      }
      if ( surrogate_response_mode() != AUTO_CORRECTED_SURROGATE &&
           ( hessianType == "quasi" ||
             ( hessianType == "mixed" && contains(hessIdQuasi, i+1) ) ) )
        use_est_deriv = true;
    }

    // Function Hessian requests
    if (asv_in[i] & 4) {
      if ( hessianType == "analytic" ||
           ( hessianType == "mixed" && contains(hessIdAnalytic, i+1) ) )
        map_asv_out[i] |= 4; // activate 3rd bit
      else if ( hessianType == "numerical" ||
                ( hessianType == "mixed" && contains(hessIdNumerical, i+1) ) ) {
        if ( gradientType == "analytic" ||
             ( gradientType == "mixed" && contains(gradIdAnalytic, i+1) ) ) {
          // numerical Hessians from 1st-order gradient differences
          fd_hess_asv_out[i] = 2;
          map_asv_out[i] |= 2; // activate 2nd bit
        }
        else { // numerical Hessians from 2nd-order function differences
          fd_hess_asv_out[i] = 1;
          map_asv_out[i] |= 1; // activate 1st bit
        }
        use_est_deriv = true;
      }
      else if ( hessianType == "quasi" ||
                (hessianType == "mixed" && contains(hessIdQuasi, i+1))) {
        quasi_hess_asv_out[i] = 2; // value not currently used
        use_est_deriv = true; // update_response needed even if no secant update
      }
      else { // could happen if an iterator requiring Hessians is selected
        // with no_hessians and it lacks a separate error check.
        Cerr << "Error: unsupported asv Hessian request in Model::manage_asv."
             << std::endl;
        abort_handler(MODEL_ERROR);
      }
    }
  }

  // Depending on bounds-respecting differencing, finite difference gradients
  // may require f(x0).  The following computes the step and updates shortStep.
  if (fd_grad_flag && !ignoreBounds) { // protect call to forward_grad_step
    size_t num_deriv_vars = original_dvv.size();

    // define x0 and mode flags
    bool active_derivs = false;    // derivatives w.r.t. active vars
    bool inactive_derivs = false;  // derivs w.r.t. inactive vars
    RealVector x0;

    // define lower/upper bounds for finite differencing and cv_ids
    RealVector fd_lb(num_deriv_vars), fd_ub(num_deriv_vars);
    SizetMultiArrayConstView cv_ids = 
      initialize_x0_bounds(original_dvv, active_derivs, inactive_derivs,
                           x0, fd_lb, fd_ub);

    // Accumulate short step over all derivative variables
    bool short_step = false;
    for (size_t j=0; j<num_deriv_vars; j++) {
      size_t xj_index = find_index(cv_ids, original_dvv[j]);
      Real x0_j = x0[xj_index], lb_j = fd_lb[j], ub_j = fd_ub[j];
      
      // NOTE: resets shortStep to false for each variable
      Real h = forward_grad_step(num_deriv_vars, xj_index, x0_j, lb_j, ub_j);
      if (intervalType == "central")
        Real h2 = FDstep2(x0_j, lb_j, ub_j, h);
      
      if (shortStep)
        short_step = true;
    }
    
    // update ASV with f(x0) requests needed for shortStep
    for (i=0; i<asv_len; ++i)
      if ( (fd_grad_asv_out[i] & 1) && short_step) 
        map_asv_out[i] |= 1; // activate 1st bit
  }

  return use_est_deriv;
}


/** Constructor helper to manage model recastings for data import/export. */
bool Model::manage_data_recastings()
{
  if (modelRep) // should not occur: protected fn only used by the letter
    return modelRep->manage_data_recastings(); // fwd to letter
  else { // letter lacking redefinition of virtual fn.
    // Test for any recasting or nesting within actualModel: we assume that
    // user data import is post-nesting, but pre-recast.
    // (1) data is imported at the user-space level but then must be applied
    //     within the transformed space.  Transform imported data at run time
    //     in order to capture latest initialize() calls to RecastModels.
    // (2) stop the recursion if a nested model is encountered: we will apply
    //     any recastings that occur following the last nesting. 
    // (3) Additional surrogates in this recursion hierarchy are ignored.
    ModelList& sub_models = subordinate_models(); // populates/returns modelList
    ModelLIter ml_it; size_t i, num_models = sub_models.size();
    bool manage_recasting = false;
    recastFlags.assign(num_models, false);
    // detect recasting needs top down
    for (ml_it=sub_models.begin(), i=0; ml_it!=sub_models.end(); ++ml_it, ++i)
      if (ml_it->model_type()      == "recast")
	manage_recasting = recastFlags[i] = true;
      else if (ml_it->model_type() == "nested")
	break;

    if (!manage_recasting) recastFlags.clear();
    return manage_recasting;
  }
}


void Model::
user_space_to_iterator_space(const Variables& user_vars,
			     const Response&  user_resp,
			     Variables& iter_vars, Response& iter_resp)
{
  if (modelRep) // fwd to letter
    return modelRep->user_space_to_iterator_space(user_vars, user_resp,
						  iter_vars, iter_resp);
  else { // letter lacking redefinition of virtual fn.

    iter_vars = user_vars; iter_resp = user_resp; // shallow copies

    // apply recastings bottom up (e.g., for data import)
    // modelList assigned in manage_data_recastings() -> subordinate_models()
    // (don't want to incur this overhead for every import/export)
    ModelLRevIter ml_rit; size_t i;
    for (ml_rit =modelList.rbegin(), i=modelList.size()-1;
	 ml_rit!=modelList.rend(); ++ml_rit, --i)
      if (recastFlags[i]) {
	// utilize RecastModel::current{Variables,Response} to xform data
	Variables recast_vars = ml_rit->current_variables(); // shallow copy
	Response  recast_resp = ml_rit->current_response();  // shallow copy
	// to propagate vars bottom up, inverse of std transform is reqd
	RecastModel* recast_model_rep = (RecastModel*)ml_rit->model_rep();
	recast_model_rep->inverse_transform_variables(iter_vars, recast_vars);
	//recast_model_rep->
	//  inverse_transform_set(iter_vars, iter_set, recast_set);
	// to propagate response bottom up, std transform is used
	recast_model_rep->
	  transform_response(recast_vars, iter_vars, iter_resp, recast_resp);
	// reassign rep pointers (no actual data copying)
	iter_vars = recast_vars; iter_resp = recast_resp;// iter_set=recast_set;
      }
  }
}


void Model::
iterator_space_to_user_space(const Variables& iter_vars,
			     const Response&  iter_resp,
			     Variables& user_vars, Response& user_resp)
{
  if (modelRep) // fwd to letter
    return modelRep->iterator_space_to_user_space(iter_vars, iter_resp,
						  user_vars, user_resp);
  else { // letter lacking redefinition of virtual fn.

    user_vars = iter_vars; user_resp = iter_resp; // shallow copies

    // apply recastings top down (e.g., for data export)
    // modelList assigned in manage_data_recastings() -> subordinate_models()
    // (don't want to incur this overhead for every import/export)
    ModelLIter ml_it; size_t i;
    for (ml_it=modelList.begin(), i=0; ml_it!=modelList.end(); ++ml_it, ++i)
      if (recastFlags[i]) {
	// utilize RecastModel::current{Variables,Response} to xform data
	Variables recast_vars = ml_it->current_variables(); // shallow copy
	Response  recast_resp = ml_it->current_response();  // shallow copy
	// to propagate vars top down, forward transform is reqd
	RecastModel* recast_model_rep = (RecastModel*)ml_it->model_rep();
	recast_model_rep->transform_variables(user_vars, recast_vars);
	//recast_model_rep->transform_set(user_vars, user_set, recast_set);
	// to propagate response top down, inverse transform is used.  Note:
	// derivatives are not currently exported --> a no-op for Nataf.
	recast_model_rep->inverse_transform_response(recast_vars, user_vars,
						     user_resp, recast_resp);
	// reassign rep pointers (no actual data copying)
	user_vars = recast_vars; user_resp = recast_resp;// user_set=recast_set;
      }
  }
}


void Model::derived_evaluate(const ActiveSet& set)
{
  if (modelRep) // should not occur: protected fn only used by the letter
    modelRep->derived_evaluate(set); // envelope fwd to letter
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual derived_compute_"
         << "response() function.\nNo default defined at base class."
	 << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::derived_evaluate_nowait(const ActiveSet& set)
{
  if (modelRep) // should not occur: protected fn only used by the letter
    modelRep->derived_evaluate_nowait(set); // envelope fwd to letter
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual derived_asynch_"
         << "evaluate() function.\nNo default defined at base class."
         << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


const IntResponseMap& Model::derived_synchronize()
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual derived_synchronize"
         << "() function.\n       derived_synchronize is not available for this"
	 << " Model." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  // should not occur: protected fn only used by the letter
  return modelRep->derived_synchronize(); // envelope fwd to letter
}


const IntResponseMap& Model::derived_synchronize_nowait()
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual derived_synchronize"
         << "_nowait() function.\n       derived_synchronize_nowait is not "
	 << "available for this Model." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  // should not occur: protected fn only used by the letter
  return modelRep->derived_synchronize_nowait(); // envelope fwd to letter
}


bool Model::derived_master_overload() const
{
  if (modelRep) // should not occur: protected fn only used by the letter
    return modelRep->derived_master_overload(); // envelope fwd to letter
  else // letter lacking redefinition of virtual fn.
    return false; // default for Surrogate models
}


/** return by reference requires use of dummy objects, but is
    important to allow use of assign_rep() since this operation must
    be performed on the original envelope object. */
Iterator& Model::subordinate_iterator()
{
  if (modelRep)
    return modelRep->subordinate_iterator(); // envelope fwd to letter
  else // letter lacking redefinition of virtual fn.
    return dummy_iterator; // return null/empty envelope
}


/** return by reference requires use of dummy objects, but is
    important to allow use of assign_rep() since this operation must
    be performed on the original envelope object. */
Model& Model::subordinate_model()
{
  if (modelRep) // envelope fwd to letter
    return modelRep->subordinate_model();
  else // letter lacking redefinition of virtual fn.
    return dummy_model; // return null/empty envelope
}


/** return by reference requires use of dummy objects, but is
    important to allow use of assign_rep() since this operation must
    be performed on the original envelope object. */
Model& Model::surrogate_model()
{
  if (modelRep) // envelope fwd to letter
    return modelRep->surrogate_model();
  else // letter lacking redefinition of virtual fn.
    return dummy_model; // return null/empty envelope
}


void Model::
surrogate_model_indices(size_t lf_model_index, size_t lf_soln_lev_index)
{
  if (modelRep) // envelope fwd to letter
    modelRep->surrogate_model_indices(lf_model_index, lf_soln_lev_index);
  else {
    Cerr << "Error: Letter lacking redefinition of virtual surrogate_model_"
	 << "indices(size_t, size_t) function.\n       surrogate model "
	 << "activation is not supported by this Model class." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::surrogate_model_indices(const SizetSizetPair& lf_form_level)
{
  if (modelRep) // envelope fwd to letter
    modelRep->surrogate_model_indices(lf_form_level);
  else {
    Cerr << "Error: Letter lacking redefinition of virtual surrogate_model_"
	 << "indices(SizetSizetPair) function.\n       surrogate model "
	 << "activation is not supported by this Model class." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


const SizetSizetPair& Model::surrogate_model_indices() const
{
  if (!modelRep) {
    Cerr << "Error: Letter lacking redefinition of virtual surrogate_model_"
	 << "indices() function.\n       active surrogate model indices are "
	 << "not supported by this Model class." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  return modelRep->surrogate_model_indices();
}


/** return by reference requires use of dummy objects, but is
    important to allow use of assign_rep() since this operation must
    be performed on the original envelope object. */
Model& Model::truth_model()
{
  if (modelRep) // envelope fwd to letter
    return modelRep->truth_model();
  else // letter lacking redefinition of virtual fn.
    return dummy_model; // return null/empty envelope
}


void Model::truth_model_indices(size_t hf_model_index, size_t hf_soln_lev_index)
{
  if (modelRep) // envelope fwd to letter
    modelRep->truth_model_indices(hf_model_index, hf_soln_lev_index);
  else {
    Cerr << "Error: Letter lacking redefinition of virtual truth_model_indices"
	 << "(size_t, size_t) function.\n       truth_model activation is not "
	 << "supported by this Model class." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::truth_model_indices(const SizetSizetPair& hf_form_level)
{
  if (modelRep) // envelope fwd to letter
    modelRep->truth_model_indices(hf_form_level);
  else {
    Cerr << "Error: Letter lacking redefinition of virtual truth_model_indices"
	 << "(SizetSizetPair) function.\n       truth_model activation is not "
	 << "supported by this Model class." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


const SizetSizetPair& Model::truth_model_indices() const
{
  if (!modelRep) {
    Cerr << "Error: Letter lacking redefinition of virtual truth_model_indices"
	 << "() function.\n       active truth_model indices are not supported "
	 << "by this Model class." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  return modelRep->truth_model_indices();
}


/** since modelList is built with list insertions (using envelope
    copies), these models may not be used for model.assign_rep() since
    this operation must be performed on the original envelope object.
    They may, however, be used for letter-based operations (including
    assign_rep() on letter contents such as an interface). */
ModelList& Model::subordinate_models(bool recurse_flag)
{
  if (modelRep) // envelope fwd to letter
    return modelRep->subordinate_models(recurse_flag);
  else { // letter (not virtual)
    modelList.clear();
    derived_subordinate_models(modelList, recurse_flag);
    return modelList;
  }
}


void Model::derived_subordinate_models(ModelList& ml, bool recurse_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->derived_subordinate_models(ml, recurse_flag);
  // else: default implementation (SimulationModel) is no-op.
}


/** used only for instantiate-on-the-fly model recursions (all RecastModel
    instantiations and alternate DataFitSurrModel instantiations).  Simulation,
    Hierarchical, and Nested Models do not redefine the function since they
    do not support instantiate-on-the-fly.  This means that the recursion
    will stop as soon as it encounters a Model that was instantiated normally,
    which is appropriate since ProblemDescDB-constructed Models use top-down
    information flow and do not require bottom-up updating. */
void Model::update_from_subordinate_model(size_t depth)
{
  if (modelRep) // envelope fwd to letter
    modelRep->update_from_subordinate_model(depth);
  // else default if no redefinition is no-op
}


/** return by reference requires use of dummy objects, but is
    important to allow use of assign_rep() since this operation must
    be performed on the original envelope object. */
Interface& Model::derived_interface()
{
  if (modelRep)
    return modelRep->derived_interface(); // envelope fwd to letter
  else // letter lacking redefinition of virtual fn.
    return dummy_interface; // return null/empty envelope
}


/** return the number of levels within a solution / discretization hierarchy. */
size_t Model::solution_levels() const
{
  if (modelRep)
    return modelRep->solution_levels(); // envelope fwd to letter
  else // letter lacking redefinition of virtual fn.
    return 1;
}


/** activate a particular level within a solution / discretization
    hierarchy and return the cost estimate. */
void Model::solution_level_index(size_t index)
{
  if (modelRep)
    modelRep->solution_level_index(index); // envelope fwd to letter
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual solution_level_index"
         << "() function.\n       solution_level_index is not supported by this"
	 << " Model class." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


RealVector Model::solution_level_cost() const
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual solution_level_cost"
         << "() function.\n       solution_level_cost is not supported by this "
	 << "Model class." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  return modelRep->solution_level_cost(); // envelope fwd to letter
}


/** return by reference requires use of dummy objects, but is
    important to allow use of assign_rep() since this operation must
    be performed on the original envelope object. */
const String& Model::interface_id() const
{
  if (modelRep)
    return modelRep->interface_id(); // envelope fwd to letter
  else // letter lacking redefinition of virtual fn.
    return dummy_interface.interface_id(); // return empty string
}


void Model::
primary_response_fn_weights(const RealVector& wts, bool recurse_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->primary_response_fn_weights(wts, recurse_flag);
  else // default does not support recursion (SimulationModel, NestedModel)
    primaryRespFnWts = wts;
}


void Model::surrogate_function_indices(const IntSet& surr_fn_indices)
{
  if (modelRep)
    modelRep->surrogate_function_indices(surr_fn_indices); // fwd to letter
  // else: default implementation is no-op
}


/** This function assumes derivatives with respect to the active
    continuous variables.  Therefore, concurrency with respect to the
    inactive continuous variables is not captured. */
int Model::derivative_concurrency() const
{
  if (modelRep)
    return modelRep->derivative_concurrency(); // envelope fwd to letter
  else { // not a virtual function: base class definition for all letters
    int deriv_conc = 1;
    if ( (gradientType=="numerical" || gradientType=="mixed") &&
	 methodSource == "dakota" )
      deriv_conc += (intervalType == "central") ? 2*numDerivVars : numDerivVars;
    if ( hessianType == "numerical" ||
	 ( hessianType == "mixed" && !hessIdNumerical.empty())) {
      if (gradientType == "analytic")
	deriv_conc += numDerivVars;
      else if (gradientType == "numerical")
	deriv_conc += 2*numDerivVars*numDerivVars;
      else if (gradientType == "mixed") {
	bool first_order = false, second_order = false;
	if (hessianType == "mixed") { // mixed Hessians with mixed gradients
	  for (ISCIter cit=hessIdNumerical.begin();
	       cit!=hessIdNumerical.end(); ++cit) {
	    if (contains(gradIdAnalytic, *cit))
	      first_order  = true;
	    else
	      second_order = true;
	  }
	}
	else // numerical Hessians with mixed gradients
	  first_order = second_order = true;
	// First and second order differences are not mutually exclusive
	if (first_order)  // 1st-order forward gradient differences
	  deriv_conc += numDerivVars;
	if (second_order) // 2nd-order central function differences
	  deriv_conc += 2*numDerivVars*numDerivVars;
      }
    }
    return deriv_conc;
  }
}


bool Model::initialize_mapping(ParLevLIter pl_iter)
{
  if (modelRep)
    return modelRep->initialize_mapping(pl_iter);
  else {
    // restore initial states for re-entrancy
    currentResponse.reset(); // for completeness
    if (!warmStartFlag) {
      // Dakota::Variables does not support reset() since initial points are not
      // cached in Model/Variables -- they are generally (re)set from Iterator.
      //currentVariables.reset(); // not supported

      if (!quasiHessians.empty()) {
	for (size_t i=0; i<numFns; ++i)
	  quasiHessians[i] = 0.;
	numQuasiUpdates.assign(numFns, 0);// {x,fnGrads}Prev will be overwritten
      }
    }

    return false; // size did not change
  }
}


bool Model::finalize_mapping()
{
  if (modelRep)
    return modelRep->finalize_mapping();
  else // Base class default behavior is no-op
    return false; // size did not change
}


bool Model::mapping_initialized() const
{
  if (modelRep)
    return modelRep->mapping_initialized();
  else // Base class default is true
    return true;
}


bool Model::resize_pending() const
{
  if (modelRep)
    return modelRep->resize_pending();
  else // Base class default is false
    return false;
}


void Model::build_approximation()
{
  if (modelRep) // envelope fwd to letter
    modelRep->build_approximation();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual build_approximation"
         << "() function.\nThis model does not support approximation "
	 << "construction." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


bool Model::
build_approximation(const Variables& vars, const IntResponsePair& response_pr)
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual build_approximation"
         << "(Variables, IntResponsePair) function.\nThis model does not "
	 << "support constrained approximation construction." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  // envelope fwd to letter
  return modelRep->build_approximation(vars, response_pr);
}


void Model::update_approximation(bool rebuild_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->update_approximation(rebuild_flag);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual update_"
	 << "approximation(bool) function.\nThis model does not support "
	 << "approximation updating." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::
update_approximation(const Variables& vars, const IntResponsePair& response_pr,
		     bool rebuild_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->update_approximation(vars, response_pr, rebuild_flag);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual update_approximation"
         << "(Variables, IntResponsePair) function.\nThis model does not "
	 << "support approximation updating." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::
update_approximation(const VariablesArray& vars_array,
		     const IntResponseMap& resp_map, bool rebuild_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->update_approximation(vars_array, resp_map, rebuild_flag);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual update_approximation"
         << "(VariablesArray, IntResponseMap) function.\nThis model does not "
         << "support approximation updating." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::
update_approximation(const RealMatrix& samples, const IntResponseMap& resp_map,
		     bool rebuild_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->update_approximation(samples, resp_map, rebuild_flag);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual update_approximation"
         << "(RealMatrix, IntResponseMap) function.\nThis model does not "
         << "support approximation updating." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::append_approximation(bool rebuild_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->append_approximation(rebuild_flag);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual append_"
	 << "approximation(bool) function.\nThis model does not support "
	 << "approximation appending." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::
append_approximation(const Variables& vars, const IntResponsePair& response_pr,
		     bool rebuild_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->append_approximation(vars, response_pr, rebuild_flag);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual append_approximation"
         << "(Variables, IntResponsePair) function.\nThis model does not "
	 << "support approximation appending." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::
append_approximation(const VariablesArray& vars_array,
		     const IntResponseMap& resp_map, bool rebuild_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->append_approximation(vars_array, resp_map, rebuild_flag);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual append_approximation"
         << "(VariablesArray, IntResponseMap) function.\nThis model does not "
         << "support approximation appending." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::
append_approximation(const RealMatrix& samples, const IntResponseMap& resp_map,
		     bool rebuild_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->append_approximation(samples, resp_map, rebuild_flag);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual append_approximation"
         << "(RealMatrix, IntResponseMap) function.\nThis model does not "
         << "support approximation appending." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::pop_approximation(bool save_surr_data, bool rebuild_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->pop_approximation(save_surr_data, rebuild_flag);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual\n       "
	 << "pop_approximation(bool, bool) function.  This model does not\n"
	 << "       support approximation data removal." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::push_approximation()
{
  if (modelRep) // envelope fwd to letter
    modelRep->push_approximation();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual push_approximation()"
	 << " function.\n       This model does not support approximation"
	 << " augmentation." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


bool Model::push_available()
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual push_available()"
	 << "function.\n       This model does not support approximation "
	 << "augmentation." << std::endl;
    abort_handler(MODEL_ERROR);
  }
  return modelRep->push_available();
}


void Model::finalize_approximation()
{
  if (modelRep) // envelope fwd to letter
    modelRep->finalize_approximation();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual finalize_"
	 << "approximation() function.\n       This model does not support "
	 << "approximation finalization." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::store_approximation(size_t index)
{
  if (modelRep) // envelope fwd to letter
    modelRep->store_approximation(index);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual store_approximation"
	 << "() function.\n       This model does not support approximation "
	 << "storage." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::restore_approximation(size_t index)
{
  if (modelRep) // envelope fwd to letter
    modelRep->restore_approximation(index);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual restore_"
	 << "approximation() function.\n       This model does not support "
	 << "approximation restoration." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::remove_stored_approximation(size_t index)
{
  if (modelRep) // envelope fwd to letter
    modelRep->remove_stored_approximation(index);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual remove_stored_"
	 << "approximation() function.\n       This model does not support "
	 << "approximation storage." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::combine_approximation(short corr_type)
{
  if (modelRep) // envelope fwd to letter
    modelRep->combine_approximation(corr_type);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual combine_"
	 << "approximation() function.\n       This model does not support "
	 << "approximation combination." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::run_dace_iterator(bool rebuild_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->run_dace_iterator(rebuild_flag);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual run_dace_iterator()"
	 << "function.\n       This model does not support DACE executions."
	 << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


/*
const VariablesArray Model::build_variables() const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->build_variables();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual build_variables()"
         << "\n       This model does not support build variables retrieval."
	 << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


const ResponseArray Model::build_responses() const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->build_responses();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual build_responses()"
         << "\n       This model does not support build responses retrieval."
	 << std::endl;
    abort_handler(MODEL_ERROR);
  }
}
*/


bool Model::force_rebuild()
{
  if (modelRep) // envelope fwd to letter
    return modelRep->force_rebuild();
  else // default if no letter redefinition of virtual fn.
    return false;
}


SharedApproxData& Model::shared_approximation()
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual shared_approximation"
         << "() function.\nThis model does not support approximations."
	 << std::endl;
    abort_handler(MODEL_ERROR);
  }

  // envelope fwd to letter
  return modelRep->shared_approximation();
}


std::vector<Approximation>& Model::approximations()
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual approximations() "
         << "function.\nThis model does not support approximations."
	 << std::endl;
    abort_handler(MODEL_ERROR);
  }

  // envelope fwd to letter
  return modelRep->approximations();
}


const RealVectorArray& Model::approximation_coefficients(bool normalized)
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual approximation_"
         << "coefficients() function.\nThis model does not support "
         << "approximations." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  // envelope fwd to letter
  return modelRep->approximation_coefficients(normalized);
}


void Model::
approximation_coefficients(const RealVectorArray& approx_coeffs,
			   bool normalized)
{
  if (modelRep) // envelope fwd to letter
    modelRep->approximation_coefficients(approx_coeffs, normalized);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual approximation_"
         << "coefficients() function.\n       This model does not support "
         << "approximations." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


const RealVector& Model::approximation_variances(const Variables& vars)
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual approximation_"
         << "variances() function.\nThis model does not support "
         << "approximations." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  // envelope fwd to letter
  return modelRep->approximation_variances(vars);
}


const RealVector& Model::error_estimates()
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual error_estimates() "
	 << "function.\n       This model does not support error estimation."
	 << std::endl;
    abort_handler(MODEL_ERROR);
  }

  // envelope fwd to letter
  return modelRep->error_estimates();
}


const Pecos::SurrogateData& Model::approximation_data(size_t index)
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual approximation_data()"
         << " function.\nThis model does not support approximations."
	 << std::endl;
    abort_handler(MODEL_ERROR);
  }

  // envelope fwd to letter
  return modelRep->approximation_data(index);
}


void Model::surrogate_response_mode(short mode)
{
  if (modelRep) // envelope fwd to letter
    modelRep->surrogate_response_mode(mode);
  // else: default implementation is no-op
}


short Model::surrogate_response_mode() const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->surrogate_response_mode();
  else // letter lacking redefinition of virtual fn.
    return 0; // default for non-surrogate models
}


DiscrepancyCorrection& Model::discrepancy_correction()
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual discrepancy_"
	 << "correction() function.\nThis model does not support corrections."
	 << std::endl;
    abort_handler(MODEL_ERROR);
  }

  // envelope fwd to letter
  return modelRep->discrepancy_correction();
}


void Model::single_apply(const Variables& vars, Response& resp,
			 const SizetSizet2DPair& indices)
{
  if (modelRep) // envelope fwd to letter
    modelRep->single_apply(vars, resp, indices);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual single_apply() "
	 << "function.\n." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::recursive_apply(const Variables& vars, Response& resp)
{
  if (modelRep) // envelope fwd to letter
    modelRep->recursive_apply(vars, resp);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual recursive_apply() "
	 << "function.\n." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::component_parallel_mode(short mode)
{
  if (modelRep) // envelope fwd to letter
    modelRep->component_parallel_mode(mode);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual "
	 << "component_parallel_mode() function.\n." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


IntIntPair Model::estimate_partition_bounds(int max_eval_concurrency)
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual "
	 << "estimate_partition_bounds() function.\n." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  return modelRep->estimate_partition_bounds(max_eval_concurrency);
}


size_t Model::mi_parallel_level_index() const
{
  return (modelRep) ?
    modelRep->mi_parallel_level_index() : // envelope fwd to letter
    modelPCIter->mi_parallel_level_last_index(); // default definition
                   // (for Models without additional mi_pl recursions)
}


void Model::cache_unmatched_response(int raw_id)
{
  if (modelRep)
    modelRep->cache_unmatched_response(raw_id);
  else {
    // due to deriv estimation rekeying and removal of intermediate bookkeeping
    // data in Model::synchronize{,_nowait}(), caching needs to occur at the 
    // base class level using Model::responseMap, rather than derived maps.
    IntRespMIter rr_it = responseMap.find(raw_id);
    if (rr_it != responseMap.end()) {
      // insert unmatched record into cache:
      cachedResponseMap.insert(*rr_it);
      // not essential due to subsequent clear(), but avoid any redundancy:
      responseMap.erase(rr_it);
    }
  }
}


/** SimulationModels and HierarchSurrModels redefine this virtual function.
    A default value of "synchronous" prevents asynch local operations for:
\li NestedModels: a subIterator can support message passing parallelism,
    but not asynch local.
\li DataFitSurrModels: while asynch evals on approximations will work due
    to some added bookkeeping, avoiding them is preferable. */
short Model::local_eval_synchronization()
{
  if (modelRep) // should not occur: protected fn only used by the letter
    return modelRep->local_eval_synchronization(); // envelope fwd to letter
  else // letter lacking redefinition of virtual fn.
    return SYNCHRONOUS_INTERFACE; // default value
}


/** SimulationModels and HierarchSurrModels redefine this virtual function. */
int Model::local_eval_concurrency()
{
  if (modelRep) // should not occur: protected fn only used by the letter
    return modelRep->local_eval_concurrency(); // envelope fwd to letter
  else // letter lacking redefinition of virtual fn.
    return 0; // default value
}


void Model::serve_run(ParLevLIter pl_iter, int max_eval_concurrency)
{
  if (modelRep) // envelope fwd to letter
    modelRep->serve_run(pl_iter, max_eval_concurrency);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual serve_run() function"
	 << ".\nThis model does not support server operations." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::stop_servers()
{
  if (modelRep) // envelope fwd to letter
    modelRep->stop_servers();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual stop_servers() "
         << "function.\nThis model does not support server operations."
	 << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


/** The init_communicators() and derived_init_communicators() functions are
    stuctured to avoid performing the messageLengths estimation more than once.
    init_communicators() (not virtual) performs the estimation and then
    forwards the results to derived_init_communicators (virtual) which uses
    the data in different contexts. */
void Model::
init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
		   bool recurse_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->init_communicators(pl_iter, max_eval_concurrency, recurse_flag);
  else { // not a virtual function: base class definition for all letters

    // Undefined mi_pl can happen for IteratorScheduler::configure(), as
    // estimation of concurrency involves instantiation of Iterators
    // prior to IteratorScheduler::partition(), and some Iterators invoke
    // init_communicators() for contained helper iterators.  Abandoning a
    // parallel configuration means that these iterator instances should
    // be discarded and replaced once the mi_pl context is available.
    //if (!parallelLib.mi_parallel_level_defined())
    //  return;
    // Note for updated design: could replace with check miPLIters.size() <= 1,
    // but w_pl has now been expanded to be a sufficient starting pt for ie/ea
    // (meta-iterator partitioning is no longer required) --> leave commented
    // out for now. 

    // matches bcast in Model::serve_init() called from
    // IteratorScheduler::init_iterator().  bcastFlag assures that, when Model
    // recursions are present in Iterator instantiations, only the matching
    // Model instance participates in this collective communication.
    if (initCommsBcastFlag && pl_iter->server_communicator_rank() == 0)
      parallelLib.bcast(max_eval_concurrency, *pl_iter);

    // estimate messageLengths
    if (messageLengths.empty())
      estimate_message_lengths();

    // since the same Model instance may be reused in multiple contexts
    // (involving multiple calls to init_communicators()), multiple parallel
    // configurations must be supported.  This is managed using a map<> with
    // concurrency level as the lookup key.  Creation of a new parallel
    // configuration is avoided if an equivalent one already exists.
    SizetIntPair key(parallelLib.parallel_level_index(pl_iter),
		     max_eval_concurrency);
    std::map<SizetIntPair, ParConfigLIter>::iterator map_iter
      = modelPCIterMap.find(key);

    // NOTE: modelPCIter update belongs in set_communicators().  However, also
    // updating it here allows passing of analysisComm into a parallel plugin
    // interface constructor (see main.cpp).
    if (map_iter == modelPCIterMap.end()) { // this config does not yet exist

      // increment the PC every time (the first PC instance is wasted), such
      // that each Model points to its corresponding configuration, even if
      // incomplete (for a Model only contained subModels, no increment for
      // incomplete results in a shared complete configuration between top
      // level Model and subModel, and erroneous settings in top level
      // set_communicators()).
      //if ( parallelLib.num_parallel_configurations() > 1 ||
      //     parallelLib.parallel_configuration_is_complete() )
      parallelLib.increment_parallel_configuration(pl_iter);

      // Setting modelPCIter here is insufficient; it must be set at run time
      // (within set_communicators()) according to the iterator context.
      modelPCIterMap[key] = modelPCIter
	= parallelLib.parallel_configuration_iterator();
      derived_init_communicators(pl_iter, max_eval_concurrency, recurse_flag);
    }
    else
      modelPCIter = map_iter->second;
      // Parallel configuration already exists within the Model for this
      // concurrency level.  Configurations must also exist for any sub-models
      // -> no call to derived_init_communicators() needed.
  }
}


void Model::stop_init_communicators(ParLevLIter pl_iter)
{
  if (modelRep) // envelope fwd to letter
    modelRep->stop_init_communicators(pl_iter);
  else { // not a virtual function: base class definition for all letters
    int term_code = 0;
    parallelLib.bcast(term_code, *pl_iter);
  }
}


int Model::serve_init_communicators(ParLevLIter pl_iter)
{
  if (modelRep) // envelope fwd to letter
    return modelRep->serve_init_communicators(pl_iter);
  else { // not a virtual function: base class definition for all letters
    int max_eval_concurrency = 1, last_eval_concurrency = 1;
    while (max_eval_concurrency) {
      parallelLib.bcast(max_eval_concurrency, *pl_iter);
      if (max_eval_concurrency) {
	init_communicators(pl_iter, max_eval_concurrency);
	last_eval_concurrency = max_eval_concurrency;
      }
    }
    return last_eval_concurrency;
  }
}


void Model::stop_init_mapping(ParLevLIter pl_iter)
{
  if (modelRep) // envelope fwd to letter
    modelRep->stop_init_mapping(pl_iter);
  else {
    // Base class is a no-op
  }
}


int Model::serve_init_mapping(ParLevLIter pl_iter)
{
  if (modelRep) // envelope fwd to letter
    return modelRep->serve_init_mapping(pl_iter);
  else {
    // Base class is a no-op, return 0 since init_communicators() was not called
    return 0;
  }
}


void Model::stop_finalize_mapping(ParLevLIter pl_iter)
{
  if (modelRep) // envelope fwd to letter
    modelRep->stop_finalize_mapping(pl_iter);
  else {
    // Base class is a no-op
  }
}


int Model::serve_finalize_mapping(ParLevLIter pl_iter)
{
  if (modelRep) // envelope fwd to letter
    return modelRep->serve_finalize_mapping(pl_iter);
  else {
    // Base class is a no-op, return 0 since init_communicators() was not called
    return 0;
  }
}


void Model::warm_start_flag(const bool flag)
{
  if (modelRep) modelRep->warm_start_flag(flag);
  else          warmStartFlag = flag;
}


void Model::
set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
		  bool recurse_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->set_communicators(pl_iter, max_eval_concurrency, recurse_flag);
  else { // not a virtual function: base class definition for all letters

    SizetIntPair key(parallelLib.parallel_level_index(pl_iter),
		     max_eval_concurrency);
    std::map<SizetIntPair, ParConfigLIter>::iterator map_iter
      = modelPCIterMap.find(key);
    if (map_iter == modelPCIterMap.end()) { // this config does not exist
      Cerr << "Error: failure in parallel configuration lookup in "
           << "Model::set_communicators() for key(" << key.first << ", "
           << key.second << ")." << std::endl;
      abort_handler(MODEL_ERROR);
    }
    else
      modelPCIter = map_iter->second;

    // Unlike init_comms, set_comms DOES need to be recursed each time
    // to activate the correct comms at each level of the recursion.
    derived_set_communicators(pl_iter, max_eval_concurrency, recurse_flag);
  }
}


void Model::set_ie_asynchronous_mode(int max_eval_concurrency)
{
  // no rep forward required: called from derived classes

  // Set asynchEvalFlag for either evaluation message passing or asynch local
  // evaluations (or both).  Note that asynch local analysis concurrency by
  // itself does not trigger an asynchronous model, since this concurrency
  // can be handled within a synchronous model evaluation.
  // In the case of Surrogate or Nested models, this sets the asynch flag for
  // the top level iterator & model; the asynch flag for the sub-iterator &
  // sub-model must be set by calling init_communicators on the sub-model
  // within derived_init_communicators.
  if (modelPCIter->ie_parallel_level_defined()) {
    const ParallelLevel& ie_pl = modelPCIter->ie_parallel_level();

    // Note: local_eval_synchronization() handles case of eval concurrency==1
    bool message_passing = ie_pl.message_pass(), asynch_local_eval
      = (local_eval_synchronization() == ASYNCHRONOUS_INTERFACE);
    if ( message_passing || asynch_local_eval )
      asynchEvalFlag = true;

    // Set evaluationCapacity for use by iterators (e.g., COLINY).
    int local_eval_conc = local_eval_concurrency();
    if (message_passing) { // message passing mode
      evaluationCapacity = ie_pl.num_servers();
      if (local_eval_conc) // hybrid mode: capacity augmented
	evaluationCapacity *= local_eval_conc;
    }
    else if (asynch_local_eval) // asynch local mode: capacity limited
      evaluationCapacity = (local_eval_conc)
	?  local_eval_conc : max_eval_concurrency;
  }
}


void Model::
free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
		   bool recurse_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->free_communicators(pl_iter, max_eval_concurrency, recurse_flag);
  else { // not a virtual function: base class definition for all letters

    // Note: deallocations do not utilize reference counting -> the _first_
    // call to free a particular configuration deallocates it and all
    // subsequent calls are ignored (to prevent multiple deallocations).
    SizetIntPair key(parallelLib.parallel_level_index(pl_iter),
		     max_eval_concurrency);
    std::map<SizetIntPair, ParConfigLIter>::iterator map_iter
      = modelPCIterMap.find(key);
    if (map_iter != modelPCIterMap.end()) { // this config still exists
      modelPCIter = map_iter->second;
      derived_free_communicators(pl_iter, max_eval_concurrency, recurse_flag);
      modelPCIterMap.erase(key);
    }
  }
}


MPI_Comm Model::analysis_comm() const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->analysis_comm();
  else
    return modelPCIter->ea_parallel_level().server_intra_communicator();
}


/** This functionality has been pulled out of init_communicators() and
    defined separately so that it may be used in those cases when
    messageLengths is needed but model.init_communicators() is not
    called, e.g., for the master processor in the self-scheduling of a
    concurrent iterator strategy. */
void Model::estimate_message_lengths()
{
  if (modelRep) // envelope fwd to letter
    modelRep->estimate_message_lengths();
  else { // not a virtual function: base class definition for all letters
    // currently, every processor does this estimation (no Bcast needed)
    messageLengths.assign(4, 0);

    if (parallelLib.mpirun_flag()) {
      MPIPackBuffer buff;

      // A Variables object could later be larger if it has string set
      // elements that are longer than the current value.  Create a
      // new Variables object and set the string variables to the
      // worst case before packing. Variables aren't aware of the set
      // elements, so set them here with helper functions.
      Variables new_vars(currentVariables.copy());
      size_t offset = 0;
      string_variable_max(discreteDesignSetStringValues, offset, new_vars);
      offset += discreteDesignSetStringValues.size();
      string_variable_max(aleatDistParams.histogram_point_string_pairs(), 
			  offset, new_vars);
      offset += aleatDistParams.histogram_point_string_pairs().size();
      string_variable_max(
        epistDistParams.discrete_set_string_values_probabilities(),
	offset, new_vars);
      offset +=
	epistDistParams.discrete_set_string_values_probabilities().size();
      string_variable_max(discreteStateSetStringValues, offset, new_vars);

      buff << new_vars;
      messageLengths[0] = buff.size(); // length of message containing vars

      // The grad/Hessian arrays in currentResponse get dynamically resized as
      // needed.  Thesefore, the buffer estimation must assume the worst case.
      size_t num_deriv_vars = std::max(currentVariables.cv(),
                                       currentVariables.icv());
      Response new_response;
      if (num_deriv_vars >
	  currentResponse.active_set_derivative_vector().size()) {
	new_response = currentResponse.copy(); // deep copy
	ActiveSet new_set(numFns, num_deriv_vars);
	new_response.active_set(new_set); // resizes grad/Hessian arrays
      }
      else
	new_response = currentResponse; // shallow copy (shared representation)

      buff << new_response.active_set();
      messageLengths[1] = buff.size(); // length of message containing vars/set
      buff.reset();
      buff << new_response;
      messageLengths[2] = buff.size(); // length of message containing response
      buff.reset();
      ParamResponsePair current_pair(new_vars, interface_id(),
				     new_response);
      buff << current_pair;
      messageLengths[3] = buff.size(); // length of message containing a PRPair
#ifdef MPI_DEBUG
      Cout << "Message Lengths:\n" << messageLengths << std::endl;
#endif // MPI_DEBUG
    }
  }
}


void Model::string_variable_max(const StringSetArray& ssa, size_t offset, 
				Variables& vars) {
  if (modelRep) // envelope fwd to letter
    modelRep->string_variable_max(ssa, offset, vars);
  else { // not a virtual function: base class definition for all letters
    size_t num_vars = ssa.size();
    for (size_t i=0; i<num_vars; ++i) {
      String max_string("");
      SSCIter ss_it = ssa[i].begin(), ss_end = ssa[i].end();
      for ( ; ss_it!=ss_end; ++ss_it) {
	if (ss_it->size() > max_string.size())
	  max_string = *ss_it;
      }
      if (!max_string.empty())
	vars.all_discrete_string_variable(max_string, offset+i);
    }
  }
}


void Model::string_variable_max(const StringRealMapArray& srma, size_t offset, 
				Variables& vars) {
  if (modelRep) // envelope fwd to letter
    modelRep->string_variable_max(srma, offset, vars);
  else { // not a virtual function: base class definition for all letters
    size_t num_vars = srma.size();
    for (size_t i=0; i<num_vars; ++i) {
      String max_string("");
      SRMCIter ss_it = srma[i].begin(), ss_end = srma[i].end();
      for ( ; ss_it!=ss_end; ++ss_it) {
	if (ss_it->first.size() > max_string.size())
	  max_string = ss_it->first;
      }
      if (!max_string.empty())
	vars.all_discrete_string_variable(max_string, offset+i);
    }
  }
}


/** The init_serial() and derived_init_serial() functions are
    stuctured to separate base class (common) operations from derived
    class (specialized) operations. */
void Model::init_serial()
{
  if (modelRep) // envelope fwd to letter
    modelRep->init_serial();
  else { // not a virtual function: base class definition for all letters

    derived_init_serial();

    // restricted parallelism support: allow local asynchronous
    // operations but not message passing parallelism.
    if ( local_eval_synchronization() == ASYNCHRONOUS_INTERFACE )
      asynchEvalFlag = true;
  }
}


void Model::
derived_init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			   bool recurse_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->
      derived_init_communicators(pl_iter, max_eval_concurrency, recurse_flag);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual derived_init_"
	 << "communicators() function.\n       This model does not support "
	 << "communicator operations." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::derived_init_serial()
{
  if (modelRep) // envelope fwd to letter
    modelRep->derived_init_serial();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual derived_init_serial"
         << "() function.\nNo default defined at base class." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::
derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			  bool recurse_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->
      derived_set_communicators(pl_iter, max_eval_concurrency, recurse_flag);
  // else default is nothing additional beyond set_communicators()
}


void Model::
derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			   bool recurse_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->
      derived_free_communicators(pl_iter, max_eval_concurrency, recurse_flag);
  // else default is nothing additional beyond free_communicators()
}


void Model::inactive_view(short view, bool recurse_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->inactive_view(view, recurse_flag);
  else { // default does not support recursion (SimulationModel, NestedModel)
    currentVariables.inactive_view(view);
    userDefinedConstraints.inactive_view(view);
  }
}


const BitArray& Model::discrete_int_sets(short active_view)
{
  if (modelRep)
    return modelRep->discrete_int_sets(active_view);

  // identify discrete integer sets within active discrete int variables
  // (excluding discrete integer ranges)

  bool relax = (active_view == RELAXED_ALL ||
    ( active_view >= RELAXED_DESIGN && active_view <= RELAXED_STATE ) );
  const SharedVariablesData&  svd = currentVariables.shared_data();
  const SizetArray& active_totals = svd.active_components_totals();

  discreteIntSets.resize(currentVariables.div()); discreteIntSets.reset();
  size_t i, di_cntr = 0;
  if (relax) {
    // This case is complicated by the promotion of active discrete variables
    // into active continuous variables.  all_relax_di and ardi_cntr operate
    // over all of the discrete variables from the input specification, but
    // discreteIntSets operates only over the non-relaxed/categorical active
    // discrete variables, for which it distinguishes sets from ranges.
    const BitArray& all_relax_di = svd.all_relaxed_discrete_int();
    const SizetArray& all_totals = svd.components_totals();
    size_t ardi_cntr = 0;
    // discrete design
    if (active_totals[TOTAL_DDIV]) {
      size_t num_ddsiv = discreteDesignSetIntValues.size(),
	num_ddriv = all_totals[TOTAL_DDIV] - num_ddsiv;
      for (i=0; i<num_ddriv; ++i, ++ardi_cntr)
	if (!all_relax_di[ardi_cntr]) // part of active discrete vars
	  ++di_cntr;                  // leave bit as false
      for (i=0; i<num_ddsiv; ++i, ++ardi_cntr)
	if (!all_relax_di[ardi_cntr]) // part of active discrete vars
	  { discreteIntSets.set(di_cntr); ++di_cntr; } // set bit to true
    }
    else ardi_cntr += all_totals[TOTAL_DDIV];
    // discrete aleatory uncertain
    if (active_totals[TOTAL_DAUIV]) {
      size_t num_dausiv = aleatDistParams.histogram_point_int_pairs().size(),
	num_dauriv = all_totals[TOTAL_DAUIV] - num_dausiv; 
      for (i=0; i<num_dauriv; ++i, ++ardi_cntr)
	if (!all_relax_di[ardi_cntr]) // part of active discrete vars
	  ++di_cntr;                  // leave bit as false
      for (i=0; i<num_dausiv; ++i, ++ardi_cntr)
	if (!all_relax_di[ardi_cntr]) // part of active discrete vars
	  { discreteIntSets.set(di_cntr); ++di_cntr; } // set bit to true
    }
    else ardi_cntr += all_totals[TOTAL_DAUIV];
    // discrete epistemic uncertain
    if (active_totals[TOTAL_DEUIV]) {
      size_t num_deuriv
	  = epistDistParams.discrete_interval_basic_probabilities().size(),
	num_deusiv
	  = epistDistParams.discrete_set_int_values_probabilities().size();
      for (i=0; i<num_deuriv; ++i, ++ardi_cntr)
	if (!all_relax_di[ardi_cntr]) // part of active discrete vars
	  ++di_cntr;                  // leave bit as false
      for (i=0; i<num_deusiv; ++i, ++ardi_cntr)
	if (!all_relax_di[ardi_cntr]) // part of active discrete vars
	  { discreteIntSets.set(di_cntr); ++di_cntr; } // set bit to true
    }
    else ardi_cntr += all_totals[TOTAL_DEUIV];
    // discrete state
    if (active_totals[TOTAL_DSIV]) {
      size_t num_dssiv = discreteStateSetIntValues.size(),
	num_dsriv = all_totals[TOTAL_DSIV] - num_dssiv;
      for (i=0; i<num_dsriv; ++i, ++ardi_cntr)
	if (!all_relax_di[ardi_cntr]) // part of active discrete vars
	  ++di_cntr;                  // leave bit as false
      for (i=0; i<num_dssiv; ++i, ++ardi_cntr)
	if (!all_relax_di[ardi_cntr]) // part of active discrete vars
	  { discreteIntSets.set(di_cntr); ++di_cntr; } // set bit to true
    }
  }
  else { // MIXED_*
    size_t num_ddiv, num_dauiv, num_deuiv, num_dsiv;
    if (num_ddiv = active_totals[TOTAL_DDIV]) {
      size_t set_ddiv = discreteDesignSetIntValues.size();
      di_cntr += num_ddiv - set_ddiv;//svd.vc_lookup(DISCRETE_DESIGN_RANGE)
      for (i=0; i<set_ddiv; ++i, ++di_cntr)
	discreteIntSets.set(di_cntr);
    }
    if (num_dauiv = active_totals[TOTAL_DAUIV]) {
      size_t set_dauiv = aleatDistParams.histogram_point_int_pairs().size();
      di_cntr += num_dauiv - set_dauiv; // range_dauiv
      for (i=0; i<set_dauiv; ++i, ++di_cntr)
	discreteIntSets.set(di_cntr);
    }
    if (num_deuiv = active_totals[TOTAL_DEUIV]) {
      size_t set_deuiv
	= epistDistParams.discrete_set_int_values_probabilities().size();
      di_cntr += num_deuiv - set_deuiv;//vc_lookup(DISCRETE_INTERVAL_UNCERTAIN)
      for (i=0; i<set_deuiv; ++i, ++di_cntr)
	discreteIntSets.set(di_cntr);
    }
    if (num_dsiv = active_totals[TOTAL_DSIV]) {
      size_t set_dsiv = discreteStateSetIntValues.size();
      di_cntr += num_dsiv - set_dsiv;//svd.vc_lookup(DISCRETE_STATE_RANGE)
      for (i=0; i<set_dsiv; ++i, ++di_cntr)
	discreteIntSets.set(di_cntr);
    }
  }

  return discreteIntSets;
}


/*
const BitArray& Model::discrete_string_sets()
{
  if (modelRep)
    return modelRep->discrete_string_sets();

  discreteStringSets.resize(currentVariables.dsv());
  discreteStringSets.set(); // all active discrete string vars are set types
  return discreteStringSets;
}


const BitArray& Model::discrete_real_sets()
{
  if (modelRep)
    return modelRep->discrete_real_sets();

  discreteRealSets.resize(currentVariables.drv());
  discreteRealSets.set(); // all active discrete real vars are set types
  return discreteRealSets;
}
*/


const IntSetArray& Model::discrete_set_int_values(short active_view)
{
  if (modelRep)
    return modelRep->discrete_set_int_values(active_view);

  // TO DO: return if already defined by a previous invocation

  switch (active_view) {
  case MIXED_DESIGN:
    return discreteDesignSetIntValues; break;
  case MIXED_ALEATORY_UNCERTAIN: {
    const IntRealMapArray& h_pt_prs
      = aleatDistParams.histogram_point_int_pairs();
    size_t i, num_dausiv = h_pt_prs.size();
    activeDiscSetIntValues.resize(num_dausiv);
    for (i=0; i<num_dausiv; ++i)
      map_keys_to_set(h_pt_prs[i], activeDiscSetIntValues[i]);
    break;
  }
  case MIXED_EPISTEMIC_UNCERTAIN: {
    const IntRealMapArray& deusi_vals_probs
      = epistDistParams.discrete_set_int_values_probabilities();
    size_t i, num_deusiv = deusi_vals_probs.size();
    activeDiscSetIntValues.resize(num_deusiv);
    for (i=0; i<num_deusiv; ++i)
      map_keys_to_set(deusi_vals_probs[i], activeDiscSetIntValues[i]);
    break;
  }
  case MIXED_UNCERTAIN: {
    const IntRealMapArray& h_pt_prs
      = aleatDistParams.histogram_point_int_pairs();
    const IntRealMapArray& deusi_vals_probs
      = epistDistParams.discrete_set_int_values_probabilities();
    size_t i, num_dausiv = h_pt_prs.size(),
      num_deusiv = deusi_vals_probs.size();
    activeDiscSetIntValues.resize(num_dausiv+num_deusiv);
    for (i=0; i<num_dausiv; ++i)
      map_keys_to_set(h_pt_prs[i], activeDiscSetIntValues[i]);
    for (i=0; i<num_deusiv; ++i)
      map_keys_to_set(deusi_vals_probs[i],
		      activeDiscSetIntValues[i+num_dausiv]);
    break;
  }
  case MIXED_STATE:
    return discreteStateSetIntValues; break;
  case MIXED_ALL: {
    const IntRealMapArray& h_pt_prs
      = aleatDistParams.histogram_point_int_pairs();
    const IntRealMapArray& deusi_vals_probs
      = epistDistParams.discrete_set_int_values_probabilities();
    size_t i, offset, num_ddsiv = discreteDesignSetIntValues.size(),
      num_dausiv = h_pt_prs.size(), num_deusiv = deusi_vals_probs.size(),
      num_dssiv  = discreteStateSetIntValues.size();
    activeDiscSetIntValues.resize(num_ddsiv  + num_dausiv +
				  num_deusiv + num_dssiv);
    for (i=0; i<num_ddsiv; ++i)
      activeDiscSetIntValues[i] = discreteDesignSetIntValues[i];
    offset = num_ddsiv;
    for (i=0; i<num_dausiv; ++i)
      map_keys_to_set(h_pt_prs[i], activeDiscSetIntValues[i+offset]);
    offset += num_dausiv;
    for (i=0; i<num_deusiv; ++i)
      map_keys_to_set(deusi_vals_probs[i], activeDiscSetIntValues[i+offset]);
    offset += num_deusiv;
    for (i=0; i<num_dssiv; ++i)
      activeDiscSetIntValues[i+offset] = discreteStateSetIntValues[i];
    break;
  }
  default: { // RELAXED_*
    const SharedVariablesData& svd = currentVariables.shared_data();
    const BitArray& all_relax_di = svd.all_relaxed_discrete_int();
    const SizetArray& all_totals = svd.components_totals();
    const SizetArray& active_totals = svd.active_components_totals();
    size_t i, di_cntr = 0, ardi_cntr = 0;
    // discrete design
    if (active_totals[TOTAL_DDIV]) {
      size_t num_ddsiv = discreteDesignSetIntValues.size(),
	num_ddriv = all_totals[TOTAL_DDIV] - num_ddsiv;
      for (i=0; i<num_ddriv; ++i, ++ardi_cntr)
	if (!all_relax_di[ardi_cntr]) // part of active discrete vars
	  ++di_cntr;
      for (i=0; i<num_ddsiv; ++i, ++ardi_cntr)
	if (!all_relax_di[ardi_cntr]) { // part of active discrete vars
	  activeDiscSetIntValues[di_cntr] = discreteDesignSetIntValues[i];
	  ++di_cntr;
	}
    }
    else ardi_cntr += all_totals[TOTAL_DDIV];
    // discrete aleatory uncertain
    if (active_totals[TOTAL_DAUIV]) {
      const IntRealMapArray& h_pt_prs
	= aleatDistParams.histogram_point_int_pairs();
      size_t num_dausiv = h_pt_prs.size(),
	num_dauriv = all_totals[TOTAL_DAUIV] - num_dausiv; 
      for (i=0; i<num_dauriv; ++i, ++ardi_cntr)
	if (!all_relax_di[ardi_cntr]) // part of active discrete vars
	  ++di_cntr;
      for (i=0; i<num_dausiv; ++i, ++ardi_cntr)
	if (!all_relax_di[ardi_cntr]) { // part of active discrete vars
	  map_keys_to_set(h_pt_prs[i], activeDiscSetIntValues[di_cntr]);
	  ++di_cntr;
	}
    }
    else ardi_cntr += all_totals[TOTAL_DAUIV];
    // discrete epistemic uncertain
    if (active_totals[TOTAL_DEUIV]) {
      const IntRealMapArray& deusi_vals_probs
       = epistDistParams.discrete_set_int_values_probabilities();
      size_t num_deuriv
	  = epistDistParams.discrete_interval_basic_probabilities().size(),
	num_deusiv = deusi_vals_probs.size();
      for (i=0; i<num_deuriv; ++i, ++ardi_cntr)
	if (!all_relax_di[ardi_cntr]) // part of active discrete vars
	  ++di_cntr;
      for (i=0; i<num_deusiv; ++i, ++ardi_cntr)
	if (!all_relax_di[ardi_cntr]) { // part of active discrete vars
	  map_keys_to_set(deusi_vals_probs[i], activeDiscSetIntValues[di_cntr]);
	  ++di_cntr;
	}
    }
    else ardi_cntr += all_totals[TOTAL_DEUIV];
    // discrete state
    if (active_totals[TOTAL_DSIV]) {
      size_t num_dssiv = discreteStateSetIntValues.size(),
	num_dsriv = all_totals[TOTAL_DSIV] - num_dssiv;
      for (i=0; i<num_dsriv; ++i, ++ardi_cntr)
	if (!all_relax_di[ardi_cntr]) // part of active discrete vars
	  ++di_cntr;                  // leave bit as false
      for (i=0; i<num_dssiv; ++i, ++ardi_cntr)
	if (!all_relax_di[ardi_cntr]) { // part of active discrete vars
	  activeDiscSetIntValues[di_cntr] = discreteStateSetIntValues[i];
	  ++di_cntr;
	}
    }
    break;
  }
  }

  return activeDiscSetIntValues;
}


const StringSetArray& Model::discrete_set_string_values(short active_view)
{
  if (modelRep)
    return modelRep->discrete_set_string_values(active_view);

  // TO DO: return if already defined (previous call)

  switch (active_view) {
  case MIXED_DESIGN: case RELAXED_DESIGN:
    return discreteDesignSetStringValues; break;
  case MIXED_ALEATORY_UNCERTAIN: case RELAXED_ALEATORY_UNCERTAIN: {
    const StringRealMapArray& h_pt_prs
      = aleatDistParams.histogram_point_string_pairs();
    size_t i, num_dausrv = h_pt_prs.size();
    activeDiscSetStringValues.resize(num_dausrv);
    for (i=0; i<num_dausrv; ++i)
      map_keys_to_set(h_pt_prs[i], activeDiscSetStringValues[i]);
    break;
  }
  case MIXED_EPISTEMIC_UNCERTAIN: case RELAXED_EPISTEMIC_UNCERTAIN: {
    const StringRealMapArray& deusr_vals_probs
      = epistDistParams.discrete_set_string_values_probabilities();
    size_t i, num_deusrv = deusr_vals_probs.size();
    activeDiscSetStringValues.resize(num_deusrv);
    for (i=0; i<num_deusrv; ++i)
      map_keys_to_set(deusr_vals_probs[i], activeDiscSetStringValues[i]);
    break;
  }
  case MIXED_UNCERTAIN: case RELAXED_UNCERTAIN: {
    const StringRealMapArray& h_pt_prs
      = aleatDistParams.histogram_point_string_pairs();
    const StringRealMapArray& deusr_vals_probs
      = epistDistParams.discrete_set_string_values_probabilities();
    size_t i, num_dausrv = h_pt_prs.size(),
      num_deusrv = deusr_vals_probs.size();
    activeDiscSetStringValues.resize(num_dausrv+num_deusrv);
    for (i=0; i<num_dausrv; ++i)
      map_keys_to_set(h_pt_prs[i], activeDiscSetStringValues[i]);
    for (i=0; i<num_deusrv; ++i)
      map_keys_to_set(deusr_vals_probs[i],
		      activeDiscSetStringValues[i+num_dausrv]);
    break;
  }
  case MIXED_STATE: case RELAXED_STATE:
    return discreteStateSetStringValues; break;
  case MIXED_ALL: case RELAXED_ALL: {
    const StringRealMapArray& h_pt_prs
      = aleatDistParams.histogram_point_string_pairs();
    const StringRealMapArray& deusr_vals_probs
      = epistDistParams.discrete_set_string_values_probabilities();
    size_t i, offset, num_ddsiv = discreteDesignSetStringValues.size(),
      num_dausrv = h_pt_prs.size(), num_deusrv = deusr_vals_probs.size(),
      num_dssiv = discreteStateSetStringValues.size();
    activeDiscSetStringValues.resize(num_ddsiv  + num_dausrv +
				   num_deusrv + num_dssiv);
    for (i=0; i<num_ddsiv; ++i)
      activeDiscSetStringValues[i] = discreteDesignSetStringValues[i];
    offset = num_ddsiv;
    for (i=0; i<num_dausrv; ++i)
      map_keys_to_set(h_pt_prs[i], activeDiscSetStringValues[i+offset]);
    offset += num_dausrv;
    for (i=0; i<num_deusrv; ++i)
      map_keys_to_set(deusr_vals_probs[i], activeDiscSetStringValues[i+offset]);
    offset += num_deusrv;
    for (i=0; i<num_dssiv; ++i)
      activeDiscSetStringValues[i+offset] = discreteStateSetStringValues[i];
    break;
  }
  }

  return activeDiscSetStringValues; // if not previously returned
}


const RealSetArray& Model::discrete_set_real_values(short active_view)
{
  if (modelRep)
    return modelRep->discrete_set_real_values(active_view);

  // TO DO: return if already defined (previous call)

  switch (active_view) {
  case MIXED_DESIGN:
    return discreteDesignSetRealValues; break;
  case MIXED_ALEATORY_UNCERTAIN: {
    const RealRealMapArray& h_pt_prs
      = aleatDistParams.histogram_point_real_pairs();
    size_t i, num_dausrv = h_pt_prs.size();
    activeDiscSetRealValues.resize(num_dausrv);
    for (i=0; i<num_dausrv; ++i)
      map_keys_to_set(h_pt_prs[i], activeDiscSetRealValues[i]);
    break;
  }
  case MIXED_EPISTEMIC_UNCERTAIN: {
    const RealRealMapArray& deusr_vals_probs
      = epistDistParams.discrete_set_real_values_probabilities();
    size_t i, num_deusrv = deusr_vals_probs.size();
    activeDiscSetRealValues.resize(num_deusrv);
    for (i=0; i<num_deusrv; ++i)
      map_keys_to_set(deusr_vals_probs[i], activeDiscSetRealValues[i]);
    break;
  }
  case MIXED_UNCERTAIN: {
    const RealRealMapArray& h_pt_prs
      = aleatDistParams.histogram_point_real_pairs();
    const RealRealMapArray& deusr_vals_probs
      = epistDistParams.discrete_set_real_values_probabilities();
    size_t i, num_dausrv = h_pt_prs.size(),
      num_deusrv = deusr_vals_probs.size();
    activeDiscSetRealValues.resize(num_dausrv+num_deusrv);
    for (i=0; i<num_dausrv; ++i)
      map_keys_to_set(h_pt_prs[i], activeDiscSetRealValues[i]);
    for (i=0; i<num_deusrv; ++i)
      map_keys_to_set(deusr_vals_probs[i],
		      activeDiscSetRealValues[i+num_dausrv]);
    break;
  }
  case MIXED_STATE:
    return discreteStateSetRealValues; break;
  case MIXED_ALL: {
    const RealRealMapArray& h_pt_prs
      = aleatDistParams.histogram_point_real_pairs();
    const RealRealMapArray& deusr_vals_probs
      = epistDistParams.discrete_set_real_values_probabilities();
    size_t i, offset, num_ddsiv = discreteDesignSetRealValues.size(),
      num_dausrv = h_pt_prs.size(), num_deusrv = deusr_vals_probs.size(),
      num_dssiv = discreteStateSetRealValues.size();
    activeDiscSetRealValues.resize(num_ddsiv  + num_dausrv +
				   num_deusrv + num_dssiv);
    for (i=0; i<num_ddsiv; ++i)
      activeDiscSetRealValues[i] = discreteDesignSetRealValues[i];
    offset = num_ddsiv;
    for (i=0; i<num_dausrv; ++i)
      map_keys_to_set(h_pt_prs[i], activeDiscSetRealValues[i+offset]);
    offset += num_dausrv;
    for (i=0; i<num_deusrv; ++i)
      map_keys_to_set(deusr_vals_probs[i], activeDiscSetRealValues[i+offset]);
    offset += num_deusrv;
    for (i=0; i<num_dssiv; ++i)
      activeDiscSetRealValues[i+offset] = discreteStateSetRealValues[i];
    break;
  }
  default: { // RELAXED_*
    const SharedVariablesData& svd = currentVariables.shared_data();
    const BitArray& all_relax_dr = svd.all_relaxed_discrete_real();
    const SizetArray& all_totals = svd.components_totals();
    const SizetArray& active_totals = svd.active_components_totals();
    size_t i, dr_cntr = 0, ardr_cntr = 0;
    // discrete design
    if (active_totals[TOTAL_DDRV]) {
      size_t num_ddsrv = discreteDesignSetRealValues.size();
      for (i=0; i<num_ddsrv; ++i, ++ardr_cntr)
	if (!all_relax_dr[ardr_cntr]) { // part of active discrete vars
	  activeDiscSetRealValues[dr_cntr] = discreteDesignSetRealValues[i];
	  ++dr_cntr;
	}
    }
    else ardr_cntr += all_totals[TOTAL_DDRV];
    // discrete aleatory uncertain
    if (active_totals[TOTAL_DAURV]) {
      const RealRealMapArray& h_pt_prs
	= aleatDistParams.histogram_point_real_pairs();
      size_t num_dausrv = h_pt_prs.size(); 
      for (i=0; i<num_dausrv; ++i, ++ardr_cntr)
	if (!all_relax_dr[ardr_cntr]) { // part of active discrete vars
	  map_keys_to_set(h_pt_prs[i], activeDiscSetRealValues[dr_cntr]);
	  ++dr_cntr;
	}
    }
    else ardr_cntr += all_totals[TOTAL_DAURV];
    // discrete epistemic uncertain
    if (active_totals[TOTAL_DEURV]) {
      const RealRealMapArray& deusr_vals_probs
       = epistDistParams.discrete_set_real_values_probabilities();
      size_t num_deusrv = deusr_vals_probs.size();
      for (i=0; i<num_deusrv; ++i, ++ardr_cntr)
	if (!all_relax_dr[ardr_cntr]) { // part of active discrete vars
	  map_keys_to_set(deusr_vals_probs[i],activeDiscSetRealValues[dr_cntr]);
	  ++dr_cntr;
	}
    }
    else ardr_cntr += all_totals[TOTAL_DEURV];
    // discrete state
    if (active_totals[TOTAL_DSRV]) {
      size_t num_dssrv = discreteStateSetRealValues.size();
      for (i=0; i<num_dssrv; ++i, ++ardr_cntr)
	if (!all_relax_dr[ardr_cntr]) { // part of active discrete vars
	  activeDiscSetRealValues[dr_cntr] = discreteStateSetRealValues[i];
	  ++dr_cntr;
	}
    }
    break;
  }
  }

  return activeDiscSetRealValues; // if not previously returned
}


int Model::derived_evaluation_id() const
{
  if (!modelRep) {
    Cerr << "Error: Letter lacking redefinition of virtual "
	 << "derived_evaluation_id() function.\n" << std::endl;
    abort_handler(MODEL_ERROR);
  }
  
  return modelRep->derived_evaluation_id();
}


/** Only Models including ApplicationInterfaces support an evaluation cache:
    surrogate, nested, and recast mappings are not stored in the cache. 
    Possible exceptions: HierarchSurrModel, NestedModel::optionalInterface. */
bool Model::evaluation_cache(bool recurse_flag) const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->evaluation_cache(recurse_flag);
  else // letter lacking redefinition of virtual fn.
    return false; // default
}


/** Only Models including ApplicationInterfaces interact with the restart 
    file: surrogate, nested, and recast mappings are not stored in restart. 
    Possible exceptions: DataFitSurrModel::import_points(),
    NestedModel::optionalInterface. */
bool Model::restart_file(bool recurse_flag) const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->restart_file(recurse_flag);
  else // letter lacking redefinition of virtual fn.
    return false; // default
}


void Model::set_evaluation_reference()
{
  if (modelRep) // envelope fwd to letter
    modelRep->set_evaluation_reference();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual set_evaluation_"
	 << "reference() function.\n" << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::fine_grained_evaluation_counters()
{
  if (modelRep) // envelope fwd to letter
    modelRep->fine_grained_evaluation_counters();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual fine_grained_"
	 << "evaluation_counters() function.\n" << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::
print_evaluation_summary(std::ostream& s, bool minimal_header,
			 bool relative_count) const
{
  if (modelRep) // envelope fwd to letter
    modelRep->print_evaluation_summary(s, minimal_header, relative_count);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual print_evaluation_"
	 << "summary() function.\n" << std::endl;
    abort_handler(MODEL_ERROR);
  }
}

/// Derived classes containing additional models or interfaces should
/// implement this function to pass along to their sub Models/Interfaces
void Model::eval_tag_prefix(const String& eval_id_str)
{
  if (modelRep) {
    // update the base class cached value
    modelRep->evalTagPrefix = eval_id_str;
    // then derived classes may further forward this ID
    modelRep->eval_tag_prefix(eval_id_str);
  }
  else
    evalTagPrefix = eval_id_str;  // default is to set at base only
  // Models are not required to forward this as they may not have an Interface
  // else { // letter lacking redefinition of virtual fn.
  //   Cerr << "Error: Letter lacking redefinition of virtual eval_tag_prefix()"
  // 	 << "function.\n" << std::endl;
  //   abort_handler(MODEL_ERROR);
  // }
}


bool Model::db_lookup(const Variables& search_vars, const ActiveSet& search_set,
		      Response& found_resp)
{
  if (modelRep) // envelope fwd to letter
    modelRep->db_lookup(search_vars, search_set, found_resp);
  else { // default implementation
    // dependence on interface_id() restricts successful find() operation to
    // cases where response is generated by a single non-approximate interface
    // at this level.  For Nested and Surrogate models, duplication detection
    // must occur at a lower level.
    PRPCacheHIter cache_it
      = lookup_by_val(data_pairs, interface_id(), search_vars, search_set);
    if (cache_it != data_pairs.get<hashed>().end()) {
      found_resp.update(cache_it->response());
      return true;
    }
    return false;
  }
}


/** config_vars consists of [continuous, integer, string, real]. */
void Model::active_variables(const RealVector& config_vars, Model& model)
{
  // TODO: If (as hoped) we convert the configuration reader to read
  // values instead of indices for strings (and actually read
  // integers), we can avoid the conversions below.

  size_t offset = 0;  // current index into configuration variables

  RealVector ccv(Teuchos::View, config_vars.values() + offset, model.cv());
  model.continuous_variables(ccv);
  offset += model.cv();

  RealVector dicv(Teuchos::View, config_vars.values() + offset, model.div());
  IntVector dicv_as_int(model.div());
  iround(dicv, dicv_as_int);
  model.discrete_int_variables(dicv_as_int);
  offset += model.div();

  RealVector dscv(Teuchos::View, config_vars.values() + offset, model.dsv());
  const StringSetArray& discrete_str_vals = model.discrete_set_string_values();
  for (size_t i=0; i<model.dsv(); ++i) {
    String str_value = 
      set_index_to_value(boost::math::iround(dscv[i]), discrete_str_vals[i]);
    model.current_variables().discrete_string_variable(str_value, i);
  }
  offset += model.dsv();

  RealVector drcv(Teuchos::View, config_vars.values() + offset, model.drv());
  model.discrete_real_variables(drcv);
  //offset += model.drv();
}



/** config_vars consists of [continuous, integer, string, real]. */
void Model::inactive_variables(const RealVector& config_vars, Model& model)
{
  inactive_variables(config_vars, model, model.current_variables());
}


/** config_vars consists of [continuous, integer, string, real]. */
void Model::inactive_variables(const RealVector& config_vars, Model& model,
			       Variables& vars)
{
  // TODO: If (as hoped) we convert the configuration reader to read
  // values instead of indices for strings (and actually read
  // integers), we can avoid the conversions below.

  size_t offset = 0;  // current index into configuration variables

  RealVector ccv(Teuchos::View, config_vars.values() + offset, model.icv());
  vars.inactive_continuous_variables(ccv);
  offset += model.icv();

  RealVector dicv(Teuchos::View, config_vars.values() + offset, model.idiv());
  IntVector dicv_as_int(model.idiv());
  iround(dicv, dicv_as_int);
  vars.inactive_discrete_int_variables(dicv_as_int);
  offset += model.idiv();

  RealVector dscv(Teuchos::View, config_vars.values() + offset, model.idsv());
  // the admissible _inactive_ discrete string values
  const StringSetArray& discrete_str_vals =
    model.discrete_set_string_values(model.current_variables().view().second);
  for (size_t i=0; i<model.idsv(); ++i) {
    String str_value = 
      set_index_to_value(boost::math::iround(dscv[i]), discrete_str_vals[i]);
    vars.inactive_discrete_string_variable(str_value, i);
  }
  offset += model.idsv();

  RealVector drcv(Teuchos::View, config_vars.values() + offset, model.idrv());
  vars.inactive_discrete_real_variables(drcv);
  //offset += model.idrv();
}


} // namespace Dakota
