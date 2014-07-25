/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        UsageTracker
//- Description:  Implementation of usage tracking wrapper
//-               
//- Owner:        Brian Adams, Sandia National Laboratories


#include "UsageTracker.hpp"
#ifdef DAKOTA_USAGE_TRACKING
#include "TrackerHTTP.hpp"
#endif

namespace Dakota {

UsageTracker::UsageTracker()
{ /* empty ctor */ }

/** standard constructor; will output on rank 0 and only initializes
    if tracking compiled in and not disable by environment */
UsageTracker::UsageTracker(int world_rank) 
{
#ifdef DAKOTA_USAGE_TRACKING
  // Avoid tracking if user specified none via env var or on rank > 0
  char *ptr_notrack = std::getenv("DAKOTA_NO_TRACKING");
  if (ptr_notrack == NULL && world_rank == 0)
    pTrackerHTTP.reset( new TrackerHTTP(world_rank) );
#endif
}

void UsageTracker::post_start(ProblemDescDB& problem_db)
{
#ifdef DAKOTA_USAGE_TRACKING
  if (pTrackerHTTP.get() != NULL)
    pTrackerHTTP->post_start(problem_db);
#endif
}

void UsageTracker::post_finish(unsigned runtime)
{
#ifdef DAKOTA_USAGE_TRACKING
  if (pTrackerHTTP.get() != NULL)
    pTrackerHTTP->post_finish(runtime);
#endif
}


}  // namespace Dakota
