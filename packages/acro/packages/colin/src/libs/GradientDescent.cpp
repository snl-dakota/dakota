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
// GradientDescent.cpp
//

#include <acro_config.h>

#include <colin/solver/GradientDescent.h>
#include <colin/SolverMngr.h>

#include <utilib/_math.h>

using namespace std;

namespace colin
{

namespace StaticInitializers {

namespace {

bool RegisterGradientDescent()
{
   SolverMngr().declare_solver_type<GradientDescent>
      ("colin:GradientDescent", "A simple gradient descent local search");

   SolverMngr().declare_solver_type<GradientDescent>
      ("colin:gd", "An alias to colin:GradientDescent");

   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool gradient_descent = RegisterGradientDescent();

} // namespace colin::StaticInitializers




GradientDescent::GradientDescent()
{
   this->reset_signal.connect
      ( boost::bind( &GradientDescent::reset_GradientDescent, this ) );

   bc_flag = false;
}


void GradientDescent::reset_GradientDescent()
{
   if ( problem.empty() ) 
      return;

   bc_flag = problem->enforcing_domain_bounds;
   if (bc_flag)
   {
      rlower = problem->real_lower_bounds;
      rupper = problem->real_upper_bounds;
   }
}


void GradientDescent::optimize()
{
   //
   // Misc initialization of the optimizer
   //
   unsigned int num_iters;
   if (max_iters <= 0)
      num_iters = MAXINT;
   else
      num_iters = curr_iter + max_iters;
   //
   // Setup initial point
   //
   utilib::BasicArray<double> tmp_pt(problem->num_real_vars.as<size_t>());
   utilib::BasicArray<double> best_pt(problem->num_real_vars.as<size_t>());
   best_pt << initial_point;
   //
   // Evaluate the initial point
   //
   colin::AppRequest request = problem->set_domain(best_pt);
   problem->Request_F(request, best().value());
   problem->Request_G(request, grad);
   best().response = eval_mngr().perform_evaluation(request);
   debug_io(ucout);
   //
   // Iterate
   //
   real tmp_value;
   colin::AppResponse tmp_response;
   //bool improving = true;
   double step = 1.0;
   for (curr_iter++; curr_iter <= num_iters;  curr_iter++)
   {
      //
      // Determine if the algorithm is finished
      //
      if (check_convergence())
         break;
      //
      // Step in the direction of the negative gradient until we find an
      // improving step.
      //
      bool contracting=true;
      bool success=false;
      real tmp_fval;
      tmp_pt << best_pt;
	for (size_t i=0; i<tmp_pt.size(); i++) {
	  tmp_pt[i] = best_pt[i] - grad[i]*step;
	  }
      problem->EvalF(eval_mngr(),tmp_pt,tmp_fval);
      if (tmp_fval > best().value()) {
         contracting=true;
      }
      while (true) {
        if (contracting) {
	   step /= 2.0;
	} else {
	    step *= 2.0;
	}
        if (step > 10e6) {success=true; break;}
        if (step < 1e-16){success=(tmp_fval < best().value()); break;}
	tmp_pt << best_pt;
	for (size_t i=0; i<tmp_pt.size(); i++) {
	  tmp_pt[i] = best_pt[i] - grad[i]*step;
	  }
        problem->EvalF(eval_mngr(),tmp_pt,tmp_fval);
	DEBUGPR(100,ucout << "Evaluating point " << tmp_pt << " value=" << tmp_fval);
        if (contracting && (tmp_fval < best().value())) {
	   success=true;
           break;
	   }
        if (!contracting && (tmp_fval > best().value())) {
           step /= 2.0;
	   tmp_pt << best_pt;
	   for (size_t i=0; i<tmp_pt.size(); i++) {
	     tmp_pt[i] = best_pt[i] - grad[i]*step;
	     }
	   success=true;
           break;
           }
	 }
      if (!success) {
	stringstream tmp;
         tmp << "Unsuccessful line search: FinalStep=" << step;
         solver_status.termination_info = tmp.str();

	break ;
	}
      best_pt << tmp_pt;
      colin::AppRequest request = problem->set_domain(best_pt);
      problem->Request_F(request, best().value());
      problem->Request_G(request, grad);
      best().response = eval_mngr().perform_evaluation(request);
      DEBUGPR(100, ucout << "New point " << best_pt << " value=" << best().value() << " grad=" << grad);
      //
      // Debugging IO
      //
      debug_io(ucout);
   }

   best().point = best_pt;
   debug_io(ucout, true);
   ucout << utilib::Flush;
}

///
bool GradientDescent::check_convergence()
{
   if ( ColinSolver<utilib::BasicArray<double>, UNLP1_problem >
        ::check_convergence() )
      return true;

   real norm = length(grad);
   if (norm <= 1e-8)
   {
      std::stringstream tmp;
      tmp << "Grad-Norm Norm=" << norm << "<="
          << 1e-8 << "=Norm_thresh";
      solver_status.termination_info = tmp.str();
      return true;
   }
   return false;
}

std::string GradientDescent::define_solver_type() const
{
   return "GradientDescent";
}

} // namespace colin

