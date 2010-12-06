/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       COLINOptimizer
//- Description: Implementation of the COLINOptimizer class, including type 
//-              manager registrations for conversion
//- Owner:       Patty Hough/John Siirola/Brian Adams
//- Checked by:
//- Version: $Id$

#include "COLINApplication.H"
#include "COLINOptimizer.H"
#include "ProblemDescDB.H"
#include "ParamResponsePair.H"
#include "PRPMultiIndex.H"
using Dakota::RealVector;
using Dakota::IntVector;

#include <vector>
using std::vector;
#include <list>
using std::list;

// for type management and conversion system
#include <utilib/TypeManager.h>
using utilib::TypeManager;
#include <utilib/Any.h>
using utilib::Any;
#include <utilib/MixedIntVars.h>
using utilib::MixedIntVars;
#include <utilib/PropertyDict_YamlPrinter.h>

// needed for correct registration of linked solvers
#include <interfaces/InterfacesRegistrations.h>
#include <scolib/SCORegistrations.h>

// for COLINOptimizer implementation
#include <colin/ApplicationMngr.h>
using colin::ApplicationHandle;
#include <colin/reformulation/ConstraintPenalty.h>
using colin::ConstraintPenaltyApplication;
#include <colin/SolverMngr.h>
using colin::SolverMngr;
#include <colin/PointSet.h>
using colin::PointSet;
#include <utilib/PM_LCG.h>
using utilib::PM_LCG;

using std::endl;
using std::string;
using std::runtime_error;

//
// - COLINOptimizer implementation
//

namespace Dakota {

//
// - TypeManager registrations
//

namespace {

int cast_from_realvector_to_vector(const Any& src, Any& dest)
{
  const RealVector& tmp = src.expose<RealVector>();
  vector<double>& ans = dest.set<vector<double> >();
  ans.reserve(tmp.length());
  for (size_t i=0; i<tmp.length(); i++) {
    ans.push_back(tmp[i]);
  }
  return 0;
}

int cast_from_vector_to_realvector(const Any& src, Any& dest)
{ 
  const vector<double>& tmp = src.expose<vector<double> >();
  RealVector& ans = dest.set<RealVector>();
  ans.resize(tmp.size());
  for (size_t i=0; i<tmp.size(); i++) {
    ans[i] = tmp[i];
  }
  return 0;
}

int cast_from_intvector_to_vector(const Any& src, Any& dest)
{
  const IntVector& tmp = src.expose<IntVector>();
  vector<int>& ans = dest.set<vector<int> >();
  ans.reserve(tmp.length());
  for (size_t i=0; i<tmp.length(); i++) {
    ans.push_back(tmp[i]);
  }
  return 0;
}

int cast_from_vector_to_intvector(const Any& src, Any& dest)
{ 
  const vector<int>& tmp = src.expose<vector<int> >();
  IntVector& ans = dest.set<IntVector>();
  ans.resize(tmp.size());
  for (size_t i=0; i<tmp.size(); i++) {
    ans[i] = tmp[i];
  }
  return 0;
}

int cast_from_int_to_unsignedint(const Any& src, Any& dest)
{ 
  const int& tmp = src.expose<int>();
  unsigned int& ans = dest.set<unsigned int>();
  ans = (unsigned int)tmp;
  return static_cast<int>(ans) == tmp 
     ? 0 : utilib::Type_Manager::CastWarning::ValueOutOfRange;
}

int cast_from_unsignedint_to_int(const Any& src, Any& dest)
{ 
  const unsigned int& tmp = src.expose<unsigned int>();
  int& ans = dest.set<int>();
  ans = tmp;
  return static_cast<unsigned int>(ans) == tmp 
     ? 0 : utilib::Type_Manager::CastWarning::ValueOutOfRange;
}

int cast_from_charconst_to_string(const Any& src, Any& dest)
{ 
  char const* tmp = src.expose<char const*>();
  string& ans = dest.set<string>();
  ans = tmp;
  return 0;
}

bool register_dakota_cast(){

  TypeManager()->register_lexical_cast
    ( typeid(RealVector), typeid(vector<double>), 
      &cast_from_realvector_to_vector);
  TypeManager()->register_lexical_cast
    ( typeid(vector<double>), typeid(RealVector), 
      &cast_from_vector_to_realvector);
  TypeManager()->register_lexical_cast
    ( typeid(IntVector), typeid(vector<int>), 
      &cast_from_intvector_to_vector);
  TypeManager()->register_lexical_cast
    ( typeid(vector<int>), typeid(IntVector), 
      &cast_from_vector_to_intvector);
  TypeManager()->register_lexical_cast
    ( typeid(int), typeid(unsigned int), 
      &cast_from_int_to_unsignedint);
  TypeManager()->register_lexical_cast
    ( typeid(unsigned int), typeid(int), 
      &cast_from_unsignedint_to_int);
  TypeManager()->register_lexical_cast
    ( typeid(char const*), typeid(string), 
      &cast_from_charconst_to_string);
  return(true);
}

const volatile bool dakota_cast_registered = register_dakota_cast();

} // private namespace

// used to enumerate the solverType
enum { COBYLA, DIRECT, EA, MS, PS, SW };

COLINOptimizer::COLINOptimizer(Model& model):
  Optimizer(model)
{
  // (iteratedModel initialized in Optimizer(Model&))
  solver_setup(model);
  set_rng(probDescDB.get_int("method.random_seed"));
  set_solver_parameters(); // set specification values using DB

  // The following is not performed in the Optimizer constructor since
  // maxConcurrency is updated within set_method_parameters().  The
  // matching free_communicators() appears in the Optimizer destructor.
  if (scaleFlag || multiObjFlag)
    iteratedModel.init_communicators(maxConcurrency);
 
}

COLINOptimizer::COLINOptimizer(Model& model, int seed):
  Optimizer(NoDBBaseConstructor(), model), blockingSynch(false)
{
  solver_setup(model);
  set_rng(seed);
  set_solver_parameters(); // DB is null: set inherited attributes and defaults
}

COLINOptimizer::COLINOptimizer(NoDBBaseConstructor, Model& model):
  Optimizer(NoDBBaseConstructor(), model), rng(NULL), blockingSynch(false)
{
  solver_setup(model);
  set_solver_parameters(); // DB is null: set inherited attributes and defaults
}

/** find_optimum redefines the Optimizer virtual function to perform
    the optimization using COLIN. It first sets up the problem data,
    then executes optimize() on the COLIN solver and finally
    catalogues the results. */
void COLINOptimizer::find_optimum()
{
  // Much of this is performed in find_optimum in order to capture any
  // reassignment at the strategy layer (after iterator construction).
  try {
     colinEvalMgr = colin::EvalManagerFactory()
        .create(iteratedModel.asynch_flag() ? "Concurrent" : "Serial");
     if (colinEvalMgr->has_property("max_concurrency"))
       colinEvalMgr->property("max_concurrency") = iteratedModel.evaluation_capacity();
     colinSolver->set_evaluation_manager(colinEvalMgr);
     colinProblem.second->set_evaluation_manager(colinEvalMgr);

    // Initialize the COLINApplication with the appropriate model
    // this will initialize functions, constraints, and bounds
    colinProblem.second->set_problem(iteratedModel);
    colinSolver->set_problem(colinProblem.first);

    // JDS: We need to wait to set the constraint penalty until after
    // the problem is handed to the solver and all reformulations have
    // taken place.
    colin::ApplicationHandle problem = colinSolver->get_problem_handle();
    if ( constraint_penalty >= 0. )
      problem->property("constraint_penalty") = constraint_penalty;
    if ( problem->has_property("apply_convergence_factor") )
      problem->property("apply_convergence_factor") = !constant_penalty;

    // Set initial point; the following discards any initial point
    // data existing in the cache 
    // BMA TODO: handle multiple intial  points, check memory mngmnt/efficiency
    // ask for MixedIntVars set functions; consider convenience function
    const RealVector& cv_init = iteratedModel.continuous_variables();
    const IntVector&  dv_init = iteratedModel.discrete_int_variables();
    MixedIntVars miv_init(0, dv_init.length(), cv_init.length());
    TypeManager()->lexical_cast(cv_init, miv_init.Real());
    TypeManager()->lexical_cast(dv_init, miv_init.Integer());

    PointSet ps;  // allocates ResponseCache_local
    ps.add_point(colinProblem.first, miv_init);
    colinSolver->set_initial_points(ps);

    if (outputLevel == DEBUG_OUTPUT) {
      Cout << "COLIN Solver initial status and options:" << endl;
      utilib::PropertyDict_YamlPrinter printer;
      printer.print(Cout, colinSolver->Properties());
    }

    // Solve the optimization problem
    colinSolver->reset(); // only call once just before optimize
    colinSolver->optimize();

    if (outputLevel == DEBUG_OUTPUT) {
      Cout << "COLIN Solver final status:" << endl;
      utilib::PropertyDict_YamlPrinter printer;
      printer.print(Cout, colinSolver->results());
    }
 
    // Get the best points from the solver; JDS says there's no way to
    // get the "best" point out of a COLIN solver, so we have to
    // manage ourselves.  OLD COMMENT: COLIN methods do a
    // func->set_vars(best_pt) at the end of minimize, so just
    // retrieve this data and copy it into bestVariables.
    get_final_points();

  }
  catch(const runtime_error &exception) {
    Cerr << "***COLINY run-time exception*** " << exception.what() << endl;
  }
}


bool COLINOptimizer::returns_multiple_points() const
{ 
  switch(solverType) {

  case DIRECT:
  case EA:
    return true;
    break;

  default:
    return false;
    break;
  }
}


// convenience function to be called by constructors
void COLINOptimizer::solver_setup(Model& model)
{
   // This exception should never be thrown.  Normally, static_casting
   // the registration indicators to void is sufficient to have the
   // linker include the respective static libraries, but some compilers
   // (PGI 9.x) appear to be "smart" enough to recognize the no-op and
   // not link in the library.
   if ( ! ( interfaces::StaticInitializers::static_interfaces_registrations
            && scolib::StaticInitializers::static_scolib_registrations ) )
      EXCEPTION_MNGR(std::runtime_error, "COLINOptimizer::solver_setup(): "
                     "error: Acro incompletely registered (likely an issue "
                     "with the library link step.");

  // initialize some buffer variables
  constraint_penalty = 0.;
  constant_penalty = false;

  string solverstr;
  string method_name = probDescDB.get_string("method.algorithm");

  if (method_name == "coliny_cobyla") {
    solverType = COBYLA;
    solverstr = "cobyla:Cobyla";
  }
  else if (method_name == "coliny_direct") {
    solverType = DIRECT;
    solverstr = "sco:DIRECT";
  }
  else if (method_name == "coliny_ea") {
    solverType = EA;
    solverstr = "sco:EAminlp";
  }
  else if (method_name == "coliny_multi_start") {
    solverType = MS;
    solverstr = "sco:MultiStart";
  }
  else if (method_name == "coliny_pattern_search") {
    solverType = PS;
    solverstr = "sco:PatternSearch";
  }
  //else if (method_name == "coliny_pga_real")
  else if (method_name == "coliny_solis_wets") {
    solverType = SW;
    solverstr = "sco:SolisWets";
  }
  else {
    Cerr << "Error (COLINOptimizer): unknown method " << method_name << endl;
  }

  // Create the solver handle with defaults
  colinSolver = SolverMngr().create_solver(solverstr.data());
  if (colinSolver.empty())
    EXCEPTION_MNGR(std::runtime_error, "COLINOptimizer:"
		   " Solver \"" << solverstr << "\" not found");

  // We create a COLINApplication, but defer its configuration until runtime
  colinProblem = ApplicationHandle::create<COLINApplication>();
}


// BMA TODO: how to get seed from colin and set rng if necessary
void COLINOptimizer::set_rng(int seed)
{
  try {
    if (colinSolver->has_property("seed")) {
      //
      // Instantiate random number generator (RNG). Can either pass the RNG in
      // the constructor or use optimizer->set_rng(RNG) prior to 
      // interface->setup (NOTE: passing it to the SWOpt constructor has 
      // failed, so use set_rng).  LCG = a portable linear congruential 
      // generator (better than Unix RAND).
      //
      PM_LCG* prng = new PM_LCG(seed);
      if (seed != 0) // default is zero if no spec.
	Cout << "\nSeed (user-specified) = " << seed << '\n';
      else
	Cout << "\nSeed (system-generated) = " << prng->get_seed() <<'\n';
      colinSolver->set_rng(prng);
      colinSolver->property("seed") = (utilib::seed_t)seed;
      rng = prng;
    }
    else 
      rng = 0;
  }
  catch(const runtime_error &exception) {
    Cerr << "***COLINY run-time exception*** " << exception.what() << endl;
  }
}

/*string propName;
if ( solver->has_property(propName) ) {
   Property& tmp = solver->property(propName);
   if ( ! tmp.readonly() )
      tmp == newValue;
      } */

/// sets options for specific methods based on user specifications
/// (called at construction time)
void COLINOptimizer::set_solver_parameters()
{
  unsigned int pop_size;
  String pattern_basis;
  int total_pattern_size;

  if (probDescDB.is_null()) { 
    // instantiate on-the-fly:
    // rely on internal COLINY defaults for the most part, but set any
    // default overrides (including enforcement of DAKOTA defaults) here
    pop_size = 100;
    pattern_basis = "coordinate";
    total_pattern_size = 2*numContinuousVars;
  }
  else {
    // previous cobyla, pattern search, and solis-wets parameters
    const Real& init_delta 
      = probDescDB.get_real("method.coliny.initial_delta");
    if (init_delta >= 0.0 && colinSolver->has_property("initial_step"))
      colinSolver->property("initial_step") = init_delta;

    const Real& thresh_delta
      = probDescDB.get_real("method.coliny.threshold_delta");
    if (thresh_delta >= 0.0 && colinSolver->has_property("step_tolerance"))
      colinSolver->property("step_tolerance") = thresh_delta;

    // previous direct parameters
    // returns "major_dimension", "all_dimensions", or empty string (default)
    const String& division_type
      = probDescDB.get_string("method.coliny.division");
    if (colinSolver->has_property("division")) {
      if (division_type == "major_dimension")
	colinSolver->property("division") = (string)"single";
      else if (division_type == "all_dimensions")
	colinSolver->property("division") = (string)"multi";
      else {
	// user didn't specify, so by default use multiple division in asynch
	// case, otherwise single division (a maximum of 2 asynch evals)
	if (iteratedModel.asynch_flag())
	  colinSolver->property("division") = (string)"multi";
	else
	  colinSolver->property("division") = (string)"single";
      }
    }

    const Real& global_bal_param
      = probDescDB.get_real("method.coliny.global_balance_parameter");
    if (global_bal_param >= 0. && colinSolver->has_property("global_search_balance"))
      colinSolver->property("global_search_balance") = global_bal_param;

    const Real& local_bal_param
      = probDescDB.get_real("method.coliny.local_balance_parameter");
    if (local_bal_param >= 0. && colinSolver->has_property("local_search_balance"))
      colinSolver->property("local_search_balance") = local_bal_param;

    const Real& max_box
      = probDescDB.get_real("method.coliny.max_boxsize_limit");
    if (max_box >= 0. && colinSolver->has_property("max_boxsize_limit"))
      colinSolver->property("max_boxsize_limit") = max_box;

    const Real& min_box = probDescDB.get_real("method.min_boxsize_limit");
    if (min_box >= 0. && colinSolver->has_property("min_boxsize_limit"))
      colinSolver->property("min_boxsize_limit") = min_box;

    pop_size = probDescDB.get_int("method.population_size");
    if (!pop_size)
      pop_size = 100;

    const String& pop_init_type =
      probDescDB.get_string("method.initialization_type");
    if (colinSolver->has_property("population_unique")) {
      if (pop_init_type == "simple_random") 
	colinSolver->property("population_unique") = false;
      else if (pop_init_type == "unique_random")
	colinSolver->property("population_unique") = true;
      else if (pop_init_type == "flat_file") {
	const String& flat_file = probDescDB.get_string("method.flat_file");
	colinSolver->property("init_filename") = flat_file.c_str();
      }
    }

    const String& selection_type =
      probDescDB.get_string("method.fitness_type");
    if (colinSolver->has_property("selection_type")) {
      if (selection_type == "merit_function") 
	colinSolver->property("selection_type") = (string)"proportional";
      else if (selection_type == "linear_rank")
	colinSolver->property("selection_type") = (string)"linear_rank";
    }

    const String& replacement_type =
      probDescDB.get_string("method.replacement_type");
    if (colinSolver->has_property("replacement_method"))
      colinSolver->property("replacement_method") = replacement_type.c_str();

    int keep_num = probDescDB.get_int("method.coliny.number_retained");
    if (keep_num >= 0 && colinSolver->has_property("keep_num")) {
      unsigned int tmp = static_cast<unsigned int>(keep_num);
      colinSolver->property("keep_num") = tmp;
    }

    int new_solutions = 
      probDescDB.get_int("method.coliny.new_solutions_generated");
    if (new_solutions >= 0 && colinSolver->has_property("num_trial_points")) {
      unsigned int tmp = static_cast<unsigned int>(new_solutions);
      colinSolver->property("num_trial_points") = tmp;
    }

    double crossover_rate  = probDescDB.get_real("method.crossover_rate");
    if (colinSolver->has_property("xover_rate"))
      colinSolver->property("xover_rate") = crossover_rate;

    const String& crossover_type =
      probDescDB.get_string("method.crossover_type");
    if (colinSolver->has_property("realarray_xover_type") && colinSolver->has_property("intarray_xover_type")) {
      if (crossover_type == "blend") {
	colinSolver->property("realarray_xover_type") = (string)"blend";
	colinSolver->property("intarray_xover_type") = (string)"twopoint";
      }
      else if (crossover_type == "uniform") {
	colinSolver->property("realarray_xover_type") = (string)"uniform";
	colinSolver->property("intarray_xover_type") = (string)"uniform";
      }
      else { // default crossover type = "two_point"
	colinSolver->property("realarray_xover_type") = (string)"twopoint";
	colinSolver->property("intarray_xover_type") = (string)"twopoint";
      }
    }

    double mutation_rate  = 
      probDescDB.get_real("method.mutation_rate");
    if (colinSolver->has_property("mutation_rate"))
      colinSolver->property("mutation_rate") = mutation_rate;

    const String& mutation_type = 
      probDescDB.get_string("method.mutation_type");
    if (colinSolver->has_property("realarray_mutation_type")) {
      if (mutation_type == "replace_uniform" ||
	  mutation_type == "offset_normal" || mutation_type == "offset_cauchy")
	colinSolver->property("realarray_mutation_type") = mutation_type.c_str();
      else // default mutation type = "offset_uniform"
	colinSolver->property("realarray_mutation_type") = (string)"offset_uniform";
    }

    if (colinSolver->has_property("intarray_mutation_type"))
      colinSolver->property("intarray_mutation_type") = (string)"uniform";

    double mutation_scale = probDescDB.get_real("method.mutation_scale");
    if (colinSolver->has_property("realarray_mutation_scale"))
      colinSolver->property("realarray_mutation_scale") = mutation_scale;

    int mutation_range = probDescDB.get_int("method.coliny.mutation_range");
    if (mutation_range >= 0 && colinSolver->has_property("intarray_mutation_range")) {
      unsigned int tmp = static_cast<unsigned int>(mutation_range);
      colinSolver->property("intarray_mutation_range") = tmp;
    }

    const bool& mutation_adaptive = 
      probDescDB.get_bool("method.mutation_adaptive");
    if (colinSolver->has_property("realarray_mutation_selfadaptation")) {
      if (mutation_adaptive)
	colinSolver->property("realarray_mutation_selfadaptation") = true;
      else
	colinSolver->property("realarray_mutation_selfadaptation") = false;
    }

    // previous pattern search and solis-wets parameters
    // A null string is the DB default and nonblocking is the PS default, so
    // the flag is true only for an explicit blocking user specification.
    blockingSynch = (probDescDB.get_string("method.coliny.synchronization")
		     == "blocking") ? true : false;

    // no need to check for case of -1.e+25 since the COLINY defaults are
    // replicated in DataMethod.C
    const Real& contraction_factor
      = probDescDB.get_real("method.coliny.contraction_factor");
    if (colinSolver->has_property("contraction_factor"))
      colinSolver->property("contraction_factor") = contraction_factor;

    const int& contract_after_failure
      = probDescDB.get_int("method.coliny.contract_after_failure");
    if (contract_after_failure>=0 && colinSolver->has_property("max_failure"))
      colinSolver->property("max_failure") = contract_after_failure;

    const bool& expansion_flag
      = probDescDB.get_bool("method.coliny.expansion");
    if (expansion_flag) {
      const int& expand_after_success
	= probDescDB.get_int("method.coliny.expand_after_success");
      if (expand_after_success>=0 && colinSolver->has_property("max_success"))
	colinSolver->property("max_success") = expand_after_success;
      if (colinSolver->has_property("expansion_factor"))
	colinSolver->property("expansion_factor") = 1.0/contraction_factor;
    }
    else { // turn expansion "off" by using expansion_factor = 1.0 
      if (colinSolver->has_property("expansion_factor"))
	colinSolver->property("expansion_factor") = 1.0;
    }

    const String& exploratory_moves
      = probDescDB.get_string("method.coliny.exploratory_moves");
    if (!exploratory_moves.empty() && colinSolver->has_property("exploratory_move"))
      colinSolver->property("exploratory_move") = exploratory_moves.c_str();

    const bool& coliny_randomize
      = probDescDB.get_bool("method.coliny.randomize");
    if (colinSolver->has_property("step_selection")) {
      if (coliny_randomize)
	colinSolver->property("step_selection") = (string)"random";
      else
	colinSolver->property("step_selection") = (string)"fixed";
    }

    pattern_basis = probDescDB.get_string("method.coliny.pattern_basis");
    total_pattern_size
      = probDescDB.get_int("method.coliny.total_pattern_size");
    // 'coordinate' is current COLINY default:
    int basic_pattern_size = (pattern_basis == "simplex")
      ? numContinuousVars + 1 : 2*numContinuousVars;
    int num_augmented_trials = total_pattern_size - basic_pattern_size;

    // catch bad input or default total_pattern_size = 0
    if (num_augmented_trials > 0 && colinSolver->has_property("num_augmented_trials"))
      colinSolver->property("num_augmented_trials") = num_augmented_trials;
    else // enforce lower bound of basic pattern
      total_pattern_size = basic_pattern_size;

    //
    // COLINY specification attributes from database
    //
    const Real& solution_accuracy
      = probDescDB.get_real("method.solution_target");
    if (solution_accuracy > -DBL_MAX && colinSolver->has_property("sufficient_objective_value"))
      colinSolver->property("sufficient_objective_value") = solution_accuracy;

    // JDS: buffer the constraint_penalty here: in DAKOTA, this is a
    // property of the solver; in COLIN it is a property of the
    // reformulated application.  Unfortunately, the probDescDB is
    // locked by the time we hit find_optimum().
    constraint_penalty = probDescDB.get_real("method.constraint_penalty");

    const bool& show_misc_options
      = probDescDB.get_bool("method.coliny.show_misc_options");
    if (show_misc_options){
      Cout << "---------------------------SOLVER OPTIONS"
	   << "---------------------------\n";
      //	colinSolver->options().write(Cout, true);
      //	colinSolver->options().write_parameters(Cout);
      Cout << "---------------------------SOLVER OPTIONS"
	   << "---------------------------" << endl;
    }

    const StringArray& misc_options
      = probDescDB.get_dsa("method.coliny.misc_options");
    size_t num_mo = misc_options.size();
    for (size_t i=0; i<num_mo; i++) {
      string thisOption(misc_options[i]);
      size_t equalPos = thisOption.find("=",0);
      if (equalPos == string::npos)
	Cout << "Warning - COLINY ignoring option " << misc_options[i] <<endl;
      else {
	string option(thisOption.substr(0, equalPos));
	string value(thisOption.substr(equalPos+1, thisOption.size()-1));
	colinSolver->property(option) = value;
      }
    }
  }

  if (solverType == DIRECT)
    maxConcurrency *= 2*numContinuousVars;
  else if (solverType == EA) {
    maxConcurrency *= pop_size;
    if (colinSolver->has_property("population_size")) {
      colinSolver->property("population_size") = pop_size;
    }
  }
  else if (solverType == PS) {
    if (!pattern_basis.empty() && colinSolver->has_property("basis"))
      colinSolver->property("basis") = (string)pattern_basis;
    maxConcurrency *= total_pattern_size;
    if (!blockingSynch)
      Cout << "\n Coliny Asynchronous Pattern Search temporarily not supported."
	   << "\n Will return in future releases.  Running synchronously.\n\n";
  }

  // previous parameters for all algorithms
  //
  // Inherited attributes
  //

  if (solverType != COBYLA) {
    if (colinSolver->has_property("max_function_evaluations"))
      colinSolver->property("max_function_evaluations") = maxFunctionEvals;
    if (colinSolver->has_property("max_iterations"))
      colinSolver->property("max_iterations") = maxIterations;
  }
  if (colinSolver->has_property("function_value_tolerance"))
    colinSolver->property("function_value_tolerance") = convergenceTol;

  switch (outputLevel) {

  case DEBUG_OUTPUT:
      
    if (colinSolver->has_property("output_level"))
      colinSolver->property("output_level") = (string)"verbose";
    if (colinSolver->has_property("debug"))
      colinSolver->property("debug") = 10000;
    break;

  case VERBOSE_OUTPUT:
      
    if (colinSolver->has_property("output_level"))
      colinSolver->property("output_level") = (string)"summary";
    break;

  case NORMAL_OUTPUT:
  case QUIET_OUTPUT:

    if (colinSolver->has_property("output_level"))
      colinSolver->property("output_level") = (string)"normal";
    break;

  case SILENT_OUTPUT:
    if (colinSolver->has_property("output_level"))
      colinSolver->property("output_level") = (string)"none";

  }
}

void COLINOptimizer::get_final_points()
{
  // retreive the COLIN points (local cache) in the right Application context
  list<Any> points;
  PointSet ps = colinSolver->get_final_points();
  ps.get_points(colinProblem.second, points);

  // prepare to receive them on the DAKOTA side
  resize_final_points(points.size());
  resize_final_responses(points.size());

  // crude tracking of best function value and its index for now
  Real min_fn = DBL_MAX;
  size_t best_idx = 0;
  list<Any>::iterator pt_it = points.begin();
  list<Any>::iterator pt_end = points.end();

  for(size_t i=0; pt_it!=pt_end; i++, pt_it++) {

    utilib::Any tmp;
    TypeManager()->lexical_cast(*pt_it, tmp, typeid(utilib::MixedIntVars));
    utilib::MixedIntVars& miv = tmp.expose<utilib::MixedIntVars>();

    RealVector cdv;
    TypeManager()->lexical_cast(miv.Real(), cdv);
    bestVariablesArray[i].continuous_variables(cdv);
    IntVector ddv;
    TypeManager()->lexical_cast(miv.Integer(), ddv);
    bestVariablesArray[i].discrete_int_variables(ddv);


    // get the optimal response and constraint values (presumably from
    // the cache)
    // there are three possible courses here: DAKOTA cache, DAKOTA model eval, COLIN model eval, all of which ultimately results in a cache lookup, but only the first won't increment eval duplicates.

    // BMA TODO: use DAKOTA PRP cache; utlimately want to store gradients in
    // BestReponses too; what if there was a recast?!?
    //    RealVector fn_vals(numFunctions);
    ShortArray search_asv(numFunctions, 1);
    for (size_t j=numObjectiveFns; j<numFunctions; j++)
      search_asv[j] = 0; // assuming we don't need constraints for now
    ActiveSet search_set;
    // BMA TODO: might not have evaluated both functions and
    // constraints at this point

    search_set.request_vector(search_asv);
    extern PRPCache data_pairs; // global container
    Response desired_resp;
    if (lookup_by_val(data_pairs, iteratedModel.interface_id(), 
		      bestVariablesArray[i], activeSet, desired_resp)) {
      const RealVector& fn_vals = desired_resp.function_values();
      bestResponseArray[i].function_values(fn_vals);

      // save best response in single objective case
      if (!multiObjFlag && fn_vals[0] < min_fn) {
	best_idx = i;
	min_fn = fn_vals[0];

      }
    }
    else {
      Cerr << "Warning: failure in recovery of final objective/constraints."
	   <<endl;
      //abort_handler(-1);
    }

    //    for(size_t j=0; j<numNonlinearConstraints; j++)
    //  fn_vals[j+numObjectiveFns] = fc[j];


  }
  
  // this stinks, but COLIN doesn't return a best point anymore
  if (!multiObjFlag) {
    //    bestVariables = bestVariablesArray[best_idx].copy();
    //    bestResponse = bestResponseArray[best_idx].copy();
  }

}


void COLINOptimizer::resize_final_points(size_t newsize)
{
  // If there is no change in size, we needn't continue.
  if(newsize == bestVariablesArray.size()) return;

  // If this is a reduction in size, we can use the standard resize method
  if(newsize < bestVariablesArray.size())
  { bestVariablesArray.resize(newsize); return; }

  // Otherwise, we have to do the iteration ourselves so that we make use
  // of the model's current variables for envelope-letter requirements.
  bestVariablesArray.reserve(newsize);
  for(std::size_t i=bestVariablesArray.size(); i<newsize; ++i)
    bestVariablesArray.push_back(iteratedModel.current_variables().copy());
}


void COLINOptimizer::resize_final_responses(size_t newsize)
{
  // If there is no change in size, we needn't continue.
  if(newsize == bestResponseArray.size()) return;

  // If this is a reduction in size, we can use the standard resize method
  if(newsize < bestResponseArray.size())
  { bestResponseArray.resize(newsize); return; }

  // Otherwise, we have to do the iteration ourselves so that we make use
  // of the model's current variables for envelope-letter requirements.
  bestResponseArray.reserve(newsize);
  for(std::size_t i=bestResponseArray.size(); i<newsize; ++i)
    bestResponseArray.push_back(iteratedModel.current_response().copy());
}

} // namespace Dakota
