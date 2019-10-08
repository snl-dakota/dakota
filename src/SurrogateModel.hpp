/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SurrogateModel
//- Description: A model which provides a surrogate for a truth model.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: SurrogateModel.hpp 7024 2010-10-16 01:24:42Z mseldre $

#ifndef SURROGATE_MODEL_H
#define SURROGATE_MODEL_H

#include "DakotaModel.hpp"
#include "DakotaInterface.hpp"
#include "DiscrepancyCorrection.hpp"
#include "DataMethod.hpp"


namespace Dakota {

class ProblemDescDB;
class ParallelLibrary;

/// Base class for surrogate models (DataFitSurrModel and HierarchSurrModel).

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
		 const SharedVariablesData& svd, bool share_svd,
		 const SharedResponseData&  srd, bool share_srd,
		 const ActiveSet& set, short corr_type, short output_level);
  /// destructor
  ~SurrogateModel();

  //
  //- Heading: Virtual function redefinitions
  //

  Pecos::ProbabilityTransformation& probability_transformation();

  void activate_distribution_parameter_derivatives();
  void deactivate_distribution_parameter_derivatives();

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

  void surrogate_model_key(unsigned short model_index,
			   unsigned short soln_lev_index);
  void surrogate_model_key(const UShortArray& key);
  const UShortArray& surrogate_model_key() const;

  void truth_model_key(unsigned short model_index,
		       unsigned short soln_lev_index);
  void truth_model_key(const UShortArray& key);
  const UShortArray& truth_model_key() const;

  /// return responseMode
  short surrogate_response_mode() const;

  /// return the current evaluation id for this Model
  int derived_evaluation_id() const;

  /// return miPLIndex
  size_t mi_parallel_level_index() const;

  //
  //- Heading: New virtual functions
  //

  /// distributes the incoming orig_asv among actual_asv and approx_asv
  virtual void asv_split(const ShortArray& orig_asv, ShortArray& actual_asv,
			 ShortArray& approx_asv, bool build_flag);

  /// verify compatibility between SurrogateModel attributes and
  /// attributes of the submodel (DataFitSurrModel::actualModel or
  /// HierarchSurrModel::highFidelityModel)
  virtual void check_submodel_compatibility(const Model& sub_model);

  //
  //- Heading: Member functions
  //

  /// return the level index from active low fidelity model key
  unsigned short surrogate_level_index() const;
  /// return the level index from active high fidelity model key
  unsigned short truth_level_index() const;

  /// check for consistency in response map keys
  void check_key(int key1, int key2) const;

  /// evaluate whether a rebuild of the approximation should be
  /// forced based on changes in the inactive data
  bool force_rebuild();

  /// reconstitutes a combined_asv from actual_asv and approx_asv
  void asv_combine(const ShortArray& actual_asv, const ShortArray& approx_asv,
		   ShortArray& combined_asv);
  /// overlays actual_response and approx_response to update combined_response
  void response_combine(const Response& actual_response,
                        const Response& approx_response,
                        Response& combined_response);
  /// aggregate {HF,LF} response data to create a new response with 2x size
  void aggregate_response(const Response& hf_resp, const Response& lf_resp,
			  Response& agg_resp);

  //
  //- Heading: Data
  //

  /// for mixed response sets, this array specifies the response function
  /// subset that is approximated
  IntSet surrogateFnIndices;

  /// an enumeration that controls the response calculation mode in
  /// {DataFit,Hierarch}SurrModel approximate response computations
  /** SurrBasedLocalMinimizer toggles this mode since compute_correction()
      does not back out old corrections. */
  short responseMode;

  /// array of indices that identify the surrogate (e.g., low fidelity) model
  /// that is currently active within orderedModels
  UShortArray surrModelKey;
  /// array of indices that identify the truth (e.g., high fidelity) model that
  /// is currently active within orderedModels
  UShortArray truthModelKey;

  /// type of correction: additive, multiplicative, or combined
  short corrType;

  /// map from actualModel/highFidelityModel evaluation ids to
  /// DataFitSurrModel/HierarchSurrModel ids
  IntIntMap truthIdMap;
  /// map from approxInterface/lowFidelityModel evaluation ids to
  /// DataFitSurrModel/HierarchSurrModel ids
  IntIntMap surrIdMap;

  /// counter for calls to derived_evaluate()/derived_evaluate_nowait();
  /// used to key response maps from SurrogateModels
  int surrModelEvalCntr;
  /// map of surrogate responses returned by derived_synchronize() and
  /// derived_synchronize_nowait()
  IntResponseMap surrResponseMap;
  /// map of approximate responses retrieved in derived_synchronize_nowait()
  /// that could not be returned since corresponding truth model response
  /// portions were still pending.
  IntResponseMap cachedApproxRespMap;
  /// map of raw continuous variables used by apply_correction().
  /// Model::varsList cannot be used for this purpose since it does
  /// not contain lower level variables sets from finite differencing.
  IntVariablesMap rawVarsMap;

  /// number of calls to build_approximation()
  /** used as a flag to automatically build the approximation if one of the
      derived evaluate functions is called prior to build_approximation(). */
  size_t approxBuilds;

  /// the index of the active metaiterator-iterator parallelism level
  /// (corresponding to ParallelConfiguration::miPLIters) used at runtime
  size_t miPLIndex;

  /// stores a reference copy of active continuous lower bounds when the
  /// approximation is built; used to detect when a rebuild is required.
  RealVector referenceCLBnds;
  /// stores a reference copy of active continuous upper bounds when the
  /// approximation is built; used to detect when a rebuild is required.
  RealVector referenceCUBnds;
  /// stores a reference copy of active discrete int lower bounds when the
  /// approximation is built; used to detect when a rebuild is required.
  IntVector referenceDILBnds;
  /// stores a reference copy of active discrete int upper bounds when the
  /// approximation is built; used to detect when a rebuild is required.
  IntVector referenceDIUBnds;
  /// stores a reference copy of active discrete real lower bounds when the
  /// approximation is built; used to detect when a rebuild is required.
  RealVector referenceDRLBnds;
  /// stores a reference copy of active discrete real upper bounds when the
  /// approximation is built; used to detect when a rebuild is required.
  RealVector referenceDRUBnds;

  /// stores a reference copy of the inactive continuous variables when the
  /// approximation is built using a Distinct view; used to detect when a
  /// rebuild is required.
  RealVector referenceICVars;
  /// stores a reference copy of the inactive discrete int variables when
  /// the approximation is built using a Distinct view; used to detect when
  /// a rebuild is required.
  IntVector referenceIDIVars;
  /// stores a reference copy of the inactive discrete string variables when
  /// the approximation is built using a Distinct view; used to detect when
  /// a rebuild is required.
  StringMultiArray referenceIDSVars;
  /// stores a reference copy of the inactive discrete real variables when
  /// the approximation is built using a Distinct view; used to detect when
  /// a rebuild is required.
  RealVector referenceIDRVars;

private:

  //
  //- Heading: Data
  //

  /// copy of the truth model variables object used to simplify conversion 
  /// among differing variable views in force_rebuild()
  Variables   truthModelVars;
  /// copy of the truth model constraints object used to simplify conversion 
  /// among differing variable views in force_rebuild()
  Constraints truthModelCons;
};


inline SurrogateModel::~SurrogateModel()
{ } // Virtual destructor handles referenceCount at Strategy level.


inline Pecos::ProbabilityTransformation& SurrogateModel::
probability_transformation()
{ return truth_model().probability_transformation(); } // forward along


inline void SurrogateModel::activate_distribution_parameter_derivatives()
{ truth_model().activate_distribution_parameter_derivatives(); }


inline void SurrogateModel::deactivate_distribution_parameter_derivatives()
{ truth_model().deactivate_distribution_parameter_derivatives(); }


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


inline void SurrogateModel::
surrogate_model_key(unsigned short model_index, unsigned short soln_lev_index)
{
  if (model_index == USHRT_MAX) surrModelKey.resize(0);
  else {
    if (soln_lev_index == USHRT_MAX)
      surrModelKey.resize(1);
    else {
      surrModelKey.resize(2);
      surrModelKey[1] = soln_lev_index;
    }
    surrModelKey[0] = model_index;
  }
}


inline void SurrogateModel::surrogate_model_key(const UShortArray& key)
{ surrModelKey = key; }


inline const UShortArray& SurrogateModel::surrogate_model_key() const
{ return surrModelKey; }


inline unsigned short SurrogateModel::surrogate_level_index() const
{ return (surrModelKey.size() >= 2) ? surrModelKey.back() : USHRT_MAX; }


inline void SurrogateModel::
truth_model_key(unsigned short model_index, unsigned short soln_lev_index)
{
  if (model_index == USHRT_MAX) truthModelKey.resize(0);
  else {
    if (soln_lev_index == USHRT_MAX)
      truthModelKey.resize(1);
    else {
      truthModelKey.resize(2);
      truthModelKey[1] = soln_lev_index;
    }
    truthModelKey[0] = model_index;
  }
}


inline void SurrogateModel::truth_model_key(const UShortArray& key)
{ truthModelKey = key; }


inline const UShortArray& SurrogateModel::truth_model_key() const
{ return truthModelKey; }


inline unsigned short SurrogateModel::truth_level_index() const
{ return (truthModelKey.size() >= 2) ? truthModelKey.back() : USHRT_MAX; }


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


/** return the SurrogateModel evaluation id counter.  Due to possibly
    intermittent use of lower level components, this is not the same as
    approxInterface, actualModel, or orderedModels evaluation counts,
    which requires a consistent evaluation rekeying process. */
inline int SurrogateModel::derived_evaluation_id() const
{ return surrModelEvalCntr; }

} // namespace Dakota

#endif
