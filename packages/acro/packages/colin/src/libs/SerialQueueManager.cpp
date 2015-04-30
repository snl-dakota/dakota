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

#include <colin/eval/SerialQueueManager.h>

using std::runtime_error;
using std::map;
using std::pair;
using std::cerr;
using std::endl;

namespace colin
{

const double SerialQueueManager::ZeroAlloc = 0.001;

//======================================================================
// SerialQueueManager member functions
//======================================================================

SerialQueueManager::SerialQueueManager()
{
}


//----------------------------------------------------------------------
// 

/** If queue_id == ALL_SUBQUEUES, all queues are deleted; otherwise,
 *  only the specified queue is deleted.
 */
void
SerialQueueManager::clear_evaluations(solverID_t solver_id, queueID_t queue_id)
{
   queueMap_t::iterator s_it = m_queues->find(solver_id);
   if ( s_it == m_queues->end() )
      return;

   if ( queue_id == ALL_SUBQUEUES )
   {
      subqueueMap_t::iterator q_it = s_it->second.subqueues.begin();
      subqueueMap_t::iterator q_itEnd = s_it->second.subqueues.end();
      for ( ; q_it != q_itEnd; ++q_it )
         q_it->second.pqueues.clear();
   }
   else
   {
      subqueueMap_t::iterator q_it = s_it->second.subqueues.find(queue_id);
      if ( q_it != s_it->second.subqueues.end() )
         q_it->second.pqueues.clear();
   }
}


EvaluationID 
SerialQueueManager::queue_evaluation( AppRequest &request,
                                      evalMngrID_t evalMngr_id,
                                      solverID_t solver_id,
                                      queueID_t queue_id,
                                      evalPriority_t priority )
{
   queueMap_t::iterator s_it = m_queues->find(solver_id);
   if (s_it == m_queues->end())
   {
      EXCEPTION_MNGR(runtime_error, "SerialQueueManager::queue_evaluation():"
                     " invalid (unknown) solver id (" << solver_id << ")");
   }
   if (queue_id != NO_SUBQUEUE)
   {
      if (s_it->second.subqueues.find(queue_id)
            == s_it->second.subqueues.end())
      {
         EXCEPTION_MNGR(runtime_error, "SerialQueueManager::"
                        "queue_evaluation(): invalid (unknown) queue id (" 
                        << queue_id << ")");
      }
   }

   // Insert the request...
   EvaluationID evalID = EvaluationID(evalMngr_id, solver_id, queue_id);
   s_it->second.subqueues[queue_id].pqueues[priority].push_back
      (RequestRecord(evalID, request));
   return evalID;
}



EvaluationID
SerialQueueManager::get_next_request( AppRequest& request, 
                                      solverID_t solver_id, 
                                      queueID_t queue_id )
{
   // check for stupidity...
   queueMap_t::iterator s_it = m_queues->find(solver_id);
   if ( s_it == m_queues->end() )
      return EvaluationID();

   subqueueMap_t::iterator q_it;
   subqueueMap_t::iterator q_itEnd = s_it->second.subqueues.end();

   // Did the user specify a specific queue?
   if ( queue_id != ALL_SUBQUEUES )
   {
      q_it = s_it->second.subqueues.find(queue_id);
      if ( q_it == q_itEnd )
         return EvaluationID();
      if ( q_it->second.pqueues.empty() )
         return EvaluationID();
   }
   else
   {
      // find the next evaluation we *should* perform
      execSequence_t &execSequence = execSequenceMap[solver_id];
      int pass = 0;
      while ( pass < 2 )
      {
         while ( ! execSequence.empty() )
         {
            q_it = s_it->second.subqueues.find(execSequence.front());
            execSequence.pop_front();
            if (( q_it == q_itEnd ) || ( q_it->second.pqueues.empty() ))
               continue;

            pass = 3;
            break;
         }

         if (( execSequence.empty() ) && ( pass < 2 ))
         {
            generate_exec_sequence(execSequence, s_it);
            pass++;
         }
      }
      if ( pass == 2 )
         return EvaluationID();
   }   

   priorityMap_t::iterator pq_it = q_it->second.pqueues.begin();
   request = pq_it->second.front().request;
   EvaluationID evalID = pq_it->second.front().evalID;

   pq_it->second.pop_front();
   if ( pq_it->second.empty() )
      q_it->second.pqueues.erase(pq_it); 

   return evalID;
}


void SerialQueueManager::new_queue_alloc(queueMap_t::iterator s_it)
{
   if ( s_it == m_queues->end() )
      return;

   generate_exec_sequence(execSequenceMap[s_it->first], s_it);

   LocalQueueManager::new_queue_alloc(s_it);
}


void
SerialQueueManager::
generate_exec_sequence(execSequence_t &sequence, queueMap_t::iterator s_it)
{
   sequence.clear();
   if ( s_it == m_queues->end() )
      return;

   // If there are no queues defined and no one has ever set a queue
   // allocation, then we just return the default queue.
   if (s_it->second.subqueues.empty())
   {
      sequence.push_back(NO_SUBQUEUE);
      return;
   }

   subqueueMap_t::iterator q_it = s_it->second.subqueues.begin();
   subqueueMap_t::iterator q_itEnd = s_it->second.subqueues.end();
   // as the allocations should be normalized, the max should be <= 1;
   double min = 2;
   for( ; q_it != q_itEnd; ++q_it )
      if (( min > q_it->second.allocation ) 
          && ( q_it->second.allocation > ZeroAlloc))
         min = q_it->second.allocation;

   if ( min > 1.1 )
   {
      // If we don't have a sensible allocation, just return equal allocation
      for ( q_it = s_it->second.subqueues.begin(); q_it != q_itEnd; ++q_it)
         sequence.push_back(q_it->first); 
   }
   else
   {
      // The "best" thing to do here is to find least multiple that gives
      // a rationally close to integer ratio among allocations.  Yippie.
      // This is more work than it is worth... so we will just get it to
      // within a multiple of 4
      map<queueID_t, double> tmp;
      for( q_it = s_it->second.subqueues.begin(); q_it != q_itEnd; ++q_it)
         tmp.insert( tmp.end(), pair<queueID_t, double>
                     (q_it->first, q_it->second.allocation) );

      double frac;
      double i_part;
      int mult = 1;

      map<queueID_t, double>::iterator it = tmp.begin();
      map<queueID_t, double>::iterator itEnd = tmp.end();
      for ( ; it != itEnd; ++it)
      {
         if ( it->second < ZeroAlloc )
            it->second = 0; 
         else
         {
            it->second /= min;
            frac = std::modf(it->second, &i_part);
            if ((mult <= 2) && (frac > .37) && (frac < .63))
               mult = 2; 
            else if ((mult < 4) && (frac > 0.13) && (frac < 0.87))
               mult = 4; 
         }
      }

      if (mult > 1)
         for (it = tmp.begin(); it != itEnd; ++it)
            it->second = std::floor(it->second*mult + 0.5);

      for (it = tmp.begin(); it != itEnd; ++it)
         if ( it->second > ZeroAlloc )
            sequence.insert( sequence.end(), 
                             static_cast<size_t>(it->second),
                             it->first );
   }

   // (pseudo) randomize the new sequence... believe it or not, this is
   // O(N*log(N))
   map<int, execSequence_t> sorted;
   while ( ! sequence.empty() )
   {
      execSequence_t &tmp = sorted[std::rand()];
      tmp.splice(tmp.end(), sequence, sequence.begin());
   }
   map<int, execSequence_t>::iterator sorted_it = sorted.begin();
   map<int, execSequence_t>::iterator sorted_itEnd = sorted.end();
   for (; sorted_it != sorted_itEnd; ++sorted_it)
      sequence.splice(sequence.end(), sorted_it->second); 

}

} //namespace colin
