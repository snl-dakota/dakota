/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
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
#include "EnsembleSurrModel.hpp"
#include "ActiveSubspaceModel.hpp"
#include "AdaptedBasisModel.hpp"
#include "RandomFieldModel.hpp"
#include "MarginalsCorrDistribution.hpp"
#include "DakotaGraphics.hpp"
#include "pecos_stat_util.hpp"
#include "EvaluationStore.hpp"

static const char rcsId[]="@(#) $Id: DakotaModel.cpp 7029 2010-10-22 00:17:02Z mseldre $";


namespace Dakota 
{
extern PRPCache        data_pairs;
extern EvaluationStore evaluation_store_db; // defined in dakota_global_defs.cpp

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

// Initialization of static model ID counters
size_t Model::noSpecIdNum = 0;


/** This constructor builds the base class data for all inherited
    models.  get_model() instantiates a derived class and the derived
    class selects this base class constructor in its initialization
    list (to avoid the recursion of the base class constructor calling
    get_model() again).  Since the letter IS the representation, its
    representation pointer is set to NULL. */
Model::Model(BaseConstructor, ProblemDescDB& problem_db):
  currentVariables(problem_db.get_variables()),
  numDerivVars(currentVariables.cv()),
  currentResponse(
    problem_db.get_response(SIMULATION_RESPONSE, currentVariables)),
  numFns(currentResponse.num_functions()),
  userDefinedConstraints(problem_db, currentVariables.shared_data()),
  evaluationsDB(evaluation_store_db),
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
  warmStartFlag(false), supportsEstimDerivs(true), mappingInitialized(false),
  probDescDB(problem_db), parallelLib(problem_db.parallel_library()),
  modelPCIter(parallelLib.parallel_configuration_iterator()),
  componentParallelMode(NO_PARALLEL_MODE), asynchEvalFlag(false),
  evaluationCapacity(1), 
  // See base constructor in DakotaIterator.cpp for full discussion of output
  // verbosity.  For models, QUIET_OUTPUT turns off response reporting and
  // SILENT_OUTPUT additionally turns off fd_gradient parameter set reporting.
  outputLevel(problem_db.get_short("method.output")),
  primaryRespFnWts(probDescDB.get_rv("responses.primary_response_fn_weights")),
  hierarchicalTagging(probDescDB.get_bool("model.hierarchical_tags")),
  scalingOpts(problem_db, currentResponse.shared_data()),
  modelEvaluationsDBState(EvaluationsDBState::UNINITIALIZED),
  interfEvaluationsDBState(EvaluationsDBState::UNINITIALIZED),
  modelId(problem_db.get_string("model.id")), modelEvalCntr(0),
  estDerivsFlag(false), initCommsBcastFlag(false), modelAutoGraphicsFlag(false),
  prevDSIView(EMPTY_VIEW), prevDSSView(EMPTY_VIEW), prevDSRView(EMPTY_VIEW)
{
  // weights have length group if given; expand if fields present
  expand_for_fields_sdv(currentResponse.shared_data(),
			probDescDB.get_rv("responses.primary_response_fn_weights"),
			"primary response weights", false, primaryRespFnWts);

  initialize_distribution(mvDist);
  initialize_distribution_parameters(mvDist);

  if (modelId.empty())
    modelId = user_auto_id();

  // TODO: Latent bug here as sense will be size 1 or group (must acct for fields)
  // Define primaryRespFnSense BoolDeque from DB StringArray
  StringArray db_sense
    = problem_db.get_sa("responses.primary_response_fn_sense");
  if (!db_sense.empty()) {
    size_t i, num_sense = db_sense.size(), num_primary = num_primary_fns();
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

  bool estimating_derivs = false;
  // Promote fdGradStepSize/fdHessByFnStepSize/fdHessByGradStepSize to defaults
  // if needed.  Note: the fdStepSize arrays specialize by variable, whereas
  // mixed grads/Hessians specialize by function.
  if ( gradientType == "numerical" ||
       ( gradientType == "mixed" && !gradIdNumerical.empty() ) ) {
    estimating_derivs = true;
    if (fdGradStepSize.empty()) {
      fdGradStepSize.resize(1);
      fdGradStepSize[0] = 0.001;
    }
  }
  if ( hessianType == "numerical" ||
       ( hessianType == "mixed" && !hessIdNumerical.empty() ) ) {
    estimating_derivs = true;
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

  // TODO: Tried to be aggressive and swallow metadata when numerical
  // derivatives are active, even though they may be active for some
  // evals and inactive for others. Causes problems with reading the
  // results files for the underlying finite difference evals if they
  // contain metadata...
  //  if (estimating_derivs)
  //  currentResponse.reshape_metadata(0);

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
}


Model::
Model(LightWtBaseConstructor, const SharedVariablesData& svd, bool share_svd,
      const SharedResponseData& srd, bool share_srd, const ActiveSet& set,
      short output_level, ProblemDescDB& problem_db,
      ParallelLibrary& parallel_lib):
  numDerivVars(set.derivative_vector().size()),
  numFns(set.request_vector().size()), evaluationsDB(evaluation_store_db),
  fdGradStepType("relative"), fdHessStepType("relative"), warmStartFlag(false), 
  supportsEstimDerivs(true), mappingInitialized(false), probDescDB(problem_db),
  parallelLib(parallel_lib),
  modelPCIter(parallel_lib.parallel_configuration_iterator()),
  componentParallelMode(NO_PARALLEL_MODE), asynchEvalFlag(false),
  evaluationCapacity(1), outputLevel(output_level),
  mvDist(Pecos::MARGINALS_CORRELATIONS), hierarchicalTagging(false),
  modelEvaluationsDBState(EvaluationsDBState::UNINITIALIZED),
  interfEvaluationsDBState(EvaluationsDBState::UNINITIALIZED),
  modelId(no_spec_id()), // to be replaced by derived ctors
  modelEvalCntr(0), estDerivsFlag(false), initCommsBcastFlag(false),
  modelAutoGraphicsFlag(false), prevDSIView(EMPTY_VIEW),
  prevDSSView(EMPTY_VIEW), prevDSRView(EMPTY_VIEW)
{
  if (share_svd) {
    currentVariables       =   Variables(svd);
    userDefinedConstraints = Constraints(svd);
  }
  else {
    SharedVariablesData new_svd(svd.copy());
    //SharedVariablesData new_svd(svd.view(), svd.components_totals()); // alt
    currentVariables       =   Variables(new_svd);
    userDefinedConstraints = Constraints(new_svd);
  }

  currentResponse = (share_srd) ?
    Response(srd, set) : Response(srd.response_type(), set);

  // TODO: unsure if this is too aggressive due to supportsEstimDerivs = true:
  //  currentResponse.reshape_metadata(0)
}


/** This constructor also builds the base class data for inherited models.
    However, it is used for derived models which are instantiated on the fly.
    Therefore it only initializes a small subset of attributes. */
Model::
Model(LightWtBaseConstructor, ProblemDescDB& problem_db,
      ParallelLibrary& parallel_lib):
  warmStartFlag(false), supportsEstimDerivs(true), mappingInitialized(false),
  probDescDB(problem_db), parallelLib(parallel_lib),
  evaluationsDB(evaluation_store_db),
  modelPCIter(parallel_lib.parallel_configuration_iterator()),
  componentParallelMode(NO_PARALLEL_MODE), asynchEvalFlag(false),
  evaluationCapacity(1), outputLevel(NORMAL_OUTPUT),
  mvDist(Pecos::MARGINALS_CORRELATIONS), hierarchicalTagging(false),
  modelEvaluationsDBState(EvaluationsDBState::UNINITIALIZED),
  interfEvaluationsDBState(EvaluationsDBState::UNINITIALIZED),
  modelId(no_spec_id()), // to be replaced by derived ctors
  modelEvalCntr(0), estDerivsFlag(false),
  initCommsBcastFlag(false), modelAutoGraphicsFlag(false),
  prevDSIView(EMPTY_VIEW), prevDSSView(EMPTY_VIEW), prevDSRView(EMPTY_VIEW)
{ /* empty ctor */ }


/** The default constructor is used in vector<Model> instantiations and for
    default initialization of Model objects.  modelRep is NULL in this case
    (a populated problem_db is needed to build a meaningful Model object). */
Model::Model():
  probDescDB(dummy_db), parallelLib(dummy_lib),
  evaluationsDB(evaluation_store_db)
{ /* empty ctor */ }


/** Used for envelope instantiations within strategy constructors.
    Envelope constructor only needs to extract enough data to properly
    execute get_model, since Model(BaseConstructor, problem_db)
    builds the actual base class data for the derived models. */
Model::Model(ProblemDescDB& problem_db):
  probDescDB(problem_db), parallelLib(problem_db.parallel_library()),
  evaluationsDB(evaluation_store_db), modelRep(get_model(problem_db))
{
  if ( !modelRep ) // bad type or insufficient memory
    abort_handler(MODEL_ERROR);
}


/** Used only by the envelope constructor to initialize modelRep to the
    appropriate derived type, as given by the modelType attribute. */
std::shared_ptr<Model> Model::get_model(ProblemDescDB& problem_db)
{
  // These instantiations will NOT recurse on the Model(problem_db)
  // constructor due to the use of BaseConstructor.

  const String& model_type = problem_db.get_string("model.type");
  if ( model_type == "simulation" )
    return std::make_shared<SimulationModel>(problem_db);
  else if ( model_type == "nested")
    return std::make_shared<NestedModel>(problem_db);
  else if ( model_type == "surrogate") {
    const String& surr_type = problem_db.get_string("model.surrogate.type");
    if (surr_type == "ensemble")
      return std::make_shared<EnsembleSurrModel>(problem_db);
    else // all other surrogates (local/multipt/global) managed by DataFitSurr
      return std::make_shared<DataFitSurrModel>(problem_db);
  }
  else if ( model_type == "active_subspace" )
    return std::make_shared<ActiveSubspaceModel>(problem_db);
  else if ( model_type == "adapted_basis" )
    return std::make_shared<AdaptedBasisModel>(problem_db);
  else if ( model_type == "random_field" )
    return std::make_shared<RandomFieldModel>(problem_db);
  else
    Cerr << "Invalid model type: " << model_type << std::endl;

  return std::shared_ptr<Model>();
}


/** Copy constructor manages sharing of modelRep. */
Model::Model(const Model& model): probDescDB(model.problem_description_db()),
  parallelLib(probDescDB.parallel_library()),
  evaluationsDB(evaluation_store_db), modelRep(model.modelRep)
{ /* empty ctor */ }


Model Model::operator=(const Model& model)
{
  modelRep = model.modelRep;
  return *this;
}


Model::~Model()
{ /* empty dtor */ }


/** The assign_rep() function is used for publishing derived class
    letters to existing envelopes, as opposed to sharing
    representations among multiple envelopes (in particular,
    assign_rep is passed a letter object and operator= is passed an
    envelope object).

    Use case assumes the incoming letter is instantiated on the fly
    and has no envelope.  This case is modeled after get_model(): a
    letter is dynamically allocated and passed into assign_rep (its
    memory management is passed over to the envelope).

    If the letter happens to be managed by another envelope, it will
    persist as long as the last envelope referencing it. */
void Model::assign_rep(std::shared_ptr<Model> model_rep)
{
  modelRep = model_rep;
}


/** Build random variable distribution types and active subset.  This
    function is used when the Model variables are in x-space. */
void Model::
initialize_distribution(Pecos::MultivariateDistribution& mv_dist,
			bool active_only)
{
  // Notes:
  // > Model base instantiates the x-space MultivariateDistribution, while
  //   derived ProbabilityTransformModel manages a ProbabilityTransform
  //   (which makes a shallow copy of x-dist and creates a u-dist).
  // > This fn houses data for discrete design/state and must now be invoked
  //   in non-UQ contexts.

  // Previous (transformation-based) logic was restricted to active continuous:
  //ShortArray x_types(currentVariables.cv()); // active cont
  //ShortArray rv_types(probDescDB.get_sizet("variables.uncertain")); c/d uv
  size_t num_rv = (active_only) ?
    currentVariables.cv()  + currentVariables.div() +
    currentVariables.dsv() + currentVariables.drv() :
    currentVariables.tv(); // all vars (active subset defined using BitArray)
  ShortArray rv_types(num_rv);  BitArray active_vars(num_rv);// init bits to 0

  bool cdv, ddv, cauv, dauv, ceuv, deuv, csv, dsv;
  currentVariables.shared_data().active_subsets(cdv,  ddv,  cauv, dauv,
						ceuv, deuv, csv,  dsv);
  size_t i, start_rv = 0;

  // Implied by call to this function ... ?
  //switch (mv_dist.type()) {
  //case Pecos::MARGINALS_CORRELATIONS: {

  // Continuous design

  if (!active_only || cdv) {
    num_rv = probDescDB.get_sizet("variables.continuous_design");
    if (num_rv) {
      assign_value(rv_types, Pecos::CONTINUOUS_RANGE, start_rv, num_rv);
      if (cdv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
  }

  // Discrete design

  if (!active_only || ddv) {
    num_rv = probDescDB.get_sizet("variables.discrete_design_range");
    if (num_rv) {
      assign_value(rv_types, Pecos::DISCRETE_RANGE, start_rv, num_rv);
      if (ddv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.discrete_design_set_int");
    if (num_rv) {
      assign_value(rv_types, Pecos::DISCRETE_SET_INT, start_rv, num_rv);
      if (ddv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.discrete_design_set_string");
    if (num_rv) {
      assign_value(rv_types, Pecos::DISCRETE_SET_STRING, start_rv, num_rv);
      if (ddv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.discrete_design_set_real");
    if (num_rv) {
      assign_value(rv_types, Pecos::DISCRETE_SET_REAL, start_rv, num_rv);
      if (ddv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
  }

  // Continuous aleatory

  if (!active_only || cauv) {
    Real dbl_inf = std::numeric_limits<Real>::infinity();
    num_rv = probDescDB.get_sizet("variables.normal_uncertain");
    if (num_rv) {
      const RealVector& n_l_bnds
	= probDescDB.get_rv("variables.normal_uncertain.lower_bounds");
      const RealVector& n_u_bnds
	= probDescDB.get_rv("variables.normal_uncertain.upper_bounds");
      bool l_bnds = !n_l_bnds.empty(), u_bnds = !n_u_bnds.empty();
      if (!l_bnds && !u_bnds) // won't happen: parser -> +/-inf
	assign_value(rv_types, Pecos::NORMAL, start_rv, num_rv);
      else
	for (i=0; i<num_rv; ++i)
	  rv_types[start_rv+i] = ( ( l_bnds && n_l_bnds[i] > -dbl_inf ) ||
				   ( u_bnds && n_u_bnds[i] <  dbl_inf ) ) ?
	    Pecos::BOUNDED_NORMAL : Pecos::NORMAL;
      if (cauv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.lognormal_uncertain");
    if (num_rv) {
      const RealVector& ln_l_bnds
	= probDescDB.get_rv("variables.lognormal_uncertain.lower_bounds");
      const RealVector& ln_u_bnds
	= probDescDB.get_rv("variables.lognormal_uncertain.upper_bounds");
      bool l_bnds = !ln_l_bnds.empty(), u_bnds = !ln_u_bnds.empty();
      if (!l_bnds && !u_bnds) // won't happen: parser -> 0/inf
	assign_value(rv_types, Pecos::LOGNORMAL, start_rv, num_rv);
      else
	for (i=0; i<num_rv; ++i)
	  rv_types[start_rv+i] = ( ( l_bnds && ln_l_bnds[i] > 0. ) ||
				   ( u_bnds && ln_u_bnds[i] < dbl_inf ) ) ?
	    Pecos::BOUNDED_LOGNORMAL : Pecos::LOGNORMAL;
      if (cauv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.uniform_uncertain");
    if (num_rv) {
      assign_value(rv_types, Pecos::UNIFORM, start_rv, num_rv);
      if (cauv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.loguniform_uncertain");
    if (num_rv) {
      assign_value(rv_types, Pecos::LOGUNIFORM, start_rv, num_rv);
      if (cauv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.triangular_uncertain");
    if (num_rv) {
      assign_value(rv_types, Pecos::TRIANGULAR, start_rv, num_rv);
      if (cauv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.exponential_uncertain");
    if (num_rv) {
      assign_value(rv_types, Pecos::EXPONENTIAL, start_rv, num_rv);
      if (cauv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.beta_uncertain");
    if (num_rv) {
      assign_value(rv_types, Pecos::BETA, start_rv, num_rv);
      if (cauv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.gamma_uncertain");
    if (num_rv) {
      assign_value(rv_types, Pecos::GAMMA, start_rv, num_rv);
      if (cauv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }

    // Note: Inv gamma is not part of variable spec (calibration hyperparameter)

    num_rv = probDescDB.get_sizet("variables.gumbel_uncertain");
    if (num_rv) {
      assign_value(rv_types, Pecos::GUMBEL, start_rv, num_rv);
      if (cauv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.frechet_uncertain");
    if (num_rv) {
      assign_value(rv_types, Pecos::FRECHET, start_rv, num_rv);
      if (cauv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.weibull_uncertain");
    if (num_rv) {
      assign_value(rv_types, Pecos::WEIBULL, start_rv, num_rv);
      if (cauv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.histogram_uncertain.bin");
    if (num_rv) {
      assign_value(rv_types, Pecos::HISTOGRAM_BIN, start_rv, num_rv);
      if (cauv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
  }

  // Discrete aleatory

  if (!active_only || dauv) {
    num_rv = probDescDB.get_sizet("variables.poisson_uncertain");
    if (num_rv) {
      assign_value(rv_types, Pecos::POISSON, start_rv, num_rv);
      if (dauv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.binomial_uncertain");
    if (num_rv) {
      assign_value(rv_types, Pecos::BINOMIAL, start_rv, num_rv);
      if (dauv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.negative_binomial_uncertain");
    if (num_rv) {
      assign_value(rv_types, Pecos::NEGATIVE_BINOMIAL, start_rv, num_rv);
      if (dauv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.geometric_uncertain");
    if (num_rv) {
      assign_value(rv_types, Pecos::GEOMETRIC, start_rv, num_rv);
      if (dauv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.hypergeometric_uncertain");
    if (num_rv) {
      assign_value(rv_types, Pecos::HYPERGEOMETRIC, start_rv, num_rv);
      if (dauv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.histogram_uncertain.point_int");
    if (num_rv) {
      assign_value(rv_types, Pecos::HISTOGRAM_PT_INT, start_rv, num_rv);
      if (dauv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.histogram_uncertain.point_string");
    if (num_rv) {
      assign_value(rv_types, Pecos::HISTOGRAM_PT_STRING, start_rv, num_rv);
      if (dauv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.histogram_uncertain.point_real");
    if (num_rv) {
      assign_value(rv_types, Pecos::HISTOGRAM_PT_REAL, start_rv, num_rv);
      if (dauv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
  }

  // Continuous epistemic

  if (!active_only || ceuv) {
    num_rv = probDescDB.get_sizet("variables.continuous_interval_uncertain");
    if (num_rv) {
      assign_value(rv_types, Pecos::CONTINUOUS_INTERVAL_UNCERTAIN,
		   start_rv, num_rv);
      if (ceuv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
  }

  // Discrete epistemic

  if (!active_only || deuv) {
    num_rv = probDescDB.get_sizet("variables.discrete_interval_uncertain");
    if (num_rv) {
      assign_value(rv_types,Pecos::DISCRETE_INTERVAL_UNCERTAIN,start_rv,num_rv);
      if (deuv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.discrete_uncertain_set_int");
    if (num_rv) {
      assign_value(rv_types, Pecos::DISCRETE_UNCERTAIN_SET_INT,start_rv,num_rv);
      if (deuv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.discrete_uncertain_set_string");
    if (num_rv) {
      assign_value(rv_types, Pecos::DISCRETE_UNCERTAIN_SET_STRING,
		   start_rv, num_rv);
      if (deuv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
    num_rv = probDescDB.get_sizet("variables.discrete_uncertain_set_real");
    if (num_rv) {
      assign_value(rv_types,Pecos::DISCRETE_UNCERTAIN_SET_REAL,start_rv,num_rv);
      if (deuv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }
  }

  // Continuous state

  if (!active_only || csv) {
    num_rv = probDescDB.get_sizet("variables.continuous_state");
    if (num_rv) {
      assign_value(rv_types, Pecos::CONTINUOUS_RANGE, start_rv, num_rv);
      if (csv) assign_value(active_vars, true, start_rv, num_rv);
      start_rv += num_rv;
    }

    // Discrete state

    if (!active_only || dsv) {
      num_rv = probDescDB.get_sizet("variables.discrete_state_range");
      if (num_rv) {
	assign_value(rv_types, Pecos::DISCRETE_RANGE, start_rv, num_rv);
	if (dsv) assign_value(active_vars, true, start_rv, num_rv);
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.discrete_state_set_int");
      if (num_rv) {
	assign_value(rv_types, Pecos::DISCRETE_SET_INT, start_rv, num_rv);
	if (dsv) assign_value(active_vars, true, start_rv, num_rv);
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.discrete_state_set_string");
      if (num_rv) {
	assign_value(rv_types, Pecos::DISCRETE_SET_STRING, start_rv, num_rv);
	if (dsv) assign_value(active_vars, true, start_rv, num_rv);
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.discrete_state_set_real");
      if (num_rv) {
	assign_value(rv_types, Pecos::DISCRETE_SET_REAL, start_rv, num_rv);
	if (dsv) assign_value(active_vars, true, start_rv, num_rv);
	//start_rv += num_rv;
      }
    }
  }

  mv_dist = Pecos::MultivariateDistribution(Pecos::MARGINALS_CORRELATIONS);
  std::shared_ptr<Pecos::MarginalsCorrDistribution> mvd_rep =
    std::static_pointer_cast<Pecos::MarginalsCorrDistribution>
    (mv_dist.multivar_dist_rep());
  mvd_rep->initialize_types(rv_types, active_vars);
}


void Model::
initialize_distribution_parameters(Pecos::MultivariateDistribution& mv_dist,
				   bool active_only)
{
  // Implied by call to this function ... ?
  //switch (mv_dist.type()) {
  //case Pecos::MARGINALS_CORRELATIONS: {

    std::shared_ptr<Pecos::MarginalsCorrDistribution> mvd_rep =
      std::static_pointer_cast<Pecos::MarginalsCorrDistribution>
      (mv_dist.multivar_dist_rep());
    size_t start_rv = 0, num_rv = (active_only) ?
      currentVariables.cv()  + currentVariables.div() +
      currentVariables.dsv() + currentVariables.drv() :
      currentVariables.tv(); // all vars (active subset defined using BitArray)
    BitArray active_corr(num_rv); // init bits to 0; activate c/d auv below

    bool cdv, ddv, cauv, dauv, ceuv, deuv, csv, dsv;
    currentVariables.shared_data().active_subsets(cdv,  ddv,  cauv, dauv,
						  ceuv, deuv, csv,  dsv);

    // Continuous design
    // RANGE type could be design or state, so use count-based API

    if (!active_only || cdv) {
      num_rv = probDescDB.get_sizet("variables.continuous_design");
      if (num_rv) {
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::CR_LWR_BND,
	  probDescDB.get_rv("variables.continuous_design.lower_bounds"));
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::CR_UPR_BND,
	  probDescDB.get_rv("variables.continuous_design.upper_bounds"));
	start_rv += num_rv;
      }
    }

    // Discrete design
    // RANGE and SET types could be design or state, so use count-based API

    if (!active_only || ddv) {
      num_rv = probDescDB.get_sizet("variables.discrete_design_range");
      if (num_rv) {
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::DR_LWR_BND,
          probDescDB.get_iv("variables.discrete_design_range.lower_bounds"));
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::DR_UPR_BND,
          probDescDB.get_iv("variables.discrete_design_range.upper_bounds"));
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.discrete_design_set_int");
      if (num_rv) {
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::DSI_VALUES,
          probDescDB.get_isa("variables.discrete_design_set_int.values"));
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.discrete_design_set_string");
      if (num_rv) {
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::DSS_VALUES,
          probDescDB.get_ssa("variables.discrete_design_set_string.values"));
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.discrete_design_set_real");
      if (num_rv) {
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::DSR_VALUES,
          probDescDB.get_rsa("variables.discrete_design_set_real.values"));
	start_rv += num_rv;
      }
    }

    // Continuous aleatory

    if (!active_only || cauv) {
      // RV type could be {,BOUNDED_}NORMAL, so use count-based API
      num_rv = probDescDB.get_sizet("variables.normal_uncertain");
      if (num_rv) {
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::N_MEAN,
          probDescDB.get_rv("variables.normal_uncertain.means"));
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::N_STD_DEV,
          probDescDB.get_rv("variables.normal_uncertain.std_deviations"));
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::N_LWR_BND,
          probDescDB.get_rv("variables.normal_uncertain.lower_bounds"));
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::N_UPR_BND,
          probDescDB.get_rv("variables.normal_uncertain.upper_bounds"));
	//N_LOCATION,N_SCALE not mapped from ProblemDescDB
	assign_value(active_corr, true, start_rv, num_rv);
	start_rv += num_rv;
      }
      // RV type could be {,BOUNDED_}LOGNORMAL, so use count-based API
      num_rv = probDescDB.get_sizet("variables.lognormal_uncertain");
      if (num_rv) {
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::LN_MEAN,
          probDescDB.get_rv("variables.lognormal_uncertain.means"));
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::LN_STD_DEV,
          probDescDB.get_rv("variables.lognormal_uncertain.std_deviations"));
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::LN_LAMBDA,
          probDescDB.get_rv("variables.lognormal_uncertain.lambdas"));
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::LN_ZETA,
          probDescDB.get_rv("variables.lognormal_uncertain.zetas"));
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::LN_ERR_FACT,
          probDescDB.get_rv("variables.lognormal_uncertain.error_factors"));
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::LN_LWR_BND,
          probDescDB.get_rv("variables.lognormal_uncertain.lower_bounds"));
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::LN_UPR_BND,
          probDescDB.get_rv("variables.lognormal_uncertain.upper_bounds"));
	assign_value(active_corr, true, start_rv, num_rv);
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.uniform_uncertain");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::UNIFORM, Pecos::U_LWR_BND,
          probDescDB.get_rv("variables.uniform_uncertain.lower_bounds"));
	mvd_rep->push_parameters(Pecos::UNIFORM, Pecos::U_UPR_BND,
          probDescDB.get_rv("variables.uniform_uncertain.upper_bounds"));
	//U_LOCATION,U_SCALE not mapped from ProblemDescDB
	assign_value(active_corr, true, start_rv, num_rv);
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.loguniform_uncertain");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::LOGUNIFORM, Pecos::LU_LWR_BND,
          probDescDB.get_rv("variables.loguniform_uncertain.lower_bounds"));
	mvd_rep->push_parameters(Pecos::LOGUNIFORM, Pecos::LU_UPR_BND,
          probDescDB.get_rv("variables.loguniform_uncertain.upper_bounds"));
	assign_value(active_corr, true, start_rv, num_rv);
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.triangular_uncertain");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::TRIANGULAR, Pecos::T_MODE,
          probDescDB.get_rv("variables.triangular_uncertain.modes"));
	mvd_rep->push_parameters(Pecos::TRIANGULAR, Pecos::T_LWR_BND,
          probDescDB.get_rv("variables.triangular_uncertain.lower_bounds"));
	mvd_rep->push_parameters(Pecos::TRIANGULAR, Pecos::T_UPR_BND,
          probDescDB.get_rv("variables.triangular_uncertain.upper_bounds"));
	//T_LOCATION,T_SCALE not mapped from ProblemDescDB
	assign_value(active_corr, true, start_rv, num_rv);
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.exponential_uncertain");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::EXPONENTIAL, Pecos::E_BETA,
          probDescDB.get_rv("variables.exponential_uncertain.betas"));
	assign_value(active_corr, true, start_rv, num_rv);
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.beta_uncertain");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::BETA, Pecos::BE_ALPHA,
          probDescDB.get_rv("variables.beta_uncertain.alphas"));
	mvd_rep->push_parameters(Pecos::BETA, Pecos::BE_BETA,
          probDescDB.get_rv("variables.beta_uncertain.betas"));
	mvd_rep->push_parameters(Pecos::BETA, Pecos::BE_LWR_BND,
          probDescDB.get_rv("variables.beta_uncertain.lower_bounds"));
	mvd_rep->push_parameters(Pecos::BETA, Pecos::BE_UPR_BND,
          probDescDB.get_rv("variables.beta_uncertain.upper_bounds"));
	assign_value(active_corr, true, start_rv, num_rv);
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.gamma_uncertain");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::GAMMA, Pecos::GA_ALPHA,
          probDescDB.get_rv("variables.gamma_uncertain.alphas"));
	mvd_rep->push_parameters(Pecos::GAMMA, Pecos::GA_BETA,
          probDescDB.get_rv("variables.gamma_uncertain.betas"));
	assign_value(active_corr, true, start_rv, num_rv);
	start_rv += num_rv;
      }

      // Inverse gamma is not part of variable spec (calibration hyperparameter)

      num_rv = probDescDB.get_sizet("variables.gumbel_uncertain");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::GUMBEL, Pecos::GU_ALPHA,
          probDescDB.get_rv("variables.gumbel_uncertain.alphas"));
	mvd_rep->push_parameters(Pecos::GUMBEL, Pecos::GU_BETA,
          probDescDB.get_rv("variables.gumbel_uncertain.betas"));
	assign_value(active_corr, true, start_rv, num_rv);
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.frechet_uncertain");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::FRECHET, Pecos::F_ALPHA,
          probDescDB.get_rv("variables.frechet_uncertain.alphas"));
	mvd_rep->push_parameters(Pecos::FRECHET, Pecos::F_BETA,
          probDescDB.get_rv("variables.frechet_uncertain.betas"));
	assign_value(active_corr, true, start_rv, num_rv);
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.weibull_uncertain");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::WEIBULL, Pecos::W_ALPHA,
          probDescDB.get_rv("variables.weibull_uncertain.alphas"));
	mvd_rep->push_parameters(Pecos::WEIBULL, Pecos::W_BETA,
          probDescDB.get_rv("variables.weibull_uncertain.betas"));
	assign_value(active_corr, true, start_rv, num_rv);
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.histogram_uncertain.bin");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::HISTOGRAM_BIN, Pecos::H_BIN_PAIRS,
          probDescDB.get_rrma("variables.histogram_uncertain.bin_pairs"));
	assign_value(active_corr, true, start_rv, num_rv);
	start_rv += num_rv;
      }
    }

    // Discrete aleatory

    if (!active_only || dauv) {
      num_rv = probDescDB.get_sizet("variables.poisson_uncertain");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::POISSON, Pecos::P_LAMBDA,
          probDescDB.get_rv("variables.poisson_uncertain.lambdas"));
	assign_value(active_corr, true, start_rv, num_rv);
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.binomial_uncertain");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::BINOMIAL, Pecos::BI_P_PER_TRIAL,
          probDescDB.get_rv("variables.binomial_uncertain.prob_per_trial"));
	UIntArray num_tr;
	copy_data(probDescDB.get_iv(
	  "variables.binomial_uncertain.num_trials"), num_tr);
	mvd_rep->push_parameters(Pecos::BINOMIAL, Pecos::BI_TRIALS, num_tr);
	assign_value(active_corr, true, start_rv, num_rv);
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.negative_binomial_uncertain");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::NEGATIVE_BINOMIAL,
	  Pecos::NBI_P_PER_TRIAL, probDescDB.get_rv(
          "variables.negative_binomial_uncertain.prob_per_trial"));
	UIntArray num_tr;
	copy_data(probDescDB.get_iv(
	  "variables.negative_binomial_uncertain.num_trials"), num_tr);
	mvd_rep->
	  push_parameters(Pecos::NEGATIVE_BINOMIAL, Pecos::NBI_TRIALS, num_tr);
	assign_value(active_corr, true, start_rv, num_rv);
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.geometric_uncertain");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::GEOMETRIC, Pecos::GE_P_PER_TRIAL,
          probDescDB.get_rv("variables.geometric_uncertain.prob_per_trial"));
	assign_value(active_corr, true, start_rv, num_rv);
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.hypergeometric_uncertain");
      if (num_rv) {
	UIntArray tot_pop, sel_pop, num_drawn;
	copy_data(probDescDB.get_iv(
	  "variables.hypergeometric_uncertain.total_population"), tot_pop);
	copy_data(probDescDB.get_iv(
          "variables.hypergeometric_uncertain.selected_population"), sel_pop);
	copy_data(probDescDB.get_iv(
	  "variables.hypergeometric_uncertain.num_drawn"), num_drawn);
	mvd_rep->
	  push_parameters(Pecos::HYPERGEOMETRIC, Pecos::HGE_TOT_POP, tot_pop);
	mvd_rep->
	  push_parameters(Pecos::HYPERGEOMETRIC, Pecos::HGE_SEL_POP, sel_pop);
	mvd_rep->
	  push_parameters(Pecos::HYPERGEOMETRIC, Pecos::HGE_DRAWN, num_drawn);
	assign_value(active_corr, true, start_rv, num_rv);
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.histogram_uncertain.point_int");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::HISTOGRAM_PT_INT, Pecos::H_PT_INT_PAIRS,
          probDescDB.get_irma("variables.histogram_uncertain.point_int_pairs"));
	assign_value(active_corr, true, start_rv, num_rv);
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet(
	"variables.histogram_uncertain.point_string");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::HISTOGRAM_PT_STRING,
	  Pecos::H_PT_STR_PAIRS, probDescDB.get_srma(
	  "variables.histogram_uncertain.point_string_pairs"));
	assign_value(active_corr, true, start_rv, num_rv);
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.histogram_uncertain.point_real");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::HISTOGRAM_PT_REAL,
	  Pecos::H_PT_REAL_PAIRS, probDescDB.get_rrma(
	  "variables.histogram_uncertain.point_real_pairs"));
	assign_value(active_corr, true, start_rv, num_rv);
	start_rv += num_rv;
      }
    }

    // Continuous epistemic

    if (!active_only || ceuv) {
      num_rv = probDescDB.get_sizet("variables.continuous_interval_uncertain");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::CONTINUOUS_INTERVAL_UNCERTAIN,
          Pecos::CIU_BPA, probDescDB.get_rrrma(
          "variables.continuous_interval_uncertain.basic_probs"));
	start_rv += num_rv;
      }
    }

    // Discrete epistemic

    if (!active_only || deuv) {
      num_rv = probDescDB.get_sizet("variables.discrete_interval_uncertain");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::DISCRETE_INTERVAL_UNCERTAIN,
          Pecos::DIU_BPA, probDescDB.get_iirma(
          "variables.discrete_interval_uncertain.basic_probs"));
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.discrete_uncertain_set_int");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::DISCRETE_UNCERTAIN_SET_INT,
          Pecos::DUSI_VALUES_PROBS, probDescDB.get_irma(
          "variables.discrete_uncertain_set_int.values_probs"));
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.discrete_uncertain_set_string");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::DISCRETE_UNCERTAIN_SET_STRING,
          Pecos::DUSS_VALUES_PROBS, probDescDB.get_srma(
          "variables.discrete_uncertain_set_string.values_probs"));
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.discrete_uncertain_set_real");
      if (num_rv) {
	mvd_rep->push_parameters(Pecos::DISCRETE_UNCERTAIN_SET_REAL,
          Pecos::DUSR_VALUES_PROBS, probDescDB.get_rrma(
          "variables.discrete_uncertain_set_real.values_probs"));
	start_rv += num_rv;
      }
    }

    // Continuous state
    // RANGE type could be design or state, so use count-based API

    if (!active_only || csv) {
      num_rv = probDescDB.get_sizet("variables.continuous_state");
      if (num_rv) {
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::CR_LWR_BND,
          probDescDB.get_rv("variables.continuous_state.lower_bounds"));
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::CR_UPR_BND,
          probDescDB.get_rv("variables.continuous_state.upper_bounds"));
	start_rv += num_rv;
      }
    }

    // Discrete state
    // RANGE and SET types could be design or state, so use count-based API

    if (!active_only || dsv) {
      num_rv = probDescDB.get_sizet("variables.discrete_state_range");
      if (num_rv) {
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::DR_LWR_BND,
          probDescDB.get_iv("variables.discrete_state_range.lower_bounds"));
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::DR_UPR_BND,
          probDescDB.get_iv("variables.discrete_state_range.upper_bounds"));
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.discrete_state_set_int");
      if (num_rv) {
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::DSI_VALUES,
          probDescDB.get_isa("variables.discrete_state_set_int.values"));
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.discrete_state_set_string");
      if (num_rv) {
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::DSS_VALUES,
          probDescDB.get_ssa("variables.discrete_state_set_string.values"));
	start_rv += num_rv;
      }
      num_rv = probDescDB.get_sizet("variables.discrete_state_set_real");
      if (num_rv) {
	mvd_rep->push_parameters(start_rv, num_rv, Pecos::DSR_VALUES,
          probDescDB.get_rsa("variables.discrete_state_set_real.values"));
	//start_rv += num_rv;
      }
    }

    mvd_rep->initialize_correlations(
      probDescDB.get_rsm("variables.uncertain.correlation_matrix"),
      active_corr);

  //  break;
  //}
}


SizetMultiArrayConstView
Model::initialize_x0_bounds(const SizetArray& original_dvv, 
			    bool& active_derivs, bool& inactive_derivs, 
			    RealVector& x0, 
			    RealVector& fd_lb, RealVector& fd_ub) const
{
  // Are derivatives w.r.t. active or inactive variables?
  active_derivs = inactive_derivs = false;
  if (original_dvv == currentVariables.continuous_variable_ids()) {
    active_derivs = true;
    copy_data(currentVariables.continuous_variables(), x0);        // view->copy
  }
  else if (original_dvv ==
	   currentVariables.inactive_continuous_variable_ids()) {
    inactive_derivs = true;
    copy_data(currentVariables.inactive_continuous_variables(), x0);//view->copy
  }
  else // general derivatives
    copy_data(currentVariables.all_continuous_variables(), x0);    // view->copy

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
  size_t num_deriv_vars = original_dvv.size();
  fd_lb.resize(num_deriv_vars);  fd_ub.resize(num_deriv_vars);
  Real dbl_inf = std::numeric_limits<Real>::infinity();
  if (ignoreBounds)
    { fd_lb = -dbl_inf;  fd_ub = dbl_inf; }
  else { // manage global/inferred vs. distribution bounds
    std::shared_ptr<Pecos::MarginalsCorrDistribution> mvd_rep =
      std::static_pointer_cast<Pecos::MarginalsCorrDistribution>
      (mvDist.multivar_dist_rep());
    for (size_t j=0; j<num_deriv_vars; j++) {
      size_t cv_index = find_index(cv_ids, original_dvv[j]);
      switch (cv_types[cv_index]) {
      case NORMAL_UNCERTAIN: {    // +/-infinity or user-specified
	size_t rv_index = original_dvv[j] - 1;// id to index (full variable set)
	fd_lb[j] = mvd_rep->pull_parameter<Real>(rv_index, Pecos::N_LWR_BND);
	fd_ub[j] = mvd_rep->pull_parameter<Real>(rv_index, Pecos::N_UPR_BND);
	break;
      }
      case LOGNORMAL_UNCERTAIN: { // 0/inf or user-specified
	size_t rv_index = original_dvv[j] - 1;// id to index (full variable set)
	fd_lb[j] = mvd_rep->pull_parameter<Real>(rv_index, Pecos::LN_LWR_BND);
	fd_ub[j] = mvd_rep->pull_parameter<Real>(rv_index, Pecos::LN_UPR_BND);
	break;
      }
      case EXPONENTIAL_UNCERTAIN: case GAMMA_UNCERTAIN:
      case FRECHET_UNCERTAIN:     case WEIBULL_UNCERTAIN:
	fd_lb[j] = c_l_bnds[cv_index]; fd_ub[j] = dbl_inf;            break;
      case GUMBEL_UNCERTAIN:
	fd_lb[j] = -dbl_inf;           fd_ub[j] = dbl_inf;            break;
      default:
	fd_lb[j] = c_l_bnds[cv_index]; fd_ub[j] = c_u_bnds[cv_index]; break;
      }
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
    if (modelEvaluationsDBState == EvaluationsDBState::UNINITIALIZED) {
      modelEvaluationsDBState = evaluationsDB.model_allocate(modelId, modelType,
        currentVariables, mvDist, currentResponse, default_active_set());
      if (modelEvaluationsDBState == EvaluationsDBState::ACTIVE)
        declare_sources();
    }
    
    // Define default ActiveSet for iterators which don't pass one
    ActiveSet temp_set = currentResponse.active_set(); // copy
    temp_set.request_values(1); // function values only

    if(modelEvaluationsDBState == EvaluationsDBState::ACTIVE)
      evaluationsDB.store_model_variables(modelId, modelType, modelEvalCntr,
					  temp_set, currentVariables);

    if (derived_master_overload()) {
      // prevents error of trying to run a multiproc. direct job on the master
      derived_evaluate_nowait(temp_set);
      currentResponse = derived_synchronize().begin()->second;
    }
    else // perform a normal synchronous map
      derived_evaluate(temp_set);

    if (modelAutoGraphicsFlag)
      derived_auto_graphics(currentVariables, currentResponse);

    if (modelEvaluationsDBState == EvaluationsDBState::ACTIVE)
      evaluationsDB.store_model_response(modelId, modelType, modelEvalCntr,
					 currentResponse);
  }
}


void Model::evaluate(const ActiveSet& set)
{
  if (modelRep) // envelope fwd to letter
    modelRep->evaluate(set);
  else { // letter
    ++modelEvalCntr;

    if (modelEvaluationsDBState == EvaluationsDBState::UNINITIALIZED) {
      modelEvaluationsDBState = evaluationsDB.model_allocate(modelId, modelType,
        currentVariables, mvDist, currentResponse, default_active_set());
      if (modelEvaluationsDBState == EvaluationsDBState::ACTIVE)
        declare_sources();
    }

    if (modelEvaluationsDBState == EvaluationsDBState::ACTIVE)
      evaluationsDB.store_model_variables(modelId, modelType, modelEvalCntr,
					  set, currentVariables);

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

    if (modelAutoGraphicsFlag)
      derived_auto_graphics(currentVariables, currentResponse);

    if (modelEvaluationsDBState == EvaluationsDBState::ACTIVE)
      evaluationsDB.store_model_response(modelId, modelType, modelEvalCntr,
					 currentResponse);

  }
}


void Model::evaluate_nowait()
{
  if (modelRep) // envelope fwd to letter
    modelRep->evaluate_nowait();
  else { // letter
    ++modelEvalCntr;
    if(modelEvaluationsDBState == EvaluationsDBState::UNINITIALIZED) {
      modelEvaluationsDBState = evaluationsDB.model_allocate(modelId, modelType,
          currentVariables, mvDist, currentResponse, default_active_set());
      if(modelEvaluationsDBState == EvaluationsDBState::ACTIVE)
        declare_sources();
    }

    // Define default ActiveSet for iterators which don't pass one
    ActiveSet temp_set = currentResponse.active_set(); // copy
    temp_set.request_values(1); // function values only

    if(modelEvaluationsDBState == EvaluationsDBState::ACTIVE)
      evaluationsDB.store_model_variables(modelId, modelType, modelEvalCntr,
          temp_set, currentVariables);
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

    if(modelEvaluationsDBState == EvaluationsDBState::UNINITIALIZED) {
      modelEvaluationsDBState = evaluationsDB.model_allocate(modelId, modelType,
          currentVariables, mvDist, currentResponse, default_active_set());
      if(modelEvaluationsDBState == EvaluationsDBState::ACTIVE)
        declare_sources();
    }

    if(modelEvaluationsDBState == EvaluationsDBState::ACTIVE)
      evaluationsDB.store_model_variables(modelId, modelType, modelEvalCntr,
          set, currentVariables);

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
      for (r_cit = responseMap.begin(); r_cit != responseMap.end(); ++r_cit) {
	v_it = varsMap.find(r_cit->first);
	derived_auto_graphics(v_it->second, r_cit->second);
	varsMap.erase(v_it);
      }
    }

    // Now augment rekeyed response map with locally cached evals.  If
    // these are not matched in a higher-level rekey process used by the
    // calling context, then they are returned to cachedResponseMap
    // using Model::cache_unmatched_response().
    responseMap.insert(cachedResponseMap.begin(), cachedResponseMap.end());
    cachedResponseMap.clear();

    if(modelEvaluationsDBState == EvaluationsDBState::ACTIVE) {
      for(const auto  &id_r : responseMap)
        evaluationsDB.store_model_response(modelId, modelType, id_r.first,
					   id_r.second); 
    }
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
	int graphics_cntr = parallelLib.output_manager().graphics_counter();
	// find() is not really necessary due to Map ordering
	//g_it = graphicsRespMap.begin();
	//if (g_it == graphicsRespMap.end() || g_it->first != graphics_cntr)
	IntRespMIter g_it = graphicsRespMap.find(graphics_cntr);
	if (g_it == graphicsRespMap.end())
	  found = false;
	else {
	  IntVarsMIter v_it = varsMap.find(graphics_cntr);
	  derived_auto_graphics(v_it->second, g_it->second);
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
    if(modelEvaluationsDBState == EvaluationsDBState::ACTIVE) {
      for(const auto  &id_r : responseMap)
        evaluationsDB.store_model_response(modelId, modelType, id_r.first,
					   id_r.second);
    }
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
  const ShortArray& orig_asv = original_set.request_vector();
  const SizetArray& orig_dvv = original_set.derivative_vector();
  size_t i, j, k, map_counter = 0, num_deriv_vars = orig_dvv.size();
  size_t ifg, nfg = 0;

  for (i=0; i<numFns; i++) {
    if (map_asv[i]) {
      initial_map = true;
      if ( ( (map_asv[i] & 1) && !(orig_asv[i] & 1) ) ||
           ( (map_asv[i] & 2) && !(orig_asv[i] & 2) ) )
        augmented_data_flag = true; // orig_asv val/grad requests augmented
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

  ActiveSet new_set(map_asv, orig_dvv);
  Response initial_map_response(currentResponse.shared_data(), new_set);

  // The logic for incurring an additional data_pairs search (beyond the
  // existing duplicate detection) is that a data request contained in
  // orig_asv is most likely not a duplicate, but there is a good chance
  // that an augmented data reqmt (appears in map_asv but not in orig_asv)
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

    // define lower/upper bounds for finite differencing and cv_ids
    RealVector x0, fd_lb, fd_ub;
    bool active_derivs, inactive_derivs; // derivs w.r.t. {active,inactive} vars
    SizetMultiArrayConstView cv_ids = 
      initialize_x0_bounds(orig_dvv, active_derivs, inactive_derivs, x0,
			   fd_lb, fd_ub);

    const RealVector& fn_vals_x0  = initial_map_response.function_values();
    const RealMatrix& fn_grads_x0 = initial_map_response.function_gradients();

    // ------------------------
    // Loop over num_deriv_vars
    // ------------------------
    RealVector x = x0; 
    for (j=0; j<num_deriv_vars; j++) { // difference the 1st num_deriv_vars vars

      size_t xj_index = find_index(cv_ids, orig_dvv[j]);
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
              size_t xk_index = find_index(cv_ids, orig_dvv[k]);
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
              size_t xk_index = find_index(cv_ids, orig_dvv[k]);

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
  const SizetArray& orig_dvv = original_set.derivative_vector();
  size_t i, j, k, num_deriv_vars = orig_dvv.size();
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
    ActiveSet initial_map_set(asv, orig_dvv);
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
    if (orig_dvv == currentVariables.continuous_variable_ids()) {
      cv_ids.resize(boost::extents[cv()]);
      cv_ids = currentVariables.continuous_variable_ids();
    }
    else if (orig_dvv == currentVariables.inactive_continuous_variable_ids()) {
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
      size_t xj_index = find_index(cv_ids, orig_dvv[j]);

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
              size_t xk_index = find_index(cv_ids, orig_dvv[k]);
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
              size_t xk_index = find_index(cv_ids, orig_dvv[k]);
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
  const RealVector& x        = vars.continuous_variables(); // view
  const ShortArray& orig_asv = original_set.request_vector();
  const RealMatrix& fn_grads = new_response.function_gradients();

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
    if ( !quasiHessians[i].empty() && (orig_asv[i] & 2) ) {

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
	if (!Pecos::is_small(norm_s)) {

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
	    if (!Pecos::is_small(norm_y))
	      scaling2 = (!Pecos::is_small(std::sqrt(std::fabs(scaling1))))
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
      if ( numQuasiUpdates[i] == 0 || !Pecos::is_small(norm_s) ) {
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
  const ShortArray& asv_in   = original_set.request_vector();
  const SizetArray& orig_dvv = original_set.derivative_vector();

  // *_asv_out[i] have all been initialized to zero

  // For EnsembleSurr and Recast models with no scaling (which contain no
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
        Cerr << "Error: Model '" << model_id()
             << "' received unsupported gradient request from ASV in "
             << "Model::manage_asv." << std::endl;
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
        Cerr << "Error: Model '" << model_id()
             << "' received unsupported Hessian request from ASV in "
             << "Model::manage_asv." << std::endl;
        abort_handler(MODEL_ERROR);
      }
    }
  }

  // Depending on bounds-respecting differencing, finite difference gradients
  // may require f(x0).  The following computes the step and updates shortStep.
  if (fd_grad_flag && !ignoreBounds) { // protect call to forward_grad_step
    size_t num_deriv_vars = orig_dvv.size();

    // define lower/upper bounds for finite differencing and cv_ids
    RealVector x0, fd_lb, fd_ub;
    bool active_derivs, inactive_derivs; // derivs w.r.t. {active,inactive} vars
    SizetMultiArrayConstView cv_ids = 
      initialize_x0_bounds(orig_dvv, active_derivs, inactive_derivs, x0,
			   fd_lb, fd_ub);

    // Accumulate short step over all derivative variables
    bool short_step = false;
    for (size_t j=0; j<num_deriv_vars; j++) {
      size_t xj_index = find_index(cv_ids, orig_dvv[j]);
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
    for (ml_it=sub_models.begin(), i=0; ml_it!=sub_models.end(); ++ml_it, ++i) {
      const String& m_type = ml_it->model_type();
      if (m_type == "recast" ||
	  m_type == "probability_transform") // + other Recast types...
	manage_recasting = recastFlags[i] = true;
      else if (m_type == "nested")
	break;
    }

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

    iter_vars = user_vars.copy(); // deep copy to preserve inactive in source
    iter_resp = user_resp;//.copy(); // shallow copy currently sufficient

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
	ActiveSet recast_set  = recast_resp.active_set();    // copy
	// to propagate vars bottom up, inverse of std transform is reqd
	RecastModel& recast_model_rep =
	  *std::static_pointer_cast<RecastModel>(ml_rit->model_rep());
	recast_model_rep.inverse_transform_variables(iter_vars, recast_vars);
	recast_model_rep.
	  inverse_transform_set(iter_vars, iter_resp.active_set(), recast_set);
	// to propagate response bottom up, std transform is used
	recast_resp.active_set(recast_set);
	recast_model_rep.
	  transform_response(recast_vars, iter_vars, iter_resp, recast_resp);
	// update active in iter_vars
	iter_vars.active_variables(recast_vars);
	// reassign resp rep pointer (no actual data copying)
	iter_resp = recast_resp; // sufficient for now...
	//iter_resp.active_set(recast_set);
	//iter_resp.update(recast_resp);
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

    user_vars = iter_vars.copy(); // deep copy to preserve inactive in source
    user_resp = iter_resp;//.copy(); // shallow copy currently sufficient

    // apply recastings top down (e.g., for data export)
    // modelList assigned in manage_data_recastings() -> subordinate_models()
    // (don't want to incur this overhead for every import/export)
    ModelLIter ml_it; size_t i;
    for (ml_it=modelList.begin(), i=0; ml_it!=modelList.end(); ++ml_it, ++i)
      if (recastFlags[i]) {
	// utilize RecastModel::current{Variables,Response} to xform data
	Variables recast_vars = ml_it->current_variables(); // shallow copy
	Response  recast_resp = ml_it->current_response();  // shallow copy
	ActiveSet recast_set  = recast_resp.active_set();   // copy
	// to propagate vars top down, forward transform is reqd
	RecastModel& recast_model_rep =
	  *std::static_pointer_cast<RecastModel>(ml_it->model_rep());
	recast_model_rep.transform_variables(user_vars, recast_vars);
	recast_model_rep.
	  transform_set(user_vars, user_resp.active_set(), recast_set);
	// to propagate response top down, inverse transform is used.  Note:
	// derivatives are not currently exported --> a no-op for Nataf.
	recast_resp.active_set(recast_set);
	recast_model_rep.inverse_transform_response(recast_vars, user_vars,
						     user_resp, recast_resp);
	// update active in iter_vars
	user_vars.active_variables(recast_vars);
	// reassign resp rep pointer (no actual data copying)
	user_resp = recast_resp; // sufficient for now...
	//user_resp.active_set(recast_set);
	//user_resp.update(recast_resp);
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


void Model::create_2d_plots()
{
  if (modelRep) // should not occur: protected fn only used by the letter
    modelRep->create_2d_plots(); // fwd to letter
  else // default implementation (overridden by hierarch/nonhierarch surr)
    parallelLib.output_manager().graphics().create_plots_2d(currentVariables,
							    currentResponse);
}


void Model::create_tabular_datastream()
{
  if (modelRep) // should not occur: protected fn only used by the letter
    modelRep->create_tabular_datastream(); // fwd to letter
  else { // default implementation (overridden by hierarch/nonhierarch surr)
    OutputManager& mgr = parallelLib.output_manager();
    mgr.open_tabular_datastream();
    mgr.create_tabular_header(currentVariables, currentResponse);
  }
}


void Model::
derived_auto_graphics(const Variables& vars, const Response& resp)
{
  if (modelRep) // should not occur: protected fn only used by the letter
    modelRep->derived_auto_graphics(vars, resp); // fwd to letter
  else // default implementation (overridden by hierarch/nonhierarch surr)
    parallelLib.output_manager().add_tabular_data(vars, interface_id(), resp);
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


void Model::active_model_key(const Pecos::ActiveKey& key)
{
  if (modelRep) // envelope fwd to letter
    modelRep->active_model_key(key);
  else {
    Cerr << "Error: Letter lacking redefinition of virtual active_model_key() "
	 << "function.\n       model key activation is not supported by this "
	 << "Model class." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


const Pecos::ActiveKey& Model::active_model_key() const
{
  if (!modelRep) {
    Cerr << "Error: Letter lacking redefinition of virtual active_model_key() "
	 << "function.\n       model keys are not available from this Model "
	 << "class." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  return modelRep->active_model_key();
}


void Model::clear_model_keys()
{
  if (modelRep) // envelope fwd to letter
    modelRep->clear_model_keys();
  //else no-op (operation not required)
}


size_t Model::qoi() const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->qoi();
  else // default for models without aggregation
    return response_size();
}


/** return by reference requires use of dummy objects, but is
    important to allow use of assign_rep() since this operation must
    be performed on the original envelope object. */
Model& Model::surrogate_model(size_t i)
{
  if (modelRep) // envelope fwd to letter
    return modelRep->surrogate_model(i);
  else // letter lacking redefinition of virtual fn.
    return dummy_model; // default is no surrogate -> return empty envelope
}


const Model& Model::surrogate_model(size_t i) const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->surrogate_model(i);
  else // letter lacking redefinition of virtual fn.
    return dummy_model; // default is no surrogate -> return empty envelope
}


/** return by reference requires use of dummy objects, but is
    important to allow use of assign_rep() since this operation must
    be performed on the original envelope object. */
Model& Model::truth_model()
{
  if (modelRep) // envelope fwd to letter
    return modelRep->truth_model();
  else // letter lacking redefinition of virtual fn.
    return *this; // default is no surrogate -> return this model instance
}


const Model& Model::truth_model() const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->truth_model();
  else // letter lacking redefinition of virtual fn.
    return *this; // default is no surrogate -> return this model instance
}


unsigned short Model::active_surrogate_model_form(size_t i) const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->active_surrogate_model_form(i);
  else // letter lacking redefinition of virtual fn.
    return 0; // default (two models)
}


unsigned short Model::active_truth_model_form() const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->active_truth_model_form();
  else // letter lacking redefinition of virtual fn.
    return 1; // default (two models)
}


/** return by reference requires use of dummy objects, but is
    important to allow use of assign_rep() since this operation must
    be performed on the original envelope object. */
Model& Model::active_surrogate_model(size_t i)
{
  if (modelRep) // envelope fwd to letter
    return modelRep->active_surrogate_model(i);
  else // letter lacking redefinition of virtual fn.
    return surrogate_model(i); // default is no active distinction
}


const Model& Model::active_surrogate_model(size_t i) const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->active_surrogate_model(i);
  else // letter lacking redefinition of virtual fn.
    return surrogate_model(i); // default is no active distinction
}


/** return by reference requires use of dummy objects, but is
    important to allow use of assign_rep() since this operation must
    be performed on the original envelope object. */
Model& Model::active_truth_model()
{
  if (modelRep) // envelope fwd to letter
    return modelRep->active_truth_model();
  else // letter lacking redefinition of virtual fn.
    return truth_model(); // default is no active distinction
}


const Model& Model::active_truth_model() const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->active_truth_model();
  else // letter lacking redefinition of virtual fn.
    return truth_model(); // default is no active distinction
}


bool Model::multifidelity() const
{
  if (modelRep) return modelRep->multifidelity();
  else          return false; // default
}


bool Model::multilevel() const
{
  if (modelRep) return modelRep->multilevel();
  else          return false; // default
}


bool Model::multilevel_multifidelity() const
{
  if (modelRep) return modelRep->multilevel_multifidelity();
  else          return false; // default
}


bool Model::multifidelity_precedence() const
{
  if (modelRep) return modelRep->multifidelity_precedence();
  else          return true; // default
}


void Model::multifidelity_precedence(bool mf_prec, bool update_default)
{
  if (modelRep)
    modelRep->multifidelity_precedence(mf_prec, update_default);
  else {
    Cerr << "Error: Letter lacking redefinition of virtual multifidelity_"
	 << "precedence() function.\n       multifidelity_precedence is not "
	 << "supported by this Model class." << std::endl;
    abort_handler(MODEL_ERROR);
  }
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


void Model::resize_from_subordinate_model(size_t depth)
{
  if (modelRep) // envelope fwd to letter
    modelRep->resize_from_subordinate_model(depth);
  // else default if no redefinition is no-op
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
  if (modelRep) return modelRep->derived_interface(); // fwd to letter
  else          return dummy_interface; // return null/empty envelope
}


/** return the number of levels within a solution / discretization hierarchy. */
size_t Model::solution_levels(bool lwr_bnd) const
{
  if (modelRep) return modelRep->solution_levels(lwr_bnd); // fwd to letter
  else          return (lwr_bnd) ? 1 : 0; // default
}


/** activate a particular level within a solution / discretization hierarchy. */
void Model::solution_level_cost_index(size_t index)
{
  if (modelRep)
    modelRep->solution_level_cost_index(index); // envelope fwd to letter
  else if (index != _NPOS) {
    // letter lacking redefinition of virtual fn (for case that requires fwd)
    Cerr << "Error: Letter lacking redefinition of virtual solution_level_"
	 << "cost_index() function.\n       solution_level_cost_index is not "
	 << "supported by this Model class." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


size_t Model::solution_level_cost_index() const
{
  if (modelRep) return modelRep->solution_level_cost_index(); // fwd to letter
  else          return _NPOS; // not defined (default)
}


RealVector Model::solution_level_costs() const
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual solution_level_costs"
         << "() function.\n       solution_level_costs is not supported by "
	 << "this Model class." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  return modelRep->solution_level_costs(); // envelope fwd to letter
}


Real Model::solution_level_cost() const
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual solution_level_cost"
         << "() function.\n       solution_level_cost is not supported by this "
	 << "Model class." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  return modelRep->solution_level_cost(); // envelope fwd to letter
}


short Model::solution_control_variable_type() const
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual solution_control_"
	 << "variable_type() function.\n       solution_control_variable_"
	 << "type() is not supported by this Model class." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  return modelRep->solution_control_variable_type(); // envelope fwd to letter
}


size_t Model::solution_control_variable_index() const
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual solution_control_"
	 << "variable_index() function.\n       solution_control_variable_"
	 << "index() is not supported by this Model class." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  return modelRep->solution_control_variable_index(); // envelope fwd to letter
}


size_t Model::solution_control_discrete_variable_index() const
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual solution_control_"
	 << "discrete_variable_index() function.\n       solution_control_"
	 << "discrete_variable_index() is not supported by this Model class."
	 << std::endl;
    abort_handler(MODEL_ERROR);
  }

  return modelRep->solution_control_discrete_variable_index(); // envelope fwd
}


int Model::solution_level_int_value() const
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual solution_level_"
	 << "int_value() function.\n       solution_level_int_value is not "
	 << "supported by this Model class." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  return modelRep->solution_level_int_value(); // envelope fwd to letter
}


String Model::solution_level_string_value() const
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual solution_level_"
	 << "string_value() function.\n       solution_level_string_value is "
	 << "not supported by this Model class." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  return modelRep->solution_level_string_value(); // envelope fwd to letter
}


Real Model::solution_level_real_value() const
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual solution_level_"
	 << "real_value() function.\n       solution_level_real_value is not "
	 << "supported by this Model class." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  return modelRep->solution_level_real_value(); // envelope fwd to letter
}


size_t Model::cost_metadata_index() const
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual cost_metadata_index"
	 << "() function.\n       cost_metadata_index() is not supported by "
	 << "this Model class." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  return modelRep->cost_metadata_index(); // envelope fwd
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


void Model::surrogate_function_indices(const SizetSet& surr_fn_indices)
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


Pecos::ProbabilityTransformation& Model::probability_transformation()
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual probability_"
	 << "transformation() function.\n       Probability transformations "
	 << "are not supported by this Model class." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  return modelRep->probability_transformation(); // envelope fwd to letter
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

    mappingInitialized = true;
    return false; // size did not change
  }
}


bool Model::finalize_mapping()
{
  if (modelRep)
    return modelRep->finalize_mapping();
  else { // base class behavior
    mappingInitialized = false;
    return false; // size did not change
  }
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


void Model::rebuild_approximation()
{
  if (modelRep) // envelope fwd to letter
    modelRep->rebuild_approximation();
  else
    build_approximation(); // default: build from scratch
}


void Model::rebuild_approximation(const IntResponsePair& response_pr)
{
  if (modelRep) // envelope fwd to letter
    modelRep->rebuild_approximation(response_pr);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual rebuild_"
	 << "approximation(IntResponsePair) function.\nThis model does not "
	 << "support approximation rebuilding." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::rebuild_approximation(const IntResponseMap& resp_map)
{
  if (modelRep) // envelope fwd to letter
    modelRep->rebuild_approximation(resp_map);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual rebuild_"
	 << "approximation(IntResponseMap) function.\nThis model does not "
	 << "support approximation rebuilding." << std::endl;
    abort_handler(MODEL_ERROR);
  }
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
append_approximation(const IntVariablesMap& vars_map,
		     const IntResponseMap&  resp_map, bool rebuild_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->append_approximation(vars_map, resp_map, rebuild_flag);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual append_approximation"
         << "(IntVariablesMap, IntResponseMap) function.\nThis model does not "
         << "support approximation appending." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::
replace_approximation(const IntResponsePair& response_pr, bool rebuild_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->replace_approximation(response_pr, rebuild_flag);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual replace_"
         << "approximation(IntResponsePair) function.\nThis model does not "
	 << "support approximation data replacement." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::
replace_approximation(const IntResponseMap& resp_map, bool rebuild_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->replace_approximation(resp_map, rebuild_flag);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual replace_"
         << "approximation(IntResponseMap) function.\nThis model does not "
         << "support approximation data replacement." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::track_evaluation_ids(bool track)
{
  if (modelRep) // envelope fwd to letter
    modelRep->track_evaluation_ids(track);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual track_evaluation_"
	 << "ids() function.\n       This model does not support evaluation "
	 << "tracking." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::pop_approximation(bool save_surr_data, bool rebuild_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->pop_approximation(save_surr_data, rebuild_flag);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual pop_approximation"
	 << "(bool, bool) function.\n       This model does not support "
	 << "approximation data removal." << std::endl;
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
{ return (modelRep) ? modelRep->push_available() : false; }


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


void Model::combine_approximation()
{
  if (modelRep) // envelope fwd to letter
    modelRep->combine_approximation();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual combine_"
	 << "approximation() function.\n       This model does not support "
	 << "approximation combination." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::combined_to_active(bool clear_combined)
{
  if (modelRep) // envelope fwd to letter
    modelRep->combined_to_active(clear_combined);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual combined_to_active()"
	 << " function.\n       This model does not support approximation"
	 << " combination." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::clear_inactive()
{
  if (modelRep) // envelope fwd to letter
    modelRep->clear_inactive();
  //else no op: no inactive data to clear
}


bool Model::advancement_available()
{
  if (modelRep) return modelRep->advancement_available();
  else          return true; // only a few cases throttle advancements
}


bool Model::formulation_updated() const
{
  if (modelRep) return modelRep->formulation_updated();
  else          return false;
}


void Model::formulation_updated(bool update)
{
  if (modelRep)
    modelRep->formulation_updated(update);
  //else no op
}


void Model::run_dace()
{
  if (modelRep) // envelope fwd to letter
    modelRep->run_dace();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual run_dace() function."
	 << "\n       This model does not support DACE executions."<< std::endl;
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


const Pecos::SurrogateData& Model::approximation_data(size_t fn_index)
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual approximation_data()"
         << " function.\nThis model does not support approximations."
	 << std::endl;
    abort_handler(MODEL_ERROR);
  }

  // envelope fwd to letter
  return modelRep->approximation_data(fn_index);
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


void Model::discrepancy_emulation_mode(short mode)
{
  if (modelRep) // envelope fwd to letter
    modelRep->discrepancy_emulation_mode(mode);
  // else: default implementation is no-op
}


/*
short Model::discrepancy_emulation_mode() const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->discrepancy_emulation_mode();
  else // letter lacking redefinition of virtual fn.
    return DEFAULT_EMULATION; // default for non-surrogate models
}


void Model::link_multilevel_approximation_data()
{
  if (modelRep) // envelope fwd to letter
    modelRep->link_multilevel_approximation_data();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual link_multilevel_"
	 << "approximation_data() function.\nThis model does not support "
	 << "multilevel data." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}
*/


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


short Model::correction_type() const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->correction_type();
  else
    return NO_CORRECTION; // default for non-surrogate models
}


void Model::correction_type(short corr_type)
{
  if (modelRep) // envelope fwd to letter
    modelRep->correction_type(corr_type);
  //else no-op
}


short Model::correction_order() const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->correction_order();
  else
    return -1; // special value for no correction (0 = value correction)
}


unsigned short Model::correction_mode() const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->correction_mode();
  else
    return DEFAULT_CORRECTION; // default for non-surrogate models
}


void Model::correction_mode(unsigned short corr_mode)
{
  if (modelRep) // envelope fwd to letter
    modelRep->correction_mode(corr_mode);
  //else no-op
}


void Model::single_apply(const Variables& vars, Response& resp,
			 const Pecos::ActiveKey& paired_key)
{
  if (modelRep) // envelope fwd to letter
    modelRep->single_apply(vars, resp, paired_key);
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
  // else: default implementation is no-op
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


void Model::cache_unmatched_responses()
{
  if (modelRep)
    modelRep->cache_unmatched_responses();
  else {
    // cache all remaining entries in responseMap, as they were not successfuly
    // migrated to another bookkeeping construct (e.g. in rekey_response_map())
    cachedResponseMap.insert(responseMap.begin(), responseMap.end());
    responseMap.clear();
  }
}


/** SimulationModels and EnsembleSurrModels redefine this virtual function.
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


/** SimulationModels and EnsembleSurrModels redefine this virtual function. */
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


void Model::declare_sources() {
  if(modelRep) modelRep->declare_sources();
  else return;
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
      assign_max_strings(mvDist, new_vars);

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
      ParamResponsePair current_pair(new_vars, interface_id(), new_response);
      buff << current_pair;
      messageLengths[3] = buff.size(); // length of message containing a PRPair
#ifdef MPI_DEBUG
      Cout << "Message Lengths:\n" << messageLengths << std::endl;
#endif // MPI_DEBUG
    }
  }
}


void Model::
assign_max_strings(const Pecos::MultivariateDistribution& mv_dist,
		   Variables& vars)
{
  std::shared_ptr<Pecos::MarginalsCorrDistribution> mvd_rep =
    std::static_pointer_cast<Pecos::MarginalsCorrDistribution>
    (mv_dist.multivar_dist_rep());
  const SharedVariablesData& svd = vars.shared_data();
  StringSet ss; StringRealMap srm;
  size_t rv, start_rv, end_rv, adsv_index = 0,
    num_cv, num_div, num_dsv, num_drv;

  // discrete design set string
  svd.design_counts(num_cv, num_div, num_dsv, num_drv);
  start_rv = num_cv + num_div;  end_rv = start_rv + num_dsv;
  for (rv=start_rv; rv<end_rv; ++rv, ++adsv_index) {
    mvd_rep->pull_parameter<StringSet>(rv, Pecos::DSS_VALUES, ss);
    SSCIter max_it = max_string(ss);
    vars.all_discrete_string_variable(*max_it, adsv_index);
  }
  start_rv = end_rv + num_drv;

  // histogram pt string
  svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
  start_rv += num_cv + num_div;  end_rv = start_rv + num_dsv;
  for (rv=start_rv; rv<end_rv; ++rv, ++adsv_index) {
    mvd_rep->pull_parameter<StringRealMap>(rv, Pecos::H_BIN_PAIRS, srm);
    SRMCIter max_it = max_string(srm);
    vars.all_discrete_string_variable(max_it->first, adsv_index);
  }
  start_rv = end_rv + num_drv;

  // discrete epistemic set string
  svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
  start_rv += num_cv + num_div;  end_rv = start_rv + num_dsv;
  for (rv=start_rv; rv<end_rv; ++rv, ++adsv_index) {
    mvd_rep->pull_parameter<StringRealMap>(rv, Pecos::DUSS_VALUES_PROBS, srm);
    SRMCIter max_it = max_string(srm);
    vars.all_discrete_string_variable(max_it->first, adsv_index);
  }
  start_rv = end_rv + num_drv;

  // discrete state set string
  svd.state_counts(num_cv, num_div, num_dsv, num_drv);
  start_rv += num_cv + num_div;  end_rv = start_rv + num_dsv;
  for (rv=start_rv; rv<end_rv; ++rv, ++adsv_index) {
    mvd_rep->pull_parameter<StringSet>(rv, Pecos::DSS_VALUES, ss);
    SSCIter max_it = max_string(ss);
    vars.all_discrete_string_variable(*max_it, adsv_index);
  }
  //start_rv = end_rv + num_drv;
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


void Model::
nested_variable_mappings(const SizetArray& c_index1,
			 const SizetArray& di_index1,
			 const SizetArray& ds_index1,
			 const SizetArray& dr_index1,
			 const ShortArray& c_target2,
			 const ShortArray& di_target2,
			 const ShortArray& ds_target2,
			 const ShortArray& dr_target2)
{
  if (modelRep)
    modelRep->nested_variable_mappings(c_index1, di_index1, ds_index1,
				       dr_index1, c_target2, di_target2,
				       ds_target2, dr_target2);
  //else no-op
}


const SizetArray& Model::nested_acv1_indices() const
{
  if (!modelRep) {
    Cerr << "Error: Letter lacking redefinition of virtual nested_acv1_indices"
         << "() function.\nNo default defined at base class." << std::endl;
    abort_handler(MODEL_ERROR);
  }
  return modelRep->nested_acv1_indices();
}


const ShortArray& Model::nested_acv2_targets() const
{
  if (!modelRep) {
    Cerr << "Error: Letter lacking redefinition of virtual nested_acv2_targets"
         << "() function.\nNo default defined at base class." << std::endl;
    abort_handler(MODEL_ERROR);
  }
  return modelRep->nested_acv2_targets();
}


short Model::query_distribution_parameter_derivatives() const
{
  if (modelRep)
    return modelRep->query_distribution_parameter_derivatives();
  else // default implementation
    return NO_DERIVS;
}


void Model::activate_distribution_parameter_derivatives()
{
  if (modelRep)
    return modelRep->activate_distribution_parameter_derivatives();
  // else no-op
}


void Model::deactivate_distribution_parameter_derivatives()
{
  if (modelRep)
    return modelRep->deactivate_distribution_parameter_derivatives();
  // else no-op
}


void Model::
trans_grad_X_to_U(const RealVector& fn_grad_x, RealVector& fn_grad_u,
		  const RealVector& x_vars)
{
  if (modelRep)
    modelRep->trans_grad_X_to_U(fn_grad_x, fn_grad_u, x_vars);
  else {
    Cerr << "Error: Letter lacking redefinition of virtual trans_grad_X_to_U"
         << "() function.\nNo default defined at base class." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::
trans_grad_U_to_X(const RealVector& fn_grad_u, RealVector& fn_grad_x,
		  const RealVector& x_vars)
{
  if (modelRep)
    modelRep->trans_grad_U_to_X(fn_grad_u, fn_grad_x, x_vars);
  else {
    Cerr << "Error: Letter lacking redefinition of virtual trans_grad_U_to_X"
         << "() function.\nNo default defined at base class." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::
trans_grad_X_to_S(const RealVector& fn_grad_x, RealVector& fn_grad_s,
		  const RealVector& x_vars)
{
  if (modelRep)
    modelRep->trans_grad_X_to_S(fn_grad_x, fn_grad_s, x_vars);
  else {
    Cerr << "Error: Letter lacking redefinition of virtual trans_grad_X_to_S"
         << "() function.\nNo default defined at base class." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void Model::
trans_hess_X_to_U(const RealSymMatrix& fn_hess_x, RealSymMatrix& fn_hess_u,
		  const RealVector& x_vars, const RealVector& fn_grad_x)
{
  if (modelRep)
    modelRep->trans_hess_X_to_U(fn_hess_x, fn_hess_u, x_vars, fn_grad_x);
  else {
    Cerr << "Error: Letter lacking redefinition of virtual trans_hess_X_to_U"
         << "() function.\nNo default defined at base class." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


ActiveSet Model::default_active_set()
{
  if (modelRep)
    return modelRep->default_active_set();
  else {
    // This member fn is called from Model::evaluate(_no_wait) and the
    // ActiveSet returned is used to allocate evaluation storage in HDF5

    ActiveSet set; 
    set.derivative_vector(currentVariables.all_continuous_variable_ids());
    ShortArray asv(numFns, 1);
    if (!set.derivative_vector().empty()) {
      if ( gradientType != "none" &&
	   ( gradientType == "analytic" || supportsEstimDerivs ) )
	for(auto &a : asv)
	  a |=  2;

      if ( hessianType != "none" &&
	   ( hessianType == "analytic" || supportsEstimDerivs ) )
	for(auto &a : asv)
	  a |=  4;
    }

    set.request_vector(asv);
    return set;
  }
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
      size_t num_ddriv = svd.vc_lookup(DISCRETE_DESIGN_RANGE),
	     num_ddsiv = svd.vc_lookup(DISCRETE_DESIGN_SET_INT);
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
      size_t num_dausiv = svd.vc_lookup(HISTOGRAM_POINT_UNCERTAIN_INT),
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
      size_t num_deuriv = svd.vc_lookup(DISCRETE_INTERVAL_UNCERTAIN),
	     num_deusiv = svd.vc_lookup(DISCRETE_UNCERTAIN_SET_INT);
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
      size_t num_dsriv = svd.vc_lookup(DISCRETE_STATE_RANGE),
	     num_dssiv = svd.vc_lookup(DISCRETE_STATE_SET_INT);
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
      size_t set_ddiv = svd.vc_lookup(DISCRETE_DESIGN_SET_INT);
      di_cntr += num_ddiv - set_ddiv;//svd.vc_lookup(DISCRETE_DESIGN_RANGE)
      for (i=0; i<set_ddiv; ++i, ++di_cntr)
	discreteIntSets.set(di_cntr);
    }
    if (num_dauiv = active_totals[TOTAL_DAUIV]) {
      size_t set_dauiv = svd.vc_lookup(HISTOGRAM_POINT_UNCERTAIN_INT);
      di_cntr += num_dauiv - set_dauiv; // range_dauiv
      for (i=0; i<set_dauiv; ++i, ++di_cntr)
	discreteIntSets.set(di_cntr);
    }
    if (num_deuiv = active_totals[TOTAL_DEUIV]) {
      size_t set_deuiv = svd.vc_lookup(DISCRETE_UNCERTAIN_SET_INT);
      di_cntr += num_deuiv - set_deuiv;//vc_lookup(DISCRETE_INTERVAL_UNCERTAIN)
      for (i=0; i<set_deuiv; ++i, ++di_cntr)
	discreteIntSets.set(di_cntr);
    }
    if (num_dsiv = active_totals[TOTAL_DSIV]) {
      size_t set_dsiv = svd.vc_lookup(DISCRETE_STATE_SET_INT);
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

  // return previous result for previous invocation with consistent view
  // Note: any external update of DSI values should reset prevDSIView to 0
  if (active_view == prevDSIView) return activeDiscSetIntValues;

  std::shared_ptr<Pecos::MarginalsCorrDistribution> mvd_rep =
    std::static_pointer_cast<Pecos::MarginalsCorrDistribution>
    (mvDist.multivar_dist_rep());
  const SharedVariablesData& svd = currentVariables.shared_data();
  switch (active_view) {
  case MIXED_DESIGN: {
    size_t num_rv = svd.vc_lookup(DISCRETE_DESIGN_SET_INT),
         start_rv = svd.vc_lookup(CONTINUOUS_DESIGN)
                  + svd.vc_lookup(DISCRETE_DESIGN_RANGE);
    mvd_rep->pull_parameters<IntSet>(start_rv, num_rv, Pecos::DSI_VALUES,
				     activeDiscSetIntValues);
    break;
  }
  case MIXED_ALEATORY_UNCERTAIN: {
    IntRealMapArray h_pt_prs;
    mvd_rep->pull_parameters<IntRealMap>(Pecos::HISTOGRAM_PT_INT,
      Pecos::H_PT_INT_PAIRS, h_pt_prs);
    size_t i, num_dausiv = h_pt_prs.size();
    activeDiscSetIntValues.resize(num_dausiv);
    for (i=0; i<num_dausiv; ++i)
      map_keys_to_set(h_pt_prs[i], activeDiscSetIntValues[i]);
    break;
  }
  case MIXED_EPISTEMIC_UNCERTAIN: {
    IntRealMapArray deusi_vals_probs;
    mvd_rep->pull_parameters<IntRealMap>(Pecos::DISCRETE_UNCERTAIN_SET_INT,
      Pecos::DUSI_VALUES_PROBS, deusi_vals_probs);
    size_t i, num_deusiv = deusi_vals_probs.size();
    activeDiscSetIntValues.resize(num_deusiv);
    for (i=0; i<num_deusiv; ++i)
      map_keys_to_set(deusi_vals_probs[i], activeDiscSetIntValues[i]);
    break;
  }
  case MIXED_UNCERTAIN: {
    IntRealMapArray h_pt_prs, deusi_vals_probs;
    mvd_rep->pull_parameters<IntRealMap>(Pecos::HISTOGRAM_PT_INT,
      Pecos::H_PT_INT_PAIRS, h_pt_prs);
    mvd_rep->pull_parameters<IntRealMap>(Pecos::DISCRETE_UNCERTAIN_SET_INT,
      Pecos::DUSI_VALUES_PROBS, deusi_vals_probs);
    size_t i, num_dausiv = h_pt_prs.size(),num_deusiv = deusi_vals_probs.size();
    activeDiscSetIntValues.resize(num_dausiv+num_deusiv);
    for (i=0; i<num_dausiv; ++i)
      map_keys_to_set(h_pt_prs[i], activeDiscSetIntValues[i]);
    for (i=0; i<num_deusiv; ++i)
      map_keys_to_set(deusi_vals_probs[i],activeDiscSetIntValues[i+num_dausiv]);
    break;
  }
  case MIXED_STATE: {
    size_t num_cv, num_div, num_dsv, num_drv, start_rv = 0,
      num_rv = svd.vc_lookup(DISCRETE_STATE_SET_INT);
    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
    start_rv += num_cv + num_div + num_dsv + num_drv;
    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    start_rv += num_cv + num_div + num_dsv + num_drv;
    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    start_rv += num_cv + num_div + num_dsv + num_drv
      + svd.vc_lookup(CONTINUOUS_STATE) + svd.vc_lookup(DISCRETE_STATE_RANGE);
    mvd_rep->pull_parameters<IntSet>(start_rv, num_rv, Pecos::DSI_VALUES,
				     activeDiscSetIntValues);
    break;
  }
  case MIXED_ALL: {
    IntRealMapArray h_pt_prs, deusi_vals_probs;
    mvd_rep->pull_parameters<IntRealMap>(Pecos::HISTOGRAM_PT_INT,
      Pecos::H_PT_INT_PAIRS, h_pt_prs);
    mvd_rep->pull_parameters<IntRealMap>(Pecos::DISCRETE_UNCERTAIN_SET_INT,
      Pecos::DUSI_VALUES_PROBS, deusi_vals_probs);
    size_t i, di_cntr = 0, num_ddsi = svd.vc_lookup(DISCRETE_DESIGN_SET_INT),
      num_dausi = h_pt_prs.size(), num_deusi = deusi_vals_probs.size(),
      num_dssi  = svd.vc_lookup(DISCRETE_STATE_SET_INT);
    activeDiscSetIntValues.resize(num_ddsi + num_dausi + num_deusi + num_dssi);
    size_t num_cv, num_div, num_dsv, num_drv;
    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
    size_t rv_cntr = num_cv + num_div - num_ddsi;
    for (i=0; i<num_ddsi; ++i, ++rv_cntr, ++di_cntr)
      mvd_rep->pull_parameter<IntSet>(rv_cntr, Pecos::DSI_VALUES,
				      activeDiscSetIntValues[di_cntr]);
    rv_cntr += num_dsv + num_drv;
    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    for (i=0; i<num_dausi; ++i, ++di_cntr)
      map_keys_to_set(h_pt_prs[i], activeDiscSetIntValues[di_cntr]);
    rv_cntr += num_cv + num_div + num_dsv + num_drv;
    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    for (i=0; i<num_deusi; ++i, ++di_cntr)
      map_keys_to_set(deusi_vals_probs[i], activeDiscSetIntValues[di_cntr]);
    rv_cntr += num_cv + num_div + num_dsv + num_drv +
      svd.vc_lookup(CONTINUOUS_STATE) + svd.vc_lookup(DISCRETE_STATE_RANGE);
    for (i=0; i<num_dssi; ++i, ++rv_cntr, ++di_cntr)
      mvd_rep->pull_parameter<IntSet>(rv_cntr, Pecos::DSI_VALUES,
				      activeDiscSetIntValues[di_cntr]);
    break;
  }
  default: { // RELAXED_*
    const BitArray&    all_relax_di = svd.all_relaxed_discrete_int();
    const SizetArray&    all_totals = svd.components_totals();
    const SizetArray& active_totals = svd.active_components_totals();
    size_t i, num_cv, num_div, num_dsv, num_drv,
           di_cntr = 0, ardi_cntr = 0, rv_cntr = 0;      
    // discrete design
    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
    if (active_totals[TOTAL_DDIV]) {
      size_t num_ddsi = svd.vc_lookup(DISCRETE_DESIGN_SET_INT),
	     num_ddri = num_div - num_ddsi;
      rv_cntr = num_cv;
      for (i=0; i<num_ddri; ++i, ++ardi_cntr, ++rv_cntr)
	if (!all_relax_di[ardi_cntr]) // part of active discrete vars
	  ++di_cntr;
      for (i=0; i<num_ddsi; ++i, ++ardi_cntr, ++rv_cntr)
	if (!all_relax_di[ardi_cntr]) // part of active discrete vars
	  mvd_rep->pull_parameter<IntSet>(rv_cntr, Pecos::DSI_VALUES,
					  activeDiscSetIntValues[di_cntr++]);
      rv_cntr += num_dsv + num_drv;
    }
    else {
      ardi_cntr += num_div;
      rv_cntr   += num_cv + num_div + num_dsv + num_drv;
    }
    // discrete aleatory uncertain
    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    if (active_totals[TOTAL_DAUIV]) {
      IntRealMapArray h_pt_prs;
      mvd_rep->pull_parameters<IntRealMap>(Pecos::HISTOGRAM_PT_INT,
        Pecos::H_PT_INT_PAIRS, h_pt_prs);
      size_t num_dausi = h_pt_prs.size(), num_dauri = num_div - num_dausi; 
      for (i=0; i<num_dauri; ++i, ++ardi_cntr)
	if (!all_relax_di[ardi_cntr]) // part of active discrete vars
	  ++di_cntr;
      for (i=0; i<num_dausi; ++i, ++ardi_cntr)
	if (!all_relax_di[ardi_cntr]) // part of active discrete vars
	  map_keys_to_set(h_pt_prs[i], activeDiscSetIntValues[di_cntr++]);
    }
    else
      ardi_cntr += num_div;
    rv_cntr += num_cv + num_div + num_dsv + num_drv;
    // discrete epistemic uncertain
    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    if (active_totals[TOTAL_DEUIV]) {
      IntRealMapArray deusi_vals_probs;
      mvd_rep->pull_parameters<IntRealMap>(Pecos::DISCRETE_UNCERTAIN_SET_INT,
        Pecos::DUSI_VALUES_PROBS, deusi_vals_probs);
      size_t num_deuri = svd.vc_lookup(DISCRETE_INTERVAL_UNCERTAIN),
	     num_deusi = deusi_vals_probs.size();
      for (i=0; i<num_deuri; ++i, ++ardi_cntr)
	if (!all_relax_di[ardi_cntr]) // part of active discrete vars
	  ++di_cntr;
      for (i=0; i<num_deusi; ++i, ++ardi_cntr)
	if (!all_relax_di[ardi_cntr]) // part of active discrete vars
	  map_keys_to_set(deusi_vals_probs[i],
			  activeDiscSetIntValues[di_cntr++]);
    }
    else
      ardi_cntr += num_div;
    rv_cntr += num_cv + num_div + num_dsv + num_drv;
    // discrete state
    if (active_totals[TOTAL_DSIV]) {
      size_t num_dssi = svd.vc_lookup(DISCRETE_STATE_SET_INT),
	     num_dsri = all_totals[TOTAL_DSIV] - num_dssi;
      rv_cntr += all_totals[TOTAL_CSV];
      for (i=0; i<num_dsri; ++i, ++ardi_cntr, ++rv_cntr)
	if (!all_relax_di[ardi_cntr]) // part of active discrete vars
	  ++di_cntr;                  // leave bit as false
      for (i=0; i<num_dssi; ++i, ++ardi_cntr, ++rv_cntr)
	if (!all_relax_di[ardi_cntr]) // part of active discrete vars
	  mvd_rep->pull_parameter<IntSet>(rv_cntr, Pecos::DSI_VALUES,
					  activeDiscSetIntValues[di_cntr++]);
    }
    break;
  }
  }

  prevDSIView = active_view;
  return activeDiscSetIntValues;
}


const StringSetArray& Model::discrete_set_string_values(short active_view)
{
  if (modelRep)
    return modelRep->discrete_set_string_values(active_view);

  // return previous result for previous invocation with consistent view
  // Note: any external update of DSS values should reset prevDSSView to 0
  if (active_view == prevDSSView) return activeDiscSetStringValues;

  std::shared_ptr<Pecos::MarginalsCorrDistribution> mvd_rep =
    std::static_pointer_cast<Pecos::MarginalsCorrDistribution>
    (mvDist.multivar_dist_rep());
  const SharedVariablesData& svd = currentVariables.shared_data();
  switch (active_view) {
  case MIXED_DESIGN: case RELAXED_DESIGN: {
    size_t num_cv, num_div, num_dsv, num_drv;      
    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
    mvd_rep->pull_parameters<StringSet>(num_cv + num_div, num_dsv,
      Pecos::DSS_VALUES, activeDiscSetStringValues);
    break;
  }
  case MIXED_ALEATORY_UNCERTAIN: case RELAXED_ALEATORY_UNCERTAIN: {
    StringRealMapArray h_pt_prs;
    mvd_rep->pull_parameters<StringRealMap>(Pecos::HISTOGRAM_PT_STRING,
      Pecos::H_PT_STR_PAIRS, h_pt_prs);
    size_t i, num_dauss = h_pt_prs.size();
    activeDiscSetStringValues.resize(num_dauss);
    for (i=0; i<num_dauss; ++i)
      map_keys_to_set(h_pt_prs[i], activeDiscSetStringValues[i]);
    break;
  }
  case MIXED_EPISTEMIC_UNCERTAIN: case RELAXED_EPISTEMIC_UNCERTAIN: {
    StringRealMapArray deuss_vals_probs;
    mvd_rep->pull_parameters<StringRealMap>(
      Pecos::DISCRETE_UNCERTAIN_SET_STRING, Pecos::DUSS_VALUES_PROBS,
      deuss_vals_probs);
    size_t i, num_deuss = deuss_vals_probs.size();
    activeDiscSetStringValues.resize(num_deuss);
    for (i=0; i<num_deuss; ++i)
      map_keys_to_set(deuss_vals_probs[i], activeDiscSetStringValues[i]);
    break;
  }
  case MIXED_UNCERTAIN: case RELAXED_UNCERTAIN: {
    StringRealMapArray h_pt_prs, deuss_vals_probs;
    mvd_rep->pull_parameters<StringRealMap>(Pecos::HISTOGRAM_PT_STRING,
      Pecos::H_PT_STR_PAIRS, h_pt_prs);
    mvd_rep->pull_parameters<StringRealMap>(
      Pecos::DISCRETE_UNCERTAIN_SET_STRING,
      Pecos::DUSS_VALUES_PROBS, deuss_vals_probs);
    size_t i, num_dauss = h_pt_prs.size(), num_deuss = deuss_vals_probs.size();
    activeDiscSetStringValues.resize(num_dauss + num_deuss);
    for (i=0; i<num_dauss; ++i)
      map_keys_to_set(h_pt_prs[i], activeDiscSetStringValues[i]);
    for (i=0; i<num_deuss; ++i)
      map_keys_to_set(deuss_vals_probs[i],
		      activeDiscSetStringValues[i+num_dauss]);
    break;
  }
  case MIXED_STATE: case RELAXED_STATE: {
    size_t num_cv, num_div, num_dsv, num_drv, start_rv = 0;
    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
    start_rv += num_cv + num_div + num_dsv + num_drv;
    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    start_rv += num_cv + num_div + num_dsv + num_drv;
    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    start_rv += num_cv + num_div + num_dsv + num_drv;
    svd.state_counts(num_cv, num_div, num_dsv, num_drv);
    start_rv += num_cv + num_div;
    mvd_rep->pull_parameters<StringSet>(start_rv, num_dsv, Pecos::DSS_VALUES,
					activeDiscSetStringValues);
    break;
  }
  case MIXED_ALL: case RELAXED_ALL: {
    StringRealMapArray h_pt_prs, deuss_vals_probs;
    mvd_rep->pull_parameters<StringRealMap>(Pecos::HISTOGRAM_PT_STRING,
      Pecos::H_PT_STR_PAIRS, h_pt_prs);
    mvd_rep->pull_parameters<StringRealMap>(
      Pecos::DISCRETE_UNCERTAIN_SET_STRING, Pecos::DUSS_VALUES_PROBS,
      deuss_vals_probs);
    size_t i, ds_cntr = 0,
      num_ddss  = svd.vc_lookup(DISCRETE_DESIGN_SET_STRING),
      num_dauss = h_pt_prs.size(), num_deuss= deuss_vals_probs.size(),
      num_dsss  = svd.vc_lookup(DISCRETE_STATE_SET_STRING);
    activeDiscSetStringValues.resize(num_ddss + num_dauss +
				     num_deuss + num_dsss);
    size_t num_cv, num_div, num_dsv, num_drv;
    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
    size_t rv_cntr = num_cv + num_div;
    for (i=0; i<num_ddss; ++i, ++rv_cntr, ++ds_cntr)
      mvd_rep->pull_parameter<StringSet>(rv_cntr, Pecos::DSS_VALUES,
					 activeDiscSetStringValues[ds_cntr]);
    rv_cntr += num_drv;
    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    for (i=0; i<num_dauss; ++i, ++ds_cntr)
      map_keys_to_set(h_pt_prs[i], activeDiscSetStringValues[ds_cntr]);
    rv_cntr += num_cv + num_div + num_dsv + num_drv;
    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    for (i=0; i<num_deuss; ++i, ++ds_cntr)
      map_keys_to_set(deuss_vals_probs[i], activeDiscSetStringValues[ds_cntr]);
    rv_cntr += num_cv + num_div + num_dsv + num_drv;
    svd.state_counts(num_cv, num_div, num_dsv, num_drv);
    rv_cntr += num_cv + num_div;
    for (i=0; i<num_dsss; ++i, ++rv_cntr, ++ds_cntr)
      mvd_rep->pull_parameter<StringSet>(rv_cntr, Pecos::DSS_VALUES,
					 activeDiscSetStringValues[ds_cntr]);
    break;
  }
  }

  prevDSSView = active_view;
  return activeDiscSetStringValues;
}


const RealSetArray& Model::discrete_set_real_values(short active_view)
{
  if (modelRep)
    return modelRep->discrete_set_real_values(active_view);

  // return previous result for previous invocation with consistent view
  // Note: any external update of DSR values should reset prevDSRView to 0
  if (active_view == prevDSRView) return activeDiscSetRealValues;

  std::shared_ptr<Pecos::MarginalsCorrDistribution> mvd_rep =
    std::static_pointer_cast<Pecos::MarginalsCorrDistribution>
    (mvDist.multivar_dist_rep());
  const SharedVariablesData& svd = currentVariables.shared_data();
  switch (active_view) {
  case MIXED_DESIGN: {
    size_t num_cv, num_div, num_dsv, num_drv;      
    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
    mvd_rep->pull_parameters<RealSet>(num_cv + num_div + num_dsv, num_drv,
      Pecos::DSR_VALUES, activeDiscSetRealValues);
    break;
  }
  case MIXED_ALEATORY_UNCERTAIN: {
    RealRealMapArray h_pt_prs;
    mvd_rep->pull_parameters<RealRealMap>(Pecos::HISTOGRAM_PT_REAL,
      Pecos::H_PT_REAL_PAIRS, h_pt_prs);
    size_t i, num_dausr = h_pt_prs.size();
    activeDiscSetRealValues.resize(num_dausr);
    for (i=0; i<num_dausr; ++i)
      map_keys_to_set(h_pt_prs[i], activeDiscSetRealValues[i]);
    break;
  }
  case MIXED_EPISTEMIC_UNCERTAIN: {
    RealRealMapArray deusr_vals_probs;
    mvd_rep->pull_parameters<RealRealMap>(Pecos::DISCRETE_UNCERTAIN_SET_REAL,
      Pecos::DUSR_VALUES_PROBS, deusr_vals_probs);
    size_t i, num_deusr = deusr_vals_probs.size();
    activeDiscSetRealValues.resize(num_deusr);
    for (i=0; i<num_deusr; ++i)
      map_keys_to_set(deusr_vals_probs[i], activeDiscSetRealValues[i]);
    break;
  }
  case MIXED_UNCERTAIN: {
    RealRealMapArray h_pt_prs, deusr_vals_probs;
    mvd_rep->pull_parameters<RealRealMap>(Pecos::HISTOGRAM_PT_REAL,
      Pecos::H_PT_REAL_PAIRS, h_pt_prs);
    mvd_rep->pull_parameters<RealRealMap>(Pecos::DISCRETE_UNCERTAIN_SET_REAL,
      Pecos::DUSR_VALUES_PROBS, deusr_vals_probs);
    size_t i, num_dausr = h_pt_prs.size(), num_deusr = deusr_vals_probs.size();
    activeDiscSetRealValues.resize(num_dausr + num_deusr);
    for (i=0; i<num_dausr; ++i)
      map_keys_to_set(h_pt_prs[i], activeDiscSetRealValues[i]);
    for (i=0; i<num_deusr; ++i)
      map_keys_to_set(deusr_vals_probs[i],activeDiscSetRealValues[i+num_dausr]);
    break;
  }
  case MIXED_STATE: {
    size_t num_cv, num_div, num_dsv, num_drv, start_rv = 0;
    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
    start_rv += num_cv + num_div + num_dsv + num_drv;
    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    start_rv += num_cv + num_div + num_dsv + num_drv;
    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    start_rv += num_cv + num_div + num_dsv + num_drv;
    svd.state_counts(num_cv, num_div, num_dsv, num_drv);
    start_rv += num_cv + num_div + num_dsv;
    mvd_rep->pull_parameters<RealSet>(start_rv, num_drv, Pecos::DSR_VALUES,
				      activeDiscSetRealValues);
    break;
  }
  case MIXED_ALL: {
    RealRealMapArray h_pt_prs, deusr_vals_probs;
    mvd_rep->pull_parameters<RealRealMap>(Pecos::HISTOGRAM_PT_REAL,
      Pecos::H_PT_REAL_PAIRS, h_pt_prs);
    mvd_rep->pull_parameters<RealRealMap>(Pecos::DISCRETE_UNCERTAIN_SET_REAL,
      Pecos::DUSR_VALUES_PROBS, deusr_vals_probs);
    size_t i, dr_cntr = 0, num_dausr = h_pt_prs.size(),
      num_deusr = deusr_vals_probs.size(),
      num_dssr  = svd.vc_lookup(DISCRETE_STATE_SET_REAL);
    size_t num_cv, num_div, num_dsv, num_drv;
    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
    activeDiscSetRealValues.resize(num_drv + num_dausr + num_deusr + num_dssr);
    size_t rv_cntr = num_cv + num_div + num_dsv;
    for (i=0; i<num_drv; ++i, ++rv_cntr, ++dr_cntr)
      mvd_rep->pull_parameter<RealSet>(rv_cntr, Pecos::DSR_VALUES,
				       activeDiscSetRealValues[dr_cntr]);
    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    for (i=0; i<num_dausr; ++i, ++dr_cntr)
      map_keys_to_set(h_pt_prs[i], activeDiscSetRealValues[dr_cntr]);
    rv_cntr += num_cv + num_div + num_dsv + num_drv;
    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    for (i=0; i<num_deusr; ++i, ++dr_cntr)
      map_keys_to_set(deusr_vals_probs[i], activeDiscSetRealValues[dr_cntr]);
    rv_cntr += num_cv + num_div + num_dsv + num_drv;
    svd.state_counts(num_cv, num_div, num_dsv, num_drv);
    rv_cntr += num_cv + num_div + num_dsv;
    for (i=0; i<num_drv; ++i, ++rv_cntr, ++dr_cntr)
      mvd_rep->pull_parameter<RealSet>(rv_cntr, Pecos::DSR_VALUES,
				       activeDiscSetRealValues[dr_cntr]);
    break;
  }
  default: { // RELAXED_*
    const BitArray&    all_relax_dr = svd.all_relaxed_discrete_real();
    const SizetArray&    all_totals = svd.components_totals();
    const SizetArray& active_totals = svd.active_components_totals();
    size_t i, num_cv, num_div, num_dsv, num_drv,
           dr_cntr = 0, ardr_cntr = 0, rv_cntr = 0;
    // discrete design
    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
    if (active_totals[TOTAL_DDRV]) {
      rv_cntr = num_cv + num_div + num_dsv;
      for (i=0; i<num_drv; ++i, ++ardr_cntr, ++rv_cntr)
	if (!all_relax_dr[ardr_cntr]) // part of active discrete vars
	  mvd_rep->pull_parameter<RealSet>(rv_cntr, Pecos::DSR_VALUES,
					   activeDiscSetRealValues[dr_cntr++]);
    }
    else {
      ardr_cntr += num_drv;
      rv_cntr   += num_cv + num_div + num_dsv + num_drv;
    }
    // discrete aleatory uncertain
    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    if (active_totals[TOTAL_DAURV]) {
      RealRealMapArray h_pt_prs;
      mvd_rep->pull_parameters<RealRealMap>(Pecos::HISTOGRAM_PT_REAL,
        Pecos::H_PT_REAL_PAIRS, h_pt_prs);
      size_t num_dausr = h_pt_prs.size(); 
      for (i=0; i<num_dausr; ++i, ++ardr_cntr)
	if (!all_relax_dr[ardr_cntr]) // part of active discrete vars
	  map_keys_to_set(h_pt_prs[i], activeDiscSetRealValues[dr_cntr++]);
    }
    else
      ardr_cntr += num_drv;
    rv_cntr += num_cv + num_div + num_dsv + num_drv;
    // discrete epistemic uncertain
    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    if (active_totals[TOTAL_DEURV]) {
      RealRealMapArray deusr_vals_probs;
      mvd_rep->pull_parameters<RealRealMap>(Pecos::DISCRETE_UNCERTAIN_SET_REAL,
        Pecos::DUSR_VALUES_PROBS, deusr_vals_probs);
      size_t num_deusr = deusr_vals_probs.size();
      for (i=0; i<num_deusr; ++i, ++ardr_cntr)
	if (!all_relax_dr[ardr_cntr]) // part of active discrete vars
	  map_keys_to_set(deusr_vals_probs[i],
			  activeDiscSetRealValues[dr_cntr++]);
    }
    else
      ardr_cntr += num_drv;
    rv_cntr += num_cv + num_div + num_dsv + num_drv;
    // discrete state
    if (active_totals[TOTAL_DSRV]) {
      svd.state_counts(num_cv, num_div, num_dsv, num_drv);
      rv_cntr += num_cv + num_div + num_dsv;
      for (i=0; i<num_drv; ++i, ++ardr_cntr, ++rv_cntr)
	if (!all_relax_dr[ardr_cntr]) // part of active discrete vars
	  mvd_rep->pull_parameter<RealSet>(rv_cntr, Pecos::DSR_VALUES,
					   activeDiscSetRealValues[dr_cntr++]);
    }
    break;
  }
  }

  prevDSRView = active_view;
  return activeDiscSetRealValues;
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
    Possible exceptions: EnsembleSurrModel, NestedModel::optionalInterface. */
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
    return modelRep->db_lookup(search_vars, search_set, found_resp);
  else { // default implementation
    // dependence on interface_id() restricts successful find() operation to
    // cases where response is generated by a single non-approximate interface
    // at this level.  For Nested and Surrogate models, duplication detection
    // must occur at a lower level.
    PRPCacheHIter cache_it
      = lookup_by_val(data_pairs, interface_id(), search_vars, search_set);
    if (cache_it != data_pairs.get<hashed>().end()) {
      found_resp.active_set(search_set);
      found_resp.update(cache_it->response(), true); // update metadata
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


void Model::evaluate(const RealMatrix& samples_matrix,
		     Model& model, RealMatrix& resp_matrix)
{
  // TODO: option for setting its active or inactive variables

  RealMatrix::ordinalType i, num_evals = samples_matrix.numCols();
  resp_matrix.shape(model.response_size(), num_evals);

  for (i=0; i<num_evals; ++i) {

    const RealVector sample_i =
      Teuchos::getCol(Teuchos::View, const_cast<RealMatrix&>(samples_matrix), i);
    Model::active_variables(sample_i, model);

    if (model.asynch_flag())
      model.evaluate_nowait();
    else {
      model.evaluate();
      const RealVector& fn_vals = model.current_response().function_values();
      Teuchos::setCol(fn_vals, i, resp_matrix);
    }
  }

  // synchronize asynchronous evaluations
  if (model.asynch_flag()) {
    const IntResponseMap& resp_map = model.synchronize();
    IntRespMCIter r_cit;
    for (i=0, r_cit=resp_map.begin(); r_cit!=resp_map.end(); ++i, ++r_cit)
      Teuchos::setCol(r_cit->second.function_values(), i, resp_matrix);
  }
}


void Model::evaluate(const VariablesArray& sample_vars,
		     Model& model, RealMatrix& resp_matrix)
{
  // TODO: option for setting its active or inactive variables

  RealMatrix::ordinalType i, num_evals = sample_vars.size();
  resp_matrix.shape(model.response_size(), num_evals);

  for (i=0; i<num_evals; ++i) {
    model.active_variables(sample_vars[i]);
    if (model.asynch_flag())
      model.evaluate_nowait();
    else {
      model.evaluate();
      const RealVector& fn_vals = model.current_response().function_values();
      Teuchos::setCol(fn_vals, i, resp_matrix);
    }
  }

  // synchronize asynchronous evaluations
  if (model.asynch_flag()) {
    const IntResponseMap& resp_map = model.synchronize();
    IntRespMCIter r_cit;
    for (i=0, r_cit=resp_map.begin(); r_cit!=resp_map.end(); ++i, ++r_cit)
      Teuchos::setCol(r_cit->second.function_values(), i, resp_matrix);
  }
}


// Called from rekey_response_map to allow Models to store their interfaces asynchronous
// evaluations. When the meta_object is a model, no action is performed.
void Model::asynch_eval_store(const Model &model, const int &id, const Response &response) {
  return;
}

// Called from rekey_response_map to allow Models to store their interfaces asynchronous
// evaluations. I strongly suspect that there's a better design for this.
void Model::asynch_eval_store(const Interface &interface, const int &id, const Response &response) {
  evaluationsDB.store_interface_response(modelId, interface.interface_id(), id, response);
}

/// Return the interface flag for the EvaluationsDB state
EvaluationsDBState Model::evaluations_db_state(const Interface &interface) {
  return interfEvaluationsDBState;
}
  /// Return the model flag for the EvaluationsDB state
EvaluationsDBState Model::evaluations_db_state(const Model &model) {
  // always return INACTIVE because models don't store evaluations of their
  // submodels
  return EvaluationsDBState::INACTIVE;
}

/** Rationale: The parser allows multiple user-specified models with
    empty (unspecified) ID. However, only a single Model with empty
    ID can be constructed (if it's the only one present, or the "last
    one parsed"). Therefore decided to prefer NO_MODEL_ID over 
    NO_MODEL_ID_<num> for (some) consistency with interface 
    NO_ID convention. _MODEL_ was inserted in the middle to distinguish
    "anonymous" MODELS from methods and interfaces in the hdf5 output. 
    Note that this function is not used to name recast models; see their 
    constructors for how its done. */
String Model::user_auto_id()
{
  // // increment and then use the current ID value
  // return String("NO_ID_") + std::to_string(++userAutoIdNum);
  return String("NO_MODEL_ID");
}

/** Rationale: For now NOSPEC_MODEL_ID_ is chosen due to historical
    id="NO_SPECIFICATION" used for internally-constructed
    Models. Longer-term, consider auto-generating an ID that
    includes the context from which the method is constructed, e.g.,
    the parent method or model's ID, together with its name. 
    Note that this function is not used to name recast models; see 
    their constructors for how its done.
**/
String Model::no_spec_id()
{
  // increment and then use the current ID value
  return String("NOSPEC_MODEL_ID_") + std::to_string(++noSpecIdNum);
}

// This is overridden by RecastModel so that it and its derived classes return 
// the root_model_id() of their subModels. The base Model class version terminates
// the "recursion" for models of other types.
String Model::root_model_id() {
  if(modelRep) return modelRep->root_model_id();
  else return modelId;
}

} // namespace Dakota
