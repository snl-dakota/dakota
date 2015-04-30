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

#include <colin/eval/ConcurrentEvaluator.h>
#include <colin/application/Base.h>
#include <colin/Cache.h>
#include <colin/cache/Factory.h>

#ifdef _WIN32
#include <windows.h> // for Sleep()
#else
#include <unistd.h> // for sleep/usleep()
#endif

using std::map;
using std::pair;
using std::make_pair;
using std::set;
using std::list;
using std::cerr;
using std::endl;

using utilib::Any;
using utilib::seed_t;

namespace colin {

namespace StaticInitializers {

namespace {

bool RegisterConcurrentEvaluator()
{
   EvalManagerFactory().register_manager( "Concurrent", 
                                          &ConcurrentEvaluator::create );
   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool concurrent_evaluator = RegisterConcurrentEvaluator();

} // namespace colin::StaticInitializers



//------------------------------------------------------------------------
//

ConcurrentEvaluator::ConcurrentEvaluator(evalMngrID_t id)
   : EvaluationManager_Base(id, queue_mngr),
     max_concurrency(utilib::Property::Bind<size_t>()),
     queue_mngr(),
     queue_bypass_allowed(true),
     current_concurrency(0),
     polling_interval(1)
{
   // we will use the system default cache manager.
   set_evaluation_cache(CacheFactory().evaluation_cache());

   properties.declare("max_concurrency", max_concurrency);

   max_concurrency = 1;
}


AppResponse
ConcurrentEvaluator::
perform_evaluation(solverID_t solver_id, AppRequest request)
{
   static_cast<void>(solver_id);

   CoreRequestInfo cqi = get_core_request(request);
   AppResponse::response_map_t cache_hits;
   Cache::Key cache_key
      = request_check_cache(request.forced_recalc(), cqi, cache_hits);

   // If we still need to spawn the evaluation, do it...
   CoreResponseInfo cri;
   if ( cqi.requests.empty() )
   {
      cri.seed = cqi.seed;
      cri.responses = cache_hits;
   }
   else
   {
      while ( max_concurrency <= current_concurrency )
         spin_lock();

      cri = perform_core_evaluation(cqi);

      // Store any new responses into the cache
      if ( ! eval_cache.empty() )
         eval_cache->insert( cqi.app, cache_key, cri );

      // add any responses from the cache into what we got back from the
      // application.  If the application returned data, too, then this
      // will defer to the new data from the application, and we well
      // let the cache sort out the collision
      while ( ! cache_hits.empty() )
      {
         cri.responses.insert(*cache_hits.begin());
         cache_hits.erase(cache_hits.begin());
      }
   }

   refill_concurrent_processes(solver_id);

   return form_response(request, cri);
}


EvaluationID
ConcurrentEvaluator::queue_evaluation(solverID_t solver_id,
                                  AppRequest request,
                                  evalPriority_t priority,
                                  queueID_t queue_id)
{
   // At this point, no more changes to the request will be honored
   request.finalize_request();

   // Check to see if we already have enough info to create the response
   // (i.e. bypass the actual application function evaluation)
   if (( queue_bypass_allowed ) && ( ! request.forced_recalc() ) && 
       ( ! eval_cache.empty() ))
   {
      CoreRequestInfo cqi = get_core_request(request);
      CoreResponseInfo cri;
      request_check_cache(request.forced_recalc(), cqi, cri.responses);

      if ( cqi.requests.empty() )
      {
         // we found all the data we need
         EvaluationID evalID = EvaluationID(mngr_id, solver_id, queue_id);
         responseBuffer[solver_id].push_back
            ( ResponseRecord(evalID, form_response(request, cri)) );
         return evalID;
      }
   }

   EvaluationID eval_id = queue_mngr.queue_evaluation
      (request, mngr_id, solver_id, queue_id, priority);

   refill_concurrent_processes(solver_id);

   return eval_id;
}


std::pair<EvaluationID, AppResponse>
ConcurrentEvaluator::next_response(solverID_t solver_id, queueID_t queue_id)
{
   // pre-emptively check the current processes and spawn things if
   // something is available.
   refill_concurrent_processes(solver_id);

   do {
      // has a response already been computed?
      responseBuffer_t::iterator r_it = responseBuffer.find(solver_id);
      if (( r_it != responseBuffer.end() ) && ( ! r_it->second.empty() ))
      {
         responseBuffer_t::value_type::second_type::iterator 
            it = r_it->second.begin();
         responseBuffer_t::value_type::second_type::iterator 
            itEnd = r_it->second.end();
         
         if ( queue_id != QueueManager::ALL_SUBQUEUES )
            for ( ; it != itEnd; ++it )
               if ( queue_id == it->first.queue() )
                  break;
         
         if ( it != itEnd )
         {
            ResponseRecord ans = *it;
            r_it->second.erase(it);
            
            if ( r_it->second.empty() )
               responseBuffer.erase(r_it);
            
            return ans;
         }
      }

      // If there is nothing appropriate already executing, AND there is
      // nothing appropriate *to* execute, then return
      if ( ! pending_evaluation_for_queue(solver_id, queue_id) )
         if ( queue_mngr.queue_empty(solver_id, queue_id) )
            return ResponseRecord(EvaluationID(), AppResponse());

      // At this point, either the queue is empty, OR we are already at
      // max_concurrency: either way, we need to block (spin-lock) for
      // something to return.
      spin_lock();
      
      // If what we want is already executing, use the normal refill
      // process; otherwise, refil based on the queue we are blocking
      // for.
      if ( pending_evaluation_for_queue(solver_id, queue_id) )
         refill_concurrent_processes(solver_id);
      else
         refill_concurrent_processes(solver_id, queue_id);

   } while ( true );
}


bool
ConcurrentEvaluator::
response_available(solverID_t solver_id, queueID_t queue_id)
{
   // pre-emptively check the current processes and spawn things if
   // something is available.
   refill_concurrent_processes(solver_id);

   // has a response already been computed?
   responseBuffer_t::iterator r_it = responseBuffer.find(solver_id);
   if (( r_it != responseBuffer.end() ) && ( ! r_it->second.empty() ))
   {
      responseBuffer_t::value_type::second_type::iterator 
         it = r_it->second.begin();
      responseBuffer_t::value_type::second_type::iterator 
         itEnd = r_it->second.end();

      if ( queue_id != QueueManager::ALL_SUBQUEUES )
         for ( ; it != itEnd; ++it )
            if ( queue_id == it->first.queue() )
               break;

      if ( it != itEnd )
         return true;
   }

   return false;
}


/** If queue_id == ALL_SUBQUEUES, all queues are executed; otherwise,
 *  only the specified queue is executed.
 */
void
ConcurrentEvaluator::
synchronize( solverID_t solver_id, queueID_t queue_id, bool keep_responses )
{
   // Spin-lock until synchronize is complete
   while ( pending_evaluation_for_queue(solver_id, queue_id) ||
           ! queue_mngr.queue_empty(solver_id, queue_id) )
   {
      spin_lock();
      refill_concurrent_processes(solver_id, queue_id);
   }

   if ( keep_responses ) 
      return;

   // clear out the response buffer because the user declared they
   // didn't want any more responses from this queue returned through
   // next_response().

   if ( queue_id == QueueManager::ALL_SUBQUEUES )
      responseBuffer[solver_id].clear();
   else
   {
      std::list<ResponseRecord> &buf = responseBuffer[solver_id];
      std::list<ResponseRecord>::iterator it = buf.begin();
      std::list<ResponseRecord>::iterator tmp;
      while ( it != buf.end() )
      {
         if ( it->first.queue() == queue_id )
         {
            tmp = it++;
            buf.erase(tmp);
         }
         else
            ++it;
      }
   }
}


void
ConcurrentEvaluator::
clear_evaluations(solverID_t solver_id, queueID_t queue_id)
{ 
   // clear out the queued evaluations
   queue_mngr.clear_evaluations(solver_id, queue_id); 

   // Now, go through the evals that are already running and mark them
   // as no longer needed
   pendingEvals_t::iterator it = pendingEvals.begin();
   pendingEvals_t::iterator itEnd = pendingEvals.end();
   while( it != itEnd )
   {
      if (( it->second.eval_id.solver() == solver_id ) && 
          ( queue_id == QueueManager::ALL_SUBQUEUES || 
            queue_id == it->second.eval_id.queue() ))
      {
         pendingEvals_t::iterator tmp = it++;
         clearedEvals.insert(tmp->first);
         pendingEvals.erase(tmp);
      }
      else
         ++it;
   }
}


/** This provides an "adaptive" spin-lock for waiting for the next
 *  available evaluation.  It starts off with a VERY short polling
 *  interval (1 usec) that grows exponentially until it reaches the
 *  polling_interval.  This allows the spin-locks to return quickly for
 *  fast function evaluations, but not abuse the processor by wasting
 *  cycles continuously checking for completed responses.
 */
void 
ConcurrentEvaluator::
spin_lock()
{
   size_t sleep_time = 1;
   while ( true )
   {
      if ( collect_concurrent_processes() )
         return;

#ifdef _WIN32
      // expects msec; with truncation won't sleep for first 9 loop iterations
      Sleep(sleep_time / 1e3);
#else
      usleep(sleep_time);  // expects usec
#endif
      sleep_time *= 2;
      if ( sleep_time > polling_interval * 1e6 )
         break;
   }
   while ( 0 == collect_concurrent_processes() ) 
   {
#ifdef _WIN32
     Sleep(polling_interval * 1e3);  // expects msec
#else
     sleep(polling_interval);  // expects usec
#endif
   }
}


size_t
ConcurrentEvaluator::
collect_concurrent_processes()
{
   size_t count = 0;

   runningApps_t::iterator a_it = runningApps.begin();
   runningApps_t::iterator a_itEnd = runningApps.end();
   for ( ; a_it != a_itEnd; ++a_it )
      while ( core_response_available(a_it->first) )
      {
         CoreResponseInfo cri = collect_core_response(a_it->first);
         ++count;
         --current_concurrency;
         --(a_it->second);

         pendingEvals_t::iterator p_it = pendingEvals.find(cri.app_eval_id);
         if ( p_it == pendingEvals.end() )
         {
            if ( clearedEvals.erase(cri.app_eval_id) )
               continue;
            EXCEPTION_MNGR(std::runtime_error, "Application returned an "
                           "eval_id that was not recorded as being spawned");
         }

         PendingEvaluation &pe = p_it->second;

         // Cache the new data
         if ( ! eval_cache.empty() )
            eval_cache->insert( pe.app, pe.cache_key, cri );

         // add any responses from the cache into what we got back from the
         // application.  If the application returned data, too, then this
         // will defer to the new data from the application, and we will
         // let the cache sort out the collision
         while ( ! pe.cache_hits.empty() )
         {
            cri.responses.insert(*pe.cache_hits.begin());
            pe.cache_hits.erase(pe.cache_hits.begin());
         }


         AppResponse response = form_response(pe.request, cri);
         responseBuffer[pe.eval_id.solver()].push_back
            ( make_pair(pe.eval_id, response) );
         pendingEvals.erase(p_it);
      }

   return count;
}


size_t 
ConcurrentEvaluator::
refill_concurrent_processes( solverID_t solver, queueID_t queue )
{
   size_t collectedEvals = 0;
   if ( max_concurrency <= current_concurrency )
      collectedEvals = collect_concurrent_processes();

   while ( max_concurrency > current_concurrency )
   {
      PendingEvaluation pe;
      pe.eval_id = queue_mngr.get_next_request(pe.request, solver, queue);
      if ( pe.eval_id.empty() )
         return collectedEvals;
      
      CoreRequestInfo cqi = get_core_request(pe.request);
      pe.app = cqi.app;
      pe.domain = cqi.domain;

      pe.cache_key
         = request_check_cache(pe.request.forced_recalc(), cqi, pe.cache_hits);

      // If we still need to spawn the evaluation, do it...
      if ( cqi.requests.empty() )
      {
         CoreResponseInfo cri;
         cri.seed = cqi.seed;
         cri.responses = pe.cache_hits;
         responseBuffer[solver].push_back
            ( make_pair(pe.eval_id, form_response(pe.request, cri)) );
      }
      else
      {
         Any app_eval_id = spawn_core_request(cqi);
         ++current_concurrency;

         pendingEvals.insert(pendingEvals.end(), make_pair(app_eval_id, pe));
         ++runningApps[pe.app];
      }
   }

   return collectedEvals;
}


bool
ConcurrentEvaluator::
pending_evaluation_for_queue(solverID_t solver, queueID_t queue) const
{
   pendingEvals_t::const_iterator p_it = pendingEvals.begin();
   pendingEvals_t::const_iterator p_itEnd = pendingEvals.end();
   for(; p_it != p_itEnd; ++p_it)
      if ( solver == QueueManager::ALL_SOLVERS 
           || solver == p_it->second.eval_id.solver() )
         if ( queue == QueueManager::ALL_SUBQUEUES 
              || queue == p_it->second.eval_id.queue() )
            return true;
   return false;
}

} // namespace colin
