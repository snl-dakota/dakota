/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ForkApplicInterface
//- Description:  Derived class for the case when analysis code simulators use
//-               vfork\exec\wait to provide the function evaluations
//- Owner:        Mike Eldred
//- Version: $Id: ForkApplicInterface.H 6492 2009-12-19 00:04:28Z briadam $

#ifndef FORK_APPLIC_INTERFACE_H
#define FORK_APPLIC_INTERFACE_H

#include "ForkAnalysisCode.H"
#include "ApplicationInterface.H"


namespace Dakota {

/// Derived application interface class which spawns simulation codes
/// using forks.

/** ForkApplicInterface uses a ForkAnalysisCode object for performing
    simulation invocations. */

class ForkApplicInterface: public ApplicationInterface
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  ForkApplicInterface(const ProblemDescDB& problem_db);
  /// destructor
  ~ForkApplicInterface();

  //
  //- Heading: Virtual function redefinitions
  //

  void derived_map(const Variables& vars, const ActiveSet& set,
		   Response& response, int fn_eval_id);

  void derived_map_asynch(const ParamResponsePair& pair);

  void derived_synch(PRPQueue& prp_queue);

  void derived_synch_nowait(PRPQueue& prp_queue);

  int  derived_synchronous_local_analysis(const int& analysis_id);

  const StringArray& analysis_drivers() const;

  const AnalysisCode* analysis_code() const;

private:

  //
  //- Heading: Methods
  //

  /// Convenience function for common code between derived_synch() &
  /// derived_synch_nowait()
  void derived_synch_kernel(PRPQueue& prp_queue, const pid_t pid);

  /// perform the complete function evaluation by managing the input
  /// filter, analysis programs, and output filter
  pid_t fork_application(const bool block_flag);

  /// execute analyses asynchronously on the local processor
  void asynchronous_local_analyses(const int& start, const int& end,
				   const int& step);

  /// execute analyses synchronously on the local processor
  void synchronous_local_analyses(const int& start, const int& end,
				  const int& step);

  /// serve the analysis scheduler and execute analysis jobs asynchronously
  void serve_analyses_asynch();

  //void clear_bookkeeping(); // virtual fn redefinition: clear processIdMap

  //
  //- Heading: Data
  //

  /// ForkAnalysisCode provides convenience functions for forking
  /// individual programs and checking fork exit status
  ForkAnalysisCode forkSimulator;

  /// map of fork process id's to function evaluation id's for
  /// asynchronous evaluations
  std::map<pid_t, int> processIdMap;
};


inline ForkApplicInterface::~ForkApplicInterface() 
{ /* Virtual destructor handles referenceCount at Interface level. */ }


/** This code provides the derived function used by ApplicationInterface::
    serve_analyses_synch() as well as a convenience function for
    ForkApplicInterface::synchronous_local_analyses() below. */
inline int ForkApplicInterface::
derived_synchronous_local_analysis(const int& analysis_id)
{
#ifdef MPI_DEBUG
  Cout << "Blocking fork to analysis " << analysis_id << std::endl; // flush buf
#endif // MPI_DEBUG
  forkSimulator.driver_argument_list(analysis_id);
  forkSimulator.fork_program(BLOCK);
  return 0; // used for failure codes in DirectFn case
}


/** Execute analyses synchronously in succession on the local
    processor (start to end in step increments).  Modeled after
    ApplicationInterface::synchronous_local_evaluations(). */
inline void ForkApplicInterface::
synchronous_local_analyses(const int& start, const int& end, const int& step)
{
  for (int analysis_id=start; analysis_id<=end; analysis_id+=step)
    derived_synchronous_local_analysis(analysis_id);
}


inline const StringArray& ForkApplicInterface::analysis_drivers() const
{ return forkSimulator.program_names(); }


inline const AnalysisCode* ForkApplicInterface::analysis_code() const
{ return &forkSimulator; }


//inline void ForkApplicInterface::clear_bookkeeping()
//{ processIdMap.clear(); }

} // namespace Dakota

#endif
