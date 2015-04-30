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
 * \file Cache.h
 *
 * Defines the colin::Cache class.
 */

#ifndef colin_Cache_h
#define colin_Cache_h

#include <acro_config.h>

#include <colin/Handle.h>
#include <colin/ResponseGenerator.h>

#include <utilib/Any.h>
#include <utilib/SharedPtr.h>
#include <utilib/PropertyDict.h>

#include <boost/signals.hpp>

class TiXmlElement;

namespace colin
{
class Application_Base;
class Cache;

/// Define a handle to a (derived) Cache class
typedef Handle<Cache> CacheHandle;

/** Exception thrown by the Cache system 
 */
class cache_error : public std::runtime_error
{
public:
   /// Constructor
   cache_error(const std::string& msg) 
      : runtime_error(msg)
   {}
};

/** Exception thrown by the Cache system 
 */
class view_error : public cache_error
{
public:
   /// Constructor
   view_error(const std::string& msg) 
      : cache_error(msg)
   {}
};


struct CacheableObjectTest
{
   CacheableObjectTest()
      : cacheable(true)
   {}

   bool cacheable;
};

/**
 * The class \c Cache provides a class for managing the
 * interface for caching computed application responses.
 *
 * Some developer notes:
 *
 *  - we should strive to present a standards-compliant implementation
 *    of an STL container.
 *
 *  - we assume that there is a 1-to-1 mapping among *cacheable* keys to
 *    CoreResponseInfo objects.  However, in order to support sloppy
 *    matching (especially across seeds for stochastic domains), keys do
 *    not necessarily have a 1-to-1 mapping with responses.  That is,
 *    multiple keys may map to the same Response, as long as only ONE of
 *    those keys is cacheable.
 */
class Cache : public ResponseGenerator, 
              public Handle_Client<Cache>
{
public:
   class KeyGenerator;

   class Key
   {
      friend class KeyGenerator;
   public:
      Key() : key()
      {}

      Key(const Key& rhs)
      {
         *this = rhs;
      }

      bool operator==(const Key& rhs) const
      { return ( key == rhs.key ); }
      bool operator<(const Key& rhs) const
      { return ( key < rhs.key ); }
      bool empty() const
      { return key.empty(); }
      bool Cacheable() const;

   private:
      Key( utilib::Any key_ )
         : key(key_)
      {}

      utilib::Any key;
   };

   /// A functor for converting the domain into an index key
   class KeyGenerator
   {
   public:
      virtual ~KeyGenerator() {}
      virtual Key operator()(const utilib::Any domain) = 0;

   protected:
      Key form_key(utilib::Any key)
      {
         return Key(key);
      }
   };

   class CachedKey
   {
   public:
      CachedKey()
         : context(NULL), key()
      {}
      CachedKey(const Application_Base* context_)
         : context(context_), key()
      {}
      CachedKey(const Application_Base* context_, Key key_)
         : context(context_), key(key_)
      {}

      bool operator==(const CachedKey& rhs) const
      {
         if ( context == NULL || rhs.context == NULL )
            return true;
         if ( context != rhs.context )
            return false;
         return key.empty() || rhs.key.empty() || key == rhs.key;
      }

      bool operator!=(const CachedKey& rhs) const
      {
         return ! this->operator==(rhs);
      }

      bool operator<(const CachedKey& rhs) const
      {
         if ( context == NULL || rhs.context == NULL )
            return false;
         if ( context < rhs.context )
            return true;
         if ( context > rhs.context )
            return false;
         if ( key.empty() || rhs.key.empty() )
            return false;
         return key < rhs.key;
      }

      const Application_Base*  context;
      Cache::Key       key;
   };
   
   class CachedData
   {
   public:
      typedef std::map<std::string, utilib::Any> annotation_map_t;
      
      CachedData()
         : domain(), responses()
      {}
      
      CachedData(CoreResponseInfo &cri)
         : domain(cri.domain), responses(cri.responses)
      {}
      
      AppResponse asResponse(const AppRequest &request) const;
      AppResponse asResponse(const Application_Base* context) const;
      utilib::Any annotation(const std::string &name) const;
      
      utilib::Any                  domain;
      AppResponse::response_map_t  responses;
      annotation_map_t             annotations;
      std::list<utilib::Any>       eval_ids;
   };


   typedef std::map<CachedKey, CachedData>  cache_t;

   template<typename INNER>
   struct iterator_template {
      friend class Cache;

      typedef INNER  _base_ptr;
      typedef typename _base_ptr::value_type  value_type;
      typedef const typename _base_ptr::value_type&  reference;
      typedef const typename _base_ptr::value_type*  pointer;

      typedef std::bidirectional_iterator_tag iterator_category;
      typedef std::ptrdiff_t                  difference_type;

      typedef iterator_template<INNER>  _self;

      iterator_template()
         : it(), cache(NULL), data() {}

      iterator_template(const iterator_template& rhs)
         : it(rhs.it), cache(rhs.cache), data(rhs.data) {}

      iterator_template&
      operator=(const iterator_template& rhs)
      {
         if ( this != &rhs )
         {
            it = rhs.it;
            cache = rhs.cache;
            data = rhs.data;
         }
         return *this;
      }

      reference
      operator*() const
      {
         return *it;
      }

      pointer
      operator->() const
      {
         return &*it;
      }

      _self&
      operator++()
      {
         cache->increment_iterator(it, data);
         return *this;
      }

      _self
      operator++(int)
      {
         _self tmp = *this;
         cache->increment_iterator(it, data);
         return tmp;
      }

      _self&
      operator--()
      {
         cache->decrement_iterator(it, data);
         return *this;
      }

      _self
      operator--(int)
      {
         _self tmp = *this;
         cache->decrement_iterator(it, data);
         return tmp;
      }

      bool
      operator==(const _self& _x) const
      { return it == _x.it; }

      bool
      operator!=(const _self& _x) const
      { return it != _x.it; }

   private:
      explicit
      iterator_template( _base_ptr _x, 
                         const Cache *_owner, 
                         utilib::Any _data )
         : it(_x), cache(_owner), data(_data) {}

      _base_ptr  it;
      const Cache  *cache;
      utilib::Any  data;
   };

   typedef iterator_template< cache_t::iterator >  iterator;

   typedef iterator_template< cache_t::const_iterator >  const_iterator;

public:

   /// Constructor
   Cache()
   {}

   /// Destructor
   virtual ~Cache()
   {}

   virtual void set_key_generator(utilib::SharedPtr<KeyGenerator> keygen_) = 0;

   virtual Key generate_key(const utilib::Any domain) const = 0;

   /// Returns the number of keys in the cache
   virtual size_t size(const Application_Base *context = NULL) const = 0;

   /// Removes all data from the cache
   virtual void clear(const Application_Base *context = NULL) = 0;

   virtual iterator begin(const Application_Base *context = NULL) const = 0;

   virtual iterator end() const = 0;

   /// Annotate an item in the cache
   virtual void
   annotate( iterator pos, std::string attribute, utilib::Any value ) = 0;

   /// Remove an item's annotation (an empty attribute name removes all)
   virtual size_t
   erase_annotation( iterator pos, std::string attribute = "" ) = 0;


   /// Returns the number of keys in the cache
   bool empty(const Application_Base *context = NULL) const
   {
      return size(context) == 0;
   }

   /// Remove a CachedKey from the cache
   size_t erase(CachedKey key);

   /// Remove a key from the cache
   size_t erase(const Application_Base *context, Key key = Key());

   /// Remove a domain point from the cache
   size_t erase(const Application_Base *context, const utilib::Any domain);

   /// Remove an element ponted to by pos from the cache
   void erase( iterator pos );

   /// Insert into the cache, return an iterator to the inserted element
   /// and a bool indicating if an insertion took place.
   std::pair<iterator, bool> 
   insert( const Application_Base *context, 
           const Key& key, const CoreResponseInfo &value );

   /// Convenience: Insert an AppResponse into the cache
   std::pair<iterator, bool> 
   insert(const Key key, const AppResponse response);

   /// Convenience: Insert an AppResponse (calls insert(key, response))
   std::pair<iterator, bool> 
   insert(const AppResponse response);

   /// Convenience: Insert an iterator from another Cache
   std::pair<iterator, bool> 
   insert(iterator it);

   /// Convenience: Insert an iterator from another Cache
   std::pair<iterator, bool> 
   insert(const_iterator it);

   /// Return an iterator to the element whose Key is key
   iterator find( CachedKey key) const;

   /// Convenience: Return an iterator to the element whose Key is key
   iterator find(const Application_Base *context, Key key) const;

   /// Convenience: Return cache iterator and key matching the CoreRequestInfo
   std::pair<iterator, Key> find( const CoreRequestInfo &cqi ) const;

   /// Convenience: Return a cache iterator and key matching the domain
   std::pair<iterator, Key> find( const Application_Base *context, 
                                  const utilib::Any domain ) const;

   /// Return an iterator to the first element whose CachedKey is >= key
   iterator lower_bound( CachedKey key) const;

   /// Convenience: Return an iterator to the first element whose Key is >= key
   iterator lower_bound(const Application_Base *context, Key key) const;

   /// Convenience: Return a cache iterator, key to the first element >= domain
   std::pair<iterator, Key> lower_bound( const Application_Base *context, 
                                         const utilib::Any domain ) const;

   /// Return an iterator to the first element whose CachedKey is > key
   iterator upper_bound( CachedKey key) const;

   /// Convenience: Return an iterator to the first element whose Key is > key
   iterator upper_bound(const Application_Base *context, Key key) const;

   /// Convenience: Return a cache iterator, key to the first element > domain
   std::pair<iterator, Key> upper_bound( const Application_Base *context, 
                                         const utilib::Any domain ) const;


   /// get a reference to the named property
   utilib::Property& property( std::string name );

   /// Return a copy of the the underlying property dictionary
   utilib::PropertyDict Properties()
   { return properties; }


   /// signal called when a new point is added to this cache
   boost::signal<void(cache_t::iterator)>  onInsert;

   /// signal called when an existing point has new data added to it
   boost::signal<void( cache_t::iterator,
                       AppResponse::response_map_t& )>  onUpdate;

   /// signal called just before a point is removed from the cache
   boost::signal<void(cache_t::iterator)>  onErase;

   /// signal called just before an application context is cleared
   boost::signal<void(const Application_Base*)>  onClear;

   /// signal called when an annotation is created or updated
   boost::signal<void(cache_t::iterator, std::string, utilib::Any)> onAnnotate;

   /// signal called when an annotation is erased
   boost::signal<void(cache_t::iterator, std::string)>  onEraseAnnotation;

protected:

   /// The actual virtual routine for inserting data into the cache
   /** [must be named differently to prevent masking the overloaded
    *  versions of insert() in derived classes]
    */
   virtual std::pair<iterator, bool> 
   insert_impl( const Application_Base *context, 
           const Key& key, const CoreResponseInfo &value ) = 0;

   /// Remove a key from the cache
   virtual size_t erase_item( CachedKey cache_key ) = 0;

   /// Remove an element ponted to by pos from the cache
   virtual void erase_iterator( iterator pos ) = 0;

   /// Return an iterator to the subset of elements matching the CachedKey
   virtual iterator find_impl( CachedKey key ) const = 0;

   /// Return an iterator to the first element whose CachedKey is >= key
   virtual iterator lower_impl( CachedKey key ) const = 0;

   /// Return an iterator to the first element whose CachedKey is > key
   virtual iterator upper_impl( CachedKey key ) const = 0;

   virtual void 
   increment_iterator(iterator::_base_ptr &it, utilib::Any &it_data) const = 0;

   virtual void 
   decrement_iterator(iterator::_base_ptr &it, utilib::Any &it_data) const = 0;

   iterator generate_iterator(iterator::_base_ptr it_, utilib::Any data) const
   { 
      return iterator(it_, this, data);
   }

   iterator::_base_ptr extract_iterator(iterator it_) const
   { 
      return it_.it;
   }

protected:
   /// All properties registered by derived classes
   utilib::Privileged_PropertyDict properties;
  
};


class ConcreteCache : public Cache
{
public:

   /// Constructor
   ConcreteCache()
      : keygen()
   {}

   /// Destructor
   virtual ~ConcreteCache()
   {}

   /// Set the KeyGenerator to use to determine cache hits/misses
   virtual void set_key_generator(utilib::SharedPtr<KeyGenerator> keygen_);

   ///
   virtual Key generate_key(const utilib::Any domain) const
   {
      if ( domain.empty() )
         return Key();

      assert( !keygen.empty() && "Cache missing a required KeyGenerator");
      return (*keygen)(domain);
   }

private:
   ///
   utilib::SharedPtr<KeyGenerator> keygen;
};


class VirtualCache : public Cache
{
public:

   /// Constructor
   VirtualCache()
   {}

   /// Destructor
   virtual ~VirtualCache()
   {}

   /// Set the KeyGenerator to use to determine cache hits/misses
   virtual void set_key_generator(utilib::SharedPtr<KeyGenerator> keygen_)
   {
      core_cache->set_key_generator(keygen_);
   }

   ///
   virtual Key generate_key(const utilib::Any domain) const
   {
      return core_cache->generate_key(domain);
   }

   virtual void set_core_cache(CacheHandle src) = 0;

protected:
   ///
   CacheHandle core_cache;
};


} // namespace colin

#endif // defined colin_Cache_h
