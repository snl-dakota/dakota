/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ActiveSubspaceModel.hpp"
#include "NonDLHSSampling.hpp"
#include "BootstrapSampler.hpp"
#include "dakota_linear_algebra.hpp"
#include "ParallelLibrary.hpp"

namespace Dakota {

/// initialization of static needed by RecastModel
ActiveSubspaceModel* ActiveSubspaceModel::asmInstance(NULL);

/** An ActiveSubspaceModel will be built over all functions, without
    differentiating primary vs. secondary constraints.  However the
    associated RecastModel has to differentiate. Currently identifies
    subspace for continuous variables only, but carries other active
    variables along for the ride. */
ActiveSubspaceModel::
ActiveSubspaceModel(const Model& sub_model, 
		    int random_seed, int initial_samples, int batch_size,
		    double conv_tol, size_t max_iter, size_t max_evals):
  RecastModel(sub_model), randomSeed(random_seed), 
  initialSamples(initial_samples), batchSize(batch_size), 
  maxIterations(max_iter), maxFunctionEvals(max_evals),
  svTol(std::max(conv_tol, std::numeric_limits<Real>::epsilon())),
  performAssessment(false), nullspaceTol(conv_tol/1.0e3),
  subspaceIDMethod(SUBSPACE_ID_CONSTANTINE), numReplicates(100),
  numFullspaceVars(sub_model.cv()), numFunctions(sub_model.num_functions()),
  currIter(0), totalSamples(0), totalEvals(0), svRatio(0.0),  reducedRank(0),
  gradientScaleFactors(RealArray(numFunctions, 1.0))
{
  asmInstance = this;
  
  // BMA TODO: probably want to do numerical derivatives in the
  // smaller subspace, not in the subModel space...
  //supportsEstimDerivs = true;

  // We can't even initialize the RecastModel sizes until after the
  // build has completed.  Can only construct the fullspace sampler.

  // initialize the fullspace Monte Carlo derivative sampler; this
  // will configure it to perform initialSamples
  init_fullspace_sampler();
}


ActiveSubspaceModel::~ActiveSubspaceModel()
{  /* empty dtor */  }


/** May eventually take on init_comms and related operations.  Also
    may want ide of build/update like DataFitSurrModel, eventually. */
void ActiveSubspaceModel::initialize()
{
  // runtime operation to identify the subspace model (if not later
  // returning to update the subpsace)
  identify_subspace();

  // complete initialization of the base RecastModel
  initialize_recast();

  // TODO: generalize to other distribution types
  // convert the normal distributions to the reduced space and set in the
  // reduced model
  uncertain_vars_to_subspace();

  // update with subspace constraints
  update_linear_constraints();
}


void ActiveSubspaceModel::init_fullspace_sampler()
{
  // use Monte Carlo due to iterative growth process
  unsigned short sample_type = SUBMETHOD_RANDOM;
  std::string rng; // use default random number generator

  // configure this sampler initially to work with initialSamples
  NonDLHSSampling* ndlhss =
    new NonDLHSSampling(subModel, sample_type, initialSamples, randomSeed,
                        rng, true);

  fullspaceSampler.assign_rep(ndlhss, false);

  // BMA: shouldn't need if this is modular on the inbound model...
  // if (transformVars)
  //   ndlhss->initialize_random_variables(natafTransform); // shallow copy

  // TODO: review whether this is needed
  fullspaceSampler.sub_iterator_flag(true);
}


/**  This specialization is because the model is used in multiple
     contexts in this iterator, depending on build phase.  Note that
     this overrides the default behavior at Iterator which recurses
     into any submodels. */
void ActiveSubspaceModel::
derived_init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			   bool recurse_flag)
{
  // In subspace identification, the same model will be used in multiple contexts:
  //  - fullspaceSampler(subModel) with initialSamples
  //  - fullspaceSampler(subModel) with batchSamples
  //  - direct compute_response() of verif_samples, one at a time

  // The fullspace sampler will be used for both initialSamples and batchSamples

  // BMA: taken from DataFitSurrModel daceIterator; is this correct?
  // init comms for daceIterator
  if (recurse_flag) {
    size_t method_index = probDescDB.get_db_method_node(); // for restoration
    probDescDB.set_db_list_nodes(fullspaceSampler.method_id());
    fullspaceSampler.init_communicators(pl_iter);
    
    // batch additions support concurrency up to batchSize * model concurrency
    int batch_concurrency = batchSize * subModel.derivative_concurrency();
    subModel.init_communicators(pl_iter, batch_concurrency);

    probDescDB.set_db_method_node(method_index); // restore method only
  }
}


void ActiveSubspaceModel::
derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			  bool recurse_flag)
{
  miPLIndex = modelPCIter->mi_parallel_level_index(pl_iter);// run time setting

  if (recurse_flag)
    fullspaceSampler.set_communicators(pl_iter);
}


void ActiveSubspaceModel::
derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			   bool recurse_flag)
{
  if (recurse_flag) {
    fullspaceSampler.free_communicators(pl_iter);

    // batch additions support concurrency up to batchSize * model concurrency
    int batch_concurrency = batchSize * subModel.derivative_concurrency();
    subModel.free_communicators(pl_iter, batch_concurrency);
  }
}


void ActiveSubspaceModel::identify_subspace()
{
  Cout << "ESM: Performing sampling to build reduced space" << std::endl;

  // whether singular value tolerance met
  bool svtol_met = false;
  // whether reconstruction error tolerance met
  bool recon_tol_met = false;

  // iterate until numerically singular or small reconstruction error
  while (!svtol_met && !recon_tol_met &&
         currIter < maxIterations && totalEvals < maxFunctionEvals &&
         reducedRank < numFullspaceVars) {

    // Run the inner iteration until user tol met (if possible), then check
    // reconstruction error, then continue outer if not tight enough
    if (currIter == 0) {

      // initially do this loop until tolerance met
      // once met (svTol will always be met),
      while (!svtol_met && currIter < maxIterations && 
             totalEvals < maxFunctionEvals
             && reducedRank < numFullspaceVars) {

        ++currIter;  // any addition of batch of points counts as an iteration
        Cout << "\nESM: Iteration " << currIter << "." << std::endl;
        expand_basis(svtol_met);
        print_svd_stats();

      } // until SVD converged

      if (svtol_met) {
        Cout << "\nESM: SVD converged to tolerance.\n     Proceeding to "
             << "reconstruction with reduced rank = " << reducedRank << "."
             << std::endl;
      }
      else {
        Cout << "\nESM: SVD not converged within budget.";
        if (currIter >= maxIterations)
          Cout << "\n     Maximum iterations reached.";
        if (totalEvals >= maxFunctionEvals)
          Cout << " \n    Maximum function evaluations reached.";
        Cout << "\n    Proceeding to reconstruction with reducedRank = "
             << reducedRank << std::endl;
      }

    }
    else {
      ++currIter;  // any addition of batch of points counts as an iteration
      Cout << "\nESM: Iteration " << currIter << "." << std::endl;
      expand_basis(svtol_met);
      print_svd_stats();
    }

    // update the reducedBasis
    // the reduced basis is dimension N x r and stored in the first r
    // cols of leftSingularVectors; extract it instead of using BLAS directly
    RealMatrix reduced_basis_U(Teuchos::View, leftSingularVectors,
                               numFullspaceVars, reducedRank);
    reducedBasis = reduced_basis_U;
    if (outputLevel >= DEBUG_OUTPUT) {
      Cout << "\nESM: Reduced basis is";
      reducedBasis.print(Cout);
    }

    // evaluate the fidelity of the reconstruction via orthogonal subspace
    // evaluations (constrained to stay in bounds)
    if (performAssessment) {
      assess_reconstruction(recon_tol_met);
    }
    else {
      // Bypass tolerance check if no reconstruction:
      recon_tol_met = true;
      Cout << "\nESM: Reconstruction has been bypassed. performAssessment = false" << std::endl;
    }

    if (!recon_tol_met)
      Cout << "\nESM: Reconstruction tolerance not met." << std::endl;

  } // until reconstruction converged

  Cout << "\n --- ESM Final Build Statistics ---"
       << "\n  total iterations: " << currIter
       << "\n  small/large singular value: " << svRatio
       << "\n  build samples: " << totalSamples
       << "\n  reduced rank: " << reducedRank
       << std::endl;

  Cout << "\n --- ESM Build Convergence Criteria ---"
       << "\n  tolerance on SVD met?: " << svtol_met;
  if(performAssessment) {
    Cout << "\n  reconstruction tolerance met?: " << recon_tol_met;
  }
  else {    
    Cout << "\n  reconstruction tolerance met?: bypassed";
  }
  Cout << "\n  max_iterations reached: " << (bool) (currIter >= maxIterations)
       << "\n  max_evals reached: " << (bool) (totalEvals >= maxFunctionEvals)
       << "\n  max rank reached: " << (bool) (reducedRank >= numFullspaceVars)
       << std::endl;


}




void ActiveSubspaceModel::
expand_basis(bool& svtol_met)
{
  // determine number of points to add
  unsigned int diff_samples = calculate_fullspace_samples();
  totalSamples += diff_samples;
  totalEvals += diff_samples;

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "ESM: Adding " << diff_samples << " full-space samples."
         << std::endl;

  // evaluate samples with fullspaceSampler
  generate_fullspace_samples(diff_samples);

  // add the generated points to the matrices
  append_sample_matrices(diff_samples);

  // factor the derivative matrix and estimate the information content
  compute_svd(svtol_met);
}


unsigned int ActiveSubspaceModel::calculate_fullspace_samples()
{
  // on first pass, use initial build samples
  if (currIter == 1)
    return initialSamples;

  unsigned int diff_samples;

  // for now only supporting growth by fixed batch size
  diff_samples = batchSize;

  // TODO: allow options for fullspace sample growth based on
  // fixed user-supplied factor, logarithmic growth

  // factor by which to increase number samples at each minor iteration
  // therefore also each major iteration (> 1.0) (not used for now)
  //double sample_growth_factor = 2.0;
  //double desired_total_samples = sample_growth_factor*totalSamples;
  //diff_samples =
  //  (unsigned int) std::ceil(desired_total_samples) - totalSamples;

  // limit the increment by maximum evaluations control
  diff_samples = std::min(diff_samples, maxFunctionEvals - totalEvals);

  return diff_samples;
}


void ActiveSubspaceModel::
generate_fullspace_samples(unsigned int diff_samples)
{
  // Rank-revealing phase requires derivatives (for now)
  ActiveSet dace_set = fullspaceSampler.active_set(); // copy
  // TODO: always gradients only; no functions
  //       analysis_driver needs to parse active_set
  unsigned short request_value = 3;
  dace_set.request_values(request_value);
  fullspaceSampler.active_set(dace_set);

  // Generate the samples.  Have to adjust the base number of samples
  // with sampling_reference() since the number of samples may go down
  // from intialSamples to batchSamples
  fullspaceSampler.sampling_reference(diff_samples);
  fullspaceSampler.sampling_reset(diff_samples, true, false);

  // BMA: shouldn't need this anymore as the fullspaceSampler should
  // call set as needed?

  // select the right comm for current model evaluation concurrency
  // (initial build or batch update)
  // ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
  // if (currIter == 1)
  //   subModel.set_communicators(pl_iter, maxEvalConcurrency);
  // else {
  //   int batch_concurrency = batchSize * subModel.derivative_concurrency();
  //   subModel.set_communicators(pl_iter, batch_concurrency);
  // }

  // and generate the additional samples
  ParLevLIter pl_iter = modelPCIter->mi_parallel_level_iterator(miPLIndex);
  fullspaceSampler.run(pl_iter);
}


void ActiveSubspaceModel::
append_sample_matrices(unsigned int diff_samples)
{
  // extract into a matrix
  // all_samples vs. all_variables
  const RealMatrix& all_vars = fullspaceSampler.all_samples();
  const IntResponseMap& all_responses = fullspaceSampler.all_responses();

  // TODO: could easily filter NaN/Inf responses and omit
  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "\nESM: Iteration " << currIter << ". DACE iterator returned "
         << all_responses.size() << " samples. (Expected diff_samples = "
         << diff_samples << ".)" << std::endl;
  }

  int sample_insert_point = varsMatrix.numCols();
  derivativeMatrix.reshape(numFullspaceVars,
                           totalSamples*numFunctions);
  varsMatrix.reshape(numFullspaceVars, totalSamples);

  unsigned int diff_sample_ind = 0;
  IntRespMCIter resp_it  = all_responses.begin();
  IntRespMCIter resp_end = all_responses.end();

  // Compute gradient scaling factors if more than 1 response function
  // and only on the first iteration
  if(numFunctions > 1 && currIter == 1) {
    for ( ; resp_it != resp_end ; ++resp_it, ++diff_sample_ind) {
      const RealVector& resp_vector = resp_it->second.function_values();
      for (unsigned int fn_ind = 0; fn_ind < numFunctions; ++fn_ind) {
        gradientScaleFactors[fn_ind] += resp_vector(fn_ind) / 
          static_cast<Real>(diff_samples);
      }
    }
  }

  // Reset iterators and indices
  diff_sample_ind = 0;
  resp_it  = all_responses.begin();
  resp_end = all_responses.end();

  for ( ; resp_it != resp_end ; ++resp_it, ++diff_sample_ind) {
    // the absolute sample number to insert into
    unsigned int sample_ind = sample_insert_point + diff_sample_ind;
    // matrix of num_variables x num_functions
    const RealMatrix& resp_matrix = resp_it->second.function_gradients();
    for (unsigned int fn_ind = 0; fn_ind < numFunctions; ++fn_ind) {
      unsigned int col_ind = sample_ind * numFunctions + fn_ind;
      for (unsigned int var_ind = 0; var_ind < numFullspaceVars; ++var_ind) {
        derivativeMatrix(var_ind, col_ind) = resp_matrix(var_ind, fn_ind) / 
          gradientScaleFactors[fn_ind];
      }
    }
    for (unsigned int var_ind = 0; var_ind < numFullspaceVars; ++var_ind) {
      varsMatrix(var_ind, sample_ind) = all_vars(var_ind, diff_sample_ind);
    }
  }

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "\nESM: Iteration " << currIter
         << ". Compiled derivative matrix is:\n";
    derivativeMatrix.print(Cout);
    Cout << std::endl;
  }
}


void ActiveSubspaceModel::
compute_svd(bool& svtol_met)
{
  // Want eigenvalues of derivMatrix*derivMatrix^T, so perform SVD of
  // derivMatrix and square them

  RealVector singular_values;
  RealMatrix V_transpose;
  leftSingularVectors = derivativeMatrix;
  svd(leftSingularVectors, singular_values, V_transpose);

  // TODO: Analyze whether we need to worry about this
  if(singular_values.length() == 0)
  {
    Cerr << "No computed singular values available!" << std::endl;
    abort_handler(-1);
  }

  // TODO: Analyze whether we need this check and can have differing numbers
  // of singular values returned
  if(derivativeMatrix.numRows() != singular_values.length())
  {
    Cerr << "Number of computed singular_values does not match the dimension "
            "of the space of gradient samples! Logic not currently supported!"
         << std::endl;
    abort_handler(-1);
  }

  // Identify the active subspace using one of the methods below:
  switch(subspaceIDMethod) {
  case SUBSPACE_ID_CONSTANTINE:
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "ESM: Determining eigenvalue gap with boostrap and Constantine "
	   << "metric." << std::endl;
    computeConstantineMetric(singular_values, svtol_met);
    break;
  case SUBSPACE_ID_BING_LI: default:
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "ESM: Determining eigenvalue gap with boostrap and Bing-Li "
	   << "metric." << std::endl;
    computeBingLiCriterion(singular_values, svtol_met);
    break;
  }

  int num_singular_values = singular_values.length();

  // Compute ratio of largest singular value not in active subspace to
  // largest singular value in active subspace:
  int sv_cutoff_ind = (reducedRank < num_singular_values)?(reducedRank):(num_singular_values-1);
  svRatio = singular_values[sv_cutoff_ind]/singular_values[0];

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "\nESM: Iteration " << currIter << ": singular values are [ ";
    for (unsigned int i=0; i<num_singular_values; ++i)
      Cout << singular_values[i] << " ";
    Cout << "]" << std::endl;
  }
}

void ActiveSubspaceModel::
computeBingLiCriterion(RealVector& singular_values, bool& svtol_met)
{
  int num_vars = derivativeMatrix.numRows();

  // Stores Bing Li's criterion
  std::vector<RealMatrix::scalarType> bing_li_criterion(num_vars, 0);

  // Compute part 1 of criterion: relative energy in next eigenvalue in the
  // spectrum

  RealMatrix::scalarType eigen_sum = 0.0;
  for(size_t i = 0; i < num_vars; ++i)
  {
    RealMatrix::scalarType eigen_val = singular_values[i] *
                                       singular_values[i];
    bing_li_criterion[i] = eigen_val;
    eigen_sum += eigen_val;
  }

  for(size_t i = 0; i < num_vars; ++i)
    bing_li_criterion[i] /= eigen_sum;

  // Compute part 2 of criterion: bootstrapped determinant metric

  RealMatrix bootstrapped_sample(num_vars, derivativeMatrix.numCols());
  RealVector sample_sing_vals;
  RealMatrix sample_sing_vectors;

  Teuchos::LAPACK<RealMatrix::ordinalType, RealMatrix::scalarType> lapack;

  std::vector<RealMatrix::scalarType> bootstrapped_determinants(num_vars);

  BootstrapSampler<RealMatrix> bootstrap_sampler(derivativeMatrix,
    numFunctions);

  // TODO: Get number of bootstrap samples from problem desc database
  for (size_t i = 0; i < numReplicates; ++i)
  {
    bootstrap_sampler(bootstrapped_sample);

    svd(bootstrapped_sample, sample_sing_vals, sample_sing_vectors);

    // Overwrite bootstrap replicate with singular matrix product
    bootstrapped_sample.multiply(Teuchos::TRANS, Teuchos::NO_TRANS, 1.0,
                                 leftSingularVectors, bootstrapped_sample, 0.0);

    for(size_t j = 1; j < num_vars; ++j)
    {
      size_t num_sing_vec = j;

      RealMatrix submatrix(Teuchos::Copy, bootstrapped_sample, num_sing_vec,
                           num_sing_vec);

      // Get determinant from LU decomposition

      Teuchos::SerialDenseVector<RealMatrix::ordinalType,
              RealMatrix::ordinalType> pivot(num_sing_vec);
      RealMatrix::ordinalType info;

      lapack.GETRF(num_sing_vec, num_sing_vec, submatrix.values(),
                   num_sing_vec, pivot.values(), &info);

      RealMatrix::scalarType det = 1.0;
      for (size_t i = 0; i < j; ++i)
      {
        det *= submatrix(i,i);
      }

      bootstrapped_determinants[j] += std::abs(det);
    }
  }

  RealMatrix::scalarType det_sum = 0.0;
  bootstrapped_determinants[0] = 0.0;
  for (size_t i = 1; i < num_vars; ++i)
  {
    bootstrapped_determinants[i] = 1.0 - bootstrapped_determinants[i] /
                                   static_cast<RealMatrix::scalarType>(numReplicates);
    det_sum += bootstrapped_determinants[i];
  }

  for (size_t i = 0; i < num_vars; ++i)
  {
    bing_li_criterion[i] += bootstrapped_determinants[i] / det_sum;
  }

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "\nESM: Iteration " << currIter
         << ". Bing Li Criterion values are [ ";
    for (size_t i = 0; i < num_vars; ++i)
    {
      Cout << bing_li_criterion[i] << " ";
    }
    Cout << "]" << std::endl;
  }

  // Cutoff is 1st minimum of the criterion
  reducedRank = 0;
  for (size_t i = 1; i < num_vars; ++i)
  {
    if(bing_li_criterion[i-1] < bing_li_criterion[i])
    {
      svtol_met = true;
      reducedRank = i-1;
      break;
    }
  }
}

void ActiveSubspaceModel::
computeConstantineMetric(RealVector& singular_values, bool& svtol_met)
{
  int num_vars = derivativeMatrix.numRows();

  // Stores Constantine's metric
  RealArray constantine_metric(num_vars-1, 0);

  // Compute bootstrapped subspaces
  RealMatrix bootstrapped_sample(num_vars, derivativeMatrix.numCols());
  RealMatrix dist_mat(num_vars, num_vars);
  RealVector sample_sing_vals;
  RealMatrix sample_sing_vectors;
  RealVector dist_sing_vals;
  RealMatrix dist_sing_vectors;

  Teuchos::LAPACK<RealMatrix::ordinalType, RealMatrix::scalarType> lapack;

  BootstrapSampler<RealMatrix> bootstrap_sampler(derivativeMatrix,
    numFunctions);

  // TODO: Get number of bootstrap samples from problem desc database
  for (size_t i = 0; i < numReplicates; ++i)
  {
    bootstrap_sampler(bootstrapped_sample);

    svd(bootstrapped_sample, sample_sing_vals, sample_sing_vectors);

    for(size_t j = 0; j < num_vars-1; ++j)
    {
      size_t num_sing_vec = j+1;

      RealMatrix submatrix(Teuchos::View, leftSingularVectors, num_vars,
                           num_sing_vec);

      RealMatrix submatrix_bootstrap(Teuchos::View, bootstrapped_sample,
                                     num_vars, num_sing_vec);

      dist_mat.multiply(Teuchos::NO_TRANS, Teuchos::TRANS, 1.0,
                        submatrix, submatrix, 0.0);

      dist_mat.multiply(Teuchos::NO_TRANS, Teuchos::TRANS, -1.0,
                        submatrix_bootstrap, submatrix_bootstrap, 1.0);

      // The spectral norm is slow, let's use the Frobenius norm instead.
      // Compute the spectral norm of dist_mat (largest singular value):
      //svd(dist_mat, dist_sing_vals, dist_sing_vectors);
      //constantine_metric[j] += dist_sing_vals(0) / numReplicates;

      constantine_metric[j] += dist_mat.normFrobenius() / numReplicates;
    }
  }

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "\nESM: Iteration " << currIter
         << ". Constantine metric are [ ";
    for (size_t i = 0; i < num_vars-1; ++i)
    {
      Cout << constantine_metric[i] << " ";
    }
    Cout << "]" << std::endl;
  }

  // Cutoff is global minimum of metric
  reducedRank = 0;
  Real min_val = 0;
  svtol_met = true; // This just bypasses the tolerance check
  for (size_t i = 0; i < num_vars-1; ++i)
  {
    if(constantine_metric[i] < min_val || i == 0)
    {
      min_val = constantine_metric[i];
      reducedRank = i+1;
    }
  }
}

void ActiveSubspaceModel::print_svd_stats()
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n --- ESM Iteration " << currIter << " Statistics --- "
         << "\n  small/large singular value: " << svRatio
         << "\n  build samples: " << totalSamples
         << "\n  estimated reduced rank: " << reducedRank
         << std::endl;
}


/** This function is experimental and needs to be carefully reviewed
    and cleaned up */
void ActiveSubspaceModel::
assess_reconstruction(bool& recon_tol_met)
{
  // calculate upper bound on number of verification samples
  // allow maxFunctionEvals to be exceeded
  // TODO: likely shouldn't do more than N-reducedRank samples?
  // TODO: Could generate additional random samples in the perp space
  unsigned int wilks_max_samples = 100; // maximum based on Wilk's formula
  unsigned int verif_samples = std::min(wilks_max_samples, totalSamples);
  if (verif_samples > maxFunctionEvals - totalEvals) {
    verif_samples = maxFunctionEvals - totalEvals;
    Cout << "\nESM: Warning: " << verif_samples << " verification samples "
         << "desired, but budget only permits " << verif_samples << std::endl;
  }
  totalEvals += verif_samples;

  // Calculate perturbations of the samples from nominal and project
  // orthogonal to reducedBasis U

  // TODO: Relies on normal distribution for now
  const RealVector& nominal_vars =
    subModel.aleatory_distribution_parameters().normal_means();

  // Find vectors orthogonal to each initial perturbation
  // for test problem, nominal is 0.5 for all x
  // TODO: use actual nominal values

  RealMatrix perturbations(numFullspaceVars, verif_samples);
  for (int i=0; i<numFullspaceVars; ++i)
    for (int j=0; j<verif_samples; ++j)
      perturbations(i,j) = varsMatrix(i,j) - nominal_vars(i);


  // want to compute perturbations delta_x_perp in the nullspace of
  // U^T, i.e.,
  //   delta_x_perp = (I - U*U^T)*delta_x = delta_x - U*U^T*delta_x

  // compute U^T * delta_x via
  // u_trans_delta_x = 1.0 * U^T * delta_x + 0.0 * u_trans_delta_x
  RealMatrix u_trans_delta_x(reducedRank, verif_samples); // init to zero

  Real alpha = 1.0;
  Real beta = 0.0;
  u_trans_delta_x.multiply(Teuchos::TRANS, Teuchos::NO_TRANS, alpha,
                           reducedBasis, perturbations, beta);

  // compute -1.0 * ( U * u_trans_delta_x ) + 1.0 * delta_x
  alpha = -1.0;
  beta = 1.0;
  perturbations.multiply(Teuchos::NO_TRANS, Teuchos::NO_TRANS, alpha,
                         reducedBasis, u_trans_delta_x, beta);

  // the matrix perturbations now contains the delta_x_perp
  // NEED to evaluate at nomimal + delta_x_perp

  // evaluate the model at the sample points in perturbations
  // for now in serial
  // TODO: initialize to nominal
  RealMatrix perp_points(numFullspaceVars, verif_samples, false);
  for (int i=0; i<numFullspaceVars; ++i)
    for (int j=0; j<verif_samples; ++j)
      perp_points(i,j) = nominal_vars(i);
  perp_points += perturbations;

  // TODO: restrict perp_points to the variable bounds

  // Reconstruction phase requires only functions
  // TODO: get off the model?
  //   ActiveSet recon_set = dace_iterator.active_set(); // copy
  //   unsigned short request_value = 1;
  //   recon_set.request_values(request_value);
  ActiveSet active_set = subModel.current_response().active_set();
  active_set.request_values(1);
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\nESM: Evaluating at nominal, and at " << verif_samples
         << " points orthogonal to the subspace" << std::endl;

  // evaluate model at nominal values
  subModel.continuous_variables(nominal_vars);
  subModel.compute_response(active_set);
  const RealVector& ynominal =
    subModel.current_response().function_values();

  // TODO: use array of RealVectors here

  // TODO: convenience function to evaluate a matrix of samples
  // TODO: asynch as well
  RealMatrix Kmat(verif_samples, numFunctions);
  for (int j=0; j<verif_samples; ++j) {
    subModel.continuous_variables(getCol(Teuchos::View, perp_points, j));
    subModel.compute_response(active_set);
    // compute y(perp) - y(nominal)
    RealVector deviation(subModel.current_response().function_values());
    deviation -= ynominal;
    Teuchos::setCol(deviation, j, Kmat);
  }

  recon_tol_met = true;
  RealVector recon_error(numFunctions);
  for (int i=0; i<numFunctions; ++i) {

    RealVector Kvec = Teuchos::getCol(Teuchos::View, Kmat, i);

    // two-norm of recon error for now
    double recon_error = std::sqrt(Kvec.dot(Kvec)/verif_samples);

    // standard deviation of Kvec
    double K_mu = std::accumulate(&Kvec[0], &Kvec[0]+verif_samples, 0.0) /
                  (Real) verif_samples;
    double K_sigma = 0.0;
    // Could decimate Kvec with (K-mu)^2; opt for simplicity
    for (unsigned int j=0; j<verif_samples; ++j) {
      K_sigma += (Kvec(j) - K_mu)*(Kvec(j) - K_mu);
    }
    K_sigma = std::sqrt(K_sigma / (Real) (verif_samples-1));

    Cout << "\nESM: Reconstruction statistics over " << verif_samples
         << " samples for function " << i << ":\n  K_sigma = " << K_sigma
         << "\n  K_mu = " << K_mu
         << "\n  L2 (recon) error = " << recon_error << std::endl;

    // if any function violates, continue process
    if (recon_error > nullspaceTol)
      recon_tol_met = false;
  }
}




/** Initialize the recast model based on the reduced space, with no
    response function mapping (for now).  TODO: use a surrogate model
    over the inactive dimension. */
void ActiveSubspaceModel::initialize_recast()
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

  // Initial response order for the newly built subspace model same as
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
	      nonlinear_resp_mapping, NULL, NULL);
}

/// Create a variables components totals array with the reduced space
/// size for continuous variables
SizetArray ActiveSubspaceModel::variables_resize()
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
      Cerr << "\nError: invalid active variables view " << active_view 
	   << " in ActiveSubspaceModel.\n";
      abort_handler(-1);
      break;

    }
  
  }
  return vc_totals;
}


void ActiveSubspaceModel::update_linear_constraints()
{

}



/** Convert the user-specified normal random variables to the
    appropriate reduced space variables, based on the orthogonal
    transformation.

    TODO: Generalize to convert other random variable types (non-normal)
*/
/// transform and set the distribution parameters in the reduced model
void ActiveSubspaceModel::uncertain_vars_to_subspace()
{
  const Pecos::AleatoryDistParams& native_params =
    subModel.aleatory_distribution_parameters();

  // native space characterization
  const RealVector& mu_x = native_params.normal_means();
  const RealVector& sd_x = native_params.normal_std_deviations();
  const RealSymMatrix& correl_x = native_params.uncertain_correlations();

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "\ncorrel_x = \n" << correl_x;

  bool native_correl = correl_x.empty() ? false : true;
  if (native_correl && correl_x.numRows() != numFullspaceVars) {
    Cerr << "Wrong correlation size";
    abort_handler(-1);
  }

  // reduced space characterization: mean mu, std dev sd
  RealVector mu_xi(reducedRank), sd_xi(reducedRank);


  // xi_mu = reducedBasis^T * x_mu
  int m = reducedBasis.numRows();
  int n = reducedBasis.numCols();
  double alpha = 1.0;
  double beta = 0.0;

  int incx = 1;
  int incy = 1;

  // y <-- alpha*A*x + beta*y
  // mu_xi <-- 1.0 * reducedBasis^T * mu_x + 0.0 * mu_xi
  Teuchos::BLAS<int, Real> teuchos_blas;
  teuchos_blas.GEMV(Teuchos::TRANS, m, n, alpha, reducedBasis.values(), m,
                    mu_x.values(), incx, beta, mu_xi.values(), incy);

  // convert the correlations C_x to variance V_x
  // V_x <-- diag(sd_x) * C_x * diag(sd_x)
  // not using symmetric so we can multiply() below
  RealMatrix V_x(reducedBasis.numRows(), reducedBasis.numRows(), false);
  if (native_correl) {
    for (int row=0; row<reducedBasis.numRows(); ++row)
      for (int col=0; col<reducedBasis.numRows(); ++col)
        V_x(row, col) = sd_x(row)*correl_x(row,col)*sd_x(col);
  }
  else {
    V_x = 0.0;
    for (int row=0; row<reducedBasis.numRows(); ++row)
      V_x(row, row) = sd_x(row)*sd_x(row);
  }


  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "\nreducedBasis = \n" << reducedBasis;
    Cout << "\nV_x =\n" << V_x;
  }

  // compute V_xi = U^T * V_x * U
  alpha = 1.0;
  beta = 0.0;
  RealMatrix UTVx(n, m, false);
  UTVx.multiply(Teuchos::TRANS, Teuchos::NO_TRANS, alpha, reducedBasis, V_x, beta);
  RealMatrix V_xi(reducedRank, reducedRank, false);
  V_xi.multiply(Teuchos::NO_TRANS, Teuchos::NO_TRANS, alpha, UTVx, reducedBasis, beta);

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "\nV_xi = \n" << V_xi;

  // compute the standard deviations in reduced space
  for (int i=0; i<reducedRank; ++i)
    sd_xi = std::sqrt(V_xi(i,i));

  // update the reduced space model
  Pecos::AleatoryDistParams& reduced_dist_params =
    aleatory_distribution_parameters();

  reduced_dist_params.normal_means(mu_xi);
  reduced_dist_params.normal_std_deviations(sd_xi);


  // compute the correlations in reduced space
  // TODO: fix symmetric access to not loop over whole matrix
  //  if (native_correl) {

  // Unless the native correl was alpha*I, the reduced variables will
  // be correlated in general, so always set the correltions
  RealSymMatrix correl_xi(reducedRank, false);
  for (int row=0; row<reducedRank; ++row)
    for (int col=0; col<reducedRank; ++col)
      correl_xi(row, col) = V_xi(row,col)/sd_xi(row)/sd_xi(col);

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "\ncorrel_xi = \n" << correl_xi;

  reduced_dist_params.uncertain_correlations(correl_xi);
  //  }

  //   RealVector x_reduced(reducedRank);
  //   x_reduced = 1.0;

  //   Cout << "x_reduced is\n";
  //   Cout << x_reduced << std::endl;

  //   vars_transform_model.continuous_variables(x_reduced);
  //   vars_transform_model.compute_response();

}



/**
  Perform the variables mapping from recast reduced dimension
  variables xi to original model x variables via linear transformation.
  Maps only continuous variables.
*/
void ActiveSubspaceModel::
vars_mapping(const Variables& recast_xi_vars, Variables& sub_model_x_vars)
{
  Teuchos::BLAS<int, Real> teuchos_blas;

  const RealVector& xi = recast_xi_vars.continuous_variables();
  // TODO: does this yield a view or a copy?
  //RealVector x = sub_model_x_vars.continuous_variables();
  RealVector x;
  copy_data(sub_model_x_vars.continuous_variables(), x);

  //  Calculate x = reducedBasis*xi via matvec directly into x cv in submodel
  //void 	GEMV (ETransp trans, const OrdinalType m, const OrdinalType n, const alpha_type alpha, const A_type *A, const OrdinalType lda, const x_type *x, const OrdinalType incx, const beta_type beta, ScalarType *y, const OrdinalType incy) const
  // 	Performs the matrix-std::vector operation: y <- alpha*A*x+beta*y or y <- alpha*A'*x+beta*y where A is a general m by n matrix.
  const RealMatrix& reduced_basis = asmInstance->reducedBasis;
  int m = reduced_basis.numRows();
  int n = reduced_basis.numCols();

  double alpha = 1.0;
  double beta = 0.0;

  int incx = 1;
  int incy = 1;

  teuchos_blas.GEMV(Teuchos::NO_TRANS, m, n, alpha, reduced_basis.values(), m,
                    xi.values(), incx, beta, x.values(), incy);

  if (asmInstance->outputLevel >= DEBUG_OUTPUT) {
    Cout << "Recast vars are\n";
    Cout << recast_xi_vars << std::endl;
    //    xi.print(Cout);

    Cout << "Submodel vars are\n";
    Cout << sub_model_x_vars << std::endl;
    //x.print(Cout);
  }

  sub_model_x_vars.continuous_variables(x);

}


/** Simplified derivative variables mapping where all continuous
    depend on all others.  TODO: Could instead rely on a richer
    default in RecastModel based on varsMapIndices. */
void ActiveSubspaceModel::set_mapping(const Variables& recast_vars,
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
  size_t max_sm_id = asmInstance->subModel.cv();
  for (size_t i=0; i<recast_dvv.size(); ++i)
    if (1 <= recast_dvv[i] && recast_dvv[i] <= recast_cv) {
      for (size_t j=1; j<=max_sm_id; ++j)
	sub_model_dvv.push_back(j);
      break;
    }
  sub_model_set.derivative_vector(sub_model_dvv);  
}

}  // namespace Dakota


