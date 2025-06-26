/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaTPLDataTransfer.hpp"

#include "dakota_data_types.hpp"
#include "dakota_global_defs.hpp"

namespace {

// We might want to replace this with std::numeric_limits<Real>::max() or
// similar - RWH
const Dakota::Real local_bigRealBoundSize = Dakota::BIG_REAL_BOUND;

}  // namespace

// -----------------------------------------------------------------

namespace Dakota {

TPLDataTransfer::TPLDataTransfer()
    : numDakotaObjectiveFns(
          1),  // should always be the case with Dakota (?) RWH
      numDakotaNonlinearEqConstraints(0),
      numTPLNonlinearEqConstraints(0),
      numDakotaNonlinearIneqConstraints(0),
      numTPLNonlinearIneqConstraints(0) {}

// -----------------------------------------------------------------

void TPLDataTransfer::configure_nonlinear_eq_adapters(
    NONLINEAR_EQUALITY_FORMAT format, const Constraints& constraints) {
  numDakotaNonlinearEqConstraints = constraints.num_nonlinear_eq_constraints();
  const RealVector& targets = constraints.nonlinear_eq_constraint_targets();

  for (int i = 0; i < numDakotaNonlinearEqConstraints; ++i) {
    nonlinearEqConstraintMapIndices.push_back(numDakotaObjectiveFns + i);
    nonlinearEqConstraintMapMultipliers.push_back(1.0);
    nonlinearEqConstraintTargets.push_back(-targets[i]);
  }

  if (format == NONLINEAR_EQUALITY_FORMAT::TRUE_EQUALITY)
    numTPLNonlinearEqConstraints = numDakotaNonlinearEqConstraints;
  else if (format == NONLINEAR_EQUALITY_FORMAT::TWO_INEQUALITY)
    numTPLNonlinearEqConstraints = 0;

  // Add an error if we aren't either of the above cases ?
}

// -----------------------------------------------------------------

void TPLDataTransfer::configure_nonlinear_ineq_adapters(
    NONLINEAR_INEQUALITY_FORMAT format, const Constraints& constraints,
    bool split_eqs_into_two_ineqs) {
  const RealVector& ineq_lwr_bnds =
      constraints.nonlinear_ineq_constraint_lower_bounds();
  const RealVector& ineq_upr_bnds =
      constraints.nonlinear_ineq_constraint_upper_bounds();
  numDakotaNonlinearIneqConstraints =
      constraints.num_nonlinear_ineq_constraints();

  numTPLNonlinearIneqConstraints = 0;

  bool lower_bnds_specified = false;
  // Determine if the user specified lower bounds for nonlinear inequalities;
  // assumes all-or-none which is a Dakota expectation. (?) RWH
  if (!ineq_lwr_bnds.empty() &&
      *std::max_element(&ineq_lwr_bnds[0],
                        &ineq_lwr_bnds[0] + ineq_lwr_bnds.length() - 1) >
          -local_bigRealBoundSize)
    lower_bnds_specified = true;

  if (format == NONLINEAR_INEQUALITY_FORMAT::ONE_SIDED_UPPER) {
    for (int i = 0; i < numDakotaNonlinearIneqConstraints; i++) {
      // Always do upper bounds which may be the Dakota default of 0.0
      nonlinearIneqConstraintMapIndices.push_back(
          numDakotaObjectiveFns + numDakotaNonlinearEqConstraints + i);
      nonlinearIneqConstraintMapMultipliers.push_back(1.0);
      nonlinearIneqConstraintMapShifts.push_back(-1.0 * ineq_upr_bnds[i]);
      numTPLNonlinearIneqConstraints++;

      if (lower_bnds_specified) {
        nonlinearIneqConstraintMapIndices.push_back(
            numDakotaObjectiveFns + numDakotaNonlinearEqConstraints + i);
        nonlinearIneqConstraintMapMultipliers.push_back(-1.0);
        nonlinearIneqConstraintMapShifts.push_back(ineq_lwr_bnds[i]);
        numTPLNonlinearIneqConstraints++;
      }
    }
  } else if (format == NONLINEAR_INEQUALITY_FORMAT::ONE_SIDED_LOWER) {
    for (int i = 0; i < numDakotaNonlinearIneqConstraints; i++) {
      // Always do upper bounds which may be the Dakota default of 0.0
      nonlinearIneqConstraintMapIndices.push_back(
          numDakotaObjectiveFns + numDakotaNonlinearEqConstraints + i);
      nonlinearIneqConstraintMapMultipliers.push_back(-1.0);
      nonlinearIneqConstraintMapShifts.push_back(ineq_upr_bnds[i]);
      numTPLNonlinearIneqConstraints++;

      if (lower_bnds_specified) {
        nonlinearIneqConstraintMapIndices.push_back(
            numDakotaObjectiveFns + numDakotaNonlinearEqConstraints + i);
        nonlinearIneqConstraintMapMultipliers.push_back(1.0);
        nonlinearIneqConstraintMapShifts.push_back(-1.0 * ineq_lwr_bnds[i]);
        numTPLNonlinearIneqConstraints++;
      }
    }
  } else if (format == NONLINEAR_INEQUALITY_FORMAT::TWO_SIDED) {
    for (int i = 0; i < numDakotaNonlinearIneqConstraints; i++) {
      nonlinearIneqConstraintMapIndices.push_back(
          numDakotaObjectiveFns + numDakotaNonlinearEqConstraints + i);
      nonlinearIneqConstraintMapMultipliers.push_back(1.0);
      nonlinearIneqConstraintMapShifts.push_back(0.0);
      numTPLNonlinearIneqConstraints++;
    }
  }

  if (split_eqs_into_two_ineqs) {
    for (int i = 0; i < numDakotaNonlinearEqConstraints; i++) {
      nonlinearIneqConstraintMapIndices.push_back(
          nonlinearEqConstraintMapIndices[i]);
      nonlinearIneqConstraintMapIndices.push_back(
          nonlinearEqConstraintMapIndices[i]);
      nonlinearIneqConstraintMapMultipliers.push_back(1.0);
      nonlinearIneqConstraintMapMultipliers.push_back(-1.0);
      nonlinearIneqConstraintMapShifts.push_back(
          nonlinearEqConstraintTargets[i]);
      nonlinearIneqConstraintMapShifts.push_back(
          -1.0 * nonlinearEqConstraintTargets[i]);
      numTPLNonlinearIneqConstraints += 2;
    }
  }
}

// -----------------------------------------------------------------

void TPLDataTransfer::configure_data_adapters(
    std::shared_ptr<TraitsBase> traits, const Model& model) {
  const Constraints& constraints = model.user_defined_constraints();

  // Need to do these first
  if (traits->supports_nonlinear_equality())
    configure_nonlinear_eq_adapters(traits->nonlinear_equality_format(),
                                    constraints);

  if (traits->supports_nonlinear_inequality()) {
    bool split_eqs = traits->supports_nonlinear_equality() &&
                     (traits->nonlinear_equality_format() ==
                      NONLINEAR_EQUALITY_FORMAT::TWO_INEQUALITY);
    configure_nonlinear_ineq_adapters(traits->nonlinear_inequality_format(),
                                      constraints, split_eqs);
  }

  // Determine and store min/max response sense (assumes single for now)
  maxSense = (!model.primary_response_fn_sense().empty() &&
              model.primary_response_fn_sense()[0]);
}

// -----------------------------------------------------------------

}  // namespace Dakota
