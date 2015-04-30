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


#include "TestOptSolver.h"
#include <colin/AppResponseAnalysis.h>
#include <colin/SolverMngr.h>

using namespace std;

namespace {

const bool register_1 = SolverMngr().declare_solver_type<TestOptSolver>
   ("colin:test", "A simple test minimizer");

const bool register_2 = SolverMngr().declare_solver_type<AnotherOptSolver>
   ("colin:test_a", "A simple test minimizer using BasicArray<double> types");

const bool register_3 = SolverMngr().declare_solver_type<TestOptSolver_g>
   ("colin:test_g", "A simple test minimizer using gradients");

} // namespace (local)


void TestOptSolver::optimize()
{
   if ( problem->num_real_vars != curr.size() )
   {
      EXCEPTION_MNGR(std::runtime_error, "TestOptSolver::optimize - "
                     "curr has the wrong number of parameters ("
                     << curr.size() << " != " << problem->num_real_vars
                     << ")");
   }
   colin::real tmp;
   std::vector<colin::real> ctmp;
   std::vector<colin::real> clower, cupper;
   std::vector<colin::real> rlower, rupper;
   colin::real cviol;

   if ( problem->enforcing_domain_bounds )
   { 
      rlower = problem->real_lower_bounds;
      rupper = problem->real_upper_bounds;
   }
   clower = problem->nonlinear_constraint_lower_bounds;
   cupper = problem->nonlinear_constraint_upper_bounds;
   problem->EvalF(eval_mngr(), curr, tmp);
   if (problem->num_nonlinear_constraints > 0)
   {
      problem->EvalCF(eval_mngr(), curr, ctmp);
      constraint_violation(ctmp, clower, cupper, cviol, 1e-7);
   }
   else
      cviol = 0.0;
   opt_response.value() = tmp;

   int j = 0;
   while (!check_convergence())
   {
      for (unsigned int i = 0; i < curr.size(); i++)
      {
         if ( ! problem->enforcing_domain_bounds 
              || ((curr[i] - 0.1 + 1e-6) >= rlower[i]) )
         { curr[i] -= 0.1; }
      }
      problem->EvalF(eval_mngr(), curr, tmp);
      if (opt_response.value() > tmp)
      { opt_response.value() = tmp; }
      if (problem->num_nonlinear_constraints > 0)
      {
         problem->EvalCF(eval_mngr(), curr, ctmp);
         constraint_violation(ctmp, clower, cupper, cviol, 1e-7);
      }
      else cviol = 0.0;
      ucout << "TestOptSolver: iter " << j << " value " << opt_response.value() << " pt "
      << curr << " viol " << cviol << endl << utilib::Flush;
      j++;
   }

   opt_response.constraint_violation = real::positive_infinity;
   opt_response.point = curr;
}


void TestOptSolver_g::optimize()
{
   if ( problem->num_real_vars != curr.size() )
   {
      EXCEPTION_MNGR(std::runtime_error, "TestOptSolver::optimize - "
                     "curr has the wrong number of parameters");
   }
   colin::real tmp;
   std::vector<colin::real> grad;
   std::vector<colin::real> trial(curr.size());
   colin::real cviol;

   problem->EvalF(eval_mngr(), curr, tmp);
   problem->EvalG(eval_mngr(), curr, grad);
   opt_response.value() = tmp;

   int j = 0;
   double alpha = 1.0;
   while (!check_convergence())
   {
      problem->EvalG(eval_mngr(), curr, grad);
      for (unsigned int i = 0; i < curr.size(); i++)
      { trial[i] = curr[i] - alpha * grad[i]; }
      //ucout << "     TRIAL " << j << " " << trial << " " << grad << endl;
      problem->EvalF(eval_mngr(), trial, tmp);
      if (opt_response.value() > tmp)
      {
         opt_response.value() = tmp;
         for (unsigned int i = 0; i < curr.size(); i++)
         { curr[i] = curr[i] - alpha * grad[i]; }
      }
      else
      { alpha /= 3.0; }

      ucout << "TestOptSolver_g: " << j << " " << opt_response.value()
      << " " << curr << " " << 0.0 << endl;
      j++;
   }
   opt_response.constraint_violation = real::positive_infinity;
   opt_response.point = curr;
}

void AnotherOptSolver::optimize()
{
   if ( problem->num_real_vars != curr.size() )
   {
      EXCEPTION_MNGR(std::runtime_error, "TestOptSolver::optimize - "
                     "curr has the wrong number of parameters");
   }
   colin::real tmp;
   utilib::BasicArray<colin::real> ctmp;
   utilib::BasicArray<colin::real> clower, cupper;
   colin::real cviol;

   clower = problem->nonlinear_constraint_lower_bounds;
   cupper = problem->nonlinear_constraint_upper_bounds;
   problem->EvalF(eval_mngr(), curr, tmp);
   if (problem->num_nonlinear_constraints > 0)
   {
      problem->EvalCF(eval_mngr(), curr, ctmp);
      constraint_violation(ctmp, clower, cupper, cviol, 1e-7);
   }
   constraint_violation(ctmp, clower, cupper, cviol, 1e-7);
   opt_response.value() = tmp;

   int j = 0;
   while (!check_convergence())
   {
      for (unsigned int i = 0; i < curr.size(); i++)
      { curr[i] -= 0.1; }
      problem->EvalF(eval_mngr(), curr, tmp);
      if (problem->num_nonlinear_constraints > 0)
      {
         problem->EvalCF(eval_mngr(), curr, ctmp);
         constraint_violation(ctmp, clower, cupper, cviol, 1e-7);
      }
      if (opt_response.value() > tmp)
      { opt_response.value() = tmp; }
      constraint_violation(ctmp, clower, cupper, cviol, 1e-7);
      ucout << "AnotherOptSolver: " << j << " " << opt_response.value()
      << " " << curr << " " << cviol << endl << utilib::Flush;
      j++;
   }

   opt_response.constraint_violation = real::positive_infinity;
   opt_response.point = curr;
}
