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

#include<colin/AsynchronousApplication.h>

using utilib::Any;
using utilib::seed_t;

using std::list;

namespace colin {

void 
AsynchronousApplication::
perform_evaluation_impl( const utilib::Any &domain,
                         const AppRequest::request_map_t &requests,
                         utilib::seed_t &seed,
                         AppResponse::response_map_t &responses )
{
   Any evalID = spawn_evaluation(domain, requests, seed);

   while ( true )
   {
      AppResponse::response_map_t tmp_responses;
      utilib::seed_t tmp_seed;
      Any tmp_evalId;

      tmp_evalId = async_collect_evaluation(tmp_responses, tmp_seed);
      try {
         if ( evalID.references_same_data_as(tmp_evalId) ||
              evalID == tmp_evalId )
         {
            responses = tmp_responses;
            seed = tmp_seed;
            return;
         }
         else
         {
            evaluatedBuffer.push_back
               ( Evaluation(tmp_evalId, tmp_seed, tmp_responses) );
         }
      } catch ( utilib::any_not_comparable& ) {
         // silently ignore non-comparable EvalIDs
      }
   }
}


utilib::Any
AsynchronousApplication::
collect_evaluation_impl( AppResponse::response_map_t &responses,
                         utilib::seed_t &seed )
{
   if ( evaluatedBuffer.empty() )
      return async_collect_evaluation(responses, seed);
   else
   {
      responses = evaluatedBuffer.front().responses;
      seed = evaluatedBuffer.front().seed;
      Any evalID = evaluatedBuffer.front().evalID;
      evaluatedBuffer.pop_front();
      return evalID;
   }
}


} // namespace colin
