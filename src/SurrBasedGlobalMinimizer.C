/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SurrBasedGlobalMinimizer
//- Description: Implementation code for the SurrBasedGlobalMinimizer class
//- Owner:       John Eddy, Laura Swiler
//- Checked by:

#include "SurrBasedGlobalMinimizer.H"
#include "ProblemDescDB.H"
#include "ParallelLibrary.H"
#include "ParamResponsePair.H"
#include "DakotaGraphics.H"
#include "system_defs.h"
#include "DakotaInterface.H"
#include "DakotaApproximation.H"
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <iostream>
#include <string>

static const char rcsId[]="@(#) $Id: SurrBasedGlobalMinimizer.C 7041 2010-10-25 18:28:51Z briadam $";


using namespace std;

namespace Dakota {

// define special values for componentParallelMode
//#define SURROGATE_MODEL 1
#define TRUTH_MODEL 2


SurrBasedGlobalMinimizer::SurrBasedGlobalMinimizer(Model& model):
  SurrBasedMinimizer(model),
  replacePoints(probDescDB.get_bool("method.sbg.replace_points"))
{
  // Verify that iteratedModel is a surrogate model so that
  // approximation-related functions are defined.
  if (iteratedModel.model_type() != "surrogate") {
    Cerr << "Error: SurrBasedGlobalMinimizer::iteratedModel must be a "
	 << "surrogate model." << endl;
    abort_handler(-1);
  }

  // BMA temp hack: re-enabled this until Mike can review, since best
  // solution processing currently relies on at least one point in
  // this array:

  // Deferred until best update, since this object replaced if
  // approxSubProbMinimizer uses same view
  bestVariablesArray.push_back(
    iteratedModel.truth_model().current_variables().copy());

  // Instantiate the approximate sub-problem minimizer
  const String& approx_method_ptr
    = probDescDB.get_string("method.sub_method_pointer");
  const String& approx_method_name
    = probDescDB.get_string("method.sub_method_name");
  if (!approx_method_ptr.empty()) {
    // Approach 1: method spec support for approxSubProbMinimizer
    const String& model_ptr = probDescDB.get_string("method.model_pointer");
    size_t method_index = probDescDB.get_db_method_node(); // for restoration
    probDescDB.set_db_method_node(approx_method_ptr); // set method only
    approxSubProbMinimizer = probDescDB.get_iterator(iteratedModel);
    // suppress DB ctor default and don't output summary info
    approxSubProbMinimizer.summary_output(false);
    // verify approx method's modelPointer is empty or consistent
    const String& am_model_ptr = probDescDB.get_string("method.model_pointer");
    if (!am_model_ptr.empty() && am_model_ptr != model_ptr)
      Cerr << "Warning: SBO approx_method_pointer specification include an\n"
	   << "         inconsistent model_pointer that will be ignored."<<endl;
    probDescDB.set_db_method_node(method_index); // restore method only
  }
  else if (!approx_method_name.empty())
    // Approach 2: instantiate on-the-fly w/o method spec support
    approxSubProbMinimizer = Iterator(approx_method_name, iteratedModel);

  // Allocate comms for parallel.  For DataFitSurrModel, concurrency
  // is from daceIterator evals (global) or numerical derivatives
  // (local/multipt) on actualModel.  For HierarchSurrModel,
  // concurrency is from approxSubProbMinimizer on lowFidInterface.
  iteratedModel.init_communicators(
    approxSubProbMinimizer.maximum_concurrency());
}


SurrBasedGlobalMinimizer::~SurrBasedGlobalMinimizer()
{
  // Virtual destructor handles referenceCount at Iterator level.

  // free communicators for iteratedModel
  iteratedModel.free_communicators(
    approxSubProbMinimizer.maximum_concurrency());
}


void SurrBasedGlobalMinimizer::minimize_surrogates()
{
  // Extract subIterator/subModel(s) from the SurrogateModel
  Model&    truth_model   = iteratedModel.truth_model();
  Model&    approx_model  = iteratedModel.surrogate_model();
  Iterator& dace_iterator = iteratedModel.subordinate_iterator();
  
  // This flag controls the method by which we introduce new results data
  // into the surrogate for updating.  Right now, there are two methods
  // supported.  The first is to create each subsequent surrogate using every
  // truth point evaluated in all previous iterations.  This is identified with
  // the string "append".  The second is to rebuild the surrogate at each
  // iteration using the original truth samples and the truth solutions from
  // the previous iteration only.  This is identified with the string "replace".

  // Update DACE settings for global approximations.  Check that dace_iterator
  // is defined (a dace_iterator specification is not required when the data
  // samples are read in from a file rather than obtained from sampling).
  if (!dace_iterator.is_null()) {
    short asv_val = 1;
    ActiveSet dace_set = truth_model.current_response().active_set(); // copy
    dace_set.request_values(asv_val);
    dace_iterator.active_set(dace_set);
  }

  // get data points using sampling, file read, or whatever.
  iteratedModel.build_approximation();

  // The points obtained using sampling will not change from here on out nor
  // will the arrays that store them.  We will keep them here for use in
  // rebuilding the surrogate if using "replace" for example.

  // Make shallow copies of the variables and responses used to
  // construct the initial surrogate, for use in "replace" strategy.
  // These must come from the surrogate model since they may include
  // daceIterator points as well as reused points.
  VariablesArray initial_vars = dace_iterator.all_variables();//approx_model.build_variables(); // temp hack
  ResponseArray  initial_resp = dace_iterator.all_responses();//approx_model.build_responses(); // temp hack
  // *** TO DO ***: use increment and restore

  bool returns_multipoint = approxSubProbMinimizer.returns_multiple_points(),
       accepts_multipoint = approxSubProbMinimizer.accepts_multiple_points();

  // This flag will be used to indicate when we are finished iterating.  An
  // iteration is a solution of the approximate model followed by an update
  // of the surrogate.
  while (sbIterNum < maxIterations) {

    // use the iterator to solve the approximate model.  At the first iteration,
    // this is the surrogate built using only the original truth samples.  At
    // each subsequent iteration, the surrogate includes additional truth
    // samples depending on the method of adding points
    // This iterator constructed from DB, but summary output suppressed:
    approxSubProbMinimizer.run_iterator(Cout);

    // Get the results from the iterator execution.
    VariablesArray vars_array_results;
    if (returns_multipoint)
      vars_array_results = approxSubProbMinimizer.variables_array_results();
    else
      vars_array_results.push_back(approxSubProbMinimizer.variables_results());
    size_t i, num_results = vars_array_results.size();

    ResponseArray resp_array_results;
    if (!truth_model.asynch_flag())
      resp_array_results.resize(num_results);

    // These results were generated using the current approximate model.
    // They are the ones that we wish to append to the approximate
    // model which requires that we evaluate them with the truth model.
    iteratedModel.component_parallel_mode(TRUTH_MODEL);

    // If replacing, there is no sense in updating the approximation if we are
    // on the first iteration since the approximation on the first iteration
    // is built using the sampled points only.
    if (sbIterNum > 0 && replacePoints)
      // use update approximation here to replace the current approximation
      // completely with the original.  To the reinstalled original, we will add
      // new truth points from the iterator.
      approx_model.update_approximation(initial_vars, initial_resp, false);

    // beyond this point, we will want to know if this is the last iteration.
    // We will use this information to prevent updating of the surrogate since
    // it will not be used again.  We don't simply exit here because we want
    // our truth values computed.
    bool lastIter = ++sbIterNum >= maxIterations;

    for (i=0; i<num_results; i++) {
      // first, set the current values of the variables in the model.
      const Variables& vars_results_i = vars_array_results[i];
      truth_model.continuous_variables(vars_results_i.continuous_variables());

      // now request the evaluation in synchronous or asyncronous mode.
      if (truth_model.asynch_flag())
        truth_model.asynch_compute_response();
      else {
        // in synchronous mode, we can add the new responses to our
	// approximation right away but be sure to pass false as the rebuild
	// flag until we're on the last point.  Also note that we don't update
	// the surrogate if this is the last iteration.
        truth_model.compute_response();
        if (!lastIter)
	  approx_model.append_approximation(vars_results_i,
	    truth_model.current_response(), i == num_results-1 );
	resp_array_results[i] = truth_model.current_response().copy();
      }
    }

    // If we did our evaluations asynchronously, we did not yet append
    // the results (because they were not available).  We need to do so
    // now.  The call to truth_model.synchronize causes the program to block
    // until all the results are available.  We can then append the responses.
    if (truth_model.asynch_flag()) {
      copy_data(truth_model.synchronize(), resp_array_results); // discards keys
      // Again, we do not update the approximation on the last iteration.
      if(!lastIter)
        approx_model.append_approximation(vars_array_results,
					  resp_array_results, true);
    }

    if (outputLevel > QUIET_OUTPUT) {
      // In here we want to write the truth values into a simple tab delimited
      // file so that we can easily compare them with the surrogate values of
      // the points returned by the iterator.
      std::string ofname("finaldatatruth");
      ofname += boost::lexical_cast<std::string>(sbIterNum);
      ofname += ".dat";
      std::ofstream ofile(ofname.c_str());
      ofile.precision(12);
      for (i=0; i<num_results; i++) {
	const RealVector& c_vars
	  = vars_array_results[i].continuous_variables();
	const RealVector& fn_vals
	  = resp_array_results[i].function_values();
	std::copy(c_vars.values(), c_vars.values() + c_vars.length(),
	          std::ostream_iterator<double>(ofile,"\t"));
	std::copy(fn_vals.values(), fn_vals.values() + fn_vals.length(),
	          std::ostream_iterator<double>(ofile,"\t"));
	ofile << '\n';
      }
      ofile.close();
    }

    // See if we are done. There are two parts of this logic. 
    // The next section tests how well the surrogate matches up with the truth
    // model (assuming that if the surrogate matches the truth, then
    // the approximate solution is the true solution).  
    // For this test, we are currently using R-squared as a measure of goodness 
    // of fit, although we can easily generalize it.  Also, currently we state 
    // that if R-squared is less than 0.7 or greater than 1.01 (R-squared can be
    // greater than one in abnormal cases for surrogates that are not polynomial
    // regression models), we stop the surrogate-based global minimization
    // process because it will not work with such an inaccurate model.  If the
    // surrogates are "good", the section after this tests if we are at the last
    // iteration.    

    // Store the approximation list for repeated use and to avoid repeated calls
    // to a chain of virtual functions.
    std::vector<Approximation>& approxs
      = approx_model.interface().approximations();

    // Iterate the approximations explicitly instead of by index.  That way,
    // we don't have to care if some functions are approximated and some are
    // not, we can just visit the approximations directly.
    for(std::vector<Approximation>::iterator it = approxs.begin();
        it!=approxs.end(); ++it) {
        // Store the current approximation for ease of syntax.
        Approximation& approx = *it;

        // If the current approximation does not provide diagnostics, then
        // skip this check.
        if(!approx.diagnostics_available()) continue;

        // Start the check with the r-squared value.  Store it for reporting
        // and testing.
        Real this_diagnostic = approx.get_diagnostic("rsquared");

        // Report the found value.
        if (outputLevel > NORMAL_OUTPUT) {
	  Cout << "R-squared = " << this_diagnostic << endl;
	}

        // If outside of tolerable range, report and abort.  TODO: find some way
        // to determine which function this diagnostic is for so that info can
        // be reported as well.
        if ((this_diagnostic < 0.5) || (this_diagnostic > 1.1))
        {
   	  Cerr << "Surrogate approximation is not accurate enough for " 
               << "surrogate-based global optimization strategy." 
	       << "The strategy has quit before the requested number " 
               << "of iterations due to poor surrogate fit." << endl;
	       abort_handler(-1);
        }

        // Add some additional diagnostics?
    }

    // See if we are done.  If we pass the diagnostic test, we perform a fixed
    // number of iterations in which case this is the place where we would stop.
  
    if (lastIter) { // catalogue best solutions
      // ------------------------------------------------------------------
      // catalogue multiple results in bestVariablesArray/bestResponseArray
      // ------------------------------------------------------------------
      if (returns_multipoint) {
	if (vars_array_results.front().view() !=
	    bestVariablesArray.front().view()) {
	  // preserve truth_model variables view
	  bestVariablesArray.resize(num_results);
	  for (i=0; i<num_results; ++i) {
	    bestVariablesArray[i] = truth_model.current_variables().copy();
	    bestVariablesArray[i].all_continuous_variables(
	      vars_array_results[i].all_continuous_variables());
	  }
	}
	else
	  bestVariablesArray = vars_array_results;
	bestResponseArray = resp_array_results;
      }
      else {
	// -----------------------------------------------------
	// catalogue single result in bestVariables/bestResponse
	// -----------------------------------------------------
	const Variables& vars_results
	  = approxSubProbMinimizer.variables_results();
	if (vars_results.view() != bestVariablesArray.front().view()) {
	  // preserve truth_model variables view
	  bestVariablesArray.push_back(truth_model.current_variables().copy());
	  bestVariablesArray.front().all_continuous_variables(
	    vars_results.all_continuous_variables());
	}
	else
	  bestVariablesArray.push_back(vars_results);
	bestResponseArray.front() = approxSubProbMinimizer.response_results();
      }
    }
    else { // pass iterator's final vars for use as next set of initial points
      if (accepts_multipoint)
	approxSubProbMinimizer.initial_points(vars_array_results);
      else
	approx_model.active_variables(vars_array_results[0]);
    }
  }
}

} // namespace Dakota
