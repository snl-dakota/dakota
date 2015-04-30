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
 * \file View_Subset.cpp
 *
 * Implements the colin::View_Subset class.
 */

#include <colin/cache/View_Subset.h>
#include <colin/cache/Factory.h>

#include <boost/bind.hpp>

using std::cerr;
using std::endl;

using std::list;
using std::map;
using std::pair;
using std::string;
using std::make_pair;

using utilib::Any;

namespace colin {

namespace StaticInitializers {

namespace {

/// Function to create a new View_Subset
CacheHandle create_view_subset(CacheHandle core_cache)
{
   std::pair<CacheHandle, cache::View_Subset*> tmp
      = CacheHandle::create<cache::View_Subset>();
   tmp.second->set_core_cache(core_cache);
   return tmp.first;
}

bool RegisterView_SubsetCache()
{
   CacheFactory().declare_view_type("Subset", create_view_subset);
   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool subset_view = RegisterView_SubsetCache();

} // namespace colin::StaticInitializers


namespace cache {


//================================================================
// View_Subset Public Member Definitions
//================================================================

void View_Subset::set_core_cache(CacheHandle src)
{
   if ( src.empty() )
      EXCEPTION_MNGR(std::runtime_error, "View_Subset::set_core_cache(): "
                     "Cannot form a view into a nonexistent Cache");

   clear();
   drop_connections();
   core_cache = src;

   // Clearing the core cache should clear this cache
   connections.push_back
      ( core_cache->onClear.connect
        ( boost::bind(&View_Subset::clear, this, _1) ) );

   // We can ignore all insertions into the underlying cache
   //cache.onInsert.connect
   //   ( boost::bind(boost::ref(cache.onInsert), _1) );

   // But we need to pass on Update signals (for points in this subset)
   connections.push_back
      ( core_cache->onUpdate.connect
        ( boost::bind(&View_Subset::cb_update, this, _1, _2) ) );

   // Catch any erase events that affect points in this view
   connections.push_back
      ( core_cache->onErase.connect
        ( boost::bind(&View_Subset::cb_erase, this, _1) ) );

   // Pass on all annotation events for points in this view to any clients
   connections.push_back
      ( core_cache->onAnnotate.connect
        ( boost::bind(&View_Subset::cb_annotate, this, _1, _2, _3) ) );
   connections.push_back
      ( core_cache->onEraseAnnotation.connect
        ( boost::bind(&View_Subset::cb_erase_annotation, this, _1, _2) ) );
}


void View_Subset::clear( const Application_Base* context )
{
   if ( context == NULL )
   {
      onClear(NULL);
      members.clear();
   }
   else
   {
      const Application_Base* app = get_core_application(context);
      onClear(app);
      members.erase( members.lower_bound(app), members.upper_bound(app) );
   }
}


std::pair<Cache::iterator, bool>
View_Subset::insert_impl( const Application_Base *context,
                          const Key& key,
                          const CoreResponseInfo& response )
{
   std::pair<Cache::iterator, bool> raw_inserted = 
      core_cache->insert(context, key, response);

   cache_t::iterator it = extract_iterator(raw_inserted.first);
   
   std::pair<key_map_t::iterator, bool> inserted = 
      members.insert(make_pair(it->first, it));

   if ( inserted.second )
   {
      onInsert(it);
   }

   return make_pair
      ( generate_iterator(it, IteratorData(inserted.first, it->first)), 
        inserted.second );
}


size_t
View_Subset::erase_item( CachedKey key )
{
   key_map_t::iterator c_it = members.lower_bound(key);
   key_map_t::iterator c_itEnd = members.upper_bound(key);
   if ( c_it == c_itEnd )
   {
      return 0;
   }
   else
   {
      size_t ans = 0;
      for( key_map_t::iterator tmp = c_it; tmp != c_itEnd; ++tmp )
      {
         onErase(tmp->second.src_it);
         ++ans;
      }
      
      members.erase(c_it, c_itEnd);
      return ans;
   }
}


void
View_Subset::erase_iterator( Cache::iterator pos )
{
   if ( pos == end() )
      return;

   cache_t::iterator it = extract_iterator(pos);
   key_map_t::iterator c_it = members.find(it->first);
   if ( c_it == members.end() || c_it->second.src_it != it )
      return;

   onErase(it);
   members.erase(c_it);
}


void 
View_Subset::cb_erase(cache_t::iterator it)
{
   key_map_t::iterator tmp = members.find(it->first);
   if ( tmp != members.end() && tmp->second.src_it == it )
   {
      onErase(it);
      members.erase(tmp);
   }
}


void
View_Subset::
cb_update(cache_t::iterator it, AppResponse::response_map_t &new_data)
{
   key_map_t::iterator tmp = members.find(it->first);
   if ( tmp != members.end() && tmp->second.src_it == it )
      onUpdate(it, new_data);
}


void
View_Subset::
cb_annotate(cache_t::iterator it, std::string attr, utilib::Any value)
{
   key_map_t::iterator tmp = members.find(it->first);
   if ( tmp != members.end() && tmp->second.src_it == it )
      onAnnotate(it, attr, value);
}


void 
View_Subset::cb_erase_annotation(cache_t::iterator it, std::string attr)
{
   key_map_t::iterator tmp = members.find(it->first);
   if ( tmp != members.end() && tmp->second.src_it == it )
      onEraseAnnotation(it, attr);
}



} // namespace colin::cache
} // namespace colin
