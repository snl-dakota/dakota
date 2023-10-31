/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef SUBSPACE_MODEL_H
#define SUBSPACE_MODEL_H

#include "RecastModel.hpp"

namespace Dakota {

// define special values for componentParallelMode
#define CONFIG_PHASE  0
#define OFFLINE_PHASE 1
#define ONLINE_PHASE  2

/// forward declarations
class ProblemDescDB;


/// Subspace model for input (variable space) reduction

/** Specialization of a RecastModel that identifies a subspace during
    build phase and creates a RecastModel in the reduced space */
class SubspaceModel: public RecastModel
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// Problem database constructor
  SubspaceModel(ProblemDescDB& problem_db, const Model& sub_model);

  /// lightweight constructor
  SubspaceModel(const Model& sub_model, unsigned int dimension,
		short output_level);

  /// destructor
  ~SubspaceModel();

  //
  //- Heading: Virtual function redefinitions
  //

  bool initialize_mapping(ParLevLIter pl_iter);
  //bool finalize_mapping();
  bool resize_pending() const;

  /// called from IteratorScheduler::init_iterator() for iteratorComm rank 0 to
  /// terminate serve_init_mapping() on other iteratorComm processors
  void stop_init_mapping(ParLevLIter pl_iter);

  /// called from IteratorScheduler::init_iterator() for iteratorComm rank != 0
  /// to balance resize() calls on iteratorComm rank 0
  int serve_init_mapping(ParLevLIter pl_iter);

  //
  //- Heading: accessors
  //

  /// return reducedBasis
  const RealMatrix& reduced_basis() const;

protected:

  //
  //- Heading: Virtual function redefinitions
  //

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
  /// server operations when iteration on the SubspaceModel is complete
  void stop_servers();

  void assign_instance();

  // ---
  // New virtual functions
  // ---

  /// validate the build controls and set defaults
  virtual void validate_inputs();


  // ---
  // Subspace identification functions: rank-revealing build phase
  // ---

  /// sample the model's gradient, computed the SVD, and form the active
  /// subspace rotation matrix.
  virtual void compute_subspace() = 0;

  /// helper for shared code between lightweight ctor and initialize_mapping()
  virtual void initialize_subspace();

  /// translate the characterization of uncertain variables in the
  /// native_model to the reduced space of the transformed model
  virtual void uncertain_vars_to_subspace();

  // ---
  // Problem transformation functions
  // ---

  /// Initialize the base class RecastModel with reduced space variable sizes
  void initialize_base_recast(
    void (*variables_map)      (const Variables& recast_vars,
				Variables& sub_model_vars),
    void (*set_map)            (const Variables& recast_vars,
				const ActiveSet& recast_set,
				ActiveSet& sub_model_set),
    void (*primary_resp_map)   (const Variables& sub_model_vars,
				const Variables& recast_vars,
				const Response& sub_model_response,
				Response& recast_response));

  /// Create a variables components totals array with the reduced space
  /// size for continuous variables
  SizetArray resize_variable_totals();

  /// transform the original bounded domain (and any existing linear
  /// constraints) into linear constraints in the reduced space
  void update_linear_constraints();

  /// update variable labels
  void update_var_labels();


  // ---
  // Callback functions that perform data transform during the Recast operations
  // ---

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

  /// seed controlling all samplers
  int randomSeed;

  /// Number of fullspace active continuous variables
  size_t numFullspaceVars;

  /// current approximation of system rank
  unsigned int reducedRank;
  /// basis for the reduced subspace
  RealMatrix reducedBasis;

  /// the index of the active metaiterator-iterator parallelism level
  /// (corresponding to ParallelConfiguration::miPLIters) used at runtime
  size_t miPLIndex;

  /// Concurrency to use once subspace has been built.
  int onlineEvalConcurrency;
  /// Concurrency to use when building subspace.
  int offlineEvalConcurrency;

  /// static pointer to this class for use in static member fn callbacks
  static SubspaceModel* smInstance;
};


inline void SubspaceModel::assign_instance()
{ smInstance = this; }


inline bool SubspaceModel::resize_pending() const
{ return !mappingInitialized; }


inline const RealMatrix& SubspaceModel::reduced_basis() const
{ return reducedBasis; }


inline void SubspaceModel::stop_servers()
{ component_parallel_mode(CONFIG_PHASE); }

} // namespace Dakota

#endif
