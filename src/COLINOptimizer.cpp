/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       COLINOptimizer
//- Description: Implementation of the COLINOptimizer class, including type 
//-              manager registrations for conversion
//- Owner:       Patty Hough/John Siirola/Brian Adams
//- Checked by:
//- Version: $Id$

#include "COLINApplication.hpp"
#include "COLINOptimizer.hpp"
#include "ProblemDescDB.hpp"
#include "ParamResponsePair.hpp"
#include "PRPMultiIndex.hpp"
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
#include <colin/AppResponseInfo.h>
using colin::mf_info;
using colin::nlcf_info;
using colin::ConstraintPenaltyApplication;
#include <colin/SolverMngr.h>
using colin::SolverMngr;
#include <colin/PointSet.h>
using colin::PointSet;
#include <utilib/PM_LCG.h>
using utilib::PM_LCG;
#include <colin/TinyXML_data_parser.h>

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

/// helper function to initialize the COLIN cache
void init_colin_cache()
{
  static colin::CacheHandle colinCache;
  // we only want to initialize the cache once... so if the handle is
  // not empty, this function should become a no-op.
  if ( ! colinCache.empty() )
    return;

  // Tell COLIN solver to use COLIN's cache.  We don't need to
  // register it.

  colinCache = colin::CacheFactory().create("Local");

  colin::CacheFactory().intersolver_cache() 
    = colin::CacheFactory().evaluation_cache() = colinCache;
}

  // These functions define the operations needed to cast from one
  // type to another in order to freely make assignments of one type
  // of data to another.  This is particularly useful in converting
  // DAKOTA types when setting algorithm properties.

  /// Cast from DAKOTA RealVector to std::vector<double>.

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

  /// Cast from std::vector<double> to DAKOTA RealVector.

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

  /// Cast from DAKOTA IntVector to std::vector<int>.

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

  /// Cast from std::vector<int> to DAKOTA IntVector.

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

  /// Cast from int to unsigned int.

int cast_from_int_to_unsignedint(const Any& src, Any& dest)
{ 
  const int& tmp = src.expose<int>();
  unsigned int& ans = dest.set<unsigned int>();
  ans = (unsigned int)tmp;
  return static_cast<int>(ans) == tmp 
     ? 0 : utilib::Type_Manager::CastWarning::ValueOutOfRange;
}

  /// Cast for unsigned in to int.

int cast_from_unsignedint_to_int(const Any& src, Any& dest)
{ 
  const unsigned int& tmp = src.expose<unsigned int>();
  int& ans = dest.set<int>();
  ans = tmp;
  return static_cast<unsigned int>(ans) == tmp 
     ? 0 : utilib::Type_Manager::CastWarning::ValueOutOfRange;
}

  /// Cast from char const* to std::string.

int cast_from_charconst_to_string(const Any& src, Any& dest)
{ 
  char const* tmp = src.expose<char const*>();
  string& ans = dest.set<string>();
  ans = tmp;
  return 0;
}

  /// Register the cast functions.

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

// Enumerate the solverType

enum { COBYLA, DIRECT, EA, MS, PS, SW, BETA };

  /// Standard constructor.

COLINOptimizer::COLINOptimizer(ProblemDescDB& problem_db, Model& model):
  Optimizer(problem_db, model)
{
  // (iteratedModel initialized in Optimizer(Model&))
  // Set solver properties.

  solver_setup(probDescDB.get_ushort("method.algorithm"));
  set_rng(probDescDB.get_int("method.random_seed"));
  set_solver_parameters();
}

  /// Alternate constructor for on-the-fly instantiations.

COLINOptimizer::
COLINOptimizer(const String& method_string, Model& model, int seed,
	       int max_iter, int max_eval):
  Optimizer(method_string_to_enum(method_string), model), blockingSynch(true)
{
  // (iteratedModel initialized in Optimizer(Model&))
  // Set solver properties.

  solver_setup(method_string_to_enum(method_string));
  set_rng(seed);

  maxIterations = max_iter; maxFunctionEvals = max_eval;
  set_solver_parameters();
}

  /// Alternate constructor for Iterator instantiations by name.

COLINOptimizer::
COLINOptimizer(const String& method_string, Model& model):
  Optimizer(method_string_to_enum(method_string), model), rng(NULL),
  blockingSynch(true)
{
  // (iteratedModel initialized in Optimizer(Model&))
  // Set solver properties.
  solver_setup(method_string_to_enum(method_string));
  set_solver_parameters();
}

/** core_run redefines the Optimizer virtual function to perform
    the optimization using COLIN. It first sets up the problem data,
    then executes optimize() on the COLIN solver and finally
    catalogues the results. */

void COLINOptimizer::core_run()
{
  // Much of this is performed in core_run in order to capture any
  // reassignment at the strategy layer (after iterator construction).

  try {
    // Instantiate the evaluation manager.  Use the concurrent
    // evaluator for asynchronous evaluations and the serial evaluator
    // for synchronous.

     colinEvalMgr = colin::EvalManagerFactory()
        .create(iteratedModel.asynch_flag() ? "Concurrent" : "Serial");
     if (colinEvalMgr->has_property("max_concurrency"))
       colinEvalMgr->property("max_concurrency")
	 = iteratedModel.evaluation_capacity();

     // Instantiate the solver.

     colinSolver->set_evaluation_manager(colinEvalMgr);
     colinProblem.second->set_evaluation_manager(colinEvalMgr);

    // Initialize the COLINApplication with the appropriate model
    // this will initialize functions, constraints, and bounds

    colinProblem.second->set_problem(iteratedModel);
    colinProblem.second->set_blocking_synch(blockingSynch);
    colinSolver->set_problem(colinProblem.first);

    // Set the constraint penalty.  This is a property of the
    // application/problem, not the solver.  It must be done after the
    // problem is handed to the solver and all reformulations have
    // taken place.  Therefore, it is set here rather than at
    // construction time.

    // TODO: Revisit default penalty.  Here it is currently algorithm
    // specific.  This is to enforce consistency with COLIN 2
    // behavior.

    colin::ApplicationHandle problem = colinSolver->get_problem_handle();
    if ( constraint_penalty >= 0. )
      problem->property("constraint_penalty") = constraint_penalty;
    else if (solverType == PS || solverType == SW)
      problem->property("constraint_penalty") = 1.0;
    if ( problem->has_property("apply_convergence_factor") )
      problem->property("apply_convergence_factor") = !constant_penalty;

    // Set initial point.  The following discards any initial point
    // data existing in the cache.

    // TODO: handle multiple intial points, check memory
    // mngmnt/efficiency

    // There are continuous, discrete integer, and discrete real
    // variables.  COLIN does not distinguish between discrete integer
    // and discrete real.

    const RealVector&  cv_init = iteratedModel.continuous_variables();
    const IntVector&  div_init = iteratedModel.discrete_int_variables();
    const RealVector& drv_init = iteratedModel.discrete_real_variables();
    StringMultiArrayConstView dsv_init = 
	    iteratedModel.discrete_string_variables();
    IntVector dv_init(numDiscreteIntVars
		    + numDiscreteRealVars
		    + numDiscreteStringVars);

    // Initialize the continuous variables in COLIN's mixed-variable
    // object.

    MixedIntVars miv_init(0,
		    numDiscreteIntVars
		    + numDiscreteRealVars
		    + numDiscreteStringVars,
		    cv_init.length());
    TypeManager()->lexical_cast(cv_init, miv_init.Real());

    // Need to consolidate all of the discrete variables in one place.
    // First do all of the active discrete integer variables.  Note that
    // this could involve a reordering if more than one type of discrete
    // integer variables is active, as we won't try to mimic the full DAKOTA
    // variables ordering.  Note: the abstraction to active discrete vars
    // is necessary to allow usage beyond design optimization; e.g., for
    // epistemic interval estimation.

    const       BitArray& di_set_bits = iteratedModel.discrete_int_sets();
    const    IntSetArray& dsiv_values = iteratedModel.discrete_set_int_values();
    const   RealSetArray& dsrv_values = iteratedModel.discrete_set_real_values();
    const StringSetArray& dssv_values = iteratedModel.discrete_set_string_values();
    size_t i, index, dsi_cntr;
    for (i=0, dsi_cntr=0; i<numDiscreteIntVars; ++i) {
      if (di_set_bits[i]) { // this active discrete int var is a set type
	// Given the initial value, find its index (i.e., relative location)
	// in the list of possible values.  Treat that index as the value
	// on which the COLIN solver will operate.  We can put bounds on
	// the index (done in COLINApplication), thereby restricting the
	// solver's exploration to the prescribed values.  We have to
	// undo this transformation when doing a function evaluation
	// (in COLINApplication) and when getting the final points.
	index = set_value_to_index(div_init[i], dsiv_values[dsi_cntr]);
	if (index == _NPOS) {
	  Cerr << "\nError: failure in discrete integer set lookup within "
	       << "COLINOptimizer::core_run()" << std::endl;
	  abort_handler(-1);
	}
	else
	  dv_init[i] = (int)index;
	++dsi_cntr;
      }
      else                  // this active discrete int var is a range type
	dv_init[i] = div_init[i];
    }

    // Do the same for the active discrete real vars, which are all set types

    for (i=0; i<numDiscreteRealVars; ++i) {
      index = set_value_to_index(drv_init[i], dsrv_values[i]);
      if (index == _NPOS) {
	Cerr << "\nError: failure in discrete real set lookup within "
	     << "COLINOptimizer::core_run()" << std::endl;
	abort_handler(-1);
      }
      else
	dv_init[i+numDiscreteIntVars] = (int)index;
    }

    // Finally, for active discrete string vars

    for (i=0; i<numDiscreteStringVars; ++i) {
      index = set_value_to_index(dsv_init[i], dssv_values[i]);
      if (index == _NPOS) {
	Cerr << "\nError: failure in discrete string set lookup within "
	     << "COLINOptimizer::core_run()" << std::endl;
	abort_handler(-1);
      }
      else
	dv_init[i+numDiscreteIntVars+numDiscreteRealVars] = (int)index;
    }


    // Now initialize the discrete variables in COLIN's mixed-variable object

    TypeManager()->lexical_cast(dv_init, miv_init.Integer());
    colin::CacheFactory().evaluation_cache()->clear();

    // Set the initial point.

    PointSet ps;  // allocates ResponseCache_local
    ps.add_point(colinProblem.first, miv_init);
    colinSolver->set_initial_points(ps);

    if (outputLevel == DEBUG_OUTPUT) {
      Cout << "COLIN Solver initial status and options:" << endl;
      utilib::PropertyDict_YamlPrinter printer;
      printer.print(Cout, colinSolver->Properties());
    }

    // Solve the optimization problem.  Only call reset once just
    // before optimize.

    colinSolver->reset();
    colinSolver->optimize();

    if (outputLevel == DEBUG_OUTPUT) {
      Cout << "COLIN Solver final status:" << endl;
      utilib::PropertyDict_YamlPrinter printer;
      printer.print(Cout, colinSolver->results());
    }
 
    // Getting the best and final points from the solver is done in
    // the override of post_run().

  }
  catch(const runtime_error &exception) {
    Cerr << "***COLINY run-time exception*** " << exception.what() << endl;
  }
}

  /// Designate which solvers can return multiple final points.

bool COLINOptimizer::returns_multiple_points() const
{
  // Need to know this for sequential hybrid methods.

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


  /// This convenience function is called by the constructors in order to
  /// instantiate the solver.

void COLINOptimizer::solver_setup(unsigned short method_name)
{
   // This exception should never be thrown.  Normally, static_casting
   // the registration indicators to void is sufficient to have the
   // linker include the respective static libraries, but some compilers
   // (PGI 9.x) appear to be "smart" enough to recognize the no-op and
   // not link in the library.

   if ( interfaces::StaticInitializers::static_interfaces_registrations
        || scolib::StaticInitializers::static_scolib_registrations )
      EXCEPTION_MNGR(std::runtime_error, "COLINOptimizer::solver_setup(): "
                     "error: Acro incompletely registered (likely an issue "
                     "with the library link step ("
                     << scolib::StaticInitializers::static_scolib_registrations
                     << ","
                     << interfaces::StaticInitializers::static_interfaces_registrations
                     << ").");

   init_colin_cache();

  // Initialize constraint-related buffer variables.  Explanation for
  // why these are need is in set_method_parameters().

  constraint_penalty = 0.;
  constant_penalty = false;

  // Determine which solver is specified.

  string solverstr;
  switch (method_name) {
  case COLINY_COBYLA:
    solverType = COBYLA; solverstr = "cobyla:Cobyla";     break;
  case COLINY_DIRECT:
    solverType = DIRECT; solverstr = "sco:DIRECT";        break;
  case COLINY_EA:
    solverType = EA;     solverstr = "sco:EAminlp";       break;
  case COLINY_MULTI_START:
    solverType = MS;     solverstr = "sco:MultiStart";    break;
  case COLINY_PATTERN_SEARCH:
    solverType = PS;     solverstr = "sco:PatternSearch"; break;
  //case COLINY_PGA_REAL:
  case COLINY_SOLIS_WETS:
    solverType = SW;     solverstr = "sco:SolisWets";     break;
  case COLINY_BETA:
    solverType = BETA;
    solverstr = probDescDB.get_string("method.coliny.beta_solver_name");
    break;
  default:
    Cerr << "Error (COLINOptimizer): unknown method "
	 << method_enum_to_string(method_name) << std::endl;
  }

  // Create the appropriate solver handle with defaults.

  colinSolver = SolverMngr().create_solver(solverstr.data());
  if (colinSolver.empty())
    EXCEPTION_MNGR(std::runtime_error, "COLINOptimizer:"
		   " Solver \"" << solverstr << "\" not found");

  // We create a COLINApplication but defer its configuration until
  // runtime.

  colinProblem = ApplicationHandle::create<COLINApplication>();
}

  /// Instantiate random number generator (RNG).

void COLINOptimizer::set_rng(int seed)
{
  try {
    if (colinSolver->has_property("seed")) {
      //
      //  Can either pass the RNG in the constructor or use
      // optimizer->set_rng(RNG) prior to interface->setup (NOTE:
      // passing it to the SWOpt constructor has failed, so use
      // set_rng).  LCG = a portable linear congruential generator
      // (better than Unix RAND).
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

/// Sets solver properties based on user specifications.  Called at
/// construction time.

void COLINOptimizer::set_solver_parameters()
{
  // TODO:  Make this more robust.  Really should be doing something
  // like the following:
  //    string propName;
  //    if ( solver->has_property(propName) ) {
  //       Property& tmp = solver->property(propName);
  //       if ( ! tmp.readonly() )
  //       tmp == newValue;
  //    }
  // Also, create a map from DAKOTA specs to COLIN properties.  Then
  // loop through the map to set the properties.  This will eliminate
  // a lot of duplicate code.

  // Need these variables throughout.

  unsigned int pop_size;
  String pattern_basis;
  int total_pattern_size;

  if (probDescDB.is_null()) { 
    // Instantiate on-the-fly.  Rely on internal COLINY defaults for
    // the most part, but set any default overrides (including
    // enforcement of DAKOTA defaults).

    pop_size = 100;
    pattern_basis = "coordinate";
    total_pattern_size = 2*numContinuousVars;
  }
  else {

    // Previous COBYLA, Pattern Search, and Solis-Wets parameters.

    const Real& init_delta 
      = probDescDB.get_real("method.coliny.initial_delta");
    if (init_delta >= 0.0 && colinSolver->has_property("initial_step"))
      colinSolver->property("initial_step") = init_delta;

    const Real& thresh_delta
      = probDescDB.get_real("method.coliny.threshold_delta");
    if (thresh_delta >= 0.0 && colinSolver->has_property("step_tolerance"))
      colinSolver->property("step_tolerance") = thresh_delta;

    // Previous DIRECT parameters.
    // Returns "major_dimension", "all_dimensions", or empty string
    // (default).

    const String& division_type
      = probDescDB.get_string("method.coliny.division");
    if (colinSolver->has_property("division")) {
      if (division_type == "major_dimension")
	colinSolver->property("division") = string("single");
      else if (division_type == "all_dimensions")
	colinSolver->property("division") = string("multi");
      else {
	// User didn't specify, so by default use multiple division in
	// asynch case, otherwise single division (a maximum of 2
	// asynch evals).

	if (iteratedModel.asynch_flag())
	  colinSolver->property("division") = string("multi");
	else
	  colinSolver->property("division") = string("single");
      }
    }

    const Real& global_bal_param
      = probDescDB.get_real("method.coliny.global_balance_parameter");
    if (global_bal_param >= 0. && colinSolver->has_property("min_improvement"))
      colinSolver->property("min_improvement") = global_bal_param;

    const Real& local_bal_param
      = probDescDB.get_real("method.coliny.local_balance_parameter");
    if (local_bal_param >= 0. && colinSolver->has_property("max_boxsize_ratio"))
      colinSolver->property("max_boxsize_ratio") = local_bal_param;

    const Real& max_box
      = probDescDB.get_real("method.coliny.max_boxsize_limit");
    if (max_box >= 0. && colinSolver->has_property("max_boxsize_limit"))
      colinSolver->property("max_boxsize_limit") = max_box;

    const Real& min_box = probDescDB.get_real("method.min_boxsize_limit");
    if (min_box >= 0. && colinSolver->has_property("min_boxsize_limit"))
      colinSolver->property("min_boxsize_limit") = min_box;

    // Previous EA parameters.

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
      if (selection_type == "proportional")
	colinSolver->property("selection_type") = string("proportional");
      else // default selection_type = "linear_rank"
	colinSolver->property("selection_type") = string("linear_rank");
    }

    const String& replacement_type =
      probDescDB.get_string("method.replacement_type");
    if (colinSolver->has_property("replacement_method")) {
      if (replacement_type != "")
	colinSolver->property("replacement_method") = replacement_type.c_str();
      else
	colinSolver->property("replacement_method") = string("elitist");
    }

    int keep_num = probDescDB.get_int("method.coliny.number_retained");
    if (colinSolver->has_property("keep_num")) {
      if (keep_num >= 0)
	colinSolver->property("keep_num") = keep_num;
      else
	colinSolver->property("keep_num") = 1;
    }

    int new_solutions = 
      probDescDB.get_int("method.coliny.new_solutions_generated");
    if (new_solutions >= 0 && colinSolver->has_property("num_trial_points")) {
      colinSolver->property("num_trial_points") = new_solutions;
    }

    double crossover_rate  = probDescDB.get_real("method.crossover_rate");
    if (colinSolver->has_property("xover_rate")) {
      if (crossover_rate >= 0.)
	colinSolver->property("xover_rate") = crossover_rate;
      else
	colinSolver->property("xover_rate") = 0.8;
    }

    const String& crossover_type =
      probDescDB.get_string("method.crossover_type");
    if (colinSolver->has_property("realarray_xover_type") && colinSolver->has_property("intarray_xover_type")) {
      if (crossover_type == "blend") {
	colinSolver->property("realarray_xover_type") = string("blend");
	colinSolver->property("intarray_xover_type") = string("twopoint");
      }
      else if (crossover_type == "uniform") {
	colinSolver->property("realarray_xover_type") = string("uniform");
	colinSolver->property("intarray_xover_type") = string("uniform");
      }
      else { // default crossover type = "two_point"
	colinSolver->property("realarray_xover_type") = string("twopoint");
	colinSolver->property("intarray_xover_type") = string("twopoint");
      }
    }

    double mutation_rate  = 
      probDescDB.get_real("method.mutation_rate");
    if (mutation_rate >= 0.0 && mutation_rate <= 1.0) {
      if (colinSolver->has_property("mutation_rate"))
	colinSolver->property("mutation_rate") = mutation_rate;
    }
    else {
      Cerr << "\nWarning: mutation_rate outside acceptable range of [0,1]."
           << "\n         Using default value of 1.0.\n\n";
    }

    const String& mutation_type = 
      probDescDB.get_string("method.mutation_type");
    if (colinSolver->has_property("realarray_mutation_type")) {
      if (mutation_type == "replace_uniform" ||
	  mutation_type == "offset_normal" || mutation_type == "offset_cauchy")
	colinSolver->property("realarray_mutation_type") = mutation_type.c_str();
      else // default mutation type = "offset_uniform"
	colinSolver->property("realarray_mutation_type") = string("offset_uniform");
    }

    if (colinSolver->has_property("intarray_mutation_type"))
      colinSolver->property("intarray_mutation_type") = string("uniform");

    double mutation_scale = probDescDB.get_real("method.mutation_scale");
    if (colinSolver->has_property("realarray_mutation_scale"))
      colinSolver->property("realarray_mutation_scale") = mutation_scale;

    int mutation_range = probDescDB.get_int("method.coliny.mutation_range");
    if (mutation_range >= 0 && colinSolver->has_property("intarray_mutation_range")) {
      colinSolver->property("intarray_mutation_range") = mutation_range;
    }

    const bool& mutation_adaptive = 
      probDescDB.get_bool("method.mutation_adaptive");
    if (colinSolver->has_property("realarray_mutation_selfadaptation")) {
      if (mutation_adaptive)
	colinSolver->property("realarray_mutation_selfadaptation") = true;
      else
	colinSolver->property("realarray_mutation_selfadaptation") = false;
    }

    // Previous Pattern Search and Solis-Wets parameters.  A null
    // string is the DB default and nonblocking is the PS default, so
    // the flag is true only for an explicit blocking user
    // specification.

    // default is blocking (most solvers) unless explicit override for PS
    blockingSynch =
      (probDescDB.get_string("method.pattern_search.synchronization") ==
       "nonblocking") ? false : true;

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
      if (expand_after_success>0 && colinSolver->has_property("max_success"))
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
	colinSolver->property("step_selection") = string("random");
      else
	colinSolver->property("step_selection") = string("fixed");
    }

    pattern_basis = probDescDB.get_string("method.coliny.pattern_basis");
    total_pattern_size
      = probDescDB.get_int("method.coliny.total_pattern_size");
    // 'coordinate' is current COLINY default:
    int basic_pattern_size = (pattern_basis == "simplex")
      ? numContinuousVars + 1 : 2*numContinuousVars;
    int num_augmented_trials = total_pattern_size - basic_pattern_size;

    if (num_augmented_trials > 0 && colinSolver->has_property("num_augmented_trials"))
      colinSolver->property("num_augmented_trials") = num_augmented_trials;
    else // enforce lower bound of basic pattern
      total_pattern_size = basic_pattern_size;

    // Parameters relevant to all methods.

    const Real& solution_accuracy
      = probDescDB.get_real("method.solution_target");
    if (solution_accuracy > -DBL_MAX && colinSolver->has_property("sufficient_objective_value"))
      colinSolver->property("sufficient_objective_value") = solution_accuracy;

    // Buffer the constraint_penalty here.  In DAKOTA, this is a
    // property of the solver; in COLIN, it is a property of the
    // reformulated application.  Unfortunately, the probDescDB is
    // locked by the time we hit core_run().

    constraint_penalty = probDescDB.get_real("method.constraint_penalty");
    constant_penalty = probDescDB.get_bool("method.coliny.constant_penalty");

    const bool& show_misc_options
      = probDescDB.get_bool("method.coliny.show_misc_options");
    if (show_misc_options){
      Cout << "---------------------------SOLVER OPTIONS"
	   << "---------------------------\n";
      Cout << "The functionality to show solver options is temporarily unavailable.\n"
	   << "It will return in future releases.\n";
      //	colinSolver->options().write(Cout, true);
      //	colinSolver->options().write_parameters(Cout);
      Cout << "---------------------------SOLVER OPTIONS"
	   << "---------------------------" << endl;
    }

    const StringArray& misc_options
      = probDescDB.get_sa("method.coliny.misc_options");
    size_t num_mo = misc_options.size();
    for (size_t i=0; i<num_mo; i++) {
      string thisOption(misc_options[i]);
      size_t equalPos = thisOption.find("=",0);
      if (equalPos == string::npos)
	Cout << "Warning - COLINY ignoring option " << misc_options[i] << endl;
      else {
	string option(thisOption.substr(0, equalPos));
	string value(thisOption.substr(equalPos+1, thisOption.size()-1));
	if (colinSolver->has_property(option))
	  colinSolver->property(option) = colin::parse_data(value);
      }
    }
  }

  // Maximum concurrency is algorithm dependent.

  if (solverType == DIRECT)
    maxEvalConcurrency *= 2*numContinuousVars;
  else if (solverType == EA) {
    maxEvalConcurrency *= pop_size;
    if (colinSolver->has_property("population_size")) {
      colinSolver->property("population_size") = pop_size;
    }
  }
  else if (solverType == PS) {
    if (!pattern_basis.empty() && colinSolver->has_property("basis"))
      colinSolver->property("basis") = string(pattern_basis);
    maxEvalConcurrency *= total_pattern_size;
  }

  if (solverType != COBYLA) {
    if (colinSolver->has_property("max_function_evaluations_this_trial"))
      colinSolver->property("max_function_evaluations_this_trial") = maxFunctionEvals;
    if (colinSolver->has_property("max_iterations"))
      colinSolver->property("max_iterations") = maxIterations;
  }
  else {
    if (colinSolver->has_property("max-neval"))
      colinSolver->property("max-neval") = maxFunctionEvals;
  }
  if (colinSolver->has_property("function_value_tolerance"))
    colinSolver->property("function_value_tolerance") = convergenceTol;

  switch (outputLevel) {

  case DEBUG_OUTPUT:
      
    if (colinSolver->has_property("output_level"))
      colinSolver->property("output_level") = string("verbose");
    if (colinSolver->has_property("debug"))
      colinSolver->property("debug") = 10000;
    break;

  case VERBOSE_OUTPUT:
      
    if (colinSolver->has_property("output_level"))
      colinSolver->property("output_level") = string("summary");
    break;

  case NORMAL_OUTPUT:
  case QUIET_OUTPUT:

    if (colinSolver->has_property("output_level"))
      colinSolver->property("output_level") = string("normal");
    break;

  case SILENT_OUTPUT:
    if (colinSolver->has_property("output_level"))
      colinSolver->property("output_level") = string("none");

  }
}


/** Supplement Optimizer::post_run to first retrieve points from the
    Colin cache (or possibly the Dakota DB) and rank them.  When
    complete, this function will populate bestVariablesArray and
    bestResponsesArray with iterator-space data, that is, in the
    context of the solver, leaving any further untransformation to
    Optimizer. */
void COLINOptimizer::post_run(std::ostream& s)
{
  // Retreive the COLIN points (local cache) in the right Application context.  
  PointSet ps = colinSolver->get_final_points();

  // Maps to hold the sorted list of variables and responses.
  std::multimap<RealRealPair, Variables> variableSortMap;
  std::multimap<RealRealPair, Response> responseSortMap;

  // COLIN Cache iterator for primary lookup mechanism
  colin::Cache::iterator cache_it  = ps->begin(colinProblem.second);
  colin::Cache::iterator cache_end = ps->end();

  // Temporary data structures needed for sort.
  RealVector cdv;
  IntVector ddv;
  Variables tmpVariableHolder = iteratedModel.current_variables().copy();
  // BMA: Current convention across the code is that bestResponsesArray is
  // sized in the original problem space, even when reduction is active...
  Model& model_for_responses = localObjectiveRecast ?
    iteratedModel.subordinate_model() : iteratedModel;
  Response tmpResponseHolder = model_for_responses.current_response().copy();

  // One specification type for discrete variables is a set of values.
  // Get that list of values if the user provided one.  Also,
  // determine the size of those sets and the number of non-set
  // integer variables.

  const       BitArray& di_set_bits = iteratedModel.discrete_int_sets();
  const    IntSetArray& dsiv_values = iteratedModel.discrete_set_int_values();
  const   RealSetArray& dsrv_values = iteratedModel.discrete_set_real_values();
  const StringSetArray& dssv_values = iteratedModel.discrete_set_string_values();
  size_t i, j, dsi_cntr;

  // Iterate through points returned by COLIN.

  bool colin_cache_warn = false;
  for(i=0; cache_it != cache_end; ++i, ++cache_it) {

    // Extract the Colin response from the cache
    colin::AppResponse colinResponse = 
      cache_it->second.asResponse(colinProblem.second);

    // Get the mixed variables for the point from the cache.
    utilib::MixedIntVars miv;
    colinResponse.get_domain(miv);

    // Cast the continuous variables from COLIN to DAKOTA.

    TypeManager()->lexical_cast(miv.Real(), cdv);
    tmpVariableHolder.continuous_variables(cdv);

    // Cast the discrete variables from COLIN to a temporary vector.

    TypeManager()->lexical_cast(miv.Integer(), ddv);

    // Assign COLIN integer variables to DAKOTA discrete integer {range,set}
    // variables.  For discrete set types, COLIN is operating on the index
    // and we must map these back to the corresponding integer set values.
    for (j=0, dsi_cntr=0; j<numDiscreteIntVars; ++j) {
      if (di_set_bits[j]) { // this active discrete int var is a set type
	int dakota_value = set_index_to_value(ddv[j], dsiv_values[dsi_cntr]);
	tmpVariableHolder.discrete_int_variable(dakota_value, j);
	++dsi_cntr;
      }
      else                  // this active discrete int var is a range type
	tmpVariableHolder.discrete_int_variable(ddv[j], j);
    }

    // Do the same for the active discrete real vars, which are all set types

    for (j=0; j<numDiscreteRealVars; ++j) {
      Real dakota_value
	= set_index_to_value(ddv[j+numDiscreteIntVars], dsrv_values[j]);
      tmpVariableHolder.discrete_real_variable(dakota_value, j);
    }

    // And again for the active discrete string vars, which are all set types

    for (j=0; j<numDiscreteStringVars; ++j) {
      String dakota_value
	= set_index_to_value(ddv[j+numDiscreteIntVars+numDiscreteRealVars],
	    dssv_values[j]);
      tmpVariableHolder.discrete_string_variable(dakota_value, j);
    }

    // BMA: cautiously optimistic implementation, assuming we can
    // reliably get functions and constraints from the Colin cache...

    // Get the objective and constraint values associated with the
    // point.  Fn and constraint status: true if okay (needed/present
    // or unneeded).  Using two bools in case we need to later rely on
    // Dakota for one, but not the other.
    std::pair<bool, bool> cache_status = 
      colin_cache_lookup(colinResponse, tmpResponseHolder);

    bool cache_success = (cache_status.first && cache_status.second);

    if (!cache_success) {
      if (!cache_status.first)
	Cerr << "\nWarning: Couldn't get objectives from Colin cache.";
      if (!cache_status.second)
	Cerr << "\nWarning: Couldn't get constraints from Colin cache.";
      Cerr << "\nAll function values from COLIN:";
      write_data(Cout, tmpResponseHolder.function_values());
      Cerr << std::endl;
    }

    // BMA TODO: incorporate constraint tolerance, possibly via
    // elevating SurrBasedMinimizer::constraint_violation() to Minimizer
    double constraintViolation = constraint_violation(tmpResponseHolder);

    // For feasible points, compute (sum of) objectives.
    const RealVector& fn_vals = tmpResponseHolder.function_values();

    // Use the objective "reduction" even in the single objective case
    // as it will get the sense (min/max) correct
    double obj_fn_metric = 
      objective(fn_vals, numObjectiveFns,
		iteratedModel.primary_response_fn_sense(),
		iteratedModel.primary_response_fn_weights());

    RealRealPair metrics(constraintViolation, obj_fn_metric);

    if (variableSortMap.size() < numFinalSolutions) {
      // If there's still room in the map, insert the point.

      variableSortMap.insert(std::make_pair(metrics, tmpVariableHolder.copy()));
      responseSortMap.insert(std::make_pair(metrics, tmpResponseHolder.copy()));
    }
    else {
      // Otherwise, boot out the worst point first and then insert
      // the new point.

      std::multimap<RealRealPair, Variables>::iterator var_worst_it = 
	--variableSortMap.end();
      std::multimap<RealRealPair, Response>::iterator resp_worst_it = 
	--responseSortMap.end();

      if(metrics < var_worst_it->first) {
	variableSortMap.erase(var_worst_it);
	variableSortMap.insert(std::make_pair(metrics, tmpVariableHolder.copy()));
      }
      if(metrics < resp_worst_it->first) {
	responseSortMap.erase(resp_worst_it);
	responseSortMap.insert(std::make_pair(metrics, tmpResponseHolder.copy()));
      }
    }
  }

  if (colin_cache_warn)
    Cerr << "Warning: Lookup of some COLIN final points failed.\n         If you"
	 << " have constraints, some points returned may be infeasible." << endl;

  // Make sure bestVariablesArray and bestResponseArray are the right size.
  resize_best_vars_array(numFinalSolutions);
  resize_best_resp_array(numFinalSolutions);

  // Iterate through the map and populate bestVariablesArray and
  // bestResponseArray with sorted points and corresponding responses.

  std::multimap<RealRealPair, Variables>::const_iterator var_best_it = 
    variableSortMap.begin(); 
  const std::multimap<RealRealPair, Variables>::const_iterator var_best_end = 
    variableSortMap.end(); 
  std::multimap<RealRealPair, Response>::const_iterator resp_best_it = 
    responseSortMap.begin(); 
  ResponseArray::size_type index = 0;
  for( ; var_best_it != var_best_end; ++var_best_it, ++resp_best_it, ++index) {
    bestVariablesArray[index] = var_best_it->second.copy();
    if (!localObjectiveRecast) {  // else local_recast_retrieve
      bestResponseArray[index] = resp_best_it->second.copy();
    }
  }

  ps->clear();

  if (solverType == COBYLA) {
    Cout << "********************************************************" << endl;
    Cout << "WARNING: The implementation of COBYLA is such that the"   << endl;
    Cout << "best function value is not always returned to Dakota"     << endl;
    Cout << "for reporting.  The user is advised to look through the"  << endl;
    Cout << "Dakota output to confirm what the best function value"    << endl;
    Cout << "and corresponding parameter values are."                  << endl;
    Cout << "********************************************************" << endl;
    Cout << endl;
  }

  // New design relies on Optimizer to perform transformation from
  // iterated to user space as needed
  Optimizer::post_run(s);
}



/** Encapsulated Colin Cache response extraction, which will
    ultimately become the default lookup.  Might want to return
    separate vectors of function values and constraints for use in the
    sort, but not for now (least change).  Return true if not needed or
    successful lookup. */
std::pair<bool, bool> COLINOptimizer::
colin_cache_lookup(const colin::AppResponse& colinResponse,
		   Response& tmpResponseHolder)
{
  bool fns_status = colinResponse.is_computed(mf_info);
  if (fns_status) {
    RealVector fn_vals(numObjectiveFns);    
    colinResponse.get(mf_info, fn_vals);
    for (size_t i = 0; i < numObjectiveFns; ++i)
      tmpResponseHolder.function_value(fn_vals[i], i);
  }

  bool cons_status = true;
  if (numNonlinearConstraints > 0) {
    cons_status = colinResponse.is_computed(nlcf_info);
    if (cons_status) {
      RealVector cons_vals(numNonlinearConstraints);
      colinResponse.get(nlcf_info, cons_vals);
      for (size_t i = 0; i < numNonlinearConstraints; ++i)
	tmpResponseHolder.function_value(cons_vals[i], numObjectiveFns + i);
    }
  }

  return std::make_pair(fns_status, cons_status);
}


/** BMA TODO: incorporate constraint tolerance, possibly via elevating
    SurrBasedMinimizer::constraint_violation().  Always use
    iteratedModel to get the constraints; they are in the right
    space. */
double COLINOptimizer::constraint_violation(const Response& tmpResponseHolder)
{
  // Need number of constraints and bounds so we can determine
  // constraint violation.

  size_t num_nln_ineq = iteratedModel.num_nonlinear_ineq_constraints(),
         num_nln_eq   = iteratedModel.num_nonlinear_eq_constraints();
  const RealVector& nln_ineq_lwr_bnds
    = iteratedModel.nonlinear_ineq_constraint_lower_bounds();
  const RealVector& nln_ineq_upr_bnds
    = iteratedModel.nonlinear_ineq_constraint_upper_bounds();
  const RealVector& nln_eq_targets
    = iteratedModel.nonlinear_eq_constraint_targets();

  // Compute constraint violation for nonlinear inequality and
  // equality constraints using sum of squares of component-wise
  // differences.  Only done for infeasible points.

  double cons_violation = 0.0;
  const RealVector& fn_vals = tmpResponseHolder.function_values();

  for(size_t j=0; j<num_nln_ineq; j++) {
    if (fn_vals[j+numIterPrimaryFns] > nln_ineq_upr_bnds[j])
      cons_violation += std::pow(fn_vals[j+numIterPrimaryFns]-nln_ineq_upr_bnds[j],2);
    else if (fn_vals[j+numIterPrimaryFns] < nln_ineq_lwr_bnds[j])
      cons_violation += std::pow(nln_ineq_lwr_bnds[j]-fn_vals[j+numIterPrimaryFns],2);
  }
  for (size_t j=0; j<num_nln_eq; j++) {
    if (std::fabs(fn_vals[j+numIterPrimaryFns+num_nln_ineq] - nln_eq_targets[j]) > 0.)
      cons_violation += std::pow(fn_vals[j+numIterPrimaryFns+num_nln_ineq]-nln_eq_targets[j], 2);
  }

  return cons_violation;
}


} // namespace Dakota
