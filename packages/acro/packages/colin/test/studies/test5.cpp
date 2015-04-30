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

#include <colin/OptApplications.h>

#include <TestOptSolver.h>

using namespace colin;
using namespace std;

namespace
{

double func(vector<double>& vec)
{
   double val = 0.0;
   for (unsigned int i = 0; i < vec.size(); i++)
      val = (i + 1) * vec[i];
   return val;
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
double funcA(A& point)
{
   double val;
   for (unsigned int i = 0; i < point.vec.size(); i++)
      val = (i + 1) * point.vec[i];
   return val;
}

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


int test_test5a(int, char**)
{
   std::cout << std::endl;
   std::cout << "Test5a - a simple minimization example" << std::endl;

   ConfigurableApplication<NLP0_problem> *app
      = new_application("test5a", func);
   app->num_real_vars = 3;
   app->set_bounds("[-10.0,10.0]^3");

   Problem<NLP0_problem> prob;
   prob.set_application(app);

   TestOptSolver opt;

   vector<double> point(3);
   point << 10.0;

   opt.set_initial_point(point);
   opt.set_problem(prob);
   opt["sufficient_objective_value"] = 29.0;

   opt.reset();
   opt.optimize();

   ApplicationMngr().clear();
   return 0;
}


int test_test5b(int, char**)
{
   std::cout << std::endl;
   std::cout << "Test5b - a simple minimization example with a different domain" << std::endl;


   utilib::TypeManager()->register_lexical_cast
      (typeid(A), typeid(vector<double>), &lexical_cast_A_vector);
   utilib::TypeManager()->register_lexical_cast
      (typeid(vector<double>), typeid(A), &lexical_cast_vector_A);

   ConfigurableApplication<NLP0_problem> *app 
      = new_application("test5b", funcA);
   app->num_real_vars = 3;
   app->set_bounds("[-10.0,10.0]^3");

   Problem<NLP0_problem> prob;
   prob.set_application(app);

   TestOptSolver opt;

   vector<double> point(3);
   point << 10.0;

   opt.set_initial_point(point);
   opt.set_problem(prob);
   opt["sufficient_objective_value"] = 29.0;

   opt.reset();
   opt.optimize();

   ApplicationMngr().clear();
   return 0;
}


int test_test5c(int, char**)
{
   ColinGlobals::output_level = "quiet";
   std::cout << std::endl;
   std::cout << "Test5c - a simple minimization example with a shell command" << std::endl;

   ConfigurableApplication<NLP0_problem> *app 
      = new_application<std::vector<double>, NLP0_problem > 
      ( "test75", "shell_func5", "shell_func5.in", "shell_func5.out", 
        true, true );
   app->num_real_vars = 3;
   app->set_bounds("[-10.0,10.0]^3");

   Problem<NLP0_problem> prob;
   prob.set_application(app);

   TestOptSolver opt;

   vector<double> point(3);
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


int test_test5d(int, char**)
{
   std::cout << std::endl;
   std::cout << "Test5d - a simple minimization example converting vectors to BasicArrays" << std::endl;

   ConfigurableApplication<NLP0_problem> *app
      = new_application("test5d", func);
   app->num_real_vars = 3;
   app->set_bounds("[-10.0,10.0]^3");

   Problem<NLP0_problem> prob;
   prob.set_application(app);

   TestOptSolver opt;

   vector<double> point(3);
   point << 10.0;

   opt.set_initial_point(point);
   opt.set_problem(prob);
   opt["sufficient_objective_value"] = 29.0;

   opt.reset();
   opt.optimize();

   ApplicationMngr().clear();
   return 0;
}


