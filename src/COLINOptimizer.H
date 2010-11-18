/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       COLINOptimizer
//- Description: Wrapper class for COLIN
//- Owner:       Jean-Paul Watson/Bill Hart
//- Checked by:
//- Version: $Id

#ifndef COLIN_OPTIMIZER_H
#define COLIN_OPTIMIZER_H

#include "data_types.h"
#include <string>
#include <coliny/coliny.h>
#include "DakotaOptimizer.H"
#include "COLINApplication.H"
#include "ProblemDescDB.H"
#include "ParamResponsePair.H"
#include "PRPMultiIndex.H"
#ifdef DAKOTA_UTILIB
#include "utilib/seconds.h"
#endif // DAKOTA_UTILIB

namespace Dakota {


/// Wrapper class for optimizers defined using COLIN 

/** The COLINOptimizer class provides a templated wrapper for COLIN, a
    Sandia-developed C++ optimization interface library.  A variety of
    COLIN optimizers are defined in the COLINY optimization library,
    which contains the optimization components from the old SGOPT
    library. COLINY contains optimizers such as genetic algorithms,
    pattern search methods, and other nongradient-based
    techniques. COLINOptimizer uses a COLINApplication object to
    perform the function evaluations.

    The user input mappings are as follows: \c max_iterations, \c
    max_function_evaluations, \c convergence_tolerance, \c
    solution_target and \c max_cpu_time are mapped into COLIN's \c
    max_iters, \c max_neval, \c ftol, \c accuracy, and \c max_time
    data attributes.  An \c output setting of \c verbose is passed to
    COLIN's set_output() function and a setting of \c debug activates
    output of method initialization and sets the COLIN \c debug
    attribute to 10000. Refer to [Hart, W.E., 2006] for additional
    information on COLIN objects and controls. */

template <class OptimizerT>
class COLINOptimizer : public Optimizer
{
public:

  /// constructor
  COLINOptimizer(Model& model);
  /// alternate constructor for on-the-fly instantiations
  COLINOptimizer(Model& model, int seed);
  /// alternate constructor for Iterator instantiations by name
  COLINOptimizer(NoDBBaseConstructor, Model& model);
  /// destructor
  ~COLINOptimizer() {
    if (rng) delete rng;
    //if (application) delete application;
    if (optimizer) delete optimizer;
  }

  /// Performs the iterations to determine the optimal solution.
  void find_optimum();

  // COLINY methods cannot yet accept multiple initial points
  //bool accepts_multiple_points() const;
  /// COLINY methods can return multiple points
  bool returns_multiple_points() const;

protected:

  //
  //- Heading: Member functions
  //

  /// sets up the random number generator for stochastic methods
  virtual void set_rng(int seed);

  /// sets the iteration starting point prior to minimization
  virtual void set_initial_point(ColinPoint& pt)
  { optimizer->set_initial_point(pt.rvec); }

  /// retrieves the final solution after minimization
  virtual void get_min_point(ColinPoint& pt)
  { colin::map_domain(pt,optimizer->opt_response().point); }

  /// sets options for specific methods based on user specifications
  /// (called at construction time)
  virtual void set_method_parameters()
  { set_standard_method_parameters(); }

  /// sets the standard method parameters shared by all methods
  void set_standard_method_parameters();

  /// sets method parameters for specific methods using data that is
  /// not available until run time
  virtual void set_runtime_parameters()
  { }

  /// Get the set of best points from the solver
  virtual void get_final_points();

  /// resize bestVariablesArray
  void resize_final_points(size_t newsize);

  //
  //- Heading: Data
  //

  /// Pointer to COLIN base optimizer object
  OptimizerT* optimizer;

  /// Pointer to the COLINApplication object
  COLINApplication* application;

  /// the COLIN problem object
  colin::OptProblem<ColinPoint> problem;

  /// RNG ptr
  utilib::RNG* rng;
  
  /// the \c synchronization setting: true if \c blocking, false if
  /// \c nonblocking
  bool blockingSynch;

#ifdef DAKOTA_UTILIB
  /// Start time for keeping track of time for solver to run
  Real solverStartTime;

  /// Time taken by solver to run
  Real solverTime;
#endif // DAKOTA_UTILIB
};


///--------------------------------------------------------------------------
/// Section 2
///--------------------------------------------------------------------------

template <class OptimizerT>
inline COLINOptimizer<OptimizerT>::COLINOptimizer(Model& model):
  Optimizer(model)
{
  // (iteratedModel initialized in Optimizer(Model&))
  application = new COLINApplication(iteratedModel);
  problem.set_application(application);

  optimizer = new OptimizerT();
  optimizer->set_problem(problem);

  set_rng(probDescDB.get_int("method.random_seed"));
  set_method_parameters(); // set specification values using DB

  // The following is not performed in the Optimizer constructor since
  // maxConcurrency is updated within set_method_parameters().  The
  // matching free_communicators() appears in the Optimizer destructor.
  if (scaleFlag || multiObjFlag)
    iteratedModel.init_communicators(maxConcurrency);
}


template <class OptimizerT>
inline COLINOptimizer<OptimizerT>::COLINOptimizer(Model& model, int seed):
  Optimizer(NoDBBaseConstructor(), model), blockingSynch(false)
{
  // (iteratedModel initialized in Optimizer(Model&))
  application = new COLINApplication(iteratedModel);
  problem.set_application(application);

  optimizer = new OptimizerT();
  optimizer->set_problem(problem);

  set_rng(seed);
  set_method_parameters(); // DB is null: set inherited attributes and defaults
}


template <class OptimizerT>
inline COLINOptimizer<OptimizerT>::
COLINOptimizer(NoDBBaseConstructor, Model& model):
  Optimizer(NoDBBaseConstructor(), model), rng(NULL), blockingSynch(false)
{
  // (iteratedModel initialized in Optimizer(Model&))
  application = new COLINApplication(iteratedModel);
  problem.set_application(application);

  optimizer = new OptimizerT();
  optimizer->set_problem(problem);

  //set_rng(seed);
  set_method_parameters(); // DB is null: set inherited attributes and defaults
}


template <class OptimizerT>
inline void COLINOptimizer<OptimizerT>::set_rng(int seed)
{
  try {
    if (optimizer->parameter_exists("seed")) {
       //
       // Instantiate random number generator (RNG). Can either pass the RNG in
       // the constructor or use optimizer->set_rng(RNG) prior to 
       // interface->setup (NOTE: passing it to the SWOpt constructor has 
       // failed, so use set_rng).  LCG = a portable linear congruential 
       // generator (better than Unix RAND).
       //
       utilib::PM_LCG* prng = new utilib::PM_LCG(seed);
       if (seed) // default is zero if no spec.
         Cout << "\nSeed (user-specified) = " << seed << '\n';
       else
         Cout << "\nSeed (system-generated) = " << prng->get_seed() <<'\n';
       optimizer->set_rng(prng);
       optimizer->set_parameter("seed", seed);
       rng = prng;
    }
  else 
    rng=0;
  }
  catch(const std::runtime_error &exception) {
    Cerr << "***COLINY run-time exception*** " << exception.what() << std::endl;
  }
}


/** find_optimum redefines the Optimizer virtual function to perform
    the optimization using COLIN. It first sets up the problem data,
    then executes minimize() on the COLIN optimizer, and finally
    catalogues the results. */
template <class OptimizerT>
inline void COLINOptimizer<OptimizerT>::find_optimum()
{
  try {
    set_runtime_parameters();

    // Initialize variables and bounds.  This is performed in find_optimum
    // in order to capture any reassignment at the strategy layer (after
    // iterator construction).  
    ColinPoint point;
    colin::map_domain(point,iteratedModel);
    set_initial_point(point);

    // For COLIN 2.0, discrete set int/real are handled as int ranges
    const IntSetArray& ddsiv_values
      = iteratedModel.discrete_design_set_int_values();
    const RealSetArray& ddsrv_values
      = iteratedModel.discrete_design_set_real_values();
    size_t i, num_ddv = numDiscreteIntVars + numDiscreteRealVars,
      num_ddsiv = ddsiv_values.size(), num_ddsrv = ddsrv_values.size(),
      num_ddrv  = numDiscreteIntVars - num_ddsiv;

    problem.init_real_params(numContinuousVars);
    problem.init_int_params(num_ddv);

    application->dakota_asynch_flag(asynchFlag);

    if (numContinuousVars > 0) {
      std::vector<colin::real> lower(numContinuousVars);
      std::vector<colin::real> upper(numContinuousVars);
      const RealVector& lower_bnds
	= iteratedModel.continuous_lower_bounds();
      const RealVector& upper_bnds
	= iteratedModel.continuous_upper_bounds();
      for (i=0; i<numContinuousVars; ++i) {
	lower[i] = lower_bnds[i];
	upper[i] = upper_bnds[i];
      }
      problem.set_real_bounds(lower,upper);
    }

    // For COLIN 2.0, use mapping from int/real set values to int ranges
    if (num_ddv > 0) {
      std::vector<int> lower(num_ddv);
      std::vector<int> upper(num_ddv);
      // integer range
      const IntVector& lower_bnds = iteratedModel.discrete_int_lower_bounds();
      const IntVector& upper_bnds = iteratedModel.discrete_int_upper_bounds();
      for (i=0; i<num_ddrv; ++i) {
	lower[i] = lower_bnds[i];
	upper[i] = upper_bnds[i];
      }
      // integer set: map to integer index sequence [0,num_items-1]
      size_t offset = num_ddrv;
      for (i=0; i<num_ddsiv; ++i) {
	lower[i+offset] = 0;
	upper[i+offset] = ddsiv_values[i].size() - 1;
      }
      // real set: map to integer index sequence [0,num_items-1]
      offset += num_ddsiv;
      for (i=0; i<num_ddsrv; ++i) {
	lower[i+offset] = 0;
	upper[i+offset] = ddsrv_values[i].size() - 1;
      }
      problem.set_int_bounds(lower,upper);
    }

    if (problem.numConstraints() > 0) {
      std::vector<colin::real> clower(problem.numConstraints());
      std::vector<colin::real> cupper(problem.numConstraints());
      const RealVector& nln_ineq_lwr_bnds
	= iteratedModel.nonlinear_ineq_constraint_lower_bounds();
      const RealVector& nln_ineq_upr_bnds
	= iteratedModel.nonlinear_ineq_constraint_upper_bounds();
      const RealVector& nln_eq_targets
	= iteratedModel.nonlinear_eq_constraint_targets();
      int ndx=0;
      for (i=0; i<numNonlinearIneqConstraints; ++i) {
	clower[ndx]   = nln_ineq_lwr_bnds[i];
	cupper[ndx++] = nln_ineq_upr_bnds[i];
      }
      for (i=0; i<numNonlinearEqConstraints; ++i) {
	clower[ndx]   = nln_eq_targets[i];
	cupper[ndx++] = nln_eq_targets[i];
      }
      problem.set_constraint_bounds(clower,cupper);
    }

    problem.reset_neval(iteratedModel.evaluation_id());
    problem.reset_neval_request_id(iteratedModel.evaluation_id());
    optimizer->reset(); // only call once just before minimize
    if (outputLevel == DEBUG_OUTPUT) {
      optimizer->write_parameter_values(Cout);
      }
#ifdef DAKOTA_UTILIB
    solverStartTime = CPUSeconds();
    optimizer->minimize();
    solverTime = CPUSeconds() - solverStartTime;
    optimizer->print_stats(Cout);
    Cout << "Solver-Time: " << solverTime << std::endl;
#else
    optimizer->minimize();
    optimizer->print_stats(Cout);
#endif // DAKOTA_UTILIB


    // Get the best points from the solver
    if (numBest > 1) {

      // For multi-point returns, retrieve the best vars for use at
      // the strategy level.  This retrieval, including DB lookup is a
      // workaround until COLIN-3.x.  Empirical evidence indicates the
      // points are returned sorted first by L2 constraint violation,
      // then by objective value, but the API docs don't guarantee it.
      // For now, we presume this will put the single best point at
      // front.
      
      // populate bestVariablesArray
      get_final_points();

      // try to retrieve corresponding results from DB (will fail if Recast)
      extern PRPCache data_pairs; // global container
      const String& interface_id = iteratedModel.interface_id();
      ActiveSet search_set(numFunctions, numContinuousVars); // asv = 1's
   
      size_t num_points = bestVariablesArray.size();
      for (size_t point_index = 0; point_index < num_points; ++point_index) {

	Variables& best_vars = bestVariablesArray[point_index];
	Response& best_response = bestResponseArray[point_index];
	// array already sized; if not found, reset data in the response
	if (!lookup_by_val(data_pairs, interface_id, best_vars, search_set,
			   best_response))
	  best_response.reset();

      }

    }
    else {

      // For single-point returns, leverage fact that COLIN methods do
      // a func->set_vars(best_pt) at the end of minimize, so just
      // retrieve this data and copy it into the front of the best
      // arrays.

      get_min_point(point);
      colin::map_domain(bestVariablesArray.front(),
			iteratedModel.discrete_design_set_int_values(),
			iteratedModel.discrete_design_set_real_values(), point);

      // Retrieve the best responses for use at the strategy level.
      if (!multiObjFlag) { // else multi_objective_retrieve() is used in
	                   // Optimizer::post_run()
	RealVector best_fns(numFunctions);
	best_fns[0] = optimizer->opt_response().response.function_value();
	for (i=0; i<numNonlinearConstraints; ++i)
	  best_fns[i+1]
	    = optimizer->opt_response().response.constraint_values()[i];
	bestResponseArray.front().function_values(best_fns);
      }

    }

  }
  catch(const std::runtime_error &exception) {
    Cerr << "***COLINY run-time exception*** " << exception.what() << std::endl;
  }
}


// Returns multiple points:
// No:  coliny_apps, coliny_pattern_search, coliny_solis_wets, coliny_cobyla
// Yes: coliny_ea,   coliny_direct
template <class OptimizerT>
inline bool COLINOptimizer<OptimizerT>::returns_multiple_points() const
{ return false; }


template <>
inline bool COLINOptimizer<coliny::DIRECT>::returns_multiple_points() const
{ return true; }


template <>
inline bool COLINOptimizer<coliny::EAminlp>::returns_multiple_points() const
{ return true; }


template <class OptimizerT>
inline void COLINOptimizer<OptimizerT>::resize_final_points(size_t newsize)
{
  size_t curr_size = bestVariablesArray.size();
  if (newsize < curr_size) // if reduction in size, use the standard resize
    bestVariablesArray.resize(newsize);
  else if (newsize > curr_size) {
    // Otherwise, we have to do the iteration ourselves so that we make use
    // of the model's current variables for envelope-letter requirements.
    bestVariablesArray.reserve(newsize);
    for (size_t i=curr_size; i<newsize; ++i)
      bestVariablesArray.push_back(iteratedModel.current_variables().copy());
  }

  curr_size = bestResponseArray.size();
  if (newsize < curr_size) // if reduction in size, use the standard resize
    bestResponseArray.resize(newsize);
  else if (newsize > curr_size) {
    // Otherwise, we have to do the iteration ourselves so that we make use
    // of the model's current variables for envelope-letter requirements.
    bestResponseArray.reserve(newsize);
    for (size_t i=curr_size; i<newsize; ++i)
      bestResponseArray.push_back(iteratedModel.current_response().copy());
  }

}


/** set_standard_method_parameters propagates standard DAKOTA user
    input to the optimizer. */
template <class OptimizerT>
inline void COLINOptimizer<OptimizerT>::set_standard_method_parameters()
{
  try {

    //
    // Inherited attributes
    //
    optimizer->set_parameter("max_func_evaluations_this_trial",
			     maxFunctionEvals);
    optimizer->set_parameter("max_iterations", (unsigned int)maxIterations);
    optimizer->set_parameter("function_value_tolerance", convergenceTol);

    switch (outputLevel) {
    case DEBUG_OUTPUT:
      optimizer->debug=10000;
      break;
    case VERBOSE_OUTPUT:
      optimizer->set_parameter("output_level", "verbose");
      optimizer->debug=1;
      break;
    case NORMAL_OUTPUT:
      optimizer->debug=1;
      break;
    }

    //
    // COLINY specification attributes
    //
    if (probDescDB.is_null()) { // instantiate on-the-fly
      // rely on internal COLINY defaults for the most part, but set any
      // default overrides (including enforcement of DAKOTA defaults) here
    }
    else {
      const Real& solution_target
	= probDescDB.get_real("method.solution_target");
      if (solution_target!=-1.e+25)
	optimizer->set_parameter("accuracy", solution_target);

      //const Real& max_cpu_time
      //  = probDescDB.get_real("method.coliny.max_cpu_time");
      //if (max_cpu_time >= 0.0)
      //  optimizer->set_parameter("max_time", max_cpu_time);

      bool show_misc_options
	= probDescDB.get_bool("method.coliny.show_misc_options");
      if (show_misc_options){
	Cout << "---------------------------SOLVER OPTIONS"
	     << "---------------------------\n";
	optimizer->write_parameters(Cout, true);
	Cout << "---------------------------SOLVER OPTIONS"
	     << "---------------------------" << std::endl;
      }

      const StringArray& misc_options
	= probDescDB.get_dsa("method.coliny.misc_options");
      size_t num_mo = misc_options.size();
      for (size_t i=0; i<num_mo; ++i) {
	std::string thisOption(misc_options[i]);
	size_t equalPos = thisOption.find("=",0);
	if (equalPos == std::string::npos)
	  Cout << "Warning - COLINY ignoring option " << misc_options[i]
	       << std::endl;
	else {
	  std::string option(thisOption.substr(0, equalPos));
	  std::string value(thisOption.substr(equalPos+1, thisOption.size()-1));
	  optimizer->set_parameter_with_string(option, value);
	}
      }
    }
  }
  catch(const std::runtime_error &exception) {
    Cerr << "***COLINY run-time exception*** " << exception.what() << std::endl;
  }
}


///--------------------------------------------------------------------------
/// Section 3
///--------------------------------------------------------------------------

/** specialization of set_method_parameters() for DIRECT */
template <>
inline void COLINOptimizer<coliny::DIRECT>::set_method_parameters()
{
  try {

    //
    // COLINY specification attributes
    //
    if (probDescDB.is_null()) { // instantiate on-the-fly
      // rely on internal COLINY defaults for the most part, but set any
      // default overrides (including enforcement of DAKOTA defaults) here
      outputLevel = VERBOSE_OUTPUT; // for debugging for now
    }
    else {

      // returns "major_dimension", "all_dimensions", or empty string (default)
      const String& division_type
	= probDescDB.get_string("method.coliny.division");
      if (division_type == "major_dimension")
	optimizer->set_parameter("division", "single");
      else if (division_type == "all_dimensions")
	optimizer->set_parameter("division", "multi");
      else {
	// user didn't specify, so by default use multiple division in asynch
	// case, otherwise single division (a maximum of 2 asynch evals)
	if (asynchFlag)
	  optimizer->set_parameter("division", "multi");
	else
	optimizer->set_parameter("division", "single");
      }

      double global_bal_param
	= probDescDB.get_real("method.coliny.global_balance_parameter");
      if (global_bal_param >= 0.)
	optimizer->set_parameter("global_search_balance", global_bal_param);
      double local_bal_param
	= probDescDB.get_real("method.coliny.local_balance_parameter");
      if (local_bal_param >= 0.)
	optimizer->set_parameter("local_search_balance", local_bal_param);
      double max_box = probDescDB.get_real("method.coliny.max_boxsize_limit");
      if (max_box >= 0.)
	optimizer->set_parameter("max_boxsize_limit", max_box);
      double min_box = probDescDB.get_real("method.min_boxsize_limit");
      if (min_box >= 0.)
	optimizer->set_parameter("min_boxsize_limit", min_box);

      double constr_penalty = probDescDB.get_real("method.constraint_penalty");
      if (constr_penalty >= 0.)
	optimizer->set_parameter("constraint_penalty", constr_penalty);
    }

    //
    // THIS IS A BOGUS VALUE ... BUT IT ISN"T CLEAR WHAT THE _RIGHT_ VALUE IS.
    //
    maxConcurrency *= 2*numContinuousVars;

    set_standard_method_parameters();
  }
  catch(const std::runtime_error &exception) {
    Cerr << "***COLINY run-time exception*** " << exception.what() << std::endl;
  }
}


/** specialization of set_method_parameters() for Cobyla */
template <>
inline void COLINOptimizer<coliny::Cobyla>::set_method_parameters()
{
  try {

    //
    // COLINY specification attributes
    //
    if (probDescDB.is_null()) { // instantiate on-the-fly
      // rely on internal COLINY defaults for the most part, but set any
      // default overrides (including enforcement of DAKOTA defaults) here
    }
    else {
      const Real& init_delta 
	= probDescDB.get_real("method.coliny.initial_delta");
      if (init_delta >= 0.0)
	optimizer->set_parameter("initial_step", init_delta);
      const Real& thresh_delta
	= probDescDB.get_real("method.coliny.threshold_delta");
      if (thresh_delta >= 0.0)
	optimizer->set_parameter("step_tolerance", thresh_delta);
    }

    // COBYLA currently supports max function evals, and will eventually
    // support solution accuracy
    set_standard_method_parameters();
  }
  catch(const std::runtime_error &exception) {
    Cerr << "***COLINY run-time exception*** " << exception.what() << std::endl;
  }
}


#ifdef ACRO_APPS
/** specialization of set_method_parameters() for APPS */
template <>
inline void COLINOptimizer<coliny::APPS>::set_method_parameters()
{
  try {

    //
    // COLINY specification attributes
    //
    if (probDescDB.is_null()) { // instantiate on-the-fly
      // rely on internal COLINY defaults for the most part, but set any
      // default overrides (including enforcement of DAKOTA defaults) here
    }
    else {

      // A null string is the DB default and nonblocking is the APPS default, so
      // the flag is true only for an explicit blocking user specification.
      blockingSynch = (probDescDB.get_string("method.coliny.synchronization")
		       == "blocking") ? true : false;

      const Real& init_delta 
	= probDescDB.get_real("method.coliny.initial_delta");
      if (init_delta >= 0.0)
	optimizer->set_parameter("initial_step", init_delta);
      const Real& thresh_delta
	= probDescDB.get_real("method.coliny.threshold_delta");
      if (thresh_delta >= 0.0)
	optimizer->set_parameter("step_tolerance", thresh_delta);

      // optional parameters
      // Bug: APPS is not automatically updating min_step_allowed from the
      // step_tolerance specification, presumably since COLINY is interfering
      // (by already setting some value).
      optimizer->set_parameter("min_step_allowed", 2.*thresh_delta);

      // set "Synchronous" when DAKOTA is not in asynch mode.  This may allow
      // APPS to run more efficiently in serial mode.
      //if (!asynchFlag)
      //  optimizer->set_parameter("synchronous", true);

      // ----------------------------------------------------------------
      // Current APPS is hardwired for coordinate bases, no expansion,
      // and no pattern augmentation.
      //
      // See http://software.sandia.gov/appspack/pageParameters.html
      // for valid specification options.
      //
      // APPS wish list:
      //   basis control (only coordinate now supported due to simplified
      //     constraint management)
      //   some form of load balancing (total size preferred; Tammy says 
      //     that pattern augmentation can be done, but would not account 
      //     for cache management)
      // ----------------------------------------------------------------

      //bool expansion_flag = probDescDB.get_bool("method.coliny.expansion");
      const Real& contraction_factor
	= probDescDB.get_real("method.coliny.contraction_factor");
      //if (expansion_flag) {
      optimizer->set_parameter("contraction_factor", contraction_factor);
      //  optimizer->set_parameter("expansion_factor", 1.0/contraction_factor);
      //}
      //else { // turn expansion "off" by using expansion_factor = 1.0 
      //  optimizer->set_parameter("contraction_factor", contraction_factor);
      //  optimizer->set_parameter("expansion_factor", 1.0);
      //}

      //const String& pattern_basis
      //  = probDescDB.get_string("method.coliny.pattern_basis");
      //if (!pattern_basis.empty())
      //  optimizer->set_parameter("basis", (string)pattern_basis);

      //int total_pattern_size
      //  = probDescDB.get_int("method.coliny.total_pattern_size");
      // 'coordinate' is current COLINY default:
      //int basic_pattern_size = (pattern_basis == "simplex")
      //                       ? numContinuousVars + 1 : 2*numContinuousVars;
      //int num_augmented_trials = total_pattern_size - basic_pattern_size;
      // catch bad input or default total_pattern_size = 0
      //if (num_augmented_trials > 0) {
      //  optimizer->set_parameter("num_augmented_trials",num_augmented_trials);
      //  maxConcurrency *= total_pattern_size;
      //}
      //else
      //  maxConcurrency *= basic_pattern_size;

      const Real& constraint_penalty
	= probDescDB.get_real("method.constraint_penalty");
      if (constraint_penalty >= 0.0)
	optimizer->set_parameter("constraint_penalty", constraint_penalty);
    }

    if (blockingSynch)
      optimizer->set_parameter("batch_eval", "sync");
    else // nonblocking (default)
      optimizer->set_parameter("batch_eval", "async");
    // APPS does not support a "sequential" option
    application->blocking_synch(blockingSynch);

    maxConcurrency *= 2*numContinuousVars;

    switch (outputLevel) {
    case DEBUG_OUTPUT:
      optimizer->set_parameter("debug", 7); break;
    case VERBOSE_OUTPUT:
      optimizer->set_parameter("debug", 4); break;
    case NORMAL_OUTPUT:
      optimizer->set_parameter("debug", 3); break;
    case QUIET_OUTPUT:
      optimizer->set_parameter("debug", 2); break;
    case SILENT_OUTPUT:
      optimizer->set_parameter("debug", 1); break;
    }

    set_standard_method_parameters();
  }
  catch(const std::runtime_error &exception) {
    Cerr << "***COLINY run-time exception*** " << exception.what() << std::endl;
  }
}
#endif // ACRO_APPS


/** specialization of set_runtime_parameters() for PatternSearch */
template <>
inline void COLINOptimizer<coliny::PatternSearch>::set_runtime_parameters()
{
  try {

    // This parameter assignment must occur at runtime since evaluation
    // capacity is set within Model::init_communicators(), which is called
    // between iterator construction and execution.
    if (blockingSynch)
      optimizer->set_parameter("batch_eval", "all");
    else { // nonblocking (default)
      if (problem.num_evaluation_servers() > 1) // parallel PS
	optimizer->set_parameter("batch_eval", "async");
      else                                      // serial PS
	optimizer->set_parameter("batch_eval", "sequential");
    }

  }
  catch(const std::runtime_error &exception) {
    Cerr << "***COLINY run-time exception*** " << exception.what() << std::endl;
  }
}


/** specialization of set_method_parameters() for PatternSearch */
template <>
inline void COLINOptimizer<coliny::PatternSearch>::set_method_parameters()
{
  try {

    //
    // COLINY specification attributes
    //
    String pattern_basis;
    int total_pattern_size;
    if (probDescDB.is_null()) { // instantiate on-the-fly
      // rely on internal COLINY defaults for the most part, but set any
      // default overrides (including enforcement of DAKOTA defaults) here
      pattern_basis = "coordinate";
      total_pattern_size = 2*numContinuousVars;
    }
    else {

      // required parameters

      const Real& init_delta
	= probDescDB.get_real("method.coliny.initial_delta");
      if (init_delta >= 0.0)
	optimizer->set_parameter("initial_step", init_delta);
      const Real& thresh_delta
	= probDescDB.get_real("method.coliny.threshold_delta");
      if (thresh_delta >= 0.0)
	optimizer->set_parameter("step_tolerance", thresh_delta);

      // optional parameters

      // A null string is the DB default and nonblocking is the PS default, so
      // the flag is true only for an explicit blocking user specification.
      // mapped to "batch_eval" within set_runtime_parameters().
      blockingSynch = (probDescDB.get_string("method.coliny.synchronization")
		       == "blocking") ? true : false;

      bool expansion_flag = probDescDB.get_bool("method.coliny.expansion");
      // no need to check for case of -1.e+25 since the COLINY defaults are
      // replicated in DataMethod.C
      const Real& contraction_factor
	= probDescDB.get_real("method.coliny.contraction_factor");
      if (expansion_flag) {
	int expand_after_success
	  = probDescDB.get_int("method.coliny.expand_after_success");
	if (expand_after_success>=0)
	  optimizer->set_parameter("max_success", expand_after_success);
	optimizer->set_parameter("contraction_factor", contraction_factor);
	optimizer->set_parameter("expansion_factor", 1.0/contraction_factor);
      }
      else { // turn expansion "off" by using expansion_factor = 1.0 
	optimizer->set_parameter("contraction_factor", contraction_factor);
	optimizer->set_parameter("expansion_factor", 1.0);
      }

      const String& exploratory_moves
	= probDescDB.get_string("method.coliny.exploratory_moves");
      if (!exploratory_moves.empty())
	optimizer->set_parameter("exploratory_move",
	                         (std::string)exploratory_moves);

      bool coliny_randomize = probDescDB.get_bool("method.coliny.randomize");
      if (coliny_randomize)
	optimizer->set_parameter("step_selection", "random");
      else
	optimizer->set_parameter("step_selection", "fixed");

      pattern_basis = probDescDB.get_string("method.coliny.pattern_basis");
      total_pattern_size
	= probDescDB.get_int("method.coliny.total_pattern_size");
      // 'coordinate' is current COLINY default:
      int basic_pattern_size = (pattern_basis == "simplex")
	? numContinuousVars + 1 : 2*numContinuousVars;
      int num_augmented_trials = total_pattern_size - basic_pattern_size;

      // catch bad input or default total_pattern_size = 0
      if (num_augmented_trials > 0)
	optimizer->set_parameter("num_augmented_trials", num_augmented_trials);
      else // enforce lower bound of basic pattern
	total_pattern_size = basic_pattern_size;

      const Real& constraint_penalty
	= probDescDB.get_real("method.constraint_penalty");
      if (constraint_penalty >= 0.0)
	optimizer->set_parameter("constraint_penalty", constraint_penalty);

      bool constant_constraint_penalty
	= probDescDB.get_bool("method.coliny.constant_penalty");
      if (constant_constraint_penalty)
	optimizer->set_parameter("constant_constraint_penalty", true);
    }

    if (!pattern_basis.empty())
      optimizer->set_parameter("basis", (std::string)pattern_basis);
    maxConcurrency *= total_pattern_size;
    application->blocking_synch(blockingSynch);

    set_standard_method_parameters();
  }
  catch(const std::runtime_error &exception) {
    Cerr << "***COLINY run-time exception*** " << exception.what() << std::endl;
  }
}


/** specialization of set_method_parameters() for SolisWets */
template <>
inline void COLINOptimizer<coliny::SolisWets>::set_method_parameters()
{
  try {

    //
    // COLINY specification attributes
    //
    if (probDescDB.is_null()) { // instantiate on-the-fly
      // rely on internal COLINY defaults for the most part, but set any
      // default overrides (including enforcement of DAKOTA defaults) here
    }
    else {
      // init/thresh_delta are required inputs.  No need to check for defaults.
      const Real& init_delta 
	= probDescDB.get_real("method.coliny.initial_delta");
      if (init_delta >= 0.0)
	optimizer->set_parameter("initial_step", init_delta);
      const Real& thresh_delta
	= probDescDB.get_real("method.coliny.threshold_delta");
      if (thresh_delta >= 0.0)
	optimizer->set_parameter("step_tolerance", thresh_delta);
      
      // no need to check for case of -1.e+25 since the COLINY defaults are 
      // replicated in DataMethod.C
      const Real& contraction_factor
	= probDescDB.get_real("method.coliny.contraction_factor");
      optimizer->set_parameter("contraction_factor", contraction_factor);

      int contract_after_failure
	= probDescDB.get_int("method.coliny.contract_after_failure");
      if (contract_after_failure>=0)
	optimizer->set_parameter("max_failure", contract_after_failure);

      bool expansion_flag = probDescDB.get_bool("method.coliny.expansion");
      if (expansion_flag) {
	int expand_after_success
	  = probDescDB.get_int("method.coliny.expand_after_success");
	if (expand_after_success)
	  optimizer->set_parameter("max_success", expand_after_success);
	optimizer->set_parameter("expansion_factor", 1.0/contraction_factor);
      }
      else // turn expansion "off" by using expansion_factor = 1.0
	optimizer->set_parameter("expansion_factor", 1.0);
      
      const Real& constraint_penalty
	= probDescDB.get_real("method.constraint_penalty");
      if (constraint_penalty >= 0.0)
	optimizer->set_parameter("constraint_penalty", constraint_penalty);

      bool constant_constraint_penalty
	= probDescDB.get_bool("method.coliny.constant_penalty");
      if (constant_constraint_penalty)
	optimizer->set_parameter("constant_constraint_penalty", true);
    }

    set_standard_method_parameters();

    // no parallel SW -->> leave maxConcurrency at 1
  }
  catch(const std::runtime_error &exception) {
    Cerr << "***COLINY run-time exception*** " << exception.what() << std::endl;
  }
}


/** specialization of set_method_parameters() for EAminlp */
template <>
inline void COLINOptimizer<coliny::EAminlp>::set_method_parameters()
{
  try {

    //
    // COLINY specification attributes
    //
    unsigned int pop_size;
    if (probDescDB.is_null()) { // instantiate on-the-fly
      // rely on internal COLINY defaults for the most part, but set any
      // default overrides (including enforcement of DAKOTA defaults) here
      pop_size = 100;
    }
    else {
      pop_size = probDescDB.get_int("method.population_size");
      if (!pop_size)
	pop_size = 100;

      const String& pop_init_type =
	probDescDB.get_string("method.initialization_type");
      if (pop_init_type == "simple_random") 
	optimizer->set_parameter("population_unique", false);
      else if (pop_init_type == "unique_random")
	optimizer->set_parameter("population_unique", true);
      else if (pop_init_type == "flat_file") {
	const String& flat_file = probDescDB.get_string("method.flat_file");
	optimizer->set_parameter("init_filename", flat_file.c_str());
      }

      const String& selection_type =
	probDescDB.get_string("method.fitness_type");
      if (selection_type == "merit_function") 
	optimizer->set_parameter("selection_type","proportional");
      else if (selection_type == "linear_rank")
	optimizer->set_parameter("selection_type","linear_rank");

      const String& replacement_type =
	probDescDB.get_string("method.replacement_type");
      int keep_num = probDescDB.get_int("method.coliny.number_retained");
      int new_solutions = 
	probDescDB.get_int("method.coliny.new_solutions_generated");
      optimizer->set_parameter("replacement_method",replacement_type.c_str());
      if (keep_num >= 0) {
	unsigned int tmp = static_cast<unsigned int>(keep_num);
	optimizer->set_parameter("keep_num",tmp);
      }
      if (new_solutions >= 0) {
	unsigned int tmp = static_cast<unsigned int>(new_solutions);
	optimizer->set_parameter("num_trial_points",tmp);
      }

      double crossover_rate  = probDescDB.get_real("method.crossover_rate");
      optimizer->set_parameter("xover_rate",crossover_rate);

      const String& crossover_type =
	probDescDB.get_string("method.crossover_type");
      if (crossover_type == "blend") {
	optimizer->set_parameter("realarray_xover_type","blend");
	optimizer->set_parameter("intarray_xover_type","twopoint");
      } 
      else if (crossover_type == "uniform") {
	optimizer->set_parameter("realarray_xover_type","uniform");
	optimizer->set_parameter("intarray_xover_type","uniform");
      }
      else { // default crossover type = "two_point"
	optimizer->set_parameter("realarray_xover_type","twopoint");
	optimizer->set_parameter("intarray_xover_type","twopoint");
      }

      double mutation_rate  = 
	probDescDB.get_real("method.mutation_rate");
      optimizer->set_parameter("mutation_rate",mutation_rate);

      const String& mutation_type = 
	probDescDB.get_string("method.mutation_type");
      if (mutation_type == "replace_uniform" ||
	  mutation_type == "offset_normal" || mutation_type == "offset_cauchy")
	optimizer->set_parameter("realarray_mutation_type",
				 mutation_type.c_str());
      else // default mutation type = "offset_uniform"
	optimizer->set_parameter("realarray_mutation_type","offset_uniform");
      optimizer->set_parameter("intarray_mutation_type","uniform");

      double mutation_scale = probDescDB.get_real("method.mutation_scale");
      optimizer->set_parameter("realarray_mutation_scale",mutation_scale);

      int mutation_range = probDescDB.get_int("method.coliny.mutation_range");
      if (mutation_range >= 0) {
	unsigned int tmp = static_cast<unsigned int>(mutation_range);
	optimizer->set_parameter("intarray_mutation_range",tmp);
      }

      // WEH - this is only meaningful if we use allele mutation rates less
      // than one.
      //
      //double mutation_ratio = 
      //  probDescDB.get_real("method.coliny.mutation_ratio");
      //WEH, what does this map to? This is DMD's guess:
      //optimizer->set_parameter("mutation_ratio",mutation_ratio);

      bool mutation_adaptive = probDescDB.get_bool("method.mutation_adaptive");
      if (mutation_adaptive)
	optimizer->set_parameter("realarray_mutation_selfadaptation",true);
      else
	optimizer->set_parameter("realarray_mutation_selfadaptation",false);

      const Real& constraint_penalty
	= probDescDB.get_real("method.constraint_penalty");
      if (constraint_penalty >= 0.0)
	optimizer->set_parameter("constraint_penalty", constraint_penalty);
    }

    optimizer->set_parameter("population_size", pop_size);
    maxConcurrency *= pop_size;
    set_standard_method_parameters();
  }
  catch(const std::runtime_error &exception) {
    Cerr << "***COLINY run-time exception*** " << exception.what() << std::endl;
  }
}


// WEH - this code is a bit of a hack.  The problem is that Dakota doesn't
// know the type of the points used in COLIN optimizers.  However, we're
// being asked to return these points.  For now, I'm just going to use
// template specialization to resolve this.
template <class OptimizerT>
inline void COLINOptimizer<OptimizerT>::get_final_points()
{
  std::vector< utilib::BasicArray<double> > points;
  optimizer->get_final_points(points);
  resize_final_points(points.size());

  for (size_t i=0; i<points.size(); ++i)
    for (size_t j=0; j<numContinuousVars; ++j)
      bestVariablesArray[i].continuous_variable(points[i][j], j);
}

template <>
inline void COLINOptimizer<coliny::DIRECT>::get_final_points()
{
  std::vector< std::vector<double> > points;
  optimizer->get_final_points(points);
  resize_final_points(points.size());

  for (size_t i=0; i<points.size(); ++i)
    for (size_t j=0; j<numContinuousVars; ++j)
      bestVariablesArray[i].continuous_variable(points[i][j], j);
}

#ifdef ACRO_APPS
template <>
inline void COLINOptimizer<coliny::APPS>::get_final_points()
{
  std::vector< std::vector<double> > points;
  optimizer->get_final_points(points);
  resize_final_points(points.size());

  for (size_t i=0; i<points.size(); ++i)
    for (size_t j=0; j<numContinuousVars; ++j)
      bestVariablesArray[i].continuous_variable(points[i][j], j);
}
#endif // ACRO_APPS

template <>
inline void COLINOptimizer<coliny::EAminlp>::get_final_points()
{
  std::vector< utilib::MixedIntVars > points;
  optimizer->get_final_points(points);
  resize_final_points(points.size());

  const IntSetArray& ddsiv_values
    = iteratedModel.discrete_design_set_int_values();
  const RealSetArray& ddsrv_values
    = iteratedModel.discrete_design_set_real_values();
  size_t  num_ddsiv = ddsiv_values.size(), num_ddsrv = ddsrv_values.size(),
    num_ddrv  = numDiscreteIntVars - num_ddsiv, offset;

  for (size_t i=0; i<points.size(); ++i) {

    // continuous variables
    for (size_t j=0; j<numContinuousVars; ++j)
      bestVariablesArray[i].continuous_variable(points[i].Real()[j], j);

    // range of int variables; direct mappng of value
    for (size_t j=0; j<num_ddrv; ++j)
      bestVariablesArray[i].discrete_int_variable(points[i].Integer()[j], j);

    // set of integer variables; map from index back to integer value
    offset = num_ddrv;
    for (size_t j=0; j<num_ddsiv; ++j) {
      int colin_index = points[i].Integer()[offset + j];
      int dakota_value = 
	Dakota::set_index_to_value(colin_index, ddsiv_values[j]);
      bestVariablesArray[i].discrete_int_variable(dakota_value, offset+j);
    }

    // set of real variables; map from index back to real value
    offset += num_ddsiv;
    for (size_t j=0; j<num_ddsrv; ++j) {
      int colin_index = points[i].Integer()[offset + j];
      double dakota_value = 
	Dakota::set_index_to_value(colin_index, ddsrv_values[j]);
      bestVariablesArray[i].discrete_real_variable(dakota_value, j);
    }

  }

}

} // namespace Dakota

#endif
