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
 * \file AppResponse_Impl.h
 *
 * Defines the colin::AppResponse_Impl class.
 */

#ifndef colin_AppResponse_Impl_h
#define colin_AppResponse_Impl_h

#include <colin/AppResponse.h>

#include <utilib/RNG.h>


namespace colin
{

struct AppResponse::ApplicationInfo {

   const Application_Base*  app;
   
   utilib::Any     domain;

   response_map_t  responses;
};


/**
 * Defines the implementation data for an AppResponse object.
 *
 * Normally, I would put this in the cpp file; however, because
 * Application_Base and EvaluationManager_Base are friends of
 * AppResponse, they need to be able to import the "full" definition of
 * the AppResponse object.
 */
struct AppResponse::Implementation
{
   /// Constructor
   Implementation( const app_request_id_t &_request_id,
                   const utilib::seed_t &_seed,
                   const AppResponse::response_map_t &_raw_responses )
      : refCount(1),
        request_id(_request_id),
        seed(_seed),
        raw_responses(_raw_responses)
   {}

   typedef std::list<ApplicationInfo> xform_path_t;


   /// Number of AppResponse objects that refer to this Data object
   size_t  refCount;

   /// A "unique" id for this response
   const app_request_id_t request_id;

   /// The seed the Application used for this response
   utilib::seed_t  seed;

   /// The map of raw computed responses
   response_map_t  raw_responses;

   xform_path_t transform_path;


   xform_path_t::const_iterator 
   getAppIterator(const Application_Base* app) const;
};

}

#endif // defined colin_AppResponse_Impl_h
