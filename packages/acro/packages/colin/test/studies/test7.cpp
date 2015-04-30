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
// Solver tests - with gradients
//

#include <acro_config.h>
#include <colin/OptApplications.h>
#include <utilib/BasicArray.h>
#include <TestOptSolver.h>

using namespace colin;
using namespace std;

namespace
{

void func(const std::vector<double>& vec, int asv, colin::real& ans,
          std::vector<double>& cvec, std::vector<double>& grad)
{
   if (asv & 1)
   {
      ans = 0.0;
      for (size_t i = 0; i < vec.size(); i++)
      { ans += pow(vec[i], (int)i + 1); }
   }

   if (asv & 2)
   {
      cvec << 0.0;
   }

   if (asv & 4)
   {
      grad.resize(vec.size());

      for (size_t i = 0; i < vec.size(); i++)
      {
         if (i == 0)
         { grad[i] = (i + 1.0); }
         else
         { grad[i] = (i + 1.0) * pow(vec[i], (int)i); }
      }
   }
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
void funcA(const A& point, int asv, colin::real& ans,
           std::vector<double>& cvec, std::vector<double>& grad)
{ func(point.vec, asv, ans, cvec, grad); }


}

//
// Define necessary operator functions
//
ostream& operator<<(ostream& os, const A& a)
{ os << a.vec; return os; }

A& operator&=(A& a1, A& a2)
{ a1.vec &= a2.vec; return a1; }

istream& operator>>(istream& is, A& a)
{ is >> a.vec; return is; }


int test_test7a(int, char**)
{
   std::cout << std::endl;
   std::cout << "Test7a - a simple minimization example (Test7a+gradient)"
   << std::endl;

   ConfigurableApplication<NLP1_problem> *app
      = new_application("test7a", &func);
   app->num_real_vars = 4;
   app->set_bounds("[-10.0,10.0]^4");

   Problem<NLP1_problem> prob;
   prob.set_application(app);

   TestOptSolver_g opt;

   vector<double> point(4);
   point << 10.0;

   opt.set_initial_point(point);
   opt.set_problem(prob);
   opt["sufficient_objective_value"] = 29.0;

   opt.reset();
   opt.optimize();

   ApplicationMngr().clear();
   return 0;
}

int test_test7b(int, char**)
{
   std::cout << std::endl;
   std::cout << "Test7b - a simple minimization example with a different "
   "domain (Test7b+gradient)" << std::endl;


   utilib::TypeManager()->register_lexical_cast
      (typeid(A), typeid(vector<double>), &lexical_cast_A_vector);
   utilib::TypeManager()->register_lexical_cast
      (typeid(vector<double>), typeid(A), &lexical_cast_vector_A);

   ConfigurableApplication<NLP1_problem> *app 
      = new_application("test7b", &funcA);
   app->num_real_vars = 4;
   app->set_bounds("[-10.0,10.0]^4");

   Problem<NLP1_problem> prob;
   prob.set_application(app);

   TestOptSolver_g opt;

   vector<double> point(4);
   point << 10.0;

   opt.set_initial_point(point);
   opt.set_problem(prob);
   opt["sufficient_objective_value"] = 29.0;

   opt.reset();
   opt.optimize();

   ApplicationMngr().clear();
   return 0;
}


int test_test7c(int, char**)
{
   ColinGlobals::output_level = "quiet";
   std::cout << std::endl;
   std::cout << "Test7c - a simple minimization example with a shell command "
   "(Test7c+gradient)" << std::endl;

   ConfigurableApplication<NLP1_problem> *app = 
      new_application<std::vector<double>, NLP1_problem > 
      ( "test7c", "shell_func7", "shell_func7.in", "shell_func7.out", 
        true, true );
   app->num_real_vars = 4;
   app->set_bounds("[-10.0,10.0]^4");

   Problem<NLP1_problem> prob;
   prob.set_application(app);

   TestOptSolver_g opt;

   vector<double> point(4);
   point << 10.0;

   opt.set_initial_point(point);
   opt.set_problem(prob);
   opt["sufficient_objective_value"] = 29.0;

   opt.reset();
   opt.optimize();

   ColinGlobals::reset();
   ApplicationMngr().clear();
   return 0;
}


int test_test7d(int, char**)
{
   std::cout << std::endl;
   std::cout << "Test7d - a simple minimization example converting vectors to "
   "BasicArrays (Test7d+gradient)" << std::endl;

   ConfigurableApplication<NLP1_problem> *app
      = new_application("test7d", &func);
   app->num_real_vars = 4;
   app->set_bounds("[-10.0,10.0]^4");

   Problem<NLP1_problem> prob;
   prob.set_application(app);

   TestOptSolver_g opt;

   utilib::BasicArray<colin::real> point(4);
   point << 10.0;

   opt.set_initial_point(point);
   opt.set_problem(prob);
   opt["sufficient_objective_value"] = 29.0;

   opt.reset();
   opt.optimize();

   ApplicationMngr().clear();
   return 0;
}
