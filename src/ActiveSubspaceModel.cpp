/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ActiveSubspaceModel.hpp"
#include "ProbabilityTransformModel.hpp"
#include "NonDLHSSampling.hpp"
#include "BootstrapSampler.hpp"
#include "dakota_linear_algebra.hpp"
#include "ParallelLibrary.hpp"
#include "DataFitSurrModel.hpp"

namespace Dakota
{

/// initialization of static needed by RecastModel
ActiveSubspaceModel* ActiveSubspaceModel::asmInstance(NULL);

// BMA TODO: Consider whether a DACE Iterator is justified; don't need
// the modularity yet, but a lot of the build controls better belong
// in a helper iterator specification.

ActiveSubspaceModel::ActiveSubspaceModel(ProblemDescDB& problem_db):
  RecastModel(problem_db, get_sub_model(problem_db)),
  randomSeed(24620),
  initialSamples(problem_db.get_int("model.initial_samples")),
  maxFunctionEvals(std::numeric_limits<int>::max()),
  subspaceIdBingLi(
    probDescDB.get_bool("model.active_subspace.truncation_method.bing_li")),
  subspaceIdConstantine(
    probDescDB.get_bool("model.active_subspace.truncation_method.constantine")),
  subspaceIdEnergy(
    probDescDB.get_bool("model.active_subspace.truncation_method.energy")),
  subspaceIdCV(
    probDescDB.get_bool("model.active_subspace.truncation_method.cv")),
  numReplicates(problem_db.get_int("model.active_subspace.bootstrap_samples")),
  numFullspaceVars(subModel.cv()), numFunctions(subModel.num_functions()),
  totalSamples(0), subspaceInitialized(false),
  reducedRank(problem_db.get_int("model.active_subspace.dimension")),
  gradientScaleFactors(RealArray(numFunctions, 1.0)),
  truncationTolerance(probDescDB.get_real("model.active_subspace.truncation_method.energy.truncation_tolerance")),
  buildSurrogate(probDescDB.get_bool("model.active_subspace.build_surrogate")),
  refinementSamples(0),
  subspaceNormalization(
    probDescDB.get_ushort("model.active_subspace.normalization")),
  cvIncremental(probDescDB.get_bool("model.active_subspace.cv.incremental")),
  cvIdMethod(probDescDB.get_ushort("model.active_subspace.cv.id_method")),
  cvRelTolerance(
    probDescDB.get_real("model.active_subspace.cv.relative_tolerance")),
  cvDecreaseTolerance(
    probDescDB.get_real("model.active_subspace.cv.decrease_tolerance")),
  cvMaxRank(problem_db.get_int("model.active_subspace.cv.max_rank"))
{
  asmInstance = this;
  modelType = "active_subspace";

  // Set seed of bootstrap sampler:
  BootstrapSamplerBase<RealMatrix>::set_seed(randomSeed);

  componentParallelMode = CONFIG_PHASE;
  offlineEvalConcurrency = initialSamples * subModel.derivative_concurrency();
  // Will be overwritten with correct value in derived_init_communicators():
  onlineEvalConcurrency = 1; 

  validate_inputs();

  // initialize the fullspace derivative sampler; this
  // will configure it to perform initialSamples
  init_fullspace_sampler(
    probDescDB.get_ushort("model.active_subspace.sample_type"));

  const IntVector& db_refine_samples =
    problem_db.get_iv("model.refinement_samples");
  if (db_refine_samples.length() == 1)
    refinementSamples = db_refine_samples[0];
  else if (db_refine_samples.length() > 1) {
    Cerr << "\nError (subspace model): refinement_samples must be "
         << "length 1 if specified." << std::endl;
    abort_handler(PARSE_ERROR);
  }
}


/** An ActiveSubspaceModel will be built over all functions, without
    differentiating primary vs. secondary constraints.  However the
    associated RecastModel has to differentiate. Currently identifies
    subspace for continuous variables only, but carries other active
    variables along for the ride. */
ActiveSubspaceModel::
ActiveSubspaceModel(const Model& sub_model, unsigned int dimension,
                    const RealMatrix &rotation_matrix,
                    short output_level) :
  RecastModel(sub_model),
  numFullspaceVars(sub_model.cv()), numFunctions(sub_model.num_functions()),
  subspaceInitialized(false), reducedRank(dimension),
  gradientScaleFactors(RealArray(numFunctions, 1.0)),
  buildSurrogate(false), refinementSamples(0),
  subspaceNormalization(SUBSPACE_NORM_DEFAULT)
{
  modelType = "active_subspace";

  outputLevel = output_level;

  componentParallelMode = CONFIG_PHASE;
  offlineEvalConcurrency = initialSamples * subModel.derivative_concurrency();
  // Will be overwritten with correct value in derived_init_communicators():
  onlineEvalConcurrency = 1; 

  RealMatrix reduced_basis_W1(Teuchos::View, rotation_matrix,
                              numFullspaceVars, reducedRank);
  activeBasis = reduced_basis_W1;

  RealMatrix reduced_basis_W2(Teuchos::View, rotation_matrix,
                              numFullspaceVars,numFullspaceVars - reducedRank,
                              0, reducedRank);

  inactiveBasis = reduced_basis_W2;

  // complete initialization of the base RecastModel
  initialize_recast();

  // convert the normal distributions to the reduced space and set in the
  // reduced model
  uncertain_vars_to_subspace();

  // update with subspace constraints
  update_linear_constraints();

  // set new subspace variable labels
  update_var_labels();

  if (buildSurrogate)
    build_surrogate();

  subspaceInitialized = true;

  // Perform numerical derivatives in subspace:
  supportsEstimDerivs = true;

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\nSubspace Model: Initialization of active subspace is complete."
         << std::endl;
}


ActiveSubspaceModel::~ActiveSubspaceModel()
{
  /* empty dtor */
}


Model ActiveSubspaceModel::get_sub_model(ProblemDescDB& problem_db)
{
  Model sub_model;

  const String& actual_model_pointer
    = problem_db.get_string("model.surrogate.actual_model_pointer");
  size_t model_index = problem_db.get_db_model_node(); // for restoration
  problem_db.set_db_model_nodes(actual_model_pointer);

  transformVars = true;

  if (transformVars) {
    sub_model.assign_rep(new ProbabilityTransformModel(problem_db.get_model()),
                         false);
  } else {
    sub_model = problem_db.get_model();
  }

  problem_db.set_db_model_nodes(model_index); // restore

  return sub_model;
}


void ActiveSubspaceModel::validate_inputs()
{
  bool error_flag = false;

  // set default initialSamples equal to 2
  int min_initial_samples = 2;
  if (initialSamples < min_initial_samples) {
    initialSamples = min_initial_samples;
    Cout << "\nWarning (subspace model): resetting samples to minimum "
         << "allowed = " << initialSamples << ". Note that the accuracy of the "
         << "subspace may be poor with this few samples.\n" << std::endl;
  }

  if (initialSamples > maxFunctionEvals) {
    error_flag = true;
    Cerr << "\nError (subspace model): build samples exceeds function "
         << "evaluation budget.\n" << std::endl;
  }

  // validate variables specification
  // BMA TODO: allow other variable types
  if (subModel.div() > 0 || subModel.dsv() > 0 || subModel.drv() > 0) {
    error_flag = true;
    Cerr << "\nError (subspace model): only normal uncertain variables are "
         << "supported;\n                        remove other variable "
         << "specifications.\n" << std::endl;
  }

  // validate response data
  if (subModel.gradient_type() == "none") {
    error_flag = true;
    Cerr << "\nError (subspace model): gradients are required;"
         << "\n                        Please select numerical, analytic "
         << "(recommended), or mixed gradients.\n" << std::endl;
  }

  if (error_flag)
    abort_handler(-1);
}


void ActiveSubspaceModel::build_subspace()
{
  Cout << "\nSubspace Model: Performing sampling to build reduced space"
       << std::endl;

  // determine number of points to add
  totalSamples += initialSamples;

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\nSubspace Model: Adding " << initialSamples
         << " full-space samples." << std::endl;

  // evaluate samples with fullspaceSampler
  generate_fullspace_samples(initialSamples);

  // populate the derivative and vars matrices with fullspaceSampler samples
  populate_matrices(initialSamples);

  // factor the derivative matrix using singular value decomposition
  compute_svd();

  // use the SVD results and the truncation methods to determine the size of the
  // active subspace
  identify_subspace();

  // update the activeBasis
  // the reduced basis is dimension N x r and stored in the first r
  // cols of leftSingularVectors; extract it instead of using BLAS directly
  RealMatrix reduced_basis_W1(Teuchos::View, leftSingularVectors,
                              numFullspaceVars, reducedRank);
  activeBasis = reduced_basis_W1;

  RealMatrix reduced_basis_W2(Teuchos::View, leftSingularVectors,
                              numFullspaceVars,numFullspaceVars - reducedRank,
                              0, reducedRank);

  inactiveBasis = reduced_basis_W2;

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "\nSubspace Model: Active basis is:\n";
    write_data(Cout, activeBasis, true, true, true);
  }

  Cout << "\n**************************************************************"
       << "************\nSubspace Model: Build Statistics"
       << "\nbuild samples: " << totalSamples
       << "\nsubspace size: " << reducedRank
       << std::endl;
  Cout << "****************************************************************"
       << "**********\n";
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

  // and generate the additional samples
  ParLevLIter pl_iter = modelPCIter->mi_parallel_level_iterator(miPLIndex);
  fullspaceSampler.run(pl_iter);
}


void ActiveSubspaceModel::
populate_matrices(unsigned int diff_samples)
{
  // extract into a matrix
  // all_samples vs. all_variables
  const RealMatrix& all_vars = fullspaceSampler.all_samples();
  const IntResponseMap& all_responses = fullspaceSampler.all_responses();

  // TODO: could easily filter NaN/Inf responses and omit
  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "\nSubspace Model: DACE iterator returned "
         << all_responses.size() << " samples. (expected "
         << diff_samples << " samples)" << std::endl;
  }

  int sample_insert_point = varsMatrix.numCols();
  derivativeMatrix.reshape(numFullspaceVars, totalSamples*numFunctions);
  varsMatrix.reshape(numFullspaceVars, totalSamples);

  unsigned int diff_sample_ind = 0;
  IntRespMCIter resp_it = all_responses.begin();
  IntRespMCIter resp_end = all_responses.end();

  // Compute gradient scaling factors if more than 1 response function
  if(numFunctions > 1) {
    for ( ; resp_it != resp_end ; ++resp_it, ++diff_sample_ind) {
      if (subspaceNormalization == SUBSPACE_NORM_MEAN_VALUE) {
        const RealVector& resp_vector = resp_it->second.function_values();
        for (unsigned int fn_ind = 0; fn_ind < numFunctions; ++fn_ind) {
          gradientScaleFactors[fn_ind] += resp_vector(fn_ind) /
                                          static_cast<Real>(diff_samples);
        }
      } // The SUBSPACE_NORM_MEAN_GRAD and SUBSPACE_NORM_DEFAULT cases will be
        // handled later
      else if (subspaceNormalization == SUBSPACE_NORM_MEAN_GRAD) {
        const RealMatrix& resp_matrix = resp_it->second.function_gradients();
        for (unsigned int fn_ind = 0; fn_ind < numFunctions; ++fn_ind) {
          RealVector grad(numFullspaceVars);
          for (size_t ii = 0; ii < numFullspaceVars; ++ii)
            grad[ii] = resp_matrix(ii,fn_ind);

          Real norm_grad = std::sqrt(grad.dot(grad));

          gradientScaleFactors[fn_ind] += norm_grad /
                                          static_cast<Real>(diff_samples);
        }
      }
    }
  }

  // Reset iterators and indices
  diff_sample_ind = 0;
  resp_it = all_responses.begin();
  resp_end = all_responses.end();

  for ( ; resp_it != resp_end ; ++resp_it, ++diff_sample_ind) {
    // the absolute sample number to insert into
    unsigned int sample_ind = sample_insert_point + diff_sample_ind;
    // matrix of num_variables x num_functions
    const RealMatrix& resp_matrix = resp_it->second.function_gradients();
    for (unsigned int fn_ind = 0; fn_ind < numFunctions; ++fn_ind) {
      unsigned int col_ind = sample_ind * numFunctions + fn_ind;
      for (unsigned int var_ind = 0; var_ind < numFullspaceVars; ++var_ind) {
        Real scale = 1.0;
        if (numFunctions > 1 &&
            (subspaceNormalization == SUBSPACE_NORM_DEFAULT ||
             subspaceNormalization == SUBSPACE_NORM_LOCAL_GRAD)) {
          RealVector grad(numFullspaceVars);
          for (size_t ii = 0; ii < numFullspaceVars; ++ii)
            grad[ii] = resp_matrix(ii,fn_ind);

          scale = 1.0 / std::sqrt(grad.dot(grad));
        }

        derivativeMatrix(var_ind, col_ind) = 
          scale * resp_matrix(var_ind, fn_ind) / gradientScaleFactors[fn_ind];
      }
    }
    for (unsigned int var_ind = 0; var_ind < numFullspaceVars; ++var_ind) {
      varsMatrix(var_ind, sample_ind) = all_vars(var_ind, diff_sample_ind);
    }
  }

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "\nSubspace Model: Compiled derivative matrix is:\n";
    write_data(Cout, derivativeMatrix, true, true, true);
  }
}


void ActiveSubspaceModel::
compute_svd()
{
  // Want eigenvalues of derivMatrix*derivMatrix^T, so perform SVD of
  // derivMatrix and square them

  RealMatrix V_transpose;
  leftSingularVectors = derivativeMatrix;
  svd(leftSingularVectors, singularValues, V_transpose);

  // TODO: Analyze whether we need to worry about this
  if(singularValues.length() == 0) {
    Cerr << "\nError (subspace model): No computed singular values available!"
         << std::endl;
    abort_handler(-1);
  }

  int num_singular_values = singularValues.length();

  if (outputLevel >= NORMAL_OUTPUT) {
    Cout << "\nSubspace Model: Singular values are:\n[ ";
    for (unsigned int i=0; i<num_singular_values; ++i)
      Cout << singularValues[i] << " ";
    Cout << "]" << std::endl;
  }
}


void ActiveSubspaceModel::
identify_subspace()
{
  unsigned int bing_li_rank = computeBingLiCriterion(singularValues);
  unsigned int constantine_rank = computeConstantineMetric(singularValues);
  unsigned int energy_rank = computeEnergyCriterion(singularValues);

  unsigned int cv_rank = 0;
  if (subspaceIdCV)
    cv_rank = computeCrossValidationMetric();

  if (reducedRank > 0 && reducedRank <= singularValues.length()) {
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "\nSubspace Model: Subspace size has been specified as dimension "
           << "= " << reducedRank << "." << std::endl;
  } else {
    // Initialize reducedRank
    reducedRank = 1;

    if (subspaceIdBingLi) {
      if (outputLevel >= NORMAL_OUTPUT)
        Cout << "\nSubspace Model: Bing Li truncation method is active."
             << std::endl;

      if (reducedRank < bing_li_rank)
        reducedRank = bing_li_rank;
    }

    if (subspaceIdConstantine) {
      if (outputLevel >= NORMAL_OUTPUT)
        Cout << "\nSubspace Model: Constantine truncation method is active."
             << std::endl;

      if (reducedRank < constantine_rank)
        reducedRank = constantine_rank;
    }

    if (subspaceIdEnergy) {
      if (outputLevel >= NORMAL_OUTPUT)
        Cout << "\nSubspace Model: Eigenvalue energy truncation method is "
             << "active." << std::endl;

      if (reducedRank < energy_rank)
        reducedRank = energy_rank;
    }

    if (subspaceIdCV) {
      if (outputLevel >= NORMAL_OUTPUT)
        Cout << "\nSubspace Model: Cross validation truncation method is "
             << "active." << std::endl;

      if (reducedRank < cv_rank)
        reducedRank = cv_rank;
    }

    // Default case:
    if (!(subspaceIdBingLi || subspaceIdConstantine || subspaceIdEnergy
          || subspaceIdCV)) {
      if (outputLevel >= NORMAL_OUTPUT)
        Cout << "\nSubspace Model: Determining subspace size with Constantine "
             << "metric." << std::endl;

      reducedRank = constantine_rank;
    }
  }

  // Check to make sure subspace size is smaller than numerical rank of the
  // derivative matrix:
  double inf_norm = derivativeMatrix.normInf();
  double mach_svtol = inf_norm * std::numeric_limits<Real>::epsilon();
  if (singularValues[reducedRank-1] < mach_svtol) {
    Cout << "\nWarning (subspace model): Computed subspace size is greater than"
         << " numerical rank. Changing subspace size to numerical rank."
         << std::endl;
    for (unsigned int i=0; i<reducedRank; ++i) {
      if (singularValues[i] < mach_svtol) {
        reducedRank = i;
        break;
      }
    }

    if (reducedRank < 1) {
      Cerr << "\nError (subspace model): Derivative matrix has numerical rank "
           << "of 0. Something may be wrong with the gradient calculations."
           << std::endl;
      abort_handler(-1);
    }

    Cout << "\nSubspace Model: New subspace size is dimension = "
         << reducedRank << "." << std::endl;
  }

  int k_temp = std::ceil(initialSamples / 
                         (2.0*std::log10((double)numFullspaceVars)));

  if (reducedRank >= k_temp) {
    Cout << "\nWarning (subspace model): Computed subspace may be inaccurate. "
         << "Consider increasing the number of samples to satisfy: "
         << "N > 2*k*log(m), where N is the number of samples, k is the "
         << "subspace size, and m is the dimension of the original model."
         << std::endl;
  }
}

unsigned int ActiveSubspaceModel::
computeBingLiCriterion(RealVector& singular_values)
{
  int num_vars = derivativeMatrix.numRows();
  int num_vals;
  if (derivativeMatrix.numCols() < num_vars)
    num_vals = derivativeMatrix.numCols();
  else
    num_vals = num_vars;

  // Stores Bing Li's criterion
  std::vector<RealMatrix::scalarType> bing_li_criterion(num_vals, 0);

  // Compute part 1 of criterion: relative energy in next eigenvalue in the
  // spectrum

  RealMatrix::scalarType eigen_sum = 0.0;
  for(size_t i = 0; i < num_vals; ++i) {
    RealMatrix::scalarType eigen_val = singular_values[i] * singular_values[i];
    bing_li_criterion[i] = eigen_val;
    eigen_sum += eigen_val;
  }

  for(size_t i = 0; i < num_vals; ++i)
    bing_li_criterion[i] /= eigen_sum;

  // Compute part 2 of criterion: bootstrapped determinant metric

  RealMatrix bootstrapped_sample(num_vars, derivativeMatrix.numCols());
  RealVector sample_sing_vals;
  RealMatrix sample_sing_vectors;

  Teuchos::LAPACK<RealMatrix::ordinalType, RealMatrix::scalarType> lapack;

  std::vector<Real> bootstrapped_det(bing_li_criterion.size());

  BootstrapSampler<RealMatrix> bootstrap_sampler(derivativeMatrix,numFunctions);

  for (size_t i = 0; i < numReplicates; ++i) {
    bootstrap_sampler(bootstrapped_sample);

    svd(bootstrapped_sample, sample_sing_vals, sample_sing_vectors);

    // Overwrite bootstrap replicate with singular matrix product
    RealMatrix bootstrapped_sample_copy = bootstrapped_sample;
    bootstrapped_sample.multiply(Teuchos::TRANS, Teuchos::NO_TRANS, 1.0,
                                 leftSingularVectors, bootstrapped_sample_copy,
                                 0.0);

    for(size_t j = 1; j < bootstrapped_det.size(); ++j) {
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
      for (size_t i = 0; i < j; ++i) {
        det *= submatrix(i,i);
      }

      bootstrapped_det[j] += std::abs(det);
    }
  }

  RealMatrix::scalarType det_sum = 0.0;
  bootstrapped_det[0] = 0.0;
  for (size_t i = 1; i < bootstrapped_det.size(); ++i) {
    bootstrapped_det[i] = 1.0 - bootstrapped_det[i] /
                          static_cast<RealMatrix::scalarType>(numReplicates);
    det_sum += bootstrapped_det[i];
  }

  for (size_t i = 0; i < bootstrapped_det.size(); ++i) {
    bing_li_criterion[i] += bootstrapped_det[i] / det_sum;
  }

  if (outputLevel >= NORMAL_OUTPUT) {
    Cout << "\nSubspace Model: Bing Li Criterion values are:\n[ ";
    for (size_t i = 0; i < bing_li_criterion.size(); ++i) {
      Cout << bing_li_criterion[i] << " ";
    }
    Cout << "]" << std::endl;
  }

  // Cutoff is 1st minimum of the criterion
  unsigned int rank = 0;
  for (unsigned int i = 1; i < bing_li_criterion.size(); ++i) {
    if(bing_li_criterion[i-1] < bing_li_criterion[i]) {
      rank = i-1;
      break;
    }
  }

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\nSubspace Model: Bing Li metric subspace size estimate = "
         << rank << std::endl;

  return rank;
}

unsigned int ActiveSubspaceModel::
computeConstantineMetric(RealVector& singular_values)
{
  int num_vars = derivativeMatrix.numRows();
  int num_vals;
  if (derivativeMatrix.numCols() < num_vars)
    num_vals = derivativeMatrix.numCols();
  else
    num_vals = num_vars;

  // Stores Constantine's metric
  RealArray constantine_metric((num_vals < num_vars-1) ? num_vals : num_vars-1,
                               0);

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

  for (size_t i = 0; i < numReplicates; ++i) {
    bootstrap_sampler(bootstrapped_sample);

    svd(bootstrapped_sample, sample_sing_vals, sample_sing_vectors);

    for(size_t j = 0; j < constantine_metric.size(); ++j) {
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

  if (outputLevel >= NORMAL_OUTPUT) {
    Cout << "\nSubspace Model: Constantine metric values are:\n[ ";
    for (size_t i = 0; i < constantine_metric.size(); ++i) {
      Cout << constantine_metric[i] << " ";
    }
    Cout << "]" << std::endl;
  }

  // Cutoff is global minimum of metric
  unsigned int rank = 0;
  Real min_val = 0;
  for (unsigned int i = 0; i < constantine_metric.size(); ++i) {
    if(constantine_metric[i] < min_val || i == 0) {
      min_val = constantine_metric[i];
      rank = i+1;
    }
  }

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\nSubspace Model: Constantine metric subspace size estimate = "
         << rank << std::endl;

  return rank;
}

unsigned int ActiveSubspaceModel::
computeEnergyCriterion(RealVector& singular_values)
{
  int num_vars = derivativeMatrix.numRows();
  int num_vals;
  if (derivativeMatrix.numCols() < num_vars)
    num_vals = derivativeMatrix.numCols();
  else
    num_vals = num_vars;

  Real total_energy = 0.0;
  for (size_t i = 0; i < num_vals; ++i) {
    // eigenvalue = (singular_value)^2
    total_energy += std::pow(singular_values[i],2);
  }

  RealVector energy_metric(num_vals);
  energy_metric[0] = std::pow(singular_values[0],2)/total_energy;
  for (size_t i = 1; i < num_vals; ++i) {
    energy_metric[i] = std::pow(singular_values[i],2)/total_energy
                       + energy_metric[i-1];
  }

  if (outputLevel >= NORMAL_OUTPUT) {
    Cout << "\nSubspace Model: Energy criterion values are:\n[ ";
    for (size_t i = 0; i < num_vals; ++i) {
      Cout << energy_metric[i] << " ";
    }
    Cout << "]" << std::endl;
  }

  unsigned int rank = 0;
  for (unsigned int i = 0; i < num_vals; ++i) {
    if(std::abs(1.0 - energy_metric[i]) < truncationTolerance) {
      rank = i+1;
      break;
    }
  }

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\nSubspace Model: Eigenvalue energy metric subspace size estimate "
         << "= " << rank << ". (truncation_tolerance = " << truncationTolerance
         << ")" << std::endl;

  return rank;
}


unsigned int ActiveSubspaceModel::computeCrossValidationMetric()
{
  // Compute max_rank, that is the maximum dimension that we have enough samples
  // to build a surrogate for

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\nSubspace Model: Beginning cross validation subspace id "
         << "method.\n" << std::endl;

  int num_folds = 10;

  int poly_degree = 2; // quadratic bases

  const RealMatrix& all_x = fullspaceSampler.all_samples();
  const IntResponseMap& all_y = fullspaceSampler.all_responses();
  int n_samps = all_x.numCols();

  int num_samples_req = 1;
  int max_rank = 1;
  while (n_samps >= num_samples_req && max_rank < numFullspaceVars) {
    num_samples_req = 1;
    for (int ii = max_rank + poly_degree; ii > max_rank; ii--) {
      num_samples_req *= ii; // Numerator
    }
    for (int ii = poly_degree; ii > 0; ii--) {
      num_samples_req /= ii; // Denomenator
    }

    if (n_samps < num_samples_req) {
      max_rank--; // Too big by one
      break;
    } else
      max_rank++; // Increment for next loop iteration
  }

  if (cvMaxRank >= 0 && max_rank > cvMaxRank)
    max_rank = cvMaxRank;

  // Loop over all feasible subspace sizes
  std::vector<Real> cv_error;
  bool found_acceptable_subspace = false;
  bool rel_tol_met = false, decrease_tol_met = false;
  for (unsigned int ii = 1; ii <= max_rank; ii++) {
    if (found_acceptable_subspace && cvIncremental)
      break;

    // Add element to the list of metrics:
    cv_error.push_back(0.0);

    // Create a local active subspace model using the light-weight constructor:
    Model asm_model_tmp;
    asm_model_tmp.assign_rep(new ActiveSubspaceModel(subModel, ii,
						     leftSingularVectors,
						     QUIET_OUTPUT), false);

    String sample_reuse = "", approx_type = "global_moving_least_squares";
    ActiveSet surr_set = current_response().active_set(); // copy

    UShortArray approx_order(reducedRank, poly_degree);
    short corr_order = -1, corr_type = NO_CORRECTION, data_order = 1;
    Iterator dace_iterator;

    Model cv_surr_model;
    cv_surr_model.assign_rep(new DataFitSurrModel(dace_iterator, asm_model_tmp,
                             surr_set, approx_type, approx_order, corr_type,
                             corr_order, data_order, QUIET_OUTPUT,sample_reuse),
                             false);


    Teuchos::BLAS<int, Real> teuchos_blas;

    //  Project fullspace samples onto active directions
    //  Calculate x_active = W1^T*x
    Real alpha = 1.0;
    Real beta = 0.0;

    RealMatrix all_x_active(ii, all_x.numCols());

    RealMatrix W1(Teuchos::Copy, leftSingularVectors,
                  numFullspaceVars, ii);
    int m = W1.numCols();
    int k = W1.numRows();
    int n = all_x.numCols();

    teuchos_blas.GEMM(Teuchos::TRANS, Teuchos::NO_TRANS, m, n, k, alpha,
                      W1.values(), k, all_x.values(), k, beta,
                      all_x_active.values(), m);

    // Randomly partion sample matrix into k folds

    std::vector<int> random_index_vec;
    // set sequential index values:
    for (int ind = 0; ind < n; ++ind)
      random_index_vec.push_back(ind);
    // shuffle these indices:
    std::random_shuffle(random_index_vec.begin(), random_index_vec.end());

    // Compute the size of each fold:
    std::vector<int> fold_size(num_folds, n / num_folds);
    for (int jj = 0; jj < n % num_folds; jj++)
      fold_size[jj]++;

    // Loop over k folds holding one fold out at a time as a test set and using
    // the rest to build the surrogate
    for (int jj = 0; jj < num_folds; jj++) {
      // Get kth fold and put into test sets, put the rest into training sets:

      int fold_start_ind = 0;
      for (int kk = 0; kk < jj; kk++)
        fold_start_ind += fold_size[kk];

      RealMatrix training_x(all_x_active.numRows(), n - fold_size[jj]);
      RealMatrix test_x(all_x_active.numRows(), fold_size[jj]);

      IntResponseMap training_y;
      IntResponseMap test_y;

      IntRespMCIter resp_it = all_y.begin();
      IntRespMCIter resp_end = all_y.end();

      // Fill test_x, test_y, training_x, and training_y
      int training_count = 0;
      int test_count = 0;
      for (int kk = 0; kk < all_x_active.numCols(); kk++) {
        if (fold_start_ind <= random_index_vec[kk] &&
            (fold_start_ind + fold_size[jj]) > random_index_vec[kk]) {
          test_y[test_count] = resp_it->second;
          for (int mm = 0; mm < all_x_active.numRows(); mm++)
            test_x(mm, test_count) = all_x_active(mm, kk);

          test_count++;
        } else {
          training_y[training_count] = resp_it->second;
          for (int mm = 0; mm < all_x_active.numRows(); mm++)
            training_x(mm, training_count) = all_x_active(mm, kk);

          training_count++;
        }

        resp_it++;
      }

      cv_error[ii-1] += build_cv_surrogate(cv_surr_model, training_x,
                                           training_y,test_x, test_y)/num_folds;
    }

    if (cvIdMethod == RELATIVE_TOLERANCE) {
      tolerance_met_index(cv_error, cvRelTolerance, rel_tol_met);
      if (rel_tol_met)
        found_acceptable_subspace = true;
    }

    if (cvIdMethod == DECREASE_TOLERANCE) {
      tolerance_met_index(negative_diff(cv_error), cvDecreaseTolerance,
                          decrease_tol_met);
      if (decrease_tol_met)
        found_acceptable_subspace = true;
    }
  }

  return determine_rank_cv(cv_error);
}

unsigned int ActiveSubspaceModel::
determine_rank_cv(const std::vector<Real> &cv_error)
{
  if (outputLevel >= NORMAL_OUTPUT) {
    Cout << "\nSubspace Model: Cross validation metric values are:\n[ ";
    for (size_t i = 0; i < cv_error.size(); ++i) {
      Cout << cv_error[i] << " ";
    }
    Cout << "]" << std::endl;
  }

  bool rel_tol_met = false, decrease_tol_met = false;

  unsigned int rank_min_metric = min_index(cv_error) + 1;
  unsigned int rank_rel_tol = tolerance_met_index(cv_error, cvRelTolerance,
                              rel_tol_met) + 1;
  unsigned int rank_decrease_tol = tolerance_met_index(negative_diff(cv_error),
                                   cvDecreaseTolerance, decrease_tol_met) + 1;

  unsigned int rank = 0;
  bool fallback = false;
  switch (cvIdMethod) {
  case MINIMUM_METRIC:
    rank = rank_min_metric;
    break;
  case RELATIVE_TOLERANCE:
  case CV_ID_DEFAULT:
    if (rel_tol_met)
      rank = rank_rel_tol;
    else {
      rank = rank_min_metric;
      fallback = true;
    }
    break;
  case DECREASE_TOLERANCE:
    if (decrease_tol_met)
      rank = rank_decrease_tol;
    else {
      rank = rank_min_metric;
      fallback = true;
    }
    break;
  }

  if (outputLevel >= NORMAL_OUTPUT) {
    Cout << "\nSubspace Model: Cross validation metric: minimum metric subspace"
         << " size estimate = " << rank_min_metric << ".";
    if (cvIdMethod == MINIMUM_METRIC)
      Cout << " (active)";
    else if (fallback)
      Cout << " (active as a fallback)";
    else
      Cout << " (inactive)";
    Cout << std::endl;

    Cout << "\nSubspace Model: Cross validation metric: relative tolerance "
         << "subspace size estimate = " << rank_rel_tol << ".";
    if (cvIdMethod == RELATIVE_TOLERANCE || cvIdMethod == CV_ID_DEFAULT)
      Cout << " (active, ";
    else
      Cout << " (inactive, ";
    Cout << "tolerance = " << cvRelTolerance << ")" << std::endl;

    Cout << "\nSubspace Model: Cross validation metric: decrease tolerance "
         << "subspace size estimate = " << rank_decrease_tol << ".";
    if (cvIdMethod == DECREASE_TOLERANCE)
      Cout << " (active, ";
    else
      Cout << " (inactive, ";
    Cout << "tolerance = " << cvDecreaseTolerance << ")" << std::endl;
  }

  return rank;
}

unsigned int ActiveSubspaceModel::
min_index(const std::vector<Real> &cv_error)
{
  if (cv_error.empty()) // Return full rank since this shouldn't be empty
    return numFullspaceVars-1;

  Real min_val = cv_error[0];
  unsigned int min_ind = 0;
  for (unsigned int ii = 1; ii < cv_error.size(); ii++) {
    if (cv_error[ii] < min_val) {
      min_val = cv_error[ii];
      min_ind = ii;
    }
  }

  return min_ind;
}

unsigned int ActiveSubspaceModel::
tolerance_met_index(const std::vector<Real> &cv_error, Real tolerance,
                    bool &tol_met)
{
  tol_met = false;
  for (unsigned int ii = 0; ii < cv_error.size(); ii++) {
    if (cv_error[ii] < tolerance) {
      tol_met = true;
      return ii;
    }
  }

  // Return full rank since tolerance is not met
  return numFullspaceVars-1;
}

std::vector<Real> ActiveSubspaceModel::
negative_diff(const std::vector<Real> &cv_error)
{
  std::vector<Real> neg_diff(cv_error.size()-1);
  for (unsigned int ii = 0; ii < neg_diff.size(); ii++) {
    neg_diff[ii] = cv_error[ii] - cv_error[ii+1];
  }

  return neg_diff;
}

/// Build global moving least squares surrogate model to use in cross validation
/// to estimate active subspace size
Real ActiveSubspaceModel::
build_cv_surrogate(Model &cv_surr_model, RealMatrix training_x,
                   IntResponseMap training_y,
                   RealMatrix test_x, IntResponseMap test_y)
{
  cv_surr_model.update_approximation(training_x, training_y, true);

  int num_test_points = test_x.numCols();

  // evaluate surrogate at test points:
  IntResponseMap test_y_surr;
  ActiveSet surr_set = current_response().active_set(); // copy
  for (int ii = 0; ii < num_test_points; ii++) {
    cv_surr_model.continuous_variables(Teuchos::getCol(Teuchos::Copy, test_x,
                                                       ii));

    cv_surr_model.evaluate(surr_set);

    test_y_surr[ii] = cv_surr_model.current_response().copy();
  }

  // compute L2 norm of error between test_prediction and actual response:
  IntRespMCIter resp_it = test_y.begin();
  IntRespMCIter resp_end = test_y.end();
  IntRespMCIter resp_surr_it = test_y_surr.begin();
  IntRespMCIter resp_surr_end = test_y_surr.end();
  RealVector error_norm(numFunctions);
  RealVector mean(numFunctions);
  for (; resp_it != resp_end
       && resp_surr_it != resp_surr_end; resp_it++, resp_surr_it++) {
    const RealVector& resp_vector = resp_it->second.function_values();
    const RealVector& resp_surr_vector = resp_surr_it->second.function_values();

    for (size_t ii = 0; ii < numFunctions; ii++) {
      error_norm[ii] += std::pow(resp_vector[ii] - resp_surr_vector[ii], 2);
      mean[ii] += resp_vector[ii] / test_y.size();
    }
  }


  resp_it = test_y.begin();
  resp_end = test_y.end();
  RealVector stdev(numFunctions);
  for (; resp_it != resp_end; resp_it++) {
    const RealVector& resp_vector = resp_it->second.function_values();
    for (size_t ii = 0; ii < numFunctions; ii++)
      stdev[ii] += std::pow(resp_vector[ii] - mean[ii], 2) / test_y.size();
  }
  for (size_t ii = 0; ii < numFunctions; ii++)
    stdev[ii] = std::sqrt(stdev[ii]);

  Real max_error_norm = 0;
  for (size_t ii = 0; ii < numFunctions; ii++) {
    Real norm_rms_error = std::sqrt(error_norm[ii] / test_y.size()) / stdev[ii];

    if (norm_rms_error > max_error_norm)
      max_error_norm = norm_rms_error;
  }

  return max_error_norm;
}


/** May eventually take on init_comms and related operations.  Also
    may want ide of build/update like DataFitSurrModel, eventually. */
bool ActiveSubspaceModel::initialize_mapping(ParLevLIter pl_iter)
{
  RecastModel::initialize_mapping(pl_iter);

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\nSubspace Model: Initializing active subspace." << std::endl;

  // init-time setting of miPLIndex for use in component_parallel_mode()
  miPLIndex = modelPCIter->mi_parallel_level_index(pl_iter);

  // Set mode OFFLINE_PHASE
  component_parallel_mode(OFFLINE_PHASE);

  bool sub_model_resize = subModel.initialize_mapping(pl_iter);

  // TODO: create modes to switch between active, inactive, and complete
  //       subspaces

  // runtime operation to identify the subspace model (if not later
  // returning to update the subspace)
  build_subspace();

  // complete initialization of the base RecastModel
  initialize_recast();

  // TODO: generalize to other distribution types
  // convert the normal distributions to the reduced space and set in the
  // reduced model
  uncertain_vars_to_subspace();

  // update with subspace constraints
  update_linear_constraints();

  // set new subspace variable labels
  update_var_labels();

  if (buildSurrogate)
    build_surrogate();

  subspaceInitialized = true;

  // Perform numerical derivatives in subspace:
  supportsEstimDerivs = true;

  // Kill servers and return ranks [1,n-1] to serve_init_mapping()
  component_parallel_mode(CONFIG_PHASE);

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\nSubspace Model: Initialization of active subspace is complete."
         << std::endl;

  if (reducedRank != numFullspaceVars || // Active SS is reduced rank
      sub_model_resize) // Active SS is full rank but subModel resized
    return true; // Size of variables has changed
  else
    return false;
}


bool ActiveSubspaceModel::finalize_mapping()
{
  // TODO: return to full space
  //subspaceInitialized = false;

  RecastModel::finalize_mapping();

  return false; // This will become true when TODO is implemented.
}


void ActiveSubspaceModel::component_parallel_mode(short mode)
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

  // activate new serve mode (matches ActiveSubspaceModel::serve_run(pl_iter)).
  if (componentParallelMode != mode &&
      modelPCIter->mi_parallel_level_defined(miPLIndex)) {
    ParLevLIter pl_iter = modelPCIter->mi_parallel_level_iterator(miPLIndex);
    const ParallelLevel& mi_pl = modelPCIter->mi_parallel_level(miPLIndex);
    if (mi_pl.server_communicator_size() > 1) {
      if (mode == OFFLINE_PHASE) {
        // This block tells Model::serve_init_mapping() to go into
        // ActiveSubspaceModel::serve_run() to build the subspace
        short mapping_code = SERVE_RUN;
        parallelLib.bcast(mapping_code, *pl_iter);
        parallelLib.bcast(offlineEvalConcurrency, *pl_iter);
      }

      // bcast mode to ActiveSubspaceModel::serve_run()
      parallelLib.bcast(mode, mi_pl);

      if (mode == OFFLINE_PHASE)
        subModel.set_communicators(pl_iter, offlineEvalConcurrency);
      else if (mode == ONLINE_PHASE)
        set_communicators(pl_iter, onlineEvalConcurrency);
    }
  }

  componentParallelMode = mode;
}


void ActiveSubspaceModel::serve_run(ParLevLIter pl_iter,
                                    int max_eval_concurrency)
{
  do {
    parallelLib.bcast(componentParallelMode, *pl_iter);
    if (componentParallelMode == OFFLINE_PHASE) {
      subModel.serve_run(pl_iter, offlineEvalConcurrency);
    } else if (componentParallelMode == ONLINE_PHASE) {
      set_communicators(pl_iter, onlineEvalConcurrency, false);
      subModel.serve_run(pl_iter, onlineEvalConcurrency);
    }
  } while (componentParallelMode != CONFIG_PHASE);
}


void ActiveSubspaceModel::stop_servers()
{
  component_parallel_mode(CONFIG_PHASE);
}


void ActiveSubspaceModel::stop_init_mapping(ParLevLIter pl_iter)
{
  short term_code = 0;
  parallelLib.bcast(term_code, *pl_iter);
}


int ActiveSubspaceModel::serve_init_mapping(ParLevLIter pl_iter)
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


void ActiveSubspaceModel::derived_evaluate(const ActiveSet& set)
{
  if (!subspaceInitialized) {
    Cerr << "\nError (subspace model): model has not been initialized."
         << std::endl;
    abort_handler(-1);
  }

  component_parallel_mode(ONLINE_PHASE);

  if (buildSurrogate) {
    ++recastModelEvalCntr;

    surrogateModel.active_variables(currentVariables);
    surrogateModel.evaluate(set);

    currentResponse.active_set(set);
    currentResponse.update(surrogateModel.current_response());
  } else
    RecastModel::derived_evaluate(set);
}


void ActiveSubspaceModel::derived_evaluate_nowait(const ActiveSet& set)
{
  if (!subspaceInitialized) {
    Cerr << "\nError (subspace model): model has not been initialized."
         << std::endl;
    abort_handler(-1);
  }

  component_parallel_mode(ONLINE_PHASE);

  if (buildSurrogate) {
    ++recastModelEvalCntr;

    surrogateModel.active_variables(currentVariables);
    surrogateModel.evaluate_nowait(set);

    // store map from surrogateModel eval id to ActiveSubspaceModel id
    surrIdMap[surrogateModel.evaluation_id()] = recastModelEvalCntr;
  } else
    RecastModel::derived_evaluate_nowait(set);
}


const IntResponseMap& ActiveSubspaceModel::derived_synchronize()
{
  if (!subspaceInitialized) {
    Cerr << "\nError (subspace model): model has not been initialized."
         << std::endl;
    abort_handler(-1);
  }

  component_parallel_mode(ONLINE_PHASE);

  if (buildSurrogate) {
    surrResponseMap.clear();
    rekey_synch(surrogateModel, true, surrIdMap, surrResponseMap);
    return surrResponseMap;
  } else
    return RecastModel::derived_synchronize();
}


const IntResponseMap& ActiveSubspaceModel::derived_synchronize_nowait()
{
  if (!subspaceInitialized) {
    Cerr << "\nError (subspace model): model has not been initialized."
         << std::endl;
    abort_handler(-1);
  }

  component_parallel_mode(ONLINE_PHASE);

  if (buildSurrogate) {
    surrResponseMap.clear();
    rekey_synch(surrogateModel, false, surrIdMap, surrResponseMap);
    return surrResponseMap;
  } else
    return RecastModel::derived_synchronize_nowait();
}


void ActiveSubspaceModel::update_var_labels()
{
  StringMultiArray subspace_var_labels(boost::extents[reducedRank]);
  for (int i = 0; i < reducedRank; i++) {
    subspace_var_labels[i] = "ssv_" + boost::lexical_cast<std::string>(i+1);
  }

  continuous_variable_labels(
    subspace_var_labels[boost::indices[idx_range(0, reducedRank)]]);
}


/**  This specialization is because the model is used in multiple
     contexts in this iterator, depending on build phase.  Note that
     this overrides the default behavior at Iterator which recurses
     into any submodels. */
void ActiveSubspaceModel::
derived_init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                           bool recurse_flag)
{
  // The inbound subModel concurrency accounts for any finite differences

  onlineEvalConcurrency = max_eval_concurrency;

  if (recurse_flag) {
    if (!subspaceInitialized)
      fullspaceSampler.init_communicators(pl_iter);

    subModel.init_communicators(pl_iter, max_eval_concurrency);
  }
}


void ActiveSubspaceModel::
derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                          bool recurse_flag)
{
  miPLIndex = modelPCIter->mi_parallel_level_index(pl_iter);// run time setting

  if (recurse_flag) {
    if (!subspaceInitialized)
      fullspaceSampler.set_communicators(pl_iter);

    subModel.set_communicators(pl_iter, max_eval_concurrency);

    // RecastModels do not utilize default set_ie_asynchronous_mode() as
    // they do not define the ie_parallel_level
    asynchEvalFlag = subModel.asynch_flag();
    evaluationCapacity = subModel.evaluation_capacity();
  }
}


void ActiveSubspaceModel::
derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                           bool recurse_flag)
{
  if (recurse_flag) {
    fullspaceSampler.free_communicators(pl_iter);

    subModel.free_communicators(pl_iter, max_eval_concurrency);
  }
}


void ActiveSubspaceModel::init_fullspace_sampler(unsigned short sample_type)
{
  std::string rng; // use default random number generator

  if (sample_type == SUBMETHOD_DEFAULT)
    sample_type = SUBMETHOD_RANDOM; // default to Monte Carlo sampling

  // configure this sampler initially to work with initialSamples
  NonDLHSSampling* ndlhss =
    new NonDLHSSampling(subModel, sample_type, initialSamples, randomSeed,
                        rng, true);

  fullspaceSampler.assign_rep(ndlhss, false);

  // TODO: review whether this is needed
  fullspaceSampler.sub_iterator_flag(true);
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
            nonlinear_resp_mapping, response_mapping, NULL);
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

    case MIXED_DESIGN:
    case RELAXED_DESIGN:
      // resize continuous design
      vc_totals[TOTAL_CDV] = reducedRank;
      break;

    case MIXED_ALEATORY_UNCERTAIN:
    case RELAXED_ALEATORY_UNCERTAIN:
      // resize continuous aleatory
      vc_totals[TOTAL_CAUV] = reducedRank;
      break;

    case MIXED_UNCERTAIN:
    case RELAXED_UNCERTAIN:
    case MIXED_EPISTEMIC_UNCERTAIN:
    case RELAXED_EPISTEMIC_UNCERTAIN:
      // resize continuous epistemic (note there may not actually be
      // any epistemic variables in the *_UNCERTAIN cases)
      vc_totals[TOTAL_CEUV] = reducedRank;
      break;

    case MIXED_ALL:
    case RELAXED_ALL:
    case MIXED_STATE:
    case RELAXED_STATE:
      // resize continuous state
      vc_totals[TOTAL_CSV] = reducedRank;
      break;

    default:
      Cerr << "\nError (subspace model): invalid active variables view "
           << active_view << "." << std::endl;
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

    TODO: The translation of the correlations from full to reduced
    space is likely wrong for rank correlations; should be correct for
    covariance.
*/
/// transform and set the distribution parameters in the reduced model
void ActiveSubspaceModel::uncertain_vars_to_subspace()
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
    Cout << "\nSubspace Model: correl_x = \n";
    write_data(Cout, correl_x, true, true, true);
  }

  bool native_correl = correl_x.empty() ? false : true;
  if (native_correl && correl_x.numRows() != numFullspaceVars) {
    Cerr << "\nError (subspace model): Wrong correlation size." << std::endl;
    abort_handler(-1);
  }

  // reduced space characterization: mean mu, std dev sd
  RealVector mu_y(reducedRank), sd_y(reducedRank);
  RealVector mu_z(inactiveBasis.numCols());


  // mu_y = activeBasis^T * mu_x
  int m = activeBasis.numRows();
  int n = activeBasis.numCols();
  Real alpha = 1.0;
  Real beta = 0.0;

  int incx = 1;
  int incy = 1;

  // y <-- alpha*A*x + beta*y
  // mu_y <-- 1.0 * activeBasis^T * mu_x + 0.0 * mu_y
  Teuchos::BLAS<int, Real> teuchos_blas;
  teuchos_blas.GEMV(Teuchos::TRANS, m, n, alpha, activeBasis.values(), m,
                    mu_x.values(), incx, beta, mu_y.values(), incy);

  // convert the correlations C_x to variance V_x
  // V_x <-- diag(sd_x) * C_x * diag(sd_x)
  // not using symmetric so we can multiply() below
  RealMatrix V_x(activeBasis.numRows(), activeBasis.numRows(), false);
  if (native_correl) {
    for (int row=0; row<activeBasis.numRows(); ++row)
      for (int col=0; col<activeBasis.numRows(); ++col)
        V_x(row, col) = sd_x(row)*correl_x(row,col)*sd_x(col);
  } else {
    V_x = 0.0;
    for (int row=0; row<activeBasis.numRows(); ++row)
      V_x(row, row) = sd_x(row)*sd_x(row);
  }


  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "\nSubspace Model: activeBasis = \n";
    write_data(Cout, activeBasis, true, true, true);
    Cout << "\nSubspace Model: V_x =\n";
    write_data(Cout, V_x, true, true, true);
  }

  // compute V_y = U^T * V_x * U
  alpha = 1.0;
  beta = 0.0;
  RealMatrix UTVx(n, m, false);
  UTVx.multiply(Teuchos::TRANS, Teuchos::NO_TRANS,
                alpha, activeBasis, V_x, beta);
  RealMatrix V_y(reducedRank, reducedRank, false);
  V_y.multiply(Teuchos::NO_TRANS, Teuchos::NO_TRANS,
               alpha, UTVx, activeBasis, beta);

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "\nSubspace Model: V_y = \n";
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
    Cout << "\nSubspace Model: correl_y = \n";
    write_data(Cout, correl_y, true, true, true);
  }

  reduced_dist_params.uncertain_correlations(correl_y);

  // Set inactive subspace variables
  // mu_z = inactiveBasis^T * mu_x
  m = inactiveBasis.numRows();
  n = inactiveBasis.numCols();
  alpha = 1.0;
  beta = 0.0;

  incx = 1;
  int incz = 1;

  teuchos_blas.GEMV(Teuchos::TRANS, m, n, alpha, inactiveBasis.values(), m,
                    mu_x.values(), incx, beta, mu_z.values(), incz);

  inactiveVars = mu_z;


  // Set continuous variable types:
  UShortMultiArray cont_variable_types(boost::extents[reducedRank]);
  for (int i = 0; i < reducedRank; i++) {
    cont_variable_types[i] = NORMAL_UNCERTAIN;
  }
  currentVariables.continuous_variable_types(
    cont_variable_types[boost::indices[idx_range(0, reducedRank)]]);

  // Set currentVariables to means of active variables:
  continuous_variables(mu_y);
}



/**
  Perform the variables mapping from recast reduced dimension
  variables y to original model x variables via linear transformation.
  Maps only continuous variables.
*/
void ActiveSubspaceModel::
vars_mapping(const Variables& recast_y_vars, Variables& sub_model_x_vars)
{
  Teuchos::BLAS<int, Real> teuchos_blas;

  const RealVector& y = recast_y_vars.continuous_variables();
  // TODO: does this yield a view or a copy?
  //RealVector x = sub_model_x_vars.continuous_variables();
  RealVector x;
  copy_data(sub_model_x_vars.continuous_variables(), x);

  //  Calculate x = activeBasis*y + inactiveBasis*inactiveVars via matvec
  //  directly into x cv in submodel
  const RealMatrix& W1 = asmInstance->activeBasis;
  int m = W1.numRows();
  int n = W1.numCols();

  Real alpha = 1.0;
  Real beta = 0.0;

  int incx = 1;
  int incy = 1;

  teuchos_blas.GEMV(Teuchos::NO_TRANS, m, n, alpha, W1.values(), m,
                    y.values(), incy, beta, x.values(), incx);

  // Now add the inactive variable's contribution:
  const RealMatrix& W2 = asmInstance->inactiveBasis;
  const RealVector& z = asmInstance->inactiveVars;
  m = W2.numRows();
  n = W2.numCols();

  alpha = 1.0;
  beta = 1.0;

  int incz = 1;

  teuchos_blas.GEMV(Teuchos::NO_TRANS, m, n, alpha, W2.values(), m,
                    z.values(), incz, beta, x.values(), incx);

  sub_model_x_vars.continuous_variables(x);

  if (asmInstance->outputLevel >= DEBUG_OUTPUT) {
    Cout << "\nSubspace Model: Subspace vars are\n";
    Cout << recast_y_vars << std::endl;

    Cout << "\nSubspace Model: Fullspace vars are\n";
    Cout << sub_model_x_vars << std::endl;
  }

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


/**
  Perform the response mapping from submodel to recast response
*/
void ActiveSubspaceModel::
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
    const RealMatrix& W1 = asmInstance->activeBasis;
    int m = W1.numCols();
    int k = W1.numRows();
    int n = dg_dx.numCols();

    Real alpha = 1.0;
    Real beta = 0.0;

    teuchos_blas.GEMM(Teuchos::TRANS, Teuchos::NO_TRANS, m, n, k, alpha,
                      W1.values(), k, dg_dx.values(), k, beta,dg_dy.values(),m);

    recast_resp.function_gradients(dg_dy);
  }


  // Now transform the Hessians:
  const RealSymMatrixArray& H_x_all = sub_model_resp.function_hessians();
  if(!H_x_all.empty()) {
    RealSymMatrixArray H_y_all(H_x_all.size());
    for (int i = 0; i < H_x_all.size(); i++) {
      // compute H_y = W1^T * H_x * W1
      const RealMatrix& W1 = asmInstance->activeBasis;
      int m = W1.numRows();
      int n = W1.numCols();

      Real alpha = 1.0;

      RealSymMatrix H_y(n, false);
      Teuchos::symMatTripleProduct<int,Real>(Teuchos::TRANS, alpha,
                                             H_x_all[i], W1, H_y);
      H_y_all[i] = H_y;
    }

    recast_resp.function_hessians(H_y_all);
  }
}


/**
  Build surrogate over active subspace
*/
void ActiveSubspaceModel::build_surrogate()
{
  // Initialize surrogateModel here, switch it out with subModel after subspace
  // is built.

  Model asm_model;
  asm_model.assign_rep(asmInstance, false);

  String sample_reuse = "", approx_type = "global_moving_least_squares";
  ActiveSet surr_set = current_response().active_set(); // copy

  int poly_degree = 2; // quadratic bases
  UShortArray approx_order(reducedRank, poly_degree);
  short corr_order = -1, corr_type = NO_CORRECTION, data_order = 1;
  Iterator dace_iterator;

  surrogateModel.assign_rep(new DataFitSurrModel(dace_iterator, asm_model,
                            surr_set, approx_type, approx_order, corr_type,
                            corr_order, data_order, outputLevel, sample_reuse),
                            false);

  const RealMatrix& all_vars_x = fullspaceSampler.all_samples();
  const IntResponseMap& all_responses = fullspaceSampler.all_responses();

  Teuchos::BLAS<int, Real> teuchos_blas;

  //  Project fullspace samples onto active directions
  //  Calculate y = W1^T*x
  Real alpha = 1.0;
  Real beta = 0.0;

  RealMatrix all_vars_y(reducedRank, all_vars_x.numCols());

  const RealMatrix& W1 = activeBasis;
  int m = W1.numCols();
  int k = W1.numRows();
  int n = all_vars_x.numCols();

  teuchos_blas.GEMM(Teuchos::TRANS, Teuchos::NO_TRANS, m, n, k, alpha,
                    W1.values(), k, all_vars_x.values(), k, beta,
                    all_vars_y.values(), m);

  // Check to make sure we have enough samples to build a moving least squares
  // surrogate. If not, add them using additional refinement samples.
  // First calculate the number of samples needed.
  // num_samples_req = (reducedRank + poly_degree)!/(reducedRank!*poly_degree!)
  // Below is a simplified form of the equation above: (avoids large factorials)
  int num_samples_req = 1;
  for (int ii = reducedRank + poly_degree; ii > reducedRank; ii--) {
    num_samples_req *= ii; // Numerator
  }
  for (int ii = poly_degree; ii > 0; ii--) {
    num_samples_req /= ii; // Denomenator
  }

  if ((n + refinementSamples) < num_samples_req) {
    int num_new_samples = num_samples_req - (n + refinementSamples);
    refinementSamples += num_new_samples;
    if (outputLevel >= NORMAL_OUTPUT) {
      Cout << "\nWarning (subspace model):  Moving least squares surrogate "
           << "needs at least " << num_samples_req << " samples. Adding "
           << num_new_samples << " additional refinement_samples for building "
           << "surrogate." << std::endl;
    }
  }

  surrogateModel.append_approximation(all_vars_y, all_responses,
                                      (refinementSamples == 0));

  // If user requested refinement_samples for building the surrogate evaluate
  // them here. Since moving least squares doesn't use gradients only request
  // function values here.
  if (refinementSamples > 0) {
    if (outputLevel >= DEBUG_OUTPUT) {
      Cout << "\nSubspace Model: adding " << refinementSamples
           << " refinement_samples for building surrogate." << std::endl;
    }

    ActiveSet dace_set = fullspaceSampler.active_set(); // copy
    unsigned short request_value = 1;
    dace_set.request_values(request_value);
    fullspaceSampler.active_set(dace_set);
    fullspaceSampler.sampling_reference(refinementSamples);
    fullspaceSampler.sampling_reset(refinementSamples, true, false);

    // and generate the additional samples
    ParLevLIter pl_iter = modelPCIter->mi_parallel_level_iterator(miPLIndex);
    fullspaceSampler.run(pl_iter);

    const RealMatrix& all_vars_x_ref = fullspaceSampler.all_samples();
    const IntResponseMap& all_responses_ref = fullspaceSampler.all_responses();

    RealMatrix all_vars_y_ref(reducedRank, all_vars_x_ref.numCols());

    n = all_vars_x_ref.numCols();

    teuchos_blas.GEMM(Teuchos::TRANS, Teuchos::NO_TRANS, m, n, k, alpha,
                      W1.values(), k, all_vars_x_ref.values(), k, beta,
                      all_vars_y_ref.values(), m);

    surrogateModel.append_approximation(all_vars_y_ref, all_responses_ref,true);
  }
}

}  // namespace Dakota


