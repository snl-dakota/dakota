/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef ACTIVE_SUBSPACE_MODEL_H
#define ACTIVE_SUBSPACE_MODEL_H

#include "RecastModel.hpp"
#include "DakotaIterator.hpp"

namespace Dakota {

//---
// BMA: Wishlist / notes:
//
// * Review use of linear algebra and dimensions throughout
//
// Convergence criteria:
//  * Consider using a single tolerance and using the derivative to
//    estimate the relationship between the ytol and svdtol
//  * Warn if convergenceTol < macheps since we reset it
//  * nullspaceTol needs to be bounded below too
//  * Add a convergence manager class
//  * Subtract UQ samples from the original function budget
//  * User control of bootstrap and choice of gap detection
//
// Algorithm enhancements
//  * Build surrogate in inactive dirs based on conditional expectation
//  * Add linear constraints to this Model for the recast domain
// 
// Architecture: 
//  * Be able to construct this Model type directly from problem db and
//    offer a user option.
//  * Verbosity control
//---

/// Active subspace model for input (variable space) reduction

/** Specialization of a RecastModel that identifies an active
    subspace during build phase and creates a RecastModel in the
    reduced space */
class ActiveSubspaceModel: public RecastModel
{
public:
  
  //
  //- Heading: Constructor and destructor
  //

  /// standard constructor
  ActiveSubspaceModel(const Model& sub_model, 
		      int random_seed, int initial_samples, int batch_size,
		      double conv_tol, size_t max_iter, size_t max_evals,
          unsigned short subspace_id_method);

  /// destructor
  ~ActiveSubspaceModel();

  /// sample the full space to identify the subspace, and finish
  /// initializing the RecastModel so we have valid variables/responses
  void initialize();


  unsigned short subspace_id() const;
  void subspace_id(short id);
  

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void derived_init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				  bool recurse_flag);

  void derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				 bool recurse_flag);

  void derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				  bool recurse_flag);

  // ---
  // Construct time convenience functions
  // ---

  /// initialize the native problem space Monte Carlo sampler
  void init_fullspace_sampler();

  // ---
  // Subspace identification functions: rank-revealing build phase
  // ---

  // Iteratively sample the fullspace model until subspace identified
  // that meets user-specified criteria
  void identify_subspace();

  /// generate fullspace samples, append to matrix, and factor,
  /// returning whether tolerance met
  void expand_basis(bool& svtol_met);

  /// determine the number of full space samples for next iteration,
  /// based on batchSize, limiting by remaining function evaluation
  /// budget
  unsigned int calculate_fullspace_samples();

  /// sample the derivative at diff_samples points and leave temporary
  /// in dace_iterator
  void generate_fullspace_samples(unsigned int diff_samples); 

  /// append the fullspaceSampler samples to the derivative and vars matrices
  void append_sample_matrices(unsigned int diff_samples);

  /// factor the derivative matrix and analyze singular values,
  /// assessing convergence and rank, returning whether tolerance met
  void compute_svd(bool& svtol_met);

  /// compute Bing Li's criterion to identify the active subspace
  void computeBingLiCriterion(RealVector& singular_values, bool& svtol_met);

  /// compute Constantine's metric to identify the active subspace
  void computeConstantineMetric(RealVector& singular_values, bool& svtol_met);

  /// print inner iteration stats after SVD
  void print_svd_stats();

  /// determine if the reduced basis yields acceptable reconstruction
  /// error, based on sampling in the orthogonal complement of the
  /// reduced basis
  void assess_reconstruction(bool& recon_tol_met);


  // ---
  // Problem transformation functions
  // ---

  /// Initialize the base class RecastModel with reduced space variable sizes
  void initialize_recast();

  /// Create a variables components totals array with the reduced space
  /// size for continuous variables
  SizetArray variables_resize();

  /// translate the characterization of uncertain variables in the
  /// native_model to the reduced space of the transformed model
  void uncertain_vars_to_subspace();

  /// transform the original bounded domain (and any existing linear
  /// constraints) into linear constraints in the reduced space
  void update_linear_constraints();

  
  // ---
  // Callback functions that perform data transform during the Recast operations
  // ---

  /// map the active continuous recast variables to the active
  /// submodel variables (linear transformation)
  static void vars_mapping(const Variables& recast_xi_vars, 
			   Variables& sub_model_x_vars);

  /// map the inbound ActiveSet to the sub-model (map derivative variables)
  static void set_mapping(const Variables& recast_vars,
			  const ActiveSet& recast_set,
			  ActiveSet& sub_model_set);

  // ---
  // Member data
  // ---

  /// seed controlling all samplers
  int randomSeed;

  // Build phase controls

  /// initial number of samples at which to query the truth model
  int initialSamples;

  /// number of points to add at each iteration
  int batchSize;
  /// maximum number of build iterations
  int maxIterations;
  /// maximum number of build evaluations
  int maxFunctionEvals;

  /// tolerance on singular value ratio, max of user-specified and macheps
  double svTol;

  /// boolean flag to determine if reconstruction assessment is performed
  bool performAssessment;

  /// user-specified tolerance on nullspace 
  double nullspaceTol;

  /// Contains which method should be used to identify active subspace dimension
  unsigned short subspaceIdMethod;

  /// Number of bootstrap samples for Bing Li criterion
  size_t numReplicates;

  // ---
  // TODO: add these criteria

  /// max bases to retain
  //  int maxBases;

  /// minimum variance to capture in reduced basis
  //  double minVariance;

  // ---

  // Number of fullspace active continuous variables
  size_t numFullspaceVars;
  // Total number of response functions
  size_t numFunctions;

  /// current iteration
  unsigned int currIter;

  /// total construction samples evaluated so far
  unsigned int totalSamples;

  /// total evaluations of model (accounting for UQ phase)
  unsigned int totalEvals;

  /// current singular value ratio (sigma_k/sigma_0)
  double svRatio;


  // Data for numerical representation

  /// current approximation of system rank
  unsigned int reducedRank;

  /// basis for the reduced subspace
  RealMatrix reducedBasis;

  /// matrix of derivative data with numFunctions columns per fullspace sample;
  /// each column contains the gradient of one function at one sample point, 
  /// so total matrix size is numContinuousVars * (numFunctions * numSamples)
  /// [ D1 | D2 | ... | Dnum_samples]
  /// [ dy1/dx(k=1) | dy2/dx(k=1) | ... | dyM/dx(k=1) | k=2 | ... | k=n_s ]
  RealMatrix derivativeMatrix;

  /// matrix of the left singular vectors of derivativeMatrix
  RealMatrix leftSingularVectors;

  /// matrix of fullspace variable points samples
  /// size numContinuousVars * (numSamples)
  RealMatrix varsMatrix;

  /// Gradient scaling factors to make multiple response function gradients
  /// similar orders of magnitude.
  RealArray gradientScaleFactors;


  // Helper members

  /// Monte Carlo sampler for the full parameter space
  Iterator fullspaceSampler;

  /// static pointer to this class for use in static callbacks
  static ActiveSubspaceModel* asmInstance;

  /// the index of the active metaiterator-iterator parallelism level
  /// (corresponding to ParallelConfiguration::miPLIters) used at runtime
  size_t miPLIndex;

};

} // namespace Dakota

#endif
