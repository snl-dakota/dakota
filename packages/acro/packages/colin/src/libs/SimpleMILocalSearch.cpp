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
// SimpleMILocalSearch.cpp
//

#include <acro_config.h>
#include <colin/solver/SimpleMILocalSearch.h>
#include <colin/Application.h>
#include <colin/SolverMngr.h>

using namespace std;

namespace colin
{

namespace StaticInitializers {

namespace {

bool RegisterSimpleMILocalSearch()
{
   SolverMngr().declare_solver_type<SimpleMILocalSearch>
      ("colin:SimpleMILocalSearch", "A simple mixed-integer local search");

   SolverMngr().declare_solver_type<SimpleMILocalSearch>
      ("colin:ls", "An alias to colin:SimpleMILocalSearch");

   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool simple_mi_local_search =
   RegisterSimpleMILocalSearch();

} // namespace colin::StaticInitializers



SimpleMILocalSearch::SimpleMILocalSearch()
{
   init_step_length = 1.0;
   fini_step_length = 1e-4;
   bc_flag = false;

   this->reset_signal.connect
      ( boost::bind( &SimpleMILocalSearch::reset_SimpleMILocalSearch, this ) );
   //register_reset(&reset_SimpleMILocalSearch, ObjectType::get(this));
}


void SimpleMILocalSearch::reset_SimpleMILocalSearch()
{
   if ( problem.empty() )
      return;

   bc_flag = problem->enforcing_domain_bounds;
   if (bc_flag)
   {
      rlower = problem->real_lower_bounds;
      rupper = problem->real_upper_bounds;
      ilower = problem->int_lower_bounds;
      iupper = problem->int_upper_bounds;
   }
   step_length = init_step_length;
}


void SimpleMILocalSearch::optimize()
{
   //
   // Misc initialization of the optimizer
   //
   //opt_init();
   unsigned int num_iters;
   if (max_iters <= 0)
      num_iters = MAXINT;
   else
      num_iters = curr_iter + max_iters;

   size_t numBinary = problem->num_binary_vars;
   size_t numInt = problem->num_int_vars;
   size_t numReal = problem->num_real_vars; 
   //
   // Setup initial point
   //
   utilib::MixedIntVars tmp_pt( numBinary, numInt, numReal );
   utilib::MixedIntVars best_pt( numBinary, numInt, numReal );
   best_pt << initial_point;
   //
   // Evaluate the initial point
   //
   colin::AppRequest request = problem->set_domain(best_pt);
   problem->Request_F(request, best().value());
   best().response = eval_mngr().perform_evaluation(request);
   debug_io(ucout);
   //
   // Iterate
   //
   real tmp_value;
   colin::AppResponse tmp_response;
   bool improving = true;
   for (curr_iter++; curr_iter <= num_iters;  curr_iter++)
   {
      //
      // Determine if the algorithm is finished
      //
      if (check_convergence())
         break;
      //
      // After an unsuccessful evaluation of the LS neighborhood, we
      // shrink the step length if there are real variables.
      // If there are no real variables, then we have exhausted the local neighborhood,
      // so we terminate.
      //
      if (!improving)
      {
         if ( numReal > 0)
            step_length *= 0.5;
         else
         {
            solver_status.termination_condition = termination_locallyOptimal;
            break;
         }
      }
      //
      // Generate a new point
      //
      improving = false;
      DEBUGPR(1000, ucout << "Best Point: " << best_pt << std::endl;);
      for (size_t i = 0; i < numBinary; i++)
      {
         tmp_pt = best_pt;
         if (tmp_pt.Binary()(i))
            tmp_pt.Binary().reset(i);
         else
            tmp_pt.Binary().set(i);
         DEBUGPR(1000, ucout << "Trial Point: " << tmp_pt << std::endl;);
         colin::AppRequest request = problem->set_domain(tmp_pt);
         problem->Request_F(request, tmp_value);
         tmp_response = eval_mngr().perform_evaluation(request);
         DEBUGPR(1000, ucout << "Point Value: " << tmp_value << std::endl;);
         if (tmp_value < best().value())
         {
            best().value() = tmp_value;
            best().response = tmp_response;
            best().point = tmp_pt;
            best_pt = tmp_pt;
            improving = true;
            break;
         }
      }
      if (!improving)
      {
         for (size_t i = 0; i < numInt; i++)
         {
            tmp_pt = best_pt;
            int tmp = tmp_pt.Integer()[i];
            {
               tmp_pt.Integer()[i] = tmp - 1;
               if ( (problem->intLowerBoundType(i) == colin::no_bound) ||
		    ( (problem->intLowerBoundType(i) == colin::hard_bound) 
                      && ((tmp - 1) >= ilower[i]) ) )
               {
                  DEBUGPR(1000, ucout << "Trial Point: " << tmp_pt << std::endl;);
                  colin::AppRequest request = problem->set_domain(tmp_pt);
                  problem->Request_F(request, tmp_value);
                  tmp_response = eval_mngr().perform_evaluation(request);
                  DEBUGPR(1000, ucout << "Point Value: " << tmp_value << std::endl;);
                  if (tmp_value < best().value())
                  {
                     best().value() = tmp_value;
                     best().response = tmp_response;
                     best().point = tmp_pt;
                     best_pt = tmp_pt;
                     improving = true;
                     break;
                  }
               }
            }
            {
               tmp_pt.Integer()[i] = tmp + 1;
               if ( (problem->intUpperBoundType(i) == colin::no_bound) ||
		    ( (problem->intUpperBoundType(i) == colin::hard_bound) 
                      && ((tmp + 1) <= iupper[i])) )
               {
                  DEBUGPR(1000, ucout << "Trial Point: " << tmp_pt << std::endl;);
                  colin::AppRequest request = problem->set_domain(tmp_pt);
                  problem->Request_F(request, tmp_value);
                  tmp_response = eval_mngr().perform_evaluation(request);
                  DEBUGPR(1000, ucout << "Point Value: " << tmp_value << std::endl;);
                  if (tmp_value < best().value())
                  {
                     best().value() = tmp_value;
                     best().response = tmp_response;
                     best().point = tmp_pt;
                     best_pt = tmp_pt;
                     improving = true;
                     break;
                  }
               }
            }
         }
      }
      if (!improving)
      {
         for (size_t i = 0; i < numReal; i++)
         {
            tmp_pt = best_pt;
            double tmp = tmp_pt.Real()[i];
            {
               tmp_pt.Real()[i] = tmp - step_length;
               DEBUGPR(1000, ucout << "Step Down: " << 
                       (problem->realLowerBoundType(i) == colin::hard_bound)
                       << " " << (tmp - step_length) << std::endl;);
               if ( (problem->realLowerBoundType(i) == colin::no_bound) ||
                    ( (problem->realLowerBoundType(i) == colin::hard_bound) && 
                      ((tmp - step_length) >= rlower[i]))
                  )
               {
                  DEBUGPR(1000, ucout << "Trial Point: " << tmp_pt << std::endl;);
                  colin::AppRequest request = problem->set_domain(tmp_pt);
                  problem->Request_F(request, tmp_value);
                  tmp_response = eval_mngr().perform_evaluation(request);
                  DEBUGPR(1000, ucout << "Point Value: " << tmp_value << std::endl;);
                  if (tmp_value < best().value())
                  {
                     best().value() = tmp_value;
                     best().response = tmp_response;
                     best().point = tmp_pt;
                     best_pt = tmp_pt;
                     improving = true;
                     break;
                  }
               }
            }
            {
               tmp_pt.Real()[i] = tmp + step_length;
               DEBUGPR(1000, ucout << "Step Up: " << 
                       (problem->realUpperBoundType(i) == colin::hard_bound)
                       << " " << (tmp + step_length) << std::endl;);
               if ( (problem->realUpperBoundType(i) == colin::no_bound) ||
		    ( (problem->realUpperBoundType(i) == colin::hard_bound) &&
                      ((tmp + step_length) <= rupper[i])) )
               {
                  DEBUGPR(1000, ucout << "Trial Point: " << tmp_pt << std::endl;);
                  colin::AppRequest request = problem->set_domain(tmp_pt);
                  problem->Request_F(request, tmp_value);
                  tmp_response = eval_mngr().perform_evaluation(request);
                  DEBUGPR(1000, ucout << "Point Value: " << tmp_value << std::endl;);
                  if (tmp_value < best().value())
                  {
                     best().value() = tmp_value;
                     best().response = tmp_response;
                     best().point = tmp_pt;
                     best_pt = tmp_pt;
                     improving = true;
                     break;
                  }
               }
            }
         }
      }
      //
      // Debugging IO
      //
      debug_io(ucout);
   }

   final_points.add_point(problem, best().point);
   debug_io(ucout, true);
   ucout << utilib::Flush;
}


void
SimpleMILocalSearch::initialize_best_point()
{
   if (initial_point.numAllVars() > 0)
   {
      if ( problem->num_binary_vars != initial_point.numBinaryVars() )
         EXCEPTION_MNGR(std::runtime_error, "initialize_best_point - "
                        "user-provided best point has " << 
                        initial_point.numBinaryVars() << 
                        " binary variables, but the problem has " << 
                        problem->num_binary_vars << std::endl);
      if ( problem->num_int_vars != initial_point.numGeneralIntVars() )
         EXCEPTION_MNGR(std::runtime_error, "initialize_best_point - "
                        "user-provided best point has " << 
                        initial_point.numGeneralIntVars() << 
                        " general integer variables, but the problem has " << 
                        problem->num_int_vars << std::endl);
      if ( problem->num_real_vars != initial_point.numDoubleVars() )
         EXCEPTION_MNGR(std::runtime_error, "initialize_best_point - "
                        "user-provided best point has " << 
                        initial_point.numDoubleVars() << 
                        " general real variables, but the problem has " << 
                        problem->num_real_vars << std::endl);
   }

   initial_point.resize( problem->num_binary_vars, 
                         problem->num_int_vars, 
                         problem->num_real_vars );
}


std::string
SimpleMILocalSearch::define_solver_type() const
{
   return "SimpleMILocalSearch";
}


} // namespace colin

