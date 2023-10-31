/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef SOLEIL_DIRECT_APPLIC_INTERFACE_H
#define SOLEIL_DIRECT_APPLIC_INTERFACE_H

#include "DirectApplicInterface.hpp"


/// A sample namespace for derived classes that use assign_rep() to 
/// plug facilities into DAKOTA.

/** A typical use of plug-ins with assign_rep() is to publish a 
    simulation interface for use in library mode  See \ref DakLibrary
    for more information. */

namespace StanfordPSAAP {

/// Sample derived interface class for testing serial simulator
/// plug-ins using assign_rep().

/** The plug-in SoleilDirectApplicInterface resides in namespace SIM
    and uses a copy of rosenbrock() to perform serial parameter to
    response mappings.  It is used to demonstrate plugging in a serial
    direct analysis driver into Dakota in library mode.  Test input
    files can then use an analysis_driver of "plugin_rosenbrock". */
class SoleilDirectApplicInterface: public Dakota::DirectApplicInterface
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// constructor
  SoleilDirectApplicInterface(const Dakota::ProblemDescDB& problem_db);
  /// destructor
  ~SoleilDirectApplicInterface();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

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
  void set_communicators_checks(int max_eval_concurrency);

private:

  //
  //- Heading: Convenience functions
  //

  /// Rosenbrock plug-in test function
  int rosenbrock(const Dakota::RealVector& c_vars, short asv,
		 Dakota::Real& fn_val, Dakota::RealVector& fn_grad,
		 Dakota::RealSymMatrix& fn_hess);

  //
  //- Heading: Data
  //

};


inline SoleilDirectApplicInterface::
SoleilDirectApplicInterface(const Dakota::ProblemDescDB& problem_db):
  Dakota::DirectApplicInterface(problem_db)
{ batchEval = true; }


inline SoleilDirectApplicInterface::~SoleilDirectApplicInterface()
{ }


inline void SoleilDirectApplicInterface::
derived_map_asynch(const Dakota::ParamResponsePair& pair)
{
  if (batchEval) {
    // no-op: jobs are run exclusively within wait_local_evaluations(), as
    // called by ApplicationInterface::asynchronous_local_evaluations(), based
    // on jobs added to asynchLocalActivePRPQueue by ApplicationInterface::
    // launch_asynch_local().
  }
  else
    Dakota::DirectApplicInterface::derived_map_asynch(pair); // error trap
}


/** For use by ApplicationInterface::serve_evaluations_asynch(), which can
    provide a batch processing capability within message passing schedulers
    (called using chain IteratorScheduler::run_iterator() --> Model::serve()
    --> ApplicationInterface::serve_evaluations()
    --> ApplicationInterface::serve_evaluations_asynch()). */
inline void SoleilDirectApplicInterface::
test_local_evaluations(Dakota::PRPQueue& prp_queue)
{ wait_local_evaluations(prp_queue); }


// Hide default run-time error checks at DirectApplicInterface level
inline void SoleilDirectApplicInterface::
set_communicators_checks(int max_eval_concurrency)
{ }

} // namespace StanfordPSAAP

#endif
