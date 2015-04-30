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
// Problem test
//

#include <acro_config.h>
#include <utilib/std_headers.h>
#include <utilib/stl_auxiliary.h>
#include <colin/OptApplications.h>
#include <colin/Problem.h>

using namespace std;
using namespace colin;

namespace
{

class A
{
public:
   vector<double> vec;

   bool operator==(const A& rhs) const
   { return vec == rhs.vec; }
   bool operator<(const A& rhs) const
   { return vec < rhs.vec; }
};

int lexical_cast_A_vector(const utilib::Any& from_, utilib::Any& to_)
{
   to_.set<vector<double> > () << (utilib::anyref_cast<A>(from_)).vec;

   return OK;
}

int lexical_cast_vector_A(const utilib::Any& from_, utilib::Any& to_)
{
//const vector<double>& from = utilib::anyref_cast<vector<double> >(from_);
//to_.set<A>();
   (to_.set<A>()).vec << utilib::anyref_cast<vector<double> > (from_);

   return OK;
}

}

ostream& operator<<(ostream& os, const A& a)
{ os << a.vec; return os; }

istream& operator>>(istream& is, A& a)
{ is >> a.vec; return is; }

namespace
{

double fn1(const std::vector<double>& x)
{
   double ans = 1.0;
   for (unsigned int i = 0; i < x.size(); i++)
      ans *= x[i];
   return ans;
}

double fn2(const double* x, int n)
{
   double ans = 1.0;
   for (int i = 0; i < n; i++)
      ans *= x[i];
   return ans;
}

#if 0
void func1(const A& x, AppResponse& response)
{ response->set(f_info, fn1(x.vec)); }

void func2(A& x, AppResponse& response)
{ response->set(f_info, fn1(x.vec)); }

void func3(const utilib::pvector<double>& x, AppResponse& response)
{ response->set(f_info, fn1(x)); }

void func4(utilib::pvector<double>& x, AppResponse& response)
{ response->set(f_info, fn1(x)); }
#endif

double func5(const double* x, int n)
{ return fn2(x, n); }

double func6(double* x, int n)
{ return fn2(x, n); }

double func11(const utilib::pvector<double>& x)
{ return fn1(x); }

double func12(utilib::pvector<double>& x)
{ return fn1(x); }


template <class FuncT>
void do_test(const char* name, FuncT fn)
{
   ConfigurableApplication<NLP0_problem> *app = new_application(name, fn);
   app->num_real_vars = 3;

   Problem<NLP0_problem> prob;
   prob.set_application(app);

   std::vector<double> vec(3);
   vec[0] = 1.0;
   vec[1] = 2.0;
   vec[2] = 4.0;
   colin::real value;
   prob->EvalF(prob->eval_mngr(), vec, value);
   std::cout << "Value: " << value << std::endl;
}

}

int test_test2(int, char**)
{
   utilib::TypeManager()->register_lexical_cast
      (typeid(A), typeid(vector<double>), &lexical_cast_A_vector);
   utilib::TypeManager()->register_lexical_cast
      (typeid(vector<double>), typeid(A), &lexical_cast_vector_A);
   std::cout << std::endl;
   std::cout << "Test2 - test direct application interfaces" << std::endl;

//do_test("func1",&func1);
//do_test("func2",&func2);
//do_test("func3",&func3);
//do_test("func4",&func4);
   do_test("func5", &func5);
   do_test("func6", &func6);
   do_test("func11", &func11);
   do_test("func12", &func12);

   ApplicationMngr().clear();
   return OK;
}
