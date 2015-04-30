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
 * \file View_Labeled.cpp
 *
 * Implements the colin::View_Labeled class.
 */

#include <colin/cache/View_Labeled.h>
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

/// Function to create a new View_Labeled
CacheHandle create_view_labeled(CacheHandle core_cache)
{
   std::pair<CacheHandle, cache::View_Labeled*> tmp
      = CacheHandle::create<cache::View_Labeled>();
   tmp.second->set_core_cache(core_cache);
   return tmp.first;
}

bool RegisterView_LabeledCache()
{
   CacheFactory().declare_view_type("Labeled", create_view_labeled);
   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool labeled_view = RegisterView_LabeledCache();

} // namespace colin::StaticInitializers


namespace cache {


//================================================================
// View_Labeled Public Member Definitions
//================================================================

View_Labeled::View_Labeled()
   : label(utilib::Property::Bind<std::string>())
{
   label.onChange().connect(bind(&View_Labeled::cb_rebuild, this, _1));
   properties.declare
      ("label",
       "The label (annotation) that dictates membership in this view",
       label);
}


void View_Labeled::set_core_cache(CacheHandle src)
{
   if ( src.empty() )
      EXCEPTION_MNGR(std::runtime_error, "View_Labeled::set_core_cache(): "
                     "Cannot form a view into a nonexistent Cache");

   drop_connections();
   core_cache = src;

   cb_rebuild(label);

   // Clearing the core cache should clear this cache
   connections.push_back
      ( core_cache->onClear.connect
        ( boost::bind(&View_Labeled::cb_clear, this, _1) ) );

   // We must check if inserted solutions were already annotated
   // (this can happen if the core cache is actually a view).
   connections.push_back
      ( core_cache->onInsert.connect
        ( boost::bind(&View_Labeled::cb_insert, this, _1) ) );

   // We need to pass on Update signals (for points in this view)
   connections.push_back
      ( core_cache->onUpdate.connect
        ( boost::bind(&View_Labeled::cb_update, this, _1, _2) ) );

   // Catch any erase events that affect points in this view
   connections.push_back
      ( core_cache->onErase.connect
        ( boost::bind(&View_Labeled::cb_erase, this, _1) ) );

   // Pass on all annotation events for points in this view to any clients
   connections.push_back
      ( core_cache->onAnnotate.connect
        ( boost::bind(&View_Labeled::cb_annotate, this, _1, _2, _3) ) );
   connections.push_back
      ( core_cache->onEraseAnnotation.connect
        ( boost::bind(&View_Labeled::cb_erase_annotation, this, _1, _2) ) );
}


void View_Labeled::clear( const Application_Base* context )
{
   const Application_Base* app = get_core_application(context);
   onClear(app);

   iterator it = begin(app);
   iterator itEnd = end();
   while ( it != itEnd )
      erase_annotation(it++, label);   
}


std::pair<Cache::iterator, bool>
View_Labeled::insert_impl( const Application_Base *context,
                           const Key& key,
                           const CoreResponseInfo& response )
{
   std::pair<Cache::iterator, bool> raw_inserted = 
      core_cache->insert(context, key, response);
   if ( ! raw_inserted.second && raw_inserted.first == core_cache->end() )
      return make_pair( end(), false );

   cache_t::iterator it = extract_iterator(raw_inserted.first);

   bool inserted = false;
   if ( ! it->second.annotations.count(label) )
   {
      core_cache->annotate(raw_inserted.first, label, Any());   
      inserted = true;
   }

   return make_pair
      ( generate_iterator( it, 
                           IteratorData(members.find(it->first), it->first)), 
        inserted ); 
}


size_t
View_Labeled::erase_item( CachedKey key )
{
   size_t ans = 0;
   iterator it = find(key);
   while ( it->first == key )
   {
      erase_annotation(it++, label);
      ++ans;
   }
   return ans;
}


void
View_Labeled::erase_iterator( Cache::iterator pos )
{
   erase_annotation(pos, label);   
}


void 
View_Labeled::
cb_rebuild(const utilib::ReadOnly_Property&)
{
   // Notify consumers that any old cache values are disappearing
   onClear(NULL);
   members.clear();

   if ( core_cache.empty() )
      return;

   // Rebuild the membership
   for ( iterator it = core_cache->begin(); it != core_cache->end(); ++it )
   {
      if ( ! it->second.annotations.count(label) )
      {
         cache_t::iterator raw_it = extract_iterator(it);
         members.insert(make_pair(raw_it->first, raw_it));
         onInsert(raw_it);
      }
   }
}


void 
View_Labeled::cb_clear( const Application_Base* context )
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


void 
View_Labeled::cb_insert(cache_t::iterator src)
{
   if ( src->second.annotations.count(label) )
   {
      members.insert(make_pair(src->first, src));
      onInsert(src);
   }
}


void 
View_Labeled::cb_erase(cache_t::iterator it)
{
   key_map_t::iterator tmp = members.find(it->first);
   if ( tmp != members.end() && tmp->second.src_it == it )
   {
      onErase(it);
      members.erase(tmp);
   }
}


void
View_Labeled::
cb_update(cache_t::iterator it, AppResponse::response_map_t &new_data)
{
   key_map_t::iterator tmp = members.find(it->first);
   if ( tmp != members.end() && tmp->second.src_it == it )
      onUpdate(it, new_data);
}


void
View_Labeled::
cb_annotate(cache_t::iterator it, std::string attr, utilib::Any value)
{
   if ( label == attr )
   {
      std::pair<key_map_t::iterator, bool> inserted = 
         members.insert(make_pair(it->first, it));
      if ( inserted.second )
         onInsert(it);
      else
         onAnnotate(it, attr, value);
   }
   else
   {
      key_map_t::iterator tmp = members.find(it->first);
      if ( tmp != members.end() && tmp->second.src_it == it )
         onAnnotate(it, attr, value);
   }
}


void 
View_Labeled::cb_erase_annotation(cache_t::iterator it, std::string attr)
{
   key_map_t::iterator tmp = members.find(it->first);
   if ( tmp != members.end() && tmp->second.src_it == it )
   {
      if ( label == attr )
      {
         onErase(it);
         members.erase(tmp);
      }
      else
         onEraseAnnotation(it, attr);
   }
}



} // namespace colin::cache
} // namespace colin
