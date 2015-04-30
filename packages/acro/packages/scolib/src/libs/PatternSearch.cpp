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
// Coliny_PatternSearch.cpp
//

#include <acro_config.h>

#include <scolib/PatternSearch.h>

#include <colin/SolverMngr.h>

#include <utilib/_math.h>
#include <utilib/Uniform.h>

#define EM_MULTISTEP            0
#define EM_SIMPLE		2
#define EM_ADAPTIVE             3
#define EM_TEST                 6

#define UNKNOWN_BASIS			0
#define COORDINATE_BASIS		1
#define ROTATED_COORDINATE_BASIS	2
#define COORDINATE_TEST_BASIS		3
#define SIMPLEX_BASIS			4
#define PRIORITIZED_COORDINATE_BASIS    6

#define SINGLE_EXPAND_UPDATE	1
#define PATTERN_SEARCH_DEFAULT	0

#define BATCH_ALL		0
#define BATCH_SEQUENTIAL	1
#define BATCH_AGGRESSIVE_ASYNC	2
#define BATCH_CONSERVATIVE_ASYNC 4

#define STEP_SELECTION_FIXED	0
#define STEP_SELECTION_RANDOM	1
#define STEP_SELECTION_BIASED	2

// This macro, and the code it encodes, is simply to debug the behavior of 
// the fast pattern search method.
#define DEBUG_FPS

using namespace utilib;
using colin::EvaluationID;
using std::stringstream;

#define PENALTY_RATIO (std::pow((1.0+std::log(Delta_init/Delta_min)),2.0))

namespace scolib {

void gps_minimal_positive_basis (int n, BasicArray<NumArray<double> >& P);

//
#ifdef DEBUG_FPS
unsigned int fps_zeros=0;
IntVector    fps_zero_ctr;
#endif


////
//// GENERAL INFORMATION
////

/*
//
// Setup initial Delta to 1/4 of the bounded domain
//
stepsize(0.25);
*/

#define AnneVersion 0

PatternSearch::PatternSearch()
 : rho(-1.0),
   update_id(PATTERN_SEARCH_DEFAULT),
   basis_id(UNKNOWN_BASIS),
   last_pt_id(-1),
   reverse_flag(false),
   trunc_fps(false),
   shrink_flag(false),
   fps_batch(1),
   extended_stopping_rule(true),
   nreq_trials(0),
   simple_app(0)
{
   //utilib::OptionParser &opt = this->option;

   auto_rescale_flag = true;
   this->properties.declare
      ( "auto_rescale", 
        "If true, then automatically rescale the search for "
        "bound-constrained problems. The initial scale is 10% of "
        "the range in each dimension.",
        utilib::Privileged_Property(auto_rescale_flag) );
   //opt.add("auto_rescale",auto_rescale_flag,
   //        "If true, then automatically rescale the search for "
   //        "bound-constrained\n"
   //        "\t  problems. The initial scale is 10% of the range in "
   //        "each dimension.");

   Delta_init = 1.0;
   this->properties.declare
      ( "initial_step", 
        "Initial step length.",
        utilib::Privileged_Property(Delta_init) );
   //opt.add("initial_step",Delta_init,
   //        "Initial step length");
   //opt.alias("initial_step","initial_stepsize");
   //opt.alias("initial_step","initial_steplength");

   Delta_thresh = 1e-5;
   this->properties.declare
      ( "step_tolerance", 
        "Convergence tolerance step length",
        utilib::Privileged_Property(Delta_thresh) );
   //opt.add("step_tolerance",Delta_thresh,
   //        "Convergence tolerance step length");

   ct_factor = 0.5;
   this->properties.declare
      ( "contraction_factor", 
        "Contraction factor",
        utilib::Privileged_Property(ct_factor) );
   //opt.add("contraction_factor",ct_factor,
   //        "Contraction factor");

   num_augmented_trials = 0;
   this->properties.declare
      ( "num_augmented_trials", 
        "The number of additional trial points used during search",
        utilib::Privileged_Property(num_augmented_trials) );
   //opt.add("num_augmented_trials",num_augmented_trials,
   //        "The number of additional trial points used during search");

   max_success = 5;
   this->properties.declare
      ( "max_success", 
        "Number of successful iterations before step length is expanded",
        utilib::Privileged_Property(max_success) );
   //opt.add("max_success",max_success,
   //        "Number of successful iterations before step length is expanded");

   Sigma.resize(1);
   Sigma << 1.0;
   this->properties.declare
      ( "step_scales", 
        "The scale factors for each dimension",
        utilib::Privileged_Property(Sigma) );
   //opt.add("step_scales",Sigma,
   //        "The scale factors for each dimension");

   update_str = "default";
   this->properties.declare
      ( "update_type", 
        "Control for step length update:\n"
        "   default: expand and contract normally\n"
        "   single_expand: allow expansions until the first contraction",
        utilib::Privileged_Property(update_str) );
   //opt.add("update_type",update_str,
   //        "Control for step length update:\n"
   //        "\t  default: expand and contract normally\n"
   //        "\t  single_expand: allow expansions until the first contraction");
   
   HJ_bias = false;
   this->properties.declare
      ( "HJ_bias", 
        "If true, then use the biased steps used in the Hooke-Jeeves "
        "direct search method.  This only works with the simple and "
        "multistep exploratory_moves options.  Note that HJ_bias with "
        "multistep is equivalent to the Hooke-Jeeves direct search method.",
        utilib::Privileged_Property(HJ_bias) );
   //opt.add("HJ_bias",HJ_bias,
   //        "If true, then use the biased steps used in the Hooke-Jeeves "
   //        "direct search method.  This only works with the simple and "
   //        "multistep exploratory_moves options.  Note that HJ_bias with "
   //        "multistep is equivalent to the Hooke-Jeeves direct search method.");

   step_selection_str = "random";
   this->properties.declare
      ( "step_selection", 
        "Defines the method used to order the selection of trial steps:\n"
        "   random:  select steps in a uniformly random order\n"
        "   biased:  select steps that look like previous improving steps.  "
        "Uses the biased_selection_factor.\n"
        "   fixed:   select steps in simple numerical order.",
        utilib::Privileged_Property(step_selection_str) );
   //opt.add("step_selection",step_selection_str,
   //        "Defines the method used to order the selection of trial steps: \
   //\n\t   random:    select steps in a uniformly random order         \
   //\n\t   biased:    select steps that look like previous improving steps. \
   //\n\t              Uses the biased_selection_factor.             \
   //\n\t   fixed:     select steps in simple numerical order.");

   biased_selection_factor = 0.9;
   this->properties.declare
      ( "biased_selection_factor", 
        "Determines how the deterministic biased selection scheme is "
        "is used to order the trial steps in the pattern.  This value must be "
        "greater than zero and less-than-or-equal to one.  The closer the "
        "value is to one the more the last improving search direction is used "
        "as a bias.",
        utilib::Privileged_Property(biased_selection_factor) );
   //opt.add("biased_selection_factor",biased_selection_factor,
   //        "Determines how the deterministic biased selection scheme is \
   //\n\t   is used to order the trial steps in the pattern.  This value must be \
   //\n\t   greater than zero and less-than-or-equal to one.  The closer the \
   //\n\t   value is to one the more the last improving search direction is used \
   //\n\t   as a bias.");

   basis_str = "coordinate";
   this->properties.declare
      ( "basis", 
        "The type of basis used for search:\n"
        "   coordinate:         2n directions along the coordinate axes\n"
        "   rotated_coordinate: 2n orthogonal directions which have been rotated\n"
        "   simplex:            n+1 directions in a regular simplex",
        utilib::Privileged_Property(basis_str) );
   //opt.add("basis",basis_str,
   //        "The type of basis used for search:\n"
   //"\t  coordinate: 2n directions along the coordinate axes\n"
   //"\t  rotated_coordinate: 2n orthogonal directions which have been rotated\n"
   //        "\t  simplex:    n+1 directions in a regular simplex");

   em_str = "simple";
   this->properties.declare
      ( "exploratory_move", 
        "The type of exploratory move used:\n"
        "   multi_step: Check for improvement in each search direction "
        "iteratively. Adapt the pattern to keep improvements when detected "
        "and look for improvement in remaining search directions.\n"
        "   simple: Use a simple pattern that is not adapted.  The entire "
        "search can be parallelized with the 'batch_mode' option.\n"
        "   adaptive: Adaptively modifies the search pattern to minimize the "
        "number of function evaluations required. This code has strong "
        "serial components, but some elements can be parallelized with the "
        "'batch_mode' option.",
        utilib::Privileged_Property(em_str) );
   //   opt.add("exploratory_move",em_str,
   //           "The type of exploratory move used:\n"
   //"\t  multi_step: Check for improvement in each search direction\n"
   //"\t     iteratively. Adapt the pattern to keep improvements when detected\n"
   //"\t     and look for improvement in remaining search directions.\n"
   //"\t  simple: Use a simple pattern that is not adapted.  The entire search\n"
   //"\t     can be parallelized with the 'batch_mode' option.\n"
   //"\t  adaptive: Adaptively modifies the search pattern to minimize the\n"
   //"\t     number of function evaluations required. This code has strong\n"
   //"\t     serial components, but some elements can be parallelized with the\n"
   //"\t     'batch_mode' option."
   //);

   Debug_success = false;
   this->properties.declare
      ( "debug_success", 
        "If true, then record whether each iteration is successful",
        utilib::Privileged_Property(Debug_success) );
   //opt.add("debug_success",Debug_success,
   //        "If true, then record whether each iteration is successful");

   sufficient_decrease_coef = 0.01;
   this->properties.declare
      ( "alpha", 
        "Sufficient decrease parameter",
        utilib::Privileged_Property(sufficient_decrease_coef) );
   //opt.add("alpha",sufficient_decrease_coef, 
   //        "Sufficient decrease parameter");

   ex_factor = 2.0; 
   this->properties.declare
      ( "expansion_factor", 
        "Expansion factor",
        utilib::Privileged_Property(ex_factor) );
   //opt.add("expansion_factor",ex_factor,
   //        "Expansion factor");

   reset_signal.connect(boost::bind(&PatternSearch::reset_PatternSearch, this));
}


PatternSearch::~PatternSearch()
{
   if ( simple_app != NULL )
      delete simple_app;
}


void PatternSearch::set_problem(const utilib::AnyRef& problem_)
{
   // IF the problem is already an UNLP0, then we are good to go.
   if (problem_.type() == typeid(colin::Problem<colin::UNLP0_problem>))
   {
      problem = problem_.expose<colin::Problem<colin::UNLP0_problem> > ();
   }
   else
   {
      // Otherwise, we will lexical_cast the problem we are handed into
      // a NLP0 using the problem manager and then BY HAND convert the
      // NLP0 to an UNLP0.  This way, we get to keep a handle on the
      // ConstraintPenaltyApplication so we can set the penalty term.
      try
      {
         colin::Problem<colin::NLP0_problem> tmp_problem;
         colin::ProblemMngr().lexical_cast(problem_, tmp_problem);
         simple_app = new colin::ConstraintPenaltyApplication
            <colin::UNLP0_problem>(tmp_problem);
         problem.set_application(simple_app);
      }
      catch (utilib::bad_lexical_cast& err)
      {
         std::cerr << "PatternSearch::set_problem - Problem casting "
            "problem into NLP0 for recasting through a "
            "ConstraintPenaltyApplicationuse." << std::endl;
         throw;
      }
   }
}


void PatternSearch::reset_PatternSearch()
{
   if ( problem.empty() ) return;
/*
if ((problem->numConstraints() > 0) &&
    ((simple_app == 0) || (problem.application() != simple_app))) {
   simple_app = new colin::ConstraintPenaltyApplication<colin::NLP0_problem>(problem.application());
   problem.set_application(simple_app);
   }
*/
  double SHRINK_BOUND = 8.0;
  
  unsigned int nvars = problem->num_real_vars;
  if (nvars == 0) return;
  
  bc_flag = problem->enforcing_domain_bounds;
  if (bc_flag)
  {
     lower_bc = problem->real_lower_bounds;
     upper_bc = problem->real_upper_bounds;
  }
  
  Sigma.resize(nvars);
  range.resize(nvars);
  if (auto_rescale_flag && bc_flag) {
    for (unsigned int i=0; i<nvars; i++)
      if ((upper_bc[i] == real :: positive_infinity) ||
	  (lower_bc[i] == real :: negative_infinity)) {
        Sigma[i] = 1.0;
	range[i] = real :: positive_infinity;
        }
      else {
        Sigma[i] = std::max((static_cast<double>(upper_bc[i])-static_cast<double>(lower_bc[i]))/10.0,1e-5);
        range[i] = upper_bc[i] - lower_bc[i];
        }
  }
  else
    Sigma << 1.0;
  
  if (Debug_success)
    succ_history.resize(100);
  
  tvec.resize(nvars);
  
#if AnneVersion
  step_selection_str = "fixed";
  //  basis_str = "prioritized_test"; 
  basis_str = "coordinate"; 
#endif
  
  if(em_str == "multi_step")
    em_case = EM_MULTISTEP;
  else if(em_str == "simple")
    em_case = EM_SIMPLE;
  else if(em_str == "adaptive")
    em_case = EM_ADAPTIVE;
  else if(em_str == "test"){ // %%%% pls---changed 6/03/04
    em_case = EM_TEST;
    step_selection_str = "fixed";
    basis_str = "prioritized_test";
    basis_id = PRIORITIZED_COORDINATE_BASIS; 
    update_str = "default";
  }
  else {
    EXCEPTION_MNGR(runtime_error,"PatternSearch::reset -- Bad move string: \"" << em_str << "\"\n\t\tValid Choices: multi_step, simple, adaptive");
  }
  
  if (update_str == "default")
    update_id = PATTERN_SEARCH_DEFAULT;
  else if (update_str == "single_expand")
    update_id = SINGLE_EXPAND_UPDATE;
  else {
    EXCEPTION_MNGR(runtime_error, "PatternSearch::reset -- bad update string\n\t\tValid modes: default, single_expand");
  }
  
  if (step_selection_str == "random")
    step_selection_id = STEP_SELECTION_RANDOM;
  else if (step_selection_str == "biased")
    step_selection_id = STEP_SELECTION_BIASED;
  else if (step_selection_str == "fixed")
    step_selection_id = STEP_SELECTION_FIXED;
  else {
    EXCEPTION_MNGR(runtime_error, "PatternSearch::reset -- bad step selection string\n\t\tValid modes: random, biased, fixed");
  }
  
  if (HJ_bias && !((em_case == EM_SIMPLE) || (em_case == EM_MULTISTEP)))
    EXCEPTION_MNGR(runtime_error, "PatternSearch::reset -- incompatable HJ_bias with exploratory moves: \n\t\t" << em_str );
      
  if ( problem->enforcing_domain_bounds &&               
      ((basis_str != "coordinate") && (basis_str != "prioritized_test")) ){
    EXCEPTION_MNGR(runtime_error,"PatternSearch::reset - must specify coordinate pattern for bound-constrained problems.");
  }
  // %%%% FIXME::: Do we still need this? --pls 7/04
  last_pt_id = -1;  // %%%% added 6/04 pls
  
  // We always recreate this, since the pattern can be reflected by the optimizer
  //
  if (1) {
    if (basis_str == "simplex") {
      basis_id = SIMPLEX_BASIS;
      scolib::gps_minimal_positive_basis(nvars, pattern);
      ncore_trials = nvars+1;
      ntrials=nvars+1+num_augmented_trials;
      pattern.resize(ntrials);
      for (unsigned int j=nvars+1; j<ntrials; j++)
	pattern[j].resize(nvars);
    }
    
    else if (basis_str == "coordinate") {
      basis_id = COORDINATE_BASIS;
      pattern.resize(num_augmented_trials);
      ncore_trials = 2*nvars;
      ntrials=2*nvars+num_augmented_trials;
      for (unsigned int i=0; i<pattern.size(); i++)
	pattern[i].resize(nvars);
    }
    
    // %%%% Added 7/04 pls
    //  At this point we are not allowing augmented trials,
    //   except for the ones we do automatically; i.e. the
    //   speculative directions.
   else if (basis_str == "prioritized_test") {
     basis_id = PRIORITIZED_COORDINATE_BASIS;
     update_id = PATTERN_SEARCH_DEFAULT;
     // pattern.resize(num_augmented_trials);
     ncore_trials = 2*nvars;
     nreq_trials = 2*nvars;
     num_augmented_trials =  0;
     ntrials=2*nreq_trials;
     for (unsigned int i=0; i<pattern.size(); i++)
       pattern[i].resize(nvars);
   }
    
    
    else if (basis_str == "coordinate_test") { 
      basis_id = COORDINATE_TEST_BASIS;
      ncore_trials = 2*nvars;
      ntrials=2*nvars+num_augmented_trials;
      pattern.resize(ntrials);
      for (unsigned int i=0; i<ntrials; i++) {
	pattern[i].resize(nvars);
	pattern[i] << 0.0;
	if (i < nvars)
	  pattern[i][i] = 1;
	else if (i < 2*nvars)
	  pattern[i][i-nvars] = -1;
      }
    }
    
    else if (basis_str == "rotated_coordinate") {
      basis_id = ROTATED_COORDINATE_BASIS;
      ncore_trials = 2*nvars;
      ntrials=2*nvars+num_augmented_trials;
      pattern.resize(ntrials);
      int flag = nvars % 2;
      for (unsigned int i=0; i<ntrials; i++) {
	pattern[i].resize(nvars);
	pattern[i] << 0.0;
     }
     double tmp = std::sqrt(2.0)/2.0;
     for (unsigned int j=0; j<nvars; ) {
       if (flag && (j == nvars-3)) {
	 pattern[j][j]     = 1/2.0;
	 pattern[j][j+1]   = 1/2.0;
	 pattern[j][j+2]   = -tmp;
	 
	 pattern[j+1][j]   = (tmp-1)/2.0;
	 pattern[j+1][j+1] = (tmp+1)/2.0;
	 pattern[j+1][j+2] = 1/2.0;
	 
	 pattern[j+2][j]   = (tmp+1)/2.0;
	 pattern[j+2][j+1] = (tmp-1)/2.0;
	 pattern[j+2][j+2] = 1/2.0;
	 j = nvars;
       }
       else {
	 pattern[j][j]     = tmp;
	 pattern[j][j+1]   = tmp;
	 
	 pattern[j+1][j]   = tmp;
	 pattern[j+1][j+1] = -tmp;
	 j += 2;
       }
     }
     {
       for (unsigned int j=0; j<nvars; j++) {
	 pattern[j+nvars] << pattern[j];
	 pattern[j+nvars] *= -1.0;
       }
     }
   }
   
   //
   // Generate random patterns that are linear combinations of the core
   // patterns.  NOTE:  should this be done only once?  How about a random
   // pattern every time a non-core trial is requested???  That seems expensive.
   //
   if (num_augmented_trials>0) {
      if (!rng)
         EXCEPTION_MNGR(runtime_error, "PatternSearch::reset - No random number generator specified, but one is needed to generate augmented trial steps");

      Uniform urnd(&rng);
      double tmp;
      for (unsigned int j=pattern.size()-num_augmented_trials; j<pattern.size(); j++) {
	if (basis_str == "coordinate") {
           for (unsigned int k=0; k<nvars; k++) {
             tmp = urnd();
             int val = (tmp < 0.333333? -1 : (tmp > 0.66666666? 1 : 0));
	     pattern[j][k] = val;
	     }
	   }
	else {
	   pattern[j] << 0.0;
	   for (unsigned int ii=0; ii<nvars+1; ii++) {
             tmp = urnd();
             int val = (tmp < 0.333333? -1 : (tmp > 0.66666666? 1 : 0));
             for (unsigned int k=0; k<nvars; k++)
	       pattern[j][k] += val*pattern[ii][k];
	     }
	   }
        }
      }
}


switch (em_case) {

  case EM_ADAPTIVE:
	FA.extended_ctr = 0;
	FA.succ_flag = -1;
	FA.rho_prev = 1.0;
	FA.first =1;

  default:
	break;
  };

bias.resize(nvars);
bias << 0.0;

if (ndx.size() != ntrials)
  ndx.resize(ntrials);
for (unsigned int i=0; i<ndx.size(); i++) 
  ndx[i] = i;
if (step_selection_id == STEP_SELECTION_BIASED) {
   step_bias.resize(ntrials);
   improving_trial.resize(nvars);
   improving_trial << 0.0;
   }

#ifdef DEBUG_FPS
scolib::fps_zeros=0;
scolib::fps_zero_ctr.resize(0);
#endif

//
// Setup miscellaneous configuration controls
//
 expand_flag=true;
 n_success=0;
 Delta_min = Delta = Delta_init;
 Delta_bound = Delta_thresh * SHRINK_BOUND;
 rho = sufficient_decrease_coef*Delta*Delta;
}



void PatternSearch::optimize()
{
//
// Misc initialization of the optimizer
//
//opt_init();
if (!(this->initial_point_flag))
   EXCEPTION_MNGR(runtime_error,"PatternSearch::minimize - no initial point specified.");
if (problem->num_real_vars != initial_point.size())
   EXCEPTION_MNGR(runtime_error,"PatternSearch::minimize - problem has " <<
                  problem->num_real_vars << 
                  " real params, but initial point has " << 
                  initial_point.size() );
if (initial_point.size() == 0) {
   solver_status.termination_info = "No-Real-Params";
   return;
   }
   
//
// Setup misc data structures
//
real prev_fret;
best_curr.resize(problem->num_real_vars);
NumArray<double> x(problem->num_real_vars.as<size_t>());
x << initial_point;
if (simple_app)
   simple_app->constraint_penalty = PENALTY_RATIO;
colin::AppRequest request = problem->set_domain(x);
problem->Request_response(request, best().response);
problem->Request_F(request, best().value());
eval_mngr().perform_evaluation(request);
if (Debug_success)
   succ_history.set(0);
//
// Get the initial point and make sure that it's feasible w.r.t. bounds
//
bool infeasible=false;
if (bc_flag) {
   for (unsigned int i=0; i<x.size(); i++) {
     if ((problem->realLowerBoundType(i) == colin::hard_bound) &&
         ((x[i]+constraint_tolerance) < lower_bc[i])) {
	infeasible=true;
	x[i] = lower_bc[i];
        }

     else if ((problem->realUpperBoundType(i) == colin::hard_bound) &&
         ((x[i]-constraint_tolerance) > upper_bc[i])) {
	infeasible=true;
	x[i] = upper_bc[i];
        }

     else if (problem->realLowerBoundType(i) == colin::periodic_bound) {
        if ((x[i]+constraint_tolerance) < (lower_bc[i]-range[i]))
	   infeasible=true;
        while (x[i] < lower_bc[i])
           x[i] += range[i];
        }

     else if (problem->realUpperBoundType(i) == colin::periodic_bound) {
        if ((x[i]-constraint_tolerance) > (upper_bc[i]+range[i])) 
	   infeasible=true;
        while (x[i] > upper_bc[i])
          x[i] -= range[i];
        }
     }
   if (infeasible)
      EXCEPTION_MNGR(runtime_error, "PatternSearch::minimize - Warning: initial point was infeasible w.r.t. bound constraints!");
   }
//
// Setup the iteration counters and do debugging IO
//
unsigned int num_iters;
if (max_iters <= 0)
   num_iters = MAXINT;
else
   num_iters = curr_iter + max_iters;
debug_io(ucout);
//
// Iterate...
//
best_curr << x;
x << initial_point;
for (curr_iter++; curr_iter <= num_iters; curr_iter++) {
  
  if (Delta <= Delta_thresh) {
     stringstream tmp;
     tmp << "Step-Length Delta=" << Delta << "<="
		<< Delta_thresh << "=Delta_thresh";
     solver_status.termination_info = tmp.str();
     break;
     }

  if (check_convergence())
     break;

  prev_fret = best().value();
  if (ExploratoryMoves(x,best().response,best().value(),best().constraint_violation) && HJ_bias) {
     bias << x;
     bias -= best_curr;
     }

  DEBUGPR(10, ucout << "Fret " << best().value() << "\t" << "Prev Fret " 
          << prev_fret << "\n");
  if (best().value() == prev_fret) {
     if (!HJ_bias || (bias == 0.0)) {
        UpdateMatrix(false);
        UpdateDelta(false);
        update_pattern(best_curr,x,false);
        }
     if (HJ_bias)
        bias << 0.0;
     DEBUGPR(10,ucout << "Delta: " << Delta << "\t" << "Unimproved TMPX\n");
     }
   else {
     UpdateMatrix(true);
     UpdateDelta(true);
     update_pattern(best_curr,x,true);
     DEBUGPR(10, ucout << "Delta: " << Delta << "\t" << "Improved TMPX\n");
     if (step_selection_id == STEP_SELECTION_BIASED) {
        //
        // Update the improving_trial vector
        //
        for (size_type i=0; i<x.size(); i++)
          improving_trial[i] = biased_selection_factor *
	    (x[i]-best_curr[i]) + improving_trial[i] *
	    (1.0-biased_selection_factor);
        }
     }
  DEBUGPR(100, ucout << "Bias " << bias << "\n");
  if (Delta < Delta_min) {
     Delta_min = Delta;
     if (simple_app)
        simple_app->constraint_penalty = PENALTY_RATIO;
     colin::AppRequest request = problem->set_domain(x);
     problem->Request_response(request, best().response);
     problem->Request_F(request, best().value());
     eval_mngr().perform_evaluation(request);
     }

  best_curr << x;
  debug_io(ucout);
  }
//
// Perform debugging IO
//
if (Debug_success) {
   succ_history.resize(curr_iter+1);
   ucout << "[SuccHistory:\t" << succ_history << "]\n";
   }
debug_io(ucout,true);
//
// Finish up
//
eval_mngr().clear_evaluations();
best().point = best_curr;
final_points.add_point(problem, best_curr);
}

void PatternSearch::UpdateDelta(bool flag)
{
update_flag=0;
if (em_case == EM_ADAPTIVE) {
   if (Debug_success) {
      if (curr_iter >= succ_history.size())
         succ_history.resize(succ_history.size()+100);
      succ_history.put(curr_iter,flag);
      }
   if (flag == true) {                      // Expand
      if (extended_stopping_rule == true)
         FA.extended_ctr=0;
      n_success++;
      if (n_success >= max_success) {
         Delta *= FA.rho_prev;
         if (FA.rho_prev > 1.0)
            update_flag = 1;
	 n_success=0;
         }
      //
      // This is a bit of a hack.  The adaptive_pattern method was originally
      // designed to implicitly expand the step length using a "delay" for
      // the underlying expansion on Delta.  However, this needs to be hacked
      // a bit to accomodate the case where max_success > 1.
      //
      if ((n_success+1) < max_success)
         FA.rho = 1.0;
      }
   else {
      n_success=0;
      if (extended_stopping_rule == true) {
         //
         // If the new step length would fall below Delta_thresh, then delay
         // this reduction until 'enough' (all) of the steps have been 
	 // considered.  This code assumes that termination is based on
	 // a simple step-length criterion, so it is not as general as I'd
	 // like.  However, this _is_ the hard-coded stopping rule in GPSOpt
	 // right now...
	 //
	 if ((Delta*ct_factor) <= Delta_thresh) {
	    FA.extended_ctr++;
	    if (FA.extended_ctr == ncore_trials) {
               Delta *= ct_factor;
               update_flag = 2;
               }
            else
               update_flag = 3;
            }
         else {
            Delta *= ct_factor;
            update_flag = 2;
            }
         }
      else {
	 //
         // Terminate when the shortest edge on the pattern becomes too small,
         // so always shrink, even if the step scale falls below the threshold.
	 //
         Delta *= ct_factor;
         update_flag = 2;
         }
      }

} else {

//
// The default update
//
if (Debug_success) {
   if (curr_iter >= succ_history.size())
      succ_history.resize(succ_history.size()+100);
   succ_history.put(curr_iter,flag);
   }
update_flag=0;
switch (update_id) {
  case SINGLE_EXPAND_UPDATE:
        if (flag == true) {
	  shrink_flag = false;    // %%% added 7/04 pls
	  n_success++;
	  if (expand_flag == true) {
	    if (n_success >= max_success) {
	      Delta *= ex_factor;         // Expand
	      update_flag = 1;
	      n_success=0;
	    }
	  }
	}
        else {
	  n_success=0;
	  Delta *= ct_factor;                          // Shrink
	  update_flag = 2;
	  expand_flag = false;
	  shrink_flag = true;    // %%% added 7/04 pls
	}
        break;
	
 case PATTERN_SEARCH_DEFAULT:
   if (flag == true) {                      // Expand
     shrink_flag = false;    // %%% added 7/04 pls
     n_success++;
     if (n_success >= max_success) {
       Delta *= ex_factor;
       update_flag = 1;
       n_success=0;
     }
   }
   else {
     n_success=0;
     Delta *= ct_factor;                  // Shrink
     update_flag = 2;
     shrink_flag = true;    // %%% added 7/04 pls
   }
   break;
}
}
rho = sufficient_decrease_coef*Delta*Delta;
}


void PatternSearch::generate_trial(int id, const DoubleVector& x_, 
				   DoubleVector& trial, double scale, 
				   bool& feasible, const DoubleVector& _bias)
{
feasible=true;

if (debug > 4) {
   ucout << "Current Point:   " << x_ << std::endl;
   }

size_type curr = ndx[id];
switch (basis_id) {
 case COORDINATE_BASIS:
   {
   trial << x_;
   if (curr < x_.size()) {       // positive directions
     trial[curr] += _bias[curr];
     trial[curr] += scale*Sigma[curr];
     if (bc_flag) {
	if ((problem->realUpperBoundType(curr) == colin::hard_bound) &&
            (trial[curr] > upper_bc[curr]))
              feasible=false;
        else if ((problem->realUpperBoundType(curr) == colin::periodic_bound) &&
                 (trial[curr] > (upper_bc[curr]+range[curr])))
              feasible=false;
        }
   }
   else {                        // negative directions
     int ndx = curr-x_.size();
     trial[ndx] += _bias[ndx];
     trial[ndx] -= scale*Sigma[ndx];
     if (bc_flag) {
	if ((problem->realLowerBoundType(ndx) == colin::hard_bound) &&
            (trial[ndx] < lower_bc[ndx]))
              feasible=false;
        else if ((problem->realLowerBoundType(ndx) == colin::periodic_bound) &&
                 (trial[ndx] < (lower_bc[ndx]-range[ndx])))
              feasible=false;
        }
   }
   }
   break;
   
 // FIXME:::  Make sure this is not messing up the indexing!!!!
//  Also, we need to set a flag so we know if we are shrinking,
//  in which case, we will accelerate the contraction for the
//  spec. directions.  
 case PRIORITIZED_COORDINATE_BASIS:
   {
   double mult_const = 3.0;
   double multiplier;
   size_type sz = curr;
   DoubleVector temp;
   HJ_bias = false;
   temp << x_;
   //  trial << x_;
   if (sz >= 2*x_.size()){
     sz -= 2*x_.size();
   }
   ucout<< "\nsz == "<<sz<<", curr == "<<curr<<", and x_.size =="<<x_.size();
   ucout<<std::endl;
   if (sz < x_.size()) {         // positive directions
     temp[sz] += _bias[sz];
     temp[sz] += scale*Sigma[sz];
   }
   else {                        // negative directions
     temp[sz-x_.size()] += _bias[sz-x_.size()];
     temp[sz-x_.size()] -= scale*Sigma[sz-x_.size()];
   }
   if(sz == curr){
     trial << temp; // will this work?
   }
   // For speculative direction, find the associated core 
   //   direction, then multiply by the appropriate multiplier
   //   Currently we have that hard-wired to 3.0, but we need to
   //   think about what it should be.
 
   else{            // speculative directions
     feasible = true;
     if (shrink_flag){
       multiplier = 1/mult_const;
     }
     else{
       multiplier = mult_const;
     }
     /*
     for(unsigned int q = 0; q < x_.size(); q++){
       trial[q] = multiplier * temp[q];
     }
     */
     sz = (sz >=  x_.size())? sz -  x_.size() : sz;
     trial[sz] = multiplier * temp[sz];
   }
   sz = (sz >=  x_.size())? sz -  x_.size() : sz;
   // Because of the way we find the spec. directions, we must check
   //   the whole vector for feasibility, not just one element.
   /*
   if(bc_flag){
     for(unsigned int q = 0; q < x_.size(); q++){
       if( (trial[q]  > upper_bc[q])
	   || (trial[q]  < lower_bc[q]) ){
	 feasible = false;
	 break;
       }
     }
   }
   */
   if ( (bc_flag) && (
          ((problem->realUpperBoundType(sz) == colin::hard_bound) && 
	    (trial[sz] > upper_bc[sz])) ||
          ((problem->realLowerBoundType(sz) == colin::hard_bound) && 
	    (trial[sz] < lower_bc[sz])) ||
          ((problem->realUpperBoundType(sz) == colin::periodic_bound) && 
	    (trial[sz] > (upper_bc[sz]+range[sz]))) ||
          ((problem->realLowerBoundType(sz) == colin::periodic_bound) && 
	    (trial[sz] < (lower_bc[sz]-range[sz])))
        ))
        feasible = false;
   }
   break;

 default:
   {for (unsigned int i=0; i<x_.size(); i++) {
     trial[i] = x_[i] + _bias[i] + pattern[curr][i] * scale * Sigma[i];
     if ( (bc_flag) && (
          ((problem->realUpperBoundType(i) == colin::hard_bound) && 
	    (trial[i] > upper_bc[i])) ||
          ((problem->realLowerBoundType(i) == colin::hard_bound) && 
	    (trial[i] < lower_bc[i])) ||
          ((problem->realUpperBoundType(i) == colin::periodic_bound) && 
	    (trial[i] > (upper_bc[i]+range[i]))) ||
          ((problem->realLowerBoundType(i) == colin::periodic_bound) && 
	    (trial[i] < (lower_bc[i]-range[i])))
        ))
        feasible = false;
     }
   }
   break;
}
 
if (bc_flag) {
   for (unsigned int i=0; i<x_.size(); i++) {
     if (problem->hasPeriodicRealBound(i)) {
        if      (trial[i] > upper_bc[i]) trial[i] -= range[i];
        else if (trial[i] < lower_bc[i]) trial[i] += range[i];
        }
     }
   }

 if (HJ_bias) {
   if (basis_id == COORDINATE_BASIS) {
      feasible=false;
      for (unsigned int i=0; i<x_.size(); i++)
        if (std::fabs(trial[i]-best_curr[i]) > scale*Sigma[i]/2.0) {
           feasible=true;
	   break;
	   }
      }
   else {
      feasible=false;
      for (unsigned int i=0; i<x_.size(); i++)
        if (std::fabs(trial[i]-best_curr[i]) > scale*Sigma[i]/2.0) {
           feasible=true;
	   break;
	   }
      }
   }

if (debug > 4) {
   ucout << "Trial Point:     " << trial << std::endl;
   ucout << "BIAS:            " << _bias << std::endl;
   ucout << "Feasible:        " << feasible << std::endl;
   }
if (debug > 4) {
   double val=0.0;
   for (unsigned int i=0; i<x_.size(); i++)
     val += (trial[i]-x_[i])*(trial[i]-x_[i]);
   ucout << "Trial Point Length: " << std::sqrt(val) << std::endl;
   }
  
//
// Collect running stats
//
if (output_level == 3) {
   ntrial_points++;
   if (feasible == true) nfeasible_points++;
   }
}


////
//// ITERATION CONTROLS
////


bool PatternSearch::ExploratoryMoves(DoubleVector& x_, response_t& response, real& _best_val, real& _best_cval)
{
ntrial_points=nfeasible_points=0;

switch (em_case) {
	case EM_SIMPLE:
		return EM_simple(x_, response, _best_val, _best_cval);

	case EM_MULTISTEP:
		return EM_multistep(x_, response, _best_val, _best_cval);

	case EM_ADAPTIVE:
		return EM_adaptive(x_, response, _best_val, _best_cval);

	case EM_TEST:
		return EM_test(x_, response, _best_val, _best_cval);

	default:
		break;
	}

return false;
}


//
// "simple" exploratory moves
//
bool PatternSearch::EM_simple(DoubleVector& x_, response_t& response,
			real& _best_val, real& _best_cval)
{
update_step_order();

bool any_feasible=false;
for (unsigned int i=0; i<ntrials; i++) {
  bool feasible;
  generate_trial(i, x_, tvec, Delta, feasible, bias);
  if (feasible) {
     colin::AppRequest request = problem->set_domain(tvec);
     problem->Request_F(request);
     eval_mngr().queue_evaluation(request);
     any_feasible=true;
     }
  }

//
// Execute evaluations if _some_ trial point was feasible.
//
if (any_feasible) {
   response_t tmp_response;
   eval_mngr().find_best_improving(tmp_response, _best_val-rho);
   if (!tmp_response.empty()) {
      //std::cerr << "HERE " << response->value<double>(colin::f_info);
      response = tmp_response;
      response.get(colin::f_info, _best_val);
      //std::cerr << " " << _best_val << std::endl;
      response.get_domain(x_);
      return true;
      }
   }
//
// Otherwise ... return 'false'
//
return false;
}


//
// Check for improvement in each pattern iteratively,
// keeping improvements and checking new pattern wrt to them.
//
bool PatternSearch::EM_multistep(DoubleVector& x_, response_t& response,
			real& _best_val, real& _best_cval)
{
update_step_order();

real ans=_best_val, cval;
response_t curr_response;
//response_t tmp_response;

for (unsigned int i=0; i<ntrials; i++) {
  bool feasible;
  generate_trial(i, x_, tvec, Delta, feasible, bias);
  if (feasible) {
     real tmp_ans = ans;
     real tmp_cval;
     if (simple_app)
        simple_app->constraint_penalty = PENALTY_RATIO;
     colin::AppRequest request = problem->set_domain(tvec);
     problem->Request_response(request, tmp_response);
     problem->Request_F(request, tmp_ans);
     eval_mngr().perform_evaluation(request);
     //
     // If improving, update current info and keep going
     //
     if (tmp_ans < (ans-rho)) {
        x_ << tvec;
        ans = tmp_ans;
	cval = tmp_cval;
	curr_response = tmp_response;
	}
     }
  }

bool status = (ans < (_best_val-rho));
if (status) {
   _best_val = ans;
   _best_cval = cval;
   response = curr_response;
   }
  
return status;
}


#if 0
//
// Check for improvement in each pattern iteratively,
// until an improvement is found.  Use a bias vector to guide
// the search into interesting regions.  This idea is inspired by
// the bias method in Solis-Wets.
//
// NOTE: this method was never very successful, so it has been suplanted
// with the HJ_bias flag.
//
bool PatternSearch::EM_biased_simple(DoubleVector& x_, response_t& response,
					real& _best_value, real& _best_cval,
					DoubleVector& tmpx)
{
bool better=false;
//
// Evaluate biased trial points
//
tmpx << x_;
DEBUGPR(10, ucout << "Current Point :\t" << tmpx << '\n');
tmpx += bias;
if ( problem->enforcing_domain_bounds ) {
   for (unsigned int i=0; i<tmpx.size(); i++)
     if (tmpx[i] > upper_bc[i])
        tmpx[i] = upper_bc[i];	    
     else if (tmpx[i] < lower_bc[i])
        tmpx[i] = lower_bc[i];	    
   }

for (unsigned int i=0; i<ntrials; i++) {
  bool feasible;
  generate_trial(i, tmpx, tvec, Delta, feasible, bias);
  if (feasible) {
     double ans;
     problem->EvalF(tvec,ans);
     if (ans < (*fret-rho)) {
	better = true;
	tmpx << tvec;
        *fret = ans;
	break;
	}
     }
  }
 
if (!better) {
   bias /= 2.0;
   tmpx << x_;
   tmpx += bias;
for (unsigned int i=0; i<ntrials; i++) {
  bool feasible;
  generate_trial(i, tmpx, tvec, Delta, feasible, bias);
  if (feasible) {
     double ans;
     problem->EvalF(tvec,ans);
     if (ans < (*fret-rho)) {
	better = true;
	tmpx << tvec;
        *fret = ans;
	break;
	}
     }
  }
  }

if (!better) {
   bias = 0.0;
   tmpx << x_;
for (unsigned int i=0; i<ntrials; i++) {
  bool feasible;
  generate_trial(i, tmpx, tvec, Delta, feasible, bias);
  if (feasible) {
     double ans;
     problem->EvalF(tvec,ans);
     if (ans < (*fret-rho)) {
	better = true;
	tmpx << tvec;
        *fret = ans;
	break;
	}
     }
  }
  }

if (better) {
   DEBUGPR(10, ucout << "TMPX    :\t" << tmpx << '\n');
   bias *= 0.2;
   bias += 0.4*(tmpx-x_);
   DEBUGPR(10, ucout << "BIAS NEW:\t" << bias << '\n');
   }
else {
   tmpx << x_;
   }

return false;
}
#endif




#if 0
namespace {

//
// This version of subshuffle seems rather specific to the code in 
// EM_adaptive, so I'm making it local to this file.
//
template <class RNGT, class IndexT>
void subshuffle(BasicArray<unsigned int>& vec, RNGT* rng, IndexT index)
{
if ((unsigned int)(index+1) != vec.size())
   swap(vec[index],vec[vec.size()-1]);

utilib::subshuffle(vec,rng,0,vec.size()-2);
}

}
#endif


void PatternSearch::update_step_order(int id)
{
if (step_selection_id == STEP_SELECTION_RANDOM) {
   if (id == -1)
      shuffle(ndx,&rng);
   else {
      if ((unsigned int)(id+1) != ndx.size())
         std::swap(ndx[id],ndx[ndx.size()-1]);
      utilib::subshuffle(ndx,&rng,0,ndx.size()-2);
      }
   }

else if (step_selection_id == STEP_SELECTION_BIASED) {
   if ((id != -1) && ((unsigned int)(id+1) != ndx.size()))
      std::swap(ndx[id],ndx[ndx.size()-1]);
   size_type tmp = (id == -1 ? ntrials : ntrials-1);
   for (size_type i=0; i<tmp; i++)
     if (basis_id == COORDINATE_BASIS)
        step_bias[ndx[i]] = (ndx[i] >= tvec.size() ? - improving_trial[ndx[i]-tvec.size()] :
				improving_trial[ndx[i]]);
     else
        step_bias[ndx[i]] = inner_product(improving_trial, pattern[ndx[i]]);
   //
   // Do a simple sort since these lists will probably not get very long
   //
   int ctr=1;
   while (ctr > 0) {
     ctr=0;
     for (size_type i=0; i<(tmp-1); i++)
       if (step_bias[ndx[i]] < step_bias[ndx[i+1]]) {
          std::swap(ndx[i],ndx[i+1]);
	  ctr++;
          }
     }
   DEBUGPR(100, ucout << "SELECTION BIAS INFO:\n";
           ucout << "Bias " << improving_trial << std::endl;
           for (size_type i=0; i<tmp; i++)
              ucout << ndx[i] << "\t" << step_bias[ndx[i]] << std::endl;
           );
   }

else if (step_selection_id == STEP_SELECTION_FIXED) {
   if ((id != -1) && ((unsigned int)(id+1) != ndx.size()))
      std::swap(ndx[id],ndx[ndx.size()-1]);
   }
}

//
// Fast adaptive unconstrained EPSA
//
bool PatternSearch::EM_adaptive(DoubleVector& x_, response_t& response,
					real& _best_val, real& _best_cval)
{
real init_value = _best_val;
bool feasible;
unsigned int i;

if (FA.first == 1) {
   FA.first=0;
   //
   // First iteraton of the pattern search (after the call to 'reset'),
   // which starts the algorithm looking like it has just completed a successful
   // iteration.
   //
   FA.succ_flag = true;
   update_step_order(0);
   DEBUGPR(10, ucout << "Initial Point: " << x_ << std::endl);
   DEBUGPR(10, ucout << "Trial step : " << (ntrials-1) << 
           " Len : " << Delta << std::endl);
   generate_trial((ntrials-1), x_, tvec, Delta, feasible, bias);
   if (feasible) {
      real ans;
      real cval;
      if (simple_app)
         simple_app->constraint_penalty = PENALTY_RATIO;
      colin::AppRequest request = problem->set_domain(tvec);
      problem->Request_response(request, tmp_response);
      problem->Request_F(request, ans);
      eval_mngr().perform_evaluation(request);
      DEBUGPR(10, ucout << "Trial Pt Value: " << ans << std::endl;
              ucout << (100000.0*ans) << " " << (100000.0* (_best_val)) 
                    <<" " << (_best_val - ans) <<  std::endl;
	      ucout << (ans < _best_val) << " " << (_best_val > ans) 
              << std::endl;
              );
      if (ans < (_best_val-rho)) {
         _best_val = ans;
	 _best_cval = cval;
	 response = tmp_response;
	 x_ << tvec;
	 }
      else {
	 FA.ctr = ntrials - 2;
	 }
      }
   FA.rho = FA.rho_prev = 1.0;
   }

DEBUGPR(3,
	ucout << "## Fast Pattern Search State" << std::endl;
	ucout << "## rho_prev " << FA.rho_prev << std::endl;
	ucout << "## rho      " << FA.rho << std::endl;
	ucout << "## ctr      " << FA.ctr << std::endl;
	ucout << "## succ     " << FA.succ_flag << std::endl;
	ucout << "## e_ctr    " << FA.extended_ctr << std::endl;
	ucout << "## Basis:" << std::endl;
	switch (basis_id) {
	  case COORDINATE_BASIS:
			ucout << "##   coordinate offsets" << std::endl;
			break;
		default:
			{for (unsigned int j=0; j<ntrials; j++)
	  		   ucout << "##   " << pattern[j] << std::endl;}
			break;
		};
	);

if (FA.succ_flag == false) {
   //
   // Previous iteration was unsuccessful
   //
   bool any_feasible=false;
   for (unsigned i=0; i<fps_batch; i++) {
     DEBUGPR(10, ucout << "Current Point:       " << x_ << std::endl);
     DEBUGPR(3, ucout << "Current Trial Index: " << ndx[FA.ctr] << std::endl);
     DEBUGPR(3, ucout << "Shuffled Indeces:    " << ndx << std::endl);
     DEBUGPR(10, ucout << "Trial step : " << FA.ctr << " Len : " 
             << (FA.rho*Delta) << std::endl);
     unsigned tmp = (FA.ctr < i ? ntrials+(FA.ctr-i) : FA.ctr-i);
     generate_trial(tmp, x_, tvec, Delta, feasible, bias);
     if (feasible) {
        any_feasible=true;
        colin::AppRequest request = problem->set_domain(tvec);
        problem->Request_F(request);
        queue_order[ eval_mngr().queue_evaluation(request) ] = i;
        }
     }
   int h=-1;
   //
   // Only execute evaluations if we had at least _one_ feasible point
   //
   if (any_feasible) {
      response_t tmp_response;
      EvaluationID id = eval_mngr().find_first_improving(tmp_response, _best_val-rho);
      if (!tmp_response.empty()) {
	 h = queue_order[id];
         response = tmp_response;
         response.get(colin::f_info, _best_val);
         response.get_domain(x_);
         }
      }
   DEBUGPR(10, ucout << "After UnSuccessful: h= " << h << std::endl);
   queue_order.clear();
   if (h != -1) {
      eval_mngr().clear_evaluations();
      FA.succ_flag = true;
      FA.ctr = (FA.ctr < (unsigned int)h ? ntrials+(FA.ctr-h) : FA.ctr-h);
      reflect_pattern(ndx[FA.ctr]);
      update_step_order(FA.ctr);
      FA.rho_prev = 1.0;
      FA.rho = ex_factor;
      }
   else {
      if (trunc_fps) {
	 FA.succ_flag = 2;
	 update_step_order(FA.ctr);
         FA.rho = FA.rho_prev = 1.0;
	 }
      else {
	 FA.ctr = (FA.ctr == 0 ? ntrials-1 : FA.ctr-1);
	 FA.succ_flag = false;
	 }
      }
   }

else {
   //
   // Previous iteration was successful
   //
   DEBUGPR(3, ucout << "Shuffled Indeces:    " << ndx << std::endl);
   bool any_feasible=false;
   for (i=0; i<ntrials-1; i++) {
     DEBUGPR(10, ucout << "Current Point: " << x_ << std::endl);
     DEBUGPR(10, ucout << "Trial step : " << i << " Len : " 
             << (FA.rho*Delta) << " Rho: " << FA.rho << std::endl);
     generate_trial(i, x_, tvec, (FA.rho*Delta), feasible, bias);
     if (feasible) {
       any_feasible=true;
       colin::AppRequest request = problem->set_domain(tvec);
       problem->Request_F(request);
       queue_order[ eval_mngr().queue_evaluation(request) ] = i;
       }
   }
   int h=-1;
   //
   // Only execute evaluations if we had at least _one_ feasible point
   //
   if (any_feasible) {
      response_t tmp_response;
      EvaluationID id = eval_mngr().find_first_improving(tmp_response, _best_val-rho);
      if (!tmp_response.empty()) {
	 h = queue_order[id];
         response = tmp_response;
         response.get(colin::f_info, _best_val);
         response.get_domain(x_);
         }
      }
   DEBUGPR(10, ucout << "After Successful: h= " << h << std::endl);
   queue_order.clear();
   if (h == -1) {
      FA.succ_flag = false;
      FA.ctr = ntrials-2;
      update_step_order(ntrials-1);
      }
   else {
      eval_mngr().clear_evaluations();
      FA.succ_flag = true;
      FA.rho_prev = FA.rho;
      FA.rho = ex_factor;
      reflect_pattern(ndx[h]);
      update_step_order(h);
      }
   }

#ifdef DEBUG_FPS
if (FA.succ_flag == true) {
   if (scolib::fps_zeros > 0) {
      if (scolib::fps_zero_ctr.size() <= scolib::fps_zeros)
         scolib::fps_zero_ctr.resize(scolib::fps_zeros+10);
      scolib::fps_zero_ctr[scolib::fps_zeros]++;
      }
   scolib::fps_zeros=0;
   }
else {
   scolib::fps_zeros++;
   }
#endif

return (_best_val < (init_value-rho));
}
//==============================================================
//  %%%%     EM_test  added 6/04 pls
//==============================================================
//   We will have to add the priority and so on to the obj so
// the BatchEval will be able to see it as well as the pattern
// search. 
bool PatternSearch::EM_test(DoubleVector& x_, response_t& response,
					real& _best_val, real& _best_cval)
{
  //real init_value = _best_val;
  unsigned int i;
  int hipri = 1;
  int lopri = 2;
  
  // we need to check for feasibility of x.

  if (ndx.size() != ntrials){
    ndx.resize(ntrials);
  }
  update_step_order();
  if(ntrials != 2 * nreq_trials){    
    EXCEPTION_MNGR(runtime_error,
       "PatternSearch::EM_test-- problem with number of directions.\n");
  }
  ucout<< "ntrials == "<<ntrials<<",and size of ndx =  "<<ndx.size()<<std::endl;
  bool any_feasible=false;
  for (i = 0; i < nreq_trials; i++) {
    bool feasible;
    // We could do it in generate_trial.  If i >= ntrials and
    //  some flag is set, then x_i is just the double or triple
    //  or whatever of its associated direction.  We do two 
    //  loops, with the q_eval call in the second loop having
    //  lopri in the priority spot and true in the speculative
    //  spot.  BatchEval should take care of downgrading the 
    //  priorities when necessary.
    //
    //   We need to add the priority here.  We also need to
    //     add some speculative work.  So we will need a pattern
    //     with twice as many points as the one in the simple case.  
    //     The second half of the pattern will have steps
    //     two or three times as long as the ones in the
    //     ones in the basic compass pattern.
    
    //  queue_evaluation(tvec,PENALTY_RATIO);

    // Generate a core direction
    // cerr<<"________generating trial..."<<std::endl;
    generate_trial(i, x_, tvec, Delta, feasible, bias);
    // if(!feasible)cerr<<"Infeas in test too!!"<<std::endl;
    if (feasible){      
      any_feasible=true;
      ucout<<"________q-ing point...."<< tvec<<std::endl;
      colin::AppRequest request = problem->set_domain(tvec);
      problem->Request_F(request);
      eval_mngr().queue_evaluation(request, hipri);
    }
    // Generate its associated speculative direction.

    //  FIXME:::  Don't forget to modify the generate_trial method
    //    to do this properly.  It will have to check to see that
    //    the index is greater than ntrials.
    //  ALSO----
    //    Be sure ntrials is indeed the number of core directions.  Do
    //  we need to put another field into the class for this,
    //  just to be sure?

    // Should all this be inside the brackets for the principal
    //  point?
    generate_trial(i+nreq_trials, x_, tvec, Delta, feasible, bias);
    if(feasible){
      any_feasible=true;
      ucout<<"________q-ing spec point...."<< tvec<<std::endl;
      colin::AppRequest request = problem->set_domain(tvec);
      problem->Request_F(request);
      eval_mngr().queue_evaluation(request, lopri);
    }
  }

  //
  // Execute evaluations is _some_ trial point was feasible.
  //
  #if COLIN
  if (any_feasible)
     return execute_evaluations(response, _best_val, _best_cval, x_, rho);
  #endif
  //
  // Otherwise, return false
  //
  return false;
}


void PatternSearch::write(std::ostream& os) const
{
colin::ColinSolver<BasicArray<double>,colin::UNLP0_problem> :: write(os);

os << "##\n## Pattern Search Controls\n##\n";
os << "update_id\t";
if (update_id == PATTERN_SEARCH_DEFAULT)
   os << "default\t\t# Expand after max_succ successes." << std::endl;
else if (update_id == SINGLE_EXPAND_UPDATE)
   os << "single_expand\t# Expand after max_succ successes.\n\t\t\t\t# No expanding after 1st contraction." << std::endl;

os << "max_success\t" << max_success << std::endl;
os << "Delta_init\t" << Delta_init << std::endl;
os << "Delta_thresh\t" << Delta_thresh << std::endl;
os << "Contraction Factor\t" << ct_factor << std::endl;
os << "Expansion Factor\t" << ex_factor << std::endl;

os << "em_case\t\t";
switch (em_case) {

	case EM_MULTISTEP:
		os << "multi_step";
		os << "\t# Check for improvement in each search direction" << std::endl;
		os << "\t\t\t\t# iteratively.  Adapt the pattern to keep improvements" << std::endl;
		os << "\t\t\t\t# when detected and continue looking for improvement in" << std::endl;
		os << "\t\t\t\t# in the remaining search directions." << std::endl;
		break;

	case EM_SIMPLE:
		os << "simple";
		os << "\t# Use a simple pattern that is not adapted." << std::endl;
		break;

	case EM_ADAPTIVE:
		os << "adaptive";
		os << "\t# Adaptively modifies the search pattern to minimize the" << std::endl;
		os << "\t\t\t\t# number of function evaluations required." << std::endl;
		break;

	case EM_TEST:
		os << "test" << std::endl;
		break;

	default:
		break;
	}

os << "basis_str\t\t";
if ((basis_str == "coordinate") || (basis_str == "coordinate"))
   os << "coordinate\t# Basis of coordinate offsets" << std::endl;
else if (basis_str == "simplex")
   os << "simplex\t\t# Offsets defining a tetrahedron simplex" << std::endl;
else if (basis_str == "rotated_coordinate")
   os << "rotated\t\t# Basis of rotated coordinate offsets" << std::endl;

os << "step_selection\t" << step_selection_str << std::endl;
/// os << "reverse_flag\t" << reverse_flag << std::endl;
os << "num_augmented_trials\t" << num_augmented_trials << std::endl;
DEBUGPR(2,os << "Sigma\t" << Sigma << "\n");
}

void PatternSearch::virt_debug_io(std::ostream& os, const bool finishing, 
				const int io_level)
{
if (io_level == 3) {
   os << std::endl;
   os << "\tScale factor = " << Delta << " Sigma[i]" << std::endl;
   os << "\tSigma = " << Sigma << std::endl;

   if (curr_iter == 0) {
      os << std::endl;
      os << "\tContraction factor: " << ct_factor << std::endl;
      os << "\tExpansion factor:   " << ex_factor << std::endl;
      if (update_id == SINGLE_EXPAND_UPDATE) {
         os << "\tExpand after " << max_success << " successes." << std::endl;
         os << "\tNo expanding after 1st contraction." << std::endl;
         }
      else if (update_id == PATTERN_SEARCH_DEFAULT) {
         os << "\tExpand after " << max_success << " successes." << std::endl;
         }
      }
   else {
      os << std::endl;
      if (update_flag == 0)
         os << "\tFound an improving step. Step length not expanded." << std::endl;
      else if (update_flag == 1)
         os << "\tFound an improving step. Step length expanded." << std::endl;
      else if (update_flag == 2)
         os << "\tNo improving steps found. Step length contracted." << std::endl;
      else
         os << "\tNo improving steps found, but delaying step length contraction." << std::endl;
      }
   }

if (debug && (io_level == -1)) {
   os << "[Delta:\t" << Delta << "]\n";
   os << "[Steps:\t";
   for (unsigned int i=0; i<Sigma.size(); i++)
     os << Sigma[i]*Delta << " ";
   os << "]\n";
   }

if (io_level < 3)
   return;

if (curr_iter > 0) {
   os << std::endl;
   os << "\tGenerated " << ntrial_points << " trial points (" << nfeasible_points << " were feasible)" << std::endl;
   return;
   }

os << std::endl;
switch (em_case) {

	case EM_MULTISTEP:
		os << "\tUsing the \"multi_step\" search strategy:" << std::endl;
		os << "\t\tCheck for improvement in each search direction" << std::endl;
		os << "\t\titeratively.  Adapt the pattern to keep improvements" << std::endl;
		os << "\t\twhen detected and continue looking for improvement in" << std::endl;
		os << "\t\t in the remaining search directions." << std::endl;
		break;

	case EM_SIMPLE:
		os << "\tUsing the \"simple\" search strategy:" << std::endl;
		os << "\t\tUse a simple pattern that is not adapted." << std::endl;
		break;

	case EM_ADAPTIVE:
		os << "\tUsing the \"adaptive\" search strategy:" << std::endl;
		os << "\t\tAdaptively modifies the search pattern to minimize the" << std::endl;
		os << "\t\tnumber of function evaluations required." << std::endl;
		break;

	case EM_TEST:
		os << "\tUsing the \"test\" search strategy:" << std::endl;
		break;

	default:
		break;
	}

os << std::endl;
if ((basis_str == "coordinate") || (basis_str == "coordinate_test"))
   os << "\tBasis of coordinate offsets" << std::endl;
else if (basis_str == "simplex")
   os << "\tBasis of offsets defining a tetrahedron simplex" << std::endl;
else if (basis_str == "rotated_coordinate")
   os << "\tBasis of rotated coordinate offsets" << std::endl;
if (num_augmented_trials)
   os << "\tBasis augmented with " << num_augmented_trials << " additional trial point " << std::endl;
#if 0
if (reverse_flag && (em_case == EM_SIMPLE))
   os << "\tAdding reversed trial points (v and -v) to the set of trial points" << std::endl;
#endif
os << "\tTotal number of trial points considered is " << ntrials << std::endl;

os << std::endl;
if (step_selection_id == STEP_SELECTION_RANDOM)
   os << "\tPatterns examined in a random order." << std::endl;
else if (step_selection_id == STEP_SELECTION_BIASED)
   os << "\tPatterns examined in a biased order." << std::endl;
else
   os << "\tPatterns examined in a fixed order." << std::endl;
}

// Do not try this with the prioritized coord. basis at
//   this point.  
void PatternSearch::reflect_pattern(const unsigned int id)
{
if (ntrials == 0)
   return;

switch (basis_id) {

     case ROTATED_COORDINATE_BASIS:
     case COORDINATE_BASIS:
	//
	// This can be accomplished by swapping two elements in the array:
	// id and it's complement.
	//
	// Note: this code does not permit the use of augmented trials!
	//
	{
	unsigned int nvars = problem->num_real_vars;
	unsigned int alt = (id < nvars ? id+nvars : id-nvars);

	for (unsigned int j=0; j<ntrials; j++) {
	  if (ndx[j] == alt) ndx[j] = id;
	  else if (ndx[j] == id) ndx[j] = alt;
	  }
	}
	break;

     default:
	{
	for (unsigned int j=0; j<ntrials; j++)
	  if (j != id) {
	     double tmp = 2.0 * inner_product(pattern[j],pattern[id]) /
				inner_product(pattern[id],pattern[id]);
	     for (unsigned int i=0; i<pattern[j].size(); i++)
	       pattern[j][i] = pattern[j][i] - tmp * pattern[id][i];
	     }
        pattern[id] *= -1;
   	}
	break;

     };
}


// Defines scolib::StaticInitializers::PatternSearch_bool
REGISTER_COLIN_SOLVER_WITH_ALIAS( PatternSearch,"sco:PatternSearch","sco:ps", 
                                  "The SCO PatternSearch optimizer" )

} // namespace scolib
