/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef ACTIVE_SUBSPACE_MODEL_H
#define ACTIVE_SUBSPACE_MODEL_H

#include "SubspaceModel.hpp"
#include "DakotaIterator.hpp"

namespace Dakota {

/// forward declarations
class ProblemDescDB;


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

/** Specialization of a RecastModel that identifies an active subspace
    during build phase and creates a RecastModel in the reduced space */

class ActiveSubspaceModel: public SubspaceModel
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// Problem database constructor
  ActiveSubspaceModel(ProblemDescDB& problem_db);

  /// lightweight constructor
  ActiveSubspaceModel(const Model& sub_model, unsigned int dimension,
                      const RealMatrix &rotation_matrix, short output_level);

  /// destructor
  ~ActiveSubspaceModel();

  //
  //- Heading: Virtual function redefinitions
  //

  //bool initialize_mapping(ParLevLIter pl_iter);
  //bool finalize_mapping();
  //bool resize_pending() const;

  //void stop_init_mapping(ParLevLIter pl_iter);
  //int serve_init_mapping(ParLevLIter pl_iter);

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

  void derived_evaluate(const ActiveSet& set);
  void derived_evaluate_nowait(const ActiveSet& set);
  const IntResponseMap& derived_synchronize();
  const IntResponseMap& derived_synchronize_nowait();

  /*
  /// update component parallel mode for supporting parallelism in
  /// the offline and online phases
  void component_parallel_mode(short mode);

  /// Service the offline and online phase job requests received from the
  /// master; completes when termination message received from stop_servers().
  void serve_run(ParLevLIter pl_iter, int max_eval_concurrency);

  /// Executed by the master to terminate the offline and online phase
  /// server operations when iteration on the ActiveSubspaceModel is complete
  void stop_servers();
  */

  void validate_inputs();

  void assign_instance();

  // ---
  // Construct time convenience functions
  // ---

  /// retrieve the sub-Model from the DB to pass up the constructor chain
  Model get_sub_model(ProblemDescDB& problem_db);

  /// initialize the native problem space Monte Carlo sampler
  void init_fullspace_sampler(unsigned short sample_type);


  // ---
  // Subspace identification functions: rank-revealing build phase
  // ---

  /// sample the model's gradient, computed the SVD, and form the active
  /// subspace rotation matrix.
  void compute_subspace();

  /// helper for shared code between lightweight ctor and initialize_mapping()
  void initialize_subspace();

  /// sample the derivative at diff_samples points and leave temporary
  /// in dace_iterator
  void generate_fullspace_samples(unsigned int diff_samples);

  /// populate the derivative and vars matrices with fullspaceSampler samples
  void populate_matrices(unsigned int diff_samples);

  /// factor the derivative matrix and analyze singular values,
  /// assessing convergence and rank, returning whether tolerance met
  void compute_svd();

  /// use the truncation methods to identify the size of an active subspace
  void truncate_subspace();

  /// compute Bing Li's criterion to identify the active subspace
  unsigned int compute_bing_li_criterion(RealVector& singular_values);

  /// compute Constantine's metric to identify the active subspace
  unsigned int compute_constantine_metric(RealVector& singular_values);

  /// Compute active subspace size based on eigenvalue energy. Compatible with
  /// other truncation methods.
  unsigned int compute_energy_criterion(RealVector& singular_values);

  /// Use cross validation of a moving least squares surrogate to identify the
  /// size of an active subspace that meets an error tolerance
  unsigned int compute_cross_validation_metric();

  /// Build moving least squares surrogate over candidate active subspace
  Real build_cv_surrogate(Model &cv_surr_model, RealMatrix training_x,
                          IntResponseMap training_y,
                          RealMatrix test_x, IntResponseMap test_y);


  // Helper functions for the cross validation metric:

  unsigned int determine_rank_cv(const std::vector<Real> &cv_error);

  unsigned int min_index(const std::vector<Real> &cv_error);

  unsigned int tolerance_met_index(const std::vector<Real> &cv_error,
                                   Real tolerance, bool &tol_met);

  std::vector<Real> negative_diff(const std::vector<Real> &cv_error);


  /// Build surrogate over active subspace
  void build_surrogate();


  // ---
  // Problem transformation functions
  // ---

  /// Create a variables components totals array with the reduced space
  /// size for continuous variables
  SizetArray resize_variable_totals();

  /// translate the characterization of uncertain variables in the
  /// native_model to the reduced space of the transformed model
  void uncertain_vars_to_subspace();

  // ---
  // Callback functions that perform data transform during the Recast operations
  // ---

  /// map the active continuous recast variables to the active
  /// submodel variables (linear transformation)
  static void variables_mapping(const Variables& recast_xi_vars,
				Variables& sub_model_x_vars);

  // ---
  // Member data
  // ---

  // Build phase controls

  /// initial number of samples at which to query the truth model
  int initialSamples;

  /// Boolean flag signaling use of Bing Li criterion to identify active
  /// subspace dimension
  bool subspaceIdBingLi;
  /// Boolean flag signaling use of Constantine criterion to identify active
  /// subspace dimension
  bool subspaceIdConstantine;
  /// Boolean flag signaling use of eigenvalue energy criterion to identify
  /// active subspace dimension
  bool subspaceIdEnergy;
  /// Boolean flag signaling use of cross validationto identify active
  /// subspace dimension
  bool subspaceIdCV;

  /// Number of bootstrap samples for subspace identification
  size_t numReplicates;

  /// boolean flag to determine if variables should be transformed to u-space
  /// before active subspace initialization
  bool transformVars;

  /// total construction samples evaluated so far
  unsigned int totalSamples;

  /// Normalization to use in the case of multiple QoI's
  unsigned short subspaceNormalization;


  // Data for numerical representation

  /// basis for the inactive subspace
  RealMatrix inactiveBasis;

  /// current inactive variables
  RealVector inactiveVars;

  /// matrix of derivative data with numFns columns per fullspace sample;
  /// each column contains the gradient of one function at one sample point,
  /// so total matrix size is numContinuousVars * (numFns * numSamples)
  /// [ D1 | D2 | ... | Dnum_samples]
  /// [ dy1/dx(k=1) | dy2/dx(k=1) | ... | dyM/dx(k=1) | k=2 | ... | k=n_s ]
  RealMatrix derivativeMatrix;

  /// matrix of the left singular vectors of derivativeMatrix
  RealMatrix leftSingularVectors;

  /// singular values of derivativeMatrix
  RealVector singularValues;

  /// matrix of fullspace variable points samples
  /// size numContinuousVars * (numSamples)
  RealMatrix varsMatrix;

  /// Gradient scaling factors to make multiple response function gradients
  /// similar orders of magnitude.
  RealArray gradientScaleFactors;

  /// Truncation tolerance for eigenvalue energy subspace identification
  Real truncationTolerance;

  bool cvIncremental;

  short cvIdMethod;

  Real cvRelTolerance;

  Real cvDecreaseTolerance;

  /// maximum subspace size to consider using cross validation
  unsigned int cvMaxRank;

  /// model containing a surrogate built over the active subspace
  Model surrogateModel;

  /// flag specifying whether or not a surrogate is built over the subspace
  bool buildSurrogate;

  /// Number of refinement samples to use when building a surrogate
  int refinementSamples;


  // Helper members

  /// Monte Carlo sampler for the full parameter space
  Iterator fullspaceSampler;

  /// static pointer to this class for use in static callbacks
  static ActiveSubspaceModel* asmInstance;

  /// map of responses returned in buildSurrogate mode
  IntResponseMap surrResponseMap;
  /// map from surrogateModel evaluation ids to RecastModel ids
  IntIntMap surrIdMap;
};


inline void ActiveSubspaceModel::assign_instance()
{ asmInstance = this; SubspaceModel::assign_instance(); }


inline unsigned int ActiveSubspaceModel::
min_index(const std::vector<Real> &cv_error)
{
  if (cv_error.empty()) // Return full rank since this shouldn't be empty
    return numFullspaceVars-1;

  Real min_val = cv_error[0];
  unsigned int ii, min_ind = 0;
  for (ii = 1; ii < cv_error.size(); ii++)
    if (cv_error[ii] < min_val)
      { min_val = cv_error[ii]; min_ind = ii; }

  return min_ind;
}


inline unsigned int ActiveSubspaceModel::
tolerance_met_index(const std::vector<Real> &cv_error, Real tolerance,
                    bool &tol_met)
{
  tol_met = false;
  for (unsigned int ii = 0; ii < cv_error.size(); ii++)
    if (cv_error[ii] < tolerance)
      { tol_met = true; return ii; }

  // Return full rank since tolerance is not met
  return numFullspaceVars-1;
}


inline std::vector<Real> ActiveSubspaceModel::
negative_diff(const std::vector<Real> &cv_error)
{
  std::vector<Real> neg_diff(cv_error.size()-1);
  for (unsigned int ii = 0; ii < neg_diff.size(); ii++)
    neg_diff[ii] = cv_error[ii] - cv_error[ii+1];

  return neg_diff;
}

} // namespace Dakota

#endif
