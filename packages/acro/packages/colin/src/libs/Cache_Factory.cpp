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
 * \file Cache_Factory.cpp
 *
 * Defines the colin::cache::Factory class.
 */

#include <colin/cache/Factory.h>
#include <colin/Cache.h>
#include <colin/XMLProcessor.h>
#include <colin/TinyXML_data_parser.h>
#include <utilib/Property.h>
#include <utilib/TinyXML_helper.h>
#include <boost/algorithm/string.hpp>

using utilib::Any;

using std::cerr;
using std::endl;
using std::string;
using std::map;
using std::make_pair;

namespace colin { // colin
namespace StaticInitializers {
namespace {

int string2cache(const Any& from, Any& to)
{
   CacheHandle ans = CacheFactory().get_cache(from.expose<std::string>());
   to = ans;
   return ans.empty() ? utilib::Type_Manager::CastWarning::ValueMissing : 0;
}

bool RegisterCacheManager()
{
   bool ans = utilib::TypeManager()->register_lexical_cast
      (typeid(std::string), typeid(CacheHandle), &string2cache);
   return ans;
}

const bool register_all = RegisterCacheManager();
} // namespace colin::StaticInitializers::(local)
} // namespace colin::StaticInitializers


namespace { // colin::(local)

class ConfigureCacheFactory : public XML_Processor::ElementFunctor
{
   virtual void process(TiXmlElement* root, int version)
   {
      static_cast<void>(version);
      string txt;

      utilib::get_string_attribute(root, "default_cache_type", txt, "");
      if ( ! txt.empty() )
         CacheFactory().set_default_cache_type(txt);

      utilib::get_string_attribute(root, "default_indexer_type", txt, "");
      if ( ! txt.empty() )
         CacheFactory().set_default_indexer_type(txt);

      TiXmlElement *node = root->FirstChildElement("UnifiedGlobalCache");
      if ( node )
      {
         utilib::get_string_attribute(node, "master", txt, "");
         if ( txt.empty() )
            txt = "eval";

         if ( txt.compare("eval") == 0 )
            CacheFactory().intersolver_cache() = 
               CacheFactory().evaluation_cache();
         else if ( txt.compare("intersolver") == 0 )
            CacheFactory().evaluation_cache() = 
               CacheFactory().intersolver_cache();
         else
            EXCEPTION_MNGR(std::runtime_error, 
                           "ConfigureCacheFactory::process(): "
                           "invalid value for 'master' argument "
                           "(must be [eval,intersolver])");

         if ( utilib::get_string_attribute(node, "id", txt, "") )
            CacheFactory().register_cache
               ( CacheFactory().evaluation_cache(), txt );
      }
   }

   virtual void describe(std::ostream &os, size_t indent = 0) const
   {
      for( ; indent > 0; --indent )
         os << XML_Processor::indent_string;
      os << "Please implement ConfigureCacheFactory::describe" << endl;
   }
};

class CacheElement : public XML_Processor::ElementFunctor
{
   virtual void process(TiXmlElement* root, int version)
   {
      static_cast<void>(version);
      string type;
      utilib::get_string_attribute(root, "type", type, "");
      string indexer;
      utilib::get_string_attribute(root, "indexer", indexer, "");
      string name;
      utilib::get_string_attribute(root, "id", name, "");

      CacheFactory().register_cache
         ( CacheFactory().create(type, indexer), name );
   }

   virtual void describe(std::ostream &os, size_t indent = 0) const
   {
      for( ; indent > 0; --indent )
         os << XML_Processor::indent_string;
      os << "Please implement CacheElement::describe" << endl;
   }
};

class CacheViewElement : public XML_Processor::ElementFunctor
{
   virtual void process(TiXmlElement* root, int version)
   {
      static_cast<void>(version);
      string type;
      utilib::get_string_attribute(root, "type", type);
      string core_name;
      utilib::get_string_attribute(root, "base_cache", core_name, "");
      string name;
      utilib::get_string_attribute(root, "id", name, "");

      CacheHandle core = CacheFactory().get_cache(core_name);
      CacheHandle cache = CacheFactory().register_cache
         ( CacheFactory().create_view(type, core), name );

      TiXmlElement *child = root->FirstChildElement();
      while ( child )
      {
         if ( boost::iequals(child->ValueStr(), "options") )
         {
            TiXmlElement *tmp = child->FirstChildElement();
            while ( tmp )
            {
               if ( boost::iequals(tmp->ValueStr(), "option") )
               {
                  utilib::get_string_attribute(tmp, "name", name, "");
                  cache->property(name) = parse_xml_data(tmp);
               }
               else
                  UNEXPECTED_ELEMENT(tmp, "CacheViewElement::process()");
               tmp = tmp->NextSiblingElement();
            }
         }
         else
            UNEXPECTED_ELEMENT(child, "CacheViewElement::process()");
         child = child->NextSiblingElement();
      }
   }

   virtual void describe(std::ostream &os, size_t indent = 0) const
   {
      for( ; indent > 0; --indent )
         os << XML_Processor::indent_string;
      os << "Please implement CacheViewElement::describe" << endl;
   }
};

} // namespace colin::(local)

cache::Factory& CacheFactory()
{
   static cache::Factory factory;
   return factory;
}


namespace cache { // colin::cache

const std::string ImplicitInterSolverCacheName = "InterSolver";


//================================================================
// Fundamental KeyGenerators for Cache objects...
//
// ... they have to go somewhere, might as well be here.
//================================================================


/** \brief A key generator that uses default comparisons of the raw
 *  domain type.
 *
 *  This is a "dead-trivial" key generator: it simply uses the raw
 *  domain type.  Note: this requires the raw domain type to be
 *  Any-comparable.
 */
class KeyGenerator_ExactMatch : public Cache::KeyGenerator
{
public:
   virtual ~KeyGenerator_ExactMatch() {}
   virtual Cache::Key operator()(const utilib::Any domain)
   {
      return form_key(domain);
   }
};


/** \brief A key generator that returns a unique id for each request.
 *
 *  This is a "dead-trivial" key generator: it completely ignores the
 *  domain passed in and instead returns a unique id each time it is
 *  called.  This Key generator will NEVER result in a cache hit, unless
 *  the originator happens to remember the Key from when the point was
 *  originally inserted.
 *
 *  The only practical use for this cache is for a "write-only" archive.
 */
class KeyGenerator_Unique : public Cache::KeyGenerator
{
public:
   KeyGenerator_Unique() : last_id(0) {}
   virtual ~KeyGenerator_Unique() {}
   virtual Cache::Key operator()(const utilib::Any domain)
   {
      static_cast<void>(domain);
      return form_key(++last_id);
   }

private:
   size_t last_id;
};

//================================================================
// Fundamental creation functions for Factory registrations
//================================================================

namespace {

CacheHandle create_no_cache()
{
   return CacheHandle();
}
Cache::KeyGenerator* create_exact_indexer()
{
   return new KeyGenerator_ExactMatch;
}
Cache::KeyGenerator* create_unique_indexer()
{
   return new KeyGenerator_Unique;
}

} // namespace (local)


//================================================================
// Factory Public members
//================================================================

struct Factory::Data {
   Data()
      : default_cache_name("None"),
        default_indexer_name("Exact")
   {
      cache_factory.insert( cache_factory.end(), 
                             make_pair("None", &create_no_cache) );
      indexer_factory.insert( indexer_factory.end(), 
                               make_pair("Exact", &create_exact_indexer) );
      indexer_factory.insert( indexer_factory.end(), 
                               make_pair("Unique", &create_unique_indexer) );
   }


   ///
   map<string, cache_create_fcn_t>  cache_factory;
   ///
   map<string, view_create_fcn_t>  view_factory;
   ///
   map<string, indexer_create_fcn_t>  indexer_factory;
   ///
   string  default_cache_name;
   ///
   string  default_indexer_name;

   ///
   map<string, CacheHandle> cache_registry;

   CacheHandle evaluation_cache;
   CacheHandle intersolver_cache;
};


Factory::Factory()
   : data(new Data)
{
   XMLProcessor().register_element("Cache", 1, new CacheElement);
   XMLProcessor().register_element("CacheView", 1, new CacheViewElement);
   XMLProcessor().register_element("CacheFactory", 1, new ConfigureCacheFactory);
}


Factory::~Factory()
{
   delete data;
}



void
Factory::set_default_cache_type(std::string name)
{
   data->default_cache_name = name; 
}



void 
Factory::set_default_indexer_type(std::string name)
{
   data->default_indexer_name = name; 
}



bool 
Factory::
declare_cache_type(std::string name, cache_create_fcn_t fcn)
{
   if ( ! data->cache_factory.insert(make_pair(name, fcn)).second )
   {
      EXCEPTION_MNGR(std::runtime_error, "cache::Factory::"
                     "declare_cache_type(): duplicate cache class type, '"
                     << name << "'");
      return false;
   }
   return true;
}



bool 
Factory::
declare_view_type(std::string name, view_create_fcn_t fcn)
{
   if ( ! data->view_factory.insert(make_pair(name, fcn)).second )
   {
      EXCEPTION_MNGR(std::runtime_error, "cache::Factory::"
                     "declare_view_type(): duplicate view class type, '"
                     << name << "'");
      return false;
   }
   return true;
}



bool 
Factory::
declare_indexer_type(std::string name, indexer_create_fcn_t fcn)
{
   if ( ! data->indexer_factory.insert(make_pair(name, fcn)).second )
   {
      EXCEPTION_MNGR(std::runtime_error, "cache::Factory::"
                     "declare_indexer_type(): duplicate indexer type, '"
                     << name << "'");
      return false;
   }
   return true;
}


CacheHandle
Factory::create(std::string type, std::string indexer)
{
   if ( type.size() == 0 )
      type = data->default_cache_name;

   map<string, cache_create_fcn_t>::iterator c_it 
      = data->cache_factory.find(type);
   if ( c_it == data->cache_factory.end() )
      EXCEPTION_MNGR(std::runtime_error, "cache::Factory::create(): "
                     "unknown cache type, '" << type << "'");

   CacheHandle ans = c_it->second();
   if ( ans.empty() )
      return ans;

   if ( indexer.size() == 0 )
      indexer = data->default_indexer_name;

   map<string, indexer_create_fcn_t>::iterator i_it
      = data->indexer_factory.find(indexer);
   if ( i_it == data->indexer_factory.end() )
      EXCEPTION_MNGR(std::runtime_error, "cache::Factory::create(): "
                     "unknown indexer type, '" << indexer << "'");
   ans->set_key_generator(i_it->second());
   return ans;
}


CacheHandle
Factory::create_view(std::string type, CacheHandle core_cache)
{
   map<string, view_create_fcn_t>::iterator c_it 
      = data->view_factory.find(type);
   if ( c_it == data->view_factory.end() )
      EXCEPTION_MNGR(std::runtime_error, "cache::Factory::create_view(): "
                     "unknown view type, '" << type << "'");

   if ( core_cache.empty() )
   {
      core_cache = intersolver_cache();
      if ( core_cache.empty() )
         return CacheHandle();
   }
   return c_it->second(core_cache);
}


CacheHandle
Factory::get_cache(std::string name)
{
   map<string, CacheHandle>::iterator it = data->cache_registry.find(name);
   return ( it == data->cache_registry.end() ) ? CacheHandle() : it->second;
}

CacheHandle
Factory::register_cache(CacheHandle cache, std::string name)
{
   if ( cache.empty() )
      EXCEPTION_MNGR(std::runtime_error, "cache::Factory::"
                     "register_cache(): refusing to register NULL cache.");
   if ( name.empty() )
      EXCEPTION_MNGR(std::runtime_error, "cache::Factory::"
                     "register_cache(): refusing to register unnamed cache.");
   if ( ! data->cache_registry.insert(make_pair(name,cache)).second )
      EXCEPTION_MNGR(std::runtime_error, "cache::Factory::"
                     "register_cache(): duplicate cache name, '" 
                     << name << "'");
   return cache;
}

void
Factory::unregister_cache(std::string name)
{
   map<string, CacheHandle>::iterator it = data->cache_registry.find(name);
   if ( it == data->cache_registry.end() ) 
      EXCEPTION_MNGR(std::runtime_error, "cache::Factory::"
                     "unregister_cache(): unknown cache, '" << name << "'");

   data->cache_registry.erase(it);
}

CacheHandle&
Factory::evaluation_cache()
{
   if ( data->evaluation_cache.empty() )
      data->evaluation_cache = create();
   return data->evaluation_cache;
}

CacheHandle&
Factory::intersolver_cache()
{
   if ( data->intersolver_cache.empty() )
      data->intersolver_cache = create();
   return data->intersolver_cache;
}

} // namespace colin::cache
} // namespace colin
