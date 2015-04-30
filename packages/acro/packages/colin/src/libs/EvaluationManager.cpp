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

#include <colin/EvaluationManager.h>
#include <colin/Cache.h>
#include <colin/application/Base.h>

using std::map;
using std::set;
using std::pair;
using std::list;
using std::cerr;
using std::endl;

using utilib::Any;
using utilib::seed_t;

namespace colin
{


EvaluationManager_Base::~EvaluationManager_Base()
{
   if (! solver_refCount.empty())
   {
      std::ostringstream buf;
      buf << "EvaluationManager_Base deleted before all EvaluationManagers ";
      buf << "(solver_id = " << solver_refCount.begin()->first;
      solver_refCount.erase(solver_refCount.begin());
      while (! solver_refCount.empty())
      {
         buf << ", " << solver_refCount.begin()->first;
         solver_refCount.erase(solver_refCount.begin());
      }
      buf << ")";
      EXCEPTION_MNGR(std::runtime_error, buf.str());
   }
}


void
EvaluationManager_Base::set_evaluation_cache(CacheHandle new_cache)
{
   eval_cache = new_cache;
}


utilib::Property& 
EvaluationManager_Base::property( std::string name )
{
   utilib::PropertyDict::iterator it = properties.find(name);
   if ( it == properties.end() )
      EXCEPTION_MNGR(std::runtime_error, "EvaluationManager_Base::property(): "
                     "Attempt to retrieve nonexistent property, '"
                     << name << "'");

   return it->second();
}


EvaluationManager_Base::solverID_t
EvaluationManager_Base::reference_solver(solverID_t existing)
{
   map<solverID_t, size_t>::iterator it;
   if (existing != 0)
   {
      it = solver_refCount.find(existing);
      if (it == solver_refCount.end())
      {
         EXCEPTION_MNGR(std::runtime_error, "EvaluationManager_Base::"
                        "get_new_solver_id(): invalid existing solverID.");
      }
   }
   else
   {
      it = solver_refCount.insert
         ( solver_refCount.end(), pair<solverID_t, size_t>
           ( queue_mngr.get_new_solver_queue(), 0 ) );
   }

   ++(it->second);
   return it->first;
}


void
EvaluationManager_Base::release_solver(solverID_t solver_id)
{
   map<solverID_t, size_t>::iterator it = solver_refCount.find(solver_id);
   if (it == solver_refCount.end())
   {
      EXCEPTION_MNGR(std::runtime_error, "EvaluationManager_Base::"
                     "release_solver(): invalid existing solverID.");
   }
   if ( --(it->second) == 0 )
   {
      queue_mngr.release_solver_queue(solver_id);
      solver_refCount.erase(it);
   }
}


utilib::Any
EvaluationManager_Base::spawn_core_request(CoreRequestInfo &cqi) const
{
   return cqi.app->spawn_evaluation(cqi.domain, cqi.requests, cqi.seed);
}


EvaluationManager_Base::CoreResponseInfo
EvaluationManager_Base::collect_core_response( Application_Base *app ) const
{
   CoreResponseInfo cri;
   cri.app_eval_id = app->collect_evaluation( cri.responses, cri.seed );
   return cri;
}


EvaluationManager_Base::CoreResponseInfo
EvaluationManager_Base::perform_core_evaluation( CoreRequestInfo &cqi ) const
{
   CoreResponseInfo cri;
   cri.seed = cqi.seed;
   cri.domain = cqi.domain;
   cqi.app->perform_evaluation( cqi.domain, cqi.requests, cri.seed,
                                cri.responses );
   return cri;
}

bool
EvaluationManager_Base::core_response_available( Application_Base *app ) const
{
   return app->evaluation_available();
}

Cache::Key 
EvaluationManager_Base::
request_check_cache( bool force_recalc, 
                     CoreRequestInfo &cqi, 
                     AppResponse::response_map_t &cache_hits ) const
{
   if ( eval_cache.empty() )
      return Cache::Key();

   pair<Cache::iterator, Cache::Key> cacheCheck =
      eval_cache->find(cqi);
   Cache::iterator &cache_it = cacheCheck.first;
   Cache::iterator cache_itEnd = eval_cache->end();
   Cache::Key &key = cacheCheck.second;

   if ( cache_it == cache_itEnd || force_recalc )
      return key;

   AppResponse::response_map_t::const_iterator r_it;
   AppResponse::response_map_t::const_iterator r_itEnd;
   AppRequest::request_map_t::iterator q_it = cqi.requests.begin();
   AppRequest::request_map_t::iterator q_itEnd = cqi.requests.end();
   Cache::iterator best_match = cache_itEnd;
   pair<size_t, size_t> best_match_count = std::make_pair(0,0);
   while ( cache_it != cache_itEnd && cache_it->first.key == key )
   {
      // Find the best match to this request from the list of responses
      r_it = cache_it->second.responses.begin();
      r_itEnd = cache_it->second.responses.end();
      q_it = cqi.requests.begin();
      pair<size_t, size_t> match_count = std::make_pair(0,0);
      while(( r_it != r_itEnd ) && ( q_it != q_itEnd ))
      {
         if ( r_it->first < q_it->first )
         {
            // cache has data that the request didn't ask for
            ++match_count.second;
            ++r_it;
         }
         else if ( r_it->first == q_it->first )
         {
            // A cache hit
            ++match_count.first;
            ++q_it;
            ++r_it;
         }
         else
         {
            // A cache miss
            ++q_it;
         }
      }
      if ( match_count > best_match_count )
      {
         best_match = cache_it;
         best_match_count = match_count;
      }
      ++cache_it;
   }

   if ( best_match == cache_itEnd )
      return key;
   cache_hits = best_match->second.responses;

   // Filter data from the core request that is already present in the
   // cached response.
   r_it = cache_hits.begin();
   r_itEnd = cache_hits.end();
   q_it = cqi.requests.begin();
   while(( r_it != r_itEnd ) && ( q_it != q_itEnd ))
   {
      if ( r_it->first < q_it->first )
      {
         // cache has data that the request didn't ask for
         ++r_it;
}
      else if ( r_it->first == q_it->first )
      {
         // A cache hit - remove from request
         AppRequest::request_map_t::iterator tmp = q_it;
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

   return cacheCheck.second;
}

//==========================================================================
// EvaluationManager methods
//==========================================================================

/** Flush queue and return first improving point (SO only).
 *  This will execute evaluations in the specified queue until the first
 *  improving response is found, returning that response.  The remainder
 *  of the requests will be cleared without executing.
 *
 *  If queue_id == 0 (default), then all requests on all queues will be
 *  executed.
 *
 *  Only works for single-objective problems.
 */
EvaluationID EvaluationManager::find_first_improving(AppResponse& ans,
      utilib::AnyRef target,
      optimizationSense sense,
      queueID_t queue_id) const
{
   std::map<queueID_t, double> old_alloc;
   if (queue_id != 0)
   {
      // temporarily reset the queue priority so that we get the responses
      // we want back asap.
      get_queue_alloc(old_alloc);
      std::map<queueID_t, double> tmp_alloc = old_alloc;
      std::map<queueID_t, double>::iterator it = tmp_alloc.begin();
      std::map<queueID_t, double>::iterator itEnd = tmp_alloc.end();
      for (; it != itEnd; ++it)
      { it->second = (it->first == queue_id ? 1 : 0); }
      set_queue_alloc(tmp_alloc);
   }

   // get the values...
   EvaluationID bestID;
   pair<EvaluationID, AppResponse> eval;
   Any test;
   Any* t1 = ( sense == minimization ? &test : &target );
   Any* t2 = ( sense == minimization ? &target : &test );
   while ( ! (eval = next_response(queue_id)).first.empty() )
   {
      if ( eval.second.is_computed(f_info) )
      {
         utilib::TypeManager()->lexical_cast( eval.second.get(f_info), test, 
                                              target.type() );
         if ( *t1 < *t2 )
         {
            ans = eval.second;
            bestID = eval.first;
            break;
         }
      }
   }

   clear_evaluations(queue_id);

   // reset the queue allocation
   if (queue_id != 0)
   { set_queue_alloc(old_alloc); }

   return bestID;
}


/** Flush queue and return best point (SO only).  This will
 *  execute ALL evaluations in the specified queue and return the best
 *  improving response found (AppResponse == false if no improving
 *  found).
 *
 *  If queue_id == 0 (default), then all requests on all queues will be
 *  executed.
 *
 *  Only works for single-objective problems.
 */
EvaluationID EvaluationManager::find_best_improving(AppResponse& ans,
      utilib::AnyRef target,
      optimizationSense sense,
      queueID_t queue_id) const
{
   std::map<queueID_t, double> old_alloc;
   if (queue_id != 0)
   {
      // temporarily reset the queue priority so that we get the responses
      // we want back asap.
      get_queue_alloc(old_alloc);
      std::map<queueID_t, double> tmp_alloc = old_alloc;
      std::map<queueID_t, double>::iterator it = tmp_alloc.begin();
      std::map<queueID_t, double>::iterator itEnd = tmp_alloc.end();
      for (; it != itEnd; ++it)
      { it->second = (it->first == queue_id ? 1 : 0); }
      set_queue_alloc(tmp_alloc);
   }

   Any best = target;
   if ( best.is_immutable() )
      best.to_value_any();
   Any test;
   Any* t1 = ( sense == minimization ? &test : &best );
   Any* t2 = ( sense == minimization ? &best : &test );

   // get the values...
   EvaluationID bestID;
   pair<EvaluationID, AppResponse> eval;
   while ( ! (eval = next_response(queue_id)).first.empty())
   {
      if ( eval.second.is_computed(f_info) )
      {
         utilib::TypeManager()->lexical_cast( eval.second.get(f_info), test, 
                                              best.type() );
         if ( *t1 < *t2 )
         {
            ans = eval.second;
            best = test;
            bestID = eval.first;
         }
      }
   }

   // reset the queue allocation
   if (queue_id != 0)
   { set_queue_alloc(old_alloc); }

   return bestID;
}


} // namespace colin
