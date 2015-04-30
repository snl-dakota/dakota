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
// PLGO.cpp
//

#include <acro_config.h>
#if defined(ACRO_USING_PEBBL) && defined(ACRO_USING_PLGO)

#include <scolib/PLGO.h>

#include <colin/SolverMngr.h>

using namespace std;
using namespace utilib;

namespace scolib {


PLGO::PLGO()
 : CommonOptions(properties),
   step_tolerance(1e-4),
   initial_step(1.0)
{
#if defined(ACRO_HAVE_MPI)
if (uMPI::running() && (uMPI::size == 1))
   solver = new plgo::PLGOSerialSolver;
else {
   solver = new plgo::PLGOParallelSolver;
   }
#else
solver = new plgo::PLGOSerialSolver;
#endif

func.solver = this;

//augment_parameters(solver->parameters(),true);

//option.disable("accuracy");
properties.erase("sufficient_objective_value");
//option.disable("ftol");
properties.erase("function_value_tolerance");
//option.enable("max_neval");
properties.privilegedGet("max-neval").unset_readonly();

solver->base()->set_parameter("absTolerance",1e-5);
solver->base()->set_parameter("lipshitzConstant",1.0);
}

void PLGO::reset_PLGO()
{
if (problem.empty())
   return;
if (problem->num_constraints > 0)
   EXCEPTION_MNGR(std::runtime_error, "PLGO::reset() - PLGO cannot solve a constrained optimization problem");

func.problem = problem;
solver->set_func(&func);

#if defined(ACRO_HAVE_MPI)
if (uMPI::size > 1)
   #ifdef UTILIB_MPI_COMM_IS_POINTER
   //option.set_parameter("mpicomm",static_cast<void*>(uMPI::comm));
   #else
   //option.set_parameter("mpicomm",uMPI::comm);
   #endif
#endif

if (problem->finite_bound_constraints())
{
   func.lower = problem->real_lower_bounds;
   func.upper = problem->real_upper_bounds;
}
x.resize(problem->num_real_vars);

ucout << utilib::Flush;
cout.flush();
solver->reset();
}


void PLGO::optimize()
{
if (problem->num_real_vars == 0) {
   solver_status.termination_info = "No-Real-Params";
   return;
   }
if (!problem->finite_bound_constraints()) {
   solver_status.termination_info = "Missing-Bound-Constraints";
   return;
   }

int tmp_neval = max(max_neval-neval(),0);
if (max_neval_curr != 0)
   tmp_neval = min(tmp_neval, max_neval_curr);

colin::real best_value;
try {
  solver->minimize(solver_status.termination_info, x, best_value);
}
STD_CATCH(;)

final_points.add_point(problem,x);
#if 0
best().point = x;
colin::AppRequest request = problem->set_domain(x);
problem->Request_response(request, best().response);
problem->Request_F(request, best().value());
eval_mngr().perform_evaluation(request);
#endif
}

// Defines scolib::StaticInitializers::PLGO_bool
REGISTER_COLIN_SOLVER_WITH_ALIAS
   (PLGO, "sco:PLGO", "sco:plgo", "The SCO PLGO optimizer")

} // namespace scolib

#else

namespace scolib {
namespace StaticInitializers {
extern const volatile bool PLGO_bool = false;
} // namespace scolib::StaticInitializers
} // namespace scolib

#endif // defined(ACRO_USING_PLGO) && defined(ACRO_USING_PEBBL)
