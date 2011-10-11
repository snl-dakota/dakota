/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Analyzer
//- Description: Implementation code for the Analyzer class
//- Owner:       Mike Eldred
//- Checked by:

#include <stdexcept>
#include "system_defs.h"
#include "data_io.h"
#include "tabular_io.h"
#include "DakotaModel.H"
#include "DakotaAnalyzer.H"
#include "ProblemDescDB.H"
#include "ParallelLibrary.H"
#include "PRPMultiIndex.H"

static const char rcsId[]="@(#) $Id: DakotaAnalyzer.C 7035 2010-10-22 21:45:39Z mseldre $";

//#define DEBUG

namespace Dakota {


Analyzer::Analyzer(Model& model):
  Iterator(BaseConstructor(), model), compactMode(true)
{
  iteratedModel = model;
  if (probDescDB.get_sizet("responses.num_response_functions"))
    numObjFns = numLSqTerms = 0; // no best data tracking
  else {
    numObjFns   = probDescDB.get_sizet("responses.num_objective_functions");
    numLSqTerms = probDescDB.get_sizet("responses.num_least_squares_terms");
  }
  if (probDescDB.get_bool("method.variance_based_decomp")) 
    vbdDropTol = probDescDB.get_real("method.vbd_drop_tolerance");

  if (!numFinalSolutions)  // default is zero
    numFinalSolutions = 1; // iterator-specific default assignment
}


Analyzer::Analyzer(NoDBBaseConstructor, Model& model):
  Iterator(NoDBBaseConstructor(), model), compactMode(true),
  numObjFns(0), numLSqTerms(0) // no best data tracking
{ iteratedModel = model; }


Analyzer::Analyzer(NoDBBaseConstructor): Iterator(NoDBBaseConstructor()),
  compactMode(true), numObjFns(0), numLSqTerms(0) // no best data tracking
{ }


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
  if (!asynchFlag && log_resp_flag) allResponses.clear();

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
    if (asynchFlag)
      model.asynch_compute_response(activeSet);
    else {
      model.compute_response(activeSet);
      const Response& resp = model.current_response();
      int eval_id = model.evaluation_id();
      if (log_best_flag) // update best variables/response
        update_best(model.current_variables(), eval_id, resp);
      if (log_resp_flag) // log response data
        allResponses[eval_id] = resp.copy();
    }
  }

  // synchronize asynchronous evaluations
  if (asynchFlag) {
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


void Analyzer::
variables_array_to_samples(const VariablesArray& vars_array,
			   RealMatrix& sample_matrix)
{
  // pack vars_array into sample_matrix
  size_t i, j, num_samples = vars_array.size();
  if (sample_matrix.numRows() != numContinuousVars ||
      sample_matrix.numCols() != num_samples)
    sample_matrix.reshape(numContinuousVars, num_samples); // #vars by #samples
  const Variables& vars = iteratedModel.current_variables();
  for (i=0; i<num_samples; ++i) {
    const RealVector& c_vars_i = vars_array[i].continuous_variables();
    Real* sample_c_vars = sample_matrix[i]; // ith column
    for (j=0; j<numContinuousVars; ++j)
      sample_c_vars[j] = c_vars_i[j]; // jth row
  }
}


/** Calculation of sensitivity indices obtained by variance based
    decomposition.  These indices are obtained by the Saltelli version
    of the Sobol VBD which uses (K+2)*N function evaluations, where K
    is the number of dimensions (uncertain vars) and N is the number
    of samples.  */
void Analyzer::
variance_based_decomp(int ncont, int ndiscint, int ndiscreal, int num_samples)
{
  using boost::multi_array;
  using boost::extents;
  size_t i, j, k;
  int ndimtotal = ncont + ndiscreal + ndiscint;

  // run derived sampling routine twice to generate input matrices, M1 and M2

  // WJB - ToDo: confer with MSE: RealVector2DArray total_c_vars(ncont+2);
  multi_array<RealVector, 2> total_c_vars(extents[ndimtotal+2][num_samples]);

  multi_array<IntVector, 2> total_di_vars = (ndiscint > 0) ?
    multi_array<IntVector, 2>(extents[ndimtotal+2][num_samples]):
    multi_array<IntVector, 2>();

  multi_array<RealVector, 2> total_dr_vars = (ndiscreal > 0 ) ? 
    multi_array<RealVector, 2>(extents[ndimtotal+2][num_samples]):
    multi_array<RealVector, 2>();

  // get first sample block
  vary_pattern(true);
  get_parameter_sets(iteratedModel);
  if (compactMode) {
    if (allSamples.numCols() != num_samples) {
      Cerr << "\nError in Analyzer::variance_based_decomp(): Expected "
	   << num_samples << " variable samples; received "
	   << allSamples.numCols() << std::endl;
      abort_handler(-1);
    }
    for (j=0; j<num_samples; ++j) {
      const Real* sample_j = allSamples[j];
      if (ncont)
	copy_data(sample_j, ncont, total_c_vars[0][j]);
      if (ndiscint) {
	IntVector& t_div_0j = total_di_vars[0][j];
	for (k=0; k<ndiscint; ++k)
	  t_div_0j[k] = (int)sample_j[ncont+k];
      }
      if (ndiscreal)
	copy_data(&sample_j[ncont+ndiscint], ndiscreal, total_dr_vars[0][j]);
    }
  }
  else {
    if (allVariables.size() != num_samples) {
      Cerr << "\nError in Analyzer::variance_based_decomp(): Expected "
	   << num_samples << " variables sets; received "
	   << allVariables.size() << std::endl;
      abort_handler(-1);
    }
    for (j=0; j<num_samples; ++j) {
      const Variables& all_vars_j = allVariables[j];
      if (ncont)
	copy_data(all_vars_j.continuous_variables(),    total_c_vars[0][j]);
      if (ndiscint)
	copy_data(all_vars_j.discrete_int_variables(),  total_di_vars[0][j]);
      if (ndiscreal)
	copy_data(all_vars_j.discrete_real_variables(), total_dr_vars[0][j]);
    }
  }

  // get second sample block
  get_parameter_sets(iteratedModel);
  if (compactMode) {
    if (allSamples.numCols() != num_samples) {
      Cerr << "\nError in Analyzer::variance_based_decomp(): Expected "
	   << num_samples << " variable samples; received "
	   << allSamples.numCols() << std::endl;
      abort_handler(-1);
    }
    for (j=0; j<num_samples; ++j) {
      const Real* sample_j = allSamples[j];
      if (ncont)
	copy_data(sample_j, ncont, total_c_vars[1][j]);
      if (ndiscint) {
	IntVector& t_div_1j = total_di_vars[1][j];
	for (k=0; k<ndiscint; ++k)
	  t_div_1j[k] = (int)sample_j[ncont+k];
      }
      if (ndiscreal)
	copy_data(&sample_j[ncont+ndiscint], ndiscreal, total_dr_vars[1][j]);
    }
  }
  else {
    if (allVariables.size() != num_samples) {
      Cerr << "\nError in Analyzer::variance_based_decomp(): Expected "
	   << num_samples << " variables sets; received "
	   << allVariables.size() << std::endl;
      abort_handler(-1);
    }
    for (j=0; j<num_samples; ++j) {
      const Variables& all_vars_j = allVariables[j];
      if (ncont)
	copy_data(all_vars_j.continuous_variables(),    total_c_vars[1][j]);
      if (ndiscint)
	copy_data(all_vars_j.discrete_int_variables(),  total_di_vars[1][j]);
      if (ndiscreal)
	copy_data(all_vars_j.discrete_real_variables(), total_dr_vars[1][j]);
    }
  }

  for (i=2; i<ndimtotal+2; ++i) {
    total_c_vars[i]  = total_c_vars[1];
    total_di_vars[i] = total_di_vars[1];
    total_dr_vars[i] = total_dr_vars[1];
  }
 
  for (i=0; i<ncont; ++i)
    for (j=0; j<num_samples; ++j)   
      total_c_vars[i+2][j][i] = total_c_vars[0][j][i];

  for (i=0; i<ndiscint; ++i)
    for (j=0; j<num_samples; ++j)   
      total_di_vars[ncont+i+2][j][i] = total_di_vars[0][j][i];
  
  for (i=0; i<ndiscreal; ++i)
    for (j=0; j<num_samples; ++j)   
      total_dr_vars[ncont+ndiscint+i+2][j][i] = total_dr_vars[0][j][i];
  
  // call evaluate parameter sets (ncont)*num_samples to get data
  //WJB - ToDo: confer with MSE: Array<Real2DArray> total_fn_vals(numFunctions);
  multi_array<Real,3>
    total_fn_vals(extents[numFunctions][ndimtotal+2][num_samples]);

  for (i=0; i<ndimtotal+2; ++i) {
    if (compactMode)
      for (j=0; j<num_samples; ++j) {
	Real* sample_j = allSamples[j];
	if (ncont)
	  copy_data(total_c_vars[i][j],  sample_j,                  ncont);
	if (ndiscint) {
	  const IntVector& t_div_ij = total_di_vars[i][j];
	  for (k=0; k<ndiscint; ++k)
	    sample_j[ncont+k] = (Real)t_div_ij[k];
	}
	if (ndiscreal)
	  copy_data(total_dr_vars[i][j], &sample_j[ncont+ndiscint], ndiscreal);
      }
    else
      for (j=0; j<num_samples; ++j) {   
	Variables& all_vars_j = allVariables[j];
	if (ncont)
	  all_vars_j.continuous_variables(total_c_vars[i][j]);
	if (ndiscint)
	  all_vars_j.discrete_int_variables(total_di_vars[i][j]);
	if (ndiscreal)
	  all_vars_j.discrete_real_variables(total_dr_vars[i][j]);
      }

    // evaluate each of the parameter sets in allVariables
    evaluate_parameter_sets(iteratedModel, true, false);
    if (allResponses.size() != num_samples) {
      Cerr << "\nError in Analyzer::variance_based_decomp(): expected "
	   << num_samples << " responses; received " << allResponses.size()
	   << std::endl;
      abort_handler(-1);
    }
    IntRespMCIter r_it;
    for (k=0; k<numFunctions; ++k)
      for (r_it=allResponses.begin(), j=0; j<num_samples; ++r_it, ++j)
	total_fn_vals[k][i][j] = r_it->second.function_value(k);
  }
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
  S4.resize(numFunctions); T4.resize(numFunctions);
  for (k=0; k<numFunctions; ++k) {
//    S1[k].resize(ndimtotal);
//    T1[k].resize(ndimtotal);
//    S2[k].resize(ndimtotal);
//    T2[k].resize(ndimtotal);
//    S3[k].resize(ndimtotal);
//    T3[k].resize(ndimtotal);
      S4[k].resize(ndimtotal);
      T4[k].resize(ndimtotal);
  }
  multi_array<Real,3>
    total_norm_vals(extents[numFunctions][ndimtotal+2][num_samples]);

  // Loop over number of responses to obtain sensitivity indices for each
  for (k=0; k<numFunctions; ++k) {
 
#ifdef DEBUG
    Cout << "Total Samples\n"; 
    for (i=0; i<ndimtotal+2; ++i) {
      for (j=0; j<num_samples; ++j) {
	Cout << "Cvar " << j << '\n' << total_c_vars[i][j] << " " ;
	Cout << "DRVar " << j << '\n' << total_dr_vars[i][j] << " " ;
	Cout << "DIVar " << j << '\n' << total_di_vars[i][j] << " " ;
	Cout << "Response " << i << '\n' << total_fn_vals[k][i][j] << '\n';
      }
    }
#endif

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
      for(i=0; i<(ndimtotal+2); i++){ 
        total_norm_vals[k][i][j]=total_fn_vals[k][i][j];
        overall_mean += total_norm_vals[k][i][j];
      } 
    }
   
    overall_mean /= Real((num_samples)* (ndimtotal+2));
    for (j=0; j<num_samples; j++)
      for(i=0; i<(ndimtotal+2); i++)
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
    for (i=0; i<ndimtotal; i++) {
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
  if (!iteratedModel.parallel_library().command_line_user_modes())
    return;

  const String& filename =
    iteratedModel.parallel_library().command_line_pre_run_output();
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
  // consider passing precision to helper functions instead of using global
  int save_precision;
  if (writePrecision == 0) {
    save_precision = write_precision;
    write_precision = 17;
  }
 
  bool active_only = compactMode;  // compactMode uses only active
  bool response_labels = true;     // write response labels for user
  bool annotated = true;
  if (annotated)
    TabularIO::write_header_tabular(tabular_file, "eval_id", 
				    iteratedModel.current_variables(), 
				    iteratedModel.current_response(),
				    active_only,
				    response_labels);
  tabular_file << std::setprecision(write_precision) 
	       << std::resetiosflags(std::ios::floatfield);

  // TODO: consider helper to output allSamples or allVariables directly
  // TODO: consider supplementing compactMode with inactive vars from model
  for (size_t eval_index = 0; eval_index < num_evals; eval_index++) {
    if(annotated)
      tabular_file << std::setw(8) << eval_index + 1 << ' ';
    if (compactMode) {
      // allSamples num_vars x num_evals, so each col becomes tabular file row
      write_data_tabular(tabular_file, 
			 getCol(Teuchos::View, allSamples, (int) eval_index));
    }
    else
      allVariables[eval_index].write_tabular(tabular_file);
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
  if (!iteratedModel.parallel_library().command_line_user_modes())
    return;

  const String& filename = 
    iteratedModel.parallel_library().command_line_post_run_input();
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

  std::ifstream tabular_file;
  TabularIO::open_file(tabular_file, filename, "post-run input");
  bool annotated = true;

  if (outputLevel > NORMAL_OUTPUT)
    Cout << "\nAttempting to read " << num_evals << " samples from file "
	 << filename << "..." << std::endl;
  
  if (annotated)
    TabularIO::read_header_tabular(tabular_file); // discard header with labels

  if (compactMode)
    allSamples.shapeUninitialized(num_vars, num_evals);
  else 
    allVariables.resize(num_evals);
  allResponses.clear();

  // now read variables and responses (minimal error checking for now)
  int cntr = -1; // use negative ids for file import
  for (size_t i=0; i<num_evals; ++i, --cntr) {
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "   reading sample " << i << std::endl;

    if (!compactMode)
      allVariables[i] = iteratedModel.current_variables().copy();
    allResponses[cntr] = iteratedModel.current_response().copy();

    try {
      if (annotated) {
	size_t discard_row_label;
	tabular_file >> discard_row_label;
      }
      if (compactMode) {
	// this doesn't work because getCol is returning a copy;
	// could construct a View...
	//read_data_tabular(tabular_file, 
	//		  Teuchos::getCol(Teuchos::View, allSamples, (int)i));
	for (size_t var_index = 0; var_index < num_vars; ++var_index) 
	  tabular_file >> allSamples(var_index, i);
      }
      else
	allVariables[i].read_tabular(tabular_file);
      allResponses[cntr].read_tabular(tabular_file);
    }
    catch (const std::ios_base::failure& failorbad_except) {
      Cerr << "\nError: insufficient data in post-run input file;\n       "
	   << "expected " << num_evals << " samples, read " << i << std::endl;
      abort_handler(-1);
    }
    if (compactMode)
      update_best(allSamples[i], i+1, allResponses[cntr]);
    else
      update_best(allVariables[i], i+1, allResponses[cntr]);
  }
  
  tabular_file.close();
  if (outputLevel > QUIET_OUTPUT)
    Cout << "\nPost-run phase initialized: variables / responses read from "
	 << "tabular file " << filename << ".\n" << std::endl;
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
  s.setf(std::ios::scientific);
  //s << "Variance Based Decomposition Sensitivity Indices\n"
  //  << "These Sobol' indices measure the importance of the uncertain input\n"
  //  << "variables in determining the uncertainty (variance) of the output.\n"
  //  << "Si measures the main effect for variable i itself, while Ti\n"
  //  << "measures the total effect (including the interaction effects\n" 
  //  << "of variable i with other uncertain variables.)\n";
  s << "\nGlobal sensitivity indices for each response function:\n";

  size_t i, k, offset;
  for (k=0; k<numFunctions; ++k) {
    s << resp_labels[k] << " Sobol indices:\n"; 
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
    const Variables&  best_vars = best_pr.prp_parameters();
    const RealVector& best_fns  = best_pr.prp_response().function_values();
    s << "<<<<< Best parameters          ";
    if (num_best_map > 1) s << "(set " << i << ") ";
    s << "=\n" << best_vars;
    num_fns = best_fns.length();
    if (numObjFns) {
      if (numObjFns > 1) s << "<<<<< Best objective functions ";
      else               s << "<<<<< Best objective function  ";
      if (num_best_map > 1) s << "(set " << i << ") "; s << "=\n";
      write_data_partial(s, 0, numObjFns, best_fns);
      offset = numObjFns;
    }
    else if (numLSqTerms) {
      s << "<<<<< Best residual terms      ";
      if (num_best_map > 1) s << "(set " << i << ") "; s << "=\n";
      write_data_partial(s, 0, numLSqTerms, best_fns);
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
  Cerr << "Error: Analyzer lacking redefinition of virtual get_parameter_sets()"
       << " function.\n       This analyzer does not support parameter sets."
       << std::endl;
  abort_handler(-1);
}

} // namespace Dakota
