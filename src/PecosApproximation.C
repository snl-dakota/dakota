/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        PecosApproximation
//- Description:  Implementation code for PecosApproximation class
//-               
//- Owner:        Mike Eldred

#include "PecosApproximation.H"
#include "ProblemDescDB.H"
#include "NonDIntegration.H"

//#define DEBUG


namespace Dakota {

PecosApproximation::
PecosApproximation(const String& approx_type, const UShortArray& approx_order, 
		   size_t num_vars, unsigned short data_order)
{
  approxOrder = approx_order; numVars = num_vars; dataOrder = data_order;
  pecosBasisApprox
    = Pecos::BasisApproximation(approx_type, approx_order, num_vars);
  polyApproxRep
    = (Pecos::PolynomialApproximation*)pecosBasisApprox.approx_rep();
}


PecosApproximation::
PecosApproximation(ProblemDescDB& problem_db, size_t num_vars):
  Approximation(BaseConstructor(), problem_db, num_vars)
{
  dataOrder = 1;
  if (approxType == "global_orthogonal_polynomial") {
    approxOrder = problem_db.get_dusa("method.nond.expansion_order");
    if (problem_db.get_bool("model.surrogate.derivative_usage")) {
      if (problem_db.get_string("responses.gradient_type") != "none")
	dataOrder |= 2;
      if (problem_db.get_string("responses.hessian_type")  != "none")
	dataOrder |= 4;
    }
  }
  pecosBasisApprox
    = Pecos::BasisApproximation(approxType, approxOrder, numVars);
  polyApproxRep
    = (Pecos::PolynomialApproximation*)pecosBasisApprox.approx_rep();
}


void PecosApproximation::integration_iterator(const Iterator& iterator)
{
  NonDIntegration* integration_rep = (NonDIntegration*)iterator.iterator_rep();
  polyApproxRep->integration_driver_rep(integration_rep->driver().driver_rep());
}

} // namespace Dakota
