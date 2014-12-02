/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DART_SERIAL_DIRECT_APPLIC_INTERFACE_H
#define DART_SERIAL_DIRECT_APPLIC_INTERFACE_H

#include "DirectApplicInterface.hpp"
#include "DakotaUtils.hpp"

namespace DART {

class DartSerialDirectApplicInterface: public Dakota::DirectApplicInterface
{
public:

  /// constructor
  DartSerialDirectApplicInterface(const Dakota::ProblemDescDB& problem_db, DakotaFunctor* f);
  /// destructor
  ~DartSerialDirectApplicInterface();

protected:

  // execute the input filter portion of a direct evaluation invocation
  //int derived_map_if(const Dakota::String& if_name);

  /// execute an analysis code portion of a direct evaluation invocation
  int derived_map_ac(const Dakota::String& ac_name);

  // execute the output filter portion of a direct evaluation invocation
  //int derived_map_of(const Dakota::String& of_name);

  /// no-op hides base error; job batching occurs within
  /// wait_local_evaluations()
  void derived_map_asynch(const Dakota::ParamResponsePair& pair);

  /// evaluate the batch of jobs contained in prp_queue
  void wait_local_evaluations(Dakota::PRPQueue& prp_queue);
  /// invokes wait_local_evaluations() (no special nowait support)
  void test_local_evaluations(Dakota::PRPQueue& prp_queue);

  /// no-op hides default run-time error checks at DirectApplicInterface level
  void set_communicators_checks(int max_iterator_concurrency);

private:

  //
  //- Heading: Convenience functions
  //

  /// test function
  int evaluate(Dakota::RealVector& fn_val);

  //
  //- Heading: Data
  //
  DakotaFunctor *functor;
};

} // namespace DART

#endif
