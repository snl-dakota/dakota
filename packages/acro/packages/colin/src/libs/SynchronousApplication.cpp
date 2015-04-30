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

#include<colin/SynchronousApplication.h>

using utilib::Any;
using utilib::seed_t;

using std::list;

namespace colin {

struct SynchronousApplication_impl::Data
{
   ///
   struct RequestInfo
   {
      RequestInfo(const Any& d, const AppRequest::request_map_t &r, seed_t &s)
         : domain(d),
           requests(r),
           seed(s)
      {}

      Any domain;
      AppRequest::request_map_t requests;
      seed_t seed;
   };

   list<Any> request_list;
};


SynchronousApplication_impl::SynchronousApplication_impl()
   : data(new Data)
{}


SynchronousApplication_impl::~SynchronousApplication_impl()
{
   delete data;
}


utilib::Any
SynchronousApplication_impl::
spawn_evaluation( const utilib::Any &domain,
                  const AppRequest::request_map_t &requests,
                  utilib::seed_t &seed )
{
   data->request_list.push_back(Data::RequestInfo(domain, requests, seed));
   return data->request_list.back();
}


utilib::Any
SynchronousApplication_impl::
collect_evaluation( AppResponse::response_map_t &responses,
                    utilib::seed_t &seed,
                    performEval_ptr performEval )
{
   if ( data->request_list.empty() )
      EXCEPTION_MNGR(std::runtime_error, "SynchronousApplication::"
                     "Implementation::collect_evaluation(): "
                     "no spawned requests to collect");

   Any eval_id = data->request_list.front();
   data->request_list.pop_front();
   const Data::RequestInfo& ri = eval_id.expose<Data::RequestInfo>();
   
   seed = ri.seed;
   performEval( ri.domain, ri.requests, seed, responses );
   return eval_id;
}


bool SynchronousApplication_impl::evaluation_available()
{
   return ! data->request_list.empty();
}


} // namespace colin
