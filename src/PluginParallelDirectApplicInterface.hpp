/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ParallelDirectApplicInterface
//- Description:  Derived class for the case when analysis code simulators are
//-               linked into the code and may be invoked directly
//- Owner:        Mike Eldred
//- Version: $Id: PluginParallelDirectApplicInterface.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef PLUGIN_PARALLEL_DIRECT_APPLIC_INTERFACE_H
#define PLUGIN_PARALLEL_DIRECT_APPLIC_INTERFACE_H

#include "DirectApplicInterface.hpp"
#ifdef DAKOTA_HAVE_MPI
#include <mpi.h>
#else
typedef int MPI_Comm;
#endif // DAKOTA_HAVE_MPI


namespace SIM {

/// Sample derived interface class for testing parallel simulator
/// plug-ins using assign_rep().

/** The plug-in ParallelDirectApplicInterface resides in namespace SIM
    and uses a copy of textbook() to perform parallel parameter to
    response mappings. It is used to demonstrate plugging in a
    parallel direct analysis driver into Dakota in library mode.  Test
    input files can then use an analysis_driver of "plugin_textbook". */
class ParallelDirectApplicInterface: public Dakota::DirectApplicInterface
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// constructor
  ParallelDirectApplicInterface(const Dakota::ProblemDescDB& problem_db,
				const MPI_Comm& analysis_comm);
  /// destructor
  ~ParallelDirectApplicInterface();

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
  //- Heading: Data
  //

  // intracommunicator for a multiprocessor analysis server
  //MPI_Comm analysisComm;

  /// demo evaluator function for parallel plug-ins
  int text_book(const Dakota::RealVector& c_vars, const Dakota::ShortArray& asv,
		Dakota::RealVector& fn_vals, Dakota::RealMatrix& fn_grads,
		Dakota::RealSymMatrixArray& fn_hessians);
};


inline ParallelDirectApplicInterface::~ParallelDirectApplicInterface()
{ /* Virtual destructor handles referenceCount at Interface level. */ }


inline void ParallelDirectApplicInterface::
derived_map_asynch(const Dakota::ParamResponsePair& pair)
{
  // no-op (just hides base class error throw). Jobs are run exclusively within
  // wait_local_evaluations(), prior to there existing true batch processing
  // facilities.
}


/** For use by ApplicationInterface::serve_evaluations_asynch(), which can
    provide a batch processing capability within message passing schedulers
    (called using chain ApplicationInterface::serve_evaluations() from
    Model::serve() from IteratorScheduler::run_iterator()). */
inline void ParallelDirectApplicInterface::
test_local_evaluations(Dakota::PRPQueue& prp_queue)
{ wait_local_evaluations(prp_queue); }


// Hide default run-time error checks at DirectApplicInterface level
inline void ParallelDirectApplicInterface::
set_communicators_checks(int max_eval_concurrency)
{ }

} // namespace Dakota

#endif
