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

//
// Solver test
//

#include <acro_config.h>
#include <TestOptSolver.h>
#include <colin/OptApplications.h>
#include <colin/reformulation/Sampling.h>
#include <colin/ResponseFunctors.h>

#include <utilib/Uniform.h>

using namespace colin;
using namespace std;
using utilib::seed_t;

namespace
{

utilib::PM_LCG rng;
utilib::Uniform urand(&rng);

void simple_func(vector<double>& vec, colin::real& val, utilib::seed_t seed)
{
   val = seed;
   for (unsigned int i = vec.size(); i > 0; )
      val += vec[--i];
}


void func(vector<double>& vec, colin::real& val, utilib::seed_t seed)
{
   rng.set_seed(seed);
   val = 0.0;
   for (unsigned int i = 0; i < vec.size(); i++)
      val = pow(vec[i], i + 1.0) + urand();
   //cout << "func: seed=" << seed << ", val=" << val << endl;
}

//
// Define a generic datatype that contains real parameters
//
class A
{
public:
   vector<double> vec;

   operator const vector<double>&() const
      {return vec;}

   bool operator==(const A& rhs) const
   { return vec == rhs.vec; }
   bool operator<(const A& rhs) const
   { return vec < rhs.vec; }
};

int lexical_cast_A_vector(const utilib::Any& from_, utilib::Any& to_)
{
   to_.set<vector<double> > () << utilib::anyref_cast<A>(from_).vec;
   return OK;
}

int lexical_cast_vector_A(const utilib::Any& from_, utilib::Any& to_)
{
   to_.set<A>().vec << utilib::anyref_cast<vector<double> > (from_);
   return OK;
}

//
// Define an objective function over A.
//
void funcA(A& point, colin::real& val, utilib::seed_t seed)
{
   func(point.vec, val, seed);
}

void print_f(std::ostream &os, AppResponse response, std::string name)
{
   assert(0&&"This test needs to be revisited after response/cache split");
#if 0
   double d;
   std::vector<utilib::Any> data;
   std::vector<seed_t> seeds;
   response->get_all(colin::f_info, data, seeds);

   os << name << ": [";
   for( size_t i = 0; i < data.size(); ++i )
   {
      utilib::TypeManager()->lexical_cast(data[i], d);
      os << " " << d << ":" << seeds[i];
   }
   os << " ]" << std::endl;
#endif
}

///
class DebugMeanDoubleFunctor : public MeanDoubleFunctor
{
public:

   utilib::Any operator()(utilib::Any state, utilib::Any value)
   {
      cout << "  sample: " << value << endl;
      return MeanDoubleFunctor::operator()(state, value);
   }

   utilib::Any result(utilib::Any state)
   {
      utilib::Any ans = MeanDoubleFunctor::result(state);
      cout << "mean = " << ans << endl;
      return ans;
   }
};

} // namespace


int test_test9a(int, char**)
{
   std::cout << std::endl;
   std::cout << "Test9a - a simple minimization example" << std::endl;

   ConfigurableApplication<SNLP0_problem> *app
      = new_application("test9a", func);
   app->num_real_vars = 3;
   app->set_bounds("[-10.0,10.0]^3");
   app->nond_objective = true;

   Problem<SNLP0_problem> prob;
   prob.set_application(app);

   TestOptSolver opt;
   rng.set_seed(100);
   opt.set_rng(&rng);

   vector<double> point(3);
   point << 4.0;

   opt.set_initial_point(point);
   opt.set_problem(prob);
   opt["sufficient_objective_value"] = 29.0;

   opt.reset();
   opt.optimize();

   std::cout << "NEval: " << prob->eval_count() << std::endl;

   ApplicationMngr().clear();
   return 0;
}


int test_test9b(int, char**)
{
   std::cout << std::endl;
   std::cout << "Test9b - a simple minimization example with a different domain" << std::endl;


   utilib::TypeManager()->register_lexical_cast
      (typeid(A), typeid(vector<double>), &lexical_cast_A_vector);
   utilib::TypeManager()->register_lexical_cast
      (typeid(vector<double>), typeid(A), &lexical_cast_vector_A);

   ConfigurableApplication<SNLP0_problem> *app
      = new_application("test9b", funcA);
   app->num_real_vars = 3;
   app->set_bounds("[-10.0,10.0]^3");
   app->nond_objective = true;

   Problem<SNLP0_problem> prob;
   prob.set_application(app);

   TestOptSolver opt;
   rng.set_seed(100);
   opt.set_rng(&rng);

   vector<double> point(3);
   point << 4.0;

   opt.set_initial_point(point);
   opt.set_problem(prob);
   opt["sufficient_objective_value"] = 29.0;

   opt.reset();
   opt.optimize();

   std::cout << "NEval: " << prob->eval_count() << std::endl;

   ApplicationMngr().clear();
   return 0;
}


int test_test9c(int, char**)
{
   ColinGlobals::output_level = "quiet";
   std::cout << std::endl;
   std::cout << "Test9c - a simple minimization example with a shell command" 
             << std::endl;

   ConfigurableApplication<SNLP0_problem> *app
      = new_application<std::vector<double>, SNLP0_problem > 
      ( "test75", "shell_func9", "shell_func9.in", "shell_func9.out",
        true, true );
   app->num_real_vars = 3;
   app->set_bounds("[-10.0,10.0]^3");
   app->nond_objective = true;

   Problem<SNLP0_problem> prob;
   prob.set_application(app);

   TestOptSolver opt;
   rng.set_seed(100);
   opt.set_rng(&rng);

   vector<double> point(3);
   point << 4.0;

   opt.set_initial_point(point);
   opt.set_problem(prob);
   opt["sufficient_objective_value"] = 29.0;

   opt.reset();
   opt.optimize();

   std::cout << "NEval: " << prob->eval_count() << std::endl;

   ColinGlobals::reset();
   ApplicationMngr().clear();
   return 0;
}


int test_test9d(int, char**)
{
   std::cout << std::endl;
   std::cout << "Test9d - a simple minimization example converting vectors "
      "to BasicArrays" << std::endl;

   ConfigurableApplication<SNLP0_problem> *app
      = new_application("test9d", func);
   app->num_real_vars = 3;
   app->set_bounds("[-10.0,10.0]^3");
   app->nond_objective = true;

   Problem<SNLP0_problem> prob;
   prob.set_application(app);

   TestOptSolver opt;
   rng.set_seed(100);
   opt.set_rng(&rng);

   vector<double> point(3);
   point << 4.0;

   opt.set_initial_point(point);
   opt.set_problem(prob);
   opt["sufficient_objective_value"] = 29.0;

   opt.reset();
   opt.optimize();

   std::cout << "NEval: " << prob->eval_count() << std::endl;

   ApplicationMngr().clear();
   return 0;
}


int test_test9e(int, char**)
{
// deactivating this test due to the rewrite of the
// AppRequest/AppResponse system
#if 0
   std::cout << std::endl;
   std::cout << "Test9e - response merging" << std::endl;


   ConfigurableApplication<SNLP0_problem> *app
      = new_application("test9a", simple_func);
   app->num_real_vars = 2;
   app->set_bounds("[-10.0,10.0]^2");
   app->nond_objective = true;

   Problem<SNLP0_problem> prob;
   prob.set_application(app);

   vector<double> point1(2);
   point1 << 4.0;

   vector<double> point2(2, 1);
   point2[1] = 2;

   colin::AppRequest req = prob->set_domain(point1, false, 50);
   prob->Request_F(req);
   colin::AppResponse r1 = prob->eval_mngr().perform_evaluation(req);
   print_f(std::cout, r1, "r1");

   req = prob->set_domain(point2, false, 50);
   prob->Request_F(req);
   colin::AppResponse r2 = prob->eval_mngr().perform_evaluation(req);
   print_f(std::cout, r2, "r2");

   try {
      r1.merge_response(r2);
   } catch (std::exception& err) {
      // We need to remove the file/line number from the exception
      // (otherwise simply adding a comment line to the source file
      // would cause the test to fail).
      std::string tmp = err.what();
      std::cout << "Exception caught:" << std::endl << "   " 
                << tmp.substr(tmp.find(": ")+2) << std::endl;
   }

   req = prob->set_domain(point1, false, 150);
   prob->Request_F(req);
   colin::AppResponse r1a = prob->eval_mngr().perform_evaluation(req);
   print_f(std::cout, r1a, "r1a");

   std::cout << "Merging r1, r1a:" << std::endl;
   r1->merge_response(r1a);
   print_f(std::cout, r1, "r1");
   print_f(std::cout, r1a, "r1a");

   req = prob->set_domain(point1);
   colin::AppResponse r1b;
   prob->Request_F(req);
   prob->Request_response(req,r1b);
   
   std::cout << "Merging r1, (empty) r1b:" << std::endl;
   r1->merge_response(r1b);
   print_f(std::cout, r1, "r1");
   print_f(std::cout, r1b, "r1b");

   // this should NOT increment the neval...
   size_t old_neval = prob->eval_count();
   prob->eval_mngr().perform_evaluation(req);
   std::cout << "check computation bypass: " 
             << ( old_neval == prob->eval_count() ) << std::endl;

   req = prob->set_domain(point1, false, 100);
   colin::AppResponse r1c;
   prob->Request_F(req);
   prob->Request_response(req,r1c);
   
   std::cout << "Merging (empty)r1c, r1:" << std::endl;
   r1c->merge_response(r1);
   print_f(std::cout, r1c, "r1c");
   print_f(std::cout, r1, "r1");

   // this should NOT increment the neval...
   old_neval = prob->eval_count();
   req->compute_response();
   std::cout << "check computation bypass: " 
             << ( old_neval + 1 == prob->eval_count() ) << std::endl;

   print_f(std::cout, r1c, "r1c");
   print_f(std::cout, r1, "r1");

   std::cout << "Merging r1c, r1:" << std::endl;
   r1c->merge_response(r1);
   print_f(std::cout, r1c, "r1c");
   print_f(std::cout, r1, "r1");

   std::cout << "NEval: " << prob->eval_count() << std::endl;

   ApplicationMngr().clear();
#endif // 0
   return 0;
}


int test_test9f(int, char**)
{
   std::cout << std::endl;
   std::cout << "Test9_ - test the sampling application" << std::endl;

   ConfigurableApplication<SNLP0_problem> *app
      = new_application("test9a", func);
   app->num_real_vars = 3;
   app->set_bounds("[-10.0,10.0]^3");
   app->nond_objective = true;


   Problem<SNLP0_problem> prob;
   prob.set_application(app);

   SamplingApplication<NLP0_problem> *sa 
      = new SamplingApplication<NLP0_problem>;
   sa->reformulate_application(prob.operator->());
   sa->setObjectiveFunctor(new DebugMeanDoubleFunctor());
   utilib::PM_LCG pm_rng;
   sa->set_rng(utilib::AnyRNG(&pm_rng));
   sa->sample_size = 4;

   double ans;
   vector<double> point(3);
   vector<int> int_point(3);
   point << 4.0;
   int_point << 4;

   AppRequest req = sa->set_domain(point, true, 12345);
   sa->Request_F(req, ans);
   sa->eval_mngr().perform_evaluation(req);
   cout << "EvalF = " << ans << endl;

   req = sa->set_domain(int_point, true, 54321);
   sa->Request_F(req, ans);
   sa->eval_mngr().perform_evaluation(req);
   cout << "EvalF = " << ans << endl;

   req = sa->set_domain(int_point, true, 12345);
   sa->Request_F(req, ans);
   sa->eval_mngr().perform_evaluation(req);
   cout << "EvalF = " << ans << endl;

   std::cout << "NEval: " << prob->eval_count() << std::endl;

   ApplicationMngr().clear();
   return 0;
}


