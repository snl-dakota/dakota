/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef RANDOM_FIELD_MODEL_H
#define RANDOM_FIELD_MODEL_H

#include "ReducedBasis.hpp"
#include "DakotaApproximation.hpp"
#include "RecastModel.hpp"
#include "DakotaIterator.hpp"

namespace Dakota {

/// forward declarations
class ProblemDescDB;

/// Random field model, capable of generating and then forward propagating

/** Specialization of a RecastModel that optionally identifies an
    approximate random field model during build phase and creates a
    RecastModel capable of performing forward UQ including the field
    and auxialliary uncertain variables reduced space.  This
    RandomFieldModel wraps the random field propagation model (not the
    RF-generating model) */
class RandomFieldModel: public RecastModel
{
public:
  
  //
  //- Heading: Constructor and destructor
  //

  /// Problem database constructor
  RandomFieldModel(ProblemDescDB& problem_db);

  /// destructor
  ~RandomFieldModel();


  //
  //- Heading: Virtual function redefinitions
  //

  /// for KL models, the model is augmented with the random coeffs of the KL
  bool initialize_mapping(ParLevLIter pl_iter);
  bool finalize_mapping();
  bool mapping_initialized() const;
  bool resize_pending() const;

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  // BMA TODO: need these because this Model and its underlying
  // model(s) will be used in different parallel configurations

  /*
  void derived_init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				  bool recurse_flag);

  void derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				 bool recurse_flag);

  void derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				  bool recurse_flag);
  */

  // ---
  // Construct time convenience functions
  // ---

  /// retrieve the sub-Model from the DB to pass up the constructor chain
  Model get_sub_model(ProblemDescDB& problem_db);

  /// initialize the RF-generating sampler
  void init_dace_iterator(ProblemDescDB& problem_db);

  /// validate the build controls and set defaults
  void validate_inputs();


  // ---
  // Convenience functions for reading and decomposing the field
  // ---

  /// Source data generation: get the field data either from file or
  /// simulation by running the DACE Iterator.  Populates rfBuildData.
  void get_field_data();

  /// Generate field representation: generate a KL or PCA/GP
  void identify_field_model();

  /// Generate field representation: utilize RF Suite
  void rf_suite_identify_field_model();

  // ---
  // Problem transformation functions
  // ---

  /// Initialize the base class RecastModel with reduced space variable sizes
  void initialize_recast();

  /// Create a variables components totals array with the reduced space
  /// size for continuous variables
  SizetArray variables_resize();

  /// For KL models, augment the subModel's uncertain variables with
  /// additional N(0,1) variables; set up AleatoryDistParams for the
  /// N(0,1)'s
  void initialize_rf_coeffs();

  
  // ---
  // Callback functions that perform data transform during the Recast
  // operations (forward propagation mode)
  // ---

  /// map the active continuous recast variables to the active
  /// submodel variables (linear transformation)
  static void vars_mapping(const Variables& recast_xi_vars, 
			   Variables& sub_model_x_vars);

  /// map the inbound ActiveSet to the sub-model (map derivative variables)
  static void set_mapping(const Variables& recast_vars,
			  const ActiveSet& recast_set,
			  ActiveSet& sub_model_set);

  /// generate a random field realization, then evaluate the submodel
  void derived_evaluate(const ActiveSet& set);
  /// generate a random field realization, then evaluate the submodel (asynch)
  void derived_evaluate_nowait(const ActiveSet& set);

  /// generate a KL realization and write to file
  void generate_kl_realization();

  /// generate a PCA/GP realization and write to file
  void generate_pca_gp_realization();

  /// write a field realization to console and file
  void write_field(const RealVector& field_prediction);

  // ---
  // Member data
  // ---

  // Total number of response functions
  size_t numFunctions;


  // ---
  // Data source
  // ---

  /// name of the data file with RF build data
  String rfDataFilename;
  /// rows of data matrix
  size_t numObservations;
  /// column partitions of data matrix
  IntVector fieldLengths;
  /// data matrix with realizations of the random field to approximate
  RealMatrix rfBuildData;

  /// matrix of samples used to build the RF data
  RealMatrix rfBuildVars;

  /// String dataDirectoryBasename;

  /// DACE Iterator to evaluate the RF generating model
  Iterator daceIterator;

  /// unsigned short analyticCovForm;


  // ---
  // Build controls (generate the RF representation) process
  // ---

  /// form of the RF representation (KL, PCA, ICA)
  unsigned short expansionForm;

  /// form of the analytic covariance function
  unsigned short covarianceForm;

  /// current approximation of system rank
  int requestedReducedRank;

  /// fraction of energy to capture
  Real percentVariance;

  /// command to run RF Suite
  //  String rfSuiteCmd;

  // ---
  // Data for both field representations
  // ---

  /// number of bases retained in decomposition
  int actualReducedRank;

  /// reduced basis representation (for KL or PCA case)
  ReducedBasis rfBasis;

  /// counter for RF Suite
  int fieldRealizationId;

  // ---
  // Data for PCA/GP model
  // ---

  /// approximate models used to map the uncertain vars through the PCA approx
  std::vector<Approximation> gpApproximations;


  // ---
  // Propagation (sub) model
  // ---
  
  // propModel: don't need to store: it's the subModel of the RecastModel

  // ---
  // Helper members
  // ---

  /// static pointer to this class for use in static callbacks
  static RandomFieldModel* rfmInstance;

  /// the index of the active metaiterator-iterator parallelism level
  /// (corresponding to ParallelConfiguration::miPLIters) used at runtime
  //  size_t miPLIndex;

  /// track use of initialize_mapping() and finalize_mapping()
  bool mappingInitialized;
};


inline bool RandomFieldModel::mapping_initialized() const
{ return mappingInitialized; }


inline bool RandomFieldModel::resize_pending() const
{ return (expansionForm == RF_KARHUNEN_LOEVE && !mappingInitialized); }

} // namespace Dakota

#endif
