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
 * \file AppRequest_Impl.h
 *
 * Defines the colin::AppRequest_Impl class.
 */

#ifndef colin_AppRequest_Impl_h
#define colin_AppRequest_Impl_h

#include <acro_config.h>
//#include <colin/AppRequest.h> // (through Application_Base.h)
#include <colin/application/Base.h>

namespace colin
{

struct AppRequest::ApplicationInfo {
   ApplicationInfo() 
      : app(NULL), domain(), requests()
   {}
   ApplicationInfo(const Application_Base* _app) 
      : app(const_cast<Application_Base*>(_app)), 
        domain(), 
        requests()
   {}
   ApplicationInfo(Application_Base* _app, utilib::Any _domain) 
      : app(_app), 
        domain(_domain), 
        requests()
   {}

   Application_Base*  app;
      
   utilib::Any  domain;

   request_map_t  requests;
};


/**
 * Defines the implementation data for an AppRequest object.
 *
 * Normally, I would put this in the cpp file; however, because
 * Application_Base, EvaluationManager_Base, and AppResponse are
 * friends of AppRequest, they need to be able to import the "full"
 * definition of the AppRequest object.
 */
struct AppRequest::Implementation 
{
   /// Constructor
   Implementation( const utilib::Any _domain,
                   const bool _recalc,
                   const utilib::seed_t _seed)
      : refCount(1),
        request_id(generate_id()),
        recalculate(_recalc),
        finalized(false),
        response(NULL),
        seed(_seed),
        raw_domain(_domain),
        raw_requests(),
        transform_path()
   {}

   /// Destructor
   ~Implementation()
   {}

   static app_request_id_t generate_id();



   /// Number of AppRequest objects that refer to this Data object
   size_t  refCount;

   const app_request_id_t request_id;

   /// Recalculate all requests?
   bool  recalculate;

   /// Is this finalized (i.e. has finalize_request been called?)
   bool  finalized;

   /// Pointer to a AppResponse to populate after the calculation is complete
   AppResponse *response;

   /// The seed the user would like the Application to use for this request
   utilib::seed_t  seed;

   utilib::Any  raw_domain;

   request_map_t  raw_requests;

   std::list<ApplicationInfo>  transform_path;
};

}

#endif // defined colin_AppRequest_Impl_h
