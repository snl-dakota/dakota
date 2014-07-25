/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        UsageTracker
//- Description:  Class declaration for usage tracker wrapper class
//-               
//- Owner:        Brian Adams, Sandia National Laboratories

#ifndef USAGE_TRACKER_H
#define USAGE_TRACKER_H

#include <boost/shared_ptr.hpp>
#include "dakota_system_defs.hpp"

namespace Dakota {

// forward declarations
class ProblemDescDB;
class TrackerHTTP;

/// Lightweight class to manage conditionally active Curl-based HTTP
/// tracker via PIMPL
/** All conditional compilation in managed in the cpp file; all
    operations are no-op in this wrapper if not enabling
    tracking... */
class UsageTracker {

public:

  /// default construction: no output 
  UsageTracker();

  /// standard constructor; will output on rank 0
  UsageTracker(int world_rank);

  /// post the start of an analysis and archive start time
  void post_start(ProblemDescDB& problem_db);

  /// post the completion of an analysis including elapsed time
  void post_finish(unsigned runtime = 0); 

private:

  /// copy construction is disallowed
  UsageTracker(const UsageTracker&);

  /// posts usage data to Web server; using shared_ptr due to
  /// potentially incomplete type and requirements for checked_delete
  /// in debug builds (scoped_ptr would suffice)
  boost::shared_ptr<TrackerHTTP> pTrackerHTTP; 

};

} // namespace Dakota

#endif // USAGE_TRACKER_H
