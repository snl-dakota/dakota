/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SurrogateModel
//- Description: A model which provides a surrogate for a truth model.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: SurrogateModel.H 7024 2010-10-16 01:24:42Z mseldre $

#ifndef SURROGATE_MODEL_H
#define SURROGATE_MODEL_H

#include "DakotaModel.H"
#include "DakotaInterface.H"
#include "DakotaApproximation.H"


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
  SurrogateModel(ParallelLibrary& parallel_lib, const SharedVariablesData& svd,
		 const ActiveSet& set, const String& corr_type,
		 short corr_order);
  /// destructor
  ~SurrogateModel();

  //
  //- Heading: Virtual function redefinitions
  //

  /// return truth_model()
  Model& subordinate_model();

  /// compute the correction required to bring approx_response into
  /// agreement with truth_response
  void compute_correction(const RealVector& c_vars,
			  const Response& truth_response, 
			  const Response& approx_response);

  /// apply the correction computed in compute_correction() to approx_response
  void apply_correction(const RealVector& c_vars, Response& approx_response,
			bool quiet_flag = false);

  /// sets autoCorrection to on (true) or off (false)
  void auto_correction(bool correction_flag);
  /// returns autoCorrection setting
  bool auto_correction();

  //
  //- Heading: Member functions
  //

  /// verify compatibility between SurrogateModel attributes and
  /// attributes of the submodel (DataFitSurrModel::actualModel or
  /// HierarchSurrModel::highFidelityModel)
  void check_submodel_compatibility(const Model& sub_model);

  /// evaluate whether a rebuild of the approximation should be
  /// forced based on changes in the inactive data
  bool force_rebuild();

  /// distributes the incoming orig_asv among actual_asv and approx_asv
  void asv_mapping(const ShortArray& orig_asv, ShortArray& actual_asv,
		   ShortArray& approx_asv, bool build_flag);
  /// reconstitutes a combined_asv from actual_asv and approx_asv
  void asv_mapping(const ShortArray& actual_asv, const ShortArray& approx_asv,
		   ShortArray& combined_asv);
  /// overlays actual_response and approx_response to update combined_response
  void response_mapping(const Response& actual_response,
                        const Response& approx_response,
                        Response& combined_response);

  //
  //- Heading: Data
  //

  /// flag for mixed approximate/actual responses
  bool mixedResponseSet;

  /// for mixed response sets, this array specifies the response function
  /// subset that is approximated
  IntSet surrogateFnIndices;

  /// map of surrogate responses used in derived_synchronize() and
  /// derived_synchronize_nowait() functions
  IntResponseMap surrResponseMap;

  /// map of raw continuous variables used by apply_correction().
  /// Model::varsList cannot be used for this purpose since it does
  /// not contain lower level variables sets from finite differencing.
  IntRealVectorMap rawCVarsMap;

  /// map from actualModel/highFidelityModel evaluation ids to
  /// DataFitSurrModel/HierarchSurrModel ids
  IntIntMap truthIdMap;
  /// map from approxInterface/lowFidelityModel evaluation ids to
  /// DataFitSurrModel/HierarchSurrModel ids
  IntIntMap surrIdMap;

  /// map of approximate responses retrieved in derived_synchronize_nowait()
  /// that could not be returned since corresponding truth model response
  /// portions were still pending.
  IntResponseMap cachedApproxRespMap;

  /// approximation correction approach to be used: additive or multiplicative
  String correctionType;

  /// approximation correction order to be used: 0, 1, or 2
  short correctionOrder;

  /// a flag which controls the use of apply_correction() in DataFitSurrModel
  /// and HierarchSurrModel approximate response computations
  /** SurrBasedOptStrategy must toggle this value since compute_correction()
      no longer automatically backs out an old correction. */
  bool autoCorrection;

  /// flag indicating whether or not a correction has been computed
  /// and is available for application
  bool correctionComputed;

  /// number of calls to build_approximation()
  /** used as a flag to automatically build the approximation if one of the
      derived compute_response functions is called prior to
      build_approximation(). */
  size_t approxBuilds;

  /// a flag which allows bypassing the approximation for evaluations
  /// on the underlying truth model.
  bool surrogateBypass;

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
  /// stores a reference copy of the inactive discrete real variables when
  /// the approximation is built using a Distinct view; used to detect when
  /// a rebuild is required.
  RealVector referenceIDRVars;

private:

  //
  //- Heading: Convenience functions
  //

  /// internal convenience function for applying additive corrections
  /// to an approximate response
  void apply_additive_correction(const RealVector& c_vars,
				 Response& approx_response);
  /// internal convenience function for applying additive corrections
  /// to a set of response functions
  void apply_additive_correction(const RealVector& c_vars,
				 RealVector& approx_fns);

  /// internal convenience function for applying multiplicative
  /// corrections to an approximate response
  void apply_multiplicative_correction(const RealVector& c_vars,
				       Response& approx_response);
  /// internal convenience function for applying multiplicative corrections
  /// to a set of response functions
  void apply_multiplicative_correction(const RealVector& c_vars,
				       RealVector& approx_fns);

  /// search data_pairs for missing approximation data
  const Response& search_db(const RealVector& c_vars,
			    const ShortArray& search_asv);

  //
  //- Heading: Data
  //

  /// flag used to indicate function values near zero for multiplicative
  /// corrections; triggers an automatic switch to additive corrections
  bool badScalingFlag;
  /// flag indicating the combination of additive/multiplicative corrections
  bool combinedFlag;
  /// flag indicating the need for additive correction calculations
  bool computeAdditive;
  /// flag indicating the need for multiplicative correction calculations
  bool computeMultiplicative;

  /// array of additive corrections; surrogate models of a model
  /// discrepancy function (formed from model differences)
  std::vector<Approximation> addCorrections;
  /// array of multiplicative corrections; surrogate models of a model
  /// discrepancy function (formed from model ratios)
  std::vector<Approximation> multCorrections;

  /// factors for combining additive and multiplicative corrections.
  /// Each factor is the weighting applied to the additive correction and
  /// 1.-factor is the weighting applied to the multiplicative correction.
  /// The factor value is determined by an additional requirement to match
  /// the high fidelity function value at the previous correction point
  /// (e.g., previous trust region center).  This results in a multipoint
  /// correction instead of a strictly local correction.
  RealVector combineFactors;

  /// copy of center point from the previous correction cycle
  RealVector correctionPrevCenterPt;
  /// truth function values at the current correction point
  RealVector truthFnsCenter;
  /// Surrogate function values at the current correction point
  RealVector approxFnsCenter;
  // Surrogate gradient values at the current correction point
  //RealMatrix approxGradsCenter;
  /// copy of truth function values at center of previous correction cycle
  RealVector truthFnsPrevCenter;
  /// copy of approximate function values at center of previous correction cycle
  RealVector approxFnsPrevCenter;

  /// copy of the truth model variables object used to simplify conversion 
  /// among differing variable views in force_rebuild()
  Variables subModelVars;
  /// copy of the truth model constraints object used to simplify conversion 
  /// among differing variable views in force_rebuild()
  Constraints subModelCons;
};


inline SurrogateModel::~SurrogateModel()
{ } // Virtual destructor handles referenceCount at Strategy level.


inline Model& SurrogateModel::subordinate_model()
{ return truth_model(); }


inline void SurrogateModel::auto_correction(bool correction_flag)
{ autoCorrection = correction_flag; }


inline bool SurrogateModel::auto_correction()
{ return autoCorrection; }

} // namespace Dakota

#endif
