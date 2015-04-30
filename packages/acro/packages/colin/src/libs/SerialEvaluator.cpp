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

#include <colin/eval/SerialEvaluator.h>
#include <colin/application/Base.h>
#include <colin/Cache.h>
#include <colin/cache/Factory.h>

using std::map;
using std::pair;
using std::set;
using std::list;
using std::cerr;
using std::endl;

using utilib::Any;
using utilib::seed_t;

namespace colin {

namespace StaticInitializers {

namespace {

bool RegisterSerialEvaluator()
{
   EvalManagerFactory().register_manager("Serial", &SerialEvaluator::create);
   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool serial_evaluator = RegisterSerialEvaluator();

} // namespace colin::StaticInitializers



//------------------------------------------------------------------------
//

SerialEvaluator::SerialEvaluator(evalMngrID_t id)
   : EvaluationManager_Base(id, queue_mngr),
     queue_mngr(),
     queue_bypass_allowed(true)
{
   // we will use the system default cache manager.
   set_evaluation_cache(CacheFactory().evaluation_cache());
}


AppResponse
SerialEvaluator::perform_evaluation(solverID_t solver_id, AppRequest request)
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
   return form_response(request, cri);
}


EvaluationID
SerialEvaluator::queue_evaluation(solverID_t solver_id,
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

   return queue_mngr.queue_evaluation
      (request, mngr_id, solver_id, queue_id, priority);
}


std::pair<EvaluationID, AppResponse>
SerialEvaluator::next_response(solverID_t solver_id, queueID_t queue_id)
{
   EvaluationID evalID = EvaluationID();

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

   // Get the next request to compute
   AppRequest request;
   evalID = queue_mngr.get_next_request(request, solver_id, queue_id);

   // If there is nothing queued, return an empty eval id
   if ( evalID.empty() )
      return ResponseRecord(evalID, AppResponse());
   else
      return ResponseRecord(evalID, perform_evaluation(solver_id, request));
}


bool
SerialEvaluator::response_available(solverID_t solver_id, queueID_t queue_id)
{
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

   // Get the next request to compute
   return ! queue_mngr.queue_empty(solver_id, queue_id);
 
}


/** If queue_id == ALL_SUBQUEUES, all queues are executed; otherwise,
 *  only the specified queue is executed.
 */
void
SerialEvaluator::
synchronize( solverID_t solver_id, queueID_t queue_id, bool keep_responses )
{
   list<ResponseRecord> &responseList = responseBuffer[solver_id];

   AppRequest request;
   EvaluationID evalID 
      = queue_mngr.get_next_request(request, solver_id, queue_id);
   while ( ! evalID.empty() )
   {
      AppResponse response = perform_evaluation(solver_id, request);
      if ( keep_responses )
         responseList.push_back(ResponseRecord(evalID, response));
      evalID = queue_mngr.get_next_request(request, solver_id, queue_id);
   }
}

} // namespace colin
