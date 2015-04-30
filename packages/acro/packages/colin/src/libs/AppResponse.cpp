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

#include <colin/AppResponse_Impl.h>
#include <colin/AppRequest_Impl.h>
#include <colin/application/Base.h>

#include <utilib/CommonIO.h>


using std::pair;
using std::make_pair;
using std::list;
using std::vector;
using std::set;

using std::cerr;
using std::endl;

using utilib::seed_t;
using utilib::Any;

namespace colin
{


//================================================================
//================================================================
// AppResponse::Implementation Public Member Definitions
//================================================================
//================================================================

inline AppResponse::Implementation::xform_path_t::const_iterator 
AppResponse::Implementation::getAppIterator(const Application_Base* app) 
const
{
   xform_path_t::const_iterator it = transform_path.begin();
   if ( app != NULL )
   {
      xform_path_t::const_iterator itEnd = transform_path.end();
      while(( it != itEnd ) && ( app != it->app ))
         ++it;

      if ( it == itEnd )
         EXCEPTION_MNGR(std::runtime_error, 
                        "AppResponse::Implementation::getAppIterator: "
                        "attempt to retrieve response application context "
                        "that is not in the AppResponse transformation path.");
   }
   return it;
}


//================================================================
//================================================================
// AppResponse Public Member Definitions
//================================================================
//================================================================


/// Destructor
AppResponse::~AppResponse()
{
   if ( data == NULL ) 
      return;
   if ( --data->refCount == 0 )
      delete data; 
}



/// Operator equal
AppResponse& AppResponse::operator=(const AppResponse& rhs)
{
   if ( data == rhs.data )
      return *this;

   if (( data != NULL ) && ( --data->refCount == 0 ))
      delete data; 

   data = rhs.data;
   if (data != NULL)
      ++data->refCount; 

   return *this;
}


app_request_id_t
AppResponse::id() const
{
   if (( data == NULL ) || ( data->transform_path.empty() ))
      EXCEPTION_MNGR(std::runtime_error, "AppResponse::id: attempt "
                     "to retrieve id from an unpopulated AppResponse.");

   return data->request_id; 
}


utilib::seed_t
AppResponse::seed() const
{
   if (( data == NULL ) || ( data->transform_path.empty() ))
      EXCEPTION_MNGR(std::runtime_error, "AppResponse::seed: attempt "
                     "to retrieve seed from an unpopulated AppResponse.");

   return data->seed; 
}


size_t
AppResponse::size(const Application_Base* app) const
{
   if (( data == NULL ) || ( data->transform_path.empty() ))
      EXCEPTION_MNGR(std::runtime_error, "AppResponse::size: attempt "
                     "to retrieve size from an unpopulated AppResponse.");

   return data->getAppIterator(app)->responses.size();
}



/// Get domain point
const utilib::Any
AppResponse::get_domain(const Application_Base* app) const
{
   if (( data == NULL ) || ( data->transform_path.empty() ))
      EXCEPTION_MNGR(std::runtime_error, "AppResponse::get_domain: attempt "
                     "to retrieve domain from an unpopulated AppResponse.");

   return data->getAppIterator(app)->domain; 
}


/// Get domain point
void
AppResponse::get_domain( utilib::AnyFixedRef point, 
                         const Application_Base* app ) const
{
   try
   { 
      utilib::TypeManager()->lexical_cast(get_domain(app), point); 
   }
   catch (utilib::bad_lexical_cast& err)
   {
      ucerr << "AppResponse::get_domain - Problem casting "
            << get_domain(app).type().name() << " to " << point.type().name()
            << endl;
      throw;
   }
}


/// True if this Response object contains a value for Response Info type
bool
AppResponse::is_computed( response_info_t id, 
                          const Application_Base* app ) const
{
   if (( data == NULL ) || ( data->transform_path.empty() ))
      EXCEPTION_MNGR(std::runtime_error, "AppResponse::is_computed: attempt "
                     "to interrogate an unpopulated AppResponse.");

   Implementation::xform_path_t::const_iterator it = data->getAppIterator(app);

   if ( it->responses.find(id) != it->responses.end() )
      return true;

   // The response isn't present.  It may be that this AppResponse was
   // pulled from the cache: see if specifically requesting the info works
   AppRequest req = it->app->set_domain(it->domain);
   req.data->raw_requests[id] = Any();
   req.finalize_request();
   AppResponse new_response;
   try 
   {
      new_response = AppResponse
         ( req, data->raw_responses, data->transform_path.back().domain );
      Implementation::xform_path_t::const_iterator new_it = 
         new_response.data->getAppIterator(app);
      if ( new_it->responses.find(id) != new_it->responses.end() )
         return true;
   } catch ( ... ) {}

   return false;
}


const utilib::Any 
AppResponse::get( response_info_t id, const Application_Base* app ) const
{
   if ( data->transform_path.empty() )
      EXCEPTION_MNGR(std::runtime_error, "AppResponse::get: attempt "
                     "to interrogate an unpopulated AppResponse.");

   Implementation::xform_path_t::const_iterator it = data->getAppIterator(app);
   
   response_map_t::const_iterator r_it = it->responses.find(id);
   response_map_t::const_iterator r_itEnd = it->responses.end();
   if ( r_it != r_itEnd )
      return r_it->second;

   // The response isn't present.  It may be that this AppResponse was
   // pulled from the cache: see if specifically requesting the info works
   AppRequest req = it->app->set_domain(it->domain);
   req.data->raw_requests[id] = Any();
   req.finalize_request();
   AppResponse new_response;
   try 
   {
      new_response = AppResponse
         ( req, data->raw_responses, data->transform_path.back().domain );
      Implementation::xform_path_t::const_iterator new_it = 
         new_response.data->getAppIterator(app);
      r_it = new_it->responses.find(id);
      r_itEnd = new_it->responses.end();
   } catch ( ... ) {}

   if ( r_it == r_itEnd )
      EXCEPTION_MNGR(std::runtime_error,
                     "AppResponse::get - Missing response type: " << id 
                     << " [" << AppResponseInfo().name(id) << "]");
   return r_it->second;
}


void 
AppResponse::get( response_info_t id, utilib::AnyFixedRef value,
                  const Application_Base* app ) const
{
   try
   { 
      utilib::TypeManager()->lexical_cast(get(id, app), value); 
   }
   catch (utilib::bad_lexical_cast& err)
   {
      ucerr << "AppResponse::get - Problem casting "
            << utilib::demangledName(get(id).type()) << " to " 
            << utilib::demangledName(value.type()) << endl;
      throw;
   }
}


const AppResponse::response_map_t
AppResponse::get(const Application_Base* app) const
{
   if ( data->transform_path.empty() )
      EXCEPTION_MNGR(std::runtime_error, "AppResponse::get: attempt "
                     "to interrogate an unpopulated AppResponse.");

   return data->getAppIterator(app)->responses;
}



//========================================================================
// AppResponse private methods
//======================================================================

/// Construct a new AppResponse for a given domain point
AppResponse::AppResponse(const AppRequest &request,
                         const AppResponse::response_map_t &raw_resp,
                         const utilib::Any &raw_domain)
   : data(new Implementation(request.id(), request.data->seed, raw_resp))
{
   Implementation::xform_path_t& transform = data->transform_path;

   set<response_info_t> missing;

   list<AppRequest::ApplicationInfo>::reverse_iterator it 
      = request.data->transform_path.rbegin();
   list<AppRequest::ApplicationInfo>::reverse_iterator itEnd 
      = request.data->transform_path.rend();
   list<AppRequest::ApplicationInfo>::reverse_iterator itCur;

   AppRequest::request_map_t *requests = NULL;
   response_map_t *responses = &data->raw_responses;
   const Any* domain = &raw_domain;
   while ( it != itEnd )
   {
      // Verify the returned responses for completeness
      AppRequest::request_map_t::iterator q_it = it->requests.begin();
      AppRequest::request_map_t::iterator q_itEnd = it->requests.end();
      AppResponse::response_map_t::iterator s_it = responses->begin();
      AppResponse::response_map_t::iterator s_itEnd = responses->end();
      for ( ; q_it != q_itEnd ; ++q_it )
      {
         while (( s_it != s_itEnd ) && ( s_it->first < q_it->first ))
            ++s_it;
         if (( s_it == s_itEnd ) || ( q_it->first != s_it->first ))
            missing.insert(q_it->first);
      }
      if ( ! missing.empty() )
      {
         std::ostringstream os;
         os << "Constructing AppResponse: response from " 
            << utilib::demangledName(typeid(*it->app)) << " missing " 
            << missing.size() << " entries: ";
         while ( ! missing.empty() )
         {
            os << endl << "    " << AppResponseInfo().name(*missing.begin());
            missing.erase(missing.begin());
         }
         EXCEPTION_MNGR(std::runtime_error, os.str());
      }
      
      itCur = it;
      ++it;
      requests = ( it == itEnd ? &request.data->raw_requests : &it->requests );

      transform.push_front(ApplicationInfo());

      transform.front().app = itCur->app;

      // copy over the domain
      if (( itCur->domain.empty() ) && ( ! domain->empty() ))
         itCur->app->map_domain(*domain, transform.front().domain, false);
      else
         transform.front().domain = itCur->domain;
      domain = &transform.front().domain;

      // map the responses out of the sub-application space
      itCur->app->map_response( transform.front().domain,
                                *requests, *responses, 
                                transform.front().responses );

      // move up to the wrapping application
      responses = &transform.front().responses;
   }

   // Transcribe over the final results into the request Anys (if applicable)
   AppRequest::request_map_t::iterator req_it
      = request.data->raw_requests.begin();
   AppRequest::request_map_t::iterator req_itEnd
      = request.data->raw_requests.end();
   for( ; req_it != req_itEnd; ++req_it )
   {
      if ( ! req_it->second.empty() )
         utilib::TypeManager()->lexical_cast
             (transform.front().responses[req_it->first], req_it->second);
   }
}



//----------------------------------------------------------------
//
//
/*
void AppResponse::write(std::ostream& os) const
{
   os << "AppResponse Data:" << std::endl;
   response_set_map_t::const_iterator it = data->responses.begin();
   response_set_map_t::const_iterator itEnd = data->responses.end();
   for ( ; it != itEnd; it++) 
   {
      os << "  " << AppResponseInfo().name(it->first) 
         << " (" << it->second.data.size() << ")" << std::endl;
   }
}
*/

} // namespace colin


std::ostream& operator<<(std::ostream& os, const colin::AppResponse& r)
{
   if ( r.data == NULL )
   {
      os << "AppResponse: [EMPTY]" << endl;
      return os;
   }

   os << "AppResponse Data:" << endl;
   if ( r.data->transform_path.empty() )
      os << "   No data yet (response incomplete)." << endl;
   else
   {
      os << "  domain: " << r.data->transform_path.front().domain << endl;
      os << "  seed:   " << r.data->seed << endl;
      colin::AppResponse::response_map_t::const_iterator it
         = r.data->transform_path.front().responses.begin();
      colin::AppResponse::response_map_t::const_iterator itEnd
         = r.data->transform_path.front().responses.end();
      for ( ; it != itEnd; it++) 
      {
         os << "  " << colin::AppResponseInfo().name(it->first) 
            << ": " << it->second << endl;
      }
   }
   return os;
}
