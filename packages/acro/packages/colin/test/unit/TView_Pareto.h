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
#include <colin/cache/View_Pareto.h>

#include <utilib/OStreamTee.h>

#include <cxxtest/TestSuite.h>

#include "TestApplications.h"
#include "CommonTestUtils.h"

namespace colin {
namespace unittest { class View_Pareto; }

class colin::unittest::View_Pareto : public CxxTest::TestSuite
{
public:
   View_Pareto()
   {
      app.set_application
         ( ApplicationHandle::create
           < TestApplications::singleObj_denseCon<UNLP1_problem> >() )
         ->configure(2);

      mo_app.set_application
         ( ApplicationHandle::create
           < TestApplications::multiObj_denseCon<MO_NLP1_problem> >() )
         ->configure(2);
   }

   void setUp()
   {
      core_cache = CacheFactory().create("Local");
      cache = CacheFactory().create_view("Pareto", core_cache);
   }

   std::vector<double> domain(double _1, double _2)
   {
      std::vector<double> ans(2);
      ans[0] = _1;
      ans[1] = _2;
      return ans;
   }

   void test_insert_and_size()
   {
      std::pair<Cache::iterator, bool> ans;

      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);

      ans = cache->insert(mo_app->EvalMF(domain(1, 1)));
      TS_ASSERT_DIFFERS( ans.first, cache->end() );
      TS_ASSERT( ans.second );
      TS_ASSERT_EQUALS(cache->size(), 1);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);

      ans = cache->insert(mo_app->EvalMF(domain(.5,1.5)));
      TS_ASSERT_DIFFERS( ans.first, cache->end() );
      TS_ASSERT( ans.second );
      TS_ASSERT_EQUALS(cache->size(), 2);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 2);

      ans = cache->insert(mo_app->EvalMF(domain(2,2)));
      TS_ASSERT_EQUALS( ans.first, cache->end() );
      TS_ASSERT( ! ans.second );
      TS_ASSERT_EQUALS(cache->size(), 2);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 2);

      ans = cache->insert(mo_app->EvalMF(domain(.5,.5)));
      TS_ASSERT_DIFFERS( ans.first, cache->end() );
      TS_ASSERT( ans.second );
      TS_ASSERT_EQUALS(cache->size(), 1);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);

      ans = cache->insert(app->EvalF(domain(-2,2)));
      TS_ASSERT_EQUALS( ans.first, cache->end() );
      TS_ASSERT( ! ans.second );
      TS_ASSERT_EQUALS(cache->size(), 1);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);

      TS_ASSERT_EQUALS(core_cache->size(), 5);
      TS_ASSERT_EQUALS(core_cache->size(&*app), 1);
      TS_ASSERT_EQUALS(core_cache->size(&*mo_app), 4);
   }

   void test_clear()
   {
      TS_ASSERT_THROWS_ASSERT
         ( cache->clear(), view_error &e,
           TEST_WHAT( e, "View_Pareto::clear(): "
                      "Membership in the Pareto set is determined by all data "
                      "in the underlying cache.  You cannot remove elements "
                      "from the View.  Instead, remove them from the "
                      "underlying cache." ) );
   }

   void test_erase()
   {
      TS_ASSERT_THROWS_ASSERT
         ( cache->erase(&*mo_app, domain(0,0)), view_error &e,
           TEST_WHAT( e, "View_Pareto::erase_item(): "
                      "Membership in the Pareto set is determined by all data "
                      "in the underlying cache.  You cannot remove elements "
                      "from the View.  Instead, remove them from the "
                      "underlying cache." ) );

      TS_ASSERT_THROWS_ASSERT
         ( cache->erase(&*mo_app, Cache::Key()), view_error &e,
           TEST_WHAT( e, "View_Pareto::erase_item(): "
                      "Membership in the Pareto set is determined by all data "
                      "in the underlying cache.  You cannot remove elements "
                      "from the View.  Instead, remove them from the "
                      "underlying cache." ) );

      TS_ASSERT_THROWS_ASSERT
         ( cache->erase(Cache::CachedKey()), view_error &e,
           TEST_WHAT( e, "View_Pareto::erase_item(): "
                      "Membership in the Pareto set is determined by all data "
                      "in the underlying cache.  You cannot remove elements "
                      "from the View.  Instead, remove them from the "
                      "underlying cache." ) );

      TS_ASSERT_THROWS_ASSERT
         ( cache->erase(cache->begin()), view_error &e,
           TEST_WHAT( e, "View_Pareto::erase_iterator(): "
                      "Membership in the Pareto set is determined by all data "
                      "in the underlying cache.  You cannot remove elements "
                      "from the View.  Instead, remove them from the "
                      "underlying cache." ) );
   }

   void test_underlying_erase()
   {
      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);

      TS_ASSERT( cache->insert(mo_app->EvalMF(domain(1, 1))).second );
      TS_ASSERT( cache->insert(mo_app->EvalMF(domain(.5,1.5))).second );
      TS_ASSERT( ! cache->insert(mo_app->EvalMF(domain(2,2))).second );
      TS_ASSERT( cache->insert(mo_app->EvalMF(domain(.5,.5))).second );
      TS_ASSERT( ! cache->insert(app->EvalF(domain(1,1))).second );

      TS_ASSERT_EQUALS(cache->size(), 1);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 1);
      TS_ASSERT_EQUALS(core_cache->size(), 5);
      TS_ASSERT_EQUALS(core_cache->size(&*app), 1);
      TS_ASSERT_EQUALS(core_cache->size(&*mo_app), 4);

      TS_ASSERT_EQUALS( core_cache->erase(&*mo_app, domain(.5,.5)), 1 );

      TS_ASSERT_EQUALS(cache->size(), 2);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 2);
      TS_ASSERT_EQUALS(core_cache->size(), 4);
      TS_ASSERT_EQUALS(core_cache->size(&*app), 1);
      TS_ASSERT_EQUALS(core_cache->size(&*mo_app), 3);

      TS_ASSERT_EQUALS( core_cache->erase(&*app, domain(1,1)), 1 );

      TS_ASSERT_EQUALS(cache->size(), 2);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 2);
      TS_ASSERT_EQUALS(core_cache->size(), 3);
      TS_ASSERT_EQUALS(core_cache->size(&*app), 0);
      TS_ASSERT_EQUALS(core_cache->size(&*mo_app), 3);

      TS_ASSERT_EQUALS( core_cache->erase(&*mo_app, domain(2,2)), 1 );

      TS_ASSERT_EQUALS(cache->size(), 2);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 2);
      TS_ASSERT_EQUALS(core_cache->size(), 2);
      TS_ASSERT_EQUALS(core_cache->size(&*app), 0);
      TS_ASSERT_EQUALS(core_cache->size(&*mo_app), 2);

   }

   void test_iterators()
   {
      Cache::iterator it;

      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->begin(), cache->end());
      TS_ASSERT_EQUALS(cache->begin(&*mo_app), cache->end());
      TS_ASSERT_EQUALS(cache->begin(&*app), cache->end());

      // All iterators should point to end() - and since the cache is
      // empty, incrementing / decrementing them should also point to
      // end()
      it = cache->begin(&*mo_app);
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_EQUALS(it, cache->begin(&*mo_app));

      it = cache->begin(&*app);
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_EQUALS(it, cache->begin(&*app));


      // Add a point to the MO application
      it = cache->insert(mo_app->EvalMF(domain(1,1))).first;

      TS_ASSERT_DIFFERS(it, cache->end());
      TS_ASSERT_EQUALS(it, cache->begin());
      TS_ASSERT_EQUALS(it, cache->begin(&*mo_app));
      TS_ASSERT_DIFFERS(it, cache->begin(&*app));

      TS_ASSERT_DIFFERS(cache->begin(), cache->end());
      TS_ASSERT_DIFFERS(cache->begin(&*mo_app), cache->end());
      TS_ASSERT_EQUALS(cache->begin(&*app), cache->end());

      it = cache->begin(&*mo_app);
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_EQUALS(it, cache->begin(&*mo_app));

      // We also want to explicitly test the iterator's != operator
      TS_ASSERT( cache->begin() != cache->end() );
      TS_ASSERT( ! ( cache->begin() != cache->begin() ) );
      TS_ASSERT( ! ( cache->end() != cache->end() ) );


      // Add a point to the SO application
      it = cache->insert(app->EvalF(domain(1,1))).first;

      TS_ASSERT_DIFFERS(it, cache->begin());
      TS_ASSERT_EQUALS(it, cache->end());
      TS_ASSERT_EQUALS(it, cache->begin(&*app));
      TS_ASSERT_DIFFERS(it, cache->begin(&*mo_app));

      TS_ASSERT_DIFFERS(cache->begin(), cache->end());
      TS_ASSERT_DIFFERS(cache->begin(&*mo_app), cache->end());
      TS_ASSERT_EQUALS(cache->begin(&*app), cache->end());

      it = cache->begin(&*app);
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_EQUALS(it, cache->begin(&*app));


      // re-add the first evaluation
      it = cache->insert(mo_app->EvalMF(domain(1,1))).first;

      TS_ASSERT_DIFFERS(it, cache->end());
      TS_ASSERT_EQUALS(it, cache->begin(&*mo_app));
      TS_ASSERT_DIFFERS(it, cache->begin(&*app));

      TS_ASSERT_DIFFERS(cache->begin(), cache->end());
      TS_ASSERT_DIFFERS(cache->begin(&*mo_app), cache->end());
      TS_ASSERT_EQUALS(cache->begin(&*app), cache->end());

      TS_ASSERT_DIFFERS(cache->begin(&*mo_app), cache->begin(&*app));

      it = cache->begin(&*mo_app);
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_EQUALS(it, cache->begin(&*mo_app));

      it = cache->begin(&*app);
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_EQUALS(it, cache->begin(&*app));


      // Add a second point to the MO application and test the postfix
      // operators
      cache->insert(mo_app->EvalMF(domain(1.5,.5)));

      it = cache->begin(&*mo_app);
      TS_ASSERT_EQUALS(it++, cache->begin(&*mo_app));

      TS_ASSERT_DIFFERS(it, cache->begin(&*mo_app));
      TS_ASSERT_DIFFERS(it, cache->end());
      ++it;

      TS_ASSERT_EQUALS(it--, cache->end());

      TS_ASSERT_DIFFERS(it, cache->begin(&*mo_app));
      TS_ASSERT_DIFFERS(it, cache->end());

      it--;
      TS_ASSERT_EQUALS(it, cache->begin(&*mo_app));


      // Test iterators that are not bound to an application
      it = cache->begin();
      TS_ASSERT_DIFFERS(it++, cache->end());
      TS_ASSERT_DIFFERS(it++, cache->end());
      TS_ASSERT_EQUALS(it--, cache->end());  
      TS_ASSERT_DIFFERS(it--, cache->end());
      TS_ASSERT_EQUALS(it, cache->begin());  
   }

   void test_find()
   {
      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);

      std::vector<double> mf;
      std::vector<double> mf_A_reference;
      std::vector<double> mf_B_reference;

      cache->insert(mo_app->EvalMF(domain(1,1), mf_A_reference));
      cache->insert(mo_app->EvalMF(domain(1.5,.5), mf_B_reference));
      cache->insert(app->EvalF(domain(1,1)));

      // A cache hit
      Cache::iterator it = cache->find(&*mo_app, domain(1,1)).first;
      TS_ASSERT_EQUALS(it, cache->begin(&*mo_app));
      TS_ASSERT_DIFFERS(it, cache->end());
      mf.clear();
      it->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_A_reference);

      // NB: searching for a specific cache key will create an iterator
      // that ONLY iterates over matching elements (i.e. searching for 10
      // will NOT find 5).
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_DIFFERS(it, cache->end());
      mf.clear();
      it->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_A_reference);
      --it;
      TS_ASSERT_EQUALS(it, cache->end());

      // A cache miss
      it = cache->find(&*mo_app, domain(1,1.5)).first;
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
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);

      Cache::iterator it = cache->lower_bound(&*mo_app, domain(1,1.5)).first;
      TS_ASSERT_EQUALS( it, cache->end() );

      std::vector<double> mf;
      std::vector<double> mf_A_reference;
      std::vector<double> mf_B_reference;

      cache->insert(mo_app->EvalMF(domain(1,1), mf_A_reference));
      cache->insert(mo_app->EvalMF(domain(1.5,.5), mf_B_reference));

      it = cache->lower_bound(&*app, domain(1,1.5)).first;
      TS_ASSERT_EQUALS( it, cache->end() );

      // A cache miss
      it = cache->lower_bound(&*mo_app, domain(1,1.5)).first;
      TS_ASSERT_EQUALS( it, cache->find(&*mo_app, domain(1.5,.5)).first );
      mf.clear();
      it->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_B_reference);

      // A cache hit
      it = cache->lower_bound(&*mo_app, domain(1.5,.5)).first;
      TS_ASSERT_DIFFERS(it, cache->begin(&*mo_app));
      TS_ASSERT_DIFFERS(it, cache->end());
      mf.clear();
      it->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_B_reference);

      // NB: searching for a specific cache key will create an iterator
      // that iterates over all elements for that mo_application
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_DIFFERS(it, cache->end());
      mf.clear();
      it->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_B_reference);
      --it;
      TS_ASSERT_DIFFERS(it, cache->end());
      mf.clear();
      it->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_A_reference);
      --it;
      TS_ASSERT_EQUALS(it, cache->end());


      // Now with two mo_applications present...
      cache->insert(app->EvalF(domain(1,1)));

      it = cache->lower_bound(&*app, domain(1,1.5)).first;
      TS_ASSERT_EQUALS( it, cache->end() );
      it = cache->lower_bound(&*app, domain(.5,.5)).first;
      TS_ASSERT_EQUALS( it, cache->find(&*app, domain(1,1)).first );

      // A cache miss
      it = cache->lower_bound(&*mo_app, domain(1,1.5)).first;
      TS_ASSERT_EQUALS( it, cache->find(&*mo_app, domain(1.5,.5)).first );
      mf.clear();
      it->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_B_reference);

      // A cache hit
      it = cache->lower_bound(&*mo_app, domain(1.5,.5)).first;
      TS_ASSERT_DIFFERS(it, cache->begin(&*mo_app));
      TS_ASSERT_DIFFERS(it, cache->end());
      mf.clear();
      it->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_B_reference);

      // NB: searching for a specific cache key will create an iterator
      // that iterates over all elements for that mo_application
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_DIFFERS(it, cache->end());
      mf.clear();
      it->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_B_reference);
      --it;
      TS_ASSERT_DIFFERS(it, cache->end());
      mf.clear();
      it->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_A_reference);
      --it;
      TS_ASSERT_EQUALS(it, cache->end());
   }

   void test_upper_bound()
   {
      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);

      Cache::iterator it = cache->upper_bound(&*mo_app, domain(1,1.5)).first;
      TS_ASSERT_EQUALS( it, cache->end() );

      std::vector<double> mf;
      std::vector<double> mf_A_reference;
      std::vector<double> mf_B_reference;

      cache->insert(mo_app->EvalMF(domain(1,1), mf_A_reference));
      cache->insert(mo_app->EvalMF(domain(1.5,.5), mf_B_reference));

      it = cache->upper_bound(&*app, domain(1,1.5)).first;
      TS_ASSERT_EQUALS( it, cache->end() );

      // A cache miss
      it = cache->upper_bound(&*mo_app, domain(1,1.5)).first;
      TS_ASSERT_EQUALS( it, cache->find(&*mo_app, domain(1.5,.5)).first );
      mf.clear();
      it->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_B_reference);

      // A cache hit
      it = cache->upper_bound(&*mo_app, domain(1.5,.5)).first;
      TS_ASSERT_EQUALS( it, cache->end() );

      it = cache->upper_bound(&*mo_app, domain(1,1)).first;
      TS_ASSERT_DIFFERS(it, cache->begin(&*mo_app));
      TS_ASSERT_DIFFERS(it, cache->end());
      mf.clear();
      it->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_B_reference);

      // NB: searching for a specific cache key will create an iterator
      // that iterates over all elements for that mo_application
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_DIFFERS(it, cache->end());
      mf.clear();
      it->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_B_reference);
      --it;
      TS_ASSERT_DIFFERS(it, cache->end());
      mf.clear();
      it->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_A_reference);
      --it;
      TS_ASSERT_EQUALS(it, cache->end());


      // Now with two mo_applications present...
      cache->insert(app->EvalF(domain(1,1)));

      it = cache->upper_bound(&*app, domain(1,1.5)).first;
      TS_ASSERT_EQUALS( it, cache->end() );
      it = cache->upper_bound(&*app, domain(.5,.5)).first;
      TS_ASSERT_EQUALS( it, cache->find(&*app, domain(1,1)).first );

      // A cache miss
      it = cache->upper_bound(&*mo_app, domain(1,1.5)).first;
      TS_ASSERT_EQUALS( it, cache->find(&*mo_app, domain(1.5,.5)).first );
      mf.clear();
      it->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_B_reference);

      // A cache hit
      it = cache->upper_bound(&*mo_app, domain(1,1)).first;
      TS_ASSERT_DIFFERS(it, cache->begin(&*mo_app));
      TS_ASSERT_DIFFERS(it, cache->end());
      mf.clear();
      it->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_B_reference);

      // NB: searching for a specific cache key will create an iterator
      // that iterates over all elements for that mo_application
      ++it;
      TS_ASSERT_EQUALS(it, cache->end());
      --it;
      TS_ASSERT_DIFFERS(it, cache->end());
      mf.clear();
      it->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_B_reference);
      --it;
      TS_ASSERT_DIFFERS(it, cache->end());
      mf.clear();
      it->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_A_reference);
      --it;
      TS_ASSERT_EQUALS(it, cache->end());
   }

   void test_find_api()
   {
      std::vector<double> mf;
      std::vector<double> mf_A_reference;
      std::vector<double> mf_B_reference;
      cache->insert(mo_app->EvalMF(domain(1,1), mf_A_reference));
      cache->insert(mo_app->EvalMF(domain(1.5,.5), mf_B_reference));

      std::pair<colin::Cache::iterator, colin::Cache::Key> tmp;

      tmp = cache->find(&*mo_app, domain(1,1));
      TS_ASSERT_DIFFERS(tmp.first, cache->end());
      mf.clear();
      tmp.first->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_A_reference);
      TS_ASSERT_EQUALS(tmp.first->first.key, tmp.second);
      TS_ASSERT_EQUALS(cache->find(&*mo_app, tmp.second), tmp.first);
      TS_ASSERT_EQUALS(cache->find(tmp.first->first), tmp.first);

      tmp = cache->find(&*mo_app, domain(1,1.5));
      TS_ASSERT_EQUALS(tmp.first, cache->end());
      TS_ASSERT_EQUALS(cache->find(&*mo_app, tmp.second), tmp.first);
   }

   void test_lower_bound_api()
   {
      std::vector<double> mf;
      std::vector<double> mf_A_reference;
      std::vector<double> mf_B_reference;
      cache->insert(mo_app->EvalMF(domain(1,1), mf_A_reference));
      cache->insert(mo_app->EvalMF(domain(1.5,.5), mf_B_reference));

      std::pair<colin::Cache::iterator, colin::Cache::Key> tmp;

      tmp = cache->lower_bound(&*mo_app, domain(1,1));
      TS_ASSERT_DIFFERS(tmp.first, cache->end());
      mf.clear();
      tmp.first->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_A_reference);
      TS_ASSERT_EQUALS(tmp.first->first.key, tmp.second);
      TS_ASSERT_EQUALS(cache->lower_bound(&*mo_app, tmp.second), tmp.first);
      TS_ASSERT_EQUALS(cache->lower_bound(tmp.first->first), tmp.first);

      tmp = cache->lower_bound(&*mo_app, domain(1,1.5));
      TS_ASSERT_DIFFERS(tmp.first, cache->end());
      mf.clear();
      tmp.first->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_B_reference);
      TS_ASSERT_DIFFERS(tmp.first->first.key, tmp.second);
      TS_ASSERT_EQUALS(cache->lower_bound(&*mo_app, tmp.second), tmp.first);
      TS_ASSERT_EQUALS(cache->lower_bound(tmp.first->first), tmp.first);

      tmp = cache->lower_bound(&*mo_app, domain(2,2));
      TS_ASSERT_EQUALS(tmp.first, cache->end());
      TS_ASSERT_EQUALS(cache->lower_bound(&*mo_app, tmp.second), tmp.first);
   }

   void test_upper_bound_api()
   {
      std::vector<double> mf;
      std::vector<double> mf_A_reference;
      std::vector<double> mf_B_reference;
      cache->insert(mo_app->EvalMF(domain(1,1), mf_A_reference));
      cache->insert(mo_app->EvalMF(domain(1.5,.5), mf_B_reference));

      std::pair<colin::Cache::iterator, colin::Cache::Key> tmp;

      tmp = cache->upper_bound(&*mo_app, domain(.5,.5));
      TS_ASSERT_DIFFERS(tmp.first, cache->end());
      mf.clear();
      tmp.first->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_A_reference);
      TS_ASSERT_DIFFERS(tmp.first->first.key, tmp.second);
      TS_ASSERT_EQUALS(cache->upper_bound(&*mo_app, tmp.second), tmp.first);
      TS_ASSERT_DIFFERS(cache->upper_bound(tmp.first->first), tmp.first);

      tmp = cache->upper_bound(&*mo_app, domain(1,1));
      TS_ASSERT_DIFFERS(tmp.first, cache->end());
      mf.clear();
      tmp.first->second.asResponse(&*mo_app).get(mf_info, mf);
      TS_ASSERT_EQUALS(mf, mf_B_reference);
      TS_ASSERT_DIFFERS(tmp.first->first.key, tmp.second);
      TS_ASSERT_EQUALS(cache->upper_bound(&*mo_app, tmp.second), tmp.first);
      TS_ASSERT_DIFFERS(cache->upper_bound(tmp.first->first), tmp.first);

      tmp = cache->upper_bound(&*mo_app, domain(1.5,.5));
      TS_ASSERT_EQUALS(tmp.first, cache->end());
      TS_ASSERT_EQUALS(cache->upper_bound(&*mo_app, tmp.second), tmp.first);
   }

   void test_annotate()
   {
      TS_ASSERT_EQUALS(cache->size(), 0);
      TS_ASSERT_EQUALS(cache->size(&*app), 0);
      TS_ASSERT_EQUALS(cache->size(&*mo_app), 0);

      Cache::iterator it1 = cache->insert(mo_app->EvalMF(domain(1,1))).first;
      Cache::iterator it2 = cache->insert(mo_app->EvalMF(domain(.5,1.5))).first;
      Cache::iterator it3 = cache->insert(mo_app->EvalMF(domain(2,2))).first;
      
      TS_ASSERT( it1->second.annotations.empty() );
      TS_ASSERT( it2->second.annotations.empty() );

      TS_ASSERT( it1->second.annotation("test").empty() );
      TS_ASSERT( it2->second.annotation("test").empty() );

      cache->annotate(it1, "test", 5);
      TS_ASSERT_EQUALS( it1->second.annotations.size(), 1 );
      TS_ASSERT( it2->second.annotations.empty() );

      TS_ASSERT_EQUALS( it1->second.annotation("test"), 5 );
      TS_ASSERT( it2->second.annotation("test").empty() );

      cache->annotate(it1, "test2", 15);
      TS_ASSERT_EQUALS( it1->second.annotations.size(), 2 );
      TS_ASSERT( it2->second.annotations.empty() );

      TS_ASSERT_EQUALS( it1->second.annotation("test"), 5 );
      TS_ASSERT_EQUALS( it1->second.annotation("test2"), 15 );
      TS_ASSERT( it2->second.annotation("test2").empty() );

      cache->annotate(it1, "test2", 150);
      TS_ASSERT_EQUALS( it1->second.annotations.size(), 2 );
      TS_ASSERT( it2->second.annotations.empty() );

      TS_ASSERT_EQUALS( it1->second.annotation("test"), 5 );
      TS_ASSERT_EQUALS( it1->second.annotation("test2"), 150 );
      TS_ASSERT( it2->second.annotation("test2").empty() );

      TS_ASSERT_THROWS_ASSERT
         ( cache->annotate(it3, "test2", 1),
           std::runtime_error &e,
           TEST_WHAT_CONTAINS
           ( e, "cannot annotate nonexistant item (end() pointer)" ) );

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

      Cache::iterator it = cache->insert(mo_app->EvalMF(domain(1,1))).first;
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

   void test_update_triggers_insert()
   {
      cache->insert(mo_app->EvalMF(domain(1,1)));
      cache->insert(mo_app->EvalMF(domain(1.5,.5)));

      TS_ASSERT_EQUALS( core_cache->size(), 2 );
      TS_ASSERT_EQUALS( cache->size(), 2 );
      TS_ASSERT_EQUALS( cache->size(&*mo_app), 2 );

      cache->insert(mo_app->EvalG(domain(.5,.5)));

      TS_ASSERT_EQUALS( core_cache->size(), 3 );
      TS_ASSERT_EQUALS( cache->size(), 2 );
      TS_ASSERT_EQUALS( cache->size(&*mo_app), 2 );
      TS_ASSERT_EQUALS( cache->find(&*mo_app, domain(.5,.5)).first, 
                        cache->end() );
      TS_ASSERT_DIFFERS( core_cache->find(&*mo_app, domain(.5,.5)).first, 
                         core_cache->end() );
      
      cache->insert(mo_app->EvalMF(domain(.5,.5)));
      
      TS_ASSERT_EQUALS( core_cache->size(), 3 );
      TS_ASSERT_EQUALS( cache->size(), 1 );
      TS_ASSERT_EQUALS( cache->size(&*mo_app), 1 );
      TS_ASSERT_EQUALS( cache->find(&*mo_app, domain(.5,.5)).first, 
                        cache->begin() );
      TS_ASSERT_DIFFERS( core_cache->find(&*mo_app, domain(.5,.5)).first, 
                         core_cache->end() );
   }


   void assert_cache_equal(CacheHandle a, CacheHandle b)
   {
      TS_ASSERT_EQUALS( a->size(), b->size() );
      Cache::iterator it_a = a->begin();
      Cache::iterator it_b = b->begin();
      while ( it_a != a->end() && it_b != b->end() )
      {
         TS_ASSERT_EQUALS(it_a->first, it_b->first);
         ++it_a;
         ++it_b;
      }
      TS_ASSERT_EQUALS(it_a, a->end());
      TS_ASSERT_EQUALS(it_b, b->end());
   }


   void test_pareto_of_an_established_cache()
   {
      cache::View_Pareto pc;
      pc.set_core_cache(core_cache);

      // trivial insertions
      cache->insert(mo_app->EvalMF(domain(2,2)));
      cache->insert(mo_app->EvalMF(domain(1,1)));
      cache->insert(mo_app->EvalMF(domain(1.5,.5)));
      TS_ASSERT_EQUALS(pc.size(), 2);

      pc.set_core_cache(CacheFactory().create("Local"));
      TS_ASSERT_EQUALS(pc.size(), 0);

      pc.set_core_cache(core_cache);
      TS_ASSERT_EQUALS(pc.size(), 2);
   }


   void test_pareto_of_an_nonexistent_cache()
   {
      cache::View_Pareto pc;
      TS_ASSERT_THROWS_ASSERT
         ( pc.set_core_cache(CacheHandle()), view_error &e,
           TEST_WHAT( e, "View_Pareto::set_core_cache(): "
                      "Cannot form a view into a nonexistent Cache" ) );
   }

   void test_nested_Pareto_views()
   {
      CacheHandle cache2 = CacheFactory().create_view("Pareto", cache);
      assert_cache_equal(cache, cache2);

      // trivial insertions
      cache->insert(mo_app->EvalMF(domain(1,1)));
      TS_ASSERT_EQUALS( cache->size(), 1 );
      assert_cache_equal(cache, cache2);
      cache->insert(mo_app->EvalMF(domain(1.5,.5)));
      TS_ASSERT_EQUALS( cache->size(), 2 );
      assert_cache_equal(cache, cache2);

      // insertion triggers deletion
      cache->insert(mo_app->EvalMF(domain(.4,.4)));
      TS_ASSERT_EQUALS( cache->size(), 1 );
      assert_cache_equal(cache, cache2);

      // core deletion triggers re-insertion
      core_cache->erase
         ( cache->property("application_context").as<ApplicationHandle>().object(),
           domain(.4,.4) );
      TS_ASSERT_EQUALS( cache->size(), 2 );
      assert_cache_equal(cache, cache2);

      // clear
      core_cache->clear();
      TS_ASSERT_EQUALS( cache->size(), 0 );
      assert_cache_equal(cache, cache2);
   }

private:

   CacheHandle cache;
   CacheHandle core_cache;

   Problem<UNLP1_problem> app;
   Problem<MO_NLP1_problem> mo_app;
};

} // namespace colin
