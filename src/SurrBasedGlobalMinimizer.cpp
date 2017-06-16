/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SurrBasedGlobalMinimizer
//- Description: Implementation code for the SurrBasedGlobalMinimizer class
//- Owner:       John Eddy, Laura Swiler
//- Checked by:

#include "SurrBasedGlobalMinimizer.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "ParamResponsePair.hpp"
#include "DakotaGraphics.hpp"
#include "dakota_system_defs.hpp"
#include "DakotaInterface.hpp"
#include "DakotaApproximation.hpp"
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <iostream>
#include <string>

static const char rcsId[]="@(#) $Id: SurrBasedGlobalMinimizer.cpp 7031 2010-10-22 16:23:52Z mseldre $";

namespace Dakota {


SurrBasedGlobalMinimizer::
SurrBasedGlobalMinimizer(ProblemDescDB& problem_db, Model& model):
  SurrBasedMinimizer(problem_db, model),
  replacePoints(probDescDB.get_bool("method.sbg.replace_points"))
{
  // Verify that iteratedModel is a surrogate model so that
  // approximation-related functions are defined.
  if (iteratedModel.model_type() != "surrogate") {
    Cerr << "Error: SurrBasedGlobalMinimizer::iteratedModel must be a "
	 << "surrogate model." << std::endl;
    abort_handler(-1);
  }

  // While this copy will be replaced in best update, initialize here
  // since relied on in Minimizer::initialize_run when a sub-iterator
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
    probDescDB.set_db_method_node(approx_method_ptr); // method only
    // sub-problem minimizer will use shallow copy of iteratedModel
    // (from problem_db.get_model())
    approxSubProbMinimizer = probDescDB.get_iterator();//(iteratedModel);
    // suppress DB ctor default and don't output summary info
    approxSubProbMinimizer.summary_output(false);
    // verify approx method's modelPointer is empty or consistent
    const String& am_model_ptr = probDescDB.get_string("method.model_pointer");
    if (!am_model_ptr.empty() && am_model_ptr != model_ptr)
      Cerr << "Warning: SBO approx_method_pointer specification includes an\n"
	   << "         inconsistent model_pointer that will be ignored."
	   << std::endl;
    probDescDB.set_db_method_node(method_index); // restore method only
  }
  else if (!approx_method_name.empty())
    // Approach 2: instantiate on-the-fly w/o method spec support
    approxSubProbMinimizer
      = probDescDB.get_iterator(approx_method_name, iteratedModel);
}


SurrBasedGlobalMinimizer::~SurrBasedGlobalMinimizer()
{ }


void SurrBasedGlobalMinimizer::core_run()
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

  bool returns_multipoint = approxSubProbMinimizer.returns_multiple_points(),
       accepts_multipoint = approxSubProbMinimizer.accepts_multiple_points(),
       truth_asynch_flag  = truth_model.asynch_flag();

  // This flag will be used to indicate when we are finished iterating.  An
  // iteration is a solution of the approximate model followed by an update
  // of the surrogate.
  while (globalIterCount < maxIterations) {

    // Test how well the surrogate matches up with the truth model.  For this
    // test, we currently use R-squared as a measure of goodness of fit,
    // although we can easily generalize it.  Also, currently we state that if
    // R-squared is < 0.5 or > 1.1 (R-squared can be greater than one in
    // abnormal cases for surrogates that are not polynomial regression models),
    // we stop the surrogate-based global minimization process because it will
    // not work with such an inaccurate model.
    std::vector<Approximation>& approxs
      = approx_model.derived_interface().approximations();
    std::vector<Approximation>::iterator it;
    for (it=approxs.begin(); it!=approxs.end(); ++it) {
      if (it->diagnostics_available()) {

	// Start the check with the r-squared value.
	Real r2_diagnostic = it->diagnostic("rsquared");
	if (outputLevel > NORMAL_OUTPUT)
	  Cout << "R-squared = " << r2_diagnostic << std::endl;

	// If outside of tolerable range, report and abort.
	// TODO: report function index along with diagnostic.
	if (r2_diagnostic < 0.5 || r2_diagnostic > 1.1) {
	  Cerr << "Surrogate approximation is not accurate enough for " 
	       << "the surrogate-based global minimization.\n" 
	       << "The minimization has quit before the requested number " 
	       << "of iterations due to poor surrogate fit." << std::endl;
	  abort_handler(-1);
	}

	// Add some additional diagnostics?
      }
    }

    // use the iterator to solve the approximate subproblem.  On the first
    // iteration, the surrogate is built using only the original truth
    // samples.  At each subsequent iteration, the surrogate includes
    // additional truth samples from validation of subproblem solutions.
    ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
    approxSubProbMinimizer.run(pl_iter);

    // Get the results from the iterator execution.
    VariablesArray vars_results;
    if (returns_multipoint)
      vars_results = approxSubProbMinimizer.variables_array_results();
    else
      vars_results.push_back(approxSubProbMinimizer.variables_results());
    size_t i, num_results = vars_results.size();

    // Variable/response results were generated using the current approximate
    // model.  For appending to the current approximate model, we must evaluate
    // the variable results with the truth model.
    iteratedModel.component_parallel_mode(TRUTH_MODEL);
    IntResponseMap truth_resp_results;
    for (i=0; i<num_results; i++) {
      // set the current values of the active variables in the truth model
      truth_model.active_variables(vars_results[i]);

      // request the evaluation in synchronous or asyncronous mode.
      if (truth_asynch_flag)
        truth_model.evaluate_nowait();
      else {
        truth_model.evaluate();
	truth_resp_results[truth_model.evaluation_id()]
	  = truth_model.current_response().copy();
      }
    }
    // If we did our evaluations asynchronously, use synchronize to block
    // until all the results are available and then store these responses.
    if (truth_asynch_flag)
      truth_resp_results = truth_model.synchronize();

    // Beyond this point, we will want to know if this is the last iteration.
    // We will use this information to prevent updating of the surrogate since
    // it will not be used again.
    bool last_iter = ++globalIterCount >= maxIterations;

    if (outputLevel > QUIET_OUTPUT) {
      // In here we want to write the truth values into a simple tab delimited
      // file so that we can easily compare them with the surrogate values of
      // the points returned by the iterator.
      std::string ofname("finaldatatruth");
      ofname += boost::lexical_cast<std::string>(globalIterCount);
      ofname += ".dat";
      std::ofstream ofile(ofname.c_str());
      ofile.precision(12);
      IntRespMCIter it = truth_resp_results.begin();
      for (i=0; i<num_results; ++i, ++it) {
	const RealVector&  c_vars = vars_results[i].continuous_variables();
	const IntVector&  di_vars = vars_results[i].discrete_int_variables();
	const RealVector& dr_vars = vars_results[i].discrete_real_variables();
	const RealVector& fn_vals = it->second.function_values();
	std::copy(c_vars.values(), c_vars.values() + c_vars.length(),
	          std::ostream_iterator<Real>(ofile,"\t"));
	std::copy(di_vars.values(), di_vars.values() + di_vars.length(),
	          std::ostream_iterator<int>(ofile,"\t"));
	std::copy(dr_vars.values(), dr_vars.values() + dr_vars.length(),
	          std::ostream_iterator<Real>(ofile,"\t"));
	std::copy(fn_vals.values(), fn_vals.values() + fn_vals.length(),
	          std::ostream_iterator<Real>(ofile,"\t"));
	ofile << '\n';
      }
      ofile.close();
    }

    // See if we are done  
    if (last_iter) { // catalogue final results in best{Variables,Response}Array
      if (returns_multipoint) {
	bestVariablesArray = vars_results;
	copy_data(truth_resp_results, bestResponseArray);
      }
      else {
	bestVariablesArray.front().active_variables(vars_results.front());
	bestResponseArray.front().function_values(
	  approxSubProbMinimizer.response_results().function_values());
      }
    }
    else {
      // restore state prior to previous append_approximation()
      if (replacePoints && globalIterCount > 1)
	approx_model.pop_approximation(false);// don't store SDP set; no restore
      // update the data set and rebuild the approximation
      approx_model.append_approximation(vars_results, truth_resp_results, true);

      // pass iterator's final vars for use as next set of initial points
      if (accepts_multipoint)
	approxSubProbMinimizer.initial_points(vars_results);
      else
	approx_model.active_variables(vars_results.front());
    }
  }
}

} // namespace Dakota
