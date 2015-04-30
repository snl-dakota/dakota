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
// Scheduler.cpp
//
// Bill Hart
//

#include <acro_config.h>
#include <utilib/_math.h>
#include <utilib/seconds.h>
#include <utilib/mpiUtil.h>
#include <pebbl/Scheduler.h>

using namespace std;
using namespace utilib;

namespace pebbl {


//
// Scheduler static data
//
//
int Scheduler::termination_flag=FALSE;


//
// Scheduler Constructor 
//
//
Scheduler::Scheduler(const int numTags)
 : wtime_flag(0),
   interrupts(numTags),
   threadGroup(0)
#ifdef ACRO_HAVE_MPI
   , num_requests(0)
#endif
{ 
  total_time=0.0;
  partial_time=0.0;
  run_time=0.0;
}


//
// Guts of destructor and reset method (JE code moved from old destructor)
// Note: the scheduler does *not* assume ownership of thread objects,
// so while this clears out all data structures in the schedule, it does 
// not attempt to delete the threads themselvs.  (JE)
//
void Scheduler::clearQueues()
{
  for (size_type i=0; i<threadGroup.size(); i++)
    delete threadGroup[i];
  threadGroup.resize(0);

  deadList.clear();
  algBlockedList.clear();

#ifdef ACRO_HAVE_MPI
  for (size_type j=0; j<interrupts.size(); j++)
    if (interrupts[j])
      delete interrupts[j];
  interrupts.resize(0);
  requests_ndx.resize(0);
  testsome_status.resize(0);
  num_requests = 0;
#endif
  
}


//
// Reset to starting state without destruction
//
//
void Scheduler::reset()
{
  clearQueues();
  threadGroup.resize(0);
  total_time=0.0;
  partial_time=0.0;
  run_time=0.0;
}


//
// Scheduler Destructor
//
//
Scheduler::~Scheduler()
{
  clearQueues();
// Tags used to be reset here, but that wasn't compatible with 
// templatized parameter registration.
}


//
// Scheduler add method
//
// Insert a ThreadQueue object into the queue.  This queue object
// goes at the end of the queue list.
//
int Scheduler::add(ThreadQueue* queue)
{
threadGroup.resize(threadGroup.size()+1);
threadGroup[threadGroup.size()-1] = queue;
return ((int)threadGroup.size() - 1);
}



//
// Scheduler dump method
//
// Prints debugging information about the threads
//
void Scheduler::dump()
{
  //
  //  INFO ABOUT ALL OF THE THREAD GROUPS
  //
    ucout << "Scheduler - --- DUMP BEGIN -----------------------------" << std::endl << Flush;
    ucout << "Scheduler - blockedList : " << Flush;
    ListItem<ThreadObj*> *item;
    item=algBlockedList.head();
    while (item) {
      ucout << "(" << item->data() << "," << item->data()->name << 
	  	"," << item->data()->priority <<
		"," << item->data()->run_time << "," << 
		item->data()->nruns << ") ";
      item = algBlockedList.next(item);
      }
    ucout << std::endl << Flush;

    ucout << "Scheduler - DeadList : ";
    item=deadList.head();
    while (item) {
      ucout << "(" << item->data() << "," << item->data()->name
	  << "," << item->data()->priority
	  << "," << item->data()->run_time 
	  << "," << item->data()->nruns << ") ";
      item = deadList.next(item);
      }
    ucout << std::endl << Flush;

    ucout << "Scheduler - Active Ready Queues" << std::endl;
    // size_type i;  JE -- this caused warnings in GCC 3.2.2
    int i;
    for (i=0; i < (int)threadGroup.size(); i++) {
      ucout << "Scheduler - Group " << i << ": ";
      OrderedListItem<ThreadObj*,double> *item = threadGroup[i]->head();
      while (item) {
        ucout << "(" << item->data() 
	  << "," << item->key() << "," << item->data()->name 
	  << "," << item->data()->priority
	  << "," << item->data()->run_time << "," 
          << item->data()->nruns << ") ";
        item = threadGroup[i]->next(item);
        }
      ucout << std::endl << Flush;
      } 
#ifdef ACRO_HAVE_MPI
    DEBUGPR(5,
     ucout << "Scheduler - Request Indeces: ndx null_req empty" << endl;
     for (i=0; i<num_requests; i++) {
       ucout << "Scheduler -  " << (requests_ndx[i] - MIN_TAG_VALUE) 
	     << " " << (requests[i] == MPI_REQUEST_NULL) 
	     << " "  << interrupts[requests_ndx[i]]->empty() << endl;
       }
     );
#endif

    for (size_type i=0; i<interrupts.size(); i++) {
      ucout << "Scheduler - Interrupt " << i << ": ";
      ListItem<ThreadObj*> *item = interrupts[i]->head();
      while (item) {
        ucout << "(" << item << " " << item->data() << "," << 
			item->data()->name << 
	  "," << item->data()->priority <<
	  "," << item->data()->run_time << "," << item->data()->nruns << ") ";
        item = interrupts[i]->next(item);
        }
      ucout << endl << Flush;
      } 
    ucout << "Scheduler - --- DUMP END   -----------------------------" << endl << Flush;
}


//
// Scheduler execute method
//
// Runs the scheduler
// 
int Scheduler::execute()
{
size_type group_ndx=0;
termination_flag = FALSE;
double ttime=getTime();


//
// Dump the state of the scheduler
//
DEBUGPR(1,dump());


//
// Main loop
//
while (!termination_flag) {
  state_changed=FALSE;
  //
  // Time this iteration began
  //
  double itime=getTime();

  //
  // Unblock threads waiting for other algorithmic factors
  //
  {
  ListItem<ThreadObj*>* ndx = algBlockedList.head();
  while (ndx) {
    if (!(ndx->data()->blocked())) {
       ListItem<ThreadObj*>* tmp = ndx;
       ndx = algBlockedList.next(ndx);
       ThreadObj* thread;
       algBlockedList.remove(tmp,thread);
       if (thread->ready())
          threadGroup[thread->group()]->unblock(thread,getTime());
       insert(ThreadObj::RunOK,thread);		// Move to an interrupt queue
       state_changed=TRUE;
       }
    else
       ndx = algBlockedList.next(ndx);
    }
  }

  check_waiting_threads();
  
  if (termination_flag) break;

  //
  // Find highest group thread that is not empty
  //
  group_ndx=0;
  while (group_ndx < threadGroup.size()) {
    if (!(threadGroup[group_ndx]->empty()))
       break;
    group_ndx++;
    }
  if (group_ndx < threadGroup.size()) {
     DEBUGPR(2, ucout << "Scheduler: Running group=" << group_ndx 
             << " empty?=" << threadGroup[group_ndx]->empty() << endl);
     }
 
  //
  // Run highest group thread that is ready and then put it in a
  // a blocked list or ready queue
  //
  if (group_ndx < threadGroup.size()) {
     ThreadObj* thread;
     double priority;
     threadGroup[group_ndx]->remove(thread,priority);	// remove from queue
     DEBUGPR(3, ucout << "Scheduler: Running thread priority=" 
             << thread->priority << " priority=" << priority 
             << " name=" << thread->name << endl);

     double time_start = getTime();
     ThreadObj::RunStatus result = thread->exec();
     double time_used = getTime() - time_start;
     thread->nruns++;
     thread->run_time += time_used;
     run_time += time_used;
     DEBUGPR(3, ucout << "Scheduler: Running thread priority=" 
             << thread->priority << " priority=" << priority << endl);
     threadGroup[thread->group()]->update_priority(thread,time_used,getTime());
     DEBUGPR(3, ucout << "Scheduler: Running thread priority=" 
             << thread->priority << " priority=" << priority << endl);

     insert(result, thread);
     DEBUGPR(3, dump());
     } 
  else {
     partial_time += getTime() - itime;
     }
  }

total_time = getTime() - ttime;
return OK;
}




void Scheduler::insert(ThreadObj::RunStatus result, ThreadObj* thread)
{
//
// Add terminated threads to the deadList.
//
if (result != ThreadObj::RunOK) {
   deadList.add(thread);
   return;
   }

switch (thread->state()) {
   //
   // The thread's state is ThreadReady, so add it to the appropriate run queue.
   //
   case ThreadObj::ThreadReady:
        if (thread->group() >= (int) threadGroup.size()) {
           EXCEPTION_MNGR(runtime_error, "Attempting to add thread in group "
			  << thread->group() << " ; Only " 
			  << threadGroup.size() << " groups exist");
           return;
           }
	threadGroup[thread->group()]->add(thread,thread->priority);	
								// add to queue
	break;

#ifdef ACRO_HAVE_MPI
   //
   // The thread's state is ThreadWaiting, so the thread is waiting for 
   case ThreadObj::ThreadWaiting:
        {
        if ((int) interrupts.size() <= (thread->tag - MIN_TAG_VALUE)) {
	  // Let's hope this stays small
	   size_type tmp = interrupts.size();
           interrupts.resize(thread->tag + 1 - MIN_TAG_VALUE);
           requests.resize(interrupts.size());
           requests_ndx.resize(interrupts.size());
           testsome_status.resize(thread->tag + 1 - MIN_TAG_VALUE);
           testsome_ndx.resize(thread->tag + 1 - MIN_TAG_VALUE);
	   for (size_type i=tmp; i<interrupts.size(); i++) {
             interrupts[i] = new LinkedList<ThreadObj*>();
             requests[i] = MPI_REQUEST_NULL;
             }
           }
        if (interrupts[(int)(thread->tag - MIN_TAG_VALUE)]->empty()) {
           requests_ndx[num_requests] = thread->tag - MIN_TAG_VALUE;
           requests[num_requests++] = thread->request;
           }
	// add to queue
        interrupts[(int)(thread->tag - MIN_TAG_VALUE)]->add(thread);
        }
	break;
#endif

   case ThreadObj::ThreadBlocked:
	algBlockedList.add(thread);
	break;

   default:
	EXCEPTION_MNGR(runtime_error, "UNKNOWN SCHEDULER STATE");
   };
}


//
// Scheduler getTime method
//
// Returns the current time stamp.  When running MPI, this may return
// the wall-clock time.  By default, this returns the number of CPU seconds
// since the process was started.
//
double Scheduler::getTime()
{
// #ifdef ACRO_HAVE_MPI        // JE: for consistency, the rest of PEBBL
// if (wtime_flag)             // is actually expecting this to be CPU time
//    return MPI_Wtime();
// else
// #endif
   return CPUSeconds();
}



//
// Unblock threads waiting for messages
//

void Scheduler::check_waiting_threads()
{
#ifdef ACRO_HAVE_MPI
//
// The 'NEW_CODE' definition enables the use of the MPI_Testsome method instead
// of the iterative use of the MPI_Test command.  This appears to be slightly
// more efficient in the tests that I've done on a NOW of Suns, but I'm leaving
// the old option in the code for now.   WEH
//
#define NEW_CODE
#ifdef NEW_CODE
  if (num_requests > 0) {
    //
    // Setup call to MPI_TESTSOME
    //
    int outcount;
    if (uMPI::testsome(num_requests, requests.data(), outcount,
		       testsome_ndx.data(), testsome_status.data())) {
      //
      // Loop through the messages received.  Note that this loop is in
      // reverse order to ensure that the deletion of requests from the
      // 'requests' and 'requests_ndx' lists works properly.
      //
      for (int j=outcount-1; j>=0; j--) {
	int i= requests_ndx[testsome_ndx[j]];

	DEBUGPR(5, ucout << "Scheduler - Received message for " 
		<< (i+MIN_TAG_VALUE) << " thread!" << endl);
          ThreadObj* thread = interrupts[i]->head()->data();
          interrupts[i]->remove(thread);
          if (interrupts[i]->empty()) {
	    requests[testsome_ndx[j]] = MPI_REQUEST_NULL;
	    if (testsome_ndx[j] < (num_requests-1)) {
	      requests[testsome_ndx[j]] = requests[--num_requests];
	      requests_ndx[testsome_ndx[j]] = requests_ndx[num_requests];
	    }
	    else
	      num_requests--;
	  }
          else
	    requests[testsome_ndx[j]] = interrupts[i]->head()->data()->request;
          
          thread->status = testsome_status[j];
          thread->unwaiting();
	  thread->request = MPI_REQUEST_NULL;

	  // If thread is ready, update it's priority
          if (thread->ready())
	    threadGroup[thread->group()]->unblock(thread,getTime());
	  insert(ThreadObj::RunOK,thread);
          state_changed=TRUE;
      }
    }
  }
#else
  for (int j=0; j<num_requests; j++) {
    if (requests[j] != MPI_REQUEST_NULL) {
      MPI_Status status;
      if (uMPI::test(&requests[j],&status)) {
	int i = requests_ndx[j];

	DEBUGPR(5, ucout << "Scheduler - Received message for " << (requests_ndx[j]+MIN_TAG_VALUE) << " thread!" << endl);
          ThreadObj* thread = interrupts[i]->head()->data();
          interrupts[i]->remove(thread);
          if (interrupts[i]->empty()) {
	    requests[j] = MPI_REQUEST_NULL;
	    requests[j] = requests[--num_requests];
	    requests_ndx[j] = requests_ndx[num_requests];
	  }
          else
	    requests[j] = interrupts[i]->head()->data()->request;
     
          thread->status = status;
          thread->unwaiting();
	  thread->request = MPI_REQUEST_NULL;

	  // If thread is ready, update it's priority
          if (thread->ready())
	    threadGroup[thread->group()]->unblock(thread,getTime());
	  insert(ThreadObj::RunOK,thread);
          state_changed=TRUE;
      }
    }
  }
#endif
#endif

  DEBUGPR(3, if (state_changed) dump());
  
}

} // namespace pebbl
