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
 * \file cache/Local.h
 *
 * Defines the colin::cache::Local class.
 */

#ifndef colin_cache_Local_h
#define colin_cache_Local_h

#include <acro_config.h>

#include <colin/Cache.h>

namespace colin {
namespace cache {

/**
 *  The class \c Local provides a basic cache that stores
 *  the cache on the local processor.  This is fundamental cache for
 *  single-process applications.
 */
class Local : public ConcreteCache
{
public:

   /// Constructor
   Local();

   /// Destructor
   virtual ~Local();

   /// Returns the number of elements in the cache
   virtual size_t size(const Application_Base *context = NULL) const;

   /// Removes all points from the cache
   virtual void clear(const Application_Base *context = NULL);

   virtual iterator begin(const Application_Base *context = NULL) const;

   virtual iterator end() const;

   /// Annotate an item in the cache
   virtual void
   annotate( iterator pos, std::string attribute, utilib::Any value );

   /// Remove an item's annotation (an empty attribute name removes all)
   virtual size_t
   erase_annotation( iterator pos, std::string attribute = "" );

protected:

   /// Insert into the cache, return an iterator to the inserted element
   /// and a bool indicating if an insertion took place.
   virtual std::pair<iterator, bool> 
   insert_impl( const Application_Base *context, 
                const Key& key, const CoreResponseInfo &value );

   /// Remove a key from the cache
   virtual size_t erase_item(CachedKey cache_key);

   /// Remove an element ponted to by pos from the cache
   virtual void erase_iterator( iterator pos );

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

private:
   ///
   struct Data;
   ///
   Data* data;
};


} // namespace colin::cache
} // namespace colin

#endif // defined colin_cache_Local_h
