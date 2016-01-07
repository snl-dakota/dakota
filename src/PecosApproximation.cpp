/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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
#include "SharedPecosApproxData.hpp"

//#define DEBUG


namespace Dakota {

PecosApproximation::PecosApproximation(const SharedApproxData& shared_data):
  Approximation(NoDBBaseConstructor(), shared_data)
  //sharedPecosDataRep((SharedPecosApproxData*)sharedDataRep)
{
  // Dakota::PecosApproximation manages the Pecos::BasisApproximation instance,
  // and Dakota::SharedPecosApproxData manages the Pecos::SharedBasisApproxData
  // instance.  The latter shared data instance must be passed through to
  // initialize the former approximation instance, which retains a pointer to
  // the shared data representation.

  SharedPecosApproxData* shared_pecos_data_rep
    = (SharedPecosApproxData*)sharedDataRep;
  pecosBasisApprox
    = Pecos::BasisApproximation(shared_pecos_data_rep->pecos_shared_data());
  polyApproxRep
    = (Pecos::PolynomialApproximation*)pecosBasisApprox.approx_rep();
  polyApproxRep->surrogate_data(approxData); // share SurrogateDataRep
}


PecosApproximation::
PecosApproximation(ProblemDescDB& problem_db,
		   const SharedApproxData& shared_data,
                   const String& approx_label):
  Approximation(BaseConstructor(), problem_db, shared_data, approx_label)
  //sharedPecosDataRep((SharedPecosApproxData*)sharedDataRep)
{
  // Dakota::PecosApproximation manages the Pecos::BasisApproximation instance,
  // and Dakota::SharedPecosApproxData manages the Pecos::SharedBasisApproxData
  // instance.  The latter shared data instance must be passed through to
  // initialize the former approximation instance, which retains a pointer to
  // the shared data representation.

  SharedPecosApproxData* shared_pecos_data_rep
    = (SharedPecosApproxData*)sharedDataRep;
  pecosBasisApprox
    = Pecos::BasisApproximation(shared_pecos_data_rep->pecos_shared_data());
  polyApproxRep
    = (Pecos::PolynomialApproximation*)pecosBasisApprox.approx_rep();
  polyApproxRep->surrogate_data(approxData); // share SurrogateDataRep
}

} // namespace Dakota
