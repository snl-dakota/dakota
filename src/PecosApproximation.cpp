/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "PecosApproximation.hpp"

#include "NonDIntegration.hpp"
#include "ProblemDescDB.hpp"
#include "SharedPecosApproxData.hpp"

// #define DEBUG

namespace Dakota {

PecosApproximation::PecosApproximation(const SharedApproxData& shared_data)
    : Approximation(NoDBBaseConstructor(), shared_data)
// sharedPecosDataRep((SharedPecosApproxData*)sharedDataRep)
{
  // Dakota::PecosApproximation manages the Pecos::BasisApproximation instance,
  // and Dakota::SharedPecosApproxData manages the Pecos::SharedBasisApproxData
  // instance.  The latter shared data instance must be passed through to
  // initialize the former approximation instance, which retains a pointer to
  // the shared data representation.

  std::shared_ptr<SharedPecosApproxData> shared_pecos_data_rep =
      std::static_pointer_cast<SharedPecosApproxData>(sharedDataRep);
  pecosBasisApprox =
      Pecos::BasisApproximation(shared_pecos_data_rep->pecos_shared_data());

  // Make a shallow copy of approxData instance (shared rep).
  // Note: this SurrogateData instance is empty at construct time.
  pecosBasisApprox.surrogate_data(approxData);

  // convenience pointer (we use PolynomialApproximation exclusively)
  polyApproxRep = std::static_pointer_cast<Pecos::PolynomialApproximation>(
      pecosBasisApprox.approx_rep());
}

PecosApproximation::PecosApproximation(ProblemDescDB& problem_db,
                                       const SharedApproxData& shared_data,
                                       const String& approx_label)
    : Approximation(BaseConstructor(), problem_db, shared_data, approx_label)
// sharedPecosDataRep((SharedPecosApproxData*)sharedDataRep)
{
  // Dakota::PecosApproximation manages the Pecos::BasisApproximation instance,
  // and Dakota::SharedPecosApproxData manages the Pecos::SharedBasisApproxData
  // instance.  The latter shared data instance must be passed through to
  // initialize the former approximation instance, which retains a pointer to
  // the shared data representation.

  std::shared_ptr<SharedPecosApproxData> shared_pecos_data_rep =
      std::static_pointer_cast<SharedPecosApproxData>(sharedDataRep);
  pecosBasisApprox =
      Pecos::BasisApproximation(shared_pecos_data_rep->pecos_shared_data());

  // Make a shallow copy of approxData instance (shared rep).
  // Note: this SurrogateData instance is empty at construct time.
  pecosBasisApprox.surrogate_data(approxData);

  // convenience pointer (we use PolynomialApproximation exclusively)
  polyApproxRep = std::static_pointer_cast<Pecos::PolynomialApproximation>(
      pecosBasisApprox.approx_rep());
}

/*
void PecosApproximation::link_multilevel_surrogate_data()
{
  // Manage {surr,modSurr}Data instances (activeKey and original/modified
  // approxData indices are managed in SharedPecosApproxData).
  // > SurrogateModel::aggregate_response() uses order of low to high fidelity
  // > ApproximationInterface::{mixed,shallow}_add() assigns aggregate response
  //   data to each approxData instance in turn.

  SharedPecosApproxData* shared_data_rep
    = (SharedPecosApproxData*)sharedDataRep;
  switch (shared_data_rep->pecos_shared_data_rep()->discrepancy_reduction()) {
  case Pecos::DISTINCT_DISCREPANCY: case Pecos::RECURSIVE_DISCREPANCY: {
    // push another SurrogateData instance for modSurrData
    // (allows consolidation of Approximation::push/pop operations)
    const UShortArray& key = approxData.back().active_key();
    Pecos::SurrogateData mod_surr(key);
    approxData.push_back(mod_surr);
    pecosBasisApprox.modified_surrogate_data(mod_surr);
    break;
  }
  default: // default ctor linkages are sufficient
    break;
  }
}
*/

}  // namespace Dakota
