/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef ADAPTED_BASIS_MODEL_H
#define ADAPTED_BASIS_MODEL_H

#include "SubspaceModel.hpp"
#include "DakotaIterator.hpp"

namespace Dakota {

/// forward declarations
class NonDPolynomialChaos;
class ParallelLibrary;
class ProblemDescDB;


/// Adapted basis model for input (variable space) reduction

/** Specialization of a RecastModel that creates an adapted basis model
    during build phase and creates a RecastModel in the reduced space */
class AdaptedBasisModel: public SubspaceModel
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// Problem database constructor
  AdaptedBasisModel(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib);

  /// DI constructor using model IR, own variables/response, and the
  /// injected truth model
  AdaptedBasisModel(std::shared_ptr<StudyServices> services,
		    const IRStore& model_store, const Variables& variables,
		    const Response& response, std::shared_ptr<Model> truth_model);

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void derived_init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                                  bool recurse_flag) override;

  void derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                                 bool recurse_flag) override;

  void derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                                  bool recurse_flag) override;

  // ---
  // Construct time convenience functions
  // ---

  /// retrieve the sub-Model from the DB to pass up the constructor chain
  std::shared_ptr<Model> get_sub_model(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib);
  /// DI counterpart: pilot PCE over an injected truth model
  std::shared_ptr<Model> get_sub_model(const IRStore& model_store,
				       std::shared_ptr<Model> truth_model);
  /// shared pilot PCE construction for both get_sub_model() overloads
  std::shared_ptr<Model>
  build_pilot_pce(std::shared_ptr<Model> actual_model, unsigned short ssg_level,
		  unsigned short exp_order, Real colloc_ratio, short refine_type,
		  short refine_cntl, short cov_cntl, short rule_nest,
		  short rule_growth, bool pw_basis, bool use_derivs,
		  bool cross_valid);

  // ---
  // Subspace identification functions: rank-revealing build phase
  // ---

  // Iteratively sample the fullspace model until subspace identified
  // that meets user-specified criteria
  void compute_subspace() override;

  /// use the truncation methods to identify the size of a reduced subspace
  void truncate_rotation();

  // ---
  // Problem transformation functions
  // ---

  /// translate the characterization of uncertain variables in the
  /// native_model to the reduced space of the transformed model
  void uncertain_vars_to_subspace() override;
  
  void validate_inputs() override;

  // ---
  // Callback functions that perform data transform during the Recast operations
  // ---

  /// map the active continuous recast variables to the active
  /// submodel variables (linear transformation)
  static void variables_mapping(const Variables& recast_xi_vars,
				Variables& sub_model_x_vars);
  
  
  /// store the rotation_method input specification, prior to run-time
  /// Options right now:
  ///   - linear = use the linear PCE coefficients
  ///   - norm = use normalized sensitivity along each direction
  short method_rotation;
  // Threshold level for the selection of the rank in the basis adaptation
  Real adaptedBasisTruncationTolerance;
  
  int subspaceDimension;

  // ---
  // Member data
  // ---

  /// BMA TODO: The initialization order of this Model, base
  /// RecastModel, and interdependence with PCE and its sub-model need
  /// fixing. Cannot make this a shared_ptr as it'll get default
  /// constructed and cleared after get_sub_model is called. Leaving
  /// as Iterator* for now, but we're just getting lucky with
  /// initialization (would probably break in a DEBUG build.

  /// PCE representation pointer that is initialized in get_sub_model() and
  /// then assigned into pcePilotExpansion in the constructor initializer list
  NonDPolynomialChaos* pcePilotExpRepPtr;
  /// low-order (linear or quadratic) PCE generator for computing rotation
  /// matrices A_i for each of the QoI; this is low-order and potentially
  /// high-dimension whereas a client PCE could be high-order in the
  /// reduced dimension
  std::shared_ptr<Iterator> pcePilotExpansion;
};

} // namespace Dakota

#endif
