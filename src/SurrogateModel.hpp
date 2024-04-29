/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef SURROGATE_MODEL_H
#define SURROGATE_MODEL_H

#include "DakotaModel.hpp"
#include "DakotaInterface.hpp"
#include "DiscrepancyCorrection.hpp"
#include "DataMethod.hpp"


namespace Dakota {

class ProblemDescDB;
class ParallelLibrary;

/// Base class for surrogate models (DataFitSurrModel and EnsembleSurrModel).

/** The SurrogateModel class provides common functions to derived
    classes for computing and applying corrections to approximations. */

class SurrogateModel: public Model
{
public:

protected:

  //
  //- Heading: Constructor and destructor
  //

  /// constructor
  SurrogateModel(ProblemDescDB& problem_db);
  /// alternate constructor
  SurrogateModel(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib,
		 const ShortShortPair& surr_view,
		 const SharedVariablesData& svd, bool share_svd,
		 const SharedResponseData&  srd, bool share_srd,
		 const ActiveSet& surr_set, short corr_type,
		 short output_level);
  /// destructor
  ~SurrogateModel();

  //
  //- Heading: Virtual function redefinitions
  //

  Pecos::ProbabilityTransformation& probability_transformation();

  void activate_distribution_parameter_derivatives();
  void deactivate_distribution_parameter_derivatives();

  void trans_U_to_X(const RealVector& u_c_vars, RealVector& x_c_vars);
  void trans_X_to_U(const RealVector& x_c_vars, RealVector& u_c_vars);

  void trans_grad_X_to_U(const RealVector& fn_grad_x, RealVector& fn_grad_u,
			 const RealVector& x_vars);
  void trans_grad_U_to_X(const RealVector& fn_grad_u, RealVector& fn_grad_x,
			 const RealVector& x_vars);
  void trans_grad_X_to_S(const RealVector& fn_grad_x, RealVector& fn_grad_s,
			 const RealVector& x_vars);
  void trans_hess_X_to_U(const RealSymMatrix& fn_hess_x,
			 RealSymMatrix& fn_hess_u, const RealVector& x_vars,
			 const RealVector& fn_grad_x);

  //bool initialize_mapping(ParLevLIter pl_iter);
  //bool finalize_mapping();

  /// return truth_model()
  Model& subordinate_model();

  void active_model_key(const Pecos::ActiveKey& key);
  const Pecos::ActiveKey& active_model_key() const;

  short surrogate_response_mode() const;
  short correction_type() const;
  short correction_order() const;

  /// return the current evaluation id for this Model
  int derived_evaluation_id() const;

  /// return miPLIndex
  size_t mi_parallel_level_index() const;

  //
  //- Heading: New virtual functions
  //

  /// verify compatibility between SurrogateModel attributes and
  /// attributes of the submodel (DataFitSurrModel::actualModel or
  /// EnsembleSurrModel::truthModel)
  virtual void check_submodel_compatibility(const Model& sub_model) = 0;
  /// initialize model with data that could change once per set of evaluations
  /// (e.g., an outer iterator execution), including active variable labels,
  /// inactive variable values/bounds/labels, and linear/nonlinear constraint
  /// coeffs/bounds
  virtual void init_model(Model& model);
  /// update model with data that could change per function evaluation
  /// (active variable values/bounds)
  virtual void update_model(Model& model);
  /// update current variables/labels/bounds/targets with data from model
  virtual void update_from_model(const Model& model);

  /// compute start index for inserting response data into aggregated response
  virtual size_t insert_response_start(size_t position);
  /// insert a single response into an aggregated response in the
  /// specified position
  virtual void insert_metadata(const RealArray& md, size_t position,
			       Response& agg_response);

  //
  //- Heading: Member functions
  //

  /// check sub_model for consistency in active variable counts
  bool check_active_variables(const Model& sub_model);
  /// check sub_model for consistency in inactive variable counts
  bool check_inactive_variables(const Model& sub_model);
  /// check sub_model for consistency in response QoI counts
  bool check_response_qoi(const Model& sub_model);

  /// initialize model with linear/nonlinear constraint data that could change
  /// once per set of evaluations (e.g., an outer iterator execution)
  void init_model_constraints(Model& model);
  /// initialize model with active/inactive variable label data that could
  /// change once per set of evaluations (e.g., an outer iterator execution)
  void init_model_labels(Model& model);
  /// initialize model with inactive variable values/bounds data that could
  /// change once per set of evaluations (e.g., an outer iterator execution)
  void init_model_inactive_variables(Model& model);
  /// initialize model with inactive variable labels that could change once
  /// per set of evaluations (e.g., an outer iterator execution)
  void init_model_inactive_labels(Model& model);

  /// update incoming (sub-)model with active bounds from userDefinedConstraints
  void update_model_active_constraints(Model& model);
  /// update model with random variable distribution data
  void update_model_distributions(Model& model);

  /// update current variables/bounds with data from model
  void update_variables_from_model(const Model& model);
  /// update current random variable distributions with data from model
  void update_distributions_from_model(const Model& model);
  /// update response/constraints with data from model
  void update_response_from_model(const Model& model);

  /// check for consistency in response map keys
  void check_key(int key1, int key2) const;

  /// evaluate whether a rebuild of the approximation should be
  /// forced based on changes in the inactive data
  bool check_rebuild(const RealVector& ref_icv, const IntVector& ref_idiv,
    const StringMultiArray& ref_idsv, const RealVector& ref_idrv,
    const RealVector& ref_c_l_bnds,   const RealVector& ref_c_u_bnds,
    const IntVector&  ref_di_l_bnds,  const IntVector&  ref_di_u_bnds,
    const RealVector& ref_dr_l_bnds,  const RealVector& ref_dr_u_bnds);

  /// reconstitutes a combined_asv from actual_asv and approx_asv
  void asv_combine(const ShortArray& actual_asv, const ShortArray& approx_asv,
		   ShortArray& combined_asv);
  /// overlays actual_response and approx_response to update combined_response
  void response_combine(const Response& actual_response,
                        const Response& approx_response,
                        Response& combined_response);

  /// aggregate 2 sets of response data to create a new response with 2x size
  void aggregate_response(const Response& resp1, const Response& resp2,
			  Response& agg_resp);
  // aggregate response array to create a new response with accumulated size
  //void aggregate_response(const ResponseArray& resp_array,Response& agg_resp);
  /// insert a single response into an aggregated response in the
  /// specified position
  void insert_response(const Response& response, size_t position,
		       Response& agg_response);

  //
  //- Heading: Data
  //

  /// for mixed response sets, this array specifies the response function
  /// subset that is approximated
  SizetSet surrogateFnIndices;

  /// an enumeration that controls the response calculation mode in
  /// {DataFit,Ensemble}SurrModel approximate response computations
  short responseMode;

  /// array of indices that identify the currently active model key
  Pecos::ActiveKey activeKey;

  /// type of correction: additive, multiplicative, or combined
  short corrType;
  /// order of correction: 0 (value), 1 (gradient), or 2 (Hessian)
  short corrOrder;
  /// map of raw continuous variables used by apply_correction().
  /// Model::varsList cannot be used for this purpose since it does
  /// not contain lower level variables sets from finite differencing.
  IntVariablesMap rawVarsMap;

  /// counter for calls to derived_evaluate()/derived_evaluate_nowait();
  /// used to key response maps from SurrogateModels
  int surrModelEvalCntr;
  /// map of surrogate responses returned by derived_synchronize() and
  /// derived_synchronize_nowait()
  IntResponseMap surrResponseMap;

  /// number of calls to build_approximation()
  /** used as a flag to automatically build the approximation if one of the
      derived evaluate functions is called prior to build_approximation(). */
  size_t approxBuilds;

  /// the index of the active metaiterator-iterator parallelism level
  /// (corresponding to ParallelConfiguration::miPLIters) used at runtime
  size_t miPLIndex;

private:

  //
  //- Heading: convenience functions
  //

  /// update all current variables/bounds/labels with data from model
  void update_all_variables_from_model(const Model& model);
  /// update complement of active variables/bounds with data from model
  void update_complement_variables_from_model(const Model& model);

  //
  //- Heading: Data
  //

};


inline SurrogateModel::~SurrogateModel()
{ }


inline Pecos::ProbabilityTransformation& SurrogateModel::
probability_transformation()
{ return truth_model().probability_transformation(); } // forward along


inline void SurrogateModel::activate_distribution_parameter_derivatives()
{ truth_model().activate_distribution_parameter_derivatives(); }


inline void SurrogateModel::deactivate_distribution_parameter_derivatives()
{ truth_model().deactivate_distribution_parameter_derivatives(); }


inline void SurrogateModel::
trans_X_to_U(const RealVector& x_c_vars, RealVector& u_c_vars)
{ truth_model().trans_X_to_U(x_c_vars, u_c_vars); }


inline void SurrogateModel::
trans_U_to_X(const RealVector& u_c_vars, RealVector& x_c_vars)
{ truth_model().trans_U_to_X(u_c_vars, x_c_vars); }


inline void SurrogateModel::
trans_grad_X_to_U(const RealVector& fn_grad_x, RealVector& fn_grad_u,
		  const RealVector& x_vars)
{ truth_model().trans_grad_X_to_U(fn_grad_x, fn_grad_u, x_vars); }


inline void SurrogateModel::
trans_grad_U_to_X(const RealVector& fn_grad_u, RealVector& fn_grad_x,
		  const RealVector& x_vars)
{ truth_model().trans_grad_U_to_X(fn_grad_u, fn_grad_x, x_vars); }


inline void SurrogateModel::
trans_grad_X_to_S(const RealVector& fn_grad_x, RealVector& fn_grad_s,
		  const RealVector& x_vars)
{ truth_model().trans_grad_X_to_S(fn_grad_x, fn_grad_s, x_vars); }


inline void SurrogateModel::
trans_hess_X_to_U(const RealSymMatrix& fn_hess_x,
		  RealSymMatrix& fn_hess_u, const RealVector& x_vars,
		  const RealVector& fn_grad_x)
{ truth_model().trans_hess_X_to_U(fn_hess_x, fn_hess_u, x_vars, fn_grad_x); }


inline Model& SurrogateModel::subordinate_model()
{ return truth_model(); }


inline void SurrogateModel::active_model_key(const Pecos::ActiveKey& key)
{
  // base implementation (augmented in derived SurrogateModels)
  activeKey = key;//.copy(); // share representations except for entering data into DB storage (reduce overhead of short-term activations)
}


inline const Pecos::ActiveKey& SurrogateModel::active_model_key() const
{return activeKey; }


inline short SurrogateModel::surrogate_response_mode() const
{ return responseMode; }


inline size_t SurrogateModel::mi_parallel_level_index() const
{ return miPLIndex; }


inline void SurrogateModel::check_key(int key1, int key2) const
{
  if (key1 != key2) {
    Cerr << "Error: failure in SurrogateModel::check_key().  Keys are not "
	 << "consistent." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


inline short SurrogateModel::correction_type() const
{ return corrType; }


inline short SurrogateModel::correction_order() const
{ return corrOrder; }


/*
inline const UShortArray& SurrogateModel::
model_indices(const Pecos::ActiveKeyData& data) const
{ return data.model_indices(); }


inline void SurrogateModel::
model_indices(Pecos::ActiveKeyData& data, const UShortArray& indices)
{ data.model_indices(indices); }
*/


/** return the SurrogateModel evaluation id counter.  Due to possibly
    intermittent use of lower level components, this is not the same as
    approxInterface, actualModel, or orderedModels evaluation counts,
    which requires a consistent evaluation rekeying process. */
inline int SurrogateModel::derived_evaluation_id() const
{ return surrModelEvalCntr; }

} // namespace Dakota

#endif
