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
 * \file cache/Factory.h
 *
 * Defines the colin::cache::Factory class.
 */

#ifndef colin_cache_Factory_h
#define colin_cache_Factory_h

#include <acro_config.h>

#include <colin/Cache.h>

namespace colin {

namespace cache { class Factory; }
///
cache::Factory& CacheFactory();

namespace cache {

/// The registered name for the default inter-solver cache
extern const std::string ImplicitInterSolverCacheName;


class Factory
{
public:
   ///
   typedef CacheHandle(*cache_create_fcn_t)();
   ///
   typedef CacheHandle(*view_create_fcn_t)(CacheHandle);
   ///
   typedef Cache::KeyGenerator*(*indexer_create_fcn_t)();

   ///
   Factory();

   ///
   ~Factory();

   ///
   void set_default_cache_type(std::string name);

   ///
   void set_default_indexer_type(std::string name);

   ///
   bool declare_cache_type(std::string name, cache_create_fcn_t fcn);

   ///
   bool declare_view_type(std::string name, view_create_fcn_t fcn);

   ///
   bool declare_indexer_type(std::string name, indexer_create_fcn_t fcn);


   ///
   CacheHandle create(std::string type = "", std::string indexer = "");

   ///
   CacheHandle 
   create_view(std::string type, CacheHandle core_cache = CacheHandle());

   ///
   CacheHandle get_cache(std::string name);

   ///
   CacheHandle register_cache(CacheHandle cache, std::string name);

   ///
   void unregister_cache(std::string name);

   ///
   CacheHandle& evaluation_cache();

   ///
   CacheHandle& intersolver_cache();

private:
   ///
   struct Data;
   ///
   Data *data;
};


} // namespace colin::cache
} // namespace colin

#endif // defined colin_cache_Factory_h
