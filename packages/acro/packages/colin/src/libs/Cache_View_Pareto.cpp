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
 * \file View_Pareto.cpp
 *
 * Implements the colin::View_Pareto class.
 */

#include <colin/cache/View_Pareto.h>
#include <colin/cache/Factory.h>
#include <colin/application/Base.h>
#include <colin/ProblemTraits.h>

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

/// Function to create a new View_Pareto
CacheHandle create_view_pareto(CacheHandle core_cache)
{
   std::pair<CacheHandle, cache::View_Pareto*> tmp
      = CacheHandle::create<cache::View_Pareto>();
   tmp.second->set_core_cache(core_cache);
   return tmp.first;
}

bool RegisterView_ParetoCache()
{
   CacheFactory().declare_view_type("Pareto", create_view_pareto);
   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool pareto_view = RegisterView_ParetoCache();

} // namespace colin::StaticInitializers



namespace cache {

namespace {

// bit 1: true iif strong dominance
// bit 2: true iif a dominates b
// bit 3: true iif b dominates a
//                                       000 001 010 011 100 101 110 111
static const int dominates_result[8] = {   0,  0,  1,  1, -1, -1,  0,  0 };

// returns 1 if a dominates b, -1 if b dominates a; otherwise 0.
inline int
dominates( const cache::View_Pareto::objective_t &a, 
           const cache::View_Pareto::objective_t &b,  
           bool force_strong )
{
   size_t i = a.size();
   if ( i != b.size() )
      EXCEPTION_MNGR(std::runtime_error, "colin::(local)::dominates() "
                     "passed vectors of unequal size.");

   int ans = 7;
   while ( i > 0 )
   {
      --i;
      ans &= ( a[i] == b[i] ? 6 : ( a[i] < b[i] ? 3 : 5 ) );
   }

   if (( force_strong ) && ( ans & 1 == 0 ))
      ans = 0;
   return dominates_result[ans];
}

} // namespace colin::cache::(local)

//================================================================
// View_Pareto Public Member Definitions
//================================================================

View_Pareto::View_Pareto()
   : strong(utilib::Property::Bind<bool>(true)),
     application_context(utilib::Property::Bind<ApplicationHandle>()),
     core_context(NULL), 
     sense(), 
     so_optima()
{
   strong.onChange()
      .connect(bind(&View_Pareto::cb_rebuild, this, _1));
   application_context.onChange()
      .connect(bind(&View_Pareto::cb_rebuild, this, _1));

   properties.declare("strong_dominance", 
                      "Use strong (true) or weak (false) Pareto Dominance",
                      strong);
   properties.declare("application_context", 
                      "The application context to use to determine "
                      "objective values and feasibility",
                      application_context);
}



void 
View_Pareto::
cb_rebuild(const utilib::ReadOnly_Property&)
{
   // First, notify any dependent views that things are changing
   cb_clear(NULL);

   ApplicationHandle context = application_context;
   core_context = get_core_application(context.object());
   sense.clear();
   so_optima.clear();

   if ( ! context.empty() && ! core_cache.empty() )
   {
      std::vector<optimizationSense> tmp = context->property("sense");
      sense.resize(tmp.size());
      for( size_t i=0; i<tmp.size(); ++i )
         sense[i] = ( tmp[i] == maximization ? -1.0 : 1.0 );
      
      so_optima.resize(tmp.size());

      Cache::iterator it = core_cache->begin(context.object());
      Cache::iterator itEnd = core_cache->end();
      for( ; it != itEnd; ++it )
         cb_insert(extract_iterator(it));
   }
}


void View_Pareto::set_core_cache(CacheHandle src)
{
   if ( src.empty() )
      EXCEPTION_MNGR(view_error, "View_Pareto::set_core_cache(): "
                     "Cannot form a view into a nonexistent Cache");

   drop_connections();
   core_cache = src;

   if ( core_cache->size() > 0 &&  
        application_context.as<ApplicationHandle>().empty() )
   {
      // arbitrarily pick a context
      application_context = core_cache->begin()->first.context->get_handle();
   }
   else
      cb_rebuild(application_context);

   if ( core_cache.empty() )
   {
      return;
   }

   // Clearing the core cache should clear this cache
   connections.push_back
      ( core_cache->onClear.connect
        ( boost::bind(&View_Pareto::cb_clear, this, _1) ) );

   // Catch any insertions so we can update the Pareto fontier
   connections.push_back
      ( core_cache->onInsert.connect
        ( boost::bind(&View_Pareto::cb_insert, this, _1) ) );

   // Catch any insertions so we can update the Pareto fontier
   connections.push_back
      ( core_cache->onUpdate.connect
        ( boost::bind(&View_Pareto::cb_update, this, _1, _2) ) );

   // Catch any erase events that affect points in this view
   connections.push_back
      ( core_cache->onErase.connect
        ( boost::bind(&View_Pareto::cb_erase, this, _1) ) );

   // Pass on all annotation events for points in this view to any clients
   connections.push_back
      ( core_cache->onAnnotate.connect
        ( boost::bind(&View_Pareto::cb_annotate, this, _1, _2, _3) ) );
   connections.push_back
      ( core_cache->onEraseAnnotation.connect
        ( boost::bind(&View_Pareto::cb_erase_annotation, this, _1, _2) ) );
}


void View_Pareto::clear( const Application_Base* context )
{
   EXCEPTION_MNGR(view_error, "View_Pareto::clear(): "
                  "Membership in the Pareto set is determined by all data "
                  "in the underlying cache.  You cannot remove elements "
                  "from the View.  Instead, remove them from the "
                  "underlying cache.");
}


std::pair<Cache::iterator, bool>
View_Pareto::insert_impl( const Application_Base *context,
                          const Key& key,
                          const CoreResponseInfo& response )
{
   if ( ! core_context )
      application_context = ApplicationHandle(context);

   bool already_pareto = 
      members.find(CachedKey(context, key)) != members.end();

   std::pair<Cache::iterator, bool> raw_inserted = 
      core_cache->insert(context, key, response);

   cache_t::iterator it = extract_iterator(raw_inserted.first);
   key_map_t::iterator pareto_it = members.find(it->first);
   if ( pareto_it == members.end() )
      return make_pair
         ( generate_iterator( extract_iterator(core_cache->end()),
                              IteratorData(pareto_it, it->first) ),
           false );

   if ( ! already_pareto )
      onInsert(pareto_it->second.src_it);

   return make_pair
      ( generate_iterator(it, IteratorData(pareto_it, it->first)), 
        ! already_pareto );
}


size_t
View_Pareto::erase_item( CachedKey key )
{
   EXCEPTION_MNGR(view_error, "View_Pareto::erase_item(): "
                  "Membership in the Pareto set is determined by all data "
                  "in the underlying cache.  You cannot remove elements "
                  "from the View.  Instead, remove them from the "
                  "underlying cache.");
   return 0;
}


void
View_Pareto::erase_iterator( Cache::iterator pos )
{
   EXCEPTION_MNGR(view_error, "View_Pareto::erase_iterator(): "
                  "Membership in the Pareto set is determined by all data "
                  "in the underlying cache.  You cannot remove elements "
                  "from the View.  Instead, remove them from the "
                  "underlying cache.");
}


void 
View_Pareto::cb_clear( const Application_Base* context )
{
   if ( context && core_context != context )
      return;

   onClear(context);
   members.clear();
   for ( size_t i = 0; i < so_optima.size(); ++i )
      so_optima[i].clear();
}


void 
View_Pareto::cb_insert(cache_t::iterator src)
{
   if ( ! core_context )
      application_context = ApplicationHandle(src->first.context);

   //cerr << "View_Pareto::cb_insert: adding point" << endl;
   if ( src->first.context != core_context )
      return;
   
   ApplicationHandle appHandle = application_context;
   AppResponse response = src->second.asResponse( appHandle.object() );
   // if ( context->is_feasible(response) )
   //    return;
   //cerr << response << endl;

   //
   // FIXME: This is a hack to allow Pareto to work on single objective
   // problems.  We should really fix this and only support f_info (and
   // not mf_info requests).
   //
   unsigned long info_type = 
      ( appHandle->problem_type() & ProblemTraitValue_multiple_objectives ) 
      ? mf_info : f_info;
   if ( ! response.is_computed(info_type) )
      return;

   objective_t obj;
   utilib::TypeManager()->lexical_cast(response.get(info_type), obj);
   size_t nObj = obj.size();

   //cerr << "processing obj = " << obj;

   // Convert to minimization
   for ( size_t i = 0; i < nObj; ++i )
      obj[i] *= sense[i];

   //cerr << "; as minimizer: " << obj << endl;

   // first, check (and update, if necessary) the global optima
   for ( size_t i = 0; i < nObj; ++i )
   {
      if ( so_optima[i].empty() )
      {
         //cerr << "  (initializing global opt for obj " << i << ")" << endl; 
         so_optima[i] = obj;
      }

      // If a single-objective optimum dominates this point, we are done
      if ( dominates(so_optima[i], obj, strong) > 0 )
      {
         //cerr << "  -> dominated by " << so_optima[i] << endl; 
         return;
      }

      if ( obj[i] < so_optima[i][i] )
      {
         //cerr << "  (new global opt for obj " << i << ")" << endl;
         so_optima[i] = obj;
      }
   }

   // Scan through all Pareto points and update
   key_map_t::iterator it = members.begin();
   while ( it != members.end() )
   {
      int is_pareto = dominates( it->second.view_data.expose<objective_t>(), 
                                 obj, strong );
      if ( is_pareto > 0 )
      {
         //cerr << "  -> dominated by " << it->second.obj << endl;
         return;
      }
      if ( is_pareto < 0 )
      {
         //cerr << "  -> dominates " << it->second.obj << " (removing)" << endl;
         onErase(it->second.src_it);
         members.erase(it++);
         continue;
      }
      ++it;
   }

   //cerr << "  -> new Pareto point." << endl;

   // if we got this far, this point is Pareto
   it = members.insert
      ( make_pair(src->first, ViewData(src, obj)) ).first;
   onInsert(src);
}


void 
View_Pareto::
cb_update( cache_t::iterator it, AppResponse::response_map_t &fields )
{
   cb_insert(it);
}


void 
View_Pareto::cb_erase(cache_t::iterator it)
{
   if ( it->first.context != core_context )
      return;

   key_map_t::iterator tmp = members.find(it->first);
   if ( tmp == members.end() || tmp->second.src_it != it )
      return;

   //cerr << "Erasing " << tmp->second.obj << endl;

   // first, check (and clear, if necessary) the global optima
   const objective_t& obj_data = tmp->second.view_data.expose<objective_t>();
   for ( size_t i = 0; i < obj_data.size(); ++i )
   {
      if ( obj_data[i] == so_optima[i][i] )
      {
         //cerr << "  (clearing global opt for obj " << i << ")" << endl;
         so_optima[i].clear();
      }
   }

   onErase(it);
   members.erase(tmp);

   // Rebuild the region of the front behind the removed element
   // FIXME: This is horribly inefficient.  A more intelligent data
   // structure would go a long way toward making this process bearable.
   Cache::iterator c_it = core_cache->begin(core_context);
   Cache::iterator c_itEnd = core_cache->end();
   for( ; c_it != c_itEnd; ++c_it )
   {
      if ( extract_iterator(c_it) != it )
         cb_insert(extract_iterator(c_it));
   }
}


void
View_Pareto::
cb_annotate(cache_t::iterator it, std::string attr, utilib::Any value)
{
   key_map_t::iterator tmp = members.find(it->first);
   if ( tmp != members.end() && tmp->second.src_it == it )
      onAnnotate(it, attr, value);
}


void 
View_Pareto::cb_erase_annotation(cache_t::iterator it, std::string attr)
{
   key_map_t::iterator tmp = members.find(it->first);
   if ( tmp != members.end() && tmp->second.src_it == it )
      onEraseAnnotation(it, attr);
}



} // namespace colin::cache
} // namespace colin
