/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "AdaptedBasisModel.hpp"
#include "ProbabilityTransformModel.hpp"
#include "dakota_linear_algebra.hpp"
#include "ParallelLibrary.hpp"
#include "DataFitSurrModel.hpp"
#include "NonDPolynomialChaos.hpp"

namespace Dakota {

/// initialization of static needed by RecastModel
AdaptedBasisModel* AdaptedBasisModel::abmInstance(NULL);


AdaptedBasisModel::AdaptedBasisModel(ProblemDescDB& problem_db):
  RecastModel(problem_db, get_sub_model(problem_db)),
  pcePilotExpansion(pcePilotExpRepPtr, false), numFullspaceVars(subModel.cv()),
  numFunctions(subModel.num_functions()), adaptedBasisInitialized(false),
  reducedRank(numFullspaceVars)//problem_db.get_int("model.subspace.dimension")
{
  abmInstance = this;
  modelType = "adapted_basis";
  supportsEstimDerivs = true;  // perform numerical derivatives in subspace
  componentParallelMode = CONFIG_PHASE;
    
  offlineEvalConcurrency = pcePilotExpansion.maximum_evaluation_concurrency();
  onlineEvalConcurrency = 1; // Will be overwritten with correct value in
                             // derived_init_communicators()

  validate_inputs();
}


Model AdaptedBasisModel::get_sub_model(ProblemDescDB& problem_db)
{
  const String& actual_model_pointer
    = problem_db.get_string("model.surrogate.actual_model_pointer");
  unsigned short ssg_level
    = problem_db.get_ushort("model.adapted_basis.sparse_grid_level");
  unsigned short exp_order
    = problem_db.get_ushort("model.adapted_basis.expansion_order");
  Real colloc_ratio
    = problem_db.get_real("model.adapted_basis.collocation_ratio");

  size_t model_index = problem_db.get_db_model_node(); // for restoration
  problem_db.set_db_model_nodes(actual_model_pointer);

  Model actual_model(problem_db.get_model());

    // configure pilot PCE object (instantiate now; build expansion at run time)
  RealVector dim_pref;
  if (ssg_level) {
    // L1 isotropic sparse grid --> Linear exp (quadratic main effects ignored)
    // L2 isotropic sparse grid --> Quadratic expansion
    UShortArray level_seq(1, ssg_level);
    pcePilotExpRepPtr
      = new NonDPolynomialChaos(actual_model, Pecos::COMBINED_SPARSE_GRID,
				level_seq, dim_pref, EXTENDED_U, false, false);
  }
  else if (exp_order) { // regression PCE: LeastSq/CS (exp_order,colloc_ratio)
    UShortArray exp_order_seq(1, exp_order); SizetArray colloc_pts_seq;
    short exp_coeffs_approach = Pecos::DEFAULT_REGRESSION;
    String import_file; unsigned short import_fmt = TABULAR_ANNOTATED;
    int seed = 12347;
    pcePilotExpRepPtr
      = new NonDPolynomialChaos(actual_model, exp_coeffs_approach,
				exp_order_seq, dim_pref, colloc_pts_seq,
				colloc_ratio, seed, EXTENDED_U,
				false, false, false,// piecewise,derivs,crossval
				import_file, import_fmt, false); // active_only
  }
  else {
    Cerr << "Error: insufficient PCE build specification in AdaptedBasisModel."
	 << std::endl;
    abort_handler(MODEL_ERROR);
  }

  // define level mappings even if none (sizing of finalStatistics deferred
  // until mappings are known)
  RealVectorArray empty_rva; // empty
  ((NonD*)pcePilotExpRepPtr)->
    requested_levels(empty_rva, empty_rva, empty_rva, empty_rva,
		     RELIABILITIES, COMPONENT, true, false);

  // since this construction precedes the construction of AdaptedBasisModel
  // member data, pcePilotExpansion would get overwritten by its (default)
  // initialization.  Therefore, we initialize pcePilotExpRepPtr above and then
  // assign it into pcePilotExpansion in the AdaptedBasisModel initializer list.
  //pcePilotExpansion.assign_rep(pce_rep);

  problem_db.set_db_model_nodes(model_index); // restore

  Model u_space_model(pcePilotExpRepPtr->algorithm_space_model());
  // Consider option of using PCE surrogate for all subsequent computations:
  //return u_space_model;

  // Return transformed model subordinate to the DataFitSurrModel:
  return u_space_model.subordinate_model();
}


AdaptedBasisModel::~AdaptedBasisModel()
{ /* empty dtor */ }


void AdaptedBasisModel::validate_inputs()
{
  bool error_flag = false;

  // validate variables specification
  if (subModel.div() > 0 || subModel.dsv() > 0 || subModel.drv() > 0) {
    error_flag = true;
    Cerr << "\nError (adapted basis model): only normal uncertain variables "
	 << "are supported;\n                        remove other variable "
         << "specifications.\n" << std::endl;
  }

  if (error_flag)
    abort_handler(-1);
}


bool AdaptedBasisModel::initialize_mapping(ParLevLIter pl_iter)
{
  RecastModel::initialize_mapping(pl_iter);

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\nAdapted Basis Model: Initializing adapted basis model."
	 << std::endl;

  // init-time setting of miPLIndex for use in component_parallel_mode()
  miPLIndex = modelPCIter->mi_parallel_level_index(pl_iter);

  // Set mode OFFLINE_PHASE
  component_parallel_mode(OFFLINE_PHASE);

  bool sub_model_resize = subModel.initialize_mapping(pl_iter);

  // runtime operation to identify the adapted basis model
  identify_subspace();
  // complete initialization of the base RecastModel
  initialize_recast();
  // convert the normal distributions to the reduced space and set in the
  // reduced model
  uncertain_vars_to_subspace();
  // adapted basis calculation now complete
  adaptedBasisInitialized = true;

  // Kill servers and return ranks [1,n-1] to serve_init_mapping()
  component_parallel_mode(CONFIG_PHASE);

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\nAdapted Basis Model: Initialization of adapted basis model "
	 << "is complete." << std::endl;

  // terminate here (without an error code) for now
  exit(0);

  // return whether size of variables has changed
  return (reducedRank != numFullspaceVars || // Active SS is reduced rank
	  sub_model_resize); // Active SS is full rank but subModel resized
}


bool AdaptedBasisModel::finalize_mapping()
{
  // TODO: return to full space
  //adaptedBasisInitialized = false;

  RecastModel::finalize_mapping();

  return false; // This will become true when TODO is implemented.
}


void AdaptedBasisModel::component_parallel_mode(short mode)
{
  // stop_servers() if they are active, componentParallelMode = 0 indicates
  // they are inactive
  if (componentParallelMode != mode &&
      componentParallelMode != CONFIG_PHASE) {
    ParConfigLIter pc_it = subModel.parallel_configuration_iterator();
    size_t index = subModel.mi_parallel_level_index();
    if (pc_it->mi_parallel_level_defined(index) &&
        pc_it->mi_parallel_level(index).server_communicator_size() > 1) {
      subModel.stop_servers();
    }
  }

  // activate new serve mode (matches AdaptedBasisModel::serve_run(pl_iter)).
  if (componentParallelMode != mode &&
      modelPCIter->mi_parallel_level_defined(miPLIndex)) {
    ParLevLIter pl_iter = modelPCIter->mi_parallel_level_iterator(miPLIndex);
    const ParallelLevel& mi_pl = modelPCIter->mi_parallel_level(miPLIndex);
    if (mi_pl.server_communicator_size() > 1) {
      if (mode == OFFLINE_PHASE) {
        // This block tells Model::serve_init_mapping() to go into 
        // AdaptedBasisModel::serve_run() to build the subspace
        short mapping_code = SERVE_RUN;
        parallelLib.bcast(mapping_code, *pl_iter);
        parallelLib.bcast(offlineEvalConcurrency, *pl_iter);
      }

      // bcast mode to AdaptedBasisModel::serve_run()
      parallelLib.bcast(mode, mi_pl);

      if (mode == OFFLINE_PHASE)
        subModel.set_communicators(pl_iter, offlineEvalConcurrency);
      else if (mode == ONLINE_PHASE)
        set_communicators(pl_iter, onlineEvalConcurrency);
    }
  }

  componentParallelMode = mode;
}


void AdaptedBasisModel::serve_run(ParLevLIter pl_iter,
                                    int max_eval_concurrency)
{
  do {
    parallelLib.bcast(componentParallelMode, *pl_iter);
    if (componentParallelMode == OFFLINE_PHASE)
      subModel.serve_run(pl_iter, offlineEvalConcurrency);
    else if (componentParallelMode == ONLINE_PHASE) {
      set_communicators(pl_iter, onlineEvalConcurrency, false);
      subModel.serve_run(pl_iter, onlineEvalConcurrency);
    }
  } while (componentParallelMode != CONFIG_PHASE);
}


int AdaptedBasisModel::serve_init_mapping(ParLevLIter pl_iter)
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


void AdaptedBasisModel::stop_init_mapping(ParLevLIter pl_iter)
{
  short term_code = 0;
  parallelLib.bcast(term_code, *pl_iter);
}


void AdaptedBasisModel::
derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                          bool recurse_flag)
{
  miPLIndex = modelPCIter->mi_parallel_level_index(pl_iter);// run time setting

  if (recurse_flag) {
    //if (!adaptedBasisInitialized) // see ActiveSubspaceModel
      pcePilotExpansion.set_communicators(pl_iter);

    subModel.set_communicators(pl_iter, max_eval_concurrency);

    // RecastModels do not utilize default set_ie_asynchronous_mode() as
    // they do not define the ie_parallel_level
    asynchEvalFlag = subModel.asynch_flag();
    evaluationCapacity = subModel.evaluation_capacity();
  }
}


void AdaptedBasisModel::identify_subspace()
{
  ////////////////////////////////////////////////////
  // Scope of AdaptedBasisModel:
  // There is the computation and then the use of the \mu subspace.
  // Current thinking is that AdaptedBasisModel does the former, but the latter
  // occurs within NonD methods such as NonDPolynomialChaos.  Special behavior
  // (e.g., coverting the PCE over subspace \mu to PCE over original \xi)
  // can be optionally performed (e.g., in post_run()) and triggered from
  // detection of the incoming model type)
  ////////////////////////////////////////////////////
  
  // Definitions: \xi is of dimension d, QoI are of dimension n,
  //              final \mu is of dimension \nu

  //////////////////////////////////////////////////////////////////////////////

  Cout << "\nAdapted Basis Model: Form pilot PCE"  << std::endl;

  // *** SNL:
  // For each QoI, we need to compute a pilot PCE:
  //   Step 0: perform any necessary probability transforms --> Wiener chaos
  //   Step 1: Compute a low order Hermite PCE: either linear or full quadratic

  ParLevLIter pl_iter = modelPCIter->mi_parallel_level_iterator(miPLIndex);
  pcePilotExpansion.run(pl_iter);
  Model pce_model(pcePilotExpansion.algorithm_space_model());
  const RealVectorArray& pce_coeffs = pce_model.approximation_coefficients();

  //////////////////////////////////////////////////////////////////////////////

  Cout << "\nAdapted Basis Model: Building A matrix for each QoI"  << std::endl;

  // For each QoI, we need to form the A_i rotation matrix:
  //   \eta_i = A_i \xi for i-th QoI
  //   \eta   = stacked \eta_i's

  Teuchos::LAPACK<int, Real> la;

  // composite set of A_i
  RealMatrix A_q(numFunctions*numFullspaceVars, numFullspaceVars, false);
  // Individual rotation matrix for each QoI
  RealMatrix A_i(numFullspaceVars, numFullspaceVars, false);

  // *** USC: 
  // Step 1a. linear PCE: use the alpha_i's as first row in A and then apply
  //          Gramm-Schmidt (BLAS/LAPACK?).
  //          [Can neglect constant term/expansion mean]
  size_t i, j, k, row_cntr = 0;
  for (i=0; i<numFunctions; ++i) {
    A_i.putScalar(0.);
    for (j=0; j<numFullspaceVars; ++j)
      A_i(0,j) = pce_coeffs[i][j+1]; // offset by 1 to neglect constant/mean
    for (j=1; j<numFullspaceVars; ++j)
      A_i(j,j) = 1.;
    // Gramm-Schmidt for each rotation matrix:
    //la.DGEQRF(A_i); ???
    // Append A_i into A_q
    for (j=0; j<numFullspaceVars; ++j, ++row_cntr)
      for (k=0; k<numFullspaceVars; ++k)
	A_q(row_cntr,k) = A_i(j,k);
  }

  // Step 1b: same as 1a expect permuted location of 1's determined from q_i's
  //          (relative sensitivities)

  // TO DO

  // Step 1c: full quadratic PCE: solve LP for A from A' S A = D for S = matrix
  //          of quadratic terms
  //          A' S A = D is same as S A = A D  because A'A = I
  //          --> A are eigenvectors and D are eigenvalues of S
  //
  //   See notes on how to form S (exploit symmetry of S from computed
  //   PCE, use quadratic coefficients organized in matrix form).
  //   [Neglect constant and linear terms for purposes of discovering A]

  //////////////////////////////////////////////////////////////////////////////

  Cout << "\nAdapted Basis Model: Composing composite reduction"  << std::endl;

  // Given A_i for i=1,..,numFunctions, we need to compute a composite eta:
  // Refer to board notes / emerging article:

  //   Stack A_i into a tall matrix A, we target KLE of \eta using eigenspectrum
  //   of covariance A A', which has rank d (Mercer's theorem)

  //   \eta = \Sum_{i=1}^d \sqrt{\lamba_i} \phi_i \mu_i
  //     for eigenvalues lambda, eigenvectors \phi, and new std normal vars \mu

  //   Form SVD of A in manner similar to active subspace for grad_f grad_f'
  //     = J' J
  //   (Eigenvectors of A A' are left singular vectors of A)

  //      *** TO DO: adapt code from  ActiveSubspaceModel::compute_svd() which 
  //          uses svd() helper from dakota_linear_algebra.hpp.
  //          Apply truncation criterion

  RealVector singular_values;
  RealMatrix V_transpose; // right eigenvectors, not used
  // we want left singular vectors but don't overwrite A, so make a deep copy
  RealMatrix left_singular_vectors = A_q;
  svd(left_singular_vectors, singular_values, V_transpose);

  //   Truncate eigenvalues of covariance at some pre-selected level
  //     --> dimension \nu reduced from dimension d

  // Could use Teuchos::View, but partial traversal of full matrices
  // works fine for computations to follow
  //RealVector truncated_singular_values       = singular_values;
  //RealMatrix truncated_left_singular_vectors = left_singular_vectors;

  reducedRank = /*truncated_*/singular_values.length(); // TO DO

  // Rewrite KLE as
  //   \eta = \Phi \Lamba \mu   where   \eta = A \xi
  // where \mu is \nu x 1 reduced vector after truncation

  // \Phi \Lambda \mu = A \xi,  pre-multiply by A'
  //    A' \Phi \Lambda \mu = (A' A) \xi
  // where A' A = \Sum_{i=1}^n A_i' A_i = n I

  // Dimension reduction map is then:
  //   \xi (full dimension d) = 1/n A' \Phi \Lambda \mu (reduced dimension)

  // Pre-compute 1/n A' \Phi \Lambda:
  rotationMatrix.shapeUninitialized(numFullspaceVars, reducedRank);
  for (i=0; i<numFullspaceVars; ++i) {
    const Real* A_col = A_q[i]; // row of A'
    for (j=0; j<reducedRank; ++j) {
      const Real* U_col = /*truncated_*/left_singular_vectors[j];
      Real sum_prod = 0.;
      for (k=0; k<numFullspaceVars*numFunctions; ++k)
	sum_prod += A_col[k] * U_col[k];
      rotationMatrix(i,j) = sum_prod * /*truncated_*/singular_values[j];
    }
  }
  rotationMatrix.scale(1./numFunctions);

  //////////////////////////////////////////////////////////////////////////////

  /*
  // TO DO: do we need to transpose or invert rotationMatrix to be consistent
  //        with A definition used by ActiveSubspaceModel ???

  A = RealMatrix(reducedRank, numFullspaceVars);
  */

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "\nAdapted Basis Model: rotation matrix is:\n";
    write_data(Cout, rotationMatrix, true, true, true);
  }
  Cout << "\n****************************************************************"
       << "**********\nAdapted Basis Model: Build Statistics"
       << "\nsubspace size: " << reducedRank << "\n**************************"
       << "************************************************\n";
}


/** Initialize the recast model based on the reduced space, with no
    response function mapping (for now). */
void AdaptedBasisModel::initialize_recast()
{
  // For now, we assume the subspace is over all functions, without
  // distinguishing primary from secondary

  // ---
  // Variables mapping: RecastModel maps subspace (reduced) variables to
  // original fullspace model
  // ---

  // We assume the mapping is for all active variables, but only
  // continuous for the active subspace
  size_t submodel_cv = subModel.cv();
  size_t submodel_dv = subModel.div() + subModel.dsv() + subModel.drv();
  size_t submodel_vars = submodel_cv + submodel_dv;
  size_t recast_cv = reducedRank;
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

  SizetArray vars_comps_total = variables_resize();
  BitArray all_relax_di, all_relax_dr; // default: empty; no discrete relaxation

  // Primary and secondary mapping are one-to-one (NULL callbacks)
  // TODO: can we get RecastModel to tolerate empty indices when no
  // map is present?
  size_t num_primary = subModel.num_primary_fns(),
    num_secondary = subModel.num_functions() - subModel.num_primary_fns(),
    num_recast_fns = num_primary + num_secondary,
    recast_secondary_offset = subModel.num_nonlinear_ineq_constraints();

  Sizet2DArray primary_resp_map_indices(num_primary);
  for (size_t i=0; i<num_primary; i++) {
    primary_resp_map_indices[i].resize(1);
    primary_resp_map_indices[i][0] = i;
  }

  Sizet2DArray secondary_resp_map_indices(num_secondary);
  for (size_t i=0; i<num_secondary; i++) {
    secondary_resp_map_indices[i].resize(1);
    secondary_resp_map_indices[i][0] = num_primary + i;
  }

  BoolDequeArray nonlinear_resp_mapping(numFunctions,
                                        BoolDeque(numFunctions, false));

  // Initial response order for the newly built adapted basis model same as
  // the subModel (does not augment with gradient request)
  const Response& curr_resp = subModel.current_response();
  short recast_resp_order = 1; // recast resp order to be same as original resp
  if (!curr_resp.function_gradients().empty()) recast_resp_order |= 2;
  if (!curr_resp.function_hessians().empty())  recast_resp_order |= 4;

  RecastModel::
    init_sizes(vars_comps_total, all_relax_di, all_relax_dr, num_primary,
               num_secondary, recast_secondary_offset, recast_resp_order);

  RecastModel::
    init_maps(vars_map_indices, nonlinear_vars_mapping, vars_mapping,
              set_mapping, primary_resp_map_indices, secondary_resp_map_indices,
              nonlinear_resp_mapping, response_mapping, NULL);
}


/// Create a variables components totals array with the reduced space
/// size for continuous variables
SizetArray AdaptedBasisModel::variables_resize()
{
  const SharedVariablesData& svd = subModel.current_variables().shared_data();
  SizetArray vc_totals = svd.components_totals();
  if (reducedRank != subModel.cv()) {
    short active_view = subModel.current_variables().view().first;
    switch (active_view) {
      
    case MIXED_DESIGN: case RELAXED_DESIGN:
      // resize continuous design
      vc_totals[TOTAL_CDV] = reducedRank;
      break;

    case MIXED_ALEATORY_UNCERTAIN: case RELAXED_ALEATORY_UNCERTAIN:
      // resize continuous aleatory
      vc_totals[TOTAL_CAUV] = reducedRank;
      break;

    case MIXED_UNCERTAIN: case RELAXED_UNCERTAIN:
    case MIXED_EPISTEMIC_UNCERTAIN: case RELAXED_EPISTEMIC_UNCERTAIN:
      // resize continuous epistemic (note there may not actually be
      // any epistemic variables in the *_UNCERTAIN cases)
      vc_totals[TOTAL_CEUV] = reducedRank;
      break;

    case MIXED_ALL: case RELAXED_ALL: case MIXED_STATE: case RELAXED_STATE:
      // resize continuous state
      vc_totals[TOTAL_CSV] = reducedRank;
      break;

    default:
      Cerr << "\nError (adapted basis model): invalid active variables view "
           << active_view << "." << std::endl;
      abort_handler(-1);
      break;

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
/// transform and set the distribution parameters in the reduced model
void AdaptedBasisModel::uncertain_vars_to_subspace()
{
  const Pecos::AleatoryDistParams& native_params =
    subModel.aleatory_distribution_parameters();

  // update the reduced space model
  Pecos::AleatoryDistParams& reduced_dist_params =
    aleatory_distribution_parameters();

  // initialize AleatoryDistParams for reduced model
  // This is necessary if subModel has been transformed
  // to standard normals from a different distribution
  reduced_dist_params.copy(native_params); // deep copy

  // native space characterization
  const RealVector& mu_x = native_params.normal_means();
  const RealVector& sd_x = native_params.normal_std_deviations();
  const RealSymMatrix& correl_x = native_params.uncertain_correlations();

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "\nAdapted Basis Model: correl_x = \n";
    write_data(Cout, correl_x, true, true, true);
  }

  bool native_correl = correl_x.empty() ? false : true;
  if (native_correl && correl_x.numRows() != numFullspaceVars) {
    Cerr << "\nError (adapted basis model): Wrong correlation size."<<std::endl;
    abort_handler(-1);
  }

  // reduced space characterization: mean mu, std dev sd
  RealVector mu_y(reducedRank), sd_y(reducedRank);

  // mu_y = A^T * mu_x
  int m = rotationMatrix.numRows(), n = rotationMatrix.numCols(),
    incx = 1, incy = 1;
  Real alpha = 1.0, beta = 0.0;
  // y <-- alpha*A*x + beta*y
  // mu_y <-- 1.0 * A^T * mu_x + 0.0 * mu_y
  Teuchos::BLAS<int, Real> teuchos_blas;
  teuchos_blas.GEMV(Teuchos::TRANS, m, n, alpha, rotationMatrix.values(), m,
                    mu_x.values(), incx, beta, mu_y.values(), incy);

  // convert the correlations C_x to variance V_x
  // V_x <-- diag(sd_x) * C_x * diag(sd_x)
  // not using symmetric so we can multiply() below
  RealMatrix V_x(m, m, false);
  if (native_correl) {
    for (int row=0; row<m; ++row)
      for (int col=0; col<m; ++col)
        V_x(row, col) = sd_x(row)*correl_x(row,col)*sd_x(col);
  }
  else {
    V_x = 0.0;
    for (int row=0; row<m; ++row)
      V_x(row, row) = sd_x(row)*sd_x(row);
  }

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "\nAdapted Basis Model: A = \n";
    write_data(Cout, rotationMatrix, true, true, true);
    Cout << "\nAdapted Basis Model: V_x =\n";
    write_data(Cout, V_x, true, true, true);
  }

  // compute V_y = U^T * V_x * U
  alpha = 1.0;
  beta = 0.0;
  RealMatrix UTVx(n, m, false);
  UTVx.multiply(Teuchos::TRANS, Teuchos::NO_TRANS,
                alpha, rotationMatrix, V_x, beta);
  RealMatrix V_y(reducedRank, reducedRank, false);
  V_y.multiply(Teuchos::NO_TRANS, Teuchos::NO_TRANS,
               alpha, UTVx, rotationMatrix, beta);

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "\nAdapted Basis Model: V_y = \n";
    write_data(Cout, V_y, true, true, true);
  }

  // compute the standard deviations in reduced space
  for (int i=0; i<reducedRank; ++i)
    sd_y(i) = std::sqrt(V_y(i,i));

  reduced_dist_params.normal_means(mu_y);
  reduced_dist_params.normal_std_deviations(sd_y);


  // compute the correlations in reduced space
  // TODO: fix symmetric access to not loop over whole matrix
  //  if (native_correl) {

  // Unless the native correl was alpha*I, the reduced variables will
  // be correlated in general, so always set the correltions
  RealSymMatrix correl_y(reducedRank, false);
  for (int row=0; row<reducedRank; ++row)
    for (int col=0; col<reducedRank; ++col)
      correl_y(row, col) = V_y(row,col)/sd_y(row)/sd_y(col);

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "\nAdapted Basis Model: correl_y = \n";
    write_data(Cout, correl_y, true, true, true);
  }

  reduced_dist_params.uncertain_correlations(correl_y);

  // Set continuous variable types:
  UShortMultiArray cont_variable_types(boost::extents[reducedRank]);
  for (int i = 0; i < reducedRank; i++) {
    cont_variable_types[i] = NORMAL_UNCERTAIN;
  }
  currentVariables.continuous_variable_types(
    cont_variable_types[boost::indices[idx_range(0, reducedRank)]]);

  // Set currentVariables to means of active variables:
  continuous_variables(mu_y);

  // update variable labels for adapted basis
  StringMultiArray adapted_basis_var_labels(boost::extents[reducedRank]);
  for (int i = 0; i < reducedRank; i++)
    adapted_basis_var_labels[i]
      = "abv_" + boost::lexical_cast<std::string>(i+1);
  continuous_variable_labels(
    adapted_basis_var_labels[boost::indices[idx_range(0, reducedRank)]]);
}



/** Perform the variables mapping from recast reduced dimension
    variables y to original model x variables via linear
    transformation.  Maps only continuous variables. */
void AdaptedBasisModel::
vars_mapping(const Variables& recast_y_vars, Variables& sub_model_x_vars)
{
  Teuchos::BLAS<int, Real> teuchos_blas;

  const RealVector& y =    recast_y_vars.continuous_variables();
  RealVector&       x = sub_model_x_vars.continuous_variables_view();

  //  Calculate x = A*y + inA*inactiveVars via matvec
  //  directly into x cv in submodel
  const RealMatrix& W1 = abmInstance->rotationMatrix;
  int m = W1.numRows(), n = W1.numCols(), incx = 1, incy = 1;
  Real alpha = 1.0, beta = 0.0;
  teuchos_blas.GEMV(Teuchos::NO_TRANS, m, n, alpha, W1.values(), m,
                    y.values(), incy, beta, x.values(), incx);

  if (abmInstance->outputLevel >= DEBUG_OUTPUT) {
    Cout << "\nAdapted Basis Model: Subspace vars are\n";
    Cout << recast_y_vars << std::endl;

    Cout << "\nAdapted Basis Model: Fullspace vars are\n";
    Cout << sub_model_x_vars << std::endl;
  }

}


/** Simplified derivative variables mapping where all continuous
    depend on all others.  TODO: Could instead rely on a richer
    default in RecastModel based on varsMapIndices. */
void AdaptedBasisModel::set_mapping(const Variables& recast_vars,
                                      const ActiveSet& recast_set,
                                      ActiveSet& sub_model_set)
{
  // if the reduced-space (recast) set specifies any continuous
  // variable, enable derivaties w.r.t. all continuous variables in
  // the full-space (sub) model

  // BMA: unless an empty DVV is allowed, could short-circuit this and
  // just always set all CV ids on the sub-model.  For now, this is
  // overly conservative.
  SizetArray sub_model_dvv;
  size_t recast_cv = recast_vars.cv();
  const SizetArray& recast_dvv = recast_set.derivative_vector();
  size_t max_sm_id = abmInstance->subModel.cv();
  for (size_t i=0; i<recast_dvv.size(); ++i)
    if (1 <= recast_dvv[i] && recast_dvv[i] <= recast_cv) {
      for (size_t j=1; j<=max_sm_id; ++j)
        sub_model_dvv.push_back(j);
      break;
    }
  sub_model_set.derivative_vector(sub_model_dvv);
}


/**
  Perform the response mapping from submodel to recast response
*/
void AdaptedBasisModel::
response_mapping(const Variables& recast_y_vars,
                 const Variables& sub_model_x_vars,
                 const Response& sub_model_resp, Response& recast_resp)
{
  Teuchos::BLAS<int, Real> teuchos_blas;

  // Function values are the same for both recast and sub_model:
  recast_resp.function_values(sub_model_resp.function_values());

  // Gradients and Hessians must be transformed though:
  const RealMatrix& dg_dx = sub_model_resp.function_gradients();
  if(!dg_dx.empty()) {
    RealMatrix dg_dy = recast_resp.function_gradients();

    //  Performs the matrix-matrix operation:
    // dg_dy <- alpha*W1^T*dg_dx + beta*dg_dy
    const RealMatrix& W1 = abmInstance->rotationMatrix;
    int m = W1.numCols(), k = W1.numRows(), n = dg_dx.numCols();
    Real alpha = 1.0, beta = 0.0;
    teuchos_blas.GEMM(Teuchos::TRANS, Teuchos::NO_TRANS, m, n, k, alpha,
                      W1.values(), k, dg_dx.values(), k, beta,
		      dg_dy.values(), m);

    recast_resp.function_gradients(dg_dy);
  }

  
  // Now transform the Hessians:
  const RealSymMatrixArray& H_x_all = sub_model_resp.function_hessians();
  if(!H_x_all.empty()) {
    RealSymMatrixArray H_y_all(H_x_all.size());
    for (int i = 0; i < H_x_all.size(); i++) {
      // compute H_y = W1^T * H_x * W1
      const RealMatrix& W1 = abmInstance->rotationMatrix;
      int m = W1.numRows(), n = W1.numCols();
      Real alpha = 1.0;
      RealSymMatrix H_y(n, false);
      Teuchos::symMatTripleProduct<int,Real>(Teuchos::TRANS, alpha,
                                             H_x_all[i], W1, H_y);
      H_y_all[i] = H_y;
    }

    recast_resp.function_hessians(H_y_all);
  }
}

}  // namespace Dakota


