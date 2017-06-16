/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Analyzer
//- Description: Implementation code for the Analyzer class
//- Owner:       Mike Eldred
//- Checked by:

#include <stdexcept>
#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaAnalyzer.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "IteratorScheduler.hpp"
#include "PRPMultiIndex.hpp"

static const char rcsId[]="@(#) $Id: DakotaAnalyzer.cpp 7035 2010-10-22 21:45:39Z mseldre $";

//#define DEBUG

namespace Dakota {

extern PRPCache data_pairs;


Analyzer::Analyzer(ProblemDescDB& problem_db, Model& model):
  Iterator(BaseConstructor(), problem_db), compactMode(true),
  numObjFns(0), numLSqTerms(0), // default: no best data tracking
  writePrecision(probDescDB.get_int("environment.output_precision"))
{
  // set_db_list_nodes() is set by a higher context
  iteratedModel = model;
  update_from_model(iteratedModel); // variable/response counts & checks

  if (model.primary_fn_type() == OBJECTIVE_FNS)
    numObjFns = model.num_primary_fns();
  else if (model.primary_fn_type() == CALIB_TERMS)
    numLSqTerms = model.num_primary_fns();
  else if (model.primary_fn_type() != GENERIC_FNS) {
    Cerr << "\nError: Unknown primary function type in Analyzer." << std::endl;
    abort_handler(-1);
  }
  
  if (probDescDB.get_bool("method.variance_based_decomp")) 
    vbdDropTol = probDescDB.get_real("method.vbd_drop_tolerance");

  if (!numFinalSolutions)  // default is zero
    numFinalSolutions = 1; // iterator-specific default assignment
}


Analyzer::Analyzer(unsigned short method_name, Model& model):
  Iterator(NoDBBaseConstructor(), method_name, model), compactMode(true),
  numObjFns(0), numLSqTerms(0), // default: no best data tracking
  writePrecision(0)
{
  update_from_model(iteratedModel); // variable/response counts & checks
}


Analyzer::Analyzer(unsigned short method_name):
  Iterator(NoDBBaseConstructor(), method_name), compactMode(true),
  numObjFns(0), numLSqTerms(0), // default: no best data tracking
  writePrecision(0)
{ }


bool Analyzer::resize()
{
  bool parent_reinit_comms = Iterator::resize();

  numContinuousVars = iteratedModel.cv();
  numDiscreteIntVars = iteratedModel.div();
  numDiscreteStringVars = iteratedModel.dsv();
  numDiscreteRealVars = iteratedModel.drv();
  numFunctions = iteratedModel.num_functions();

  return parent_reinit_comms;
}

void Analyzer::update_from_model(const Model& model)
{
  Iterator::update_from_model(model);

  numContinuousVars     = model.cv();  numDiscreteIntVars  = model.div();
  numDiscreteStringVars = model.dsv(); numDiscreteRealVars = model.drv();
  numFunctions          = model.num_functions();

  bool err_flag = false;
  // Check for correct bit associated within methodName
  if ( !(methodName & ANALYZER_BIT) ) {
    Cerr << "\nError: analyzer bit not activated for method instantiation "
	 << "(case " << methodName << ") within Analyzer branch." << std::endl;
    err_flag = true;
  }
  // Check for active design variables and discrete variable support
  if (methodName == CENTERED_PARAMETER_STUDY ||
      methodName == LIST_PARAMETER_STUDY     ||
      methodName == MULTIDIM_PARAMETER_STUDY ||
      methodName == VECTOR_PARAMETER_STUDY   || methodName == RANDOM_SAMPLING ||
      methodName == GLOBAL_INTERVAL_EST      || methodName == GLOBAL_EVIDENCE ||
      methodName == ADAPTIVE_SAMPLING ) {
    if (!numContinuousVars && !numDiscreteIntVars && !numDiscreteStringVars &&
	!numDiscreteRealVars) {
      Cerr << "\nError: " << method_enum_to_string(methodName)
	   << " requires active variables." << std::endl;
      err_flag = true;
    }
  }
  else { // methods supporting only continuous design variables
    if (!numContinuousVars) {
      Cerr << "\nError: " << method_enum_to_string(methodName)
	   << " requires active continuous variables." << std::endl;
      err_flag = true;
    }
    if (numDiscreteIntVars || numDiscreteStringVars || numDiscreteRealVars)
      Cerr << "\nWarning: active discrete variables ignored by "
	   << method_enum_to_string(methodName) << std::endl;
  }
  // Check for response functions
  if ( numFunctions <= 0 ) {
    Cerr << "\nError: number of response functions must be greater than zero."
	 << std::endl;
    err_flag = true;
  }

  if (err_flag)
    abort_handler(-1);
}


void Analyzer::initialize_run()
{
  // Verify that iteratedModel is not null (default ctor and some
  // NoDBBaseConstructor ctors leave iteratedModel uninitialized).
  if (!iteratedModel.is_null()) {
    // update context data that is outside scope of local DB specifications.
    // This is needed for reused objects.
    //iteratedModel.db_scope_reset(); // TO DO: need better name?

    // This is to catch un-initialized models used by local iterators that
    // are not called through IteratorScheduler::run_iterator().  Within a
    // recursion, it will correspond to the first initialize_run() with an
    // uninitialized mapping, such as the outer-iterator on the first pass
    // of a recursion.  On subsequent passes, it may correspond to the inner
    // iterator.  The Iterator scope should not matter for the iteratedModel
    // mapping initialize/finalize.
    if (!iteratedModel.mapping_initialized()) {
      ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator();
      bool var_size_changed = iteratedModel.initialize_mapping(pl_iter);
      if (var_size_changed)
        /*bool reinit_comms =*/ resize(); // Ignore return value
    }

    // Do not reset the evaluation reference for sub-iterators
    // (previously managed via presence/absence of ostream)
    //if (!subIteratorFlag)
    if (summaryOutputFlag)
      iteratedModel.set_evaluation_reference();
  }
}


void Analyzer::pre_run()
{ bestVarsRespMap.clear(); }


void Analyzer::post_run(std::ostream& s)
{
  if (summaryOutputFlag) {
    // Print the function evaluation summary for all Iterators
    if (!iteratedModel.is_null())
      iteratedModel.print_evaluation_summary(s); // full hdr, relative counts

    // The remaining final results output varies by iterator branch
    print_results(s);
  }

  resultsDB.write_databases();
}


void Analyzer::finalize_run()
{
  // Finalize an initialized mapping.  This will correspond to the first
  // finalize_run() with an uninitialized mapping, such as the inner-iterator
  // in a recursion.
  if (iteratedModel.mapping_initialized()) {
    bool var_size_changed = iteratedModel.finalize_mapping();
    if (var_size_changed)
      /*bool reinit_comms =*/ resize(); // Ignore return value
  }

  Iterator::finalize_run(); // included for completeness
}


/** Convenience function for derived classes with sets of function
    evaluations to perform (e.g., NonDSampling, DDACEDesignCompExp,
    FSUDesignCompExp, ParamStudy). */
void Analyzer::
evaluate_parameter_sets(Model& model, bool log_resp_flag, bool log_best_flag)
{
  // This function does not need an iteratorRep fwd because it is a
  // protected fn only called by letter classes.

  // allVariables or allSamples defines the set of fn evals to be performed
  size_t i, num_evals
    = (compactMode) ? allSamples.numCols() : allVariables.size();
  bool header_flag = (allHeaders.size() == num_evals);
  bool asynch_flag = model.asynch_flag();

  if (!asynch_flag && log_resp_flag) allResponses.clear();

  // Loop over parameter sets and compute responses.  Collect data
  // and track best evaluations based on flags.
  for (i=0; i<num_evals; i++) {
    // output the evaluation header (if present)
    if (header_flag)
      Cout << allHeaders[i];

    if (compactMode)
      update_model_from_sample(model, allSamples[i]);
    else
      update_model_from_variables(model, allVariables[i]);

    // compute the response
    if (asynch_flag)
      model.evaluate_nowait(activeSet);
    else {
      model.evaluate(activeSet);
      const Response& resp = model.current_response();
      int eval_id = model.evaluation_id();
      if (log_best_flag) // update best variables/response
        update_best(model.current_variables(), eval_id, resp);
      if (log_resp_flag) // log response data
        allResponses[eval_id] = resp.copy();
    }
  }

  // synchronize asynchronous evaluations
  if (asynch_flag) {
    const IntResponseMap& resp_map = model.synchronize();
    if (log_resp_flag) // log response data
      allResponses = resp_map;
    if (log_best_flag) { // update best variables/response
      IntRespMCIter r_cit;
      if (compactMode)
	for (i=0, r_cit=resp_map.begin(); r_cit!=resp_map.end(); ++i, ++r_cit)
	  update_best(allSamples[i], r_cit->first, r_cit->second);
      else
	for (i=0, r_cit=resp_map.begin(); r_cit!=resp_map.end(); ++i, ++r_cit)
	  update_best(allVariables[i], r_cit->first, r_cit->second);
    }
  }
}


void Analyzer::update_model_from_variables(Model& model, const Variables& vars)
{
  // default implementation is sufficient in current uses, but could
  // be overridden in future cases where a view discrepancy can exist
  // between model and vars.
  model.active_variables(vars);
}


void Analyzer::update_model_from_sample(Model& model, const Real* sample_vars)
{
  // default implementation is sufficient for FSUDesignCompExp and
  // NonD{Quadrature,SparseGrid,Cubature}, but NonDSampling overrides.
  size_t i, num_cv = model.cv();
  for (i=0; i<num_cv; ++i)
    model.continuous_variable(sample_vars[i], i);
}


/** Default mapping that maps into continuous part of Variables only */
void Analyzer::
sample_to_variables(const Real* sample_c_vars, Variables& vars)
{
  // pack sample_matrix into vars_array
  const Variables& model_vars = iteratedModel.current_variables();
  size_t i, j, num_adiv = model_vars.adiv(), num_adrv = model_vars.adrv();
  if (vars.is_null()) // use minimal data ctor
    vars = Variables(model_vars.shared_data());
  for (j=0; j<numContinuousVars; ++j)
    vars.continuous_variable(sample_c_vars[j], j); // jth row
  // BMA: this may be needed if vars wasn't initialized off the model
  vars.inactive_continuous_variables(
    model_vars.inactive_continuous_variables());
  // preserve any active discrete vars (unsupported by sample_matrix)
  if (num_adiv)
    vars.all_discrete_int_variables(model_vars.all_discrete_int_variables());
  if (num_adrv)
    vars.all_discrete_real_variables(model_vars.all_discrete_real_variables());
}


void Analyzer::
samples_to_variables_array(const RealMatrix& sample_matrix,
			   VariablesArray& vars_array)
{
  // pack sample_matrix into vars_array
  size_t num_samples = sample_matrix.numCols(); // #vars by #samples
  if (vars_array.size() != num_samples)
    vars_array.resize(num_samples);
  for (size_t i=0; i<num_samples; ++i)
    sample_to_variables(sample_matrix[i], vars_array[i]);
}


/** Default implementation maps active continuous variables only */
void Analyzer::
variables_to_sample(const Variables& vars, Real* sample_c_vars)
{
  const RealVector& c_vars = vars.continuous_variables();
  for (size_t j=0; j<numContinuousVars; ++j)
    sample_c_vars[j] = c_vars[j]; // jth row of samples_matrix
}


void Analyzer::
variables_array_to_samples(const VariablesArray& vars_array,
			   RealMatrix& sample_matrix)
{
  // pack vars_array into sample_matrix
  size_t i, j, num_samples = vars_array.size();
  if (sample_matrix.numRows() != numContinuousVars ||
      sample_matrix.numCols() != num_samples)
    sample_matrix.reshape(numContinuousVars, num_samples); // #vars by #samples
  // populate each colum of the sample matrix (one col per sample)
  for (i=0; i<num_samples; ++i)
    variables_to_sample(vars_array[i], sample_matrix[i]);
}



/** Generate (numvars + 2)*num_samples replicate sets for VBD,
    populating allSamples( numvars, (numvars + 2)*num_samples ) */
void Analyzer::get_vbd_parameter_sets(Model& model, int num_samples)
{
  if (!compactMode) {
    Cerr << "\nError: get_vbd_parameter_sets requires compactMode.\n";
    abort_handler(-1);
  }

  // BMA TODO: This may not be right for all LHS active/inactive
  // sampling modes, but is equivalent to previous code.
  size_t num_vars = numContinuousVars + numDiscreteIntVars + 
    numDiscreteStringVars + numDiscreteRealVars;
  size_t num_replicates = num_vars + 2;

  allSamples.shape(num_vars, (num_vars+2)*num_samples);

  // run derived sampling routine generate two initial matrices
  vary_pattern(true);

  // populate the first num_samples cols of allSamples
  RealMatrix sample_1(Teuchos::View, allSamples, num_vars, num_samples, 0, 0);
  get_parameter_sets(model, num_samples, sample_1);
  if (sample_1.numCols() != num_samples) {
    Cerr << "\nError in Analyzer::variance_based_decomp(): Expected "
	 << num_samples << " variable samples; received "
	 << sample_1.numCols() << std::endl;
    abort_handler(-1);
  }
  
  // populate the second num_samples cols of allSamples
  RealMatrix sample_2(Teuchos::View, allSamples, num_vars, num_samples, 0, 
		     num_samples);
  get_parameter_sets(model, num_samples, sample_2);
  if (sample_2.numCols() != num_samples) {
    Cerr << "\nError in Analyzer::variance_based_decomp(): Expected "
	 << num_samples << " variable samples; received "
	 << sample_2.numCols() << std::endl;
    abort_handler(-1);
  }

  // one additional replicate per variable
  for (int i=0; i<num_vars; ++i) {
    int replicate_index = i+2;
    RealMatrix sample_r(Teuchos::View, allSamples, num_vars, num_samples, 0, 
			replicate_index * num_samples);
    // initialize the replicate to the second sample
    sample_r.assign(sample_2);
    // now swap in a row from the first sample
    for (int j=0; j<num_samples; ++j)
      sample_r(i, j) = sample_1(i, j);
  }
}


/** Calculation of sensitivity indices obtained by variance based
    decomposition.  These indices are obtained by the Saltelli version
    of the Sobol VBD which uses (K+2)*N function evaluations, where K
    is the number of dimensions (uncertain vars) and N is the number
    of samples.  */
void Analyzer::compute_vbd_stats(const int num_samples, 
				 const IntResponseMap& resp_samples)
{
  using boost::multi_array;
  using boost::extents;

  // BMA TODO: This may not be right for all LHS active/inactive
  // sampling modes, but is equivalent to previous code.
  size_t i, j, k, num_vars = numContinuousVars + numDiscreteIntVars + 
    numDiscreteStringVars + numDiscreteRealVars;

  if (resp_samples.size() != num_samples*(num_vars+2)) {
    Cerr << "\nError in Analyzer::compute_vbd_stats: expected "
	 << num_samples << " responses; received " << resp_samples.size()
	 << std::endl;
    abort_handler(-1);
  }
  
  // BMA: for now copy the data to previous data structure 
  //      total_fn_vals[respFn][replicate][sample]
  // This is making the assumption that the responses are ordered as allSamples
  // BMA TODO: compute statistics on finite samples only
  multi_array<Real,3>
    total_fn_vals(extents[numFunctions][num_vars+2][num_samples]);
  IntRespMCIter r_it = resp_samples.begin();
  for (i=0; i<(num_vars+2); ++i)
    for (j=0; j<num_samples; ++r_it, ++j)
      for (k=0; k<numFunctions; ++k)
	total_fn_vals[k][i][j] = r_it->second.function_value(k);

#ifdef DEBUG
    Cout << "allSamples:\n" << allSamples << '\n';
    for (k=0; k<numFunctions; ++k)
      for (i=0; i<num_vars+2; ++i)
	for (j=0; j<num_samples; ++j)
	  Cout << "Response " << k << " for replicate " << i << ", sample " << j
	       << ": " << total_fn_vals[k][i][j] << '\n';
#endif

  // There are four versions of the indices being calculated. 
  // S1 is a corrected version from Saltelli's 2004 "Sensitivity 
  // Analysis in Practice" book.  S1 does not have scaled output Y, 
  // but S2 and S3 do (where scaled refers to subtracting the overall mean 
  // from all of the output samples).  S2 and S3 have different ways of 
  // calculating the overal variance.  S4 uses the scaled Saltelli 
  // formulas from the following paper:  Saltelli, A., Annoni, P., Azzini, I.,
  // Campolongo, F., Ratto, M., Tarantola, S.. Variance based sensitivity 
  // analysis of model output.  Design and estimator for the total sensitivity
  // index. Comp Physics Comm 2010;181:259--270.  We decided to use formulas S4
  // and T4 based on testing with a shock physics problem that had significant 
  // nonlinearities, interactions, and several response functions. 
  // The results are documented in a paper by Weirs et al. that will 
  // be forthcoming in RESS in 2011. For now we are leaving the different 
  // implementations in if further testing is needed. 

  //RealVectorArray S1(numFunctions), T1(numFunctions);
  //RealVectorArray S2(numFunctions), T2(numFunctions);
  //RealVectorArray S3(numFunctions), T3(numFunctions);
  S4.resize(numFunctions, RealVector(num_vars)); 
  T4.resize(numFunctions, RealVector(num_vars));

  multi_array<Real,3>
    total_norm_vals(extents[numFunctions][num_vars+2][num_samples]);

  // Loop over number of responses to obtain sensitivity indices for each
  for (k=0; k<numFunctions; ++k) {
 
    // calculate expected value of Y
    Real mean_hatY = 0., var_hatYS = 0., var_hatYT = 0.,
      mean_sq1=0., mean_sq2 = 0., var_hatYnom = 0., 
      overall_mean = 0., mean_hatB=0., var_hatYC= 0., mean_C=0., 
      mean_A_norm = 0., mean_B_norm = 0., var_A_norm = 0., var_B_norm=0., 
      var_AB_norm = 0.;
    // mean estimate of Y (possibly average over matrices later)
    for (j=0; j<num_samples; j++)
      mean_hatY += total_fn_vals[k][0][j];
    mean_hatY /= (Real)num_samples;
    mean_sq1 = mean_hatY*mean_hatY;
    for (j=0; j<num_samples; j++)
      mean_hatB += total_fn_vals[k][1][j];
    mean_hatB /= (Real)(num_samples);
    mean_sq2 = mean_hatY*mean_hatB;
    //mean_sq2 += total_fn_vals[k][0][j]*total_fn_vals[k][1][j];
    //mean_sq2 /= (Real)(num_samples);
    // variance estimate of Y for S indices
    for (j=0; j<num_samples; j++)
      var_hatYS += std::pow(total_fn_vals[k][0][j], 2.);
    var_hatYnom = var_hatYS/(Real)(num_samples) - mean_sq1;
    var_hatYS = var_hatYS/(Real)(num_samples) - mean_sq2;
    // variance estimate of Y for T indices
    for (j=0; j<num_samples; j++)
      var_hatYT += std::pow(total_fn_vals[k][1][j], 2.);
    var_hatYT = var_hatYT/(Real)(num_samples) - (mean_hatB*mean_hatB);
    for (j=0; j<num_samples; j++){
      for(i=0; i<(num_vars+2); i++){ 
        total_norm_vals[k][i][j]=total_fn_vals[k][i][j];
        overall_mean += total_norm_vals[k][i][j];
      } 
    }
   
    overall_mean /= Real((num_samples)* (num_vars+2));
    for (j=0; j<num_samples; j++)
      for(i=0; i<(num_vars+2); i++)
        total_norm_vals[k][i][j]-=overall_mean;
    mean_C=mean_hatB*(Real)(num_samples)+mean_hatY*(Real)(num_samples);
    mean_C=mean_C/(2*(Real)(num_samples));
    for (j=0; j<num_samples; j++)
      var_hatYC += std::pow(total_fn_vals[k][0][j],2);
    for (j=0; j<num_samples; j++)
      var_hatYC += std::pow(total_fn_vals[k][1][j],2);
    var_hatYC = var_hatYC/((Real)(num_samples)*2)-mean_C*mean_C;

  //  for (j=0; j<num_samples; j++)
  //    mean_A_norm += total_norm_vals[k][0][j];
  //  mean_A_norm /= (Real)num_samples;
  //  for (j=0; j<num_samples; j++)
  //    mean_B_norm += total_norm_vals[k][1][j];
  //  mean_B_norm /= (Real)(num_samples);
  //  for (j=0; j<num_samples; j++)
  //    var_A_norm += std::pow(total_norm_vals[k][0][j], 2.);
  //  var_AB_norm = var_A_norm/(Real)(num_samples) - (mean_A_norm*mean_B_norm);
  //  var_A_norm = var_A_norm/(Real)(num_samples) - (mean_A_norm*mean_A_norm);
  // variance estimate of Y for T indices
  //  for (j=0; j<num_samples; j++)
  //    var_B_norm += std::pow(total_norm_vals[k][1][j], 2.);
  //  var_B_norm = var_B_norm/(Real)(num_samples) - (mean_B_norm*mean_B_norm);


#ifdef DEBUG
    Cout << "\nVariance of Yhat for S " << var_hatYS 
	 << "\nVariance of Yhat for T " << var_hatYT
	 << "\nMeanSq1 " << mean_sq1 << "\nMeanSq2 " << mean_sq2 << '\n';
#endif

    // calculate first order sensitivity indices and first order total indices
    for (i=0; i<num_vars; i++) {
      Real sum_S = 0., sum_T = 0., sum_J = 0., sum_J2 = 0., sum_3 = 0., sum_32=0.,sum_5=0, sum_6=0;
      for (j=0; j<num_samples; j++) {
	//sum_S += total_fn_vals[k][0][j]*total_fn_vals[k][i+2][j];
	//sum_T += total_fn_vals[k][1][j]*total_fn_vals[k][i+2][j];
        //sum_5 += total_norm_vals[k][0][j]*total_norm_vals[k][i+2][j];
	//sum_6 += total_norm_vals[k][1][j]*total_norm_vals[k][i+2][j];
	//sum_J += std::pow((total_fn_vals[k][0][j]-total_fn_vals[k][i+2][j]),2.);
	sum_J2 += std::pow((total_norm_vals[k][1][j]-total_norm_vals[k][i+2][j]),2.);
	sum_3 += total_norm_vals[k][0][j]*(total_norm_vals[k][i+2][j]-total_norm_vals[k][1][j]);
	//sum_32 += total_fn_vals[k][1][j]*(total_fn_vals[k][1][j]-total_fn_vals[k][i+2][j]);
      }

      //S1[k][i] = (sum_S/(Real)(num_samples-1) - mean_sq2)/var_hatYS;  
      //S1[k][i] = (sum_S/(Real)(num_samples) - mean_sq2)/var_hatYS;  
      //T1[k][i] = 1. - (sum_T/(Real)(num_samples-1) - mean_sq2)/var_hatYS;
      //T1[k][i] = 1. - (sum_T/(Real)(num_samples) - (mean_hatB*mean_hatB))/var_hatYT;
      //S2[k][i] = (sum_S/(Real)(num_samples) - mean_sq1)/var_hatYnom;     
      //T2[k][i] = 1. - (sum_T/(Real)(num_samples) - mean_sq1)/var_hatYnom;
      //S2[k][i] = (sum_5/(Real)(num_samples) - (mean_A_norm*mean_B_norm))/var_AB_norm;  
      //T2[k][i] = 1. - (sum_6/(Real)(num_samples) - (mean_B_norm*mean_B_norm))/var_B_norm;
      //S3[k][i] = ((sum_5/(Real)(num_samples)) - (mean_A_norm*mean_A_norm))/var_A_norm;  
      //T3[k][i] = 1. - (sum_6/(Real)(num_samples) - (mean_A_norm*mean_B_norm))/var_AB_norm;
      //S3[k][i] = (sum_3/(Real)(num_samples))/var_hatYC;    
      //T3[k][i] = (sum_32/(Real)(num_samples))/var_hatYnom;
      //S4[k][i] = (var_hatYnom - (sum_J/(Real)(2*num_samples)))/var_hatYnom;
      S4[k][i] = (sum_3/(Real)(num_samples))/var_hatYC;    
      T4[k][i] = (sum_J2/(Real)(2*num_samples))/var_hatYC;
    }
  }
}


/** Generate tabular output with active variables (compactMode) or all
    variables with their labels and response labels, with no data.
    Variables are sequenced {cv, div, drv} */
void Analyzer::pre_output()
{
  // distinguish between defaulted pre-run and user-specified
  if (!parallelLib.command_line_user_modes())
    return;

  const String& filename = parallelLib.command_line_pre_run_output();
  if (filename.empty()) {
    if (outputLevel > QUIET_OUTPUT)
      Cout << "\nPre-run phase complete: no output requested.\n" << std::endl;
    return;
  }

  size_t num_evals = compactMode ? allSamples.numCols() : allVariables.size();
  if (num_evals == 0) {
    if (outputLevel > QUIET_OUTPUT)
      Cout << "\nPre-run phase complete: no variables to output.\n"
	   << std::endl;
    return;
  }

  std::ofstream tabular_file;
  TabularIO::open_file(tabular_file, filename, "pre-run output");

  // try to mitigate errors resulting from lack of precision in output
  // the full 17 digits might surprise users, but will reduce
  // numerical errors between pre/post phases
  // TODO: consider passing precision to helper functions instead of using global
  int save_precision;
  if (writePrecision == 0) {
    save_precision = write_precision;
    write_precision = 17;
  }

  // Write all variables in input spec ordering; always annotated.
  // When in compactMode, get the inactive variables off the Model and
  // use sample_to_variables to set the discrete variables not treated
  // by allSamples.
  unsigned short tabular_format = 
    parallelLib.program_options().pre_run_output_format();
  TabularIO::write_header_tabular(tabular_file,
				  iteratedModel.current_variables(), 
				  iteratedModel.current_response(),
				  "eval_id",
				  tabular_format);

  tabular_file << std::setprecision(write_precision) 
	       << std::resetiosflags(std::ios::floatfield);

  Variables vars = iteratedModel.current_variables().copy();
  for (size_t eval_index = 0; eval_index < num_evals; eval_index++) {

    TabularIO::write_leading_columns(tabular_file, eval_index+1, 
				     iteratedModel.interface_id(),
				     tabular_format);
    if (compactMode) {
      // allSamples num_vars x num_evals, so each col becomes tabular file row
      // populate the active discrete variables that aren't in sample_matrix
      size_t num_vars = allSamples.numRows();
      sample_to_variables(allSamples[eval_index], vars);
      vars.write_tabular(tabular_file);
    }
    else
      allVariables[eval_index].write_tabular(tabular_file);
    // no response data, so terminate the record
    tabular_file << '\n';
  }

  tabular_file.flush();
  tabular_file.close();

  if (writePrecision == 0)
    write_precision = save_precision;
  if (outputLevel > QUIET_OUTPUT)
    Cout << "\nPre-run phase complete: variables written to tabular file "
	 << filename << ".\n" << std::endl;
}


/// read num_evals variables/responses from file
void Analyzer::read_variables_responses(int num_evals, size_t num_vars)
{
  // distinguish between defaulted post-run and user-specified
  if (!parallelLib.command_line_user_modes())
    return;

  const String& filename = parallelLib.command_line_post_run_input();
  if (filename.empty()) {
    if (outputLevel > QUIET_OUTPUT)
      Cout << "\nPost-run phase initialized: no input requested.\n" 
	   << std::endl;
    return;
  }

  if (num_evals == 0) {
    if (outputLevel > QUIET_OUTPUT)
      Cout << "\nPost-run phase initialized: zero samples specified.\n" 
	   << std::endl;
    return;
  }

  // pre/post only supports annotated; could detect
  unsigned short tabular_format = 
    parallelLib.program_options().post_run_input_format();

  // Define modelList and recastFlags to support any recastings within
  // a model recursion
  bool map_to_iter_space = iteratedModel.manage_data_recastings();

  // TO DO: validate/accommodate incoming num_vars since it may be defined
  // from a local sampling mode (see NonDSampling) that differs from active;
  // support for active discrete also varies across the post-run Iterators.
  bool active_only = true; // consistent with PStudyDACE use cases
  Variables vars(iteratedModel.current_variables().copy());
  Response  resp(iteratedModel.current_response().copy());

  PRPList import_prp_list;
  bool verbose = (outputLevel > NORMAL_OUTPUT);
  TabularIO::read_data_tabular(filename, "post-run input", vars, resp,
			       import_prp_list, tabular_format, verbose,
			       active_only);
  size_t num_imported = import_prp_list.size();
  if (num_imported < num_evals) {
    Cerr << "Error: number of imported evaluations (" << num_imported
	 << ") less than expected (" << num_evals << ")." << std::endl;
    abort_handler(METHOD_ERROR);
  }
  else if (verbose) {
    Cout << "\nRead " << num_imported << " samples from file " << filename;
    if (num_imported > num_evals)
      Cout << " of which " << num_evals << " will be used." << std::endl;
    else Cout << std::endl;
  }

  if (compactMode) allSamples.shapeUninitialized(num_vars, num_evals);
  else             allVariables.resize(num_evals);

  size_t i; PRPLIter prp_it;
  bool cache = iteratedModel.evaluation_cache(), // recurse_flag = true
     restart = iteratedModel.restart_file();     // recurse_flag = true
  Variables iter_vars; Response iter_resp;
  for (i=0, prp_it=import_prp_list.begin(); i<num_evals; ++i, ++prp_it) {

    ParamResponsePair& pr = *prp_it;

    // insert imported user-space data into evaluation cache (for consistency)
    // and restart (more likely to be useful).  Unlike DataFitSurrModel, we
    // will preserve the incoming eval id in the post-input file import case.
    if (restart) parallelLib.write_restart(pr); // preserve eval id
    if (cache)   data_pairs.insert(pr); // duplicate ids OK for PRPCache

    // manage any model recastings to promote from user-space to iterator-space
    if (map_to_iter_space)
      iteratedModel.user_space_to_iterator_space(pr.variables(), pr.response(),
						 iter_vars, iter_resp);
    else
      { iter_vars = pr.variables(); iter_resp = pr.response(); }

    // update allVariables,allSamples
    if (compactMode) variables_to_sample(iter_vars, allSamples[i]);
    else             allVariables[i] = iter_vars;
    // update allResponses
    allResponses[pr.eval_id()] = iter_resp;

    // mirror any post-processing in Analyzer::evaluate_parameter_sets()
    if (numObjFns || numLSqTerms)
      update_best(iter_vars, i+1, iter_resp);
  }

  if (outputLevel > QUIET_OUTPUT)
    Cout << "\nPost-run phase initialized: variables / responses read from "
	 << "tabular\nfile " << filename << ".\n" << std::endl;
}


/** printing of variance based decomposition indices. */
void Analyzer::print_sobol_indices(std::ostream& s) const
{
  StringMultiArrayConstView cv_labels
    = iteratedModel.continuous_variable_labels();
  StringMultiArrayConstView div_labels
    = iteratedModel.discrete_int_variable_labels();
  StringMultiArrayConstView drv_labels
    = iteratedModel.discrete_real_variable_labels();
  const StringArray& resp_labels = iteratedModel.response_labels();
  // output explanatory info
  //s << "Variance Based Decomposition Sensitivity Indices\n"
  //  << "These Sobol' indices measure the importance of the uncertain input\n"
  //  << "variables in determining the uncertainty (variance) of the output.\n"
  //  << "Si measures the main effect for variable i itself, while Ti\n"
  //  << "measures the total effect (including the interaction effects\n" 
  //  << "of variable i with other uncertain variables.)\n";
  s << std::scientific 
    << "\nGlobal sensitivity indices for each response function:\n";

  size_t i, k, offset;
  for (k=0; k<numFunctions; ++k) {
    s << resp_labels[k] << " Sobol' indices:\n"; 
    s << std::setw(38) << "Main" << std::setw(19) << "Total\n";
    
    for (i=0; i<numContinuousVars; ++i)
      if (std::abs(S4[k][i]) > vbdDropTol || std::abs(T4[k][i]) > vbdDropTol)
        s << "                     " << std::setw(write_precision+7) << S4[k][i]
	  << ' ' << std::setw(write_precision+7) << T4[k][i] << ' '
	  << cv_labels[i] << '\n';
    offset = numContinuousVars;
    for (i=0; i<numDiscreteIntVars; ++i)
      if (std::abs(S4[k][i]) > vbdDropTol || std::abs(T4[k][i]) > vbdDropTol)
	s << "                     " << std::setw(write_precision+7) 
	  << S4[k][i+offset] << ' ' << std::setw(write_precision+7)
	  << T4[k][i+offset] << ' ' << div_labels[i] << '\n';
    offset += numDiscreteIntVars;
    //for (i=0; i<numDiscreteStringVars; ++i) // LPS TO DO
    //offset += numDiscreteStringVars;
    for (i=0; i<numDiscreteRealVars; ++i)
      if (std::abs(S4[k][i]) > vbdDropTol || std::abs(T4[k][i]) > vbdDropTol)
	s << "                     " << std::setw(write_precision+7) 
	  << S4[k][i+offset] << ' ' << std::setw(write_precision+7)
          << T4[k][i+offset] << ' ' << drv_labels[i] << '\n';
  }
}


void Analyzer::compute_best_metrics(const Response& response,
				    std::pair<Real,Real>& metrics)
{
  size_t i, constr_offset;
  const RealVector& fn_vals = response.function_values();
  const RealVector& primary_wts
    = iteratedModel.primary_response_fn_weights();
  Real& obj_fn = metrics.second; obj_fn = 0.0;
  if (numObjFns) {
    constr_offset = numObjFns;
    if (primary_wts.empty()) {
      for (i=0; i<numObjFns; i++)
	obj_fn += fn_vals[i];
      if (numObjFns > 1)
	obj_fn /= (Real)numObjFns;
    }
    else
      for (i=0; i<numObjFns; i++)
	obj_fn += primary_wts[i] * fn_vals[i];
  }
  else if (numLSqTerms) {
    constr_offset = numLSqTerms;
    if (primary_wts.empty())
      for (i=0; i<numLSqTerms; i++)
	obj_fn += std::pow(fn_vals[i], 2);
    else
      for (i=0; i<numLSqTerms; i++)
	obj_fn += std::pow(primary_wts[i]*fn_vals[i], 2);
  }
  else // no "best" metric currently defined for generic response fns
    return;
  Real& constr_viol   = metrics.first; constr_viol= 0.0;
  size_t num_nln_ineq = iteratedModel.num_nonlinear_ineq_constraints(),
         num_nln_eq   = iteratedModel.num_nonlinear_eq_constraints();
  const RealVector& nln_ineq_lwr_bnds
    = iteratedModel.nonlinear_ineq_constraint_lower_bounds();
  const RealVector& nln_ineq_upr_bnds
    = iteratedModel.nonlinear_ineq_constraint_upper_bounds();
  const RealVector& nln_eq_targets
    = iteratedModel.nonlinear_eq_constraint_targets();
  for (i=0; i<num_nln_ineq; i++) { // ineq constraint violation
    size_t index = i + constr_offset;
    Real ineq_con = fn_vals[index];
    if (ineq_con > nln_ineq_upr_bnds[i])
      constr_viol += std::pow(ineq_con - nln_ineq_upr_bnds[i],2);
    else if (ineq_con < nln_ineq_lwr_bnds[i])
      constr_viol += std::pow(nln_ineq_lwr_bnds[i] - ineq_con,2);
  }
  for (i=0; i<num_nln_eq; i++) { // eq constraint violation
    size_t index = i + constr_offset + num_nln_ineq;
    Real eq_con = fn_vals[index];
    if (std::fabs(eq_con - nln_eq_targets[i]) > 0.)
      constr_viol += std::pow(eq_con - nln_eq_targets[i], 2);
  }
}


void Analyzer::
update_best(const Real* sample_c_vars, int eval_id, const Response& response)
{
  RealRealPair metrics; 
  compute_best_metrics(response, metrics);
#ifdef DEBUG
  Cout << "Best metrics: " << metrics.first << ' ' << metrics.second
       << std::endl;
#endif

  size_t num_best_map = bestVarsRespMap.size();
  if (num_best_map < numFinalSolutions) { // initialization of best map
    Variables vars = iteratedModel.current_variables().copy();
    sample_to_variables(sample_c_vars, vars); // copy sample only when needed
    Response copy_resp = response.copy();
    ParamResponsePair prp(vars, iteratedModel.interface_id(), copy_resp,
			  eval_id, false); // shallow copy since previous deep
    std::pair<RealRealPair, ParamResponsePair> new_pr(metrics, prp);
    bestVarsRespMap.insert(new_pr);
  }
  else {
    RealPairPRPMultiMap::iterator it = --bestVarsRespMap.end();
    //   Primary criterion: constraint violation must be <= stored violation
    // Secondary criterion: for equal (or zero) constraint violation, objective
    //                      must be < stored objective
    if (metrics < it->first) { // new best
      bestVarsRespMap.erase(it);
      Variables vars = iteratedModel.current_variables().copy();
      sample_to_variables(sample_c_vars, vars); // copy sample only when needed
      Response copy_resp = response.copy();
      ParamResponsePair prp(vars, iteratedModel.interface_id(), copy_resp,
			    eval_id, false); // shallow copy since previous deep
      std::pair<RealRealPair, ParamResponsePair> new_pr(metrics, prp);
      bestVarsRespMap.insert(new_pr);
    }
  }
}


void Analyzer::
update_best(const Variables& vars, int eval_id, const Response& response)
{
  RealRealPair metrics; 
  compute_best_metrics(response, metrics);
#ifdef DEBUG
  Cout << "Best metrics: " << metrics.first << ' ' << metrics.second
       << std::endl;
#endif

  size_t num_best_map = bestVarsRespMap.size();
  if (num_best_map < numFinalSolutions) { // initialization of best map
    ParamResponsePair prp(vars, iteratedModel.interface_id(),
			  response, eval_id); // deep copy
    std::pair<RealRealPair, ParamResponsePair> new_pr(metrics, prp);
    bestVarsRespMap.insert(new_pr);
  }
  else {
    RealPairPRPMultiMap::iterator it = --bestVarsRespMap.end();
    //   Primary criterion: constraint violation must be <= stored violation
    // Secondary criterion: for equal (or zero) constraint violation, objective
    //                      must be < stored objective
    if (metrics < it->first) { // new best
      bestVarsRespMap.erase(it);
      ParamResponsePair prp(vars, iteratedModel.interface_id(),
			    response, eval_id); // deep copy
      std::pair<RealRealPair, ParamResponsePair> new_pr(metrics, prp);
      bestVarsRespMap.insert(new_pr);
    }
  }
}


void Analyzer::print_results(std::ostream& s)
{
  if (!numObjFns && !numLSqTerms) {
    s << "<<<<< Best data metrics not defined for generic response functions\n";
    return;
  }

  // -------------------------------------
  // Single and Multipoint results summary
  // -------------------------------------
  RealPairPRPMultiMap::iterator it = bestVarsRespMap.begin();
  size_t i, offset, num_fns, num_best_map = bestVarsRespMap.size();
  for (i=1; it!=bestVarsRespMap.end(); ++i, ++it) {
    const ParamResponsePair& best_pr = it->second;
    const Variables&  best_vars = best_pr.variables();
    const RealVector& best_fns  = best_pr.response().function_values();
    s << "<<<<< Best parameters          ";
    if (num_best_map > 1) s << "(set " << i << ") ";
    s << "=\n" << best_vars;
    num_fns = best_fns.length(); offset = 0;
    if (numObjFns) {
      if (numObjFns > 1) s << "<<<<< Best objective functions ";
      else               s << "<<<<< Best objective function  ";
      if (num_best_map > 1) s << "(set " << i << ") "; s << "=\n";
      write_data_partial(s, offset, numObjFns, best_fns);
      offset = numObjFns;
    }
    else if (numLSqTerms) {
      s << "<<<<< Best residual terms      ";
      if (num_best_map > 1) s << "(set " << i << ") "; s << "=\n";
      write_data_partial(s, offset, numLSqTerms, best_fns);
      offset = numLSqTerms;
    }
    if (num_fns > offset) {
      s << "<<<<< Best constraint values   ";
      if (num_best_map > 1) s << "(set " << i << ") "; s << "=\n";
      write_data_partial(s, offset, num_fns - offset, best_fns);
    }
    s << "<<<<< Best data captured at function evaluation "
      << best_pr.eval_id() << std::endl;
  }
}


void Analyzer::vary_pattern(bool pattern_flag)
{
  Cerr << "Error: Analyzer lacking redefinition of virtual vary_pattern() "
       << "function.\n       This analyzer does not support pattern variance."
       << std::endl;
  abort_handler(-1);
}


void Analyzer::get_parameter_sets(Model& model)
{
  Cerr << "Error: Analyzer lacking redefinition of virtual get_parameter_sets"
       << "(1) function.\n       This analyzer does not support parameter sets."
       << std::endl;
  abort_handler(-1);
}

void Analyzer::get_parameter_sets(Model& model, const int num_samples, 
				  RealMatrix& design_matrix)
{
  Cerr << "Error: Analyzer lacking redefinition of virtual get_parameter_sets"
       << "(3) function.\n       This analyzer does not support parameter sets."
       << std::endl;
  abort_handler(-1);
}

} // namespace Dakota
