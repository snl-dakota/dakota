/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SCALING_MODEL_H
#define DAKOTA_SCALING_MODEL_H

#include "RecastModel.hpp"

namespace Dakota {

/// Scaling specialization of RecastModel

/** Specialization of RecastModel to scale Variables and/or Responses
    This class provides a simple constructor that forwards to the more
    complicated RecastModel API */
class ScalingModel: public RecastModel
{
public:
  
  //
  //- Heading: Constructor and destructor
  //

  /// standard constructor
  ScalingModel(Model& sub_model);

  /// destructor
  ~ScalingModel();

  /// ---
  /// Public members for help in final results recovery
  /// ---

  /// recover native variable values from the scaled space
  RealVector cv_scaled2native(const RealVector& scaled_cv) const;

  /// map responses from scaled to native space, updating provided
  /// Response in-place (on entry it's scaled response, on exit it's
  /// native)
  void resp_scaled2native(const Variables& native_vars,
                          Response& updated_resp) const;

  /// Use scaled nonlinear constraints (sized total functions) to
  /// update the nonlinear constraints portion of the passed
  /// native_fns array
  void secondary_resp_scaled2native(const RealVector& scaled_nln_cons,
                                    const ShortArray& asv,
				    size_t num_native_primary,
                                    RealVector& native_fns) const;

  /// map responses from scaled to native space
  void response_modify_s2n(const Variables& native_vars,
			   const Response& scaled_response,
			   Response& native_response,
			   int start_offset, int num_responses,
			   bool response_unscale = true) const;
  
  ActiveSet default_active_set();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void assign_instance();

  void init_metadata() override { /* no-op to leave metadata intact */}

  bool update_variables_from_model(Model& model) override;

  // ---
  // Scaling initialization helper functions
  // ---

  /// initialize scaling types, multipliers, and offsets; perform error
  /// checking
  void initialize_scaling(Model& sub_model);
  
  /// check whether the passed scale types include any active (!= none)
  /// scale types
  static bool scaling_active(const UShortArray& scale_types);

  /// general helper function for initializing scaling types and factors on a 
  /// vector of variables, functions, constraints, etc.
  void compute_scaling(int auto_type, int num_vars,
                       RealVector& lbs, RealVector& ubs,
                       RealVector& targets,
                       const UShortArray& spec_types,
                       const RealVector& scales,
                       UShortArray& scale_types, RealVector& scale_mults,
                       RealVector& scale_offsets);

  /// general linear coefficients mapping from native to scaled space 
  RealMatrix lin_coeffs_modify_n2s(const RealMatrix& native_coeffs, 
    const RealVector& cv_multipliers, const RealVector& lin_multipliers) const;

  /// automatically compute a single scaling factor -- bounds case
  bool compute_scale_factor(const Real lower_bound, const Real upper_bound,
                            Real *multiplier, Real *offset);
  
  /// automatically compute a single scaling factor -- target case
  bool compute_scale_factor(const Real target, Real *multiplier);

  /// print scaling information for a particular response type in tabular form
  void print_scaling(const String& info, const UShortArray& scale_types,
		     const RealVector& scale_mults,
		     const RealVector& scale_offsets, 
		     const StringArray& labels);

  // ---
  // Callback functions that perform scaling during the Recast operations
  // ---

  /// RecastModel callback for variables scaling: transform variables
  /// from scaled to native (user) space
  static void variables_scaler(const Variables& scaled_vars, 
			       Variables& native_vars);

  /// RecastModel callback for inverse variables scaling: transform variables
  /// from native (user) to scaled space
  static void variables_unscaler(const Variables& native_vars,
				 Variables& scaled_vars);

  /// RecastModel callback for primary response scaling: transform
  /// responses (grads, Hessians) from native (user) to scaled space
  static void primary_resp_scaler(const Variables& native_vars, 
				  const Variables& scaled_vars,
				  const Response& native_response, 
				  Response& iterator_response);

  /// RecastModel callback for secondary response scaling: transform
  /// constraints (grads, Hessians) from native (user) to scaled space
  static void secondary_resp_scaler(const Variables& native_vars,
				    const Variables& scaled_vars,
				    const Response& native_response,
				    Response& scaled_response);

  // ---
  // Convenience functions to manage transformations
  // ---

  /// determine if response transformation is needed due to variable
  /// transformations
  bool need_resp_trans_byvars(const ShortArray& asv, int start_index, 
                              int num_resp) const;

  /// general RealVector mapping from native to scaled variables vectors: 
  RealVector modify_n2s(const RealVector& native_vars,
			     const UShortArray& scale_types,
			     const RealVector& multipliers,
			     const RealVector& offsets) const;

  /// general RealVector mapping from scaled to native variables (and values) 
  RealVector modify_s2n(const RealVector& scaled_vars,
			     const UShortArray& scale_types,
			     const RealVector& multipliers,
			     const RealVector& offsets) const;

  /// map reponses from native to scaled variable space
  void response_modify_n2s(const Variables& scaled_vars,
			   const Response& native_response,
			   Response& scaled_response,
			   int start_offset, int num_responses) const;

 /// static pointer to this class for use in static callbacks
  static ScalingModel* scaleModelInstance;

  bool       varsScaleFlag;          ///< flag for variables scaling
  bool       primaryRespScaleFlag;   ///< flag for primary response scaling
  bool       secondaryRespScaleFlag; ///< flag for secondary response scaling

  UShortArray cvScaleTypes;           ///< scale flags for continuous vars.
  RealVector cvScaleMultipliers;     ///< scales for continuous variables
  RealVector cvScaleOffsets;         ///< offsets for continuous variables

  UShortArray responseScaleTypes;         ///< scale flags for all responses
  RealVector responseScaleMultipliers;   ///< scales for all responses
  RealVector responseScaleOffsets;       ///< offsets for all responses (zero
                                         ///< for functions, not for nonlin con)

  UShortArray linearIneqScaleTypes;       ///< scale flags for linear ineq
  RealVector linearIneqScaleMultipliers; ///< scales for linear ineq constrs.
  RealVector linearIneqScaleOffsets;     ///< offsets for linear ineq constrs.

  UShortArray   linearEqScaleTypes;         ///< scale flags for linear eq.
  RealVector linearEqScaleMultipliers;   ///< scales for linear constraints
  RealVector linearEqScaleOffsets;       ///< offsets for linear constraints

};


inline void ScalingModel::assign_instance()
{ scaleModelInstance = this; }

} // namespace Dakota

#endif
