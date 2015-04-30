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
 * \file ThreadQueue
 *
 * Defines the pebbl::ThreadQueue class.
 */

#ifndef pebbl_ThreadQueue_h
#define pebbl_ThreadQueue_h

#include <acro_config.h>
#include <utilib/OrderedList.h>
#include <pebbl/ThreadObj.h>

namespace pebbl {

using utilib::CommonIO;
using utilib::OrderedList;

//
// ThreadQueue defines a mechanism for controling the how threads are
// scheduled within a given priority queue.
//
enum QueueType {round_robin, time_weighted_priority, bias_weighted_priority};


class ThreadQueue : public OrderedList<ThreadObj*,double>, public CommonIO
{
protected:

  QueueType queue_type;
  void reset(ThreadObj*);

public:

  double max_proc_priority;

  ThreadQueue(QueueType type=round_robin) 
	{
	queue_type=type;
	if (type == bias_weighted_priority)
	   max_proc_priority=1.0;
	else max_proc_priority=10000.0;
	}

  void update_priority(ThreadObj* thread, double time, double curr_time);
  void unblock(ThreadObj* thread, double time);

  double reset(double time);
};

} // namespace pebbl

#endif
