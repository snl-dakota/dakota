/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ExecutableEnvironment
//- Description: 
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: ExecutableEnvironment.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef EXECUTABLE_ENVIRONMENT_H
#define EXECUTABLE_ENVIRONMENT_H

#include <boost/shared_ptr.hpp>
#include "DakotaEnvironment.hpp"

namespace Dakota {

// forward declaration due to PIMPL for conditional compilation of curl
class TrackerHTTP;


/// Environment corresponding to execution as a stand-alone application.

/** This environment corresponds to a stand-alone executable program,
    e.g., main.cpp.  It sets up the ParallelLibrary, ProgramOptions,
    and ProblemDescDB objects based on access to command line arguments. */

class ExecutableEnvironment: public Environment
{
public:
  
  //
  //- Heading: Constructors and destructor
  //

  ExecutableEnvironment();                       ///< default constructor
  ExecutableEnvironment(int argc, char* argv[]); ///< constructor
  ~ExecutableEnvironment();                      ///< destructor
    
  //
  //- Heading: Virtual function redefinitions
  //

  void execute();

  //
  //- Heading: Member functions
  //

  /// determine input check versus run mode
  bool check();

private:

  //
  //- Heading: Convenience member functions
  //
    
  //
  //- Heading: Data members
  //

  /// posts usage data to Web server; using shared_ptr due to
  /// potentially incomplete type and requirements for checked_delete
  /// in debug builds (scoped_ptr would suffice)
  boost::shared_ptr<TrackerHTTP> usageTracker; 
};

} // namespace Dakota

#endif
