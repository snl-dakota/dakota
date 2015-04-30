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
// Coliny_DynamicGSS.cpp
//
// Local search without gradients.
//

#include <acro_config.h>
#include <utilib/_math.h>
#include <utilib/stl_auxiliary.h>
#include <scolib/DynamicGSS.h>

using namespace std;

namespace scolib {

DynamicGSS::DynamicGSS()
{
   reset_signal.connect(boost::bind(&DynamicGSS::reset_DynamicGSS, this));
}


void DynamicGSS::reset_DynamicGSS()
{
if ( problem.empty() ) return;

if (!rng)
   EXCEPTION_MNGR(runtime_error,"DynamicGSS::reset - undefined random number generator");

nrnd.generator(&rng);
eval_mngr().clear_evaluations();
#if COLINDONE
batch_evaluator_t::reset();
colin::AppResponseAnalysis::initialize(problem->numNonlinearIneqConstraints(),constraint_tolerance);
#endif
}


void DynamicGSS::optimize()
{
//
// Misc initialization of the optimizer
//
//opt_init();
if (!(this->initial_point_flag))
   EXCEPTION_MNGR(runtime_error,"DynamicGSS::minimize - no initial point specified.");
if (problem->num_real_vars != initial_point.size())
   EXCEPTION_MNGR(runtime_error,"DynamicGSS::minimize - problem has " <<
                  problem->num_real_vars << " real params, but initial point has " << initial_point.size() );
if (initial_point.size() == 0) {
   solver_status.termination_info = "No-Real-Params";
   return;
   }
//
//
//
unsigned int num_iters;
if (max_iters <= 0)
   num_iters = MAXINT;
else
   num_iters = curr_iter + max_iters;
//
// This is a hack to make the default number of iterations be small.
// In general, this will not be necessary...
//
num_iters = std::min(static_cast<unsigned int>(10),num_iters);

NumArray<double> best_pt(problem->num_real_vars.as<size_t>());
NumArray<double> tmp_pt(problem->num_real_vars.as<size_t>());
real fp;
best_pt << initial_point;

bool bound_feasible = problem->testBoundsFeasibility(best_pt);
real constraint_value=0.0;

colin::AppRequest request = problem->set_domain(best_pt);
problem->Request_response(request, best().response);
problem->Request_F(request, best().value());
eval_mngr().perform_evaluation(request);
#if COLINDONE
perform_evaluation(best_pt, best().response, best().value(), best().constraint_violation);
#endif
if (!bound_feasible)
   EXCEPTION_MNGR(runtime_error,"DynamicGSS::minimize - initial point is not bound-feasible");
debug_io(ucout);

colin::AppResponse tmp_response;
for (curr_iter++; curr_iter <= num_iters;  curr_iter++) {

  if (check_convergence())
     break;

  tmp_pt << best_pt;
  for (unsigned int i=0; i<tmp_pt.size(); i++)
    tmp_pt[i] += nrnd();
  //
  // TODO - should check if the point is feasible
  //
  colin::AppRequest request = problem->set_domain(tmp_pt);
  problem->Request_response(request, tmp_response);
  problem->Request_F(request, fp);
  eval_mngr().perform_evaluation(request);
  #if COLINDONE
  perform_evaluation(tmp_pt, tmp_response, fp, constraint_value);
  #endif

  if (fp < best().value()) {
     DEBUGPR(2, ucout << "BETTER POINT: " << best().value() 
             << " (" << fp << ")\n");
     best().value() = fp;
     best().constraint_violation = constraint_value;
     best().response = tmp_response;
     best_pt << tmp_pt;
     }

  best().point = best_pt;
  debug_io(ucout);
  }

final_points.add_point(problem, best_pt);
best().point = best_pt;
debug_io(ucout,true);
}


} // namespace scolib
