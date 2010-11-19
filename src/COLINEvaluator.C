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

#include "COLINEvaluator.H"
#include <colin/CachedResponseData.h>
#include <colin/ResponseCache.h>
#include <colin/ResponseCacheFactory.h>

using std::map;
using std::pair;
using std::set;
using std::list;
using std::cerr;
using std::endl;

using utilib::Any;
using utilib::seed_t;

using colin::EvalManagerFactory;

namespace Dakota {

namespace {

bool RegisterDakotaEvaluator()
{
  EvalManagerFactory().register_manager("Dakota", &COLINEvaluator::create);
  return true;
}

} // private namespace

extern const volatile bool dakota_evaluator_registered = 
   RegisterDakotaEvaluator();

//------------------------------------------------------------------------
//

COLINEvaluator::COLINEvaluator(evalMngrID_t id)
   : ConcurrentEvaluator(id)
{
}


#if 0

COLINEvaluator::COLINEvaluator(evalMngrID_t id)
   : EvaluationManager_Base(id, queue_mngr),
     queue_mngr(),
     queue_bypass_allowed(true)
{
   // we will use the system default cache manager.
  // (either COLIN or THIS class needs a cache?!?)
  set_evaluation_cache(colin::CacheFactory().create());
}





  /// blocking eval of the specified request: application immed eval, block for return
colin::AppResponse
COLINEvaluator::perform_evaluation(solverID_t solver_id, colin::AppRequest request)
{
  static_cast<void>(solver_id);

  CoreRequestInfo cqi = get_core_request(request);
  // see if request or fraction is in cache
  pair<colin::CachedResponseData, colin::ResponseCache::Key> cache_data 
    = eval_cache.empty() 
    ? pair<colin::CachedResponseData, colin::ResponseCache::Key>()
    : eval_cache->find(cqi.app, cqi.domain);
  // map of response info types to Responses:
  // type
  colin::AppResponse::response_map_t c_hits;

  // seed 0: no seed
  // If the seed is still 0, then we will interpret it as "any seed
  // will do" (just as we do when checking if we can bypass the
  // original evaluation queue); UNLESS there is already a "seed==0" in
  // the cache.
  if (( ! request.forced_recalc() ) && ( cqi.seed == 0 ) && 
      ( ! cache_data.first.has_seed( 0 ) ))
    {
      set<seed_t> seeds;
      // we will look for & use the "best match"
      if ( cache_data.first.get_seeds(cqi.requests, seeds, 1, true) )
	cqi.seed = *seeds.begin();
    }

  // Check to see if we already have any info for this response
  // (i.e. bypass/reduce the actual application function evaluation)

  // At this point, the seed is fixed.  If it is still 0, then the
  // cache had better contain info for a "0-seed" response (likely to
  // be true for deterministic applications).
  if (( !request.forced_recalc() ) && cache_data.first.get(cqi.seed, c_hits) )
    {
      // Check to see if the seed responses have anything we need
      colin::AppResponse::response_map_t::iterator r_it = c_hits.begin();
      colin::AppResponse::response_map_t::iterator r_itEnd = c_hits.end();
      colin::AppRequest::request_map_t::iterator q_it = cqi.requests.begin();
      colin::AppRequest::request_map_t::iterator q_itEnd = cqi.requests.end();
      while(( r_it != r_itEnd ) && ( q_it != q_itEnd ))
	{
	  if ( r_it->first < q_it->first )
	    {
	      // cache has data that the request didn't ask for
	      colin::AppResponse::response_map_t::iterator tmp = r_it;
	      ++r_it;
	      c_hits.erase(tmp);
	    }
	  else if ( r_it->first == q_it->first )
	    {
	      // A cache hit - remove from request
	      colin::AppRequest::request_map_t::iterator tmp = q_it;
	      ++q_it;
	      ++r_it;
	      cqi.requests.erase(tmp);
	    }
	  else
	    {
	      // A cache miss
	      ++q_it;
	    }
	}
    }

  // If we still need to spawn the evaluation, do it...
  CoreResponseInfo cri;
  if ( cqi.requests.empty() )
    {
      cri.seed = cqi.seed;
      cri.responses = c_hits;
    }
  else
    {
      Any appEvalID = spawn_core_request(cqi);
      cri = collect_core_response(cqi.app);

      // Store any new responses into the cache
      if ( ! eval_cache.empty() )
	{
	  eval_cache->insert_info( cqi.app, cache_data.second, 
				   cri.seed, cqi.domain, cri.responses );
	  /*
	    if ( cache_data.first.empty() )
            cache_data.first = CachedResponseData(cqi.domain);
	    cache_data.first.insert(cri.seed, cri.responses);
	    eval_cache->insert(cqi.app, cache_data.first, cache_data.second);
	  */
	}

      // add any responses from the cache into what we got back from the
      // application.  If the application returned data, too, then this
      // will defer to the new data from the application, and we well
      // let the cache sort out the collision
      while ( ! c_hits.empty() )
	{
	  cri.responses.insert(*c_hits.begin());
	  c_hits.erase(c_hits.begin());
	}
    }
  return form_response(request, cri);
}


  // check for complete or no work
  // insert 1 request into queue
  // collect completed, schedule any more needed (helper fn)
colin::EvaluationID
COLINEvaluator::queue_evaluation(solverID_t solver_id,
				 colin::AppRequest request,
				 evalPriority_t priority,
				 queueID_t queue_id)
{
  colin::EvaluationID evalID;

  // At this point, no more changes to the request will be honored
  request.finalize_request();

  // Check to see if we already have enough info to create the response
  // (i.e. bypass the actual application function evaluation)
  if (( queue_bypass_allowed ) && ( ! request.forced_recalc() ) && 
      ( ! eval_cache.empty() ))
    {
      CoreRequestInfo cqi = get_core_request(request);
      colin::CachedResponseData cd = eval_cache->find(cqi.app, cqi.domain).first;
      if (( cqi.seed == 0 ) && ( ! cd.has_seed( 0 ) ))
	{
	  // seed == 0 is interpreted as "any seed will do", so look for one.
	  set<seed_t> seeds;
	  if ( cd.get_seeds(cqi.requests, seeds, 1) )
            cqi.seed = *seeds.begin();
	}

      // At this point, the seed is fixed.  If it is still 0, then the
      // cache had better contain info for a "0-seed" response (likely
      // to be true for deterministic applications).
      colin::AppResponse::response_map_t tmp;
      if ( cd.get(cqi.seed, tmp) )
	{
	  CoreResponseInfo cri;
	  cri.seed = cqi.seed;

	  // Check to see if the seed responses are everything we need
	  colin::AppResponse::response_map_t::iterator r_it = tmp.begin();
	  colin::AppRequest::request_map_t::iterator q_it = cqi.requests.begin();
	  colin::AppRequest::request_map_t::iterator q_itEnd = cqi.requests.end();
	  while(( r_it != tmp.end() ) && ( q_it != q_itEnd ))
	    {
	      if ( r_it->first == q_it->first )
		{ 
		  cri.responses.insert(cri.responses.end(), *r_it);
		  ++r_it;
		  ++q_it;
		}
	      else if ( r_it->first < q_it->first )
		++r_it;
	      else
		break;
	    }
	  if ( q_it == q_itEnd )
	    {
	      // we found all the data we need
	      evalID = colin::EvaluationID(mngr_id, solver_id, queue_id);
	      responseBuffer[solver_id].push_back
		( ResponseRecord(evalID, form_response(request, cri)) );
	      return evalID;
	    }
	}
    }

  return queue_mngr.queue_evaluation
    (request, mngr_id, solver_id, queue_id, priority);

  // instead of above return, do something like this

  // re_fill: dakota via collect and spawn_evaluation

  // Get the next request to compute
  // colin::AppRequest request1;
  // evalID = queue_mngr.get_next_request(request1, solver_id, queue_id);

  // If there is nothing queued, return an empty eval id
  //   if ( evalID.empty() )
  //      return ResponseRecord(evalID, AppResponse());
  //   else
  //      return ResponseRecord(evalID, perform_evaluation(solver_id, request));
  // EvalMgr should call app.spawn

  // return evalID;

}



  // blocks for next unless buffer AND queue AND DAKOTA empty
  // blocks unless done all requests
  // could be solver_id and queue_id-specific (may not)
std::pair<colin::EvaluationID, colin::AppResponse>
COLINEvaluator::next_response(solverID_t solver_id, queueID_t queue_id)
{
  colin::EvaluationID evalID = colin::EvaluationID();

  // has a response already been computed?
  responseBuffer_t::iterator r_it = responseBuffer.find(solver_id);
  if (( r_it != responseBuffer.end() ) && ( ! r_it->second.empty() ))
    {
      responseBuffer_t::value_type::second_type::iterator 
	it = r_it->second.begin();
      responseBuffer_t::value_type::second_type::iterator 
	itEnd = r_it->second.end();

      if ( queue_id != colin::QueueManager::ALL_SUBQUEUES )
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

  // instead of the following, iteratively refill / check buffer until response to return

  // Get the next request to compute
  colin::AppRequest request;
  evalID = queue_mngr.get_next_request(request, solver_id, queue_id);

  // If there is nothing queued, return an empty eval id
  if ( evalID.empty() ) {
    return ResponseRecord(evalID, colin::AppResponse());
  }
  else {
    colin::AppResponse response = perform_evaluation(solver_id, request);
    //    return ResponseRecord(evalID, perform_evaluation(solver_id, request));
    return ResponseRecord(evalID, response);
  }
}


/** If queue_id == ALL_SUBQUEUES, all queues are executed; otherwise,
 *  only the specified queue is executed.
 */

// 
void
COLINEvaluator::
synchronize( solverID_t solver_id, queueID_t queue_id, bool keep_responses )
{
  list<ResponseRecord> &responseList = responseBuffer[solver_id];

  colin::AppRequest request;
  colin::EvaluationID evalID
    = queue_mngr.get_next_request(request, solver_id, queue_id);
  while ( ! evalID.empty() )
    {
     
      // flush queue: spin until queue empty AND all requests matching
      // solver_id and queue_id are complete

      colin::AppResponse response = perform_evaluation(solver_id, request);
      if ( keep_responses )
	responseList.push_back(ResponseRecord(evalID, response));
      evalID = queue_mngr.get_next_request(request, solver_id, queue_id);
    }
}

void
COLINEvaluator::clear_evaluations(unsigned int, unsigned int)
{
}

size_t
COLINEvaluator::num_evaluation_servers(unsigned int)
{
}

#endif // 0
} // namespace Dakota
