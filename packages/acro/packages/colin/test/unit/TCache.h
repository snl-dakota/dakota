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

#include <acro_config.h>

#include <colin/cache/Factory.h>
#include <colin/cache/Local.h>
#include <colin/cache/KeyGenerator_EpsilonMatch.h>
#include <colin/reformulation/ConstraintPenalty.h>
#include <colin/ApplicationMngr.h>
#include <colin/ExecuteMngr.h>

#include <utilib/OStreamTee.h>

#include <cxxtest/TestSuite.h>

#include "TestApplications.h"
#include "CommonTestUtils.h"

namespace colin {
namespace unittest { class Cache_Local; }

class colin::unittest::Cache_Local : public CxxTest::TestSuite
{
public:
   Cache_Local()
   {
      app.set_application
         ( ApplicationHandle::create
           < TestApplications::singleObj_denseCon<UNLP1_problem> >() )
         ->configure(1);

      mo_app.set_application
         ( ApplicationHandle::create
           < TestApplications::multiObj_denseCon<MO_NLP1_problem> >() )
         ->configure(1);
   }

   void setUp()
   {
      cache = CacheFactory().create("Local");
   }

   void tearDown()
   { }

   void test_insert_and_size()
   {
      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);

      cache->insert(app->EvalF(5));
      TS_ASSERT_EQUALS(cache->size(), 1);
      TS_ASSERT_EQUALS(cache->size(&*app), 1);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);

      cache->insert(mo_app->EvalMF(5));
      TS_ASSERT_EQUALS(cache->size(), 2);
      TS_ASSERT_EQUALS(cache->size(&*app), 1);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);

      cache->insert(mo_app->EvalMF(10));
      TS_ASSERT_EQUALS(cache->size(), 3);
      TS_ASSERT_EQUALS(cache->size(&*app), 1);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 2);
   }

   struct buffer_update {
      buffer_update()
         : call_count(0), size(0)
      {}

      void update( Cache::cache_t::iterator, 
                   AppResponse::response_map_t &response )
      {
         ++call_count;
         size = response.size();
      }

      size_t call_count;
      size_t size;
   };

   void test_augmented_insert()
   {
      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);

      buffer_update update;
      cache->onUpdate.connect(boost::bind( &buffer_update::update, &update, 
                                           _1, _2 ));
      std::pair<Cache::iterator, bool> ref, tmp;

      ref = cache->insert(app->EvalF(5));
      TS_ASSERT_DIFFERS(ref.first, cache->end());
      TS_ASSERT_EQUALS(ref.first, cache->begin());
      TS_ASSERT( ref.second );

      TS_ASSERT( cache->insert(app->EvalF(10)).second );
      TS_ASSERT( cache->insert(mo_app->EvalMF(5)).second );

      TS_ASSERT_EQUALS(cache->size(), 3);
      TS_ASSERT_EQUALS(cache->size(&*app), 2);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);

      // Given that the actual number of responses is a function of the
      // application, and that the application may change, we can only
      // reliably verify that the new responses get added.
      //TS_ASSERT_EQUALS( ref.first->second.responses.size(), 1 );
      size_t ref_responses = ref.first->second.responses.size();
      TS_ASSERT_EQUALS( update.call_count, 0 );
      TS_ASSERT_EQUALS( update.size, 0 );

      tmp = cache->insert(app->EvalF(5));
      TS_ASSERT( ! tmp.second );
      TS_ASSERT_EQUALS( tmp.first, ref.first );
      TS_ASSERT_EQUALS( ref_responses, tmp.first->second.responses.size() );
      TS_ASSERT_EQUALS( update.call_count, 0 );
      TS_ASSERT_EQUALS( update.size, 0 );

      tmp = cache->insert(app->EvalG(5));
      TS_ASSERT( ! tmp.second );
      TS_ASSERT_EQUALS( tmp.first, ref.first );
      TS_ASSERT_LESS_THAN( ref_responses, tmp.first->second.responses.size() );
      TS_ASSERT_EQUALS( update.call_count, 1 );
      TS_ASSERT_EQUALS( update.size, 
                        tmp.first->second.responses.size() - ref_responses );

      TS_ASSERT_EQUALS(cache->size(), 3);
      TS_ASSERT_EQUALS(cache->size(&*app), 2);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);
   }

   void test_clear()
   {
      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);

      cache->insert(app->EvalF(5));
      cache->insert(app->EvalF(10));
      cache->insert(mo_app->EvalMF(5));
      TS_ASSERT_EQUALS(cache->size(), 3);
      TS_ASSERT_EQUALS(cache->size(&*app), 2);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);

      cache->clear();

      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);

      cache->insert(app->EvalF(5));
      cache->insert(app->EvalF(10));
      cache->insert(mo_app->EvalMF(5));
      TS_ASSERT_EQUALS(cache->size(), 3);
      TS_ASSERT_EQUALS(cache->size(&*app), 2);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);

      cache->clear(&*app);
      TS_ASSERT_EQUALS(cache->size(), 1);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);

      // clearing a non-cached application
      cache->clear(&*app);
      TS_ASSERT_EQUALS(cache->size(), 1);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);

      cache->clear(&*mo_app);
      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);

      // clearing a non-cached application
      cache->clear(&*mo_app);
      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);
   }

   void test_erase_single()
   {
      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);

      cache->insert(app->EvalF(5));
      cache->insert(app->EvalF(10));
      cache->insert(mo_app->EvalMF(5));
      TS_ASSERT_EQUALS(cache->size(), 3);
      TS_ASSERT_EQUALS(cache->size(&*app), 2);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);

      TS_ASSERT_EQUALS(cache->erase(&*app, 1), 0);
      TS_ASSERT_EQUALS(cache->erase(&*mo_app, 10), 0);

      TS_ASSERT_EQUALS(cache->size(), 3);
      TS_ASSERT_EQUALS(cache->size(&*app), 2);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);

      TS_ASSERT_EQUALS(cache->erase(&*app, 5), 1);

      TS_ASSERT_EQUALS(cache->size(), 2);
      TS_ASSERT_EQUALS(cache->size(&*app), 1);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);

      TS_ASSERT_EQUALS(cache->erase(&*app, 5), 0);
      TS_ASSERT_EQUALS(cache->erase(&*app, 10), 1);
      TS_ASSERT_EQUALS(cache->erase(&*app, 5), 0);

      TS_ASSERT_EQUALS(cache->size(), 1);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);

      TS_ASSERT_EQUALS(cache->erase(&*mo_app, 5), 1);

      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);

      TS_ASSERT_EQUALS(cache->erase(&*mo_app, 5), 0);
      TS_ASSERT_EQUALS(cache->erase(&*app, 5), 0);
   }

   void test_erase_multiple()
   {
      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);

      cache->insert(app->EvalF(5));
      cache->insert(app->EvalF(10));
      cache->insert(mo_app->EvalMF(5));
      TS_ASSERT_EQUALS(cache->size(), 3);
      TS_ASSERT_EQUALS(cache->size(&*app), 2);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);

      TS_ASSERT_EQUALS(cache->erase(&*app), 2);
      TS_ASSERT_EQUALS(cache->size(), 1);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);

      cache->insert(app->EvalF(5));
      cache->insert(app->EvalF(10));
      TS_ASSERT_EQUALS(cache->size(), 3);
      TS_ASSERT_EQUALS(cache->size(&*app), 2);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);

      TS_ASSERT_EQUALS(cache->erase(Cache::CachedKey()), 3);
      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);
   }

   void test_erase_iterator()
   {
      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);

      cache->insert(app->EvalF(5));
      cache->insert(app->EvalF(10));
      cache->insert(mo_app->EvalMF(5));
      TS_ASSERT_EQUALS(cache->size(), 3);
      TS_ASSERT_EQUALS(cache->size(&*app), 2);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);

      Cache::iterator it = cache->find(&*app, 1).first;
      TS_ASSERT_EQUALS(it, cache->end());
      cache->erase(it);

      TS_ASSERT_EQUALS(cache->size(), 3);
      TS_ASSERT_EQUALS(cache->size(&*app), 2);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);

      it = cache->find(&*app, 5).first;
      cache->erase(it);
      TS_ASSERT_EQUALS(cache->size(), 2);
      TS_ASSERT_EQUALS(cache->size(&*app), 1);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);

      it = cache->find(&*app, 5).first;
      cache->erase(it);
      TS_ASSERT_EQUALS(cache->size(), 2);
      TS_ASSERT_EQUALS(cache->size(&*app), 1);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);
   }

   void test_iterators()
   {
      Cache::iterator it;

      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);
      TS_ASSERT_EQUALS(cache->begin(), cache->end());
      TS_ASSERT_EQUALS(cache->begin(&*app), cache->end());
      TS_ASSERT_EQUALS(cache->begin(&*mo_app), cache->end());

      // All iterators should point to end() - and since the cache is
      // empty, incrementing / decrementing them should also point to
      // end()
      it = cache->begin(&*app);
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_EQUALS(it, cache->begin(&*app));

      it = cache->begin(&*mo_app);
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_EQUALS(it, cache->begin(&*mo_app));


      // Add a point to the SO application
      it = cache->insert(app->EvalF(5)).first;

      TS_ASSERT_DIFFERS(it, cache->end());
      TS_ASSERT_EQUALS(it, cache->begin());
      TS_ASSERT_EQUALS(it, cache->begin(&*app));
      TS_ASSERT_DIFFERS(it, cache->begin(&*mo_app));

      TS_ASSERT_DIFFERS(cache->begin(), cache->end());
      TS_ASSERT_DIFFERS(cache->begin(&*app), cache->end());
      TS_ASSERT_EQUALS(cache->begin(&*mo_app), cache->end());

      it = cache->begin(&*app);
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_EQUALS(it, cache->begin(&*app));

      // We also want to explicitly test the iterator's != operator
      TS_ASSERT( cache->begin() != cache->end() );
      TS_ASSERT( ! ( cache->begin() != cache->begin() ) );
      TS_ASSERT( ! ( cache->end() != cache->end() ) );


      // Clear the cache
      cache->clear();

      // Add a point to the MO application
      it = cache->insert(mo_app->EvalMF(10)).first;

      TS_ASSERT_DIFFERS(it, cache->end());
      TS_ASSERT_EQUALS(it, cache->begin());
      TS_ASSERT_EQUALS(it, cache->begin(&*mo_app));
      TS_ASSERT_DIFFERS(it, cache->begin(&*app));

      TS_ASSERT_DIFFERS(cache->begin(), cache->end());
      TS_ASSERT_EQUALS(cache->begin(&*app), cache->end());
      TS_ASSERT_DIFFERS(cache->begin(&*mo_app), cache->end());

      it = cache->begin(&*mo_app);
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_EQUALS(it, cache->begin(&*mo_app));


      // re-add the first evaluation
      it = cache->insert(app->EvalF(5)).first;

      TS_ASSERT_DIFFERS(it, cache->end());
      TS_ASSERT_EQUALS(it, cache->begin(&*app));
      TS_ASSERT_DIFFERS(it, cache->begin(&*mo_app));

      TS_ASSERT_DIFFERS(cache->begin(), cache->end());
      TS_ASSERT_DIFFERS(cache->begin(&*app), cache->end());
      TS_ASSERT_DIFFERS(cache->begin(&*mo_app), cache->end());

      TS_ASSERT_DIFFERS(cache->begin(&*app), cache->begin(&*mo_app));

      it = cache->begin(&*app);
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_EQUALS(it, cache->begin(&*app));

      it = cache->begin(&*mo_app);
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_EQUALS(it, cache->begin(&*mo_app));


      // Add a second point to the SO application and test the postfix
      // operators
      cache->insert(app->EvalF(10));

      it = cache->begin(&*app);
      TS_ASSERT_EQUALS(it++, cache->begin(&*app));

      TS_ASSERT_DIFFERS(it, cache->begin(&*app));
      TS_ASSERT_DIFFERS(it, cache->end());
      ++it;

      TS_ASSERT_EQUALS(it--, cache->end());

      TS_ASSERT_DIFFERS(it, cache->begin(&*app));
      TS_ASSERT_DIFFERS(it, cache->end());

      it--;
      TS_ASSERT_EQUALS(it, cache->begin(&*app));


      // Test iterators that are not bound to an application
      it = cache->begin();
      TS_ASSERT_DIFFERS(it++, cache->end());
      TS_ASSERT_DIFFERS(it++, cache->end());
      TS_ASSERT_DIFFERS(it++, cache->end());
      TS_ASSERT_EQUALS(it--, cache->end());  
      TS_ASSERT_DIFFERS(it--, cache->end());
      TS_ASSERT_DIFFERS(it--, cache->end());
      TS_ASSERT_EQUALS(it, cache->begin());  
   }

   void test_find()
   {
      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);

      double f = 0;
      double f_5_reference = 0;
      double f_10_reference = 0;

      cache->insert(app->EvalF(5, f_5_reference));
      cache->insert(app->EvalF(10, f_10_reference));
      cache->insert(mo_app->EvalMF(5));

      // A cache hit
      Cache::iterator it = cache->find(&*app, 10).first;
      TS_ASSERT_DIFFERS(it, cache->begin(&*app));
      TS_ASSERT_DIFFERS(it, cache->end());
      f = 0;
      it->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_10_reference);

      // NB: searching for a specific cache key will create an iterator
      // that ONLY iterates over matching elements (i.e. searching for 10
      // will NOT find 5).
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_DIFFERS(it, cache->end());
      f = 0;
      it->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_10_reference);
      --it;
      TS_ASSERT_EQUALS(it, cache->end());

      // A cache miss
      it = cache->find(&*app, 7).first;
      TS_ASSERT_EQUALS(it, cache->end());

      // NB: searching for a specific cache key will create an iterator
      // that ONLY iterates over matching elements (i.e. searching for 10
      // will NOT find 5).
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_EQUALS(it, cache->end());
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
   }

   void test_lower_bound()
   {
      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);

      Cache::iterator it = cache->lower_bound(&*app, 7).first;
      TS_ASSERT_EQUALS( it, cache->end() );

      double f = 0;
      double f_5_reference = 0;
      double f_10_reference = 0;

      cache->insert(app->EvalF(5, f_5_reference));
      cache->insert(app->EvalF(10, f_10_reference));

      it = cache->lower_bound(&*mo_app, 7).first;
      TS_ASSERT_EQUALS( it, cache->end() );

      // A cache miss
      it = cache->lower_bound(&*app, 7).first;
      TS_ASSERT_EQUALS( it, cache->find(&*app, 10).first );
      f = 0;
      it->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_10_reference);

      // A cache hit
      it = cache->lower_bound(&*app, 10).first;
      TS_ASSERT_DIFFERS(it, cache->begin(&*app));
      TS_ASSERT_DIFFERS(it, cache->end());
      f = 0;
      it->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_10_reference);

      // NB: searching for a specific cache key will create an iterator
      // that iterates over all elements for that application
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_DIFFERS(it, cache->end());
      f = 0;
      it->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_10_reference);
      --it;
      TS_ASSERT_DIFFERS(it, cache->end());
      f = 0;
      it->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_5_reference);
      --it;
      TS_ASSERT_EQUALS(it, cache->end());


      // Now with two applications present...
      cache->insert(mo_app->EvalMF(5));

      it = cache->lower_bound(&*mo_app, 7).first;
      TS_ASSERT_EQUALS( it, cache->end() );
      it = cache->lower_bound(&*mo_app, 3).first;
      TS_ASSERT_EQUALS( it, cache->find(&*mo_app, 5).first );

      // A cache miss
      it = cache->lower_bound(&*app, 7).first;
      TS_ASSERT_EQUALS( it, cache->find(&*app, 10).first );
      f = 0;
      it->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_10_reference);

      // A cache hit
      it = cache->lower_bound(&*app, 10).first;
      TS_ASSERT_DIFFERS(it, cache->begin(&*app));
      TS_ASSERT_DIFFERS(it, cache->end());
      f = 0;
      it->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_10_reference);

      // NB: searching for a specific cache key will create an iterator
      // that iterates over all elements for that application
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_DIFFERS(it, cache->end());
      f = 0;
      it->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_10_reference);
      --it;
      TS_ASSERT_DIFFERS(it, cache->end());
      f = 0;
      it->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_5_reference);
      --it;
      TS_ASSERT_EQUALS(it, cache->end());
   }

   void test_upper_bound()
   {
      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);

      Cache::iterator it = cache->upper_bound(&*app, 7).first;
      TS_ASSERT_EQUALS( it, cache->end() );

      double f = 0;
      double f_5_reference = 0;
      double f_10_reference = 0;

      cache->insert(app->EvalF(5, f_5_reference));
      cache->insert(app->EvalF(10, f_10_reference));

      it = cache->upper_bound(&*mo_app, 7).first;
      TS_ASSERT_EQUALS( it, cache->end() );

      // A cache miss
      it = cache->upper_bound(&*app, 7).first;
      TS_ASSERT_EQUALS( it, cache->find(&*app, 10).first );
      f = 0;
      it->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_10_reference);

      // A cache hit
      it = cache->upper_bound(&*app, 10).first;
      TS_ASSERT_EQUALS( it, cache->end() );

      it = cache->upper_bound(&*app, 5).first;
      TS_ASSERT_DIFFERS(it, cache->begin(&*app));
      TS_ASSERT_DIFFERS(it, cache->end());
      f = 0;
      it->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_10_reference);

      // NB: searching for a specific cache key will create an iterator
      // that iterates over all elements for that application
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_DIFFERS(it, cache->end());
      f = 0;
      it->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_10_reference);
      --it;
      TS_ASSERT_DIFFERS(it, cache->end());
      f = 0;
      it->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_5_reference);
      --it;
      TS_ASSERT_EQUALS(it, cache->end());


      // Now with two applications present...
      cache->insert(mo_app->EvalMF(5));

      it = cache->upper_bound(&*mo_app, 7).first;
      TS_ASSERT_EQUALS( it, cache->end() );
      it = cache->upper_bound(&*mo_app, 3).first;
      TS_ASSERT_EQUALS( it, cache->find(&*mo_app, 5).first );

      // A cache miss
      it = cache->upper_bound(&*app, 7).first;
      TS_ASSERT_EQUALS( it, cache->find(&*app, 10).first );
      f = 0;
      it->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_10_reference);

      // A cache hit
      it = cache->upper_bound(&*app, 5).first;
      TS_ASSERT_DIFFERS(it, cache->begin(&*app));
      TS_ASSERT_DIFFERS(it, cache->end());
      f = 0;
      it->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_10_reference);

      // NB: searching for a specific cache key will create an iterator
      // that iterates over all elements for that application
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_DIFFERS(it, cache->end());
      f = 0;
      it->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_10_reference);
      --it;
      TS_ASSERT_DIFFERS(it, cache->end());
      f = 0;
      it->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_5_reference);
      --it;
      TS_ASSERT_EQUALS(it, cache->end());
   }

   void test_find_api()
   {
      double f = 0;
      double f_5_reference = 0;
      double f_10_reference = 0;
      cache->insert(app->EvalF(5, f_5_reference));
      cache->insert(app->EvalF(10, f_10_reference));

      std::pair<colin::Cache::iterator, colin::Cache::Key> tmp;

      tmp = cache->find(&*app, 5);
      TS_ASSERT_DIFFERS(tmp.first, cache->end());
      f = 0;
      tmp.first->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_5_reference);
      TS_ASSERT_EQUALS(tmp.first->first.key, tmp.second);
      TS_ASSERT_EQUALS(cache->find(&*app, tmp.second), tmp.first);
      TS_ASSERT_EQUALS(cache->find(tmp.first->first), tmp.first);

      tmp = cache->find(&*app, 6);
      TS_ASSERT_EQUALS(tmp.first, cache->end());
      TS_ASSERT_EQUALS(cache->find(&*app, tmp.second), tmp.first);
   }

   void test_lower_bound_api()
   {
      double f = 0;
      double f_5_reference = 0;
      double f_10_reference = 0;
      cache->insert(app->EvalF(5, f_5_reference));
      cache->insert(app->EvalF(10, f_10_reference));

      std::pair<colin::Cache::iterator, colin::Cache::Key> tmp;

      tmp = cache->lower_bound(&*app, 5);
      TS_ASSERT_DIFFERS(tmp.first, cache->end());
      f = 0;
      tmp.first->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_5_reference);
      TS_ASSERT_EQUALS(tmp.first->first.key, tmp.second);
      TS_ASSERT_EQUALS(cache->lower_bound(&*app, tmp.second), tmp.first);
      TS_ASSERT_EQUALS(cache->lower_bound(tmp.first->first), tmp.first);

      tmp = cache->lower_bound(&*app, 6);
      TS_ASSERT_DIFFERS(tmp.first, cache->end());
      f = 0;
      tmp.first->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_10_reference);
      TS_ASSERT_DIFFERS(tmp.first->first.key, tmp.second);
      TS_ASSERT_EQUALS(cache->lower_bound(&*app, tmp.second), tmp.first);
      TS_ASSERT_EQUALS(cache->lower_bound(tmp.first->first), tmp.first);

      tmp = cache->lower_bound(&*app, 11);
      TS_ASSERT_EQUALS(tmp.first, cache->end());
      TS_ASSERT_EQUALS(cache->lower_bound(&*app, tmp.second), tmp.first);
   }

   void test_upper_bound_api()
   {
      double f = 0;
      double f_5_reference = 0;
      double f_10_reference = 0;
      cache->insert(app->EvalF(5, f_5_reference));
      cache->insert(app->EvalF(10, f_10_reference));

      std::pair<colin::Cache::iterator, colin::Cache::Key> tmp;

      tmp = cache->upper_bound(&*app, 4);
      TS_ASSERT_DIFFERS(tmp.first, cache->end());
      f = 0;
      tmp.first->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_5_reference);
      TS_ASSERT_DIFFERS(tmp.first->first.key, tmp.second);
      TS_ASSERT_EQUALS(cache->upper_bound(&*app, tmp.second), tmp.first);
      TS_ASSERT_DIFFERS(cache->upper_bound(tmp.first->first), tmp.first);

      tmp = cache->upper_bound(&*app, 5);
      TS_ASSERT_DIFFERS(tmp.first, cache->end());
      f = 0;
      tmp.first->second.asResponse(&*app).get(f_info, f);
      TS_ASSERT_EQUALS(f, f_10_reference);
      TS_ASSERT_DIFFERS(tmp.first->first.key, tmp.second);
      TS_ASSERT_EQUALS(cache->upper_bound(&*app, tmp.second), tmp.first);
      TS_ASSERT_DIFFERS(cache->upper_bound(tmp.first->first), tmp.first);

      tmp = cache->upper_bound(&*app, 10);
      TS_ASSERT_EQUALS(tmp.first, cache->end());
      TS_ASSERT_EQUALS(cache->upper_bound(&*app, tmp.second), tmp.first);
   }

   void test_annotate()
   {
      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);

      Cache::iterator it1 = cache->insert(app->EvalF(5)).first;
      Cache::iterator it2 = cache->insert(app->EvalF(10)).first;
      Cache::iterator it3 = cache->insert(mo_app->EvalMF(5)).first;
      
      TS_ASSERT( it1->second.annotations.empty() );
      TS_ASSERT( it2->second.annotations.empty() );
      TS_ASSERT( it3->second.annotations.empty() );

      TS_ASSERT( it1->second.annotation("test").empty() );
      TS_ASSERT( it2->second.annotation("test").empty() );
      TS_ASSERT( it3->second.annotation("test").empty() );

      cache->annotate(it1, "test", 5);
      TS_ASSERT_EQUALS( it1->second.annotations.size(), 1 );
      TS_ASSERT( it2->second.annotations.empty() );
      TS_ASSERT( it3->second.annotations.empty() );

      TS_ASSERT_EQUALS( it1->second.annotation("test"), 5 );
      TS_ASSERT( it2->second.annotation("test").empty() );
      TS_ASSERT( it3->second.annotation("test").empty() );

      cache->annotate(it1, "test2", 15);
      TS_ASSERT_EQUALS( it1->second.annotations.size(), 2 );
      TS_ASSERT( it2->second.annotations.empty() );
      TS_ASSERT( it3->second.annotations.empty() );

      TS_ASSERT_EQUALS( it1->second.annotation("test"), 5 );
      TS_ASSERT_EQUALS( it1->second.annotation("test2"), 15 );
      TS_ASSERT( it2->second.annotation("test2").empty() );
      TS_ASSERT( it3->second.annotation("test2").empty() );

      cache->annotate(it1, "test2", 150);
      TS_ASSERT_EQUALS( it1->second.annotations.size(), 2 );
      TS_ASSERT( it2->second.annotations.empty() );
      TS_ASSERT( it3->second.annotations.empty() );

      TS_ASSERT_EQUALS( it1->second.annotation("test"), 5 );
      TS_ASSERT_EQUALS( it1->second.annotation("test2"), 150 );
      TS_ASSERT( it2->second.annotation("test2").empty() );
      TS_ASSERT( it3->second.annotation("test2").empty() );

      TS_ASSERT_THROWS_ASSERT
         ( cache->annotate(cache->end(), "test2", 1),
           std::runtime_error &e,
           TEST_WHAT_CONTAINS
           ( e, "cannot annotate nonexistant item (end() pointer)" ) );

      TS_ASSERT_THROWS_ASSERT
         ( cache->annotate(it1, "", 1),
           std::runtime_error &e,
           TEST_WHAT_CONTAINS
           ( e, "cannot annotate with empty 'attribute'" ) );

   }

   void test_erase_annotation()
   {
      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);

      Cache::iterator it = cache->insert(app->EvalF(5)).first;
      cache->annotate(it, "test1", 1);
      cache->annotate(it, "test2", 2);
      cache->annotate(it, "test3", 3);

      TS_ASSERT_EQUALS( it->second.annotations.size(), 3 );
      TS_ASSERT_EQUALS( cache->erase_annotation(it, "test1"), 1 );
      TS_ASSERT_EQUALS( it->second.annotations.size(), 2 );
      TS_ASSERT_EQUALS( cache->erase_annotation(it, "test1"), 0 );
      TS_ASSERT_EQUALS( it->second.annotations.size(), 2 );
      TS_ASSERT_EQUALS( cache->erase_annotation(it), 2 );
      TS_ASSERT_EQUALS( it->second.annotations.size(), 0 );

      TS_ASSERT_THROWS_ASSERT
         ( cache->erase_annotation(cache->end(), "test2"),
           std::runtime_error &e,
           TEST_WHAT_ENDSWITH
           ( e, "cannot erase annotations from nonexistant item "
             "(end() pointer)" ) );

   }

   void test_retrieve_data()
   {
      AppResponse ref = app->EvalF(5);
      cache->insert(ref);

      double ref_obj = 0.;
      ref.get(f_info, ref_obj, &*app);

      std::pair<colin::Cache::iterator, colin::Cache::Key> tmp;
      tmp = cache->find(&*app, 5);
      TS_ASSERT_DIFFERS(tmp.first, cache->end());
      TS_ASSERT_EQUALS(tmp.first, cache->begin());

      double obj = 0.;
      tmp.first->second.asResponse(&*app).get(f_info, obj);

      TS_ASSERT_EQUALS(ref_obj, obj);
   }

protected:

   CacheHandle cache;

   Problem<UNLP1_problem> app;
   Problem<MO_NLP1_problem> mo_app;
};


//=======================================================================

namespace unittest { class Cache_Local_reformulated; }

class colin::unittest::Cache_Local_reformulated \
   : public colin::unittest::Cache_Local
{
public:
   Cache_Local_reformulated()
   {
      Problem<NLP1_problem> tmp;
      tmp.set_application
         ( ApplicationHandle::create
           < TestApplications::singleObj_denseCon<NLP1_problem> >() )
         ->configure(1);

      app.set_application
         ( ApplicationHandle::create
           < ConstraintPenaltyApplication<UNLP1_problem> >() )
         ->reformulate_application(tmp);
   }
};


//=======================================================================

namespace unittest { class Cache_View_Subset; }

class colin::unittest::Cache_View_Subset : public colin::unittest::Cache_Local
{
public:
   void setUp()
   {
      CacheHandle core_cache = CacheFactory().create("Local");
      cache = CacheFactory().create_view("Subset", core_cache);
   }

   void tearDown()
   { }
};


//=======================================================================

namespace unittest { class Cache_View_Subset_reformulated; }

class colin::unittest::Cache_View_Subset_reformulated \
   : public colin::unittest::Cache_Local
{
public:
public:
   Cache_View_Subset_reformulated()
   {
      Problem<NLP1_problem> tmp;
      tmp.set_application
         ( ApplicationHandle::create
           < TestApplications::singleObj_denseCon<NLP1_problem> >() )
         ->configure(1);

      app.set_application
         ( ApplicationHandle::create
           < ConstraintPenaltyApplication<UNLP1_problem> >() )
         ->reformulate_application(tmp);
   }

   void setUp()
   {
      CacheHandle core_cache = CacheFactory().create("Local");
      cache = CacheFactory().create_view("Subset", core_cache);
   }

   void tearDown()
   { }
};


//=======================================================================

namespace unittest { class Cache_View_Subset_prepopulated; }

class colin::unittest::Cache_View_Subset_prepopulated \
   : public colin::unittest::Cache_Local
{
public:
   void setUp()
   {
      CacheHandle core_cache = CacheFactory().create("Local");
      cache = CacheFactory().create_view("Subset", core_cache);
      core_cache->insert(app->EvalF(5));
      core_cache->insert(app->EvalF(10));
      core_cache->insert(mo_app->EvalMF(5));
   }

   void tearDown()
   { }
};


//=======================================================================
//=======================================================================

// Being lazy: declaring this test suite after the Local testers so I
// don't have to escape all the Cache references above with
// "colin::"
namespace unittest { class Cache; }

class colin::unittest::Cache : public CxxTest::TestSuite
{
public:
   Cache()
      : app(1), mo_app(1)
   {}

   void setUp()
   {
      cache = CacheFactory().create("Local");
      CacheFactory().register_cache(cache, "LocalCacheTest");
      ApplicationMngr().register_application(&app, "LocalCacheTest_SO");
      ApplicationMngr().register_application(&mo_app, "LocalCacheTest_MO");
   }

   void tearDown()
   {
      CacheFactory().unregister_cache("LocalCacheTest");
      ApplicationMngr().unregister_application("LocalCacheTest_SO");
      ApplicationMngr().unregister_application("LocalCacheTest_MO");
   }

   void test_cachePrinter()
   {
      TiXmlElement params("PrintCache");
      params.SetAttribute("cache", "LocalCacheTest");
      int rank = ExecuteManager::local;

      {
         utilib::OStreamTee tee(std::cout);
         ExecuteMngr().run_command("PrintCache");
         TS_ASSERT_EQUALS(tee.out.str(), 
                          "<PrintCache> ERROR: unknown cache 'InterSolver'\n");
      }

      {
         utilib::OStreamTee tee(std::cout);
         ExecuteMngr().run_command("PrintCache", rank, &params);
         TS_ASSERT_EQUALS(tee.out.str(), 
                          "Cache:\n"
                          "   name: LocalCacheTest\n"
                          "   size: 0\n");
      }

      cache->insert(app.EvalF(5));

      // Still 0 points because the default application context is the
      // last one registered (mo_app).
      {
         utilib::OStreamTee tee(std::cout);
         ExecuteMngr().run_command("PrintCache", rank, &params);
         TS_ASSERT_EQUALS(tee.out.str(), 
                          "Cache:\n"
                          "   name: LocalCacheTest\n"
                          "   size: 0\n");
      }

      params.SetAttribute("problem", "bogus_problem");
      TS_ASSERT_THROWS_ASSERT
         ( ExecuteMngr().run_command("PrintCache", rank, &params),
           std::runtime_error &e,
           TEST_WHAT( e, "PrintCacheExecuteFunctor::execute(): "
                        "cannot find problem context (bogus_problem) "
                      "at element \"PrintCache\"" ) );

      params.SetAttribute("problem", "LocalCacheTest_SO");
      {
         utilib::OStreamTee tee(std::cout);
         ExecuteMngr().run_command("PrintCache", rank, &params);
         TS_ASSERT_EQUALS(tee.out.str(), 
                          "Cache:\n"
                          "   name: LocalCacheTest\n"
                          "   size: 1\n"
                          "   data:\n"
                          "    - AppResponse Data:\n"
                          "        domain: [ 5 ]\n"
                          "        seed:   0\n"
                          "        objective function: 5\n");
      }

      cache->insert(app.EvalF(2));
      {
         // NB: order is set by the KeyGenerator (and the default is to
         // use the domain).
         utilib::OStreamTee tee(std::cout);
         ExecuteMngr().run_command("PrintCache", rank, &params);
         TS_ASSERT_EQUALS(tee.out.str(), 
                          "Cache:\n"
                          "   name: LocalCacheTest\n"
                          "   size: 2\n"
                          "   data:\n"
                          "    - AppResponse Data:\n"
                          "        domain: [ 2 ]\n"
                          "        seed:   0\n"
                          "        objective function: 2\n"
                          "    - AppResponse Data:\n"
                          "        domain: [ 5 ]\n"
                          "        seed:   0\n"
                          "        objective function: 5\n");
      }

      params.SetAttribute("solver", "bogus_solver");
      TS_ASSERT_THROWS_ASSERT
         ( ExecuteMngr().run_command("PrintCache", rank, &params),
           std::runtime_error &e,
           TEST_WHAT( e, "PrintCacheExecuteFunctor::execute(): "
                      "cannot specify both problem_context and "
                      "solver_context in element \"PrintCache\"" ) );

      params.RemoveAttribute("problem");
      TS_ASSERT_THROWS_ASSERT
         ( ExecuteMngr().run_command("PrintCache", rank, &params),
           std::runtime_error &e,
           TEST_WHAT( e, "SolverMngr::get_solver(): "
                      "Solver with name 'bogus_solver' not registered" ) );

   }

   void test_resetKeyGenerator()
   {
      utilib::OStreamTee tee(std::cerr);
      cache->insert(app.EvalF(1));
      TS_ASSERT_EQUALS(cache->size(), 1);
      TS_ASSERT_EQUALS(cache->size(&app), 1);

      cache->set_key_generator(new KeyGenerator_EpsilonMatch());
      TS_ASSERT_EQUALS(tee.out.str(), 
                       "WARNING: setting key generator for a non-empty "
                       "cache results in loss of data.\n");
      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&app), 0);
   }


private:

   CacheHandle cache;

   TestApplications::singleObj_denseCon<NLP0_problem>  app;

   TestApplications::multiObj_denseCon<MO_NLP0_problem>  mo_app;
};


} // namespace colin
