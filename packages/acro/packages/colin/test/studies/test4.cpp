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

#include<colin/cache/Factory.h>
#include<colin/OptApplications.h>
#include<colin/Problem.h>

#include <utilib/default_rng.h>

#include <limits>

using namespace colin;

using utilib::Any;
using utilib::seed_t;

#define OUT std::cout
using std::endl;

using std::vector;
using std::list;
using std::make_pair;


#define EVAL_F(APP, DOMAIN, SEED, RESPONSE, ANS)                        \
   if ( true )                                                          \
   {                                                                    \
      AppRequest r = APP->set_domain(DOMAIN, false, SEED);              \
      APP->Request_F(r, ANS);                                           \
      RESPONSE = APP->eval_mngr().perform_evaluation(r);                \
      OUT << "  [" << APP->eval_count() << "," << APP->response_count() \
          << "] f(" QUOTE(DOMAIN) ", " QUOTE(SEED) ") == " << ANS << endl; \
   } else static_cast<void>(0)

#define QUICK_EVAL_F(APP, DOMAIN, SEED, ANS)            \
   EVAL_F(APP, DOMAIN, SEED, AppResponse response, ANS)
   

namespace {

void driver(const Any& domain,
            const AppRequest::request_map_t& request_map,
            AppResponse::response_map_t& response_map,
            seed_t& seed)
{
   const double& x = domain.expose<double>();
   OUT << "  ( ";
   if ( request_map.find(f_info) != request_map.end() )
   {
      OUT << "f ";
      response_map.insert(make_pair(f_info, seed + x));
   }
   if ( request_map.find(nlcf_info) != request_map.end() )
   {
      OUT << "nlcf ";
      response_map.insert(make_pair(nlcf_info, 100 + seed));
   }
   if ( request_map.find(lcf_info) != request_map.end() )
   {
      OUT << "lcf ";
      response_map.insert(make_pair(lcf_info, vector<double>()));
   }
   OUT << ")" << endl;
}

} // namespace (local)



int test_test4(int, char**)
{
   OUT << "Test LocalCache & SerialEvaluator with nondeterministic application"
       << endl;
   OUT << "*** None ***" << endl;
   EvaluationManager_Base* mngr = EvalManagerFactory().create("Serial");
   mngr->set_evaluation_cache(CacheFactory().create("None"));

   ConfigurableApplication<SNLP0_problem> *app
      = new_application<double, SNLP0_problem>("test", driver, 1, 0);
   app->set_evaluation_manager(mngr);

   double f;
   app->EvalF(app->eval_mngr(), 0.1, f);
   OUT << "  [" << app->eval_count() << "," << app->response_count()
       << "] f(0.1, -) == " << f << endl;

   QUICK_EVAL_F(app, 0.1, 0, f);
   QUICK_EVAL_F(app, 0.1, 0, f);
   QUICK_EVAL_F(app, 0.1, 10, f);

   OUT << "*** Local::Unique ***" << endl;
   mngr->set_evaluation_cache(CacheFactory().create("Local", "Unique"));

   QUICK_EVAL_F(app, 0.1, 0, f);
   QUICK_EVAL_F(app, 0.1, 0, f);
   QUICK_EVAL_F(app, 0.1, 10, f);

   OUT << "*** Local::Exact ***" << endl;
   mngr->set_evaluation_cache(CacheFactory().create("Local", "Exact"));

   QUICK_EVAL_F(app, 0.1, 0, f);
   QUICK_EVAL_F(app, 0.1, 0, f);
   QUICK_EVAL_F(app, 0.1, 10, f);
   QUICK_EVAL_F(app, 0.1, 10, f);

   app->EvalF(app->eval_mngr(), 0.1, f);
   OUT << "  [" << app->eval_count() << "," << app->response_count()
       << "] f(0.1, -) == " << f << endl;

   QUICK_EVAL_F(app, 1, 0, f);
   QUICK_EVAL_F(app, 1 + std::numeric_limits<double>::epsilon(), 0, f);
   QUICK_EVAL_F(app, 1 - std::numeric_limits<double>::epsilon(), 0, f);

   OUT << "*** Local::Epsilon ***" << endl;
   mngr->set_evaluation_cache(CacheFactory().create("Local", "Epsilon"));

   QUICK_EVAL_F(app, 1, 0, f);
   QUICK_EVAL_F(app, 1 + std::numeric_limits<double>::epsilon(), 0, f);
   QUICK_EVAL_F(app, 1 - std::numeric_limits<double>::epsilon(), 0, f);

   // testing "partial" function evaluation (f is in cache, cf is not)
   {
      std::vector<double> cf;
      AppRequest r = app->set_domain(1, false, 0);
      app->Request_F(r, f);
      app->Request_CF(r, cf);
      app->eval_mngr().perform_evaluation(r);
      OUT << "  [" << app->eval_count() << "," << app->response_count()
          << "] f(1, 0) == " << f << "; cf = " << cf << endl;
   }

   // testing multiple request function evaluation (f, cf is not in cache)
   {
      std::vector<double> cf;
      AppRequest r = app->set_domain(1, false, 10);
      app->Request_F(r, f);
      app->Request_CF(r, cf);
      app->eval_mngr().perform_evaluation(r);
      OUT << "  [" << app->eval_count() << "," << app->response_count()
          << "] f(1, 0) == " << f << "; cf = " << cf << endl;
   }

   // testing multiple cache-hit request
   {
      std::vector<double> cf;
      AppRequest r = app->set_domain(1, false, 10);
      app->Request_F(r, f);
      app->Request_CF(r, cf);
      app->eval_mngr().perform_evaluation(r);
      OUT << "  [" << app->eval_count() << "," << app->response_count()
          << "] f(1, 0) == " << f << "; cf = " << cf << endl;
   }


   OUT << "*** Wrapped application ***" << endl;
   Problem<UNLP0_problem> prob;
   prob = Problem<SNLP0_problem>(app);
   prob->set_rng(&utilib::default_rng);
   CacheHandle cache = CacheFactory().create("Local", "Epsilon");
   mngr->set_evaluation_cache(cache);

   QUICK_EVAL_F(prob, 0, 1, f);
   QUICK_EVAL_F(prob, 0, 1, f);
   // this should return a non-zero f_info from the cache (16807)
   QUICK_EVAL_F(app, 0, 0, f);

   // This should compute N new evaluations
   QUICK_EVAL_F(prob, 0, 10, f);
   QUICK_EVAL_F(app, 0, 0, f);

   OUT << "*** Seed == 0 ***" << endl;
   // this should return a non-zero f_info from the cache (16807)
   QUICK_EVAL_F(app, 0, 0, f);
   // this should compute a new value (100)
   QUICK_EVAL_F(app, 0, 100, f);
   // this should return the new value (100)
   QUICK_EVAL_F(app, 0, 0, f);
   // this should return the older value (16807)
   {
      std::vector<double> cf;
      AppRequest r = app->set_domain(0);
      app->Request_F(r, f);
      app->Request_CF(r, cf);
      app->eval_mngr().perform_evaluation(r);
      OUT << "  [" << app->eval_count() << "," << app->response_count()
          << "] f(0, 0) == " << f << "; cf = " << cf << endl;
   }
   // this should return the older value (16807)
   {
      std::vector<double> lcf;
      std::vector<double> cf;
      AppRequest r = app->set_domain(0);
      app->Request_F(r, f);
      app->Request_CF(r, cf);
      app->Request_LCF(r, lcf);
      app->eval_mngr().perform_evaluation(r);
      OUT << "  [" << app->eval_count() << "," << app->response_count()
          << "] f(0, 0) == " << f << "; cf = " << cf 
          << "; lcf = " << lcf << endl;
   }

   OUT << "*** Get AppResponses ***" << endl;
   // this should only return 1 value (0, 100)
   {
      OUT << "ALL:" << endl;

      Cache::iterator it = cache->begin(app);
      for( ; it != cache->end(); ++it )
         OUT << it->second.asResponse(app);
   }
   // Add some more values to the cache
   {
      app->EvalF(app->eval_mngr(), 100);
      app->EvalF(app->eval_mngr(), -100);
      app->EvalCF(app->eval_mngr(), 200);
      app->EvalLCF(app->eval_mngr(), 300);

      AppRequest r = app->set_domain(100, false, 100);
      app->Request_F(r);
      app->Request_CF(r);
      app->eval_mngr().perform_evaluation(r);
   }
   // This should return everything
   {
      OUT << "ALL:" << endl;
      
      Cache::iterator it = cache->begin(app);
      for( ; it != cache->end(); ++it )
         OUT << it->second.asResponse(app);
   }
#if 0 // deprecated because of the change in the cache semantics
   // This should give 3 F's
   {
      OUT << "F:" << endl;
      AppRequest r = app->set_domain(Any());
      app->Request_F(r);

      Cache::iterator it = cache->find(r);
      for( ; it != cache->end(); ++it )
         OUT << it->second.asResponse(app);
   }
   // This should give 3 CF's
   {
      OUT << "CF:" << endl;
      AppRequest r = app->set_domain(Any());
      app->Request_CF(r);

      Cache::iterator it = cache->find(r);
      for( ; it != cache->end(); ++it )
         OUT << it->second.asResponse(app);
   }
   // This is looking for a specific domain point
   {
      OUT << "F(0):" << endl;
      AppRequest r = app->set_domain(0);
      app->Request_F(r);

      Cache::iterator it = cache->find(r);
      for( ; it != cache->end(); ++it )
         OUT << it->second.asResponse(app);
   }
   {
      OUT << "CF(0):" << endl;
      AppRequest r = app->set_domain(0);
      app->Request_CF(r);

      Cache::iterator it = cache->find(r);
      for( ; it != cache->end(); ++it )
         OUT << it->second.asResponse(app);
   }
   {
      OUT << "EqCF(0):" << endl;
      AppRequest r = app->set_domain(0);
      app->Request_EqCF(r);

      Cache::iterator it = cache->find(r);
      for( ; it != cache->end(); ++it )
         OUT << it->second.asResponse(app);
   }
   {
      OUT << "EqCF(1):" << endl;
      AppRequest r = app->set_domain(1);
      app->Request_EqCF(r);

      Cache::iterator it = cache->find(r);
      for( ; it != cache->end(); ++it )
         OUT << it->second.asResponse(app);
   }
   {
      OUT << "EqCF(200):" << endl;
      AppRequest r = app->set_domain(200);
      app->Request_EqCF(r);

      Cache::iterator it = cache->find(r);
      for( ; it != cache->end(); ++it )
         OUT << it->second.asResponse(app);
   }
   {
      OUT << "EqCF(100):" << endl;
      AppRequest r = app->set_domain(100);
      app->Request_EqCF(r);

      Cache::iterator it = cache->find(r);
      for( ; it != cache->end(); ++it )
         OUT << it->second.asResponse(app);
   }
   {
      OUT << "*(*,100):" << endl;
      AppRequest r = app->set_domain(Any(), false, 100);

      Cache::iterator it = cache->find(r);
      for( ; it != cache->end(); ++it )
         OUT << it->second.asResponse(app);
   }
#endif
   return 0;
}


