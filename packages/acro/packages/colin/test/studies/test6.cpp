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
// Solver tests - with constraints
//

#include <acro_config.h>

#include <colin/OptApplications.h>

#include <TestOptSolver.h>

using namespace colin;
using namespace std;

namespace
{

void func(const vector<double>& vec, colin::real& val, vector<colin::real>& cvals)
{
   val = 0.0;
   for (unsigned int i = 0; i < vec.size(); i++)
      val = (i + 1) * vec[i];

   cvals.resize(vec.size());
   for (unsigned int i = 0; i < vec.size(); i++)
      cvals[i] = i + 1.0 + vec[i] * vec[i];
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
void funcA(const A& point, colin::real& val, vector<colin::real>& cvals)
{
   val = 0.0;
   for (unsigned int i = 0; i < point.vec.size(); i++)
      val = (i + 1) * point.vec[i];

   cvals.resize(point.vec.size());
   for (unsigned int i = 0; i < point.vec.size(); i++)
      cvals[i] = i + 1.0 + point.vec[i] * point.vec[i];
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


int test_test6a(int, char**)
{
   std::cout << std::endl;
   std::cout << "Test6a - a simple minimization example (Test5a+constraints)" << std::endl;

   colin::DirectFuncApplication< NLP0_problem > * app
      = new_application("test6a", func);
   app->num_real_vars = 4;
   app->num_nonlinear_constraints = 4;
   app->set_bounds("[-10.0,10.0]^4");
   std::vector<colin::real> clower(4), cupper(4);
   clower << 0.0;
   cupper << 1.0;
   app->nonlinear_constraint_lower_bounds = clower;
   app->nonlinear_constraint_upper_bounds = cupper;

   Problem<NLP0_problem> prob;
   prob.set_application(app);

   TestOptSolver opt;

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


int test_test6b(int, char**)
{
   std::cout << std::endl;
   std::cout << "Test6b - a simple minimization example with a different domain (Test5b+constraints)" << std::endl;


   utilib::TypeManager()->register_lexical_cast
      (typeid(A), typeid(vector<double>), &lexical_cast_A_vector);
   utilib::TypeManager()->register_lexical_cast
      (typeid(vector<double>), typeid(A), &lexical_cast_vector_A);

   colin::DirectFuncApplication< NLP0_problem > * app
      = new_application("test6a", funcA);

   app->num_real_vars = 4;
   app->num_nonlinear_constraints = 4;
   app->set_bounds("[-10.0,10.0]^4");
   std::vector<colin::real> clower(4), cupper(4);
   clower << 0.0;
   cupper << 1.0;
   app->nonlinear_constraint_lower_bounds = clower;
   app->nonlinear_constraint_upper_bounds = cupper;

   Problem<NLP0_problem> prob;
   prob.set_application(app);

   TestOptSolver opt;

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


int test_test6c(int, char**)
{
   ColinGlobals::output_level = "quiet";
   std::cout << std::endl;
   std::cout << "Test6c - a simple minimization example with a shell command (Test5c+constraints)" << std::endl;

   AnalysisCodeApplication<NLP1_problem, std::vector<double> > * app 
      = new_application<std::vector<double>, NLP1_problem > 
      ("test6c", "shell_func5", "shell_func5.in", "shell_func5.out", 
       true, true);

   app->num_real_vars = 4;
   app->num_nonlinear_constraints = 4;
   app->set_bounds("[-10.0,10.0]^4");
   std::vector<colin::real> clower(4), cupper(4);
   clower << 0.0;
   cupper << 1.0;
   app->nonlinear_constraint_lower_bounds = clower;
   app->nonlinear_constraint_upper_bounds = cupper;

   Problem<NLP1_problem> prob;
   prob.set_application(app);

   TestOptSolver opt;

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


int test_test6d(int, char**)
{
   std::cout << std::endl;
   std::cout << "Test6d - a simple minimization example converting vectors to BasicArrays (Test5d+constraints)" << std::endl;


   colin::DirectFuncApplication< NLP0_problem > * app 
      = new_application("test6a", func);

   app->num_real_vars = 4;
   app->num_nonlinear_constraints = 4;
   app->set_bounds("[-10.0,10.0]^4");
   std::vector<colin::real> clower(4), cupper(4);
   clower << 0.0;
   cupper << 1.0;
   app->nonlinear_constraint_lower_bounds = clower;
   app->nonlinear_constraint_upper_bounds = cupper;

   Problem<NLP0_problem> prob;
   prob.set_application(app);

   TestOptSolver opt;

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


