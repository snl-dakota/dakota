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
// Solver tests - test a multi-objective solver interface
//

#include <acro_config.h>

#include <colin/OptApplications.h>
#include <colin/solver/RandomMOO.h>

using namespace colin;
using namespace std;
using utilib::seed_t;

namespace
{

void func(const utilib::Any& domain,
          const AppRequest::request_map_t& request_map,
          AppResponse::response_map_t& response_map,
          seed_t&)
{
   const std::vector<double>& vec = domain.expose<std::vector<double> > ();

   AppRequest::request_map_t::const_iterator curr = request_map.begin();
   AppRequest::request_map_t::const_iterator end  = request_map.end();
   while (curr != end)
   {
      if (curr->first == f_info)
      {
         EXCEPTION_MNGR(std::runtime_error, "We shouldn't be asking for single objective values.");
      }

      else if (curr->first == mf_info)
      {
         {
            utilib::Any ans;
            std::vector<colin::real>& tmp 
               = ans.set< std::vector<colin::real> >();
            response_map.insert(make_pair(mf_info, ans));

            tmp.resize(2);
            double val = 0.0;
            for (size_t i = 0; i < vec.size(); i++)
               val += pow(vec[i] * vec[i], (int)i + 1);
            tmp[0] = val;
            if ((val < 1e-7) && (val >= 0.0))
               tmp[1] = colin::real::positive_infinity;
            else
               tmp[1] = 1.0 / val;
         }
      }

      curr++;
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
void funcA(const utilib::Any& domain,
           const AppRequest::request_map_t& request_map,
           AppResponse::response_map_t& response_map,
           seed_t& seed)
{
   const A& tmp = domain.expose<A>();
   func(tmp.vec, request_map, response_map, seed);
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


int test_test8a(int, char**)
{
   ColinGlobals::output_level = "quiet";
   std::cout << std::endl;
   std::cout << "Test8a - a simple multi-objective minimization example" 
             << std::endl;

   ConfigurableApplication<MO_UNLP0_problem> *app 
      = new_application<std::vector<double>, MO_UNLP0_problem > 
      ( "test8a", func, 2 );
   app->num_real_vars = 4;
   app->set_bounds("[-10.0,10.0]^4");

   Problem<MO_UNLP0_problem> prob;
   prob.set_application(app);

   RandomMOO opt;

   opt.set_problem(prob);
   opt["sufficient_objective_value"] = 29.0;
   opt["seed"] = static_cast<seed_t>(1);

   opt.reset();
   opt.optimize();
   opt.write_points(std::cout);

   ApplicationMngr().clear();
   return 0;
}


int test_test8b(int, char**)
{
   ColinGlobals::output_level = "quiet";
   std::cout << std::endl;
   std::cout << "Test8b - a simple multi-objective minimization example with a different domain" << std::endl;


   utilib::TypeManager()->register_lexical_cast
      ( typeid(A), typeid(vector<double>), &lexical_cast_A_vector );
   utilib::TypeManager()->register_lexical_cast
      ( typeid(vector<double>), typeid(A), &lexical_cast_vector_A );

   ConfigurableApplication<MO_UNLP0_problem> *app 
      = new_application<A, MO_UNLP0_problem>("test8b", funcA, 2);
   app->num_real_vars = 4;
   app->set_bounds("[-10.0,10.0]^4");

   Problem<MO_UNLP0_problem> prob;
   prob.set_application(app);

   RandomMOO opt;

   opt.set_problem(prob);
   opt["sufficient_objective_value"] = 29.0;
   opt["seed"] = static_cast<seed_t>(1);

   opt.reset();
   opt.optimize();
   opt.write_points(std::cout);

   ApplicationMngr().clear();
   return 0;
}


int test_test8c(int, char**)
{
   ColinGlobals::output_level = "quiet";
   std::cout << std::endl;
   std::cout << "Test8c - a simple multi-objective minimization example with a shell command" << std::endl;

   ConfigurableApplication<MO_UNLP0_problem> *app 
      = new_application<std::vector<double>, MO_UNLP0_problem > 
      ( "test8c", "shell_func8", "shell_func8.in", "shell_func8.out",
        true, true );
   app->num_objectives = 2;
   app->num_real_vars = 4;
   app->set_bounds("[-10.0,10.0]^4");

   Problem<MO_UNLP0_problem> prob;
   prob.set_application(app);

   RandomMOO opt;

   opt.set_problem(prob);
   opt["sufficient_objective_value"] = 29.0;
   opt["seed"] = static_cast<seed_t>(1);

   opt.reset();
   opt.optimize();
   opt.write_points(std::cout);

   ColinGlobals::reset();
   ApplicationMngr().clear();
   return 0;
}


int test_test8d(int, char**)
{
   ColinGlobals::output_level = "quiet";
   std::cout << std::endl;
   std::cout << "Test8d - a simple multi-objective minimization example "
      "converting vectors to BasicArrays" << std::endl;

   ConfigurableApplication<MO_UNLP0_problem> *app 
      = new_application<std::vector<double>, MO_UNLP0_problem > 
      ( "test8d", func, 2 );
   app->num_real_vars = 4;
   app->set_bounds("[-10.0,10.0]^4");

   Problem<MO_UNLP0_problem> prob;
   prob.set_application(app);

   RandomMOO opt;

   opt.set_problem(prob);
   opt["sufficient_objective_value"] = 29.0;
   opt["seed"] = static_cast<seed_t>(1);

   opt.reset();
   opt.optimize();
   opt.write_points(std::cout);

   ApplicationMngr().clear();
   return 0;
}


