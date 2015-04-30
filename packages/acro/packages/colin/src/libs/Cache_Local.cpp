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
 * \file Local.cpp
 *
 * Implements the colin::Local class.
 */

#include <colin/cache/Local.h>
#include <colin/cache/Factory.h>
#include <colin/AppRequest.h>

#include <boost/bind.hpp>

using std::cerr;
using std::endl;

using std::set;
using std::map;
using std::pair;
using std::string;
using std::make_pair;

using utilib::Any;
using utilib::seed_t;

namespace colin {

namespace StaticInitializers {

namespace {

/// Function to create a new Local
CacheHandle create_local_cache()
{
   return CacheHandle::create<cache::Local>().first;
}

bool RegisterLocalCache()
{
   CacheFactory().declare_cache_type("Local", create_local_cache);
   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool local_cache = RegisterLocalCache();

} // namespace colin::StaticInitializers


namespace cache {

//================================================================
// Local::Data Member Definitions
//================================================================

struct Local::Data {

   typedef std::map<const Application_Base*, size_t>  sizeMap_t;

   Cache::cache_t  cache;
   sizeMap_t  cache_sizes;
};



//================================================================
// Local Public Member Definitions
//================================================================

Local::Local()
   : data(new Data())
{}


Local::~Local()
{
   delete data;
}


size_t Local::size( const Application_Base* context ) const
{
   if ( context == NULL )
      return data->cache.size();

   Data::sizeMap_t::iterator it
      = data->cache_sizes.find(get_core_application(context));
   return it == data->cache_sizes.end() ? 0 : it->second;
}


void Local::clear( const Application_Base* context )
{
   if ( context == NULL )
   {
      onClear(NULL);
      data->cache.clear();
      data->cache_sizes.clear();
   }
   else
   {
      const Application_Base* app = get_core_application(context);
      onClear(app);
      data->cache.erase( data->cache.lower_bound(app),
                         data->cache.upper_bound(app) );
      data->cache_sizes.erase(app);
   }
}


Cache::iterator 
Local::begin(const Application_Base *context) const
{   
   if ( context == NULL )
      return generate_iterator(data->cache.begin(), CachedKey());

   CachedKey key = CachedKey(get_core_application(context));
   cache_t::iterator it = data->cache.lower_bound( key );
   if ( it != data->cache.end() && it->first.context == key.context )
      return generate_iterator( it, key );
   else
      return end();
}


Cache::iterator 
Local::end() const
{
   return generate_iterator(data->cache.end(), CachedKey());
}


void 
Local::annotate(Cache::iterator pos, std::string attribute, utilib::Any value)
{
   if ( pos == end() )
      EXCEPTION_MNGR(std::runtime_error, "cache::Local::annotate(): "
                     "cannot annotate nonexistant item (end() pointer)");
   if ( attribute.empty() )
      EXCEPTION_MNGR(std::runtime_error, "cache::Local::annotate(): "
                     "cannot annotate with empty 'attribute'");

   onAnnotate(extract_iterator(pos), attribute, value);
   extract_iterator(pos)->second.annotations[attribute] = value;
}


size_t
Local::erase_annotation( Cache::iterator pos, std::string attribute )
{
   if ( pos == end() )
      EXCEPTION_MNGR(std::runtime_error, 
                     "cache::Local::erase_annotation(): "
                     "cannot erase annotations from nonexistant item "
                     "(end() pointer)");

   onEraseAnnotation(extract_iterator(pos), attribute);
   if ( attribute.empty() )
   {
      size_t ans = pos->second.annotations.size();
      extract_iterator(pos)->second.annotations.clear();
      return ans;
   }
   else
      return extract_iterator(pos)->second.annotations.erase(attribute);
}


std::pair<Cache::iterator, bool>
Local::insert_impl( const Application_Base *context,
                    const Key& key,
                    const CoreResponseInfo& response )
{
   if ( context == NULL )
      EXCEPTION_MNGR(std::runtime_error, "cache::Local::insert_impl(): "
                     "cannot insert with a NULL application context");
   if ( key.empty() )
      EXCEPTION_MNGR(std::runtime_error, "cache::Local::insert_impl(): "
                     "cannot insert an empty key");
   if ( ! key.Cacheable() )
      EXCEPTION_MNGR(std::runtime_error, "cache::Local::insert_impl(): "
                     "cannot insert a non-cacheable key into the cache");

   // The consumers of the raw insert method should know what they are
   // doing and guarantee that the context is already pointing to the
   // core application.
   if ( context != get_core_application(context) )
      EXCEPTION_MNGR(std::runtime_error, "cache::Local::insert_impl(): "
                     "INTERNAL ERROR: cannot insert a non-core application "
                     "context");

   std::pair<cache_t::iterator, bool> inserted = 
      data->cache.insert(make_pair(CachedKey(context,key), CachedData()));
   CachedData &cache_record = inserted.first->second;

   if ( inserted.second )
      ++(data->cache_sizes[context]);
   if ( cache_record.domain.empty() )
      cache_record.domain = response.domain;

   AppResponse::response_map_t newFields;
   std::set_difference
      ( response.responses.begin(), 
        response.responses.end(),
        cache_record.responses.begin(),
        cache_record.responses.end(),
        std::inserter(newFields, newFields.end()),
        response.responses.value_comp());

   cache_record.responses.insert( response.responses.begin(), 
                                  response.responses.end() );
   cache_record.eval_ids.push_back(response.app_eval_id);

   if ( inserted.second )
      onInsert(inserted.first);
   else if ( ! newFields.empty() )
      onUpdate(inserted.first, newFields);

   return make_pair( generate_iterator(inserted.first, inserted.first->first),
                     inserted.second );
}


size_t
Local::erase_item( CachedKey key )
{
   if ( key.context == NULL )
   {
      size_t ans = size();
      clear();
      return ans;
   }

   cache_t::iterator c_it = data->cache.lower_bound(key);
   cache_t::iterator c_itEnd = data->cache.upper_bound(key);
   if ( c_it == c_itEnd )
      return 0;

   size_t ans = 0;
   while ( c_it != c_itEnd )
   {
      onErase(c_it);
      --(data->cache_sizes[c_it->first.context]);
      data->cache.erase(c_it++);
      ++ans;
   }
   
   return ans;
}


void
Local::erase_iterator( Cache::iterator pos )
{
   if ( pos == end() )
      return;

   onErase(extract_iterator(pos));
   --(data->cache_sizes[pos->first.context]);
   data->cache.erase(extract_iterator(pos));
}


/// Return an iterator to the subset of elements matching the CachedKey
Cache::iterator
Local::find_impl( CachedKey cached_key ) const
{
   // The consumers of the raw find method should know what they are
   // doing and guarantee that the context is already pointing to the
   // core application.
   if ( get_core_application(cached_key.context) != cached_key.context )
      EXCEPTION_MNGR(std::runtime_error, "cache::Local::find_impl(): "
                     "INTERNAL ERROR: cannot search a non-core application "
                     "context");

   cache_t::iterator it = data->cache.lower_bound(cached_key);
   if ( it != data->cache.end() && it->first == cached_key )
      return generate_iterator(it, cached_key);
   else
      return generate_iterator(data->cache.end(), cached_key);
}



/// Return a general iterator to the first element with CachedKey >= key
Cache::iterator
Local::lower_impl( CachedKey cached_key ) const
{
   // The consumers of the raw lower_impl method should know what they are
   // doing and guarantee that the context is already pointing to the
   // core application.
   if ( get_core_application(cached_key.context) != cached_key.context )
      EXCEPTION_MNGR(std::runtime_error, "cache::Local::lower_impl(): "
                     "INTERNAL ERROR: cannot search a non-core application "
                     "context");

   cache_t::iterator it = data->cache.lower_bound(cached_key);
   if ( it != data->cache.end() && it->first.context == cached_key.context )
      return generate_iterator( it, CachedKey(cached_key.context) );
   else
      return generate_iterator( data->cache.end(), 
                                CachedKey(cached_key.context) );
}


/// Return a general iterator to the first element with CachedKey > key
Cache::iterator
Local::upper_impl( CachedKey cached_key ) const
{
   // The consumers of the raw upper_impl method should know what they are
   // doing and guarantee that the context is already pointing to the
   // core application.
   if ( get_core_application(cached_key.context) != cached_key.context )
      EXCEPTION_MNGR(std::runtime_error, "cache::Local::upper_impl(): "
                     "INTERNAL ERROR: cannot search a non-core application "
                     "context");

   cache_t::iterator it = data->cache.upper_bound(cached_key);
   if ( it != data->cache.end() && it->first.context == cached_key.context )
      return generate_iterator( it, CachedKey(cached_key.context) );
   else
      return generate_iterator( data->cache.end(), 
                                CachedKey(cached_key.context) );
}


void
Local::
increment_iterator(iterator::_base_ptr &it, utilib::Any &it_data) const
{
   // perform explicit bounds checking here...
   if ( it != data->cache.end() )
      ++it;

   if (it != data->cache.end() && it->first != it_data.expose<CachedKey>())
      it = data->cache.end();
}

void
Local::
decrement_iterator(iterator::_base_ptr &it, utilib::Any &it_data) const
{
   // The following assumes a behavior that decrementing begin() leaves
   // the iterator pointing to end().  This seems like rational behavior
   // to support as we need to ensure that the iterator is always
   // pointing to something that we "know about" (either a valid node or
   // end()); However, it is worth noting that this behavior deviates
   // from the STL standard, which does not specify behavior for
   // incrementing past end() or decrementing before begin().

   if ( it == data->cache.end() )
      it = data->cache.upper_bound(it_data.expose<CachedKey>());
   
   if ( it == data->cache.begin() )
      it = data->cache.end();
   else
      --it;
   
   if (it != data->cache.end() && it->first != it_data.expose<CachedKey>())
      it = data->cache.end();
}

} // namespace colin::cache
} // namespace colin
