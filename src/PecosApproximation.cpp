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

  // Make a shallow copy of initial approxData instance (shared rep).
  // Note: there is only one empty SurrogateData instance at construct time
  pecosBasisApprox.surrogate_data(approxData.front());
  // modified SurrogateData is linked downstream, if needed (ML-MF)

  // convenience pointer (we use PolynomialApproximation exclusively)
  polyApproxRep
    = (Pecos::PolynomialApproximation*)pecosBasisApprox.approx_rep();
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

  // Make a shallow copy of initial approxData instance (shared rep):
  // Note: there is only one empty SurrogateData instance at construct time
  pecosBasisApprox.surrogate_data(approxData.front());
  // modified SurrogateData is linked downstream, if needed (ML-MF)

  // convenience pointer (we use PolynomialApproximation exclusively)
  polyApproxRep
    = (Pecos::PolynomialApproximation*)pecosBasisApprox.approx_rep();
}


void PecosApproximation::link_multilevel_surrogate_data()
{
  // Notes:
  // > SurrogateModel::aggregate_response() uses order of LF,HF (consistent with
  //   ordered_models from low to high) such that surrogate_data(fn_index,0)
  //   would retrieve the (to be modified) LF approxData.
  // > ApproximationInterface::{mixed,shallow}_add() assigns aggregate response
  //   data to each approxData instance in turn.

  SharedPecosApproxData* shared_data_rep
    = (SharedPecosApproxData*)sharedDataRep;
  const UShortArray& key = approxData.back().active_key();
  switch (shared_data_rep->pecos_shared_data_rep()->discrepancy_type()) {
  case Pecos::DISTINCT_DISCREP: case Pecos::RECURSIVE_DISCREP: {
    // push another SurrogateData instance for modSurrData
    // (allows consolidation of Approximation::push/pop operations)
    size_t mod_index = approxData.size(); // typically 1
    Pecos::SurrogateData mod_surr(key);
    approxData.push_back(mod_surr);
    pecosBasisApprox.modified_surrogate_data(mod_surr);
    // Configure active approxData such that other classes access the
    // discrepancy/surplus data rather than the raw QoI data
    surrogate_data_index(mod_index);
    break;
  }
  default: // default ctor linkages are sufficient
    break;
  }
}


/*
void PecosApproximation::activate_multilevel_surrogate_data()
{
  SharedPecosApproxData* shared_data_rep
    = (SharedPecosApproxData*)sharedDataRep;
  switch (shared_data_rep->pecos_shared_data_rep()->discrepancy_type()) {
  case Pecos::DISTINCT_DISCREP:
    // > sdRep already defined in SurrogateData ctor...
    // > active key and active iterators for SurrogateData already defined...
    // > indicate that data sets are active, but must be key-specific so that
    //   level 0 (coarse/LF) remains inactive for second approxData
    approxData[0].activate_all_keys();
    approxData[1].activate_nonbase_keys();// all keys beyond first
    surrogate_data_index(0); // reassign (is also the default)
    // TO DO: need a default activation for non-ML cases
    break;
  //case Pecos::RECURSIVE_DISCREP: default: // nothing additional necessary
  }
}
*/

} // namespace Dakota
