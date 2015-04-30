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
 * \file Application_Base.cpp
 *
 * Implements the colin::Application_Base class.
 */

#include <colin/application/Base.h>
#include <colin/ApplicationMngr.h>
#include <colin/AppRequest_Impl.h>
#include <colin/EvaluationManager.h>
#include <colin/TinyXML_data_parser.h>

#include <utilib/TinyXML_helper.h>
#include <utilib/exception_mngr.h>

#include <tinyxml/tinyxml.h>
#include <boost/algorithm/string.hpp>

#include <typeinfo>
#include <iostream>

using std::type_info;
using std::list;
using std::set;
using std::map;
using std::pair;

using std::string;
using std::cerr;
using std::endl;

using utilib::seed_t;
using utilib::Any;
using utilib::Property;

namespace colin
{

namespace // colin::(local)
{

/// map of ObjectTypes (derived classes) to specific info type they handle
std::map<ObjectType, std::set<const response_info_t*> >&
declared_response_info_types()
{
   static std::map<ObjectType, std::set<const response_info_t*> > ans;
   return ans;
}

} // namespace colin::(local)



//========================================================================
// Application_Base public members
//======================================================================


Application_Base::Application_Base()
   : eval_counter(0),
     response_map_counter(0)
{}


Application_Base::~Application_Base()
{}


void
Application_Base::initialize(TiXmlElement *element)
{
   if ( element == NULL )
      return;

   // Now, parse all valid sub-elements
   TiXmlElement* node = element->FirstChildElement();
   for ( ; node != NULL; node = node->NextSiblingElement() )
   {
      // universal fields (like the options element)
      if ( boost::iequals(node->ValueStr(), "options") )
      {
         TiXmlElement *tmp = node->FirstChildElement();
         while ( tmp )
         {
            if ( boost::iequals(tmp->ValueStr(), "option") )
            {
               std::string name;
               utilib::get_string_attribute(tmp, "name", name, "");
               property(name) = parse_xml_data(tmp);
            }
            else
               UNEXPECTED_ELEMENT(tmp, "Application_Base::initialize()");
            tmp = tmp->NextSiblingElement();
         }
         continue;
      }

      cb_initializer_t &init = initializer(node->ValueStr());
      if ( init.empty() )
         EXCEPTION_MNGR(std::runtime_error, "Application_Base::initialize: "
                        "No handlers registered for element '" << 
                        node->ValueStr() << "' found on line " << node->Row());

      try {
         init(node);
      } catch ( std::exception& e ) {
         EXCEPTION_MNGR( std::runtime_error, "Application_Base::initialize: "
                         "Exception caught processing line " << node->Row()
                         << ":" << endl << e.what() );
      } catch ( ... ) {
         EXCEPTION_MNGR( std::runtime_error, "Application_Base::initialize: "
                         "Unknown exception caught processing line " 
                         << node->Row() << "." );
      }
   }
}


void
Application_Base::finalize()
{
   finalize_signal();
}


void
Application_Base::print_summary(std::ostream& os) const
{
   print_signal(os);
}


utilib::Property&
Application_Base::property(std::string name)
{
   utilib::PropertyDict::iterator it = properties.find(name);
   if ( it == properties.end() )
      EXCEPTION_MNGR(std::runtime_error, "Application_Base::property(): "
                     "Attempt to retrieve nonexistent property, '"
                     << name << "'");

   return it->second();
}


const utilib::Property&
Application_Base::property(std::string name) const
{
   utilib::PropertyDict::const_iterator it = properties.find(name);
   if ( it == properties.end() )
      EXCEPTION_MNGR(std::runtime_error, "Application_Base::property(): "
                     "Attempt to retrieve nonexistent property, '"
                     << name << "'");

   return it->second();
}


void 
Application_Base::set_rng(utilib::Any rng_)
{
   if (rng_.empty()) return;
   if (rng_.type() == typeid(utilib::RNG*))
   {
      utilib::RNG* tmp = rng_.expose<utilib::RNG*>();
      rng = tmp;
   }
   else if (rng_.type() == typeid(utilib::AnyRNG))
   {
      const utilib::AnyRNG& tmp = rng_.expose<utilib::AnyRNG>();
      rng = tmp;
   }
   else 
      EXCEPTION_MNGR(std::runtime_error, "Application_Base::set_rng(): "
                     "unable to set rng from Any containing " <<
                     utilib::demangledName(rng_.type()));
}


void 
Application_Base::set_evaluation_manager(EvaluationManager_Handle new_mngr)
{
   eval_manager = EvaluationManager(new_mngr);
   //std::cerr << "application '" << typeid(*this).name()
   //          << "' assigned solver id = "
   //          << eval_manager.solverID() << std::endl;
}


const EvaluationManager&
Application_Base::eval_mngr()
{
   if (! eval_manager)
   {
      EvaluationManager_Handle tmp = default_eval_mngr();
      if ( tmp.empty() )
         tmp = EvalManagerFactory().default_manager();

      eval_manager = EvaluationManager(tmp);
      //cerr << "application '" << typeid(*this).name()
      //     << "' inherited solver id = "
      //     << eval_manager.solverID() << endl;
   }
   return eval_manager;
}


AppRequest
Application_Base::set_domain(const utilib::Any domain,
                             const bool recalculate,
                             const seed_t seed) const
{
   AppRequest request(domain, recalculate, seed);

   // Resolve all the "native domains" for the applications in the chain
   const Application_Base* app = this;
   Any tmp = domain;
   while ( app != NULL )
   {
      Any& sub_domain = request.add_reformulation(app).domain;
      if ( ! tmp.empty() )
      {
         app->map_domain(tmp, sub_domain, true);
         
         tmp.clear();  // use clear() in case the domain came in immutable
         tmp = sub_domain;
      }

      app = app->get_reformulated_application();
   }

   return request;
}


void
Application_Base::Request_response( AppRequest &request, 
                                    AppResponse &response )
{
   if ( request.application() != this )
      EXCEPTION_MNGR(std::runtime_error,
                     "Application_Base::Request_response(): "
                     "Application does not match request application");

   if ( request.finalized() )
      EXCEPTION_MNGR(std::runtime_error,
                     "Application_Base::Request_response(): "
                     "Attempting to request a response for a request "
                     "that is already finalized (queued or evaluated)");

   if ( request.data->response != NULL )
      EXCEPTION_MNGR(std::runtime_error,
                     "Application_Base::Request_response(): "
                     "Duplicate response request");

   request.data->response = &response;
}


void 
Application_Base::synchronize()
{
   if ( eval_manager )
      eval_manager.synchronize();
}


void
Application_Base::map_request( AppRequest::request_map_t &outer_request,
                               AppRequest::request_map_t &inner_request ) const
{
   // 1) First, expland any request aliases
   request_expansion_signal(outer_request);

   // 1) start with a direct copy of the previous request map
   inner_request = outer_request;

   // 2) call any/all callbacks registered with the mapping application
   // (in order registered: usually from base class to most derived)
   request_transform_signal(outer_request, inner_request);
}


void
Application_Base::
map_response( const utilib::Any &domain,
              const AppRequest::request_map_t &outer_request,
              const AppResponse::response_map_t &sub_response,
              //std::list<fcn_cb_response_t> &local_callbacks,
              AppResponse::response_map_t &response )
{
   // 0) increment the number of responses I have mapped
   ++response_map_counter;

   // 1) start with a direct copy of the previous request map (but only
   //    copy the info types that this application supports)
   AppResponse::response_map_t::const_iterator it = sub_response.begin();
   AppResponse::response_map_t::const_iterator itEnd = sub_response.end();
   response.clear();
   for ( ; it != itEnd; ++it )
      if ( active_response_info.count(it->first) )
         response.insert(response.end(), *it);

   // 2) call any/all callbacks registered with the mapping application
   size_t ans = 
      response_transform_signal(domain, outer_request, sub_response, response);
   if ( ans )
   {
      bool found_missing = false;
      AppRequest::request_map_t::const_iterator req_it = outer_request.begin();
      for( ; req_it != outer_request.end(); ++req_it )
         if ( ! response.count(req_it->first) )
         {
            found_missing = true;
            break;
         }
      if ( found_missing )
      {
         std::ostringstream missing;
         // NB: since we break out of the first loop at the first
         // missing entry, we don't have to re-start the search at the
         // beginning.
         for( ; req_it != outer_request.end(); ++req_it )
            if ( ! response.count(req_it->first) )
               missing << endl 
                       << "   " << AppResponseInfo().name(req_it->first);
         EXCEPTION_MNGR( std::runtime_error, 
                         "Application_Base::map_response(): "
                         << ans << " response callbacks returned insufficient "
                         "data:" << missing.str() );
      }
   }
}


bool 
Application_Base::
declare_response_info(ObjectType type, const response_info_t &info)
{ 
   return declared_response_info_types()[type].insert(&info).second; 
}


//========================================================================
// Application_Base protected members - manage the inheritance diamond
//========================================================================

void
Application_Base::
referencePropertiesFrom( Application_Base* source,
                         std::set<utilib::Any> exclude, 
                         std::set<std::string> block_promotion )
{
   properties.reference(source->properties, exclude, block_promotion);
}

void
Application_Base::
register_application_component(ObjectType type)
{
   typedef std::set<const response_info_t*> infoset_t;

   infoset_t &info = declared_response_info_types()[type];
   if ( ! info.empty() )
      for( infoset_t::iterator it = info.begin(), end = info.end();
           it != end; 
           active_response_info.insert( **(it++) ) );
   

}


//========================================================================
// Application_Base protected members - manage evals & transformations
//========================================================================

void
Application_Base::record_compute_task( response_info_t id,
                                          AppRequest& request,
                                          utilib::Any& result_loc )
{
   if ( request.application() != this )
      EXCEPTION_MNGR(std::runtime_error,
                     "Application_Base::record_compute_task(): "
                     "Application does not match request application");

   if ( request.finalized() )
      EXCEPTION_MNGR(std::runtime_error,
                     "Application_Base::Request_response(): "
                     "Attempting to request a response for a request "
                     "that is already finalized (queued or evaluated)");

   if ( ! request.data->raw_requests.insert
        ( AppRequest::request_map_t::value_type(id, result_loc) ).second )
   {
      EXCEPTION_MNGR(std::runtime_error,
                     "Application_Base::record_compute_task(): "
                     "Duplicate application response_info (" << id << 
                     ") compute request");
   }
}


void
Application_Base::record_remote_compute_task( Application_Base* app,
                                                 response_info_t id,
                                                 AppRequest& request,
                                                 utilib::Any& result_loc )
{
   if ( request.application() != app )
      EXCEPTION_MNGR(std::runtime_error,
                     "Application_Base::record_remote_compute_task(): "
                     "Application does not match request application");

   if ( request.finalized() )
      EXCEPTION_MNGR(std::runtime_error,
                     "Application_Base::Request_response(): "
                     "Attempting to request a response for a request "
                     "that is already finalized (queued or evaluated)");

   if ( ! request.data->raw_requests.insert
        ( AppRequest::request_map_t::value_type(id, result_loc) ).second )
   {
      EXCEPTION_MNGR(std::runtime_error,
                     "Application_Base::record_remote_compute_task(): "
                     "Duplicate application response_info (" << id << 
                     ") compute request");
   }
}


//========================================================================
// Application_Base private members
//========================================================================


} // namespace colin
