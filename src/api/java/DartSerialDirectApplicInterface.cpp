/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SIM::DartSerialDirectApplicInterface
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DartSerialDirectApplicInterface.hpp"
#include "DakotaResponse.hpp"
#include "ParamResponsePair.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"


namespace DART {

int DartSerialDirectApplicInterface::derived_map_ac(const Dakota::String& ac_name)
{

  int fail_code = 0;
  if (ac_name == "plugin_dart") {
    if (directFnASV[0] & 2) {
      Cerr << "DartSerialDirectApplicInterface does not currently support analytic gradients." << std::endl;
      Dakota::abort_handler(-1);
    }
    if (directFnASV[0] & 4) {
      Cerr << "DartSerialDirectApplicInterface does not currently support analytic hessians." << std::endl;
      Dakota::abort_handler(-1);
    }

    fail_code = evaluate(fnVals);
  }
  else {
    Cerr << ac_name << " is not available as an analysis within "
         << "DART::DartSerialDirectApplicInterface." << std::endl;
    Dakota::abort_handler(-1);
  }

  // Failure capturing
  if (fail_code)
    throw fail_code;

  return 0;
}

void DartSerialDirectApplicInterface::
wait_local_evaluations(Dakota::PRPQueue& prp_queue) 
{
  std::cout << "WLE DISABLED" << std::endl;
}

int DartSerialDirectApplicInterface::evaluate(Dakota::RealVector& fn_val)
{
  std::vector<double> xC_vector;
  std::vector<Dakota::String> xCLabels_vector;
  for (int i=0; i<xC.length(); ++i) {
    xC_vector.push_back(xC[i]);
    xCLabels_vector.push_back(xCLabels[i]);
  }

  functor->setContinuousRealVariables(xC_vector, xCLabels_vector);

  std::vector<int> xDI_vector;
  std::vector<Dakota::String> xDILabels_vector;
  for (int i=0; i<xDI.length(); ++i) {
    xDI_vector.push_back(xDI[i]);
    xDILabels_vector.push_back(xDILabels[i]);
  }

  functor->setDiscreteIntegerVariables(xDI_vector, xDILabels_vector);

  std::vector<double> xDR_vector;
  std::vector<Dakota::String> xDRLabels_vector;
  for (int i=0; i<xDR.length(); ++i) {
    xDR_vector.push_back(xDR[i]);
    xDRLabels_vector.push_back(xDRLabels[i]);
  }

  functor->setDiscreteRealVariables(xDR_vector, xDRLabels_vector);

  functor->setResponseLabels(fnLabels);

  std::vector<double> value;
  // currEvalId is a protected member in Dakota::Interface
  int error = functor->evalFunction(value, currEvalId);
  if (error)
    return error;
  for (int i=0; i<value.size(); ++i)
    fn_val[i] = value[i];

  return 0;
}

DartSerialDirectApplicInterface::
DartSerialDirectApplicInterface(const Dakota::ProblemDescDB& problem_db, DakotaFunctor *f):
  Dakota::DirectApplicInterface(problem_db), functor(f)
{ }


DartSerialDirectApplicInterface::~DartSerialDirectApplicInterface()
{ /* Virtual destructor handles referenceCount at Interface level. */ }


void DartSerialDirectApplicInterface::
derived_map_asynch(const Dakota::ParamResponsePair& pair)
{
  // no-op (just hides base class error throw). Jobs are run exclusively within
  // wait_local_evaluations(), prior to there existing true batch processing
  // facilities.
}


/** For use by ApplicationInterface::serve_evaluations_asynch(), which can
    provide a batch processing capability within message passing schedulers
    (called using chain Strategy::run_iterator() --> Model::serve() -->
    ApplicationInterface::serve_evaluations() -->
    ApplicationInterface::serve_evaluations_asynch()). */
void DartSerialDirectApplicInterface::
test_local_evaluations(Dakota::PRPQueue& prp_queue)
{ wait_local_evaluations(prp_queue); }


// Hide default run-time error checks at DirectApplicInterface level
void DartSerialDirectApplicInterface::
set_communicators_checks(int max_iterator_concurrency)
{ }

} // namespace DART
