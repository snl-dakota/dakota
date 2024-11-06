/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "SubspaceModel.hpp"
#include "dakota_linear_algebra.hpp"
#include "ParallelLibrary.hpp"
#include "MarginalsCorrDistribution.hpp"

namespace Dakota {

/// initialization of static needed by RecastModel callbacks
SubspaceModel* SubspaceModel::smInstance(NULL);


SubspaceModel::SubspaceModel(ProblemDescDB& problem_db, const Model& sub_model):
  RecastModel(problem_db, sub_model), randomSeed(24620),
  numFullspaceVars(ModelUtils::cv(*pSubModel)),
  // default is 0 for no user override, but only used for AdaptedBasis at
  // this time (ActiveSubspace overwrites with basis truncation procedure):
  reducedRank(problem_db.get_int("model.subspace.dimension")),
  offlineEvalConcurrency(1), onlineEvalConcurrency(1)
{
  componentParallelMode = CONFIG_PHASE;
}


SubspaceModel::
SubspaceModel(const Model& sub_model, unsigned int dimension,
	      short output_level) :
  RecastModel(sub_model), numFullspaceVars(ModelUtils::cv(sub_model)),
  reducedRank(dimension), offlineEvalConcurrency(1), onlineEvalConcurrency(1)
{
  outputLevel = output_level;
  componentParallelMode = CONFIG_PHASE;

  // synchronize output level and grad/Hess settings with subModel
  initialize_data_from_submodel(); // Note: currentVariables not defined
}


SubspaceModel::~SubspaceModel()
{ }


void SubspaceModel::validate_inputs()
{
  bool error_flag = false;

  // validate variables specification
  // BMA TODO: allow other variable types
  if (ModelUtils::div(*pSubModel) > 0 || ModelUtils::dsv(*pSubModel) > 0 || ModelUtils::drv(*pSubModel) > 0) {
    error_flag = true;
    Cerr << "\nError (subspace model): only normal uncertain variables are "
         << "supported;\n                        remove other variable "
         << "specifications.\n" << std::endl;
  }

  if (error_flag)
    abort_handler(-1);
}


/** May eventually take on init_comms and related operations.  Also
    may want ide of build/update like DataFitSurrModel, eventually. */
bool SubspaceModel::initialize_mapping(ParLevLIter pl_iter)
{
  bool sub_model_resize = RecastModel::initialize_mapping(pl_iter);

  // init-time setting of miPLIndex for use in component_parallel_mode()
  miPLIndex = modelPCIter->mi_parallel_level_index(pl_iter);
  // Set mode OFFLINE_PHASE
  component_parallel_mode(OFFLINE_PHASE);

  // runtime operations to identify the subspace model (if not later
  // returning to update the subspace)
  compute_subspace();
  initialize_subspace();

  // Kill servers and return ranks [1,n-1] to serve_init_mapping()
  component_parallel_mode(CONFIG_PHASE);

  // return true if size of variables has changed
  return (reducedRank != numFullspaceVars || // Active SS is reduced rank
	  sub_model_resize); // Active SS is full rank but subModel resized
}


void SubspaceModel::initialize_subspace()
{
  // convert subModel normal distributions to the reduced space
  // TODO: generalize to other distribution types
  uncertain_vars_to_subspace();

  // update with subspace constraints
  update_linear_constraints();
  // set new subspace variable labels
  update_var_labels();

  // Perform numerical derivatives in subspace:
  supportsEstimDerivs = true;
}


/** Initialize the recast model based on the reduced space, with no
    response function mapping (for now).  TODO: use a surrogate model
    over the inactive dimension. */
void SubspaceModel::
initialize_base_recast(
  void (*variables_map)    (const Variables& recast_vars,
			    Variables& sub_model_vars),
  void (*set_map)          (const Variables& recast_vars,
			    const ActiveSet& recast_set,
			    ActiveSet& sub_model_set),
  void (*primary_resp_map) (const Variables& sub_model_vars,
			    const Variables& recast_vars,
			    const Response& sub_model_response,
			    Response& recast_response) )
{
  // For now, we assume the subspace is over all functions, without
  // distinguishing primary from secondary

  // ------------------
  // Variables mapping: Recast maps subspace vars to original fullspace vars
  // ------------------

  // We assume the mapping is for all active variables, but only
  // continuous for the active subspace
  size_t submodel_cv = ModelUtils::cv(*pSubModel);
  size_t submodel_dv = ModelUtils::div(*pSubModel) + ModelUtils::dsv(*pSubModel) + ModelUtils::drv(*pSubModel);
  size_t submodel_vars = submodel_cv + submodel_dv;
  size_t recast_cv   = reducedRank;
  size_t recast_vars = recast_cv + submodel_dv;
  // In general, each submodel continuous variable depends on all of
  // the recast (reduced) variables; others are one-to-one.
  Sizet2DArray vars_map_indices(submodel_vars);
  for (size_t i=0; i<submodel_cv; ++i) {
    vars_map_indices[i].resize(recast_cv);
    for (size_t j=0; j<recast_cv; ++j)
      vars_map_indices[i][j] = j;
  }
  for (size_t i=0; i<submodel_dv; ++i) {
    vars_map_indices[submodel_cv + i].resize(1);
    vars_map_indices[submodel_cv + i][0] = recast_cv + i;
  }
  // Variables map is linear
  bool nonlinear_vars_mapping = false;
  SizetArray vars_comps_total = resize_variable_totals();
  BitArray all_relax_di, all_relax_dr; // default: empty; no discrete relaxation

  // -----------------
  // Response mapping: one to one
  // -----------------

  // Primary and secondary mapping are one-to-one (NULL callbacks)
  // TODO: can RecastModel tolerate empty indices when no map is present?
  size_t num_primary   = pSubModel->num_primary_fns(),
         num_secondary = pSubModel->num_secondary_fns(),
         recast_secondary_offset = ModelUtils::num_nonlinear_ineq_constraints(*pSubModel);
  BoolDequeArray nonlinear_resp_mapping(numFns, BoolDeque(numFns, false));
  Sizet2DArray primary_resp_map_indices(num_primary),
             secondary_resp_map_indices(num_secondary);
  for (size_t i=0; i<num_primary; i++) {
    primary_resp_map_indices[i].resize(1);
    primary_resp_map_indices[i][0] = i;
  }
  for (size_t i=0; i<num_secondary; i++) {
    secondary_resp_map_indices[i].resize(1);
    secondary_resp_map_indices[i][0] = num_primary + i;
  }

  // Initial response order for the newly built subspace model same as
  // the subModel (does not augment with gradient request)
  const Response& curr_resp = pSubModel->current_response();
  short recast_resp_order = 1; // recast resp order to be same as original resp
  if (!curr_resp.function_gradients().empty()) recast_resp_order |= 2;
  if (!curr_resp.function_hessians().empty())  recast_resp_order |= 4;

  // -----------------------------------
  // Invoke base class resizing routines
  // -----------------------------------

  bool copy_values;
  RecastModel::init_sizes(pSubModel->current_variables().view(), vars_comps_total,
			  all_relax_di, all_relax_dr, num_primary,
			  num_secondary, recast_secondary_offset,
			  recast_resp_order, copy_values);

  // suppress this initialization (see uncertain_vars_to_subspace())
  //RecastModel::init_distribution(copy_values);

  RecastModel::
    init_maps(vars_map_indices, nonlinear_vars_mapping, variables_map,
	      set_map, primary_resp_map_indices, secondary_resp_map_indices,
	      nonlinear_resp_mapping, primary_resp_map, NULL);
}


/// Create a variables components totals array with the reduced space
/// size for continuous variables
SizetArray SubspaceModel::resize_variable_totals()
{
  const SharedVariablesData& svd = pSubModel->current_variables().shared_data();
  SizetArray vc_totals = svd.components_totals(); // copy to be updated
  if (reducedRank != ModelUtils::cv(*pSubModel)) {
    short active_view = pSubModel->current_variables().view().first;
    switch (active_view) {
    case MIXED_DESIGN:               case RELAXED_DESIGN:
      // resize continuous design
      vc_totals[TOTAL_CDV] = reducedRank;   break;
    case MIXED_ALEATORY_UNCERTAIN:   case RELAXED_ALEATORY_UNCERTAIN:
      // resize continuous aleatory
      vc_totals[TOTAL_CAUV] = reducedRank;  break;
    case MIXED_EPISTEMIC_UNCERTAIN:  case RELAXED_EPISTEMIC_UNCERTAIN:
      // resize continuous epistemic
      vc_totals[TOTAL_CEUV] = reducedRank;  break;
    case MIXED_STATE:                case RELAXED_STATE:
      // resize continuous state
      vc_totals[TOTAL_CSV] = reducedRank;   break;
    case MIXED_UNCERTAIN:            case RELAXED_UNCERTAIN:
      // assumption: only one vars type is defined in original space
      // (unclear how to partition dimension reduction among types)
      if      (vc_totals[TOTAL_CAUV]) vc_totals[TOTAL_CAUV] = reducedRank;
      else if (vc_totals[TOTAL_CEUV]) vc_totals[TOTAL_CEUV] = reducedRank;
      break;
    case MIXED_ALL:                  case RELAXED_ALL:
      // assumption: only one vars type is defined in original space
      // (unclear how to partition dimension reduction among types)
      if      (vc_totals[TOTAL_CDV])  vc_totals[TOTAL_CDV]  = reducedRank;
      else if (vc_totals[TOTAL_CAUV]) vc_totals[TOTAL_CAUV] = reducedRank;
      else if (vc_totals[TOTAL_CEUV]) vc_totals[TOTAL_CEUV] = reducedRank;
      else if (vc_totals[TOTAL_CSV])  vc_totals[TOTAL_CSV]  = reducedRank;
      break;
    default:
      Cerr << "\nError (subspace model): invalid active variables view "
           << active_view << "." << std::endl;
      abort_handler(MODEL_ERROR);  break;
    }
  }
  return vc_totals;
}


/** Convert the user-specified normal random variables to the
    appropriate reduced space variables, based on the orthogonal
    transformation.

    TODO: Generalize to convert other random variable types (non-normal)

    TODO: The translation of the correlations from full to reduced
    space is likely wrong for rank correlations; should be correct for
    covariance.
*/
void SubspaceModel::uncertain_vars_to_subspace()
{
  std::shared_ptr<Pecos::MarginalsCorrDistribution> native_dist_rep =
    std::static_pointer_cast<Pecos::MarginalsCorrDistribution>
    (pSubModel->multivariate_distribution().multivar_dist_rep());
  std::shared_ptr<Pecos::MarginalsCorrDistribution> reduced_dist_rep =
    std::static_pointer_cast<Pecos::MarginalsCorrDistribution>
    (mvDist.multivar_dist_rep());
  const ShortArray& native_rv_types = native_dist_rep->random_variable_types();
  const BitArray&   active_vars     = native_dist_rep->active_variables();
  size_t i, num_rv = native_rv_types.size();
  if (numFullspaceVars != active_vars.count()) {
    Cerr << "Error: mismatch in active full-space variables in "
	 << "SubspaceModel::uncertain_vars_to_subspace()" << std::endl;
    abort_handler(MODEL_ERROR);
  }

  // -------------
  // Resize mvDist
  // -------------
  // Note: may be able to elevate this to RecastModel::init_sizes(), once more
  //       general.  Keep specifics in derived class for now.
  // assumption: replace contiguous set of active continuous variables
  // (see also resize_variable_totals())
  const SharedVariablesData& svd = pSubModel->current_variables().shared_data();
  size_t cv_diff    = numFullspaceVars - reducedRank, // active cv
    num_reduced_rv  = num_rv - cv_diff,               // all vars
    start_reduction = svd.cv_index_to_all_index(0),
    end_reduction   = start_reduction + reducedRank;
  ShortArray  reduced_rv_types(num_reduced_rv);
  BitArray reduced_active_vars(num_reduced_rv), // init to false
           reduced_active_corr(num_reduced_rv); // init to false
  for (i=0; i<start_reduction; ++i)              // same as native
    reduced_rv_types[i] = native_rv_types[i];
  for (i=start_reduction; i<end_reduction; ++i) {// reduced space of NORMAL vars
    reduced_rv_types[i] = Pecos::NORMAL;
    reduced_active_vars[i] = reduced_active_corr[i] = true;
  }
  // For now, correl_x limited to numFullspaceVars (active cv) below
  //for (i=0; i<num_dauv; ++i)                   // include dauv for active corr
  //  reduced_active_corr[end_reduction+i] = true;
  for (i=end_reduction; i<num_reduced_rv; ++i)   // same as native
    reduced_rv_types[i] = native_rv_types[i+cv_diff];
  reduced_dist_rep->initialize_types(reduced_rv_types, reduced_active_vars);
  reduced_dist_rep->active_correlations(reduced_active_corr);

  // copy the RVs for any active complement variables, so clients of
  // the subspace model will have access to their properties
  for (i=0; i<start_reduction; ++i)              // same as native
    reduced_dist_rep->random_variable(i) =
      native_dist_rep->random_variable(i);
  for (i=end_reduction; i<num_reduced_rv; ++i)   // same as native
    reduced_dist_rep->random_variable(i) =
      native_dist_rep->random_variable(i + cv_diff);
}


void SubspaceModel::update_var_labels()
{
  StringMultiArray subspace_var_labels(boost::extents[reducedRank]);
  for (int i = 0; i < reducedRank; i++) {
    subspace_var_labels[i] = "ssv_" + std::to_string(i+1);
  }

  current_variables().continuous_variable_labels(
    subspace_var_labels[boost::indices[idx_range(0, reducedRank)]]);
}


void SubspaceModel::update_linear_constraints()
{ }


void SubspaceModel::derived_evaluate(const ActiveSet& set)
{
  if (!mappingInitialized) {
    Cerr << "\nError: subspace model mapping has not been initialized."
         << std::endl;
    abort_handler(MODEL_ERROR);
  }

  component_parallel_mode(ONLINE_PHASE);
  RecastModel::derived_evaluate(set);
}


void SubspaceModel::derived_evaluate_nowait(const ActiveSet& set)
{
  if (!mappingInitialized) {
    Cerr << "\nError: subspace model mapping has not been initialized."
         << std::endl;
    abort_handler(MODEL_ERROR);
  }

  component_parallel_mode(ONLINE_PHASE);
  RecastModel::derived_evaluate_nowait(set);
}


const IntResponseMap& SubspaceModel::derived_synchronize()
{
  if (!mappingInitialized) {
    Cerr << "\nError: subspace model mapping has not been initialized."
         << std::endl;
    abort_handler(MODEL_ERROR);
  }

  component_parallel_mode(ONLINE_PHASE);
  return RecastModel::derived_synchronize();
}


const IntResponseMap& SubspaceModel::derived_synchronize_nowait()
{
  if (!mappingInitialized) {
    Cerr << "\nError: subspace model mapping has not been initialized."
         << std::endl;
    abort_handler(-1);
  }

  component_parallel_mode(ONLINE_PHASE);
  return RecastModel::derived_synchronize_nowait();
}


void SubspaceModel::component_parallel_mode(short mode)
{
  // stop_servers() if they are active, componentParallelMode = 0 indicates
  // they are inactive
  if (componentParallelMode != mode &&
      componentParallelMode != CONFIG_PHASE) {
    ParConfigLIter pc_it = pSubModel->parallel_configuration_iterator();
    size_t index = pSubModel->mi_parallel_level_index();
    if (pc_it->mi_parallel_level_defined(index) &&
        pc_it->mi_parallel_level(index).server_communicator_size() > 1) {
      pSubModel->stop_servers();
    }
  }

  // activate new serve mode (matches SubspaceModel::serve_run(pl_iter)).
  if (componentParallelMode != mode &&
      modelPCIter->mi_parallel_level_defined(miPLIndex)) {
    ParLevLIter pl_iter = modelPCIter->mi_parallel_level_iterator(miPLIndex);
    const ParallelLevel& mi_pl = modelPCIter->mi_parallel_level(miPLIndex);
    if (mi_pl.server_communicator_size() > 1) {
      if (mode == OFFLINE_PHASE) {
        // This block tells Model::serve_init_mapping() to go into
        // SubspaceModel::serve_run() to build the subspace
        short mapping_code = SERVE_RUN;
        parallelLib.bcast(mapping_code, *pl_iter);
        parallelLib.bcast(offlineEvalConcurrency, *pl_iter);
      }

      // bcast mode to SubspaceModel::serve_run()
      parallelLib.bcast(mode, mi_pl);

      if (mode == OFFLINE_PHASE)
        pSubModel->set_communicators(pl_iter, offlineEvalConcurrency);
      else if (mode == ONLINE_PHASE)
        set_communicators(pl_iter, onlineEvalConcurrency);
    }
  }

  componentParallelMode = mode;
}


void SubspaceModel::serve_run(ParLevLIter pl_iter,
                                    int max_eval_concurrency)
{
  do {
    parallelLib.bcast(componentParallelMode, *pl_iter);
    if (componentParallelMode == OFFLINE_PHASE) {
      pSubModel->serve_run(pl_iter, offlineEvalConcurrency);
    } else if (componentParallelMode == ONLINE_PHASE) {
      set_communicators(pl_iter, onlineEvalConcurrency, false);
      pSubModel->serve_run(pl_iter, onlineEvalConcurrency);
    }
  } while (componentParallelMode != CONFIG_PHASE);
}


int SubspaceModel::serve_init_mapping(ParLevLIter pl_iter)
{
  short mapping_code = 0;
  int max_eval_concurrency = 1;
  int last_eval_concurrency = 0;
  do {
    parallelLib.bcast(mapping_code, *pl_iter);
    switch (mapping_code) {
    case FREE_COMMS:
      parallelLib.bcast(max_eval_concurrency, *pl_iter);
      if (max_eval_concurrency)
        free_communicators(pl_iter, max_eval_concurrency);
      break;
    case INIT_COMMS:
      last_eval_concurrency = serve_init_communicators(pl_iter);
      break;
    case SERVE_RUN:
      parallelLib.bcast(max_eval_concurrency, *pl_iter);
      if (max_eval_concurrency)
        serve_run(pl_iter, max_eval_concurrency);
      break;
    case ESTIMATE_MESSAGE_LENGTHS:
      estimate_message_lengths();
      break;
    default:
      // no-op
      break;
    }
  } while (mapping_code);

  return last_eval_concurrency; // Will be 0 unless serve_init_communicators()
  // is called
}


void SubspaceModel::stop_init_mapping(ParLevLIter pl_iter)
{
  short term_code = 0;
  parallelLib.bcast(term_code, *pl_iter);
}


/** Simplified derivative variables mapping where all continuous
    depend on all others.  TODO: Could instead rely on a richer
    default in RecastModel based on varsMapIndices. */
void SubspaceModel::
set_mapping(const Variables& reduced_vars, const ActiveSet& reduced_set,
	    ActiveSet& full_set)
{
  // if the reduced-space (recast) set specifies any continuous
  // variable, enable derivaties w.r.t. all continuous variables in
  // the full-space (sub) model

  // BMA: unless an empty DVV is allowed, could short-circuit this and
  // just always set all CV ids on the sub-model.  For now, this is
  // overly conservative.
  SizetArray full_dvv;
  size_t reduced_cv = reduced_vars.cv();
  const SizetArray& reduced_dvv = reduced_set.derivative_vector();
  size_t max_sm_id = smInstance->numFullspaceVars;
  for (size_t i=0; i<reduced_dvv.size(); ++i)
    if (1 <= reduced_dvv[i] && reduced_dvv[i] <= reduced_cv) {
      for (size_t j=1; j<=max_sm_id; ++j)
        full_dvv.push_back(j);
      break;
    }

  full_set.derivative_vector(full_dvv);
}


/**
  Perform the response mapping from submodel to recast response
*/
void SubspaceModel::
response_mapping(const Variables& reduced_vars, const Variables& full_vars,
                 const Response& full_resp, Response& reduced_resp)
{
  Teuchos::BLAS<int, Real> teuchos_blas;

  // Function values are the same for both recast and sub_model:
  reduced_resp.function_values(full_resp.function_values());

  const RealMatrix& W1 = smInstance->reducedBasis;

  // Transform the gradients:
  const RealMatrix& dg_dx = full_resp.function_gradients();
  if(!dg_dx.empty()) {
    RealMatrix dg_dy = reduced_resp.function_gradients();

    //  Performs the matrix-matrix operation:
    // dg_dy <- alpha*W1^T*dg_dx + beta*dg_dy
    int m = W1.numCols(), k = W1.numRows(), n = dg_dx.numCols();
    Real alpha = 1., beta = 0.;
    teuchos_blas.GEMM(Teuchos::TRANS, Teuchos::NO_TRANS, m, n, k, alpha,
                      W1.values(), k, dg_dx.values(), k, beta,dg_dy.values(),m);

    reduced_resp.function_gradients(dg_dy);
  }

  // Transform the Hessians:
  const RealSymMatrixArray& H_x_all = full_resp.function_hessians();
  if(!H_x_all.empty()) {
    RealSymMatrixArray H_y_all(H_x_all.size());
    for (int i = 0; i < H_x_all.size(); i++) {
      // compute H_y = W1^T * H_x * W1
      int m = W1.numRows(), n = W1.numCols();
      Real alpha = 1.;
      RealSymMatrix H_y(n, false);
      Teuchos::symMatTripleProduct<int,Real>(Teuchos::TRANS, alpha,
                                             H_x_all[i], W1, H_y);
      H_y_all[i] = H_y;
    }

    reduced_resp.function_hessians(H_y_all);
  }
}

}  // namespace Dakota


