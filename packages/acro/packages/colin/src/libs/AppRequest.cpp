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

#include <colin/AppRequest_Impl.h>
#include <colin/application/Base.h>

//#include <utilib/comments.h>

//#ifdef ACRO_USING_TINYXML
//#  include <colin/TinyXML.h>
//#endif

using utilib::seed_t;
using utilib::Any;

using std::list;
using std::make_pair;

namespace colin
{

//================================================================
// AppRequest::Implementation Member Definitions
//================================================================

app_request_id_t
AppRequest::Implementation::generate_id()
{
   static app_request_id_t last_id = 0;
   return ++last_id;
}


//========================================================================
// AppRequest public methods
//======================================================================

/// Create a new request that shares an existing AppResponse
AppRequest::AppRequest( utilib::Any domain, 
                        const bool recalculate,
                        const seed_t seed )
   : data(new Implementation(domain, recalculate, seed))
{}


/// Destructor
AppRequest::~AppRequest()
{
   if (( data != NULL ) && ( --(data->refCount) == 0 ))
      delete data; 
}



/// Operator equal
AppRequest& AppRequest::operator=(const AppRequest& rhs)
{
   if ( data == rhs.data )
      return *this;

   if (( data != NULL ) && ( --data->refCount == 0 ))
      delete data;

   data = rhs.data;

   if ( data != NULL )
      ++data->refCount;

   return *this;
}

/** Return the domain in the context of the specified Application, or
 *  the outermost application if the application is NULL
 */
utilib::Any 
AppRequest::domain(Application_Base* app) const
{
   if ( app )
   {
      std::list<ApplicationInfo>::const_iterator it 
         = data->transform_path.begin();
      std::list<ApplicationInfo>::const_iterator itEnd
         = data->transform_path.end();
      for ( ; it != itEnd; ++it )
         if ( it->app == app )
            break;

      if ( it == itEnd )
         EXCEPTION_MNGR(std::runtime_error, "AppRequest::domain(): Specified "
                        "application not found in reformulation chain");

      // OK: this is weird: if the domain assigned to the specified
      // application is the SAME Any as the domain provided by the user,
      // then we want to proceed farther into the cast chain until it is
      // either resolved to a "proper" domain, or else we reach the end
      // of the chain (otherwise we would be returning a "bogus" type
      // supplied by the user, and not a type that makes sense to the
      // application.
      while ( it != itEnd && 
              it->domain.references_same_data_as(data->raw_domain) )
         ++it;

      return it != itEnd ? it->domain : data->raw_domain;
   }
   else
      return data->transform_path.front().domain;
}

app_request_id_t
AppRequest::id() const
{
   return data == NULL ? 0 : data->request_id; 
}


utilib::seed_t
AppRequest::seed() const
{
   return data == NULL ? 0 : data->seed; 
}


size_t
AppRequest::size() const
{
   return data == NULL ? 0 : data->raw_requests.size(); 
}


const Application_Base* 
AppRequest::application() const
{ 
   if (( data == NULL ) || ( data->transform_path.empty() ))
      return NULL;
   else
      return data->transform_path.front().app; 
}


bool
AppRequest::forced_recalc() const
{
   return data == NULL ? false : data->recalculate;
}


bool
AppRequest::finalized() const
{
   return data == NULL ? false : data->finalized;
}


AppRequest
AppRequest::replicate(seed_t new_seed)
{
   if ( data == NULL )
      EXCEPTION_MNGR(std::runtime_error, "AppRequest::replicate(): "
                     "attempting to replicate an empty request");
   if ( data->finalized )
      EXCEPTION_MNGR(std::runtime_error, "AppRequest::"
                     "replicate(): request already finalized");

   AppRequest req = AppRequest(data->raw_domain, data->recalculate, new_seed);

   request_map_t &new_requests = req.data->raw_requests;
   request_map_t::iterator it = data->raw_requests.begin();
   request_map_t::iterator itEnd = data->raw_requests.end();
   for (; it != itEnd; ++it)
      new_requests.insert( new_requests.end(), make_pair( it->first, Any() ) );

   list<ApplicationInfo> &new_xform = req.data->transform_path;
   list<ApplicationInfo>::iterator x_it = data->transform_path.begin();
   list<ApplicationInfo>::iterator x_itEnd = data->transform_path.end();
   for ( ; x_it != x_itEnd; ++x_it )
      new_xform.push_back( ApplicationInfo(x_it->app, x_it->domain) );

   return req;
}


/** Finalize this AppRequest.  This actually transforms the requests
 *  that the solver registered through the application chain and 
 *  generates the core requests that are actually computed by the core 
 *  application.
 *
 *  Because of the finalized() flag, it is OK to call finalize_request
 *  more than once.
 */
void
AppRequest::finalize_request()
{
   // Silently skip multiple calls to finalize_request
   if ( data->finalized )
      return;

   list<ApplicationInfo>::iterator it = data->transform_path.begin();
   list<ApplicationInfo>::iterator itEnd = data->transform_path.end();
   
   AppRequest::request_map_t *req = &data->raw_requests;
   for ( ; it != itEnd; ++it )
   {
      // map the requests into the sub-application space
      it->app->map_request(*req, it->requests);
      // move into the sub-application
      req = &it->requests;
   }

   data->finalized = true;
}




//========================================================================
// AppRequest private methods
//======================================================================


AppRequest::ApplicationInfo& 
AppRequest::add_reformulation(const Application_Base* app)
{
   if ( data == NULL ) 
      EXCEPTION_MNGR(std::runtime_error, "AppRequest::"
                     "add_wrapped_application(): empty request");
   if ( data->finalized )
      EXCEPTION_MNGR(std::runtime_error, "AppRequest::"
                     "add_wrapped_application(): request already finalized");

   data->transform_path.push_back(ApplicationInfo(app));
   return *data->transform_path.rbegin();
}


} // namespace colin




