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
// Problem Evaluation test
//

#include <acro_config.h>
#include <utilib/std_headers.h>
#include <utilib/stl_auxiliary.h>
#include <colin/OptApplications.h>
#include <colin/Problem.h>

//using namespace std;
using std::endl;
using std::cout;
using std::vector;
using std::pair;
using std::ostream;
using std::istream;

using namespace colin;

namespace
{

class A
{
public:
   vector<double> vec;
};

}

ostream& operator<<(ostream& os, const A& a)
{ os << a.vec; return os; }

istream& operator>>(istream& is, A& a)
{ is >> a.vec; return is; }

namespace
{

int ctr = 0;
double func5(const double*, int n)
{
   ctr++;
   return -99.0 - ctr;
}

void fooVec()
{
   Problem<NLP0_problem> opt;
   opt.set_application(new_application("foo", &func5));

   utilib::pvector<double> a;
   real value;
   opt->EvalF(opt->eval_mngr(), a, value);
   cout << "Sync Value: " << value << endl;
   cout << "Async ID: " << opt->AsyncEvalF(opt->eval_mngr(), a, value) << endl;
   opt->synchronize();
   cout << "AsyncSync Value: " << value << endl;
}


void fooVec2()
{
   Problem<NLP0_problem> opt;
   opt.set_application(new_application("foo2", &func5));

   utilib::pvector<double> a;
   utilib::pvector<real > svalue(10);
   utilib::pvector<real > avalue(10);

   for (int i = 0; i < 10; i++)
   {
      opt->EvalF(opt->eval_mngr(), a, svalue[i]);
      opt->AsyncEvalF(opt->eval_mngr(), a, avalue[i]);
   }
   opt->synchronize();
   cout << "Sync Value: " << svalue << endl;
   cout << "AsyncSync Value: " << avalue << endl;
}

void fooVec3()
{
   Problem<NLP0_problem> opt;
   opt.set_application(new_application("foo3", &func5));

   utilib::pvector<double> a;
   real tmp;

   for (int i = 0; i < 5; i++)
   {
      cout << "queued: " << opt->AsyncEvalF(opt->eval_mngr(), a, tmp)
      << endl;
   }

   pair<EvaluationID, AppResponse> response;
   while (!(response = opt->eval_mngr().next_response()).first.empty())
   {
      cout << "return: " << response.first << ": " 
           << response.second.get(f_info)
           << endl;
   }
}

} // namespace

int test_test3(int, char**)
{
   std::cout << std::endl;
   std::cout << "Test3 - a simple validation of async evaluations"
   << std::endl;

   for (int i = 0; i < 10; i++)
   {
      fooVec();
      ApplicationMngr().clear();
   }
   fooVec2();
   fooVec3();

   ApplicationMngr().clear();
   cout << "(Test done)" << endl;
   return 0;
}

