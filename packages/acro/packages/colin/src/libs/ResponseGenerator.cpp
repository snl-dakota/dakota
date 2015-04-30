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

#include <colin/ResponseGenerator.h>
#include <colin/AppRequest_Impl.h>
#include <colin/AppResponse_Impl.h>

namespace colin {

ResponseGenerator::CoreRequestInfo
ResponseGenerator::get_core_request(AppRequest& request) 
{
   // At this point, no more changes to the request will be honored
   request.finalize_request();

   CoreRequestInfo ans;
   ans.seed = request.data->seed;
   ans.app = request.data->transform_path.back().app;
   ans.domain = request.data->transform_path.back().domain;
   ans.requests = request.data->transform_path.back().requests;
   
   return ans;
}


ResponseGenerator::CoreResponseInfo
ResponseGenerator::get_core_response(const AppResponse& response) 
{
   CoreResponseInfo ans;
   ans.seed = response.data->seed;
   ans.domain = response.data->transform_path.back().domain;
   ans.responses = response.data->raw_responses;
  
   return ans;
}


AppResponse
ResponseGenerator::form_response( const AppRequest& request,
                                  CoreResponseInfo cri ) 
{
   AppResponse ans = AppResponse(request, cri.responses, cri.domain);
   if ( cri.seed != 0 )
   {
      if ( ans.data->seed == 0 )
         ans.data->seed = cri.seed;
      else if ( ans.data->seed != cri.seed )
         EXCEPTION_MNGR(std::runtime_error, "ResponseGenerator::"
                        "form_response(): Seed returned by "
                        "Application_Base::collect_response() does not "
                        "match seed in request.");
   }

   if ( request.data->response != NULL )
      *(request.data->response) = ans;
   return ans;
}

AppResponse
ResponseGenerator::form_response( const Application_Base* context,
                                  CoreResponseInfo cri ) 
{
   AppRequest req = context->set_domain(utilib::Any());
   return form_response(req, cri);
}

const Application_Base* 
ResponseGenerator::get_core_application(const AppResponse& response) 
{
   return response.data->transform_path.back().app;
}


const Application_Base* 
ResponseGenerator::get_core_application(const Application_Base* app) 
{
   if ( app == NULL )
      return app;

   Application_Base* tmp = app->get_reformulated_application();
   while ( tmp != NULL )
   {
      app = tmp;
      tmp = app->get_reformulated_application();
   }
   return app;
}




} // namespace colin
