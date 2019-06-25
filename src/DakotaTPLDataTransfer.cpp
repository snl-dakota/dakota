/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       TPLDataTransfer
//- Description: Implementation code for the TPLDataTransfer class
//- Owner:       Russell Hooper

#include "dakota_data_types.hpp"
#include "dakota_global_defs.hpp"
#include "DakotaTPLDataTransfer.hpp"

namespace {

  // We might want to replace this with std::numeric_limits<Real>::max() or similar - RWH
  const Dakota::Real local_bigRealBoundSize = Dakota::BIG_REAL_BOUND;

}

// -----------------------------------------------------------------

namespace Dakota {

TPLDataTransfer::TPLDataTransfer() :
  numDakotaObjectiveFns(1), // should always be the case with Dakota (?) RWH
  numDakotaNonlinearEqConstraints(0),
  numDakotaNonlinearIneqConstraints(0),
  numNonlinearIneqConstraintsActive(0)
{
}

// -----------------------------------------------------------------

void
TPLDataTransfer::configure_nonlinear_ineq_adapters(
                                NONLINEAR_INEQUALITY_FORMAT format,
                                const Constraints &         constraints)
{
  numDakotaNonlinearEqConstraints = constraints.num_nonlinear_eq_constraints();

  const RealVector& ineq_lwr_bnds   = constraints.nonlinear_ineq_constraint_lower_bounds();
  const RealVector& ineq_upr_bnds   = constraints.nonlinear_ineq_constraint_upper_bounds();
  numDakotaNonlinearIneqConstraints = constraints.num_nonlinear_ineq_constraints();

  numNonlinearIneqConstraintsActive = 0;

  bool lower_bnds_specified = false;
  // Determine if the user specified lower bounds for nonlinear inequalities; assumes all-or-none which
  // is a Dakota expectation. (?) RWH
  if( !ineq_lwr_bnds.empty() &&
      *std::max_element(&ineq_lwr_bnds[0], &ineq_lwr_bnds[0]+ineq_lwr_bnds.length()-1) > -local_bigRealBoundSize )
    lower_bnds_specified = true;


  if( format == NONLINEAR_INEQUALITY_FORMAT::STANDARD )
  {
    for (int i=0; i<numDakotaNonlinearIneqConstraints; i++) {
      // Always do upper bounds which may be the Dakota default of 0.0
      nonlinearIneqConstraintMapIndices.push_back(numDakotaObjectiveFns + numDakotaNonlinearEqConstraints + i);
      nonlinearIneqConstraintMapMultipliers.push_back(1.0);
      nonlinearIneqConstraintMapShifts.push_back(-1.0*ineq_upr_bnds[i]);
      numNonlinearIneqConstraintsActive++;

      if (lower_bnds_specified) {
        nonlinearIneqConstraintMapIndices.push_back(numDakotaObjectiveFns + numDakotaNonlinearEqConstraints + i);
        nonlinearIneqConstraintMapMultipliers.push_back(-1.0);
        nonlinearIneqConstraintMapShifts.push_back(ineq_lwr_bnds[i]);
        numNonlinearIneqConstraintsActive++;
      }
    }
  }
  else if( format == NONLINEAR_INEQUALITY_FORMAT::ANTI_STANDARD )
  {
    for (int i=0; i<numDakotaNonlinearIneqConstraints; i++) {
      // Always do upper bounds which may be the Dakota default of 0.0
        nonlinearIneqConstraintMapIndices.push_back(i);
        nonlinearIneqConstraintMapMultipliers.push_back(-1.0);
        nonlinearIneqConstraintMapShifts.push_back(ineq_upr_bnds[i]);
        numNonlinearIneqConstraintsActive++;

      if (lower_bnds_specified) {
        nonlinearIneqConstraintMapIndices.push_back(i);
        nonlinearIneqConstraintMapMultipliers.push_back(1.0);
        nonlinearIneqConstraintMapShifts.push_back(-1.0*ineq_lwr_bnds[i]);
        numNonlinearIneqConstraintsActive++;
      }
    }
  }
}

// -----------------------------------------------------------------

void
TPLDataTransfer::configure_data_adapters(std::shared_ptr<TraitsBase> traits,
                                         const Constraints & constraints    )
{
  if( traits->supports_nonlinear_inequality() )
    configure_nonlinear_ineq_adapters(traits->nonlinear_inequality_format(), constraints);
}

// -----------------------------------------------------------------

} // namespace Dakota
