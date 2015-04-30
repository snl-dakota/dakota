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
 * \file Cache.cpp
 *
 * Implements the colin::Cache class.
 */

#include <colin/Cache.h>
#include <colin/cache/Factory.h>

#include <colin/ExecuteMngr.h>
#include <colin/ApplicationMngr.h>
#include <colin/SolverMngr.h>

#include <utilib/TinyXML_helper.h>

using std::cerr;
using std::endl;

using std::string;
using std::stringstream;
using std::list;
using std::map;
using std::pair;
using std::make_pair;

using utilib::Any;
using utilib::TypeManager;


namespace colin {

class PrintCacheExecuteFunctor : public ExecuteManager::ExecuteFunctor
{
public:
   PrintCacheExecuteFunctor()
   {}

   virtual void execute(TiXmlElement* node)
   {
      string cache_name = cache::ImplicitInterSolverCacheName;
      string pname = ApplicationMngr().get_newest_application();
      ApplicationHandle problem;

      if ( node )
      {
         utilib::get_string_attribute( node, "cache", cache_name, cache_name );

         bool found_problem = 
            utilib::get_string_attribute( node, "problem", pname, pname );

         string sname = "";
         bool found_solver = 
            utilib::get_string_attribute( node, "solver", sname, "");

         if ( found_solver && found_problem )
            EXCEPTION_MNGR(std::runtime_error, 
                           "PrintCacheExecuteFunctor::execute(): "
                           "cannot specify both problem_context and "
                           "solver_context in " 
                           << utilib::get_element_info(node));

         if ( found_solver )
         {
            SolverHandle solver = SolverMngr().get_solver(sname);
            // the solver manager throws an exception for unknown solvers
            //if ( solver.empty() )
            //   EXCEPTION_MNGR(std::runtime_error,
            //                  "PrintCacheExecuteFunctor::execute(): "
            //                  "solver_context (" << sname << ") not found");
            problem = solver->get_problem_handle();
         }
      }

      CacheHandle cache = CacheFactory().get_cache(cache_name);
      if ( cache.empty() )
      {
         std::cout << "<PrintCache> ERROR: unknown cache '" 
                   << cache_name << "'" << endl;
         return;
      }

      if ( problem.empty() )
         problem = ApplicationMngr().get_application(pname);
      if ( problem.empty() )
         EXCEPTION_MNGR(std::runtime_error,
                        "PrintCacheExecuteFunctor::execute(): "
                        "cannot find problem context (" << pname << ") at "
                        << utilib::get_element_info(node));

      size_t n = cache->size(problem.object());
      std::cout << "Cache:" << endl
                << "   name: " << cache_name << endl
                << "   size: " << n << endl;
      Cache::iterator it = cache->begin(problem.object());
      Cache::iterator itEnd = cache->end();
      if ( it != itEnd )
         std::cout << "   data:" << endl;
      for ( ; it != itEnd; ++it )
      {
         std::stringstream tmp;
         tmp << it->second.asResponse(problem.object());
         std::string tmpstr = tmp.str();
         if ( tmpstr.size() >= 2 )
         {
            size_t pos = tmpstr.size()-2; //skip the last newline
            size_t found = 0;
            while((found = tmpstr.rfind("\n", pos)) != std::string::npos)
            {
               tmpstr.replace(found, 1, "\n      ");
               pos = found - 1;
            }
         }
         std::cout << "    - " << tmpstr;
         if ( ! it->second.annotations.empty() )
         {
            std::cout << "      Annotations:" << endl;
            Cache::CachedData::annotation_map_t::const_iterator a_it;
            for ( a_it = it->second.annotations.begin();
                  a_it != it->second.annotations.end();
                  ++a_it )
               std::cout << "        " << a_it->first << ": " 
                         << a_it->second << endl;
         }
      }
   }
};

namespace {

const volatile bool registered =
   ExecuteMngr().register_command("PrintCache", new PrintCacheExecuteFunctor);

} // namespace colin::(local)


//================================================================
// Cache::Key Member Definitions
//================================================================

bool Cache::Key::Cacheable() const
{
   if ( key.empty() )
      return false;

   if ( TypeManager()->lexical_castable( key.type(), 
                                         typeid(CacheableObjectTest) ) )
   {
      CacheableObjectTest test;
      TypeManager()->lexical_cast(key, test);
      return test.cacheable;
   }
   return true;
}

//================================================================
// Cache::Key Member Definitions
//================================================================

AppResponse 
Cache::CachedData::asResponse(const AppRequest &request) const
{
   return form_response(request, CoreResponseInfo(domain, responses));
}

AppResponse 
Cache::CachedData::asResponse(const Application_Base *context) const
{
   return form_response(context, CoreResponseInfo(domain, responses));
}

utilib::Any
Cache::CachedData::annotation(const std::string &name) const
{
   annotation_map_t::const_iterator a_it = annotations.find(name);
   if ( a_it == annotations.end() )
      return Any();
   else
      return a_it->second;
}

//================================================================
// Cache Public Member Definitions
//================================================================


/// Remove a CachedKey from the cache
size_t Cache::erase(CachedKey key)
{
   return erase_item(key);
}


/// Remove a key from the cache
size_t Cache::erase(const Application_Base *context, Key key)
{
   return erase_item(CachedKey(get_core_application(context), key)); 
}


/// Remove a domain point from the cache
size_t 
Cache::erase(const Application_Base *context, const utilib::Any domain)
{
   // Map the provided domain into the core application domain
   AppRequest req = context->set_domain(domain);
   return erase_item(CachedKey( get_core_application(context), 
                                generate_key(get_core_request(req).domain) ));
}


/// Remove an element ponted to by pos from the cache
void Cache::erase( iterator pos )
{
   erase_iterator(pos); 
}


/// Insert into the cache, return an iterator to the inserted element
/// and a bool indicating if an insertion took place.
std::pair<Cache::iterator, bool> 
Cache::insert( const Application_Base *context, 
                       const Key& key, const CoreResponseInfo &value )
{
   return insert_impl(context, key, value);
}


std::pair<Cache::iterator, bool>
Cache::insert(const Key key, const AppResponse response)
{
   return insert_impl( get_core_application(response),
                       key, 
                       get_core_response(response) );
}


std::pair<Cache::iterator, bool>
Cache::insert(const AppResponse response)
{
   const Application_Base* context = get_core_application(response);
   return insert_impl( context, 
                       generate_key(response.get_domain(context)),
                       get_core_response(response) );
}


/// FIXME: We should make it easier / more robust to insert an iterator
/// from one cache into another.  In particular, note that this process
/// loses all EvalID's
std::pair<Cache::iterator, bool>
Cache::insert( Cache::const_iterator it )
{
   CoreResponseInfo cri = 
      CoreResponseInfo( it->second.domain, it->second.responses );
   return insert_impl( it->first.context, it->first.key, cri);
}


std::pair<Cache::iterator, bool>
Cache::insert( Cache::iterator it )
{
   CoreResponseInfo cri = 
      CoreResponseInfo( it->second.domain, it->second.responses );
   return insert_impl( it->first.context, it->first.key, cri);
}


/// Return an iterator to the element whose CachedKey is key
Cache::iterator 
Cache::find( CachedKey key ) const
{
   return find_impl( key );
}

/// Return an iterator to the element whose Key is key
Cache::iterator 
Cache::find(const Application_Base *context, Cache::Key key) const
{
   return find_impl(CachedKey(get_core_application(context), key));
}

/// Convenience: Return cache iterator and key matching the CoreRequestInfo
std::pair<Cache::iterator, Cache::Key> 
Cache::find( const CoreRequestInfo &cqi ) const
{
   Key key = generate_key(cqi.domain);
   return std::pair<iterator, Key>(find_impl(CachedKey(cqi.app, key)), key);
}

/// Convenience: return the AppResponses matching the Context and Domain
std::pair<Cache::iterator, Cache::Key>
Cache::find( const Application_Base *context, const utilib::Any domain) const
{
   // Map the provided domain into the core application domain
   AppRequest req = context->set_domain(domain);
   return find(get_core_request(req));
}


/// Return an iterator to the first element whose CachedKey is >= key
Cache::iterator 
Cache::lower_bound( CachedKey key ) const
{
   return lower_impl( key );
}

/// Convenience: Return an iterator to the first element whose Key is >= key
Cache::iterator 
Cache::lower_bound(const Application_Base *context, Cache::Key key) const
{
   return lower_impl(CachedKey(get_core_application(context), key));
}

/// Convenience: return the first iterator, key >= domain
std::pair<Cache::iterator, Cache::Key>
Cache::lower_bound( const Application_Base *context, 
                    const utilib::Any domain) const
{
   // Map the provided domain into the core application domain
   AppRequest req = context->set_domain(domain);
   const CoreRequestInfo cqi = get_core_request(req);
   Key key = generate_key(cqi.domain);
   return std::pair<iterator, Key>(lower_impl(CachedKey(cqi.app, key)), key);
}


/// Return an iterator to the first element whose CachedKey is > key
Cache::iterator 
Cache::upper_bound( CachedKey key ) const
{
   return upper_impl( key );
}

/// Convenience: Return an iterator to the first element whose Key is > key
Cache::iterator 
Cache::upper_bound(const Application_Base *context, Cache::Key key) const
{
   return upper_impl(CachedKey(get_core_application(context), key));
}

/// Convenience: return the first iterator, key > domain
std::pair<Cache::iterator, Cache::Key>
Cache::upper_bound( const Application_Base *context, 
                    const utilib::Any domain) const
{
   // Map the provided domain into the core application domain
   AppRequest req = context->set_domain(domain);
   const CoreRequestInfo cqi = get_core_request(req);
   Key key = generate_key(cqi.domain);
   return std::pair<iterator, Key>(upper_impl(CachedKey(cqi.app, key)), key);
}

utilib::Property&
Cache::property(std::string name)
{
   utilib::PropertyDict::iterator it = properties.find(name);
   if ( it == properties.end() )
      EXCEPTION_MNGR(std::runtime_error, "Cache::property(): "
                     "Attempt to retrieve nonexistent property, '"
                     << name << "'");

   return it->second();
}



//================================================================
// ConcreteCache Public Member Definitions
//================================================================

void
ConcreteCache::
set_key_generator(utilib::SharedPtr<Cache::KeyGenerator> new_keygen)
{
   if ( size() > 0 )
   {
      cerr << "WARNING: setting key generator for a non-empty cache results "
         "in loss of data." << endl;
      clear();
   }
   keygen = new_keygen;
}


} // namespace colin
