/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SpawnApplicInterface
//- Description:  Derived class for the case when analysis code simulators use
//-               spawnvp to provide the function evaluations
//- Owner:        Dave Gay / Bill Bohnhoff
//- Version: $Id: SpawnApplicInterface.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef SPAWN_APPLIC_INTERFACE_H
#define SPAWN_APPLIC_INTERFACE_H

#include "ProcessHandleApplicInterface.hpp"


namespace Dakota {

/// Derived application interface class which spawns simulation codes
/// using spawnvp.

/** SpawnApplicInterface is used on Windows systems and is a peer to
    ForkApplicInterface for Unix systems. */

class SpawnApplicInterface: public ProcessHandleApplicInterface
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  SpawnApplicInterface(const ProblemDescDB& problem_db);
  /// destructor
  ~SpawnApplicInterface();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void wait_local_evaluations(PRPQueue& prp_queue);
  void test_local_evaluations(PRPQueue& prp_queue);

  pid_t create_analysis_process(bool block_flag, bool new_group);

  size_t wait_local_analyses();
  size_t test_local_analyses_send(int analysis_id);

  //
  //- Heading: Data
  //

};


inline SpawnApplicInterface::
SpawnApplicInterface(const ProblemDescDB& problem_db):
  ProcessHandleApplicInterface(problem_db)
{ }


inline SpawnApplicInterface::~SpawnApplicInterface() 
{ /* Virtual destructor handles referenceCount at Interface level. */ }

} // namespace Dakota

#endif

