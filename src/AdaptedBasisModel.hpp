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

// define special values for componentParallelMode
#define CONFIG_PHASE 0
#define OFFLINE_PHASE 1
#define ONLINE_PHASE 2

/// forward declarations
class ProblemDescDB;

/// Adapted basis model for input (variable space) reduction

/** Specialization of a RecastModel that creates an adapted basis model
    during build phase and creates a RecastModel in the reduced space */
class AdaptedBasisModel: public RecastModel
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// Problem database constructor
  AdaptedBasisModel(ProblemDescDB& problem_db);

  /// destructor
  ~AdaptedBasisModel();


  //
  //- Heading: Virtual function redefinitions
  //

  bool initialize_mapping(ParLevLIter pl_iter);
  bool finalize_mapping();
  bool mapping_initialized();

  /// called from IteratorScheduler::init_iterator() for iteratorComm rank 0 to
  /// terminate serve_init_mapping() on other iteratorComm processors
  void stop_init_mapping(ParLevLIter pl_iter);
  /// called from IteratorScheduler::init_iterator() for iteratorComm rank != 0
  /// to balance resize() calls on iteratorComm rank 0
  int serve_init_mapping(ParLevLIter pl_iter);


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

  /// update component parallel mode for supporting parallelism in
  /// the offline and online phases
  void component_parallel_mode(short mode);

  /// Service the offline and online phase job requests received from the
  /// master; completes when termination message received from stop_servers().
  void serve_run(ParLevLIter pl_iter, int max_eval_concurrency);

  /// Executed by the master to terminate the offline and online phase
  /// server operations when iteration on the AdaptedBasisModel is complete
  void stop_servers();

  // ---
  // Construct time convenience functions
  // ---

  /// retrieve the sub-Model from the DB to pass up the constructor chain
  Model get_sub_model(ProblemDescDB& problem_db);

  /// validate the build controls and set defaults
  void validate_inputs();


  // ---
  // Subspace identification functions: rank-revealing build phase
  // ---

  // Iteratively sample the fullspace model until subspace identified
  // that meets user-specified criteria
  void identify_subspace();


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

  /// update variable labels
  void update_var_labels();


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

  /// map responses from the sub-model to the recast model
  static void response_mapping(const Variables& recast_y_vars,
                               const Variables& sub_model_x_vars,
                               const Response& sub_model_resp,
                               Response& recast_resp);


  // ---
  // Member data
  // ---

  /// boolean flag to determine if variables should be transformed to u-space
  /// before active subspace initialization
  bool transformVars;

  /// Number of fullspace active continuous variables
  size_t numFullspaceVars;

  /// Total number of response functions
  size_t numFunctions;

  /// boolean flag to determine if mapping has been fully initialized
  bool adaptedBasisInitialized;

  // Data for numerical representation

  /// current approximation of system rank
  unsigned int reducedRank;

  /// basis for the reduced subspace
  RealMatrix A;

  /// the truth model which provides evaluations for building the active subspace
  Model actualModel;

  /// the result of performing a Nataf transformation to u-space from the truth
  /// model
  Model transformModel;

  /// model containing a surrogate built over the active subspace
  Model surrogateModel;


  // Helper members

  /// static pointer to this class for use in static callbacks
  static AdaptedBasisModel* abmInstance;

  /// the index of the active metaiterator-iterator parallelism level
  /// (corresponding to ParallelConfiguration::miPLIters) used at runtime
  size_t miPLIndex;

  /// Concurrency to use once subspace has been built.
  int onlineEvalConcurrency;
  /// Concurrency to use when building subspace.
  int offlineEvalConcurrency;

};

} // namespace Dakota

#endif
