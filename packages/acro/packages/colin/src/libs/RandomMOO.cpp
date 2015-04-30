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
// RandomMOO.cpp
//
// Randomized MOO
//

#include <acro_config.h>
#include <utilib/_math.h>
#include <utilib/stl_auxiliary.h>
#include <utilib/NumArray.h>
#include <colin/solver/RandomMOO.h>
#include <colin/SolverMngr.h>

using namespace std;

namespace colin
{

namespace StaticInitializers {

namespace {

bool RegisterRandomMOO()
{
   SolverMngr().declare_solver_type<RandomMOO>
      ("colin:RandomMOO", "A simple random multi-objective optimizer");

   SolverMngr().declare_solver_type<RandomMOO>
      ("colin:rmoo", "An alias to colin:RandomMOO");

   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool random_moo = RegisterRandomMOO();

} // namespace colin::StaticInitializers



RandomMOO::RandomMOO()
{
   this->reset_signal.connect
      ( boost::bind( &RandomMOO::reset_RandomMOO, this ) );
}


void RandomMOO::reset_RandomMOO()
{
   if ( problem.empty() )
      return;

   if (!rng)
      EXCEPTION_MNGR(runtime_error, "RandomMOO::reset_RandomMOO(): "
                     "undefined random number generator");

   urnd.generator(&rng);
   nrnd.generator(&rng);
}


void RandomMOO::optimize()
{
   //
   // Misc initialization of the optimizer
   //
   //opt_init();
   if (problem->num_real_vars == 0)
   {
      solver_status.termination_info = "No-Real-Params";
      return;
   }
   if (!(problem->finite_bound_constraints()))
   {
      solver_status.termination_info = "Missing-Bound-Constraints";
      return;
   }
   //
   // Get bounds
   //
   utilib::BasicArray<double> x_lower, x_upper;
   if ( problem->enforcing_domain_bounds )
   {
      x_lower = problem->real_lower_bounds;
      x_upper = problem->real_upper_bounds;
   }

   //
   //
   //
   unsigned int num_iters=0;
   if (max_iters <= 0)
      num_iters = 10;
   else
      num_iters = curr_iter + max_iters;

   utilib::NumArray<double> tmp_pt(problem->num_real_vars.as<size_t>());
   real constraint_value = 0.0;
   utilib::BasicArray<colin::real> values;

   debug_io(ucout);
   for (curr_iter++; curr_iter <= num_iters;  curr_iter++)
   {
      //
      // Determine if the algorithm is finished
      //
      if (check_convergence())
         break;
      //
      // Generate a random point
      //
      for (unsigned int i = 0; i < tmp_pt.size(); i++)
         tmp_pt[i] = urnd(x_lower[i], x_upper[i]);
      problem->EvalMF(eval_mngr(), tmp_pt, values);
      //perform_evaluation(tmp_pt, tmp_response, values, constraint_value);
      bool inserted = pareto_set.insert(tmp_pt, values);
      DEBUGPR(100,
              ucout << "Trying New Point" << endl;
              ucout << tmp_pt << " / " << values << endl;
              ucout << pareto_set;
              ucout << "Inserted: " << inserted << endl;
              );
      //
      // Perform random perturbation of a random cached point
      //
      do
      {
         tmp_pt << pareto_set.get_random(urnd);
         for (unsigned int i = 0; i < tmp_pt.size(); i++)
            tmp_pt[i] += nrnd();
      } while ( ! problem->testBoundsFeasibility(tmp_pt) );

      problem->EvalMF(eval_mngr(), tmp_pt, values);
      //perform_evaluation(tmp_pt, tmp_response, values, constraint_value);
      inserted = pareto_set.insert(tmp_pt, values);
      DEBUGPR(100,
              ucout << "Trying New Point" << endl;
              ucout << tmp_pt << " / " << values << endl;
              ucout << pareto_set;
              ucout << "Inserted: " << inserted << endl;
              );
      //
      // Debugging IO
      //
      debug_io(ucout);
   }

   debug_io(ucout, true);
}



std::string RandomMOO::define_solver_type() const
{
   return "RandomMOO";
}

} // namespace colin

