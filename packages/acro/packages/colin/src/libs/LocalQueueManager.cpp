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

#include <colin/eval/LocalQueueManager.h>

#include <utilib/exception_mngr.h>


using std::runtime_error;
using std::map;
using std::cerr;
using std::endl;

namespace colin
{

//======================================================================
// LocalQueueManager member functions
//======================================================================

LocalQueueManager::LocalQueueManager()
   : m_queues(new queueMap_t()),
     m_lastSolverID(0),
     m_lastQueueID(0)
{
}


LocalQueueManager::~LocalQueueManager()
{
   delete m_queues;
   m_queues = NULL;
}

//----------------------------------------------------------------------
// Solver queue management

LocalQueueManager::solverID_t 
LocalQueueManager::get_new_solver_queue()
{
   // This will re-allocate solver allocations so that the new solver
   // will get the default 1/size() allocation, and all other
   // allocations will be re-distributed proportionally.
   double scale = static_cast<double>(m_queues->size());
   scale /= (scale+1.0);
      
   queueMap_t::iterator it = m_queues->begin();
   queueMap_t::iterator itEnd = m_queues->end();
   for( ; it != itEnd; ++it )
   {
      it->second.allocation *= scale;
   }

   it = m_queues->insert( m_queues->end(), queueMap_t::value_type
                          (++m_lastSolverID, SolverInfo()) );
   it->second.allocation = 1.0 / ( 1.0 - scale );

   // Allow derived classes to know the solver allocations changes
   new_solver_alloc();

   return it->first;
}


void
LocalQueueManager::release_solver_queue(solverID_t solver_id)
{
   queueMap_t::iterator it = m_queues->find(solver_id);
   if (it == m_queues->end())
   {
      EXCEPTION_MNGR(std::runtime_error, "LocalQueueManager::"
                     "release_solver_queue(): invalid existing solverID.");
   }

   double scale = it->second.allocation;
   if ( scale < 1.0 )
   {
      scale = 1.0 / (1.0 - scale); 
   }
   m_queues->erase(it);
   
   // rescale all our remaining solver allocations.
   queueMap_t::iterator itEnd = m_queues->end();
   for(it = m_queues->begin() ; it != itEnd; ++it )
   {
      it->second.allocation *= scale;
   }

   // Allow derived classes to know the solver allocations changes
   new_solver_alloc();
}


void
LocalQueueManager::get_solver_alloc(solverAlloc_t &alloc) const
{
   alloc.clear();
   queueMap_t::const_iterator it = m_queues->begin();
   queueMap_t::const_iterator itEnd = m_queues->end();
   for (; it != itEnd; ++it)
      alloc.insert( alloc.end(), solverAlloc_t::value_type
                    (it->first, it->second.allocation) );
}


double
LocalQueueManager::get_solver_alloc(solverID_t solver) const
{
   queueMap_t::const_iterator it = m_queues->find(solver);
   return it == m_queues->end() ? 0 : it->second.allocation;
}


/** set_solver_alloc() will set the "computational allocation" that each
 *  active solver gets.  The allocations are normalized into the range
 *  [0,1].  Each allocation will (later) be subdivided among each
 *  solver's queues.
 *
 *  Nomenclature:
 *    - N = Number of active solvers 
 *    - M = Number of solvers in the provided alloc map
 *
 *  Solver allocations are set using the following rules:
 *    - Any active solver not listed in the provided alloc map gets 1/N
 *    - Allocations provided in the alloc map will be normalized so that 
 *      they sum to M/N.
 *    - If there are any allocations < 0, then the smallest one is treated 
 *      as 0, and everything is (linearly) scaled up before normalization.
 */
void
LocalQueueManager::set_solver_alloc(const solverAlloc_t &alloc)
{
   /// WEH - is this an error?
   //if (alloc.size() == 0) return;
   /// JDS - Nope. It is actually an easy way to set a uniform allocation

   // normalize the provided allocations
   solverAlloc_t::const_iterator new_it = alloc.begin();
   solverAlloc_t::const_iterator new_itEnd = alloc.end();
   double min = 0;
   double norm = 0;
   for (; new_it != new_itEnd; ++new_it)
   {
      if (min > new_it->second)
      { min = new_it->second; }
      norm += new_it->second;
   }
   if (min < 0)
   { norm -= min * alloc.size(); }
   if (( m_queues->size() > 0 ) && ( alloc.size() > 0 ))
   {
      norm *= ( static_cast<double>(m_queues->size()) / 
                static_cast<double>(alloc.size()) );
   }

   double default_alloc = m_queues->size();
   if (default_alloc > 0)
   { default_alloc = 1.0 / default_alloc; }

   // update the actual assignemnts
   new_it = alloc.begin();
   queueMap_t::iterator it = m_queues->begin();
   queueMap_t::iterator itEnd = m_queues->end();
   while (it != itEnd)
   {
      if ((new_it == new_itEnd) || (it->first < new_it->first))
      {
         it->second.allocation = default_alloc;
         ++it;
         continue;
      }
      if (it->first == new_it->first)
      {
         if ( norm == 0 )
         { it->second.allocation = 0; }
         else
         { it->second.allocation = (new_it->second - min) / norm; }

         ++it;
         ++new_it;
         continue;
      }
      if (it->first > new_it->first)
      {
         EXCEPTION_MNGR(std::runtime_error, "LocalQueueManager::"
                        "set_solver_alloc(): specified new allocation for "
                        "unknown solver ID.");
      }
   }
   if (new_it != new_itEnd)
   {
      EXCEPTION_MNGR(std::runtime_error, "LocalQueueManager::"
                     "set_solver_alloc(): specified new allocation for "
                     "unknown solver ID.");
   }

   // Allow derived classes to know the solver allocations changes
   new_solver_alloc();
}

//----------------------------------------------------------------------
// Per-solver sub-queue management

/// Get a new sub-queue id for the given solver
LocalQueueManager::queueID_t 
LocalQueueManager::get_new_subqueue(solverID_t solver_id)
{
   queueMap_t::iterator s_it = m_queues->find(solver_id);
   if ( s_it == m_queues->end() )
   {
      EXCEPTION_MNGR(std::runtime_error, "LocalQueueManager::"
                     "get_new_subqueue_id(): invalid existing solverID.");
   }
   subqueueMap_t &subqueues = s_it->second.subqueues;

   // This will re-allocate queue percentages so that the new queue will
   // get the default 1/size() allocation, and all other allocations will
   // be re-distributed proportionally.
   double scale = static_cast<double>(subqueues.size());
   scale /= (scale+1.0);
   
   subqueueMap_t::iterator it = subqueues.begin();
   subqueueMap_t::iterator itEnd = subqueues.end();
   for( ; it != itEnd; ++it )
   {
      it->second.allocation *= scale;
   }

   it = subqueues.insert( subqueues.end(), subqueueMap_t::value_type
                          (++m_lastQueueID, SubQueueInfo()) );
   it->second.allocation = 1.0 / ( 1.0 - scale );

   // Allow derived classes to know the queue allocations changes
   new_queue_alloc(s_it);

   return m_lastQueueID;
}


void
LocalQueueManager::release_subqueue(solverID_t solver_id, queueID_t queue_id)
{
   queueMap_t::iterator s_it = m_queues->find(solver_id); 
   if ( s_it == m_queues->end() )
   {
      EXCEPTION_MNGR(std::runtime_error, "LocalQueueManager::"
                     "release_subqueue(): invalid solverID.");
   }

   // remove any queued evaluations for the specified queue(s)
   clear_evaluations(solver_id, queue_id);

   // special case: delete all queues
   if ( queue_id == ALL_SUBQUEUES )
   {
      s_it->second.subqueues.clear();
      // Allow derived classes to know the queue allocations changes
      new_queue_alloc(s_it);
      return;
   }

   subqueueMap_t::iterator it = s_it->second.subqueues.find(queue_id);
   subqueueMap_t::iterator itEnd = s_it->second.subqueues.end();
   if ( it == itEnd )
   {
      EXCEPTION_MNGR(std::runtime_error, "LocalQueueManager::"
                     "release_subqueue(): invalid queueID.");
   }

   double scale = it->second.allocation;
   if ( scale < 1.0 )
   {
      scale = 1.0 / (1.0 - scale); 
   }
   s_it->second.subqueues.erase(it);
   
   // rescale all our remaining queue allocations.
   for(it = s_it->second.subqueues.begin() ; it != itEnd; ++it )
   {
      it->second.allocation *= scale;
   }

   // Allow derived classes to know the queue allocations changes
   new_queue_alloc(s_it);
}


void
LocalQueueManager::
get_subqueue_alloc(solverID_t solver_id, queueAlloc_t &alloc) const
{
   alloc.clear();
   queueMap_t::const_iterator s_it = m_queues->find(solver_id);
   if ( s_it == m_queues->end() )
      return; 

   subqueueMap_t::const_iterator it = s_it->second.subqueues.begin();
   subqueueMap_t::const_iterator itEnd = s_it->second.subqueues.end();
   for( ; it != itEnd; ++it )
      alloc.insert( alloc.end(), queueAlloc_t::value_type
                    (it->first, it->second.allocation) );
}


/** set_queue_alloc() will set the "computational allocation" that each
 *  active queue for a specific solver gets.  The allocations are
 *  normalized into the range [0,1].
 *
 *  Nomenclature:
 *    - N = Number of active queues
 *    - M = Number of queues in the provided alloc map
 *
 *  Queue allocations are set using the following rules:
 *    - Any active solver not listed in the provided alloc map gets 1/N
 *    - Allocations provided in the alloc map will be normalized so that 
 *      they sum to M/N.
 *    - If there are any allocations < 0, then the smallest one is treated 
 *      as 0, and everything is (linearly) scaled up before normalization.
 */
void
LocalQueueManager::
set_subqueue_alloc(solverID_t solver_id, const queueAlloc_t &alloc)
{
   queueMap_t::iterator s_it = m_queues->find(solver_id);
   if (s_it == m_queues->end())
   {
      EXCEPTION_MNGR(runtime_error, "LocalQueueManager::set_subqueue_alloc():"
                     " specified solver_id does not exist.");
   }

   subqueueMap_t &subqueues = s_it->second.subqueues;

   // normalize the provided allocations
   queueAlloc_t::const_iterator new_it = alloc.begin();
   queueAlloc_t::const_iterator new_itEnd = alloc.end();
   double min = 0;
   double norm = 0;
   for (; new_it != new_itEnd; ++new_it)
   {
      if (min > new_it->second)
         min = new_it->second; 
      norm += new_it->second;
   }
   if (min < 0)
      norm -= min * alloc.size(); 
   if (( subqueues.size() > 0 ) && ( alloc.size() > 0 ))
   {
      norm *= ( static_cast<double>(subqueues.size()) / 
                static_cast<double>(alloc.size()) );
   }

   double default_alloc = subqueues.size();
   if (default_alloc > 0)
      default_alloc = 1.0 / default_alloc; 

   // update the actual allocations
   new_it = alloc.begin();
   subqueueMap_t::iterator it = subqueues.begin();
   subqueueMap_t::iterator itEnd = subqueues.end();
   while (it != itEnd)
   {
      if ((new_it == new_itEnd) || (it->first < new_it->first))
      {
         it->second.allocation = default_alloc;
         ++it;
         continue;
      }
      if (it->first == new_it->first)
      {
         if ( norm == 0 )
            it->second.allocation = 0; 
         else
            it->second.allocation = (new_it->second - min) / norm; 

         ++it;
         ++new_it;
         continue;
      }
      if (it->first > new_it->first)
      {
         EXCEPTION_MNGR(std::runtime_error, "LocalQueueManager::"
                        "set_subqueue_alloc(): specified new allocation for "
                        "unknown queue ID.");
      }
   }
   if (new_it != new_itEnd)
   {
      EXCEPTION_MNGR(std::runtime_error, "LocalQueueManager::"
                     "set_subqueue_alloc(): specified new allocation for "
                     "unknown queue ID.");
   }

   // Allow derived classes to know the queue allocations changes
   new_queue_alloc(s_it);
}


//----------------------------------------------------------------------
// Evaluation queuing and management

bool
LocalQueueManager::
queue_empty( solverID_t solver_id, queueID_t queue_id ) const
{
   if ( solver_id == ALL_SOLVERS )
   {
      queueMap_t::const_iterator s_it = m_queues->begin();
      for ( ; s_it != m_queues->end(); ++s_it )
         if ( ! queue_empty_helper(s_it, queue_id) )
            return false;
      
      return true;
   }

   queueMap_t::const_iterator s_it = m_queues->find(solver_id);
   if ( s_it == m_queues->end() )
      return true;
   else
      return queue_empty_helper(s_it, queue_id);
}

inline bool
LocalQueueManager::
queue_empty_helper(queueMap_t::const_iterator s_it, queueID_t queue_id) const
{
   subqueueMap_t::const_iterator q_itEnd = s_it->second.subqueues.end();
   if ( queue_id == ALL_SUBQUEUES )
   {
      subqueueMap_t::const_iterator q_it = s_it->second.subqueues.begin();
      for ( ; q_it != q_itEnd; ++q_it )
         if ( ! q_it->second.pqueues.empty() )
            return false;

      return true;
   }

   subqueueMap_t::const_iterator q_it = s_it->second.subqueues.find(queue_id);
   return q_it == q_itEnd ? true : q_it->second.pqueues.empty();
}


} // namespace colin
