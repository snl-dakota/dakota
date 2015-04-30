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

/**
 * \file ThreadObj.h
 * \author William Hart
 *
 * Implements thread objects used with the Scheduler object.
 */

#ifndef pebbl_ThreadObj_h
#define pebbl_ThreadObj_h

#include <acro_config.h>
#include <utilib/CommonIO.h>

#ifdef ACRO_HAVE_MPI
#include <mpi.h>
#include <pebbl/MessageID.h>
#endif

namespace pebbl {

using utilib::CommonIO;

class ThreadQueue;
class Scheduler;

//
// ThreadObj is an abstract class used to define uninterupted threads
//

class ThreadObj : virtual public CommonIO
{
public:

  enum ThreadState {ThreadReady, ThreadBlocked, ThreadWaiting};
  enum RunStatus   {RunOK, RunSTOP};

private:

  static int num_threads;
  int id;

  friend class ThreadQueue;
  friend class Scheduler;

  double last_reset;
  double priority,prev_priority;
  int priority_ctr;

protected:

  int Group;
  ThreadState state_flag;
#ifdef ACRO_HAVE_MPI
  MessageID tag;
  MPI_Request request;
  MPI_Status  status;
#endif

  virtual RunStatus run(double* controlParam) = 0;

public:

  //
  // Statistics maintained by the scheduler
  //

  int nruns;
  double run_time;	
 
  double defaultControlParam;

  double bias;

  const char*  name;

#ifdef ACRO_HAVE_MPI
  ThreadObj(MPI_Comm comm=MPI_COMM_WORLD);
  ThreadObj(int group_, double bias_, MPI_Comm comm=MPI_COMM_WORLD);
#else
  ThreadObj();
  ThreadObj(int group_, double bias_);
#endif

  virtual RunStatus exec() {return run(&defaultControlParam);}

  int& group() {return Group;}

  virtual ThreadState state()  {return state_flag;}
          bool ready()   {return (state()==ThreadReady);}
          bool blocked() {return (state()==ThreadBlocked);}
          bool waiting() {return (state()==ThreadWaiting);}
  virtual void unwaiting() {state_flag=ThreadReady;}
					// Called after job is done waiting

  int operator==(const ThreadObj& thread) {return (id == thread.id);}

  void dump(std::ostream& s);

// JE this looks like it isn't used!
// private:
//   char name_string[64];
};

} // namespace pebbl

#endif
