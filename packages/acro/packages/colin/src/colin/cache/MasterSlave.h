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
 * \file MasterSlave.h
 *
 * Defines the colin::MasterSlave class.
 */

#ifndef colin_cache_MasterSlave_h
#define colin_cache_MasterSlave_h

#include <acro_config.h>

#include <colin/Cache.h>

namespace colin {
namespace cache {

/**
 *  The class \c MasterSlave provides a basic cache that stores
 *  the cache on a master process and mirrors it out to the remote processes.
 */
class MasterSlave : public ConcreteCache
{
public:

   /// Constructor
   MasterSlave();

   /// Destructor
   virtual ~MasterSlave();

   /// Set the KeyGenerator to use to determine cache hits/misses
   virtual void set_key_generator(utilib::SharedPtr<KeyGenerator> new_keygen);

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

   /// Generate a set of Core AppResponses that match the given Context & Key
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

#endif // defined colin_cache_MasterSlave_h
