/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */
//
// ThreadQueue.cpp
//
// Just to get the pragmas to work right for gcc
//

#include <acro_config.h>
#include <utilib/_math.h>
#include <pebbl/ThreadQueue.h>

using namespace std;
using namespace utilib;

namespace pebbl {


//
// This simply updates the priority
//
void ThreadQueue::unblock(ThreadObj* thread, double time)
{
if (queue_type != round_robin) {
   if (head() && (head()->data()->last_reset > thread->last_reset)) {
      reset(thread);
      thread->last_reset = time;
      }
   else {
      if (head()) {
         thread->priority = max(thread->priority,head()->data()->priority);
         if (thread->priority_ctr > 0)
            thread->priority_ctr = max(thread->priority_ctr,head()->data()->priority_ctr);
         }
      }
   }
}



void ThreadQueue::update_priority(ThreadObj* thread, double time, 
							double curr_time)
{
if (thread->priority >= max_proc_priority) {
   reset(thread);
   thread->last_reset = reset(curr_time);
   }

switch (queue_type) {
  case round_robin:
	break;

  case time_weighted_priority:
        thread->prev_priority = thread->priority;
	if (thread->bias > 0)
           thread->priority += time / thread->bias;
	break;

  case bias_weighted_priority:
        thread->prev_priority = thread->priority;
	if (thread->bias > 0) {
           thread->priority = (++thread->priority_ctr)/thread->bias;
           }
	break;
  };
}




double ThreadQueue::reset(double time)
{
if (queue_type == round_robin) return 0.0;

OrderedListItem<ThreadObj*,double> *tmp, *item = head();

while (item) {
  tmp = next(item);
  reset(item->data());
  item->data()->last_reset = time;
  DEBUGPR(2, ucout << "Updating with priority " 
          << item->data()->priority << endl);
  update(item,item->data()->priority);
  DEBUGPR(2, ucout << size() << endl);
  item = tmp;
  }

return time;
}



void ThreadQueue::reset(ThreadObj* thread)
{                          
switch (queue_type) {
  case time_weighted_priority:
      thread->priority -=  max_proc_priority;
      break;

  case bias_weighted_priority:
     DEBUGPR(2, ucout << "RESET " << thread << endl);
      thread->priority_ctr -=  (int) (thread->bias * max_proc_priority);
      thread->priority = thread->priority_ctr/thread->bias;
      DEBUGPR(2, ucout << "RESET " << thread->priority_ctr << " " 
              << thread->priority << endl);
      break;

  default:
	break;
  }
}

} // namespace pebbl
