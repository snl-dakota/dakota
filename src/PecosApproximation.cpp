/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        PecosApproximation
//- Description:  Implementation code for PecosApproximation class
//-               
//- Owner:        Mike Eldred

#include "PecosApproximation.hpp"
#include "ProblemDescDB.hpp"
#include "NonDIntegration.hpp"

//#define DEBUG


namespace Dakota {

PecosApproximation::
PecosApproximation(const String& approx_type, const UShortArray& approx_order, 
		   size_t num_vars, short data_order):
  Approximation(BaseConstructor(), approx_type, num_vars, data_order)
{
  short basis_type;  approx_type_to_basis_type(approxType, basis_type);
  bool  use_derivs = (data_order > 1);
  pecosBasisApprox = Pecos::BasisApproximation(basis_type, approx_order,
					       num_vars,   use_derivs);
  polyApproxRep
    = (Pecos::PolynomialApproximation*)pecosBasisApprox.approx_rep();
  polyApproxRep->surrogate_data(approxData); // share SurrogateDataRep
}


PecosApproximation::
PecosApproximation(ProblemDescDB& problem_db, size_t num_vars):
  Approximation(BaseConstructor(), problem_db, num_vars)
{
  short basis_type; approx_type_to_basis_type(approxType, basis_type);
  UShortArray approx_order;
  if (basis_type == Pecos::GLOBAL_ORTHOGONAL_POLYNOMIAL)
    approx_order = problem_db.get_usa("method.nond.expansion_order");
  bool use_derivs = problem_db.get_bool("model.surrogate.derivative_usage");
  buildDataOrder = 1;
  if (use_derivs) {
    if (problem_db.get_string("responses.gradient_type") != "none")
      buildDataOrder |= 2;
    if (problem_db.get_string("responses.hessian_type")  != "none")
      buildDataOrder |= 4;
  }
  pecosBasisApprox = Pecos::BasisApproximation(basis_type, approx_order,
					       numVars,    use_derivs);
  polyApproxRep
    = (Pecos::PolynomialApproximation*)pecosBasisApprox.approx_rep();
  polyApproxRep->surrogate_data(approxData); // share SurrogateDataRep
}


void PecosApproximation::
approx_type_to_basis_type(const String& approx_type, short& basis_type)
{
  basis_type = Pecos::NO_BASIS;
  if (approx_type.ends("orthogonal_polynomial")) {
    if (approx_type.begins("global_regression"))
      basis_type = Pecos::GLOBAL_REGRESSION_ORTHOGONAL_POLYNOMIAL;
    else if (approx_type.begins("global_projection"))
      basis_type = Pecos::GLOBAL_PROJECTION_ORTHOGONAL_POLYNOMIAL;
    else if (approx_type.begins("global"))
      basis_type = Pecos::GLOBAL_ORTHOGONAL_POLYNOMIAL;
    //else if (approx_type.begins("piecewise_regression"))
    //  basis_type = Pecos::PIECEWISE_REGRESSION_ORTHOGONAL_POLYNOMIAL;
    //else if (approx_type.begins("piecewise_projection"))
    //  basis_type = Pecos::PIECEWISE_PROJECTION_ORTHOGONAL_POLYNOMIAL;
    //else if (approx_type.begins("piecewise"))
    //  basis_type = Pecos::PIECEWISE_ORTHOGONAL_POLYNOMIAL;
  }
  else if (approx_type.ends("interpolation_polynomial")) {
    if (approx_type.begins("global_nodal"))
      basis_type = Pecos::GLOBAL_NODAL_INTERPOLATION_POLYNOMIAL;
    else if (approx_type.begins("global_hierarchical"))
      basis_type = Pecos::GLOBAL_HIERARCHICAL_INTERPOLATION_POLYNOMIAL;
    else if (approx_type.begins("piecewise_nodal"))
      basis_type = Pecos::PIECEWISE_NODAL_INTERPOLATION_POLYNOMIAL;
    else if (approx_type.begins("piecewise_hierarchical"))
      basis_type = Pecos::PIECEWISE_HIERARCHICAL_INTERPOLATION_POLYNOMIAL;
  }
}


void PecosApproximation::integration_iterator(const Iterator& iterator)
{
  NonDIntegration* integration_rep = (NonDIntegration*)iterator.iterator_rep();
  polyApproxRep->integration_driver_rep(integration_rep->driver().driver_rep());
}

} // namespace Dakota
