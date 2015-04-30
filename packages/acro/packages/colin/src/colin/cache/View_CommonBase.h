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
 * \file View_CommonData.h
 *
 * Defines common data representations used by cache views
 */

#ifndef colin_cache_View_CommonData_h
#define colin_cache_View_CommonData_h

#include <acro_config.h>
#include <colin/Cache.h>

namespace colin {
namespace cache {

// A common base class for managing View-related data
class View_CommonBase : public VirtualCache
{
public:
   virtual ~View_CommonBase()
   {
      drop_connections();
   }

   /// Returns the number of elements in the cache
   virtual size_t size(const Application_Base *context = NULL) const;

   virtual iterator begin(const Application_Base *context = NULL) const;

   virtual iterator end() const;
   
   /// Annotate an item in the cache
   virtual void
   annotate( iterator pos, std::string attribute, utilib::Any value );

   /// Remove an item's annotation (an empty attribute name removes all)
   virtual size_t
   erase_annotation( iterator pos, std::string attribute = "" );

protected:
   struct ViewData {
      ViewData( Cache::cache_t::iterator src_it_, utilib::Any data_ )
         : src_it(src_it_), view_data(data_)
      {}
      ViewData( Cache::cache_t::iterator src_it_ )
         : src_it(src_it_), view_data()
      {}

      Cache::cache_t::iterator  src_it;
      utilib::Any  view_data;
   };      

   typedef std::map<Cache::cache_t::key_type, ViewData>  member_map_t;

   typedef member_map_t  key_map_t;

   struct IteratorData {
      IteratorData(key_map_t::const_iterator it_, Cache::CachedKey key_)
         : it(it_), key(key_)
      {}

      key_map_t::const_iterator it;
      Cache::CachedKey key;
   };


   /// Return an iterator to the subset of elements matching the CachedKey
   virtual iterator find_impl( CachedKey key ) const;

   /// Return an iterator to the first element whose CachedKey is >= key
   virtual iterator lower_impl( CachedKey key ) const;

   /// Return an iterator to the first element whose CachedKey is > key
   virtual iterator upper_impl( CachedKey key ) const;

   virtual void 
   increment_iterator(iterator::_base_ptr &it, utilib::Any &it_data) const;

   virtual void 
   decrement_iterator(iterator::_base_ptr &it, utilib::Any &it_data) const;


   void drop_connections();


   member_map_t  members;
   //sizeMap_t  cache_sizes;
   std::list<boost::signals::connection> connections;
};

} // namespace cache
} // namespace colin

#endif // defined colin_cache_View_CommonData_h
