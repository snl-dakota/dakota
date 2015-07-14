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
// Cobyla.cpp
//

#include <acro_config.h>
#ifdef ACRO_USING_COBYLA

#include <cobyla2c/cobyla.h>
#include <utilib/DoubleMatrix.h>
#include <colin/SolverMngr.h>
#include <interfaces/Cobyla.h>

using namespace std;
using namespace utilib;

namespace interfaces {

//
// Static data in the Cobyla class
//
Cobyla* Cobyla::instance = NULL;

extern "C" int calc_func(int n, int m, double* x, double* f, double* con, void* /*state*/)
{ Cobyla::calcfc(n,m,x,f,con); return 0; }


void Cobyla::calcfc(int n, int /*m*/, double* x, double* f, double* con)
{
   if ( ! instance )
      EXCEPTION_MNGR(std::runtime_error, "Cobyla::calcfc(): "
                     "called from outside Cobyla::optimize().");
   instance->calcfc_impl(n, x, f, con);
}

void Cobyla::calcfc_impl(int n, double* x, double* f, double* con)
{
   BasicArray<double> calcfc_point;
   calcfc_point.resize(problem->num_real_vars);
   calcfc_point.set_data(n,x);
   colin::AppRequest req = problem->set_domain(calcfc_point);
   problem->Request_F(req);

   bool constrained = ( problem->num_constraints > 0 );
   if ( constrained ) {
      problem->Request_CF(req);
   }

   colin::AppResponse eval = eval_mngr().perform_evaluation(req);

   eval.get(colin::f_info, *f);

   int ndx=0;
   if ( problem->enforcing_domain_bounds ) {
      for (int i=0; i<n; i++) {
         if (finite(blower[i])) con[ndx++] = -(blower[i] - x[i]);
         if (finite(bupper[i])) con[ndx++] = -(x[i] - bupper[i]);
      }
   }
   if ( constrained ) {
      BasicArray<colin::real> calcfc_cvec;
      eval.get(colin::cf_info, calcfc_cvec);
      for (size_type i=0; i<calcfc_cvec.size(); i++) {
         if (finite(clower[i])) con[ndx++] = -(clower[i] - calcfc_cvec[i]);
         if (finite(cupper[i])) con[ndx++] = -(calcfc_cvec[i] - cupper[i]);
      }
   }
   
   //for (int i=0; i<m; i++)
   //cerr << "CON " << i << " " << con[i] << endl;
}


Cobyla::Cobyla()
   : colin::Solver<colin::NLP0_problem>(),
     CommonOptions(this->properties)
{
   reset_signal.connect( boost::bind( &Cobyla::reset_Cobyla, this ) );

   step_tolerance=1e-4;
   properties.declare
      ( "step_tolerance", 
        "Convergence tolerance step length",
        utilib::Privileged_Property(step_tolerance) );
   //option.add("step_tolerance",step_tolerance,
   //           "Convergence tolerance step length");

   initial_step=1.0;
   properties.declare
      ( "initial_step", 
        "Initial step length",
        utilib::Privileged_Property(initial_step) );
   //option.add("initial_step",initial_step,
   //           "Initial step length");

   properties.privilegedGet("debug").unset_readonly();
   properties.privilegedGet("max-neval").unset_readonly();
   properties.privilegedGet("sufficient_objective_value").unset_readonly();
   //option.enable("debug");
   //option.enable("max_neval");
   //option.enable("accuracy");
}


void Cobyla::reset_Cobyla()
{
if (problem.empty())
   return;

#if 0
if ((simple_app == 0) || (problem.application() != simple_app)) {
   simple_app = new colin::SimplifiedConstraintInterface<colin::NLP0_problem>(problem.application());
   problem.set_application(simple_app);
   }
#endif

#if 0
if ((problem->numNonlinearEqConstraints()+problem->numLinearEqConstraints()) > 0)
   EXCEPTION_MNGR(runtime_error, "Cobyla::reset - " << (problem->numNonlinearEqConstraints()+problem->numLinearEqConstraints()) << " equality constraints cannot be handled by cobyla!");
#endif

if ( problem->enforcing_domain_bounds )
{
   blower = problem->real_lower_bounds;
   bupper = problem->real_upper_bounds;
}
if (problem->num_constraints > 0)
   problem->get_constraint_bounds(clower,cupper);
}


void Cobyla::optimize()
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
if (initial_points->size( problem.application() ) == 0) {
   solver_status.termination_info = "No-Initial-Point";
   solver_status.model_status = colin::model_no_solution_returned;
   solver_status.termination_condition = colin::termination_invalidProblem;
   return;
   }
if ( initial_points->size( problem.application() ) > 1 )
   DEBUGPR(1, ucout << "Warning: more than one initial point specified. Cobyla will only use the first point.");
utilib::BasicArray<double> x(problem->num_real_vars.as<size_t>());
initial_points.get_point(problem, x);
if ( problem->num_real_vars != x.size() )
   EXCEPTION_MNGR(std::runtime_error, "User-provided initial point has length " << x.size() << " but the problem size is " << problem->num_real_vars << std::endl);
//
// Compute the number of constraints, including bound constraints
//
int ncon=problem->num_constraints;
int m=0;
if ( problem->enforcing_domain_bounds ) {
   if (problem->finite_bound_constraints())
      m += 2*n;
   else {
      for (int i=0; i<n; i++) {
	if (finite(blower[i])) m++;
	if (finite(bupper[i])) m++;
        }
      }
   }
if (ncon > 0) {
   for (int i=0; i<ncon; i++) {
     if (finite(clower[i])) m++;
     if (finite(cupper[i])) m++;
     }
   }
//
// Set the debugging level
//
int iprint=(debug>3 ? 3 : debug);
//
// Set the runtime limits
//
int tmp_neval;
#if 0
/// TODO: revisit this...
if (max_neval > 0) {
   tmp_neval = max(max_neval-neval(),0);
   if (max_neval_curr != 0) 
      tmp_neval = min(tmp_neval, max_neval_curr);
   }
else
   tmp_neval = max_neval_curr;
#endif
if (max_neval == 0)
   tmp_neval = MAXINT;
else
   tmp_neval = max_neval;

//
// Run Cobyla2
//
if ( instance )
   EXCEPTION_MNGR(std::runtime_error, "Cobyla::optimize(): Cobyla is not "
                  "thread-safe, and we detected that another instance of "
                  "Cobyla is running!");
instance = this;

// Put bounds into a double for Cobyla.
blower_d << blower;
bupper_d << bupper;

int istatus = cobyla(n, m, &(x[0]), initial_step, step_tolerance, &(blower_d[0]), &(bupper_d[0]), iprint, &tmp_neval, accuracy, calc_func,0);
instance = NULL;
//
// Setup the solver status
//
switch (istatus) {
  case COBYLA_ENOMEM:
	solver_status.termination_info="Memory-Errors";
    solver_status.model_status = colin::model_no_solution_returned;
    solver_status.termination_condition = colin::termination_internalSolverError;
	break;
  case COBYLA_ROUNDING:
	solver_status.termination_info="Roundoff-Errors";
    solver_status.model_status = colin::model_no_solution_returned;
    solver_status.termination_condition = colin::termination_internalSolverError;
	break;
  case COBYLA_NORMAL:
	solver_status.termination_info="Step-Length";
    solver_status.model_status = colin::model_intermediate_nonoptimal;
    solver_status.termination_condition = colin::termination_minStepLength;
    final_points.add_point(problem, x);
	break;
  case COBYLA_MAXFUN:
	solver_status.termination_info="Max-Num-Evals";
    solver_status.model_status = colin::model_intermediate_nonoptimal;
    solver_status.termination_condition = colin::termination_maxEvaluations;
    final_points.add_point(problem, x);
	break;
  case COBYLA_FUNCVAL:
	solver_status.termination_info="Accuracy";
    solver_status.model_status = colin::model_intermediate_nonoptimal;
    solver_status.termination_condition = colin::termination_minFunctionValue;
    final_points.add_point(problem, x);
	break;
  default:
	solver_status.termination_info="Unknown";
    solver_status.model_status = colin::model_intermediate_nonoptimal;
    solver_status.termination_condition = colin::termination_solverFailure;
    final_points.add_point(problem, x);
	break;
  };
}


REGISTER_COLIN_SOLVER_WITH_ALIAS(Cobyla, "cobyla:Cobyla", "cobyla:cobyla", "The Cobyla2 derivative-free optimizer")

}

#endif
