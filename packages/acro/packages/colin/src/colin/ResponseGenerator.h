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
 * \file ResponseGenerator.h
 *
 * Defines the colin::ResponseGenerator class.
 */

#ifndef colin_ResponseGenerator_h
#define colin_ResponseGenerator_h

#include <colin/AppResponse.h>
#include <colin/AppRequest.h>

namespace colin {

class Application_Base;


/** This is a base class for actually converting an AppRequest into an
 *  AppResponse object.
 */
class ResponseGenerator
{
public:

protected:
   struct CoreRequestInfo {
      CoreRequestInfo()
         : app(NULL), domain(), seed(0), requests()
      {}

      Application_Base*           app;
      utilib::Any                 domain;
      utilib::seed_t              seed;
      AppRequest::request_map_t   requests;
   };
   struct CoreResponseInfo {
      CoreResponseInfo()
         : domain(), seed(0), responses(), app_eval_id()
      {}
      CoreResponseInfo( utilib::Any domain_, 
                        AppResponse::response_map_t  responses_ )
         : domain(domain_), 
           seed(0), 
           responses(responses_), 
           app_eval_id()
      {}

      utilib::Any                  domain;
      utilib::seed_t               seed;
      AppResponse::response_map_t  responses;
      utilib::Any                  app_eval_id;
   };

   /// 
   static CoreRequestInfo get_core_request(AppRequest& request);

   /// 
   static CoreResponseInfo get_core_response(const AppResponse& response);

   /// 
   static AppResponse 
   form_response( const AppRequest& request, CoreResponseInfo response );
   
   /// 
   static AppResponse 
   form_response( const Application_Base* context, CoreResponseInfo response );
   
   /// 
   static const Application_Base* 
   get_core_application(const AppResponse& response);

   /// 
   static const Application_Base* 
   get_core_application(const Application_Base* app);

};


} // namespace colin
#endif // colin_ResponseGenerator_h
