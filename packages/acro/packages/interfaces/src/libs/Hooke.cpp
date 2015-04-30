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
// Hooke.cpp
//

#include <acro_config.h>
#include <colin/SolverMngr.h>
#include <interfaces/Hooke.h>

using namespace std;

extern int hooke(int,double*,double*,double,double,int,int,double);

namespace interfaces {

//
// Static data used to communicate with the C function
//
Hooke* Hooke::curr_solver = 0;

//
// The static Hooke method that performs the evaluation
//
double Hooke::test_problem(double* x, int n)
{
if ( curr_solver->problem->enforcing_domain_bounds ) {
   for (int i=0; i<n; i++) {
     if ((curr_solver->lower[i] > x[i]) ||
         (curr_solver->upper[i] < x[i]))
	return DBL_MAX;
     }
   }
curr_solver->work_point.set_data(n,x);
curr_solver->problem->EvalF(curr_solver->eval_mngr(), curr_solver->work_point, curr_solver->work_value);
return curr_solver->work_value;
}

}

//
// The function that is optimized by the hooke() subroutine
//
double hooke_func(double* x, int n)
{ return interfaces::Hooke::test_problem(x,n); }

namespace interfaces {

Hooke::Hooke()
    : colin::Solver<colin::UNLP0_problem>(),
      colin::CommonOptions(properties)
{
reset_signal.connect( boost::bind( &Hooke::reset_Hooke, this ) );
//
// Create a solver parameter for the contraction factor, rho
//
rho=0.5;
properties.declare( "rho", 
                    "Step length contraction factor",
                    utilib::Privileged_Property(rho) );
//option.add("rho",rho,
//        "Step length contraction factor");

//
// Create a solver parameter for the contraction factor, rho
//
epsilon=1e-6;
properties.declare( "min_step_length", 
                    "The minimum step length",
                    utilib::Privileged_Property(epsilon) );
//option.add("min_step_length",epsilon,
//        "The minimum step length");

properties.privilegedGet("debug").unset_readonly();
properties.privilegedGet("max_iters").unset_readonly();
properties.privilegedGet("max_neval").unset_readonly();
properties.privilegedGet("accuracy").unset_readonly();
//option.enable("debug");
//option.enable("max_iters");
//option.enable("max_neval");
//option.enable("accuracy");
}


void Hooke::reset_Hooke()
{
if (problem.empty())
   return;

work_point.resize(problem->num_real_vars);
if ( problem->enforcing_domain_bounds )
{
   lower = problem->real_lower_bounds;
   upper = problem->real_upper_bounds;
}
}


void Hooke::optimize()
{
int n=problem->num_real_vars;
if (n == 0) {
   solver_status.termination_info = "No-Real-Params";
   solver_status.model_status = colin::model_no_solution_returned;
   solver_status.termination_condition = colin::termination_invalidProblem;
   return;
   }
//
// Get the initial point
//
if ( initial_points->size( problem.application() ) == 0 ) {
   solver_status.termination_info = "No-Initial-Point";
   solver_status.model_status = colin::model_no_solution_returned;
   solver_status.termination_condition = colin::termination_invalidProblem;
   return;
   }
if ( initial_points->size( problem.application()) > 1 )
   DEBUGPR(1, ucout << "Warning: more than one initial point specified. Cobyla will only use the first point.");
utilib::BasicArray<double> x(problem->num_real_vars.as<size_t>());
initial_points.get_point(problem, x);
if ( problem->num_real_vars != x.size() )
   EXCEPTION_MNGR(std::runtime_error, "User-provided initial point has length " << x.size() << " but the problem size is " << problem->num_real_vars << std::endl);
//
// Set the runtime limits
//
int num_evals;
if (max_neval <= 0)
   num_evals = MAXINT;
else
   num_evals = max_neval;
int num_iters;
if (max_iters <= 0)
   num_iters = MAXINT;
else
   num_iters = max_iters;
//
// Call the Hooke-Jeeves optimizer
//
curr_solver = this;
utilib::BasicArray<double> final_point(x.size());
int status = hooke(n, x.data(), final_point.data(), rho, epsilon, num_iters, num_evals, accuracy);
//
// Setup the solver status
//
if (status == 0) {
   solver_status.termination_info="Max-Num-Iterations";
   solver_status.model_status = colin::model_intermediate_nonoptimal;
   solver_status.termination_condition = colin::termination_maxIterations;
   final_points.add_point(problem, final_point);
}
else if (status == 1) {
   solver_status.termination_info="Step-Length";
   solver_status.model_status = colin::model_intermediate_nonoptimal;
   solver_status.termination_condition = colin::termination_minStepLength;
   final_points.add_point(problem, final_point);
}
else if (status == 2) {
   solver_status.termination_info="Accuracy";
   solver_status.model_status = colin::model_intermediate_nonoptimal;
   solver_status.termination_condition = colin::termination_minFunctionValue;
   final_points.add_point(problem, final_point);
}
else if (status == 3) {
   solver_status.termination_info="Max-Num-Evals";
   solver_status.model_status = colin::model_intermediate_nonoptimal;
   solver_status.termination_condition = colin::termination_maxEvaluations;
   final_points.add_point(problem, final_point);
}
else {
   stringstream tmp;
   tmp << "Unknown return code: " << status;
   solver_status.termination_info=tmp.str();
   solver_status.model_status = colin::model_no_solution_returned;
   solver_status.termination_condition = colin::termination_solverFailure;
}
}

REGISTER_COLIN_SOLVER_WITH_ALIAS(Hooke, "hj:Hooke", "hj:hj", "The Hooke-Jeeves derivative-free optimizer")

}

