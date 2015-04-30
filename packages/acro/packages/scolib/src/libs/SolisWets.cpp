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
// Coliny_SolisWets.cpp
//
// Local search without gradients.
//
// TODO: cleanup the code
//   Create a 'response' value for that contains the function/constraint
//   values, feasibility flags, etc.
//

#include <acro_config.h>
#include <scolib/SolisWets.h>

#include <colin/SolverMngr.h>

#include <utilib/_math.h>
#include <utilib/pvector.h>
#include <utilib/stl_auxiliary.h>

#define SWAP(x,y)	{NumArray<double>* tmp; tmp = x; x = y; y = tmp;}

#define SPHERE_DEV  0
#define NORMAL_DEV  1
#define UNIFORM_DEV 2

#define SOLISWETS_DEFAULT       0
#define SINGLE_EXPAND_UPDATE    1

using namespace std;

namespace scolib {

SolisWets::SolisWets()
{
#if COLINDONE
   constraint_penalty=1.0;
   ParameterSet::set_parameter_default("constraint_penalty","1.0");
#endif

   //utilib::OptionParser &opt = this->option;

   auto_rescale_flag=true;
   this->properties.declare
      ( "auto_rescale", 
        "If true, then automatically rescale the search for bound-constrained "
        "problems. The initial scale is 10% of the range in each dimension.",
        utilib::Privileged_Property(auto_rescale_flag) );
   //opt.add("auto_rescale",auto_rescale_flag,
   //     "If true, then automatically rescale the search for bound-constrained\n"
   //        "\t  problems. The initial scale is 10% of the range in each dimension.");

   Delta_init = 1.0;
   this->properties.declare
      ( "initial_step", 
        "Initial step length",
        utilib::Privileged_Property(Delta_init) );
   //opt.add("initial_step",Delta_init,
   //        "Initial step length");
   //opt.alias("initial_step","initial_stepsize");
   //opt.alias("initial_step","initial_steplength");

   max_success = 5;
   this->properties.declare
      ( "max_success", 
        "Number of successful iterations before step length is expanded",
        utilib::Privileged_Property(max_success) );
   //opt.add("max_success",max_success,
   //        "Number of successful iterations before step length is expanded");

   max_failure = 0;
   this->properties.declare
      ( "max_failure", 
        "Number of unsuccessful iterations before step length is contracted",
        utilib::Privileged_Property(max_failure) );
   //opt.add("max_failure",max_failure,
   //        "Number of unsuccessful iterations before step length is contracted");

   ex_factor = 2.0; 
   this->properties.declare
      ( "expansion_factor", 
        "Expansion factor",
        utilib::Privileged_Property(ex_factor) );
   //opt.add("expansion_factor",ex_factor,
   //        "Expansion factor");

   ct_factor = 0.5;
   this->properties.declare
      ( "contraction_factor", 
        "Contraction factor",
        utilib::Privileged_Property(ct_factor) );
   //opt.add("contraction_factor",ct_factor,
   //        "Contraction factor");

   Delta_thresh = 1e-6;
   this->properties.declare
      ( "step_tolerance", 
        "Convergence tolerance step length",
        utilib::Privileged_Property(Delta_thresh) );
   //opt.add("step_tolerance",Delta_thresh,
   //        "Convergence tolerance step length");

   update_type = "default";
   this->properties.declare
      ( "update_type", 
        "Control for step length update: {default, single_expand}",
        utilib::Privileged_Property(update_type) );
   //opt.add("update_type",update_type,
   //        "Control for step length update:\n\t  default, single_expand");

   bias_flag = false;
   this->properties.declare
      ( "bias_flag", 
        "Use an adaptive step bias",
        utilib::Privileged_Property(bias_flag) );
   //opt.add("bias_flag",bias_flag,
   //        "Use an adaptive step bias");

   neighborhood_type = "normal";
   this->properties.declare
      ( "neighborhood_type", 
        "Type of neighorhood used: {normal, uniform, sphere}",
        utilib::Privileged_Property(neighborhood_type) );
   //opt.add("neighborhood_type",neighborhood_type,
   //        "Type of neighorhood used:\n\t  normal, uniform, sphere");

   //Sigma.resize(1);
   //Sigma << 1.0;
   this->properties.declare
      ( "step_scales", 
        "The scale factors for each dimension.  The default scale is 1.0",
        utilib::Privileged_Property(Sigma) );
   //opt.add("step_scales",Sigma,
   //        "The scale factors for each dimension.  The default scale is 1.0");

   reset_signal.connect(boost::bind(&SolisWets::reset_SolisWets, this));
}


void SolisWets::reset_SolisWets()
{
if ( problem.empty() ) return;

if (!rng)
   EXCEPTION_MNGR(runtime_error,"SolisWets::reset - undefined random number generator");

unif_dev.generator(&rng);
normal_dev.generator(&rng);

unsigned int n = problem->num_real_vars;
if ((Sigma.size() != 0) && (Sigma.size() != n))
   EXCEPTION_MNGR(runtime_error,"SolisWets::reset - Scale vector length " << Sigma.size() << " is \n\t  not equal to num_real_vars: " << n);
Sigma.resize(n);
bool bc_flag = problem->enforcing_domain_bounds;
utilib::pvector<colin::real> lower_bc = problem->real_lower_bounds;
utilib::pvector<colin::real> upper_bc = problem->real_upper_bounds;

if (auto_rescale_flag && bc_flag) {
  for (unsigned int i=0; i<n; i++)
    if ((upper_bc[i] == real :: positive_infinity) ||
        (lower_bc[i] == real :: negative_infinity))
      Sigma[i] = 1.0;
    else
      Sigma[i] = max((static_cast<double>(upper_bc[i])-static_cast<double>(lower_bc[i]))/10.0,1e-5);
  }
else
  Sigma << 1.0;




if (max_failure < 1)
   max_failure = 4*n;

if (update_type == "default")
   update_id = SOLISWETS_DEFAULT;
else if (update_type == "single_expand")
   update_id = SINGLE_EXPAND_UPDATE;
else
   EXCEPTION_MNGR(runtime_error,"SolisWets::reset - bad update type: " << update_type);

if (neighborhood_type == "normal")
   neighborhood_id = NORMAL_DEV;
else if (neighborhood_type == "uniform")
   neighborhood_id = UNIFORM_DEV;
else if (neighborhood_type == "sphere")
   neighborhood_id = SPHERE_DEV;
else
   EXCEPTION_MNGR(runtime_error,"SolisWets::reset - bad neighborhood type: " << neighborhood_type);

bias.resize(n);
vec1.resize(n);
vec2.resize(n);
vec3.resize(n);

//
// Setup algorithmic parameters
//
bias << 0.0;
expand_flag=true;
n_success=0;
n_failure=0;
Delta_min = Delta = Delta_init;

eval_mngr().clear_evaluations();
#if cCOLINDONE
batch_evaluator_t::reset();
colin::AppResponseAnalysis::initialize(problem->numNonlinearIneqConstraints(),constraint_tolerance);
#endif
}


void SolisWets::optimize()
{
//
// Misc initialization of the optimizer
//
//opt_init();
if (!(this->initial_point_flag))
   EXCEPTION_MNGR(runtime_error,"SolisWets::minimize - no initial point specified.");
if (problem->num_real_vars != initial_point.size())
   EXCEPTION_MNGR(runtime_error,"SolisWets::minimize - problem has " <<
                  problem->num_real_vars << 
                  " real params, but initial point has " << 
                  initial_point.size() );
if (initial_point.size() == 0) {
   solver_status.termination_info = "No-Real-Params";
   return;
   }
//
//
//
utilib::Property convergence_factor;
if ( problem->has_property("convergence_factor") )
   convergence_factor = problem->property("convergence_factor");
if ( ! convergence_factor.get().empty() )
   convergence_factor = Delta_init/Delta_min;
//
//
//
unsigned int num_iters;
if (max_iters <= 0)
   num_iters = MAXINT;
else
   num_iters = curr_iter + max_iters;
//
// Setup swap vectors
//
NumArray<double>* best_pt = &vec1;
NumArray<double>* new_point  = &vec2;
NumArray<double>* temp_point = &vec3;

real fp;
*best_pt << initial_point;

bool bound_feasible = problem->testBoundsFeasibility(*best_pt);
//real constraint_value=0.0;

colin::AppRequest request = problem->set_domain(*best_pt);
problem->Request_response(request, best().response);
problem->Request_F(request,best().value());
eval_mngr().perform_evaluation(request);
#if COLINDONE
perform_evaluation(*best_pt, Delta_init/Delta_min, best().response, best().value(), best().constraint_violation);
#endif
if (!bound_feasible)
   EXCEPTION_MNGR(runtime_error,"SolisWets::minimize - initial point is not bound-feasible");
iteration_status = -1;
debug_io(ucout);

for (curr_iter++; curr_iter <= num_iters;  curr_iter++) {

  if (Delta <= Delta_thresh) {
     stringstream tmp;
     tmp << "Step-Length Delta=" << Delta << "<=" 
				<< Delta_thresh << "=Delta_thresh";
     solver_status.termination_info = tmp.str();
     break;
     }

  if (check_convergence())
     break;

  DEBUGPR(2, ucout << "OBJECTIVE " << best().value() << "\n");
  DEBUGPR(2,ucout << "DELTA:       " << Delta << "\n");
  DEBUGPR(2,ucout << "BIAS VECTOR: " << bias << "\n");

  *temp_point << *best_pt;		// The new mean
  if (bias_flag)
     *temp_point += bias;
  gen_new_point(*new_point,*temp_point,Delta,bound_feasible);
  if (bound_feasible) {
     //tmp_response.reset();
     colin::AppRequest request = problem->set_domain(*new_point);
     problem->Request_response(request, tmp_response);
     problem->Request_F(request,fp);
     eval_mngr().perform_evaluation(request);
     #if COLINDONE
     perform_evaluation(*new_point, Delta_init/Delta_min, tmp_response, fp, constraint_value);
     #endif
     }
  DEBUGPR(2,
          ucout << "Response     " << tmp_response << endl;
          ucout << "OLD PT:      " << *best_pt << "\n";
          ucout << "OLD+BIAS:    " << *temp_point << "\n";
          ucout << "NEW PT:      " << *new_point << "\n";
          double val=0.0;
          for (unsigned int i=0; i<new_point->size(); i++)
             val += ((*new_point)[i]-(*temp_point)[i]) * 
                    ((*new_point)[i]-(*temp_point)[i]);
          val = sqrt(val);
          ucout << "STEPLEN: " << val << endl;
          );
  DEBUGPR(2,ucout << "FEval: value=" << fp << " feasible=" << 
          (bound_feasible) << "\n");

  if (bound_feasible && (fp < best().value())) {
     DEBUGPR(2, ucout << "BETTER POINT: " << best().value() 
             << " (" << fp << ")\n");
     iteration_status = 0;
     best().value() = fp;
     //best().constraint_violation = constraint_value;
     best().response = tmp_response;

     UpdateDelta(true);
     if (bias_flag) {
        bias *= 0.2;
        for (unsigned int i=0; i<bias.size(); i++)
          bias[i] += 0.4*((*new_point)[i] - (*best_pt)[i]);
        }

     /**
     if (Debug_path_len) {
        *best_pt -= *new_point;
        path_length += length(*best_pt);
        }
     **/
     SWAP(best_pt,new_point);
     }

  else {
	// Go through some shenanigans to avoid de/allocating memory
     *temp_point << *best_pt;
     *temp_point *= 2.0;
     *temp_point -= *new_point;

     DEBUGPR(2, ucout << "Reflected Point:    " << *temp_point << endl);
     DEBUGPR(2, ucout << "Response     " << tmp_response << endl);
     bound_feasible = problem->testBoundsFeasibility(*temp_point);
     if (bound_feasible) {
	//tmp_response.reset();
        colin::AppRequest request = problem->set_domain(*temp_point);
        problem->Request_response(request, tmp_response);
        problem->Request_F(request,fp);
        eval_mngr().perform_evaluation(request);
        #if COLINDONE
        perform_evaluation(*temp_point, Delta_init/Delta_min, tmp_response, fp, constraint_value);
        #endif
	}
     DEBUGPR(2,ucout << "FEval: value=" << fp << " feasible=" 
             << (bound_feasible) << "\n");
     if (bound_feasible && (fp < best().value())) {
        DEBUGPR(2, ucout << "BETTER POINT: " << best().value() 
                << " (" << fp << ")\n");
        iteration_status = 1;
        best().value() = fp;
        //best().constraint_violation = constraint_value;
	best().response = tmp_response;

	UpdateDelta(true);
	if (bias_flag) {
           for (unsigned int i=0; i<bias.size(); i++)
             bias[i] -= 0.4*((*new_point)[i] - (*best_pt)[i]);
	   }

        SWAP(best_pt,temp_point);
        }
     else {
        iteration_status = 2;
	UpdateDelta(false);
	if (bias_flag)
           bias *= 0.5;
	DEBUGPR(2, ucout << "GENERATED WORSE POINTS!\n");
        }
     }

   if (Delta < Delta_min) {
      Delta_min = Delta;
      if ( ! convergence_factor.get().empty() )
         convergence_factor = Delta_init/Delta_min;
	//best_val = compute_penalty_function(best_response.function_value(0),best_cval,Delta_init/Delta_min);
      }

  best().point = *best_pt;
  debug_io(ucout);
  }

best().point = *best_pt;
final_points.add_point(problem, *best_pt);
debug_io(ucout,true);
eval_mngr().clear_evaluations();
#if COLINDONE
this->clear_evaluations();
#endif
}


void SolisWets::UpdateDelta(bool flag)
{
switch (update_id) {
  case SINGLE_EXPAND_UPDATE:
        if (flag == true) {
           n_success++;
	   n_failure=0;
           if (expand_flag == true) {
              if (n_success >= max_success) {
                 DEBUGPR(10,ucout << "SINGLE_EXPAND: Expanding step length: " 
                         << Delta << " by " << ex_factor << "\n");
                 Delta *= ex_factor;			// Expand
                 n_success=0;
                 }
	      }
           }
        else {
           n_failure++;
	   n_success=0;
           if (n_failure >= max_failure) {
              DEBUGPR(10,ucout << "SINGLE_EXPAND: Contracting step length: " 
                      << Delta << " by " << ct_factor << "\n");
              Delta *= ct_factor;
              n_failure=0;
              expand_flag = false;
              }
           }
        break;

  case SOLISWETS_DEFAULT:
        if (flag == true) {                      // Expand
           n_success++;
	   n_failure=0;
           if (n_success >= max_success) {
              DEBUGPR(10,ucout << "SW_DEFAULT: Expanding step length: " 
                      << Delta << " by " << ex_factor << "\n");
              Delta *= ex_factor;
              n_success=0;
              }
           }
        else {
           n_failure++;
	   n_success=0;
           if (n_failure >= max_failure) {
              DEBUGPR(10,ucout << "SW_DEFAULT: Contracting step length: " 
                      << Delta << " by " << ct_factor << "\n");
              Delta *= ct_factor;
              n_failure=0;
              }
           }
	break;

  }
}


void SolisWets::write(ostream& os) const
{
colin::ColinSolver<BasicArray<double>,colin::UNLP0_problem>::write(os);
 
os << "##\n## Solis-Wets Information\n##\n";
switch (neighborhood_id) {
  case NORMAL_DEV:	os << "neighborhood\tnormal" << endl; break;
  case SPHERE_DEV:	os << "neighborhood\tsphere" << endl; break;
  case UNIFORM_DEV:	os << "neighborhood\tuniform" << endl; break;
  };
os << "update_id\t" << update_id ;
if (update_id == SOLISWETS_DEFAULT)
  os << "\t\t# No restrictions on expansion and contractions" << endl;
else
  os << "\t\t# Expansions not allowed after first contraction" << endl;
os << "max_success\t" << max_success << endl;
os << "max_failure\t" << max_failure << endl;
os << "ex_factor\t" << ex_factor << endl;
os << "ct_factor\t" << ct_factor << endl;
os << "Delta_init\t" << Delta_init << endl;
os << "Delta_thresh\t" << Delta_thresh << endl;
os << "bias_flag\t" << bias_flag;
if (bias_flag)
   os << "\t\t# Using a dynamic bias in search (default)" << endl;
else
   os << "\t\t# NOT using a dynamic bias in search" << endl;
DEBUGPR(2,os << "Sigma\t" << Sigma << "\n");
}


void SolisWets::gen_new_point(NumArray<double>& new_pt, 
		NumArray<double>& mean_vec, double _Delta, bool& bound_feasible)
{
switch (neighborhood_id) {
  case SPHERE_DEV:
     {
     for (unsigned int i=0; i<new_pt.size(); i++)
       new_pt[i] = normal_dev();
     new_pt /= length(new_pt);
     for (unsigned int i=0; i<new_pt.size(); i++)
       new_pt[i] = mean_vec[i] + new_pt[i]*_Delta*Sigma[i];
     }
     break;

  case NORMAL_DEV:
     {
     for (unsigned int i=0; i<new_pt.size(); i++)
       new_pt[i] = mean_vec[i] + normal_dev()*_Delta*Sigma[i];
     }
     break;

  case UNIFORM_DEV:
     {
     for (unsigned int i=0; i<new_pt.size(); i++)
       new_pt[i] = mean_vec[i] + (2.0*unif_dev()-1.0) * _Delta * Sigma[i];
     }
     break;
  };

bound_feasible = problem->testBoundsFeasibility(new_pt);
}


void SolisWets::virt_debug_io(ostream& os, const bool finishing,
                        const int olevel) 
{
//
// This only supports verbose output
//
if (olevel < 3)
   return;

os << endl;
if (iteration_status >= 0) {
   if (iteration_status == 0) {
      if (bias_flag)
         os << "\tCurrent Point = Prev Point + (Bias Vector + Random Deviates)" << endl << endl;
      else
         os << "\tCurrent Point = Prev Point + Random Deviates" << endl << endl;
      }
   else if (iteration_status == 1) {
      if (bias_flag)
         os << "\tCurrent Point = Prev Point - (Bias Vector + Random Deviates)" << endl << endl;
      else
         os << "\tCurrent Point = Prev Point - Random Deviates" << endl << endl;
      }
   else
      os << "\tCurrent Point = Prev Point" << endl << endl;
   }

os << "\tStep Scales: ";
for (unsigned int i=0; i<Sigma.size(); i++)
  os << Delta * Sigma[i] << " ";
os << endl;

if (curr_iter == 0) {
   os << "\n\tUsing ";
   switch (neighborhood_id) {
     case SPHERE_DEV:	os << "sphere" ; break;
     case NORMAL_DEV:	os << "normal" ; break;
     case UNIFORM_DEV:	os << "uniform" ; break;
     };
   os << " deviates to generate trial points." << endl;
   if (update_id == SOLISWETS_DEFAULT)
     os << "\tNo restrictions on expansion and contractions" << endl;
   else
     os << "\tExpansions not allowed after first contraction" << endl;
   if (bias_flag)
      os << "\tUsing a dynamic bias in search (default)" << endl;
   else
      os << "\tNOT using a dynamic bias in search" << endl;
   }
}

#if 0
void SolisWets::perform_evaluation(BasicArray<double>& point, 
			SolisWets_response_t& response,
			real& val,
			real& cval, BasicArray<real >& cvals,
			bool& constraint_feasible, bool& bound_feasible)
{				
bound_feasible = problem->test_bounds_feasibility(point);
if (!bound_feasible) return;

if (problem->numNonlinearConstraints() > 0)
   problem->Eval(point,response,colin::mode_f|colin::mode_cf);
else
   problem->Eval(point,response,colin::mode_f);

compute_response_info(response,problem->state->constraint_lower_bounds,problem->state->constraint_upper_bounds,Delta_init/Delta_min,val,cval);

constraint_feasible=(cval == 0.0);
}
#endif


// Defines scolib::StaticInitializers::SolisWets_bool
REGISTER_COLIN_SOLVER_WITH_ALIAS( SolisWets,"sco:SolisWets","sco:sw", 
                                  "The SCO SolisWets optimizer" )

} // namespace scolib
