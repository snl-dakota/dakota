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
 * \file cache/View_Pareto.h
 *
 * Defines the colin::cache::View_Pareto class.
 */

#ifndef colin_cache_View_Pareto_h
#define colin_cache_View_Pareto_h

#include <colin/cache/View_CommonBase.h>

namespace colin {
namespace cache {

class View_Pareto : public View_CommonBase
{
public:
   View_Pareto();

   ///
   virtual void set_core_cache(CacheHandle src);

   /// Removes all points from the cache
   virtual void clear(const Application_Base *context = NULL);

   /// Boolean: true if enforcing strong dominance; false if weak
   utilib::Property strong;
   /// The application context to use to generate the objective values
   utilib::Property application_context;

   typedef std::vector<double>  objective_t;

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

private:
   ///
   void cb_clear(const Application_Base *context = NULL);

   void cb_rebuild( const utilib::ReadOnly_Property& );

   void cb_insert(cache_t::iterator it);

   void cb_update(cache_t::iterator it, AppResponse::response_map_t &fields);

   void cb_erase(cache_t::iterator it);

   void cb_annotate(cache_t::iterator it, std::string attr, utilib::Any value);

   void cb_erase_annotation(cache_t::iterator it, std::string attr);

private:
   const Application_Base*  core_context;
   objective_t  sense;
   std::vector<objective_t>  so_optima;
};

} // namespace colin::cache
} // namespace colin

#endif // defined colin_cache_View_Pareto_h
