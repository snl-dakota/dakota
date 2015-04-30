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
// APPS.cpp
//
 
#include <acro_config.h>
#ifdef ACRO_USING_APPSPACK

#include <interfaces/APPS.h>
#include <colin/SolverMngr.h>
#include <colin/real.h>
#include <utilib/_math.h>
#include <utilib/pvector.h>

using namespace std;
#include "APPSPACK_Executor_Interface.hpp"
#include "APPSPACK_Parameter_List.hpp"
#include "APPSPACK_Constraints_Linear.hpp"
#include "APPSPACK_Solver.hpp"
#include "APPSPACK_Float.hpp"

#define COORDINATE_BASIS                 0
#define SIMPLEX_BASIS                    1
#define POSITIVE_COORDINATE_BASIS        2
#define SHIFTED_SIMPLEX_BASIS            3

using colin::real;
using std::cerr;
using std::endl;

namespace utilib {

bool operator<(const APPSPACK::Vector& v1, const APPSPACK::Vector& v2)
{
size_t len = v1.size();
if (v2.size() < len) return false;
if (v2.size() > len) return true;
for (size_t i=0; i<len; i++)
  if (v1[i] < v2[i]) return true;
return false;
}

}

namespace interfaces  {

namespace ColinyAPPS {

namespace {

/// APPSPACK::Vector -> std::vector<double>
int lexical_cast_APPSPACK_Vector_std_vector( const utilib::Any& from, 
                                             utilib::Any& to )
{
   const APPSPACK::Vector &tmp = from.expose<APPSPACK::Vector>();
   std::vector<double> &ans = to.set<std::vector<double> >();
   size_t size = tmp.size();
   ans.reserve(size);
   for(size_t i = 0; i < size; ++i )
      ans.push_back(tmp[i]);
   return OK;
}

/// std::vector<double> -> APPSPACK::Vector
int lexical_cast_std_vector_APPSPACK_Vector( const utilib::Any& from, 
                                             utilib::Any& to )
{
   const std::vector<double> &tmp = from.expose<std::vector<double> >();
   APPSPACK::Vector &ans = to.set<APPSPACK::Vector>();
   size_t size = tmp.size();
   ans.reserve(size);
   for(size_t i = 0; i < size; ++i )
      ans.push_back(tmp[i]);
   return OK;
}

bool register_APPSPACK_lexical_casts()
{
   utilib::TypeManager()->register_lexical_cast
      ( typeid(APPSPACK::Vector), typeid(std::vector<double>),
        &lexical_cast_APPSPACK_Vector_std_vector );
   utilib::TypeManager()->register_lexical_cast
      ( typeid(std::vector<double>), typeid(APPSPACK::Vector),
        &lexical_cast_std_vector_APPSPACK_Vector );
}

bool dummy = register_APPSPACK_lexical_casts();
} // namespace


class FevalMgr : public APPSPACK::Executor::Interface 
{
public:
   //! Constructor
   FevalMgr(APPS* apps_ptr) : apps(apps_ptr) {}

   //! Destructor
   virtual ~FevalMgr() { }
   
   /// Return true if the EvaluationManager "queue" is not full
   bool isWaiting() const
   { return tasks.size() < apps->eval_mngr().num_evaluation_servers(); }
   
   bool spawn(const APPSPACK::Vector& x, int tag_in )
   { 
      // Although not strictly necessary, this is a nice test.  That
      // said, it would be perfectly fine to go ahead and queue the
      // evaluation.  The EvaluationManager will take care of never
      // actually *running* more applications that it is allowed to.
      // Plus, if this is a multi-solver environment, there is the
      // chance that the EvaluationManager will be able to run more than
      // num_evaluation_servers() if the other solver(s) are not using
      // all of their allocation.
      if ( ! isWaiting() )
         return false;

      tasks[apps->problem->AsyncEvalF(apps->eval_mngr(), x)] = tag_in; 
      return true;
   }
   
   int recv(int& tag_out, APPSPACK::Vector& f_out, string& msg_out)
   {
      // blocking wait for next queued evaluaton
      colin::AppResponse response;
      colin::EvaluationID id;
      apps->eval_mngr().next_response(id,response);
      // id is only empty if nothing is queued.
      if ( id.empty() )
         return 0;

      map<colin::EvaluationID, int>::iterator it = tasks.find(id);
      if ( it == tasks.end() )
      {
         EXCEPTION_MNGR(runtime_error, 
                        "FevalMngr::recv - no tasks have id=" << id);
      }
      response.get(colin::f_info, f_out);
      tag_out = it->second;
      tasks.erase(it);
      return id.rank() + 1;
   }

   void reset() 
   { 
      apps->eval_mngr().clear_evaluations();
      tasks.clear(); 
   }

private:
   /// the map of spawned async evals we are waiting for
   map<colin::EvaluationID, int> tasks;
   /// a handle on the owning APPS solver (to get at the problem & eval_mngr)
   APPS* apps;
};


} // namespace ColinyAPPS



APPS::~APPS()
{
delete feval_mgr;
delete params;
}

APPS::APPS()
   : CommonOptions(properties),
     num_search_directions(-1),
     synch_flag(true)
{
   reset_signal.connect( boost::bind( &APPS::reset_APPS, this ) );

   feval_mgr = new ColinyAPPS::FevalMgr(this);
   params = new APPSPACK::Parameter::List;

   batch_str = "sync";
   properties.declare
      ( "batch_eval", 
        "Defines how the algorithm is parallelized when two or more "
        "function evaluations could be computed simultaneously:\n"
        "   sync:  All function evaluations are performed synchronously.\n"
        "   async: Function evaluations are performed asynchronously. "
        "Evaluations are recorded until an improving point is generated "
        "or all of them have completed.",
        utilib::Privileged_Property(batch_str) );
   //option.add("batch_eval",batch_str,
   //           "Defines how the algorithm is parallelized when two or more function evaluations could be computed simultaneously.\n\t  sync: All function evaluations are performed synchronously.\n\t  async: Function evaluations are performed asynchronously. Evaluations are recorded until an improving point is generated or all of them have completed."
   //           );

   auto_rescale = true;
   properties.declare
      ( "auto_rescale", 
        "If true, then automatically rescale the search for "
        "bound-constrained problems. The initial scale is 10% of the "
        "range in each dimension.",
        utilib::Privileged_Property(auto_rescale) );
   //option.add("auto_rescale",auto_rescale,
   //           "If true, then automatically rescale the search for bound-constrained problems. The initial scale is 10% of the range in each dimension.");
   //option.categorize("auto_rescale","Step Length Control");

   initial_step=1.0;
   properties.declare
      ( "initial_step", 
        "Initial step length",
        utilib::Privileged_Property(initial_step) );
   //option.add("initial_step",initial_step, 
   //           "Initial step length");
   //option.categorize("initial_step","Step Length Control");
   //option.alias("initial_step","initial_stepsize");
   //option.alias("initial_step","initial_steplength");

   step_tolerance=1.0e-5;
   properties.declare
      ( "step_tolerance", 
        "Convergence tolerance step length",
        utilib::Privileged_Property(step_tolerance) );
   //option.add("step_tolerance",step_tolerance, 
   //           "Convergence tolerance step length");
   //option.categorize("step_tolerance","Termination");

   contraction_factor=0.5;
   properties.declare
      ( "contraction_factor", 
        "Contraction factor",
        utilib::Privileged_Property(contraction_factor) );
   //option.add("contraction_factor",contraction_factor,
   //           "Contraction factor");
   //option.categorize("contraction_factor","Step Length Control");

   min_step_allowed=2.0e-5;
   properties.declare
      ( "min_step_allowed", 
        "Minimum step length allowed",
        utilib::Privileged_Property(min_step_allowed) );
   //option.add("min_step_allowed",min_step_allowed,
   //           "Minimum step length allowed");
   //option.categorize("min_step_allowed","Step Length Control");

   alpha=0.0;
   properties.declare
      ( "alpha", 
        "Sufficient decrease parameter",
        utilib::Privileged_Property(alpha) );
   //option.add("alpha",alpha,
   //           "Sufficient decrease parameter");
   //option.categorize("alpha","Step Length Control");

   profile=0;
   properties.declare
      ( "profile", 
        "Profile level",
        utilib::Privileged_Property(profile) );
   //option.add("profile",profile, 
   //           "Profile level");

   properties.privilegedGet("debug").unset_readonly();
   properties.privilegedGet("max_neval").unset_readonly();
   properties.privilegedGet("max_neval_curr").unset_readonly();
   properties.privilegedGet("sufficient_objective_value").unset_readonly();
   //option.enable("debug");
   //option.enable("max_neval");
   //option.enable("max_neval_curr");
   //option.enable("accuracy");
}


// publish options to APPS through command line arguments.
void APPS::reset_APPS()
{
if (problem.empty()) return;

if (debug > 1000) profile=10;

int tmp_neval;
if (max_neval > 0) {
   tmp_neval = max(max_neval-neval(),0);
   if (max_neval_curr != 0)
      tmp_neval = min(tmp_neval, max_neval_curr);
   }
else
   tmp_neval = max_neval_curr;

if (tmp_neval > 0)
   params->setParameter("Maximum Evaluations",tmp_neval);
params->setParameter("Function Tolerance",static_cast<double>(accuracy));
params->setParameter("Sufficient Decrease Factor",alpha);
params->setParameter("Contraction Factor",contraction_factor);
params->setParameter("Initial Step",initial_step);
params->setParameter("Debug",profile);
params->setParameter("Minimum Step",min_step_allowed);
params->setParameter("Step Tolerance",step_tolerance);
params->setParameter("Bounds Tolerance",constraint_tolerance);

if (batch_str == "sync")
   synch_flag = true;
else if (batch_str == "async")
   synch_flag = false;
else
   EXCEPTION_MNGR(runtime_error,"APPS::reset - bad value for batch_eval parameter: " << batch_str);
params->setParameter("Synchronous",synch_flag);

if (problem->num_real_vars > 0) {
   APPSPACK::Vector  isLower, isUpper;
   isLower.resize(problem->num_real_vars);
   isUpper.resize(problem->num_real_vars);

   APPSPACK::Vector scale(problem->num_real_vars.as<size_t>());
   for (int i=0; i<scale.size(); i++)
     scale[i] = 1.0;
   if ( problem->enforcing_domain_bounds ) {
      APPSPACK::Vector l_bound, u_bound;
      l_bound.resize(problem->num_real_vars);
      u_bound.resize(problem->num_real_vars);
      utilib::pvector<colin::real> L_bound = problem->real_lower_bounds;
      utilib::pvector<colin::real> U_bound = problem->real_upper_bounds;
      double DNE = APPSPACK::dne();
      for (int i=0; i<l_bound.size(); i++) {
        l_bound[i] = DNE;
        u_bound[i] = DNE;
        }

      for (int i=0; i<isLower.size(); i++) {
        l_bound[i] = L_bound[i];
        u_bound[i] = U_bound[i];
        if ((L_bound[i] == colin::real::negative_infinity) ||
	    (problem->realLowerBoundType(i) != colin::hard_bound))
           isLower[i] = 0.0;
        else
           isLower[i] = 1.0;
        if ((U_bound[i] == colin::real::positive_infinity) ||
	    (problem->realUpperBoundType(i) != colin::hard_bound))
           isUpper[i] = 0.0;
        else
           isUpper[i] = 1.0;
        if (auto_rescale && (isLower[i] == 1.0) && (isUpper[i] == 1.0))
           scale[i] = max((u_bound[i]-l_bound[i])*0.1,1e-5);
        }
      params->setParameter("Lower",l_bound);
      params->setParameter("Upper",u_bound);
      }
   params->setParameter("Scaling",scale);
   }
   

if ((num_search_directions == -1) && (problem->num_real_vars > 0))
   num_search_directions = problem->num_real_vars.as<size_t>()*2;
   params->setParameter("search",num_search_directions);

feval_mgr->reset();
}



void APPS::optimize()
{
if (problem->num_real_vars == 0) {
   solver_status.termination_info = "No-Real-Params";
   solver_status.model_status = colin::model_no_solution_returned;
   solver_status.termination_condition = colin::termination_invalidProblem;
   return;
   }
if (initial_points->size( problem.application() ) == 0) {
   solver_status.termination_info = "No-Initial-Point";
   solver_status.model_status = colin::model_no_solution_returned;
   solver_status.termination_condition = colin::termination_invalidProblem;
   return;
   }
if ( initial_points->size( problem.application() ) > 1 )
   DEBUGPR(1, ucout << "Warning: more than one initial point specified. APPS will only use the first point.");
utilib::BasicArray<double> initial_point(problem->num_real_vars.as<size_t>());
initial_points.get_point(problem, initial_point);
if ( problem->num_real_vars != initial_point.size() )
   EXCEPTION_MNGR(std::runtime_error, "User-provided initial point has length " << initial_point.size() << " but the problem size is " << problem->num_real_vars << std::endl);
//
// Call APPS
//
APPSPACK::Vector init_point(problem->num_real_vars.as<size_t>());
for (int i=0; i<init_point.size(); i++)
  init_point[i] = initial_point[i];
params->setParameter("Initial X",init_point);
APPSPACK::Constraints::Linear linear(*params);
APPSPACK::Solver solver(*params, *feval_mgr, linear);
APPSPACK::Solver::State state = solver.solve();

#if 0
//
// Set the minimum point and value
//
best().point = solver.getBestX();
colin::AppRequest request = problem->set_domain(best().point);
problem->Request_response(request,best().response);
problem->Request_F(request,best().value());
problem->get_response(request);
#if COLINDONE
if (problem->numNonlinearConstraints() > 0)
   problem->Eval(best().point,best().response,colin::mode_f|colin::mode_cf);
else
   problem->Eval(best().point,best().response,colin::mode_f);
compute_response_info(best().response,problem->state->constraint_lower_bounds,  problem->state->constraint_upper_bounds, best().value(),best().constraint_violation);
#endif
#endif
//
//
//
if (state == APPSPACK::Solver::StepConverged) {
   solver_status.model_status = colin::model_intermediate_nonoptimal;
   solver_status.termination_condition = colin::termination_minStepLength;
}
else if (state == APPSPACK::Solver::FunctionConverged) {
   solver_status.model_status = colin::model_intermediate_nonoptimal;
   solver_status.termination_condition = colin::termination_minFunctionValue;
}
else if (state == APPSPACK::Solver::EvaluationsExhausted) {
   solver_status.termination_info = "Max-Num-Evals";
   solver_status.model_status = colin::model_intermediate_nonoptimal;
   solver_status.termination_condition = colin::termination_maxEvaluations;
}
else
   solver_status.termination_info = "Unknown";
final_points.add_point(problem, solver.getBestX());
}


REGISTER_COLIN_SOLVER_WITH_ALIAS(APPS, "apps:APPS", "apps:apps",  "The APPS derivative-free optimizer")

} // namespace interfaces

#endif
