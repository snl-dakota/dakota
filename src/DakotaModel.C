/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Model
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "system_defs.h"
#include "DakotaModel.H"
#include "ParamResponsePair.H"
#include "PRPMultiIndex.H"
#include "ParallelLibrary.H"
#include "ProblemDescDB.H"
#include "SingleModel.H"
#include "NestedModel.H"
#include "DataFitSurrModel.H"
#include "HierarchSurrModel.H"
#include "DakotaGraphics.H"

static const char rcsId[]="@(#) $Id: DakotaModel.C 7029 2010-10-22 00:17:02Z mseldre $";


namespace Dakota {

extern ParallelLibrary dummy_lib;       // defined in global_defs.C
extern ProblemDescDB   dummy_db;        // defined in global_defs.C
extern Graphics        dakota_graphics; // defined in global_defs.C

// These globals defined here rather than in global_defs.C in order to minimize
// dakota_restart_util object file dependencies
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
  currentResponse(problem_db.get_response(currentVariables)),
  numFns(currentResponse.num_functions()),
  userDefinedConstraints(problem_db, currentVariables.shared_data()),
  modelType(problem_db.get_string("model.type")),
  surrogateType(problem_db.get_string("model.surrogate.type")),
  gradType(problem_db.get_string("responses.gradient_type")),
  methodSrc(problem_db.get_string("responses.method_source")),
  intervalType(problem_db.get_string("responses.interval_type")),
  fdGradSS(problem_db.get_rdv("responses.fd_gradient_step_size")),
  gradIdAnalytic(problem_db.get_dil("responses.gradients.mixed.id_analytic")),
  gradIdNumerical(problem_db.get_dil("responses.gradients.mixed.id_numerical")),
  hessType(problem_db.get_string("responses.hessian_type")),
  quasiHessType(problem_db.get_string("responses.quasi_hessian_type")),
  fdHessByFnSS(problem_db.get_rdv("responses.fd_hessian_step_size")),
  fdHessByGradSS(problem_db.get_rdv("responses.fd_hessian_step_size")),
  hessIdAnalytic(problem_db.get_dil("responses.hessians.mixed.id_analytic")),
  hessIdNumerical(problem_db.get_dil("responses.hessians.mixed.id_numerical")),
  hessIdQuasi(problem_db.get_dil("responses.hessians.mixed.id_quasi")),
  supportsEstimDerivs(true),
  probDescDB(problem_db), parallelLib(problem_db.parallel_library()),
  modelPCIter(parallelLib.parallel_configuration_iterator()),
  componentParallelMode(0), asynchEvalFlag(false),
  // See base constructor in DakotaIterator.C for full discussion of output
  // verbosity.  For models, QUIET_OUTPUT turns off response reporting and
  // SILENT_OUTPUT additionally turns off fd_gradient parameter set reporting.
  outputLevel(problem_db.get_short("method.output")),
  discreteDesignSetIntValues(
    problem_db.get_disa("variables.discrete_design_set_int.set_values")),
  discreteDesignSetRealValues(
    problem_db.get_drsa("variables.discrete_design_set_real.set_values")),
  distParams(problem_db.get_rdv("variables.normal_uncertain.means"),
    problem_db.get_rdv("variables.normal_uncertain.std_deviations"),
    problem_db.get_rdv("variables.normal_uncertain.lower_bounds"),
    problem_db.get_rdv("variables.normal_uncertain.upper_bounds"),
    problem_db.get_rdv("variables.lognormal_uncertain.means"),
    problem_db.get_rdv("variables.lognormal_uncertain.std_deviations"),
    problem_db.get_rdv("variables.lognormal_uncertain.lambdas"),
    problem_db.get_rdv("variables.lognormal_uncertain.zetas"),
    problem_db.get_rdv("variables.lognormal_uncertain.error_factors"),
    problem_db.get_rdv("variables.lognormal_uncertain.lower_bounds"),
    problem_db.get_rdv("variables.lognormal_uncertain.upper_bounds"),
    problem_db.get_rdv("variables.uniform_uncertain.lower_bounds"),
    problem_db.get_rdv("variables.uniform_uncertain.upper_bounds"),
    problem_db.get_rdv("variables.loguniform_uncertain.lower_bounds"),
    problem_db.get_rdv("variables.loguniform_uncertain.upper_bounds"),
    problem_db.get_rdv("variables.triangular_uncertain.modes"),
    problem_db.get_rdv("variables.triangular_uncertain.lower_bounds"),
    problem_db.get_rdv("variables.triangular_uncertain.upper_bounds"),
    problem_db.get_rdv("variables.exponential_uncertain.betas"),
    problem_db.get_rdv("variables.beta_uncertain.alphas"),
    problem_db.get_rdv("variables.beta_uncertain.betas"),
    problem_db.get_rdv("variables.beta_uncertain.lower_bounds"),
    problem_db.get_rdv("variables.beta_uncertain.upper_bounds"),
    problem_db.get_rdv("variables.gamma_uncertain.alphas"),
    problem_db.get_rdv("variables.gamma_uncertain.betas"),
    problem_db.get_rdv("variables.gumbel_uncertain.alphas"),
    problem_db.get_rdv("variables.gumbel_uncertain.betas"),
    problem_db.get_rdv("variables.frechet_uncertain.alphas"),
    problem_db.get_rdv("variables.frechet_uncertain.betas"),
    problem_db.get_rdv("variables.weibull_uncertain.alphas"),
    problem_db.get_rdv("variables.weibull_uncertain.betas"),
    problem_db.get_rdva("variables.histogram_uncertain.bin_pairs"),
    problem_db.get_rdv("variables.poisson_uncertain.lambdas"),
    problem_db.get_rdv("variables.binomial_uncertain.prob_per_trial"),
    problem_db.get_idv("variables.binomial_uncertain.num_trials"),
    problem_db.get_rdv("variables.negative_binomial_uncertain.prob_per_trial"),
    problem_db.get_idv("variables.negative_binomial_uncertain.num_trials"),
    problem_db.get_rdv("variables.geometric_uncertain.prob_per_trial"),
    problem_db.get_idv("variables.hypergeometric_uncertain.total_population"),
    problem_db.get_idv(
      "variables.hypergeometric_uncertain.selected_population"),
    problem_db.get_idv("variables.hypergeometric_uncertain.num_drawn"),
    problem_db.get_rdva("variables.histogram_uncertain.point_pairs"),
    problem_db.get_rdva("variables.interval_uncertain.basic_probs"),
    problem_db.get_rdva("variables.interval_uncertain.bounds"),
    problem_db.get_rsdm("variables.uncertain.correlation_matrix")),
  discreteStateSetIntValues(
    problem_db.get_disa("variables.discrete_state_set_int.set_values")),
  discreteStateSetRealValues(
    problem_db.get_drsa("variables.discrete_state_set_real.set_values")),
  primaryRespFnWts(probDescDB.get_rdv("responses.primary_response_fn_weights")),
  modelId(problem_db.get_string("model.id")), modelEvalCntr(0),
  estDerivsFlag(false), evaluationCapacity(1), initCommsBcastFlag(false),
  modelAutoGraphicsFlag(false), modelRep(NULL), referenceCount(1)
{
  // Promote fdGradSS/fdHessByFnSS/fdHessByGradSS to defaults if needed.
  // Note: the fdSS arrays specialize by variable, whereas mixed grads/Hessians
  // specialize by function.
  if ( gradType == "numerical" ||
       ( gradType == "mixed" && !gradIdNumerical.empty() ) ) {
    if (fdGradSS.empty()) {
      fdGradSS.resize(1);
      fdGradSS[0] = 0.001;
    }
  }
  if ( hessType == "numerical" ||
       ( hessType == "mixed" && !hessIdNumerical.empty() ) ) {
    // fdHessByFnSS and fdHessByGradSS can only differ currently in the case
    // of assignment of default values, since the same fd_hessian_step_size
    // input is reused for both first- and second-order differencing.  If
    // needed in the future (numerical Hessians with mixed gradients require
    // both first- and second-order step sizes), separate inputs could be
    // added and easily accomodated here.
    if (fdHessByFnSS.empty()) {
      fdHessByFnSS.resize(1);
      fdHessByFnSS[0] = 0.002;
    }
    if (fdHessByGradSS.empty()) {
      fdHessByGradSS.resize(1);
      fdHessByGradSS[0] = 0.001;
    }
  }

#ifdef REFCOUNT_DEBUG
  Cout << "Model::Model(BaseConstructor, ProblemDescDB&) called "
       << "to build letter base class\n";
#endif
}


Model::
Model(NoDBBaseConstructor, ParallelLibrary& parallel_lib,
      const SharedVariablesData& svd, const ActiveSet& set, short output_level):
  currentVariables(svd), numDerivVars(set.derivative_vector().size()),
  currentResponse(set), numFns(set.request_vector().size()),
  userDefinedConstraints(svd), supportsEstimDerivs(true),
  probDescDB(dummy_db), parallelLib(parallel_lib),
  modelPCIter(parallel_lib.parallel_configuration_iterator()),
  componentParallelMode(0), asynchEvalFlag(false), outputLevel(output_level),
  modelId("NO_DB_MODEL"), modelEvalCntr(0), estDerivsFlag(false),
  evaluationCapacity(1), initCommsBcastFlag(false),
  modelAutoGraphicsFlag(false), modelRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Model::Model(NoDBBaseConstructor) called to build letter base class "
       << "on the fly\n";
#endif
}


/** This constructor also builds the base class data for inherited
    models.  However, it is used for recast models which are
    instantiated on the fly.  Therefore it only initializes a small
    subset of attributes.  Note that parallel_lib is managed
    separately from problem_db since parallel_lib is needed even in
    cases where problem_db is an empty envelope (i.e., use of dummy_db
    in Model(NoDBBaseConstructor) above. */
Model::
Model(RecastBaseConstructor, ProblemDescDB& problem_db,
      ParallelLibrary& parallel_lib):
  probDescDB(problem_db), parallelLib(parallel_lib),
  modelPCIter(parallel_lib.parallel_configuration_iterator()),
  modelType("recast"), supportsEstimDerivs(false),
  componentParallelMode(0), asynchEvalFlag(false), modelEvalCntr(0),
  estDerivsFlag(false), evaluationCapacity(1), initCommsBcastFlag(false),
  modelAutoGraphicsFlag(false), modelRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Model::Model(RecastBaseConstructor, ProblemDescDB&) called to build "
       << "letter base class\n";
#endif
}


/** The default constructor is used in vector<Model> instantiations
    and for initialization of Model objects contained in Iterator and
    derived Strategy classes.  modelRep is NULL in this case (a
    populated problem_db is needed to build a meaningful Model
    object).  This makes it necessary to check for NULL in the copy
    constructor, assignment operator, and destructor. */
Model::Model(): modelRep(NULL), referenceCount(1), probDescDB(dummy_db),
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
    abort_handler(-1);
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
  if ( model_type == "single" )
    return new SingleModel(problem_db);
  else if ( model_type == "nested")
    return new NestedModel(problem_db);
  else if ( model_type == "surrogate") {
    if (problem_db.get_string("model.surrogate.type") == "hierarchical")
      return new HierarchSurrModel(problem_db); // hierarchical approx
    else
      return new DataFitSurrModel(problem_db);  // local/multipt/global approx
  }
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
    modelRep->referenceCount++;

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
      modelRep->referenceCount++;
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
      abort_handler(-1);
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


void Model::compute_response()
{
  if (modelRep) // envelope fwd to letter
    modelRep->compute_response();
  else { // letter
    ++modelEvalCntr;

    // Define default ActiveSet for iterators which don't pass one
    ActiveSet temp_set = currentResponse.active_set(); // copy
    temp_set.request_values(1); // function values only

    if (derived_master_overload()) {
      // prevents error of trying to run a multiproc. direct job on the master
      derived_asynch_compute_response(temp_set);
      currentResponse = derived_synchronize().begin()->second;
    }
    else // perform a normal synchronous map
      derived_compute_response(temp_set);

    if (modelAutoGraphicsFlag)
      dakota_graphics.add_datapoint(currentVariables, currentResponse);
  }
}


void Model::compute_response(const ActiveSet& set)
{
  if (modelRep) // envelope fwd to letter
    modelRep->compute_response(set);
  else { // letter
    ++modelEvalCntr;

    // Derivative estimation support goes here and is not replicated in the
    // default asv version of compute_response -> a good reason for using an
    // overloaded function design rather than a default parameter design.
    ShortArray map_asv(numFns, 0), fd_grad_asv(numFns, 0),
      fd_hess_asv(numFns, 0), quasi_hess_asv(numFns, 0);
    // Manage map/estimate_derivs for a particular asv based on responses spec.
    bool use_est_deriv = manage_asv(set.request_vector(), map_asv, fd_grad_asv,
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
      derived_asynch_compute_response(set);
      currentResponse = derived_synchronize().begin()->second;
    }
    else
      // Perform synchronous eval
      derived_compute_response(set);

    if (modelAutoGraphicsFlag)
      dakota_graphics.add_datapoint(currentVariables, currentResponse);
  }
}


void Model::asynch_compute_response()
{
  if (modelRep) // envelope fwd to letter
    modelRep->asynch_compute_response();
  else { // letter
    ++modelEvalCntr;

    // Define default ActiveSet for iterators which don't pass one
    ActiveSet temp_set = currentResponse.active_set(); // copy
    temp_set.request_values(1); // function values only

    // perform an asynchronous parameter-to-response mapping
    derived_asynch_compute_response(temp_set);

    // history of vars must be catalogued for use in synchronize()
    if (modelAutoGraphicsFlag)
      varsList.push_back(currentVariables.copy());
    rawEvalIdMap[evaluation_id()] = modelEvalCntr;
    numFDEvalsMap[modelEvalCntr] = -1;//no deriv est; distinguish from QN update
  }
}


void Model::asynch_compute_response(const ActiveSet& set)
{
  if (modelRep) // envelope fwd to letter
    modelRep->asynch_compute_response(set);
  else { // letter
    ++modelEvalCntr;

    // Manage use of estimate_derivatives() for a particular asv based on
    // the user's gradients/Hessians spec.
    ShortArray map_asv(numFns, 0), fd_grad_asv(numFns, 0),
      fd_hess_asv(numFns, 0), quasi_hess_asv(numFns, 0);
    bool use_est_deriv = manage_asv(set.request_vector(), map_asv, fd_grad_asv,
				    fd_hess_asv, quasi_hess_asv);
    int num_fd_evals;
    if (use_est_deriv) {
      // Compute requested derivatives not available from the simulation.  Since
      // we expect multiple asynch_compute_response()/estimate_derivatives()
      // calls prior to synchronize()/synchronize_derivatives(), we must perform
      // some additional bookkeeping so that the response arrays can be properly
      // recombined into estimated gradients/Hessians.
      estDerivsFlag = true;
      asvList.push_back(fd_grad_asv);
      asvList.push_back(fd_hess_asv);
      asvList.push_back(quasi_hess_asv);
      setList.push_back(set);
      // This estimate_derivatives() call always uses asynch evals.
      num_fd_evals = estimate_derivatives(map_asv, fd_grad_asv, fd_hess_asv,
					  quasi_hess_asv, set, true);
    }
    else {
      derived_asynch_compute_response(set);
      num_fd_evals = -1; // no deriv est; distinguish from QN updating
    }

    // history of vars must be catalogued for use in synchronize
    if (modelAutoGraphicsFlag || num_fd_evals >= 0)
      varsList.push_back(currentVariables.copy());
    rawEvalIdMap[evaluation_id()] = modelEvalCntr;
    numFDEvalsMap[modelEvalCntr] = num_fd_evals;
  }
}


const IntResponseMap& Model::synchronize()
{
  if (modelRep) // envelope fwd to letter
    return modelRep->synchronize();
  else { // letter
    responseMap.clear();

    const IntResponseMap& raw_response_map = derived_synchronize();
    VarsLIter     v_it;
    IntRespMCIter r_cit;
    IntIntMIter   m_it;

    if (estDerivsFlag) { // merge several responses into response gradients
      if (outputLevel > QUIET_OUTPUT)
        Cout <<"-----------------------------------------\n"
             << "Raw asynchronous response data captured.\n"
	     << "Merging data to estimate derivatives:\n"
	     << "----------------------------------------\n\n";
      v_it  = varsList.begin();
      r_cit = raw_response_map.begin();
      for (m_it = numFDEvalsMap.begin(); m_it != numFDEvalsMap.end(); ++m_it) {
	int model_id = m_it->first, num_fd_evals = m_it->second;
        if (num_fd_evals >= 0) {
	  // estimate_derivatives() was used: merge raw FD responses into 1
	  // response or augment response with quasi-Hessian updating
          if (outputLevel > QUIET_OUTPUT) {
	    if (num_fd_evals > 1)
	      Cout << "Merging asynchronous responses " << r_cit->first
		   << " through " << r_cit->first+num_fd_evals-1 << '\n';
	    else
	      Cout << "Augmenting asynchronous response " << r_cit->first
		   << " with quasi-Hessian updating\n";
	  }
          IntResponseMap tmp_response_map;
          for (size_t j=0; j<num_fd_evals; ++j, ++r_cit)
            tmp_response_map[r_cit->first] = r_cit->second;
          // Recover fd_grad/fd_hess/quasi_hess asv's from asvList and
	  // orig_set from setList
          ShortArray fd_grad_asv    = asvList.front(); asvList.pop_front();
          ShortArray fd_hess_asv    = asvList.front(); asvList.pop_front();
	  ShortArray quasi_hess_asv = asvList.front(); asvList.pop_front();
	  ActiveSet  orig_set       = setList.front(); setList.pop_front();
          synchronize_derivatives(*v_it, tmp_response_map,
				  responseMap[model_id], fd_grad_asv,
				  fd_hess_asv, quasi_hess_asv, orig_set);
	  ++v_it;
	}
        else {
	  // number of maps==1, estimate_derivatives() not called for this eval,
	  // no need to merge
          if (outputLevel > QUIET_OUTPUT)
            Cout << "Asynchronous response " << r_cit->first
		 << " does not require merging.\n";
          responseMap[model_id] = r_cit->second;
	  ++r_cit;
	  if (modelAutoGraphicsFlag)
	    ++v_it;
	}
      }
      // reset flags
      estDerivsFlag = false;
    }
    else // no calls to estimate_derivatives()
      // rekey the raw response map (lower level evaluation ids may be offset
      // from modelEvalCntr if previous finite differencing occurred)
      for (r_cit  = raw_response_map.begin(), m_it = numFDEvalsMap.begin();
	   r_cit != raw_response_map.end(); ++r_cit, ++m_it)
	responseMap[m_it->first] = r_cit->second;

    // update graphics
    if (modelAutoGraphicsFlag)
      for (r_cit  = responseMap.begin(), v_it = varsList.begin();
	   r_cit != responseMap.end(); ++r_cit, ++v_it)
	dakota_graphics.add_datapoint(*v_it, r_cit->second);
    // reset bookkeeping lists
    numFDEvalsMap.clear();
    rawEvalIdMap.clear();
    varsList.clear();
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
      abort_handler(-1);
    }

    const IntResponseMap& raw_response_map = derived_synchronize_nowait();

    // rekey and cleanup.
    // Note 1: rekeying is needed for the case of mixed usage of synchronize()
    // and synchronize_nowait(), since the former can introduce offsets.
    // Note 2: if estimate_derivatives() support is added, then rawEvalIdMap
    // data input must be expanded to include FD evals.
    for (IntRespMCIter r_cit = raw_response_map.begin();
	 r_cit != raw_response_map.end(); ++r_cit) {
      int raw_id = r_cit->first, model_id = rawEvalIdMap[raw_id];
      responseMap[model_id] = r_cit->second;
      rawEvalIdMap.erase(raw_id);
      numFDEvalsMap.erase(model_id);
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
	int graphics_cntr = dakota_graphics.graphics_counter();
	// find() is not really necessary due to Map ordering
	//g_it = graphicsRespMap.begin();
	//if (g_it == graphicsRespMap.end() || g_it->first != graphics_cntr)
	IntRespMIter g_it = graphicsRespMap.find(graphics_cntr);
	if (g_it == graphicsRespMap.end())
	  found = false;
	else {
	  dakota_graphics.add_datapoint(varsList.front(), g_it->second);
	  varsList.pop_front();
	  graphicsRespMap.erase(g_it);
	}
      }
    }
    //else // varsList already empty since estimate_derivatives() not supported
    //  varsList.clear();

    return responseMap;
  }
}


/** Auxiliary function to compute forward or first central-difference
    step size. */
Real Model::
FDstep1(const Real& x0_j, const Real& lb_j, const Real& ub_j, Real h_mag)
{
  Real h, h1, h2;
  shortStep = false;
  if (x0_j < 0.) {
    h = -h_mag;
    if (x0_j + h < lb_j) {
      if (x0_j + h_mag <= ub_j)
	h = h_mag;
      else
	goto shorten;
    }
  }
  else {
    h = h_mag;
    if (x0_j + h > ub_j) {
      if (x0_j - h_mag >= lb_j)
	h = -h_mag;
      else {
      shorten:
	shortStep = true;
	h1 = x0_j - lb_j;
	h2 = ub_j - x0_j;
	if (h1 < h2)
	  h = h2;
	else
	  h = -h1;
      }
    }
  }
  return h;
}


/** Auxiliary function to second central-difference step size,
    honoring bounds. */
Real Model::
FDstep2(const Real& x0_j, const Real& lb_j, const Real& ub_j, Real h)
{
  Real h2 = -h;

  if (shortStep)
    h2 = 0.5*h;
  else if (!ignoreBounds) {
    Real h1;
    if (h2 < 0.) {
      if (x0_j + h2 < lb_j) {
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
  // iterator such as OPT++ requests fn. values in one compute_response call
  // followed by a gradient request, followed by a Hessian request), so perform
  // a database search when appropriate to retrieve the data instead of relying
  // solely on duplication detection.
  bool initial_map = false, augmented_data_flag = false, db_capture = false,
       fd_grad_flag = false, fd_hess_flag = false, fd_hess_by_fn_flag = false,
       fd_hess_by_grad_flag = false;
  const ShortArray& original_asv = original_set.request_vector();
  const SizetArray& original_dvv = original_set.derivative_vector();
  ActiveSet new_set = original_set; // copy
  size_t i, j, k, map_counter = 0, num_deriv_vars = original_dvv.size();
  const RealVector *fn_vals_x0 = 0, *f2 = 0;
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

  // The logic for performing a data_pairs search is that a data request
  // contained in original_asv is most likely not a duplicate, but that an
  // augmented data requirement (appears in map_asv but not in original_asv)
  // may have been evaluated previously.
  Response initial_map_response(currentResponse.copy());
  new_set.request_vector(map_asv);
  initial_map_response.active_set(new_set);
  if (augmented_data_flag) {
    // dependence on interface_id() restricts successful find() operation to
    // cases where response is generated by a single non-approximate interface
    // at this level.  For Nested and Surrogate models, duplication detection
    // must occur at a lower level.
    extern PRPCache data_pairs;
    Response desired_resp;
    if( lookup_by_val(data_pairs, interface_id(), currentVariables, new_set,
		      desired_resp) ) {
      if (outputLevel > SILENT_OUTPUT)
	Cout << ">>>>> map at X performed previously and results retrieved\n\n";
      initial_map_response.update(desired_resp);
      initial_map = false; // reset
      if (asynch_flag) {
	db_capture = true;
	dbResponseList.push_back(initial_map_response);
      }
    }
  }

  if (initial_map) {
    if (outputLevel > SILENT_OUTPUT) {
      if (augmented_data_flag)
        Cout << ">>>>> Initial map for analytic portion of response\n      "
	     << "augmented with data requirements for differencing:\n";
      else
        Cout << ">>>>> Initial map for analytic portion of response:\n";
    }
    if (asynch_flag) {
      derived_asynch_compute_response(new_set);
      if (outputLevel > SILENT_OUTPUT)
	Cout << "\n\n";
    }
    else {
      derived_compute_response(new_set);
      initial_map_response.update(currentResponse);
    }
    ++map_counter;
  }
  if (asynch_flag) { // communicate settings to synchronize_derivatives()
    initialMapList.push_back(initial_map);
    dbCaptureList.push_back(db_capture);
  }
  else if (fd_grad_flag || fd_hess_by_fn_flag)
    fn_vals_x0 = &initial_map_response.function_values();

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
    bool active_derivs = false, inactive_derivs = false;
    RealVector x0;
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
    // define l_bnds, u_bnds, var_ids
    const RealVector& l_bnds = (active_derivs) ? continuous_lower_bounds() :
      ( (inactive_derivs) ? inactive_continuous_lower_bounds() :
	                         all_continuous_lower_bounds() );
    const RealVector& u_bnds = (active_derivs) ? continuous_upper_bounds() :
      ( (inactive_derivs) ? inactive_continuous_upper_bounds() :
	                         all_continuous_upper_bounds() );
    SizetMultiArrayConstView var_ids = (active_derivs) ?
      continuous_variable_ids() : ( (inactive_derivs) ?
      inactive_continuous_variable_ids() : all_continuous_variable_ids() );

    // ------------------------
    // Loop over num_deriv_vars
    // ------------------------
    RealVector x = x0;
    for (j=0; j<num_deriv_vars; j++) { // difference the 1st num_deriv_vars vars
      size_t xj_index = find_index(var_ids, original_dvv[j]);
      const Real& lb_j = l_bnds[xj_index]; const Real& ub_j = u_bnds[xj_index];
      const Real& x0_j = x0[xj_index];

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
	// Enforce a minimum delta of fdgss*.01
	Real fdgss = (fdGradSS.length() == num_deriv_vars)
	           ? fdGradSS[xj_index] : fdGradSS[0];
	Real h = FDstep1(x0_j, lb_j, ub_j, fdgss*std::max(std::fabs(x0_j),.01));
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
	  derived_asynch_compute_response(new_set);
	  if (outputLevel > SILENT_OUTPUT)
	    Cout << "\n\n";
	}
	else {
	  derived_compute_response(new_set);
	  fn_vals_x_plus_h = currentResponse.function_values();
	  if (intervalType == "forward") {
	    for (i=0; i<numFns; i++)
	      // prevent erroneous difference of vals present in fn_vals_x0 but
	      // not in fn_vals_x_plus_h because of map/fd_grad asv differences
	      if (fd_grad_asv[i])
		new_fn_grads(j,i)
		  = (fn_vals_x_plus_h[i] - (*fn_vals_x0)[i])/h;
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
	    derived_asynch_compute_response(new_set);
	    if (outputLevel > SILENT_OUTPUT)
	      Cout << "\n\n";
	  }
	  else {
	    derived_compute_response(new_set);
	    const RealVector& fn_vals_x_minus_h
	      = currentResponse.function_values();
	    // no need to check fd_grad_asv since it was used for both evals
	    if (shortStep) {
		Real h12 = h*h, h22 = h2*h2;
		h1 = h*h2*(h2-h);
		for(i = 0; i < numFns; ++i)
		  new_fn_grads(j,i)
		    = (	  h22*(fn_vals_x_plus_h[i]  - (*fn_vals_x0)[i]) -
			  h12*(fn_vals_x_minus_h[i] - (*fn_vals_x0)[i]) ) / h1;
		}
	    else {
	      h1 = h - h2;
	      for (i=0; i<numFns; i++)
		new_fn_grads(j,i)
		  = (fn_vals_x_plus_h[i] - fn_vals_x_minus_h[i])/h1;
	      }
	  }
	  ++map_counter;
	}
      }

      if (fd_hess_flag) {
	new_set.request_vector(fd_hess_asv);

	// if analytic grads, then 1st-order gradient differences
	// --> no interval type control (uses only forward diff of analytic
	//     grads), separate finite diff step size.
	// if numerical grads, then 2nd-order function differences
	// --> no interval type control (uses only central diffs of numerical
	//     grads from central fn diffs), separate finite diff step size.
	//     Could get some eval reuse for diagonal Hessian terms by setting
	//     fdHessSS to half of fdGradSS, but this is not hard-wired since
	//     one generally wants fdHessSS > fdGradSS (if desired, the user
	//     can set fdHessSS to fdGradSS/2 to get reuse).
	// if mixed grads, then mixed 1st/2nd-order diffs for numerical Hessians

	if (fd_hess_by_fn_flag) {
	  if (centralHess) {
	    RealVector fn_vals_x_plus_2h, fn_vals_x_minus_2h;

	    // Compute the 2nd-order Hessian offset for the ith variable.
	    // Enforce a minimum delta of fdhss*.01
	    Real fdhbfss = (fdHessByFnSS.length() == num_deriv_vars)
	                 ? fdHessByFnSS[xj_index] : fdHessByFnSS[0];
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
	      derived_asynch_compute_response(new_set);
	      if (outputLevel > SILENT_OUTPUT)
		Cout << "\n\n";
	    }
	    else {
	      derived_compute_response(new_set);
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
	      derived_asynch_compute_response(new_set);
	      if (outputLevel > SILENT_OUTPUT)
		Cout << "\n\n";
	    }
	    else {
	      derived_compute_response(new_set);
	      fn_vals_x_minus_2h = currentResponse.function_values();
	    }

	    map_counter += 2;
	    if (!asynch_flag) {
	      for (i=0; i<numFns; i++)
		// prevent error in differencing vals present in fn_vals_x0 but
		// not in fn_vals_x_(plus/minus)_2h due to map/fd_hess asv diffs
		if (fd_hess_asv[i] & 1)
		  new_fn_hessians[i](j,j) = (fn_vals_x_plus_2h[i]
                    - 2.*(*fn_vals_x0)[i] +  fn_vals_x_minus_2h[i])/(4.*h*h);
	    }

	    // evaluate off-diagonal terms

	    for (k=j+1; k<num_deriv_vars; k++) {
	      size_t xk_index = find_index(var_ids, original_dvv[k]);
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
		derived_asynch_compute_response(new_set);
		if (outputLevel > SILENT_OUTPUT)
		  Cout << "\n\n";
	      }
	      else {
		derived_compute_response(new_set);
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
		derived_asynch_compute_response(new_set);
		if (outputLevel > SILENT_OUTPUT)
		  Cout << "\n\n";
	      }
	      else {
		derived_compute_response(new_set);
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
		derived_asynch_compute_response(new_set);
		if (outputLevel > SILENT_OUTPUT)
		  Cout << "\n\n";
	      }
	      else {
		derived_compute_response(new_set);
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
		derived_asynch_compute_response(new_set);
		if (outputLevel > SILENT_OUTPUT)
		  Cout << "\n\n";
	      }
	      else {
		derived_compute_response(new_set);
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
	    Real fdhbfss = (fdHessByFnSS.length() == num_deriv_vars)
	                 ? fdHessByFnSS[xj_index] : fdHessByFnSS[0];
	    Real h1 = FDstep1(x0_j, lb_j, ub_j, 2. * fdhbfss *
			      std::max(std::fabs(x0_j), .01));
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
	      derived_asynch_compute_response(new_set);
	      if (outputLevel > SILENT_OUTPUT)
		Cout << "\n\n";
	    }
	    else {
	      derived_compute_response(new_set);
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
	      derived_asynch_compute_response(new_set);
	      if (outputLevel > SILENT_OUTPUT)
		Cout << "\n\n";
	    }
	    else {
	      derived_compute_response(new_set);
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
		      = (fn_vals_x1[i] - 2.*(*fn_vals_x0)[i] + fn_vals_x2[i])
		      / denom;
	      }
	      else {
		hdiff = h1 - h2;
		denom = 0.5*h1*h2*hdiff;
		for (i = 0; i < numFns; i++)
		  if (fd_hess_asv[i] & 1)
		    new_fn_hessians[i](j,j)
		      = (h2*fn_vals_x1[i] + hdiff*(*fn_vals_x0)[i] -
			 h1*fn_vals_x2[i])/denom;
	      }
	    }

	    // evaluate off-diagonal terms

	    for (k = 0; k < j; k++) {
	      size_t xk_index = find_index(var_ids, original_dvv[k]);

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
		derived_asynch_compute_response(new_set);
		if (outputLevel > SILENT_OUTPUT)
		  Cout << "\n\n";
	      }
	      else {
		derived_compute_response(new_set);
		fn_vals_x12 = currentResponse.function_values();
		denom = h1*h2;
		f2 = &fx[k];
		for (i=0; i<numFns; ++i)
		  new_fn_hessians[i](j,k) = (fn_vals_x12[i] - fn_vals_x1[i] -
		    (*f2)[i] + (*fn_vals_x0)[i]) / denom;
	      }

	      ++map_counter;
	      x[xk_index] = x0[xk_index];
	    }
	  }
	}

	if (fd_hess_by_grad_flag) {

	  // Compute the 1st-order Hessian offset for the ith variable.
	  // Enforce a minimum delta of fdhss*.01
	  Real fdhbgss = (fdHessByGradSS.length() == num_deriv_vars)
	               ? fdHessByGradSS[xj_index] : fdHessByGradSS[0];
	  Real h = FDstep1(x0_j, lb_j, ub_j, fdhbgss *
			   std::max(std::fabs(x0_j), .01));
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
	    derived_asynch_compute_response(new_set);
	    if (outputLevel > SILENT_OUTPUT)
	      Cout << "\n\n";
	  }
	  else {
	    derived_compute_response(new_set);
	    const RealMatrix& fn_grads_x_plus_h
	      = currentResponse.function_gradients();
	    const RealMatrix& fn_grads_x0
	      = initial_map_response.function_gradients();
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
    function is used both by synchronous compute_response() for the
    case of asynchronous estimate_derivatives() and by synchronize()
    for the case where one or more asynch_compute_response() calls has
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
  const RealVector *fx0;
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
    initial_map_response = fd_resp_cit->second; ++fd_resp_cit;
  }
  else if (db_capture) {
    initial_map_response = dbResponseList.front(); dbResponseList.pop_front();
  }
  else { // construct an empty initial_map_response
    initial_map_response = currentResponse.copy();
    ActiveSet initial_map_set = currentResponse.active_set(); // copy
    initial_map_set.request_values(0);
    initial_map_response.active_set(initial_map_set);
    initial_map_response.reset_inactive();
  }

  if (fd_hess_flag && fd_hess_by_fn_flag && !centralHess) {
    dx.resize(num_deriv_vars);
    fx.resize(num_deriv_vars);
    fx0 = &initial_map_response.function_values();
  }

  // Postprocess the finite difference responses
  if (fd_grad_flag || fd_hess_flag) {
    SizetMultiArray var_ids;
    if (original_dvv == currentVariables.continuous_variable_ids()) {
      var_ids.resize(boost::extents[cv()]);
      var_ids = currentVariables.continuous_variable_ids();
    }
    else if (original_dvv ==
	     currentVariables.inactive_continuous_variable_ids()) {
      var_ids.resize(boost::extents[icv()]);
      var_ids = currentVariables.inactive_continuous_variable_ids();
    }
    else { // general derivatives
      var_ids.resize(boost::extents[acv()]);
      var_ids = currentVariables.all_continuous_variable_ids();
    }
    const RealVector& fn_vals_x0 = initial_map_response.function_values();
    for (j=0; j<num_deriv_vars; j++) {
      size_t xj_index = find_index(var_ids, original_dvv[j]);

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
	    const RealVector& fn_vals_x0
	      = initial_map_response.function_values();
	    for (i=0; i<numFns; i++)
	      // prevent erroneous difference of vals present in fn_vals_x0 but
	      // not in fn_vals_x_plus_h due to map_asv & fd_grad_asv diffs
	      if (fd_grad_asv[i])
		new_fn_grads(j,i)
		  = (fn_vals_x_plus_h[i] - fn_vals_x0[i])/h;
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
	    const RealVector& fn_vals_x0
	      = initial_map_response.function_values();
	    for (i=0; i<numFns; i++)
	      // prevent erroneous difference of vals present in fn_vals_x0 but
	      // not in fn_vals_x_(plus/minus)_2h due to map/fd_hess asv diffs
	      if (fd_hess_asv[i] & 1)
		new_fn_hessians[i](j,j) = (fn_vals_x_plus_2h[i]
		  - 2.*fn_vals_x0[i] + fn_vals_x_minus_2h[i])/(4.*h*h);

	    // off-diagonal terms

	    for (k=j+1; k<num_deriv_vars; k++) {
	      size_t xk_index = find_index(var_ids, original_dvv[k]);
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
		  = (fn_vals_x_plus_h_plus_h[i] - fn_vals_x_plus_h_minus_h[i]
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
		    = ((*fx1)[i] - 2.*(*fx0)[i] + (*fx2)[i]) / denom;
	    }
	    else {
	      hdiff = h1 - h2;
	      denom = 0.5*h1*h2*hdiff;
	      for (i = 0; i < numFns; i++)
		if (fd_hess_asv[i] & 1)
		  new_fn_hessians[i](j,j)
		    = (h2*(*fx1)[i] + hdiff*(*fx0)[i] - h1*(*fx2)[i])/denom;
	    }

	    // off-diagonal terms

	    for(k = 0; k < j; ++k) {
	      size_t xk_index = find_index(var_ids, original_dvv[k]);
	      h2 = dx[k];
	      denom = h1*h2;
	      fx2 = fx[k];
	      fx12 = &fd_resp_cit->second.function_values();
	      ++fd_resp_cit;
	      for (i = 0; i < numFns; i++)
		new_fn_hessians[i](j,k) =
		  ((*fx12)[i] - (*fx1)[i] - (*fx2)[i] + (*fx0)[i]) / denom;
	    }
	  }
	}
	if (fd_hess_by_grad_flag) { // 1st-order gradient differences
	  Real h = deltaList.front(); deltaList.pop_front(); // 1st in, 1st out

	  const RealMatrix& fn_grads_x_plus_h
	    = fd_resp_cit->second.function_gradients();
	  ++fd_resp_cit;
	  const RealMatrix& fn_grads_x0
	    = initial_map_response.function_gradients();
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
      RealMatrix partial_fn_grads = initial_map_response.function_gradients();
      partial_fn_grads.reshape(new_fn_grads.numRows(), new_fn_grads.numCols());
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
       (hessType == "quasi" || (hessType == "mixed" && !hessIdQuasi.empty())) )
    update_quasi_hessians(vars, new_response, original_set);

  // overlay Hessian data as needed
  if (initial_map || hessType == "mixed") {
    // merge initial map Hessians, new_fn_hessians, and quasiHessians
    if (fd_hess_flag || quasi_hess_flag) {
      RealSymMatrixArray partial_fn_hessians;
      if (initial_map)
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
  const Real min_norm            = 1.e-25;

  // if necessary, initialize quasi-Hessians before populating
  if ( numQuasiUpdates.empty() ) {
    if (outputLevel == DEBUG_OUTPUT)
      Cout << "Reshaping quasiHessians in modelType = " << modelType
	   << std::endl;
    xPrev.resize(numFns);
    fnGradsPrev.reshape(numDerivVars, numFns);
    quasiHessians.resize(numFns);
    for (size_t i=0; i<numFns; i++) {
      if ( hessType == "quasi" ||
	   ( hessType == "mixed" && contains(hessIdQuasi, i+1) ) ) {
	quasiHessians[i].reshape(numDerivVars);
	quasiHessians[i] = 0.;
	// leave as zero matrix so that any early use of quasi-Hessian has
	// no effect.  The initial scaling updates to nonzero values at the
	// appropriate time.
	//for (size_t j=0; j<numDerivVars; j++)
	//  quasiHessians[i][j][j] = 1.; // initialize to identity
      }
    }
    numQuasiUpdates.resize(numFns);
    numQuasiUpdates.assign(numQuasiUpdates.size(), 0);
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
	if (norm_s > min_norm) {

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
	    if (norm_y > min_norm)
	      scaling2 = (std::sqrt(std::fabs(scaling1)) > min_norm)
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
      if ( numQuasiUpdates[i] == 0 || norm_s > min_norm ) {
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
bool Model::manage_asv(const ShortArray& asv_in, ShortArray& map_asv_out,
		       ShortArray& fd_grad_asv_out, ShortArray& fd_hess_asv_out,
		       ShortArray& quasi_hess_asv_out)
{
  // *_asv_out[i] have all been initialized to zero

  // For HierarchSurr and Recast models with no scaling (which contain no
  // interface object, only subordinate models), pass the ActiveSet through to
  // the sub-models in one piece and do not break it apart here. This preserves
  // sub-model parallelism.
  if (!supportsEstimDerivs)
    return false;

  bool use_est_deriv = false;
  size_t i, asv_len = asv_in.size();
  for (i=0; i<asv_len; ++i) {

    // Function value requests
    if (asv_in[i] & 1)
      map_asv_out[i] = 1;

    // Function gradient requests
    if (asv_in[i] & 2) {
      if ( gradType == "analytic" ||
	   ( gradType == "mixed" && contains(gradIdAnalytic, i+1) ) )
	map_asv_out[i] |= 2; // activate 2nd bit
      else if ( methodSrc == "dakota" && ( gradType == "numerical" ||
		( gradType == "mixed" && contains(gradIdNumerical, i+1) ) ) ) {
	fd_grad_asv_out[i] = 1;
	if (intervalType == "forward")
	  map_asv_out[i] |= 1; // activate 1st bit
	use_est_deriv = true;
      }
      else { // could happen if an iterator requiring gradients is selected
	     // with no_gradients or unsupported vendor numerical gradients
	     // and lacks a separate error check.
	Cerr << "Error: unsupported asv gradient request in Model::manage_asv."
	     << std::endl;
	abort_handler(-1);
      }
      if ( surrogate_response_mode() != AUTO_CORRECTED_SURROGATE &&
	   ( hessType == "quasi" ||
	     ( hessType == "mixed" && contains(hessIdQuasi, i+1) ) ) )
	use_est_deriv = true;
    }

    // Function Hessian requests
    if (asv_in[i] & 4) {
      if ( hessType == "analytic" ||
	   ( hessType == "mixed" && contains(hessIdAnalytic, i+1) ) )
	map_asv_out[i] |= 4; // activate 3rd bit
      else if ( hessType == "numerical" ||
		( hessType == "mixed" && contains(hessIdNumerical, i+1) ) ) {
	if ( gradType == "analytic" ||
	     ( gradType == "mixed" && contains(gradIdAnalytic, i+1) ) ) {
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
      else if ( hessType == "quasi" ||
		(hessType == "mixed" && contains(hessIdQuasi, i+1))) {
	quasi_hess_asv_out[i] = 2; // value not currently used
	use_est_deriv = true; // update_response needed even if no secant update
      }
      else { // could happen if an iterator requiring Hessians is selected
	     // with no_hessians and it lacks a separate error check.
	Cerr << "Error: unsupported asv Hessian request in Model::manage_asv."
	     << std::endl;
	abort_handler(-1);
      }
    }
  }
  return use_est_deriv;
}


void Model::derived_compute_response(const ActiveSet& set)
{
  if (modelRep) // should not occur: protected fn only used by the letter
    modelRep->derived_compute_response(set); // envelope fwd to letter
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual derived_compute_"
         << "response() function.\nNo default defined at base class."
	 << std::endl;
    abort_handler(-1);
  }
}


void Model::derived_asynch_compute_response(const ActiveSet& set)
{
  if (modelRep) // should not occur: protected fn only used by the letter
    modelRep->derived_asynch_compute_response(set); // envelope fwd to letter
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual derived_asynch_"
         << "compute_response() function.\nNo default defined at base class."
         << std::endl;
    abort_handler(-1);
  }
}


const IntResponseMap& Model::derived_synchronize()
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual derived_synchronize"
         << "() function.\n       derived_synchronize is not available for this"
	 << " Model." << std::endl;
    abort_handler(-1);
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
    abort_handler(-1);
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
  // else: default implementation (single models) is no-op.
}


/** used only for instantiate-on-the-fly model recursions (all RecastModel
    instantiations and alternate DataFitSurrModel instantiations).  Single,
    Hierarchical, and Nested Models do not redefine the function since they
    do not support instantiate-on-the-fly.  This means that the recursion
    will stop as soon as it encounters a Model that was instantiated normally,
    which is appropriate since ProblemDescDB-constructed Models use top-down
    information flow and do not require bottom-up updating. */
void Model::update_from_subordinate_model(bool recurse_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->update_from_subordinate_model(recurse_flag);
  // else default if no redefinition is no-op
}


/** return by reference requires use of dummy objects, but is
    important to allow use of assign_rep() since this operation must
    be performed on the original envelope object. */
Interface& Model::interface()
{
  if (modelRep)
    return modelRep->interface(); // envelope fwd to letter
  else // letter lacking redefinition of virtual fn.
    return dummy_interface; // return null/empty envelope
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
  else // default does not support recursion (SingleModel, NestedModel)
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
    if ( (gradType=="numerical" || gradType=="mixed") && methodSrc == "dakota" )
      deriv_conc += (intervalType == "central") ? 2*numDerivVars : numDerivVars;
    if ( hessType == "numerical" ||
	 ( hessType == "mixed" && !hessIdNumerical.empty())) {
      if (gradType == "analytic")
	deriv_conc += numDerivVars;
      else if (gradType == "numerical")
	deriv_conc += 2*numDerivVars*numDerivVars;
      else if (gradType == "mixed") {
	bool first_order = false, second_order = false;
	if (hessType == "mixed") { // mixed Hessians with mixed gradients
	  for (ILCIter cit=hessIdNumerical.begin();
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


void Model::build_approximation()
{
  if (modelRep) // envelope fwd to letter
    modelRep->build_approximation();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual build_approximation"
         << "() function.\nThis model does not support approximation "
	 << "construction." << std::endl;
    abort_handler(-1);
  }
}


bool Model::
build_approximation(const Variables& vars, const IntResponsePair& response_pr)
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual build_approximation"
         << "(Variables, IntResponsePair) function.\nThis model does not "
	 << "support constrained approximation construction." << std::endl;
    abort_handler(-1);
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
    abort_handler(-1);
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
    abort_handler(-1);
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
            "support approximation updating." << std::endl;
    abort_handler(-1);
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
    abort_handler(-1);
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
    abort_handler(-1);
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
            "support approximation appending." << std::endl;
    abort_handler(-1);
  }
}


void Model::pop_approximation(bool save_surr_data)
{
  if (modelRep) // envelope fwd to letter
    modelRep->pop_approximation(save_surr_data);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual pop_approximation"
	 << "(bool) function.\nThis model does not support approximation data "
	 << "removal." << std::endl;
    abort_handler(-1);
  }
}


void Model::restore_approximation()
{
  if (modelRep) // envelope fwd to letter
    modelRep->restore_approximation();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual restore_"
	 << "approximation() function.\nThis model does not support "
	 << "approximation restoration." << std::endl;
    abort_handler(-1);
  }
}


bool Model::restore_available()
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual restore_"
	 << "approximation(bool) function.\nThis model does not support "
	 << "approximation restoration." << std::endl;
    abort_handler(-1);
  }
  return modelRep->restore_available();
}


void Model::finalize_approximation()
{
  if (modelRep) // envelope fwd to letter
    modelRep->finalize_approximation();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual finalize_"
	 << "approximation() function.\nThis model does not support "
	 << "approximation finalization." << std::endl;
    abort_handler(-1);
  }
}


void Model::store_approximation()
{
  if (modelRep) // envelope fwd to letter
    modelRep->store_approximation();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual store_approximation"
	 << "() function.\nThis model does not support approximation storage."
	 << std::endl;
    abort_handler(-1);
  }
}


void Model::combine_approximation(short corr_type)
{
  if (modelRep) // envelope fwd to letter
    modelRep->combine_approximation(corr_type);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual combine_"
	 << "approximation() function.\nThis model does not support "
	 << "approximation combination." << std::endl;
    abort_handler(-1);
  }
}


/*
const VariablesArray Model::build_variables() const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->build_variables();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual build_variables()"
         << "\nThis model does not support build variables retrieval."
	 << std::endl;
    abort_handler(-1);
  }
}


const ResponseArray Model::build_responses() const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->build_responses();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual build_responses()"
         << "\nThis model does not support build responses retrieval."
	 << std::endl;
    abort_handler(-1);
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


std::vector<Approximation>& Model::approximations()
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual approximations() "
         << "function.\nThis model does not support approximations."
	 << std::endl;
    abort_handler(-1);
  }

  // envelope fwd to letter
  return modelRep->approximations();
}


const RealVectorArray& Model::approximation_coefficients()
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual approximation_"
         << "coefficients() function.\nThis model does not support "
         << "approximations." << std::endl;
    abort_handler(-1);
  }

  // envelope fwd to letter
  return modelRep->approximation_coefficients();
}


void Model::approximation_coefficients(const RealVectorArray& approx_coeffs)
{
  if (modelRep) // envelope fwd to letter
    modelRep->approximation_coefficients(approx_coeffs);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual approximation_"
         << "coefficients() function.\n       This model does not support "
         << "approximations." << std::endl;
    abort_handler(-1);
  }
}


const RealVector& Model::approximation_variances(const RealVector& c_vars)
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual approximation_"
         << "variances() function.\nThis model does not support "
         << "approximations." << std::endl;
    abort_handler(-1);
  }

  // envelope fwd to letter
  return modelRep->approximation_variances(c_vars);
}


const Pecos::SurrogateData& Model::approximation_data(size_t index)
{
  if (!modelRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual approximation_data()"
         << " function.\nThis model does not support approximations."
	 << std::endl;
    abort_handler(-1);
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
    abort_handler(-1);
  }

  // envelope fwd to letter
  return modelRep->discrepancy_correction();
}


void Model::component_parallel_mode(short mode)
{
  if (modelRep) // envelope fwd to letter
    modelRep->component_parallel_mode(mode);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual "
	 << "component_parallel_mode() function.\n." << std::endl;
    abort_handler(-1);
  }
}


/** SingleModels and HierarchSurrModels redefine this virtual function.  A
    default value of "synchronous" prevents asynch local operations for:
\li NestedModels: a subIterator can support message passing parallelism,
    but not asynch local.
\li DataFitSurrModels: while asynch evals on approximations will work due
    to some added bookkeeping, avoiding them is preferable. */
String Model::local_eval_synchronization()
{
  if (modelRep) // should not occur: protected fn only used by the letter
    return modelRep->local_eval_synchronization(); // envelope fwd to letter
  else // letter lacking redefinition of virtual fn.
    return String("synchronous"); // default for Nested/DataFitSurr models
}


/** SingleModels and HierarchSurrModels redefine this virtual function. */
int Model::local_eval_concurrency()
{
  if (modelRep) // should not occur: protected fn only used by the letter
    return modelRep->local_eval_concurrency(); // envelope fwd to letter
  else // letter lacking redefinition of virtual fn.
    return 0; // default for Nested/DataFitSurr models
}


void Model::serve()
{
  if (modelRep) // envelope fwd to letter
    modelRep->serve();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual serve() function.\n"
         << "This model does not support server operations." << std::endl;
    abort_handler(-1);
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
    abort_handler(-1);
  }
}


/** The init_communicators() and derived_init_communicators() functions are
    stuctured to avoid performing the messageLengths estimation more than once.
    init_communicators() (not virtual) performs the estimation and then
    forwards the results to derived_init_communicators (virtual) which uses
    the data in different contexts. */
void Model::init_communicators(int max_iterator_concurrency, bool recurse_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->init_communicators(max_iterator_concurrency, recurse_flag);
  else { // not a virtual function: base class definition for all letters

    // matches bcast in Model::serve_configurations() called from
    // Strategy::init_iterator().  bcastFlag assures that, when Model recursions
    // are present in Iterator instantiations, only the matching Model instance
    // participates in this collective communication.
    if (initCommsBcastFlag &&
	modelPCIter->si_parallel_level().server_communicator_rank() == 0) {
      int max_conc = max_iterator_concurrency; // non-const copy
      parallelLib.bcast_i(max_conc);
    }

    // estimate messageLengths
    if (messageLengths.empty())
      estimate_message_lengths();

    // since the same Model instance may be reused in multiple contexts
    // (involving multiple calls to init_communicators()), multiple parallel
    // configurations must be supported.  This is managed using a map<> with
    // concurrency level as the lookup key.  Creation of a new parallel
    // configuration is avoided if an equivalent one already exists.
    std::map<int, ParConfigLIter>::iterator map_iter
      = modelPCIterMap.find(max_iterator_concurrency);

    // NOTE: modelPCIter update belongs in set_communicators().  However, also
    // updating it here allows passing of analysisComm into a parallel plugin
    // interface constructor (see main.C).
    if (map_iter == modelPCIterMap.end()) { // this config does not yet exist

      // increment the PC every time (the first PC instance is wasted), such
      // that each Model points to its corresponding configuration, even if
      // incomplete (for a Model only contained subModels, no increment for
      // incomplete results in a shared complete configuration between top
      // level Model and subModel, and erroneous settings in top level
      // set_communicators()).
      //if ( parallelLib.num_parallel_configurations() > 1 ||
      //     parallelLib.parallel_configuration_is_complete() )
      parallelLib.increment_parallel_configuration();

      // Setting modelPCIter here is insufficient; it must be set at run time
      // (within set_communicators()) according to the iterator context.
      modelPCIterMap[max_iterator_concurrency] = modelPCIter
	= parallelLib.parallel_configuration_iterator();
      derived_init_communicators(max_iterator_concurrency, recurse_flag);
    }
    else
      modelPCIter = map_iter->second;
      //  Parallel configuration already exists within the Model for this
      //  concurrency level.  Configurations must also exist for any sub-models
      //  -> no call to derived_init_communicators() needed.
  }
}


void Model::stop_configurations()
{
  if (modelRep) // envelope fwd to letter
    modelRep->stop_configurations();
  else { // not a virtual function: base class definition for all letters
    int term_code = 0;
    parallelLib.bcast_i(term_code);
  }
}


int Model::serve_configurations()
{
  if (modelRep) // envelope fwd to letter
    return modelRep->serve_configurations();
  else { // not a virtual function: base class definition for all letters
    int max_concurrency = 1, last_concurrency = 1;
    while (max_concurrency) {
      parallelLib.bcast_i(max_concurrency);
      if (max_concurrency) {
	init_communicators(max_concurrency);
	last_concurrency = max_concurrency;
      }
    }
    return last_concurrency;
  }
}


void Model::set_communicators(int max_iterator_concurrency, bool recurse_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->set_communicators(max_iterator_concurrency, recurse_flag);
  else { // not a virtual function: base class definition for all letters

    std::map<int, ParConfigLIter>::iterator map_iter
      = modelPCIterMap.find(max_iterator_concurrency);
    if (map_iter == modelPCIterMap.end()) { // this config does not exist
      Cerr << "Error: failure in parallel configuration lookup in "
           << "Model::set_communicators()." << std::endl;
      abort_handler(-1);
    }
    else
      modelPCIter = map_iter->second;

    derived_set_communicators(max_iterator_concurrency, recurse_flag);

    // moved the following from init_communicators() since these are currently
    // needed at run time and not construct time (if set only at construct time,
    // run time values would reflect last of several init calls)

    // Set asynchEvalFlag for either evaluation message passing or asynch local
    // evaluations (or both).  Note that asynch local analysis concurrency by
    // itself does not trigger an asynchronous model, since this concurrency
    // can be handled within a synchronous model evaluation.
    // In the case of Surrogate or Nested models, this sets the asynch flag for
    // the top level iterator & model; the asynch flag for the sub-iterator &
    // sub-model must be set by calling init_communicators on the sub-model
    // within derived_init_communicators.
    parallelLib.parallel_configuration_iterator(modelPCIter); // reset
    if (parallelLib.ie_parallel_level_defined()) {
      const ParallelLevel& ie_pl
	= parallelLib.parallel_configuration().ie_parallel_level();

      // Note: local_eval_synchronization() handles case of eval concurrency==1
      bool asynch_local_eval
	= (local_eval_synchronization() == "asynchronous") ? true : false;
      if ( ie_pl.message_pass() || asynch_local_eval )
	asynchEvalFlag = true;

      // Set evaluationCapacity for use by iterators (e.g., COLINY).
      int local_eval_conc = local_eval_concurrency();
      if (parallelLib.world_size() > 1) { // message passing mode
	evaluationCapacity = ie_pl.num_servers();
	if (local_eval_conc) // hybrid mode: capacity augmented
	  evaluationCapacity *= local_eval_conc;
      }
      else if (asynch_local_eval) // asynch local mode: capacity limited
	evaluationCapacity = (local_eval_conc) ? local_eval_conc
	                                       : max_iterator_concurrency;
    }
  }
}


void Model::free_communicators(int max_iterator_concurrency, bool recurse_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->free_communicators(max_iterator_concurrency, recurse_flag);
  else { // not a virtual function: base class definition for all letters

    // Note: deallocations do not utilize reference counting -> the _first_
    // call to free a particular configuration deallocates it and all
    // subsequent calls are ignored (to prevent multiple deallocations).
    std::map<int, ParConfigLIter>::iterator map_iter
      = modelPCIterMap.find(max_iterator_concurrency);
    if (map_iter != modelPCIterMap.end()) { // this config still exists
      modelPCIter = map_iter->second;
      derived_free_communicators(max_iterator_concurrency, recurse_flag);
      modelPCIterMap.erase(max_iterator_concurrency);
    }
  }
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
    messageLengths.resize(4);
    messageLengths.assign(messageLengths.size(), 0);

    if (parallelLib.world_size() > 1) {
      MPIPackBuffer buff;
      buff << currentVariables;
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
      ParamResponsePair current_pair(currentVariables, interface_id(),
				     new_response);
      buff << current_pair;
      messageLengths[3] = buff.size(); // length of message containing a PRPair
#ifdef MPI_DEBUG
      Cout << "Message Lengths:\n" << messageLengths << std::endl;
#endif // MPI_DEBUG
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
    if ( local_eval_synchronization() == "asynchronous" )
      asynchEvalFlag = true;
  }
}


void Model::
derived_init_communicators(int max_iterator_concurrency, bool recurse_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->derived_init_communicators(max_iterator_concurrency,
					 recurse_flag);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual derived_init_"
	 << "communicators() function.\n       This model does not support "
	 << "communicator operations." << std::endl;
    abort_handler(-1);
  }
}


void Model::derived_init_serial()
{
  if (modelRep) // envelope fwd to letter
    modelRep->derived_init_serial();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual derived_init_serial"
         << "() function.\nNo default defined at base class." << std::endl;
    abort_handler(-1);
  }
}


void Model::
derived_set_communicators(int max_iterator_concurrency, bool recurse_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->derived_set_communicators(max_iterator_concurrency, recurse_flag);
  // else default is nothing additional beyond set_communicators()
}


void Model::
derived_free_communicators(int max_iterator_concurrency, bool recurse_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->derived_free_communicators(max_iterator_concurrency,
					 recurse_flag);
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual derived_free_"
	 << "communicators() function.\nThis model does not support "
	 << "communicator operations." << std::endl;
    abort_handler(-1);
  }
}


void Model::inactive_view(short view, bool recurse_flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->inactive_view(view, recurse_flag);
  else { // default does not support recursion (SingleModel, NestedModel)
    currentVariables.inactive_view(view);
    userDefinedConstraints.inactive_view(view);
  }
}


int Model::evaluation_id() const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->evaluation_id();
  else // letter lacking redefinition of virtual fn.
    return modelEvalCntr; // default
}


/** Only Models including ApplicationInterfaces support an evaluation cache:
    surrogate, nested, and recast mappings are not stored in the cache. 
    Possible exceptions: HierarchSurrModel, NestedModel::optionalInterface. */
bool Model::evaluation_cache() const
{
  if (modelRep) // envelope fwd to letter
    return modelRep->evaluation_cache();
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
    abort_handler(-1);
  }
}


void Model::fine_grained_evaluation_counters()
{
  if (modelRep) // envelope fwd to letter
    modelRep->fine_grained_evaluation_counters();
  else { // letter lacking redefinition of virtual fn.
    Cerr << "Error: Letter lacking redefinition of virtual fine_grained_"
	 << "evaluation_counters() function.\n" << std::endl;
    abort_handler(-1);
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
    abort_handler(-1);
  }
}

} // namespace Dakota
