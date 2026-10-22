/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef ITERATOR_VIEW_REQUEST_H
#define ITERATOR_VIEW_REQUEST_H

namespace Dakota {

/** Describes the active variable view an iterator wants for its iterated
 *  model when constructed through the DI (IRStore) path.
 *
 *  The request is resolved against the model inside
 *  Iterator::assign_requested_view() before update_from_model() is called.
 *  Only the DI constructors use this type; legacy ProblemDescDB constructors
 *  are unaffected.
 *
 *  responseRule: when true (the default), the "objective functions or
 *  calibration terms → DESIGN" precedence rule is applied before the spec.
 *  NonDBayesCalibration passes responseRule=false to prevent calibration
 *  terms from flipping to DESIGN. */
struct ViewRequest {
  enum Spec : short {
    KEEP = 0,              ///< do not change the model's current view
    ALL,                   ///< ALL_VIEW
    DESIGN,                ///< DESIGN_VIEW
    UNCERTAIN,             ///< UNCERTAIN_VIEW
    ALEATORY_UNCERTAIN,    ///< ALEATORY_UNCERTAIN_VIEW
    EPISTEMIC_UNCERTAIN,   ///< EPISTEMIC_UNCERTAIN_VIEW
    STATE,                 ///< STATE_VIEW
    /// Pick UNCERTAIN / ALEATORY_UNCERTAIN / EPISTEMIC_UNCERTAIN based on
    /// uncertain variable counts; keep the current view when there are none.
    UNCERTAIN_BY_COUNTS
  };

  Spec spec          = DESIGN;
  bool responseRule  = true;  ///< apply "obj fns / calib terms → DESIGN" first

  constexpr ViewRequest() = default;
  constexpr ViewRequest(Spec s, bool resp_rule = true)
    : spec(s), responseRule(resp_rule) {}
};

} // namespace Dakota

#endif // ITERATOR_VIEW_REQUEST_H
