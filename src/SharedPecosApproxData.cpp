/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SharedPecosApproxData
//- Description:  Implementation code for SharedPecosApproxData class
//-               
//- Owner:        Mike Eldred

#include "SharedPecosApproxData.hpp"
#include "ProblemDescDB.hpp"
#include "NonDIntegration.hpp"

//#define DEBUG


namespace Dakota {

SharedPecosApproxData::
SharedPecosApproxData(const String& approx_type,
		      const UShortArray& approx_order, size_t num_vars,
		      short data_order, short output_level):
  SharedApproxData(NoDBBaseConstructor(), approx_type, num_vars, data_order,
		   output_level)
{
  short basis_type; approx_type_to_basis_type(approxType, basis_type);

  // override selected ConfigOptions defaults, as supported by SharedApproxData
  // API.  All options are updated later in NonD*::initialize_u_space_model(),
  // so this step is not strictly required and is more for completeness.
  Pecos::ExpansionConfigOptions  ec_options; // set defaults
  Pecos::BasisConfigOptions      bc_options; // set defaults
  Pecos::RegressionConfigOptions rc_options; // set defaults
  ec_options.outputLevel = outputLevel;
  bc_options.useDerivs   = (buildDataOrder > 1);

  pecosSharedData =
    Pecos::SharedBasisApproxData(basis_type, approx_order, numVars,
				 ec_options, bc_options, rc_options);
  pecosSharedDataRep
    = (Pecos::SharedPolyApproxData*)pecosSharedData.data_rep();
}


SharedPecosApproxData::
SharedPecosApproxData(ProblemDescDB& problem_db, size_t num_vars):
  SharedApproxData(BaseConstructor(), problem_db, num_vars)
{
  short basis_type; approx_type_to_basis_type(approxType, basis_type);
  UShortArray approx_order;
  if (basis_type == Pecos::GLOBAL_ORTHOGONAL_POLYNOMIAL)
    approx_order = problem_db.get_usa("method.nond.expansion_order");

  // override selected ConfigOptions defaults, as supported by SharedApproxData
  // API.  All options are updated later in NonD*::initialize_u_space_model(),
  // so this step is not strictly required and is more for completeness.
  Pecos::ExpansionConfigOptions  ec_options; // set defaults
  Pecos::BasisConfigOptions      bc_options; // set defaults
  Pecos::RegressionConfigOptions rc_options; // set defaults
  ec_options.outputLevel = outputLevel;
  bc_options.useDerivs   = (buildDataOrder > 1);

  pecosSharedData =
    Pecos::SharedBasisApproxData(basis_type, approx_order, numVars,
				 ec_options, bc_options, rc_options);
  pecosSharedDataRep
    = (Pecos::SharedPolyApproxData*)pecosSharedData.data_rep();
}


void SharedPecosApproxData::
approx_type_to_basis_type(const String& approx_type, short& basis_type)
{
  basis_type = Pecos::NO_BASIS;
  if (strends(approx_type, "orthogonal_polynomial")) {
    if (strbegins(approx_type, "global_regression"))
      basis_type = Pecos::GLOBAL_REGRESSION_ORTHOGONAL_POLYNOMIAL;
    else if (strbegins(approx_type, "global_projection"))
      basis_type = Pecos::GLOBAL_PROJECTION_ORTHOGONAL_POLYNOMIAL;
    else if (strbegins(approx_type, "global"))
      basis_type = Pecos::GLOBAL_ORTHOGONAL_POLYNOMIAL;
    //else if (strbegins(approx_type, "piecewise_regression"))
    //  basis_type = Pecos::PIECEWISE_REGRESSION_ORTHOGONAL_POLYNOMIAL;
    //else if (strbegins(approx_type, "piecewise_projection"))
    //  basis_type = Pecos::PIECEWISE_PROJECTION_ORTHOGONAL_POLYNOMIAL;
    //else if (strbegins(approx_type, "piecewise"))
    //  basis_type = Pecos::PIECEWISE_ORTHOGONAL_POLYNOMIAL;
  }
  else if (strends(approx_type, "interpolation_polynomial")) {
    if (strbegins(approx_type, "global_nodal"))
      basis_type = Pecos::GLOBAL_NODAL_INTERPOLATION_POLYNOMIAL;
    else if (strbegins(approx_type, "global_hierarchical"))
      basis_type = Pecos::GLOBAL_HIERARCHICAL_INTERPOLATION_POLYNOMIAL;
    else if (strbegins(approx_type, "piecewise_nodal"))
      basis_type = Pecos::PIECEWISE_NODAL_INTERPOLATION_POLYNOMIAL;
    else if (strbegins(approx_type, "piecewise_hierarchical"))
      basis_type = Pecos::PIECEWISE_HIERARCHICAL_INTERPOLATION_POLYNOMIAL;
  }
}


void SharedPecosApproxData::integration_iterator(const Iterator& iterator)
{
  NonDIntegration* integration_rep = (NonDIntegration*)iterator.iterator_rep();
  pecosSharedDataRep->integration_driver_rep(
    integration_rep->driver().driver_rep());
}

} // namespace Dakota
