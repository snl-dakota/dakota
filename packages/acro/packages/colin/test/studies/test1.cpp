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

#include <vector>
#include <colin/Problem.h>
#include <colin/OptApplications.h>
#include <colin/reformulation/Downcast.h>
#include <colin/reformulation/ConstraintPenalty.h>
#include <utilib/pvector.h>
#include <utilib/SparseMatrix.h>

using std::endl;
using std::cout;
using std::vector;
using namespace colin;

namespace
{

void fn1(const std::vector<double>& x, real& ans)
{
   ans = 1.0;
   for (unsigned int i = 0; i < x.size(); i++)
   { ans *= x[i]; }
}

void fn2(const std::vector<double>& x,
         int asv,
         colin::real& fval,
         utilib::BasicArray<colin::real>& constraints,
         utilib::BasicArray<colin::real>& gradval)
{
   if (asv & 1)
   {
      fval = 1.0;
      for (unsigned int i = 0; i < x.size(); i++)
      { fval *= x[i]; }
   }
   if (asv & 2)
   {
      constraints.resize(3);

      double tmp = 0.0;
      for (unsigned int i = 0; i < x.size(); i++)
      { tmp += x[i] * x[i]; }
      constraints[0] = tmp;

      tmp = 0.0;
      for (unsigned int i = 0; i < x.size(); i++)
      { tmp += (i + 1) * x[i]; }
      constraints[1] = tmp;

      tmp = 0.0;
      for (unsigned int i = 0; i < x.size(); i++)
      { tmp += x[i]; }
      constraints[2] = tmp;
   }
   if (asv & 4)
   {
      gradval.resize(x.size());
      for (unsigned int i = 0; i < x.size(); i++)
      {
         gradval[i] = 1.0;
         for (unsigned int j = 0; j < x.size(); j++)
         {
            if (i != j)
            { gradval[i] *= x[j]; }
         }
      }
   }
}

void fn3(const std::vector<double>& x,
         int asv,
         colin::real& fval,
         utilib::BasicArray<colin::real>& l_constraints,
         utilib::BasicArray<colin::real>& nl_constraints,
         utilib::BasicArray<colin::real>& gradval)
{
   if (asv & 8)
      l_constraints.clear();

   fn2(x, asv, fval, nl_constraints, gradval);
}


void fn4(const std::vector<double>& x,
         int asv,
         colin::real& fval,
         utilib::BasicArray<colin::real>& l_constraints,
         utilib::BasicArray<colin::real>& nl_constraints,
         utilib::BasicArray<colin::real>& gradval)
{
   if (asv & 8)
   {
      l_constraints.resize(3);

      double tmp = 0.0;
      for (unsigned int i = 0; i < x.size(); i++)
      { tmp += x[i]; }
      l_constraints[0] = tmp;

      tmp = 0.0;
      for (unsigned int i = 0; i < x.size(); i++)
      { tmp += (i + 1) + x[i]; }
      l_constraints[1] = tmp;

      l_constraints[2] = 5;
   }

   fn2(x, asv, fval, nl_constraints, gradval);
}

} // namespace


int test_test1a(int, char**)
{
   std::cout << std::endl;
   std::cout << "Test1a - simple application setup" << std::endl;
   colin::Problem<colin::NLP0_problem> prob;
   std::cout << "problem set up" << endl;

   std::string foo = "example1";
   colin::ConfigurableApplication<colin::NLP0_problem>* app
      = colin::new_application(foo, &fn1);
   app->num_real_vars = 3;
   utilib::pvector<double> lower(3);
   utilib::pvector<double> upper(3);
   app->real_lower_bounds = lower;
   app->real_upper_bounds = upper;

   std::cout << "application set up" << endl;
   prob.set_application(app);
   std::cout << "application assigned to problem" << endl;

   std::vector<double> vec(3);
   vec[0] = 1;
   vec[1] = 2;
   vec[2] = 4;

   colin::real ans;
   prob->EvalF(prob->eval_mngr(), vec, ans);
   std::cout << "ANS: f_info= " << ans << std::endl;
   colin::AppResponse response;
   colin::AppRequest request = prob->set_domain(vec);
   ans = 0.0;
   prob->Request_F(request, ans);
   response = prob->eval_mngr().perform_evaluation(request);
   std::cout << "ANS: f_info= " << ans << std::endl;
   std::cout << "STATUS: " << (ans == 8) << std::endl;
   std::cout << "Num Eval Servers: " << prob->eval_mngr().num_evaluation_servers() << std::endl;
   //std::cout << "Response: " << response << std::endl;

   std::cout << "deleting application" << endl;
   ApplicationMngr().unregister_application(foo);
   delete app;
   std::cout << "done." << endl;

   ApplicationMngr().clear();
   return 0;
}

int test_test1b(int, char**)
{
   std::cout << std::endl;
   std::cout << "Test1b - Setup NLP1 application and downcast it before "
      "giving it to a NLP0 problem" << std::endl;
   colin::Problem<colin::NLP0_problem> prob;
   std::cout << "problem set up" << endl;

   std::string foo = "example1";
   colin::ConfigurableApplication<colin::NLP1_problem>* app 
      = colin::new_application(foo, &fn2);
   app->num_real_vars = 3;
   utilib::pvector<double> lower(3);
   utilib::pvector<double> upper(3);
   app->real_lower_bounds = lower;
   app->real_upper_bounds = upper;

   DowncastApplication<NLP0_problem>* dapp
      = new DowncastApplication<NLP0_problem>(app);
   std::cout << "application set up" << endl;

   prob.set_application(dapp);
   //prob->print_summary(cout);
   std::cout << "application assigned to problem" << endl;

   std::vector<double> vec(3);
   vec[0] = 1;
   vec[1] = 2;
   vec[2] = 4;

   colin::real ans;
   utilib::BasicArray<colin::real> gradient;
   utilib::BasicArray<colin::real> constraints;

   //std::cout << "downcast app:" << endl;
   prob->EvalF(prob->eval_mngr(), vec, ans);
   std::cout << "ANS: f_info= " << ans << std::endl;
   std::cout << "STATUS: " << (ans == 8) << std::endl;

   //std::cout << "original app:" << endl;
   colin::Problem<colin::NLP1_problem> prob2;
   prob2.set_application(app);
   prob2->EvalF(prob2->eval_mngr(), vec, ans);
   std::cout << "ANS: f_info= " << ans << std::endl;
   std::cout << "STATUS: " << (ans == 8) << std::endl;
   prob2->EvalG(prob2->eval_mngr(), vec, gradient);
   std::cout << "ANS: g_info= " << gradient << std::endl;
   prob2->EvalCF(prob2->eval_mngr(), vec, constraints);
   std::cout << "ANS: cf_info= " << constraints << std::endl;

   //prob->print_summary(cout);
   //prob2->print_summary(cout);

   ApplicationMngr().clear();
   return 0;
}


int test_test1c(int, char**)
{
   std::cout << std::endl;
   std::cout << "Test1c - Setup NLP1 application and reformulate it to an "
   "NLP0 problem" << std::endl;
   colin::Problem<colin::NLP0_problem> prob;
   std::cout << "problem set up" << endl;

   std::string foo = "example1";
   colin::ConfigurableApplication<colin::NLP1_problem>* app
      = colin::new_application(foo, &fn2);
   app->num_real_vars = 3;
   utilib::pvector<double> lower(3);
   utilib::pvector<double> upper(3);
   app->real_lower_bounds = lower;
   app->real_upper_bounds = upper;
   std::cout << "application set up" << endl;

   colin::DowncastApplication<colin::NLP0_problem> *reformulated_app
      = new colin::DowncastApplication<colin::NLP0_problem>(app);
   std::cout << "application reformulated" << endl;

   prob.set_application(reformulated_app);
   std::cout << "application assigned to problem" << endl;

   std::vector<double> vec(3);
   vec[0] = 1;
   vec[1] = 2;
   vec[2] = 4;

   colin::real ans;
   prob->EvalF(prob->eval_mngr(), vec, ans);
   std::cout << "ANS: f_info= " << ans << std::endl;
   std::cout << "STATUS: " << (ans == 8) << std::endl;

   colin::Problem<colin::NLP1_problem> prob2;
   prob2.set_application(app);
   prob2->EvalF(prob2->eval_mngr(), vec, ans);
   std::cout << "ANS: f_info= " << ans << std::endl;
   std::cout << "STATUS: " << (ans == 8) << std::endl;
   utilib::BasicArray<colin::real> gradient;
   prob2->EvalG(prob2->eval_mngr(), vec, gradient);
   std::cout << "ANS: g_info= " << gradient << std::endl;
   utilib::BasicArray<colin::real> constraints;
   prob2->EvalCF(prob2->eval_mngr(), vec, constraints);
   std::cout << "ANS: cf_info= " << constraints << std::endl;

   // cannot call clear() because the DowncastApplication is a local
   // variable and will be deleted (again) when this function exits.
   //  - can now: dynamically allocating the application
   ApplicationMngr().clear();
   return 0;
}


int test_test1d(int, char**)
{
   std::cout << std::endl;
   std::cout << "Test1d - Setup NLP1 application and reformulate it to "
   << "eliminate constraints" << std::endl;
   //colin::Problem<colin::NLP0_problem> prob;
   //std::cout << "problem set up" << endl;

   std::string foo = "example1";
   //colin::Application<colin::NLP1_problem>* app
   colin::ConfigurableApplication< colin::NLP1_problem > * app
      = colin::new_application(foo, &fn3);
   app->num_real_vars = 3;
   app->num_nonlinear_constraints = 3;
   std::vector<real> clower(3);
   std::vector<real> cupper(3);
   clower[0] = 0.0;
   clower[1] = real::negative_infinity;
   clower[2] = 0.2;
   cupper[0] = real::positive_infinity;
   cupper[1] = 1.1;
   cupper[2] = 1.2;
   app->nonlinear_constraint_lower_bounds = clower;
   app->nonlinear_constraint_upper_bounds = cupper;
   std::cout << "application set up" << endl;

   std::vector<double> vec(3);
   vec[0] = 1;
   vec[1] = 2;
   vec[2] = 4;
   colin::real ans;

   try
   {
      utilib::exception_mngr::set_mode(utilib::exception_mngr::Exit);
      colin::Problem<colin::NLP1_problem> prob2;
      prob2.set_application(app);

      std::cout << std::endl;
      std::cout << "SUMMARY" << std::endl;
      prob2->print_summary(cout);
      std::cout << std::endl;
      prob2->EvalF(prob2->eval_mngr(), vec, ans);
      std::cout << "ANS: f_info= " << ans << std::endl;
      std::cout << "STATUS: " << (ans == 8) << std::endl;
      utilib::BasicArray<colin::real> gradient;
      prob2->EvalG(prob2->eval_mngr(), vec, gradient);
      std::cout << "ANS: g_info= " << gradient << std::endl;
      utilib::BasicArray<colin::real> constraints;
      prob2->EvalNLCF(prob2->eval_mngr(), vec, constraints);
      std::cout << "ANS: nlcf_info= " << constraints << std::endl;

      std::cout << std::endl;
      colin::ConstraintPenaltyApplication<colin::UNLP1_problem> 
         penalty_app(app);
      std::cout << "application reformulated" << endl;
      Problem<UNLP1_problem> prob;
      prob.set_application(&penalty_app);
      std::cout << "application assigned to problem" << endl;

      std::cout << std::endl;
      std::cout << "SUMMARY" << std::endl;
      prob->print_summary(cout);
      std::cout << std::endl;
      prob->EvalF(prob->eval_mngr(), vec, ans);
      std::cout << "ANS: f_info= " << ans << std::endl;
      std::cout << "STATUS: " << (ans == 8) << std::endl;
      //utilib::BasicArray<colin::real> gradient;
      prob->EvalG(prob->eval_mngr(), vec, gradient);
      std::cout << "ANS: g_info= " << gradient << std::endl;
      //utilib::BasicArray<colin::real> constraints;
      app->EvalCF(prob->eval_mngr(), vec, constraints);
      std::cout << "ANS: cf_info= " << constraints << std::endl;
   }
   catch (utilib::bad_lexical_cast& err)
   {
      std::cerr << "Exception caught:" << err.what()
      << std::endl;
      throw;
   }

   ApplicationMngr().clear();
   return 0;
}


int test_test1e(int, char**)
{
   std::cout << std::endl;
   std::cout << "Test1e - Setup NLP1 application and reformulate it to "
   << "eliminate constraints using lexical casts" << std::endl;
   //colin::Problem<colin::NLP0_problem> prob;
   //std::cout << "problem set up" << endl;

   std::string foo = "example1";
   //colin::Application<colin::NLP1_problem>* app
   colin::ConfigurableApplication< colin::NLP1_problem > * app
      = colin::new_application(foo, &fn3);
   app->num_real_vars = 3;
   app->num_nonlinear_constraints = 3;
   std::vector<real> clower(3);
   std::vector<real> cupper(3);
   clower[0] = 0.0;
   clower[1] = real::negative_infinity;
   clower[2] = 0.2;
   cupper[0] = real::positive_infinity;
   cupper[1] = 1.1;
   cupper[2] = 1.2;
   app->nonlinear_constraint_lower_bounds = clower;
   app->nonlinear_constraint_upper_bounds = cupper;
   std::cout << "application set up" << endl;

   std::vector<double> vec(3);
   vec[0] = 1;
   vec[1] = 2;
   vec[2] = 4;
   colin::real ans;

   try
   {
      utilib::exception_mngr::set_mode(utilib::exception_mngr::Exit);
      colin::Problem<colin::NLP1_problem> prob2;
      prob2.set_application(app);

      std::cout << std::endl;
      std::cout << "SUMMARY" << std::endl;
      prob2->print_summary(cout);
      std::cout << std::endl;
      prob2->EvalF(prob2->eval_mngr(), vec, ans);
      std::cout << "ANS: f_info= " << ans << std::endl;
      std::cout << "STATUS: " << (ans == 8) << std::endl;
      utilib::BasicArray<colin::real> gradient;
      prob2->EvalG(prob2->eval_mngr(), vec, gradient);
      std::cout << "ANS: g_info= " << gradient << std::endl;
      utilib::BasicArray<colin::real> constraints;
      prob2->EvalNLCF(prob2->eval_mngr(), vec, constraints);
      std::cout << "ANS: nlcf_info= " << constraints << std::endl;

      Problem<UNLP0_problem> prob;
      prob = prob2;
      std::cout << "application assigned to problem" << endl;

      std::cout << std::endl;
      std::cout << "SUMMARY" << std::endl;
      prob->print_summary(cout);
      std::cout << std::endl;
      prob->EvalF(prob->eval_mngr(), vec, ans);
      std::cout << "ANS: f_info= " << ans << std::endl;
      //utilib::BasicArray<colin::real> gradient;
      //prob->EvalG(prob->eval_mngr(),vec,gradient);
      //std::cout << "ANS: g_info= " << gradient << std::endl;
      //utilib::BasicArray<colin::real> constraints;
      //prob->EvalCF(prob->eval_mngr(),vec,constraints);
      //std::cout << "ANS: cf_info= " << constraints << std::endl;
   }
   catch (utilib::bad_lexical_cast& err)
   {
      std::cerr << "Exception caught:" << err.what()
      << std::endl;
      throw;
   }

   ApplicationMngr().clear();
   return 0;
}


int test_test1f(int, char**)
{
   std::cout << std::endl;
   std::cout << "Test1f - full test of NLP application" << std::endl;

   std::string foo = "example1";
   colin::ConfigurableApplication<colin::NLP1_problem>* app
      = colin::new_application(foo, &fn4);
   app->num_real_vars = 3;

   utilib::pvector<double> lower(3);
   utilib::pvector<double> upper(3);
   app->real_lower_bounds = lower;
   app->real_upper_bounds = upper;
   app->num_nonlinear_constraints = 3;
   app->num_linear_constraints = 3;

   std::cout << "application set up" << endl;

   std::vector<double> vec(3);
   vec[0] = 1;
   vec[1] = 2;
   vec[2] = 4;

   colin::real ans;
   app->EvalF(app->eval_mngr(), vec, ans);
   std::cout << "ANS: f_info = " << ans << std::endl;

   colin::AppResponse response;
   colin::AppRequest request = app->set_domain(vec);
   ans = 0.0;
   app->Request_F(request, ans);
   app->eval_mngr().perform_evaluation(request);
   std::cout << "ANS: f_info = " << ans << std::endl;

   vector<real> v_ans;

   v_ans.clear();
   request = app->set_domain(vec);
   app->Request_CF(request, v_ans);
   response = app->eval_mngr().perform_evaluation(request);
   cout << "cf_info = " << v_ans << endl;
   cout << "response = " << response;
   cout << "  (cf) = " << response.get(cf_info) << endl;
   cout << "  (nlcf) = " << response.get(nlcf_info) << endl;

   v_ans.clear();
   app->EvalNLCF(app->eval_mngr(), vec, v_ans);
   std::cout << "nlcf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalNLEqCF(app->eval_mngr(), vec, v_ans);
   std::cout << "nleqcf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalNLIneqCF(app->eval_mngr(), vec, v_ans);
   std::cout << "nlineqcf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalLCF(app->eval_mngr(), vec, v_ans);
   std::cout << "lcf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalLEqCF(app->eval_mngr(), vec, v_ans);
   std::cout << "leqcf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalLIneqCF(app->eval_mngr(), vec, v_ans);
   std::cout << "ineqlcf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalCF(app->eval_mngr(), vec, v_ans);
   std::cout << "cf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalEqCF(app->eval_mngr(), vec, v_ans);
   std::cout << "eqcf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalIneqCF(app->eval_mngr(), vec, v_ans);
   std::cout << "ineqcf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalCFViol(app->eval_mngr(), vec, v_ans);
   std::cout << "cvf_info =\t" << v_ans << std::endl;
   
   std::vector<real> clower(3);
   std::vector<real> cupper(3);
   clower[0] = 0.0;
   clower[1] = real::negative_infinity;
   clower[2] = 10;

   cupper[0] = real::positive_infinity;
   cupper[1] = 1;
   cupper[2] = 10;
   app->nonlinear_constraint_lower_bounds = clower;
   app->nonlinear_constraint_upper_bounds = cupper;
   app->linear_constraint_lower_bounds = clower;
   app->linear_constraint_upper_bounds = cupper;
   cout << "Set constraint bounds" << endl;

   v_ans.clear();
   app->EvalCF(app->eval_mngr(), vec, v_ans);
   cout << "cf_info =\t" << v_ans << endl;

   v_ans.clear();
   app->EvalNLCF(app->eval_mngr(), vec, v_ans);
   std::cout << "nlcf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalNLEqCF(app->eval_mngr(), vec, v_ans);
   std::cout << "nleqcf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalNLIneqCF(app->eval_mngr(), vec, v_ans);
   std::cout << "nlineqcf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalLCF(app->eval_mngr(), vec, v_ans);
   std::cout << "lcf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalLEqCF(app->eval_mngr(), vec, v_ans);
   std::cout << "leqcf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalLIneqCF(app->eval_mngr(), vec, v_ans);
   std::cout << "ineqlcf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalCF(app->eval_mngr(), vec, v_ans);
   std::cout << "cf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalEqCF(app->eval_mngr(), vec, v_ans);
   std::cout << "eqcf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalIneqCF(app->eval_mngr(), vec, v_ans);
   std::cout << "ineqcf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalCFViol(app->eval_mngr(), vec, v_ans);
   std::cout << "cvf_info =\t" << v_ans << std::endl;

   
   cout << "Set linear constraint matrix" << endl;
   utilib::RMSparseMatrix<double> mat;
   int*    itmp = new int[3];
   double* dtmp = new double[3];
   *itmp = 0;
   *dtmp = 1.0;
   mat.adjoinRow(1,itmp,dtmp);
   *itmp = 1;
   *dtmp = 2.0;
   mat.adjoinRow(1,itmp,dtmp);
   itmp[0] = 0;
   dtmp[0] = -1.0;
   itmp[1] = 2;
   dtmp[1] = 1.0;
   mat.adjoinRow(2,itmp,dtmp);
   app->linear_constraint_matrix = mat;
   //  1 . . 
   //  . 2 . 
   // -1 . 1

   v_ans.clear();
   app->EvalLCF(app->eval_mngr(), vec, v_ans);
   std::cout << "lcf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalLEqCF(app->eval_mngr(), vec, v_ans);
   std::cout << "leqcf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalLIneqCF(app->eval_mngr(), vec, v_ans);
   std::cout << "ineqlcf_info =\t" << v_ans << std::endl;
   
   v_ans.clear();
   app->EvalCF(app->eval_mngr(), vec, v_ans);
   cout << "cf_info =\t" << v_ans << endl;


   std::cout << "deleting application" << endl;
   ApplicationMngr().unregister_application(foo);
   delete app;
   std::cout << "done." << endl;

   ApplicationMngr().clear();
   return 0;
}
