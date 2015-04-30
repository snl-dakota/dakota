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


#include <colin/cache/View_CommonBase.h>

using std::make_pair;
using std::runtime_error;

namespace colin {
namespace cache {

size_t
View_CommonBase::size( const Application_Base* context ) const
{
   if ( context == NULL )
      return members.size();

   CachedKey key = CachedKey(get_core_application(context));
   size_t count = 0;
   key_map_t::const_iterator it = members.lower_bound(key);
   key_map_t::const_iterator itEnd = members.end();
   while ( it != itEnd && it->first.context == key.context )
   {
      ++count;
      ++it;
   }
   return count;
}

Cache::iterator
View_CommonBase::begin( const Application_Base *context ) const
{
   if ( context == NULL )
      return generate_iterator
         ( members.empty() 
              ? extract_iterator(core_cache->end()) 
              : members.begin()->second.src_it,
           IteratorData(members.begin(), CachedKey()) );

   CachedKey key = CachedKey(get_core_application(context));
   key_map_t::const_iterator it = members.lower_bound( key );
   if ( it == members.end() || it->first.context != key.context )
      return generate_iterator
         ( extract_iterator(core_cache->end()), 
           IteratorData(members.end(), key) );
   else
      return generate_iterator( it->second.src_it, IteratorData(it, key) );
}


Cache::iterator 
View_CommonBase::end() const
{
   return generate_iterator
         ( extract_iterator(core_cache->end()), 
           IteratorData(members.end(), CachedKey()) );
}


void 
View_CommonBase::annotate( Cache::iterator pos,
                           std::string attribute, 
                           utilib::Any value )
{
   if ( pos == end() )
      EXCEPTION_MNGR(std::runtime_error, "cache::View_CommonBase::annotate(): "
                     "cannot annotate nonexistant item (end() pointer)");
   core_cache->annotate(pos, attribute, value);
}


size_t
View_CommonBase::erase_annotation( Cache::iterator pos,
                               std::string attribute )
{
   if ( pos == end() )
      EXCEPTION_MNGR(std::runtime_error, 
                     "cache::View_CommonBase::erase_annotation(): "
                     "cannot erase annotations from nonexistant item "
                     "(end() pointer)");

   return core_cache->erase_annotation(pos, attribute);
}


/// Generate a set of Core AppResponses that match the given Context & Key
Cache::iterator
View_CommonBase::find_impl( CachedKey cached_key ) const
{
   // The consumers of the raw find_impl method should know what they are
   // doing and guarantee that the context is already pointing to the
   // core application.
   if ( get_core_application(cached_key.context) != cached_key.context )
      EXCEPTION_MNGR(runtime_error, "cache::View_CommonBase::find_impl(): "
                     "INTERNAL ERROR: cannot search a non-core application "
                     "context");

   key_map_t::const_iterator it = members.lower_bound(cached_key);
   if ( it != members.end() && it->first == cached_key )
      return generate_iterator( it->second.src_it, 
                                IteratorData(it, cached_key) );
   else
      return generate_iterator
         ( extract_iterator(core_cache->end()), 
           IteratorData(members.end(), cached_key) );
}


/// Return a general iterator to the first element with CachedKey >= key
Cache::iterator
View_CommonBase::lower_impl( CachedKey cached_key ) const
{
   // The consumers of the raw lower_impl method should know what they are
   // doing and guarantee that the context is already pointing to the
   // core application.
   if ( get_core_application(cached_key.context) != cached_key.context )
      EXCEPTION_MNGR(runtime_error, "cache::View_CommonBase::lower_impl(): "
                     "INTERNAL ERROR: cannot search a non-core application "
                     "context");

   key_map_t::const_iterator it = members.lower_bound(cached_key);
   if ( it != members.end() && it->first.context == cached_key.context )
      return generate_iterator
         ( it->second.src_it, 
           IteratorData(it, CachedKey(cached_key.context)) );
   else
      return generate_iterator
         ( extract_iterator(core_cache->end()), 
           IteratorData( members.end(), 
                               CachedKey(cached_key.context) ) );
}


/// Return a general iterator to the first element with CachedKey > key
Cache::iterator
View_CommonBase::upper_impl( CachedKey cached_key ) const
{
   // The consumers of the raw upper_impl method should know what they are
   // doing and guarantee that the context is already pointing to the
   // core application.
   if ( get_core_application(cached_key.context) != cached_key.context )
      EXCEPTION_MNGR(runtime_error, "cache::View_CommonBase::upper_impl(): "
                     "INTERNAL ERROR: cannot search a non-core application "
                     "context");

   key_map_t::const_iterator it = members.upper_bound(cached_key);
   if ( it != members.end() && it->first.context == cached_key.context )
      return generate_iterator
         ( it->second.src_it, 
           IteratorData(it, CachedKey(cached_key.context)) );
   else
      return generate_iterator
         ( extract_iterator(core_cache->end()), 
           IteratorData(members.end(), CachedKey(cached_key.context)) );
}


void
View_CommonBase::
increment_iterator(iterator::_base_ptr &it, utilib::Any &it_data) const
{
   const IteratorData &base = it_data.expose<IteratorData>();
   key_map_t::const_iterator tmp = base.it;

   // perform explicit bounds checking here...
   if ( tmp != members.end() )
      ++tmp;

   if ( tmp != members.end() && tmp->first == base.key )
      it = tmp->second.src_it;
   else
   {
      it = extract_iterator(core_cache->end());
      tmp = members.end();
   }

   it_data = IteratorData(tmp, base.key);
}


void
View_CommonBase::
decrement_iterator(iterator::_base_ptr &it, utilib::Any &it_data) const
{
   // The following assumes a behavior that decrementing begin() leaves
   // the iterator pointing to end().  This seems like rational behavior
   // to support as we need to ensure that the iterator is always
   // pointing to something that we "know about" (either a valid node or
   // end()); However, it is worth noting that this behavior deviates
   // from the STL standard, which does not specify behavior for
   // incrementing past end() or decrementing before begin().

   const IteratorData &base = it_data.expose<IteratorData>();
   key_map_t::const_iterator tmp = base.it;

   if ( tmp == members.end() )
      tmp = members.upper_bound(base.key);
   
   if ( tmp == members.begin() )
      tmp = members.end();
   else
      --tmp;
   
   if ( tmp != members.end() && tmp->first == base.key )
      it = tmp->second.src_it;
   else
   {
      it = extract_iterator(core_cache->end());
      tmp = members.end();
   }

   it_data = IteratorData(tmp, base.key);
}


void 
View_CommonBase::
drop_connections()
{
   while ( ! connections.empty() )
   {
      connections.back().disconnect();
      connections.pop_back();
   }
}



} // namespace cache
} // namespace colin
