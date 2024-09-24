/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "AdaptedBasisModel.hpp"
#include "dakota_linear_algebra.hpp"
#include "ParallelLibrary.hpp"
#include "MarginalsCorrDistribution.hpp"
#include "NonDPolynomialChaos.hpp"
#include "SharedPecosApproxData.hpp"
#include <random>
#include "LHSDriver.hpp"
#define N 2

namespace Dakota {


AdaptedBasisModel::
AdaptedBasisModel(ProblemDescDB& problem_db):
  method_rotation(problem_db.get_short("model.adapted_basis.rotation_method")),
  adaptedBasisTruncationTolerance(probDescDB.get_real(
    "model.adapted_basis.truncation_tolerance")),
  subspaceDimension(probDescDB.get_int(
    "model.subspace.dimension")),  
  SubspaceModel(problem_db, get_sub_model(problem_db))
{
  // BMA: can't do this in get_sub_model as Iterator envelope hasn't
  // been default constructed yet; for now we transfer ownership of
  // this pointer into a shared pointer, as delete isn't being called
  // in this class anyway.
  pcePilotExpansion.assign_rep(std::shared_ptr<NonDPolynomialChaos>
			       (pcePilotExpRepPtr));
  modelType = "adapted_basis";
  modelId = RecastModel::recast_model_id(root_model_id(), "ADAPTED_BASIS");
  supportsEstimDerivs = true;  // perform numerical derivatives in subspace

  validate_inputs();

  offlineEvalConcurrency = pcePilotExpansion.maximum_evaluation_concurrency();
  
}


AdaptedBasisModel::~AdaptedBasisModel()
{ }


Model AdaptedBasisModel::get_sub_model(ProblemDescDB& problem_db)
{
  const String& actual_model_pointer
    = problem_db.get_string("model.surrogate.truth_model_pointer");
  unsigned short ssg_level
    = problem_db.get_ushort("model.adapted_basis.sparse_grid_level");
  unsigned short exp_order
    = problem_db.get_ushort("model.adapted_basis.expansion_order");
  Real colloc_ratio
    = problem_db.get_real("model.adapted_basis.collocation_ratio");
  short refine_type
      = problem_db.get_short("method.nond.expansion_refinement_type"),
    refine_cntl
      = problem_db.get_short("method.nond.expansion_refinement_control"),
    cov_cntl = problem_db.get_short("method.nond.covariance_control"),
    rule_nest = problem_db.get_short("method.nond.nesting_override"),
    rule_growth = problem_db.get_short("method.nond.growth_override");
  bool pw_basis = problem_db.get_bool("method.nond.piecewise_basis"),
     use_derivs = problem_db.get_bool("method.derivative_usage");

  size_t model_index = problem_db.get_db_model_node(); // for restoration
  problem_db.set_db_model_nodes(actual_model_pointer);

  Model actual_model(problem_db.get_model());

    // configure pilot PCE object (instantiate now; build expansion at run time)
  RealVector dim_pref;
  if (ssg_level) {
    // L1 isotropic sparse grid --> Linear exp (quadratic main effects ignored)
    // L2 isotropic sparse grid --> Quadratic expansion
    pcePilotExpRepPtr = new NonDPolynomialChaos(actual_model,
      Pecos::COMBINED_SPARSE_GRID, ssg_level, dim_pref, EXTENDED_U, refine_type,
      refine_cntl, cov_cntl, rule_nest, rule_growth, pw_basis, use_derivs);
  }
  else if (exp_order) { // regression PCE: LeastSq/CS (exp_order,colloc_ratio)
    short exp_coeffs_approach = Pecos::DEFAULT_REGRESSION;
    String import_file; int seed = 12347;  size_t colloc_pts = SZ_MAX;
    pcePilotExpRepPtr = new NonDPolynomialChaos(actual_model,
      exp_coeffs_approach, exp_order, dim_pref, colloc_pts, colloc_ratio, seed,
      EXTENDED_U, refine_type, refine_cntl, cov_cntl, //rule_nest, rule_growth,
      pw_basis, use_derivs, problem_db.get_bool("method.nond.cross_validation"),
      import_file, TABULAR_ANNOTATED, false);
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


/**  This specialization is because the model is used in multiple contexts
     depending on build phase. */
void AdaptedBasisModel::
derived_init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                           bool recurse_flag)
{
  // The inbound subModel concurrency accounts for any finite differences

  onlineEvalConcurrency = max_eval_concurrency;

  if (recurse_flag) {
    //if (!mappingInitialized)
      pcePilotExpansion.init_communicators(pl_iter);
    subModel.init_communicators(pl_iter, max_eval_concurrency);
  }
}


void AdaptedBasisModel::
derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                          bool recurse_flag)
{
  miPLIndex = modelPCIter->mi_parallel_level_index(pl_iter);// run time setting

  if (recurse_flag) {
    //if (!mappingInitialized)
      pcePilotExpansion.set_communicators(pl_iter);

    subModel.set_communicators(pl_iter, max_eval_concurrency);

    // RecastModels do not utilize default set_ie_asynchronous_mode() as
    // they do not define the ie_parallel_level
    asynchEvalFlag = subModel.asynch_flag();
    evaluationCapacity = subModel.evaluation_capacity();
  }
}


void AdaptedBasisModel::
derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                           bool recurse_flag)
{
  if (recurse_flag) {
    pcePilotExpansion.free_communicators(pl_iter);
    subModel.free_communicators(pl_iter, max_eval_concurrency);
  }
}


void AdaptedBasisModel::validate_inputs()
{
  SubspaceModel::validate_inputs();

  bool error_flag = false;

  // validate reduced dimension
  if (subspaceDimension > numFullspaceVars) {
    error_flag = true;
    Cerr << "\nError (dimension): Required rotation dimension larger than the full problem dimension;"
         << "\n                        Please select dimension < number of variables\n" << std::endl;
  }

  if (error_flag)
    abort_handler(-1);
}


void AdaptedBasisModel::compute_subspace()
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

  // retrieve the underlying PCE approximation model
  Model u_space_model(pcePilotExpRepPtr->algorithm_space_model());

  // dive through the class layers to grab ptr to shared approx data:
  std::shared_ptr<SharedPecosApproxData> data_rep =
      std::static_pointer_cast<SharedPecosApproxData>
      (u_space_model.shared_approximation().data_rep());

  // retrieve the multi-index as a 2D array of unsigned short integers
  // first index is term, second index is variable
  const UShort2DArray& pce_multi_index = data_rep->multi_index();

//   Cout << "\npce_multi_index = \n" << pce_multi_index << std::endl;
//   Cout <<"\npce_coeffs = \n"<<pce_coeffs<<std::endl;
  

  // find the indices of the first order PCE terms
  // which is used later for constunction of rotation matrix
  UShortArray first_ord_index(numFullspaceVars);
  size_t i;
  int num_pce, num_ones, num_zeros, idx = 0;
  num_pce = pce_multi_index.size();

  for (i=0; i<num_pce; ++i) {
    num_ones = std::count(std::begin(pce_multi_index[i]), std::end(pce_multi_index[i]), 1);
    num_zeros = std::count(std::begin(pce_multi_index[i]), std::end(pce_multi_index[i]), 0);
    if ( num_ones == 1 && (num_zeros == numFullspaceVars-1) ){
      first_ord_index[idx] = i;
      idx++;
    }
  }

  //////////////////////////////////////////////////////////////////////////////

  Cout << "\nAdapted Basis Model: Building A matrix for each QoI"  << std::endl;

  // For each QoI, we need to form the A_i rotation matrix:
  //   \eta_i = A_i \xi for i-th QoI
  //   \eta   = stacked \eta_i's

  Teuchos::LAPACK<int, Real> la;

  // composite set of A_i
  RealMatrix A_q(numFns*numFullspaceVars, numFullspaceVars, false);
  // Individual rotation matrix for each QoI
  RealMatrix A_i(numFullspaceVars, numFullspaceVars, false);

  size_t j, k, row_cntr = 0;
  int lwork;
  lwork = numFullspaceVars;
  int info;
  
  int method = -1;
    
  // Select the desired rotation method
  if (method_rotation==ROTATION_METHOD_UNRANKED){
   
      method = 0;
      std::cout << "\nSelecting UNRANKED Gaussian for the rotation matrix construction" << std::endl;
      
  } else if (method_rotation==ROTATION_METHOD_RANKED) {
  
      method = 1;
      std::cout << "\nSelecting RANKED Gaussian for the rotation matrix construction" << std::endl;
          
  }
      

  for (i=0; i<numFns; ++i) {
    A_i.putScalar(0.);

    if (method==0){
      // Step 1a. linear PCE: use the alpha_i's as first row in A and then apply
      //          Gramm-Schmidt
      //          [Can neglect constant term/expansion mean]
      for (j=0; j<numFullspaceVars; ++j)
        A_i(0,j) = pce_coeffs[i][first_ord_index[j]]; // offset by 1 to neglect constant/mean
      for (j=1; j<numFullspaceVars; ++j)
        A_i(j,j) = 1.;
    }

    else if (method==1){
      // Step 1b: same as 1a except permuted location of 1's determined from q_i's
      //          (relative sensitivities)
      for (j=0; j<numFullspaceVars; ++j)
        A_i(0,j) = pce_coeffs[i][first_ord_index[j]]; // offset by 1 to neglect constant/mean
      // rank the first order PC coefficients by absolute values
      RealVector pce_coeffs_lin_abs(numFullspaceVars);
      for (j=0; j<numFullspaceVars; ++j){
        pce_coeffs_lin_abs(j) = std::abs(pce_coeffs[i][first_ord_index[j]]);
      }
      std::vector < std::pair<double, int> > pce_coeffs_pair;
      for (int j = 0; j < numFullspaceVars; ++j) {
        pce_coeffs_pair.push_back(std::make_pair(pce_coeffs_lin_abs[j], j));
      }
      std::sort(pce_coeffs_pair.begin(), pce_coeffs_pair.end());
      // allocate most sensitive coefficients to the remaining rows of A_i
      // by descending order, the column indices are the same as in the first row
      for (j=1; j<numFullspaceVars; ++j)
      {
        int index = pce_coeffs_pair[numFullspaceVars-j].second;
        A_i(j,index) = pce_coeffs[i][first_ord_index[index]];
      }
    }

    else {
      Cout << "\nBasis adaptation method not supported!!" << std::endl;
    }

    // Gram-Schmidt for each rotation matrix:
    double *tau = (double *) malloc( sizeof(double) * numFullspaceVars );
    double *work = (double *) malloc( sizeof(double) * lwork );

    RealMatrix A_i_trans(A_i, Teuchos::TRANS);
    la.GEQRF(numFullspaceVars,  numFullspaceVars, A_i_trans.values(),  numFullspaceVars,
            tau, work, lwork, &info);
    la.ORGQR(numFullspaceVars, numFullspaceVars, numFullspaceVars,
            A_i_trans.values(), numFullspaceVars, tau, work, lwork, &info);
    free (tau);
    free (work);

    // Append A_i into A_q
    for (j=0; j<numFullspaceVars; ++j, ++row_cntr)
      for (k=0; k<numFullspaceVars; ++k)
	      A_q(row_cntr,k) = A_i_trans(k,j);
  }
  
  reducedBasis = A_q;
  // Cout << "\nreducedBasis:\n" << reducedBasis << std::endl;

  if ( !subspaceDimension )
      truncate_rotation();
  else
      reducedRank = subspaceDimension;

  Cout << "\nreducedRank = " << reducedRank << std::endl;

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

  //// Cout << "\nAdapted Basis Model: Composing composite reduction"  << std::endl;

  // Given A_i for i=1,..,numFns, we need to compute a composite eta:
  // Refer to board notes / emerging article:

  //   Stack A_i into a tall matrix A, we target KLE of \eta using eigenspectrum
  //   of covariance A A', which has rank d (Mercer's theorem)

  //   \eta = \Sum_{i=1}^d \sqrt{\lamba_i} \phi_i \mu_i
  //     for eigenvalues lambda, eigenvectors \phi, and new std normal vars \mu

  //   Form SVD of A in manner similar to active subspace for grad_f grad_f'
  //     = J' J
  //   (Eigenvectors of A A' are left singular vectors of A)

  //   *** TO DO: adapt code from  ActiveSubspaceModel::compute_svd() which
  //       uses singular_value_decomp() helper from dakota_linear_algebra.hpp.
  //       Apply truncation criterion.

  //// RealVector singular_values;
  //// RealMatrix V_transpose; // right eigenvectors, not used
  // we want left singular vectors but don't overwrite A, so make a deep copy
  //// RealMatrix left_singular_vectors = A_q;
  //// singular_value_decomp(left_singular_vectors,singular_values,V_transpose);

  //   Truncate eigenvalues of covariance at some pre-selected level
  //     --> dimension \nu reduced from dimension d

  // Could use Teuchos::View, but partial traversal of full matrices
  // works fine for computations to follow
  //RealVector truncated_singular_values       = singular_values;
  //RealMatrix truncated_left_singular_vectors = left_singular_vectors;

  //// reducedRank = /*truncated_*/singular_values.length(); // TO DO

  // Rewrite KLE as
  //   \eta = \Phi \Lamba \mu   where   \eta = A \xi
  // where \mu is \nu x 1 reduced vector after truncation

  // \Phi \Lambda \mu = A \xi,  pre-multiply by A'
  //    A' \Phi \Lambda \mu = (A' A) \xi
  // where A' A = \Sum_{i=1}^n A_i' A_i = n I

  // Dimension reduction map is then:
  //   \xi (full dimension d) = 1/n A' \Phi \Lambda \mu (reduced dimension)

  // Pre-compute 1/n A' \Phi \Lambda:
  //// reducedBasis.shapeUninitialized(numFullspaceVars, reducedRank);
  //// for (i=0; i<numFullspaceVars; ++i) {
  ////   const Real* A_col = A_q[i]; // row of A'
  ////  for (j=0; j<reducedRank; ++j) {
  ////    const Real* U_col = /*truncated_*/left_singular_vectors[j];
  ////    Real sum_prod = 0.;
  ////    for (k=0; k<numFullspaceVars*numFns; ++k)
  ////	      sum_prod += A_col[k] * U_col[k];
  ////    reducedBasis(i,j) = sum_prod * /*truncated_*/singular_values[j];
  ////  }
  ////}
  ////reducedBasis.scale(1./numFns);
  ////Cout << "\nRotation matrix\n" << reducedBasis << std::endl;


  //////////////////////////////////////////////////////////////////////////////

  /*
  // TO DO: do we need to transpose or invert reducedBasis to be consistent
  //        with A definition used by ActiveSubspaceModel ???

  A = RealMatrix(reducedRank, numFullspaceVars);
  */

  //// if (outputLevel >= DEBUG_OUTPUT)
  ////   Cout << "\nAdapted Basis Model: rotation matrix is:\n" << reducedBasis;
  //// Cout << "\n****************************************************************"
  ////      << "**********\nAdapted Basis Model: Build Statistics"
  ////      << "\nsubspace size: " << reducedRank << "\n**************************"
  ////      << "************************************************\n";
}



void AdaptedBasisModel::truncate_rotation()
{
  reducedRank = 1; // initialize reducedRank
  double threshold_ratio = adaptedBasisTruncationTolerance;  // default value 0.8
  
  std::cout << "\nThreshold Ratio: " << adaptedBasisTruncationTolerance << std::endl;
  
  // use the first order information to determine the dimension of
  // the reduce space

  // standard normal distribution generator
  int seed = 12345;
  int nsamples = 200;
  if (nsamples < numFullspaceVars*2 +1)
    nsamples = numFullspaceVars*2 +1;
  size_t i, j, k;
  RealVector means_vec(numFullspaceVars), std_deviations(numFullspaceVars),
             lower_bnds(numFullspaceVars), upper_bnds(numFullspaceVars);
  means_vec.putScalar(0.0);
  std_deviations.putScalar(1.0);
  Real dbl_inf = std::numeric_limits<Real>::infinity();
  lower_bnds.putScalar(-dbl_inf);
  upper_bnds.putScalar( dbl_inf);
  RealMatrix xi;
  RealSymMatrix correl_matrix;
  correl_matrix.shape(numFullspaceVars);
  for (i=0; i<numFullspaceVars; i++)
      correl_matrix(i,i) = 1.;
  short sample_ranks_mode = 0; //IGNORE RANKS
  Pecos::LHSDriver lhsDriver; // the C++ wrapper for the F90 LHS library
  lhsDriver.seed(seed);
  lhsDriver.initialize("lhs", sample_ranks_mode, true);
  lhsDriver.generate_normal_samples(means_vec, std_deviations, lower_bnds,
              upper_bnds, correl_matrix, nsamples, xi);
  
  // map the full dimension \xi variable to the rotated variables \eta
  const RealMatrix A(Teuchos::Copy, reducedBasis, numFullspaceVars, numFullspaceVars);
  int m = A.numRows(), n = A.numCols();
  Real alpha = 1.0, beta = 0.0;
  RealMatrix eta(numFullspaceVars, nsamples, false);
  Teuchos::BLAS<int, Real> teuchos_blas;
  teuchos_blas.GEMM(Teuchos::NO_TRANS, Teuchos::NO_TRANS, numFullspaceVars, nsamples, numFullspaceVars,
                    alpha, A.values(), numFullspaceVars, xi.values(), numFullspaceVars,
                    beta, eta.values(), numFullspaceVars);


  // 1) map \eta of reduced dimensions to the full dimensional space as \xi_trans
  // 2) compute the weighted difference between \xi and \xi_trans
  // 3) compute metric w = 1/N \sum (\xi_weigt - \xi_trans_weigt)

  RealVector w(numFullspaceVars);
  RealVector ratio_w(numFullspaceVars);
  for (i=0; i<numFullspaceVars; i++)
  {
    // Reduced dimension = i
    // extend the reduced eta variables by zeros to make it full dimensional
    RealMatrix eta_extd(n, nsamples); // init to 0
    for (j=0; j<i+1; j++){
      for (k=0; k<nsamples; k++)
          eta_extd(j, k) = eta(j, k);
    }

    // matrix multiplication: \hat \xi = A^T * \eta_{ex}
    RealMatrix xi_trans(numFullspaceVars, nsamples, false);
    teuchos_blas.GEMM(Teuchos::TRANS, Teuchos::NO_TRANS, numFullspaceVars, nsamples, numFullspaceVars,
                      alpha, A.values(), numFullspaceVars, eta_extd.values(), numFullspaceVars,
                      beta, xi_trans.values(), numFullspaceVars);

    // compute the weighted \xi and \xi_{trans}
    // compute the 2 norm of (\xi_weigt - \xi_trans_weigt)
    // compute w = 1/N sum( || \xi_weigt - \xi_trans_weigt || )
    RealMatrix xi_weigt(numFullspaceVars, nsamples, false);
    RealMatrix xi_trans_weigt(numFullspaceVars, nsamples, false);
    for (k=0; k< nsamples; k++){
      double norm_xi_diff_sq = 0.;
      for (j=0; j < numFullspaceVars; ++j){
          xi_weigt(j, k) = xi(j,k) * A(0,j);
          xi_trans_weigt(j, k) = xi_trans(j,k) * A(0,j);
          norm_xi_diff_sq += pow(xi_weigt(j, k)-xi_trans_weigt(j, k), 2);
        }
      double norm_xi_diff = sqrt(norm_xi_diff_sq);
      w(i) += norm_xi_diff;
    }
    w(i) /= nsamples;
  }
  Cout << "\nDimension reduction metric by first order info: \n" << w << std::endl;
  
  // compute the cumulative difference of w
  RealVector w_diff(numFullspaceVars);
  for (j=1; j<numFullspaceVars; j++)
    w_diff(j) = w(j-1)-w(j);
  RealVector cumsum_w_diff(numFullspaceVars);
  for (j=1; j<numFullspaceVars; j++){
    cumsum_w_diff(j) = cumsum_w_diff(j-1) + w_diff(j);
    ratio_w(j) = cumsum_w_diff(j) / w(0);
  }
  Cout << "\nRatio of the metric: \n" << ratio_w << std::endl;

  for (j=0; j<numFullspaceVars; j++){
    if (ratio_w(j) >= threshold_ratio) {
      reducedRank = j+1;
      break;
    }
  }

}



/** Define the distribution of recast reduced dimension
    variables \eta. They are standard Gaussian in adapted 
    basis model. */
void AdaptedBasisModel::uncertain_vars_to_subspace()
{
  // ----------------------------------
  // initialization of base RecastModel
  // ----------------------------------
  initialize_base_recast(variables_mapping, SubspaceModel::set_mapping,
			 SubspaceModel::response_mapping);

  // -------------
  // Resize mvDist
  // -------------
  SubspaceModel::uncertain_vars_to_subspace();

  // -----------------------
  // Extract full space data
  // -----------------------

  // -------------------------
  // Define reduced space data
  // -------------------------

  // reduced space characterization: mean mu, std dev sd
  RealVector mu_y(reducedRank), sd_y(reducedRank);
  for (int i=0; i<reducedRank; ++i)
    { mu_y(i) = 0.;  sd_y(i) = 1.; }
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "\nAdapted Basis Model: mu_y =\n" << mu_y
	 << "\nAdapted Basis Model: sd_y =\n" << sd_y;

  std::shared_ptr<Pecos::MarginalsCorrDistribution> reduced_dist_rep =
    std::static_pointer_cast<Pecos::MarginalsCorrDistribution>
    (mvDist.multivar_dist_rep());
  reduced_dist_rep->push_parameters(Pecos::NORMAL, Pecos::N_MEAN,    mu_y);
  reduced_dist_rep->push_parameters(Pecos::NORMAL, Pecos::N_STD_DEV, sd_y);

  // Set continuous variable types:
  UShortMultiArray cv_types(boost::extents[reducedRank]);
  for (int i = 0; i < reducedRank; i++)
    cv_types[i] = NORMAL_UNCERTAIN;
  currentVariables.continuous_variable_types(
    cv_types[boost::indices[idx_range(0, reducedRank)]]);

  // Set currentVariables to means of active variables:
  continuous_variables(mu_y);

  // update variable labels for adapted basis
  StringMultiArray adapted_basis_var_labels(boost::extents[reducedRank]);
  for (int i = 0; i < reducedRank; i++)
    adapted_basis_var_labels[i]
      = "abv_" + std::to_string(i+1);
  continuous_variable_labels(
    adapted_basis_var_labels[boost::indices[idx_range(0, reducedRank)]]);
}


/** Perform the variables mapping from recast reduced dimension
    variables \eta to original model \xi variables via linear
    transformation.  Maps only continuous variables. */
void AdaptedBasisModel::
variables_mapping(const Variables& reduced_vars, Variables& full_vars)
{
  const RealVector& eta = reduced_vars.continuous_variables();
  RealVector&        xi =    full_vars.continuous_variables_view();

  const RealMatrix& A = smInstance->reduced_basis();
  int m = A.numRows(), n = A.numCols(), incx = 1, incy = 1;
  Real alpha = 1.0, beta = 0.0;
  // expand \eta with zeros
  int dim_eta = eta.length();
  RealVector eta_ex(n); // init to 0
  for (int i=0; i<dim_eta; i++)
    eta_ex(i) = eta(i);
  //  y <-- alpha*A*x + beta*y
  //  Calculate \xi = 1.0 * A^T \eta + + 0.0 * \xi
  Teuchos::BLAS<int, Real> teuchos_blas;
  teuchos_blas.GEMV(Teuchos::TRANS, m, n, alpha, A.values(), m,
                    eta_ex.values(), incy, beta, xi.values(), incx);

  if (smInstance->output_level() >= DEBUG_OUTPUT)
    Cout <<   "\nAdapted Basis Model: Subspace vars are\n"  << reduced_vars
	 << "\n\nAdapted Basis Model: Fullspace vars are\n" << full_vars
	 << std::endl;
}

}  // namespace Dakota
