/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ParamStudy
//- Description: Implementation code for the ParamStudy class
//- Owner:       Mike Eldred
#include <exception>
#include "dakota_system_defs.hpp"
#include "dakota_tabular_io.hpp"
#include "ParamStudy.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "PolynomialApproximation.hpp"

static const char rcsId[]="@(#) $Id: ParamStudy.cpp 7024 2010-10-16 01:24:42Z mseldre $";

//#define DEBUG


namespace Dakota {

ParamStudy::ParamStudy(ProblemDescDB& problem_db, Model& model):
  PStudyDACE(problem_db, model)
{
  // use allVariables instead of default allSamples
  compactMode = false;

  // Extract specification from ProblemDescDB, perform sanity checking, and
  // compute/estimate maxEvalConcurrency.
  bool err_flag = false;
  switch (methodName) {
  case LIST_PARAMETER_STUDY: {
    const RealVector& pt_list
      = probDescDB.get_rv("method.parameter_study.list_of_points");
    if (pt_list.empty()) {
      const String& pt_fname
	= probDescDB.get_string("method.pstudy.import_file");
      unsigned short tabular_format
	= probDescDB.get_ushort("method.pstudy.import_format");
      bool active_only
	= probDescDB.get_bool("method.pstudy.import_active_only");
      if (load_distribute_points(pt_fname, tabular_format, active_only))
	err_flag = true;
    }
    else if (distribute_list_of_points(pt_list))
      err_flag = true;
    break;
  }
  case VECTOR_PARAMETER_STUDY: {
    const RealVector& step_vector
      = probDescDB.get_rv("method.parameter_study.step_vector");
    if (step_vector.empty()) { // final_point & num_steps spec.
      // check length and distribute
      if (check_final_point(
	  probDescDB.get_rv("method.parameter_study.final_point")))
	err_flag = true;
      // check value
      if (check_num_steps(
	  probDescDB.get_int("method.parameter_study.num_steps")))
	err_flag = true;
      // precompute steps (using construct-time initialPoint) and perform error
      // checks only if in check mode; else avoid additional overhead and rely
      // on run-time checks for run-time initialPoint.
      if (numSteps && parallelLib.command_line_check()) {
	initialCVPoint  = iteratedModel.continuous_variables();      // view
	initialDIVPoint = iteratedModel.discrete_int_variables();    // view
	initialDSVPoint.resize(boost::extents[numDiscreteStringVars]);
	initialDSVPoint = iteratedModel.discrete_string_variables(); // copy
	initialDRVPoint = iteratedModel.discrete_real_variables();   // view
	final_point_to_step_vector(); // covers check_ranges_sets(numSteps)
      }
    }
    else { // step_vector & num_steps spec.
      // check length and distribute
      if (check_step_vector(step_vector))
	err_flag = true;
       // check value
     if (check_num_steps(
	  probDescDB.get_int("method.parameter_study.num_steps")))
	err_flag = true;
      // discrete initial pts needed for check_sets(); reassigned in pre-run
      initialDIVPoint = iteratedModel.discrete_int_variables();    // view
      initialDSVPoint.resize(boost::extents[numDiscreteStringVars]);
      initialDSVPoint = iteratedModel.discrete_string_variables(); // copy
      initialDRVPoint = iteratedModel.discrete_real_variables();   // view
      if (check_ranges_sets(numSteps))
	err_flag = true;
    }
    break;
  }
  case CENTERED_PARAMETER_STUDY:
    if (check_step_vector(
	probDescDB.get_rv("method.parameter_study.step_vector")))
      err_flag = true;
    if (check_steps_per_variable(
	probDescDB.get_iv("method.parameter_study.steps_per_variable")))
      err_flag = true;
    initialDIVPoint = iteratedModel.discrete_int_variables();    // view
    initialDSVPoint.resize(boost::extents[numDiscreteStringVars]);
    initialDSVPoint = iteratedModel.discrete_string_variables(); // copy
    initialDRVPoint = iteratedModel.discrete_real_variables();   // view
    if (check_ranges_sets(contStepsPerVariable, discIntStepsPerVariable,
			  discStringStepsPerVariable, discRealStepsPerVariable))
      err_flag = true;
    break;
  case MULTIDIM_PARAMETER_STUDY:
    if (check_variable_partitions(probDescDB.get_usa("method.partitions")))
      err_flag = true;
    if (check_finite_bounds())
      err_flag = true;
    // precompute steps (using construct-time bounds) and perform error checks
    // only if in check mode; else avoid additional overhead and rely on
    // run-time checks for run-time bounds.
    if (parallelLib.command_line_check())
      distribute_partitions();
    break;
  default:
    Cerr << "\nError: bad methodName (" << method_enum_to_string(methodName)
	 << ") in ParamStudy constructor." << std::endl;
    err_flag = true;
  }
  if (err_flag)
    abort_handler(METHOD_ERROR);

  maxEvalConcurrency *= numEvals;
}

bool ParamStudy::resize()
{
  bool parent_reinit_comms = PStudyDACE::resize();

  // TODO:  To get resize() working, move contents of ParamStudy::pre_run()
  //        to ParamStudy::initialize_run() before Analyzer::initialize_run()
  //        is called. This populates allVariables. If the model is a
  //        RecastModel, call inverse_transform_variables() on each entry in
  //        allVariables to resize and transform to the RecastModel. This call
  //        to inverse_transform_variables() must occur after
  //        Analyzer::initialize_run(). Also in ActiveSubspaceModel,
  //        inverse_transform_variables() is not yet implemented.

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}


void ParamStudy::pre_run()
{
  Analyzer::pre_run();

  // Capture any changes in initialCVPoint resulting from the strategy layer's
  // passing of best variable info between iterators.  If no such variable 
  // passing has occurred, then this reassignment is merely repetitive of the 
  // one in the ParamStudy constructor.  If there is a final_point 
  // specification, then contStepVector and numSteps must be (re)computed.
  const Variables& vars = iteratedModel.current_variables();
  const SharedVariablesData& svd = vars.shared_data();
  if (methodName == VECTOR_PARAMETER_STUDY ||
      methodName == CENTERED_PARAMETER_STUDY) {
    copy_data(vars.continuous_variables(),      initialCVPoint);  // copy
    copy_data(vars.discrete_int_variables(),    initialDIVPoint); // copy
    initialDSVPoint.resize(boost::extents[numDiscreteStringVars]);
    initialDSVPoint = vars.discrete_string_variables();           // copy
    copy_data(vars.discrete_real_variables(),   initialDRVPoint); // copy
  }

  size_t av_size = allVariables.size();
  if (av_size != numEvals) {
    allVariables.resize(numEvals);
    for (size_t i=av_size; i<numEvals; ++i)
      allVariables[i] = vars.copy();
    if ( outputLevel > SILENT_OUTPUT &&
	 ( methodName == VECTOR_PARAMETER_STUDY ||
	   methodName == CENTERED_PARAMETER_STUDY ) )
      allHeaders.resize(numEvals);
  }

  switch (methodName) {
  case LIST_PARAMETER_STUDY:
    if (outputLevel > SILENT_OUTPUT)
      Cout << "\nList parameter study for " << numEvals << " samples\n\n";
    sample();
    break;
  case VECTOR_PARAMETER_STUDY:
    if (!contStepVector.empty()       || !discIntStepVector.empty() ||
	!discStringStepVector.empty() || !discRealStepVector.empty()) {
      // step_vector & num_steps
      if (outputLevel > SILENT_OUTPUT) {
	Cout << "\nVector parameter study for " << numSteps
	     << " steps starting from\n";
	write_ordered(Cout, svd.active_components_totals(), initialCVPoint,
		      initialDIVPoint, initialDSVPoint, initialDRVPoint);
	Cout << "with a step vector of\n";
	write_ordered(Cout, svd.active_components_totals(), contStepVector,
		      discIntStepVector, discStringStepVector,
		      discRealStepVector);
	Cout << '\n';
      }
    }
    else { // final_point & num_steps
      if (outputLevel > SILENT_OUTPUT) {
	Cout << "\nVector parameter study from\n";
	write_ordered(Cout, svd.active_components_totals(), initialCVPoint,
		      initialDIVPoint, initialDSVPoint, initialDRVPoint);
	Cout << "to\n";
	write_ordered(Cout, svd.active_components_totals(), finalCVPoint,
		      finalDIVPoint, finalDSVPoint, finalDRVPoint);
	Cout << "using " << numSteps << " steps\n\n";
      }
      if (numSteps) // define step vectors from initial, final, & num steps
	final_point_to_step_vector();
    }
    vector_loop();
    break;
  case CENTERED_PARAMETER_STUDY:
    if (outputLevel > SILENT_OUTPUT) {
      Cout << "\nCentered parameter study with steps per variable\n";
      write_ordered(Cout, svd.active_components_totals(), contStepsPerVariable,
		    discIntStepsPerVariable, discStringStepsPerVariable,
		    discRealStepsPerVariable);
      Cout << "and increments of\n";
      write_ordered(Cout, svd.active_components_totals(), contStepVector,
		    discIntStepVector, discStringStepVector,
		    discRealStepVector);
      Cout << "with the following center point:\n";
      write_ordered(Cout, svd.active_components_totals(), initialCVPoint,
		    initialDIVPoint, initialDSVPoint, initialDRVPoint);
      Cout << '\n';
    }
    centered_loop();
    break;
  case MULTIDIM_PARAMETER_STUDY:
    if (outputLevel > SILENT_OUTPUT) {
      Cout << "\nMultidimensional parameter study variable partitions of\n";
      write_ordered(Cout, svd.active_components_totals(), contVarPartitions,
		    discIntVarPartitions, discStringVarPartitions,
		    discRealVarPartitions);
    }
    distribute_partitions();
    multidim_loop();
    break;
  default:
    Cerr << "\nError: bad methodName (" << method_enum_to_string(methodName)
	 << ") in ParamStudy::pre_run()." << std::endl;
    abort_handler(METHOD_ERROR);
  }
}


void ParamStudy::core_run()
{

  archive_allocate_sets();
  // perform the evaluations; multidim exception
  bool log_resp_flag = (methodName == MULTIDIM_PARAMETER_STUDY)
    ? (!subIteratorFlag) : false;
  bool log_best_flag = (numObjFns || numLSqTerms); // opt or NLS data set
  evaluate_parameter_sets(iteratedModel, log_resp_flag, log_best_flag);
}

void ParamStudy::archive_model_variables(const Model& model, size_t idx) const
{
  // Let's try to write resultsDB stuff here - RWH
  if(resultsDB.active())
  {
    const RealVector& c_vars  = model.continuous_variables();
    const IntVector & di_vars = model.discrete_int_variables();
    StringMultiArrayConstView ds_vars = model.discrete_string_variables();
    const RealVector& dr_vars = model.discrete_real_variables();

    if( numContinuousVars )
      resultsDB.insert_into
	( run_identifier(),
	  {String("parameter_sets"), String("continuous_variables")},
	  c_vars,  idx );
    if( numDiscreteIntVars )
      resultsDB.insert_into
	( run_identifier(),
	  {String("parameter_sets"), String("discrete_integer_variables")},
	  di_vars, idx );
    if( numDiscreteStringVars )
      resultsDB.insert_into
	( run_identifier(),
	  {String("parameter_sets"), String("discrete_string_variables")},
	  ds_vars, idx );
    if( numDiscreteRealVars )
      resultsDB.insert_into
	( run_identifier(),
	  {String("parameter_sets"), String("discrete_real_variables")},
	  dr_vars, idx );

    if (methodName == CENTERED_PARAMETER_STUDY)
      archive_cps_vars(model, idx);
  }
}


void ParamStudy::archive_model_response(const Response& response, size_t idx) const
{
  // Let's try to write resultsDB stuff here - RWH
  if(resultsDB.active())
  {
    const RealVector& resp_vec = response.function_values();
    resultsDB.insert_into
      ( run_identifier(),
	{String("parameter_sets"), String("responses")}, resp_vec, idx );

    if (methodName == CENTERED_PARAMETER_STUDY)
      archive_cps_resp(response, idx);
  }
}


void ParamStudy::archive_allocate_sets() const
{
  if(resultsDB.active())
  {
    size_t num_evals = (compactMode) ? allSamples.numCols() : allVariables.size();

    StringMultiArrayConstView cv_labels
                = iteratedModel.continuous_variable_labels();
    StringMultiArrayConstView div_labels
                = iteratedModel.discrete_int_variable_labels();
    StringMultiArrayConstView dsv_labels
                = iteratedModel.discrete_string_variable_labels();
    StringMultiArrayConstView drv_labels
                = iteratedModel.discrete_real_variable_labels();

    const StringArray& resp_labels 
                = iteratedModel.response_labels();

    if( numContinuousVars ) {
      DimScaleMap scales;
      scales.emplace(1, StringScale("variables", cv_labels));
      resultsDB.allocate_matrix
	( run_identifier(),
	  {String("parameter_sets"), String("continuous_variables")},
          Dakota::ResultsOutputType::REAL, num_evals, numContinuousVars,
	  scales );
    }
    if( numDiscreteIntVars ) {
      DimScaleMap scales;
      scales.emplace(1, StringScale("variables", div_labels));
      resultsDB.allocate_matrix
	( run_identifier(),
	  {String("parameter_sets"), String("discrete_integer_variables")},
          Dakota::ResultsOutputType::INTEGER, num_evals, numDiscreteIntVars,
	  scales );
    }
    if( numDiscreteStringVars ) {
      DimScaleMap scales;
      scales.emplace(1, StringScale("variables", dsv_labels));
      resultsDB.allocate_matrix
	( run_identifier(),
	  {String("parameter_sets"), String("discrete_string_variables")},
          Dakota::ResultsOutputType::STRING, num_evals, numDiscreteStringVars,
	  scales );
    }
    if( numDiscreteRealVars ) {
      DimScaleMap scales;
      scales.emplace(1, StringScale("variables", drv_labels));
      resultsDB.allocate_matrix
	( run_identifier(),
	  {String("parameter_sets"), String("discrete_real_variables")},
          Dakota::ResultsOutputType::REAL, num_evals, numDiscreteRealVars,
	  scales );
    }
    DimScaleMap scales;
    scales.emplace(1, StringScale("responses", resp_labels));
    resultsDB.allocate_matrix
      ( run_identifier(), {String("parameter_sets"), String("responses")},
        Dakota::ResultsOutputType::REAL, num_evals, numFunctions, scales );

    if (methodName == CENTERED_PARAMETER_STUDY)
      archive_allocate_cps();
  }

}

void ParamStudy::post_input()
{
  // call convenience function from Analyzer
  read_variables_responses(numEvals, numContinuousVars + numDiscreteIntVars +
			   numDiscreteStringVars + numDiscreteRealVars);
}


void ParamStudy::post_run(std::ostream& s)
{
  bool log_resp_flag = (!subIteratorFlag);
  if (methodName == MULTIDIM_PARAMETER_STUDY && log_resp_flag) {
    pStudyDACESensGlobal.compute_correlations(allVariables, allResponses, 
      iteratedModel.discrete_set_string_values()); // to map string variable
                                                   // values back to indices
    if(resultsDB.active()) {
      pStudyDACESensGlobal.archive_correlations(run_identifier(), resultsDB,
                                        iteratedModel.ordered_labels(),
                                        iteratedModel.response_labels());
    }

  }

  Analyzer::post_run(s);
}


void ParamStudy::sample()
{
  // populate allVariables
  for (size_t i=0; i<numEvals; ++i) {
    if (numContinuousVars)
      allVariables[i].continuous_variables(listCVPoints[i]);
    if (numDiscreteIntVars)
      allVariables[i].discrete_int_variables(listDIVPoints[i]);
    if (numDiscreteStringVars)
      allVariables[i].discrete_string_variables(
	listDSVPoints[boost::indices[i][idx_range(0, numDiscreteStringVars)]]);
    if (numDiscreteRealVars)
      allVariables[i].discrete_real_variables(listDRVPoints[i]);
  }
  // free up redundant memory
  listCVPoints.clear();
  listDIVPoints.clear();
  listDSVPoints.resize(boost::extents[0][0]);
  listDRVPoints.clear();
}


void ParamStudy::vector_loop()
{
  // Steps along a n-dimensional vector through numSteps additions of
  // continuous/discrete step vectors.  The step is an absolute step defining
  // magnitude & direction.  The number of fn. evaluations in the study is
  // numSteps + 1 since the initial point is also evaluated.

  const BitArray&      di_set_bits = iteratedModel.discrete_int_sets();
  const IntSetArray&    dsi_values = iteratedModel.discrete_set_int_values();
  const StringSetArray& dss_values = iteratedModel.discrete_set_string_values();
  const RealSetArray&   dsr_values = iteratedModel.discrete_set_real_values();
  size_t i, j, dsi_cntr;

  for (i=0; i<=numSteps; ++i) {
    Variables& vars = allVariables[i];

    // active continuous
    for (j=0; j<numContinuousVars; ++j)
      c_step(j, i, vars);

    // active discrete int: ranges and sets
    for (j=0, dsi_cntr=0; j<numDiscreteIntVars; ++j)
      if (di_set_bits[j]) dsi_step(j, i, dsi_values[dsi_cntr++], vars);
      else                dri_step(j, i, vars);

    // active discrete string: sets only
    for (j=0; j<numDiscreteStringVars; ++j)
      dss_step(j, i, dss_values[j], vars);

    // active discrete real: sets only
    for (j=0; j<numDiscreteRealVars; ++j)
      dsr_step(j, i, dsr_values[j], vars);

    // store each output header in allHeaders
    if (outputLevel > SILENT_OUTPUT) {
      String& h_string = allHeaders[i];
      h_string.clear();
      if (iteratedModel.asynch_flag())
	h_string += "\n\n";
      if (numSteps == 0) // Allow numSteps == 0 case
	h_string += ">>>>> Initial_point only (no steps)\n";
      h_string += ">>>>> Vector parameter study evaluation for ";
      h_string += std::to_string(i*100./numSteps);
      h_string += "% along vector\n";
    }
  }
}


void ParamStudy::centered_loop()
{
  size_t k, cntr = 0, dsi_cntr = 0;
  String cv_str("cv"), div_str("div"), dsv_str("dsv"), drv_str("drv");

  // Always evaluate center point, even if steps_per_variable = 0
  if (outputLevel > SILENT_OUTPUT)
    allHeaders[cntr] = (iteratedModel.asynch_flag()) ?
      "\n\n>>>>> Centered parameter study evaluation for center point\n" :
      ">>>>> Centered parameter study evaluation for center point\n";
  if (numContinuousVars)
    allVariables[cntr].continuous_variables(initialCVPoint);
  if (numDiscreteIntVars)
    allVariables[cntr].discrete_int_variables(initialDIVPoint);
  if (numDiscreteStringVars)
    allVariables[cntr].discrete_string_variables(
      initialDSVPoint[boost::indices[idx_range(0, numDiscreteStringVars)]]);
  if (numDiscreteRealVars)
    allVariables[cntr].discrete_real_variables(initialDRVPoint);
  ++cntr;

  // Evaluate +/- steps for each continuous variable
  for (k=0; k<numContinuousVars; ++k) {
    int i, num_steps_k = contStepsPerVariable[k];
    for (i=-num_steps_k; i<=num_steps_k; ++i)
      if (i) {
	Variables& vars = allVariables[cntr];
	reset(vars); c_step(k, i, vars);
	if (outputLevel > SILENT_OUTPUT) centered_header(cv_str, k, i, cntr);
	++cntr;
      }
  }

  // Evaluate +/- steps for each discrete int variable
  const BitArray&   di_set_bits = iteratedModel.discrete_int_sets();
  const IntSetArray& dsi_values = iteratedModel.discrete_set_int_values();
  for (k=0; k<numDiscreteIntVars; ++k) {
    int i, num_steps_k = discIntStepsPerVariable[k];
    if (di_set_bits[k]) {
      const IntSet& dsi_vals_k = dsi_values[dsi_cntr];
      for (i=-num_steps_k; i<=num_steps_k; ++i)
	if (i) {
	  Variables& vars = allVariables[cntr];
	  reset(vars); dsi_step(k, i, dsi_vals_k, vars);
	  if (outputLevel > SILENT_OUTPUT) centered_header(div_str, k, i, cntr);
	  ++cntr;
	}
      ++dsi_cntr;
    }
    else {
      for (i=-num_steps_k; i<=num_steps_k; ++i)
	if (i) {
	  Variables& vars = allVariables[cntr];
	  reset(vars); dri_step(k, i, vars);
	  if (outputLevel > SILENT_OUTPUT) centered_header(div_str, k, i, cntr);
	  ++cntr;
	}
    }
  }

  // Evaluate +/- steps for each discrete string variable
  const StringSetArray& dss_values = iteratedModel.discrete_set_string_values();
  for (k=0; k<numDiscreteStringVars; ++k) {
    int i, num_steps_k = discStringStepsPerVariable[k];
    const StringSet& dss_vals_k = dss_values[k];
    for (i=-num_steps_k; i<=num_steps_k; ++i)
      if (i) {
	Variables& vars = allVariables[cntr];
	reset(vars); dss_step(k, i, dss_vals_k, vars);
	if (outputLevel > SILENT_OUTPUT) centered_header(dsv_str, k, i, cntr);
	++cntr;
      }
  }

  // Evaluate +/- steps for each discrete real variable
  const RealSetArray& dsr_values = iteratedModel.discrete_set_real_values();
  for (k=0; k<numDiscreteRealVars; ++k) {
    int i, num_steps_k = discRealStepsPerVariable[k];
    const RealSet& dsr_vals_k = dsr_values[k];
    for (i=-num_steps_k; i<=num_steps_k; ++i)
      if (i) {
	Variables& vars = allVariables[cntr];
	reset(vars); dsr_step(k, i, dsr_vals_k, vars);
	if (outputLevel > SILENT_OUTPUT) centered_header(drv_str, k, i, cntr);
	++cntr;
      }
  }
}


void ParamStudy::multidim_loop()
{
  // Perform a multidimensional parameter study based on the number of 
  // partitions specified for each variable.

  const BitArray&      di_set_bits = iteratedModel.discrete_int_sets();
  const IntSetArray&    dsi_values = iteratedModel.discrete_set_int_values();
  const StringSetArray& dss_values = iteratedModel.discrete_set_string_values();
  const RealSetArray&   dsr_values = iteratedModel.discrete_set_real_values();
  size_t i, j, p_cntr, dsi_cntr,
    num_c_di_vars    = numContinuousVars + numDiscreteIntVars,
    num_c_di_ds_vars = num_c_di_vars + numDiscreteStringVars,
    num_vars = num_c_di_ds_vars + numDiscreteRealVars;
  UShortArray multidim_indices(num_vars, 0), partition_limits(num_vars);
  copy_data_partial(contVarPartitions, partition_limits, 0);
  copy_data_partial(discIntVarPartitions, partition_limits, numContinuousVars);
  copy_data_partial(discStringVarPartitions, partition_limits, num_c_di_vars);
  copy_data_partial(discRealVarPartitions, partition_limits, num_c_di_ds_vars);

  for (i=0; i<numEvals; ++i) {
    Variables& vars = allVariables[i];
    p_cntr = 0;
    // active continuous
    for (j=0; j<numContinuousVars; ++j, ++p_cntr)
      c_step(j, multidim_indices[p_cntr], vars);
    // active discrete int: ranges and sets
    for (j=0, dsi_cntr=0; j<numDiscreteIntVars; ++j, ++p_cntr)
      if (di_set_bits[j])
	dsi_step(j, multidim_indices[p_cntr], dsi_values[dsi_cntr++], vars);
      else
	dri_step(j, multidim_indices[p_cntr], vars);
    // active discrete string: sets only
    for (j=0; j<numDiscreteStringVars; ++j, ++p_cntr)
      dss_step(j, multidim_indices[p_cntr], dss_values[j], vars);
    // active discrete real: sets only
    for (j=0; j<numDiscreteRealVars; ++j, ++p_cntr)
      dsr_step(j, multidim_indices[p_cntr], dsr_values[j], vars);
    // increment the multidimensional index set
    Pecos::SharedPolyApproxData::increment_indices(multidim_indices,
						   partition_limits, true);
  }
}


/** Load from file and distribute points; using this function to
    manage construction of the temporary arrays.  Historically all
    data was read as a real (mixture of values and indices), but now
    points_file is valued-based (reals, integers, strings) so file
    input matches tabular data output.  Return false on success. */
bool ParamStudy::
load_distribute_points(const String& points_filename, 
		       unsigned short tabular_format,
		       bool active_only)
{
  bool err = false;

  // don't know the size until the file is read, so the reader grows
  // the containers as the read takes place

  // the easiest way to read is with a variables object
  // read all variables in spec order into a temporary Variables
  Variables vars(iteratedModel.current_variables().copy());

  // then map the active data from that variables object into the
  // list*Points arrays and validate it
 
  // TODO: validate the read values of inactive variables

  // Could read into these dynamically or into a temporary and then allocate
  numEvals = TabularIO::
    read_data_tabular(points_filename, "List Parameter Study", 
		      listCVPoints, listDIVPoints, listDSVPoints, listDRVPoints,
		      tabular_format, active_only, 
		      iteratedModel.current_variables().copy());
  if (numEvals == 0) err = true;


  // validation of data: consider moving reader into this class and
  // validating while reading...
  for (size_t i=0; i<numEvals; ++i) {

    // validate continuous values read
    const RealVector& c_lb = iteratedModel.continuous_lower_bounds();
    const RealVector& c_ub = iteratedModel.continuous_upper_bounds();

    for (size_t j=0; j<numContinuousVars; ++j)
      if (listCVPoints[i][j] < c_lb[j] || listCVPoints[i][j] > c_ub[j]) {
	Cerr << "\nError: list value " << listCVPoints[i][j] 
	     << " outside bounds for continuous variable " << j+1 << '.' 
	     << std::endl;
	err = true;
      }

    // validate discrete integers (sets and ranges) read
    const BitArray& di_set_bits = iteratedModel.discrete_int_sets();
    const IntSetArray& dsi_vals = iteratedModel.discrete_set_int_values();
    const IntVector& di_lb = iteratedModel.discrete_int_lower_bounds();
    const IntVector& di_ub = iteratedModel.discrete_int_upper_bounds();

    for (size_t j=0, dsi_cntr=0; j<numDiscreteIntVars; ++j)
      if (di_set_bits[j]) {
	// set values
	if (set_value_to_index(listDIVPoints[i][j], dsi_vals[dsi_cntr]) 
	    == _NPOS) {
	  Cerr << "\nError: list value " << listDIVPoints[i][j] 
	       << " not admissble for discrete int set " << dsi_cntr+1 << '.'
	       << std::endl;
	  err = true;
	}
	++dsi_cntr;
      }
      else {
	// range values: validate bounds
	if (listDIVPoints[i][j] < di_lb[j] || listDIVPoints[i][j] > di_ub[j]) {
	  Cerr << "\nError: list value " << listDIVPoints[i][j] 
	       << " outside bounds for discrete int range variable " << j+1 
	       << '.' << std::endl;
	  err = true;
	}
      }

    // validate discrete string sets read
    const StringSetArray& dss_vals = iteratedModel.discrete_set_string_values();
    for (size_t j=0; j<numDiscreteStringVars; ++j)
      if (set_value_to_index(listDSVPoints[i][j], dss_vals[j]) == _NPOS) {
        Cerr << "\nError: list value " << listDSVPoints[i][j] 
	     << " not admissible for discrete string set " << j+1 << '.' 
	     << std::endl;
        err = true;
      }

    const RealSetArray& dsr_vals = iteratedModel.discrete_set_real_values();
    for (size_t j=0; j<numDiscreteRealVars; ++j)
      if (set_value_to_index(listDRVPoints[i][j], dsr_vals[j]) == _NPOS) {
        Cerr << "\nError: list value " << listDRVPoints[i][j] 
	     << " not admissible for discrete real set " << j+1 << '.' 
	     << std::endl;
        err = true;
      }

  } // for each eval

  return err;

}


/** Parse list of points into typed data containers; list_of_pts will
    contain values for continuous and discrete integer range, but
    indices for all discrete set types (int, string, real) */
bool ParamStudy::distribute_list_of_points(const RealVector& list_of_pts)
{
  size_t i, j, dsi_cntr, start, len_lop = list_of_pts.length(),
    num_vars = numContinuousVars     + numDiscreteIntVars
             + numDiscreteStringVars + numDiscreteRealVars;
  if (len_lop % num_vars) {
    Cerr << "\nError: length of list_of_points ("  << len_lop
	 << ") must be evenly divisable among number of active variables ("
	 << num_vars << ")." << std::endl;
    return true;
  }
  numEvals = len_lop / num_vars;
  if (numContinuousVars)   listCVPoints.resize(numEvals);
  if (numDiscreteIntVars)  listDIVPoints.resize(numEvals);
  if (numDiscreteStringVars)
    listDSVPoints.resize(boost::extents[numEvals][numDiscreteStringVars]);
  if (numDiscreteRealVars) listDRVPoints.resize(numEvals);

  const BitArray&      di_set_bits = iteratedModel.discrete_int_sets();
  const IntSetArray&    dsi_values = iteratedModel.discrete_set_int_values();
  const StringSetArray& dss_values = iteratedModel.discrete_set_string_values();
  const RealSetArray&   dsr_values = iteratedModel.discrete_set_real_values();

  bool err = false;
  RealVector empty_rv; IntVector empty_iv; StringMultiArray empty_sa;
  for (i=0, start=0; i<numEvals; ++i) {
    RealVector& list_cv_i  = (numContinuousVars)  ? listCVPoints[i]  : empty_rv;
    IntVector&  list_div_i = (numDiscreteIntVars) ? listDIVPoints[i] : empty_iv;
    StringMultiArrayView list_dsv_i = (numDiscreteStringVars) ?
      listDSVPoints[boost::indices[i][idx_range(0, numDiscreteStringVars)]] :
      empty_sa[boost::indices[idx_range(0, 0)]];
    RealVector& list_drv_i = (numDiscreteRealVars) ?
      listDRVPoints[i] : empty_rv;
    IntVector div_combined, dsv_indices, drv_indices;

    // take a view of each sample and partition it into {c,di,dr} components
    RealVector all_sample(Teuchos::View, const_cast<Real*>(&list_of_pts[start]),
			  num_vars);
    // if list_of_pts contains range and set values:
    //distribute(all_sample, list_cv_i, list_div_i, list_dsv_i, list_drv_i);
    // if list_of_pts contains range values and set indices:
    distribute(all_sample, list_cv_i, div_combined, dsv_indices, drv_indices);
    start += num_vars;

    // Promote set indices to admissible set values
    if (numDiscreteIntVars) list_div_i.sizeUninitialized(numDiscreteIntVars);
    for (j=0, dsi_cntr=0; j<numDiscreteIntVars; ++j) {
      if (di_set_bits[j]) {
	// if set values:
	//if (set_value_to_index(list_div_i[j], dsi_values[dsi_cntr]) == _NPOS){
	//  Cerr << "\nError: list value " << list_div_i[j]<< " not admissible "
	//       << "for discrete int set " << dsi_cntr+1 << '.' << std::endl;
	//  err = true;
	//}
	// if set indices:
        try {
          list_div_i[j]
            = set_index_to_value(div_combined[j], dsi_values[dsi_cntr]);
        } catch(std::out_of_range e) {
          Cerr << e.what() << " for variable '" 
            << iteratedModel.discrete_int_variable_labels()[j] << "' in method '" 
            << method_id() << "'\n";
          abort_handler(-1);
        }
	++dsi_cntr;
      }
      else // range values
	list_div_i[j] = div_combined[j];
    }

    for (j=0; j<numDiscreteStringVars; ++j) {
      // if set values:
      //if (set_value_to_index(list_dsv_i[j], dss_values[j]) == _NPOS) {
      //  Cerr << "\nError: list value " << list_dsv_i[j] << " not admissible "
      //       << "for discrete string set " << j+1 << '.' << std::endl;
      //  err = true;
      //}
      // if set indices:
      try {
          list_dsv_i[j] = set_index_to_value(dsv_indices[j], dss_values[j]);
      } catch(std::out_of_range e) {
          Cerr << e.what() << " for variable '" 
            << iteratedModel.discrete_string_variable_labels()[j] << "' in method '" 
            << method_id() << "'\n";
          abort_handler(-1);
      }
    }

    if (numDiscreteRealVars) list_drv_i.sizeUninitialized(numDiscreteRealVars);
    for (j=0; j<numDiscreteRealVars; ++j) {
      // if set values:
      //if (set_value_to_index(list_drv_i[j], dsr_values[j]) == _NPOS) {
      //  Cerr << "\nError: list value " << list_drv_i[j] << " not admissible "
      //       << "for discrete real set " << j+1 << '.' << std::endl;
      //  err = true;
      //}
      // if set indices:
      try {
          list_drv_i[j] = set_index_to_value(drv_indices[j], dsr_values[j]);
      } catch (std::out_of_range e) {
          Cerr << e.what() << " for variable '" 
            << iteratedModel.discrete_real_variable_labels()[j] << "' in method '" 
            << method_id() << "'\n";
          abort_handler(-1);
      }
    }
  }

#ifdef DEBUG
  Cout << "distribute_list_of_points():\n";
  for (i=0; i<numEvals; ++i) {
    if (numContinuousVars)
      Cout << "Eval " << i << " continuous:\n" << listCVPoints[i];
    if (numDiscreteIntVars)
      Cout << "Eval " << i << " discrete int:\n" << listDIVPoints[i];
    if (numDiscreteStringVars) {
      Cout << "Eval " << i << " discrete string:\n";
      write_data(Cout,
	listDSVPoints[boost::indices[i][idx_range(0, numDiscreteStringVars)]]);
    }
    if (numDiscreteRealVars)
      Cout << "Eval " << i << " discrete real:\n" << listDRVPoints[i];
  }
#endif // DEBUG

  return err;
}


void ParamStudy::distribute_partitions()
{
  contStepVector.sizeUninitialized(numContinuousVars);
  discIntStepVector.sizeUninitialized(numDiscreteIntVars);
  discStringStepVector.sizeUninitialized(numDiscreteStringVars);
  discRealStepVector.sizeUninitialized(numDiscreteRealVars);

  initialCVPoint.sizeUninitialized(numContinuousVars);
  initialDIVPoint.sizeUninitialized(numDiscreteIntVars);
  initialDSVPoint.resize(boost::extents[numDiscreteStringVars]);
  initialDRVPoint.sizeUninitialized(numDiscreteRealVars);

  const RealVector&          c_vars = iteratedModel.continuous_variables();
  const IntVector&          di_vars = iteratedModel.discrete_int_variables();
  StringMultiArrayConstView ds_vars = iteratedModel.discrete_string_variables();
  const RealVector&         dr_vars = iteratedModel.discrete_real_variables();

  const RealVector&  c_l_bnds = iteratedModel.continuous_lower_bounds();
  const RealVector&  c_u_bnds = iteratedModel.continuous_upper_bounds();
  const IntVector&  di_l_bnds = iteratedModel.discrete_int_lower_bounds();
  const IntVector&  di_u_bnds = iteratedModel.discrete_int_upper_bounds();
  const RealVector& dr_l_bnds = iteratedModel.discrete_real_lower_bounds();
  const RealVector& dr_u_bnds = iteratedModel.discrete_real_upper_bounds();

  const BitArray&      di_set_bits = iteratedModel.discrete_int_sets();
  const IntSetArray&    dsi_values = iteratedModel.discrete_set_int_values();
  const StringSetArray& dss_values = iteratedModel.discrete_set_string_values();
  const RealSetArray&   dsr_values = iteratedModel.discrete_set_real_values();

  size_t i, dsi_cntr; unsigned short part;
  for (i=0; i<numContinuousVars; ++i) {
    part = contVarPartitions[i];
    if (part) {
      initialCVPoint[i] = c_l_bnds[i];
      contStepVector[i] = (c_u_bnds[i] - c_l_bnds[i]) / part;
    }
    else
      { initialCVPoint[i] = c_vars[i]; contStepVector[i] = 0.; }
  }
  for (i=0, dsi_cntr=0; i<numDiscreteIntVars; ++i) {
    part = discIntVarPartitions[i];
    if (part) {
      initialDIVPoint[i] = di_l_bnds[i];
      int range = (di_set_bits[i]) ? dsi_values[dsi_cntr].size() - 1 :
	                             di_u_bnds[i] - di_l_bnds[i];
      discIntStepVector[i] = integer_step(range, part);
    }
    else
      { initialDIVPoint[i] = di_vars[i]; discIntStepVector[i] = 0; }
    // must increment the discrete set integer counter even if no partition:
    if (di_set_bits[i]) ++dsi_cntr;
  }
  for (i=0; i<numDiscreteStringVars; ++i) {
    part = discStringVarPartitions[i];
    if (part) {
      const StringSet& dss_vals_i = dss_values[i];
      initialDSVPoint[i]      = *dss_vals_i.begin();
      discStringStepVector[i] = integer_step(dss_vals_i.size() - 1, part);
    }
    else
      { initialDSVPoint[i] = ds_vars[i]; discStringStepVector[i] = 0; }
  }
  for (i=0; i<numDiscreteRealVars; ++i) {
    part = discRealVarPartitions[i];
    if (part) {
      initialDRVPoint[i]    = dr_l_bnds[i];
      discRealStepVector[i] = integer_step(dsr_values[i].size() - 1, part);
    }
    else
      { initialDRVPoint[i] = dr_vars[i]; discRealStepVector[i] = 0; }
  }

#ifdef DEBUG
  Cout << "distribute_partitions():\n";
  if (numContinuousVars)
    Cout << "c_vars:\n"   << c_vars   << "c_l_bnds:\n"       << c_l_bnds
	 << "c_u_bnds:\n" << c_u_bnds << "initialCVPoint:\n" << initialCVPoint
	 << "contStepVector:\n" << contStepVector;
  if (numDiscreteIntVars)
    Cout << "di_vars:\n" << di_vars << "di_l_bnds:\n" << di_l_bnds
	 << "di_u_bnds:\n" << di_u_bnds
	 << "initialDIVPoint:\n" << initialDIVPoint
	 << "discIntStepVector:\n" << discIntStepVector;
  if (numDiscreteStringVars) {
    Cout << "ds_vars:\n";              write_data(Cout, ds_vars);
    Cout << "initialDSVPoint:\n";      write_data(Cout, initialDSVPoint);
    Cout << "discStringStepVector:\n"; write_data(Cout, discStringStepVector);
  }
  if (numDiscreteRealVars)
    Cout << "dr_vars:\n" << dr_vars << "dr_l_bnds:\n" << dr_l_bnds
	 << "dr_u_bnds:\n" << dr_u_bnds
	 << "initialDRVPoint:\n" << initialDRVPoint
	 << "discRealStepVector:\n" << discRealStepVector;
#endif // DEBUG
}


void ParamStudy::final_point_to_step_vector()
{
  //RealVector cv_final, drv_final; IntVector div_final;
  //StringMultiArray dsv_final;
  //distribute(finalPoint, cv_final, div_final, dsv_final, drv_final);

  const BitArray&      di_set_bits = iteratedModel.discrete_int_sets();
  const IntSetArray&    dsi_values = iteratedModel.discrete_set_int_values();
  const StringSetArray& dss_values = iteratedModel.discrete_set_string_values();
  const RealSetArray&   dsr_values = iteratedModel.discrete_set_real_values();
  size_t j, dsi_cntr;

  // active continuous
  contStepVector.sizeUninitialized(numContinuousVars);
  for (j=0; j<numContinuousVars; ++j)
    contStepVector[j] = (finalCVPoint[j] - initialCVPoint[j]) / numSteps;

  // active discrete int: ranges and sets
  discIntStepVector.sizeUninitialized(numDiscreteIntVars);
  for (j=0, dsi_cntr=0; j<numDiscreteIntVars; ++j)
    if (di_set_bits[j]) {
      discIntStepVector[j] = index_step(
        set_value_to_index(initialDIVPoint[j], dsi_values[dsi_cntr]),
	// for final point defined as index:
        finalDIVPoint[j], numSteps);
	// for final point defined as admissible value:
        //set_value_to_index(div_final[j], dsi_values[dsi_cntr]), numSteps);
      ++dsi_cntr;
    }
    else
      discIntStepVector[j]
	= integer_step(finalDIVPoint[j] - initialDIVPoint[j], numSteps);

  // active discrete string: sets only
  discStringStepVector.sizeUninitialized(numDiscreteStringVars);
  for (j=0; j<numDiscreteStringVars; ++j)
    discStringStepVector[j] = index_step(
      set_value_to_index(initialDSVPoint[j], dss_values[j]),
      // for final point defined as index:
      finalDSVPoint[j], numSteps);
      // for final point defined as admissible value:
      //set_value_to_index(dsv_final[j], dss_values[j]), numSteps);

  // active discrete real: sets only
  discRealStepVector.sizeUninitialized(numDiscreteRealVars);
  for (j=0; j<numDiscreteRealVars; ++j)
    discRealStepVector[j] = index_step(
      set_value_to_index(initialDRVPoint[j], dsr_values[j]),
      // for final point defined as index:
      finalDRVPoint[j], numSteps);
      // for final point defined as admissible value:
      //set_value_to_index(drv_final[j], dsr_values[j]), numSteps);

#ifdef DEBUG
  Cout << "final_point_to_step_vector():\n";
  if (numContinuousVars)
    Cout << "continuous step vector:\n" << contStepVector;
  if (numDiscreteIntVars)
    Cout << "discrete int step vector:\n" << discIntStepVector;
  if (numDiscreteStringVars) {
    Cout << "discrete string step vector:\n";
    write_data(Cout, discStringStepVector);
  }
  if (numDiscreteRealVars)
    Cout << "discrete real step vector:\n" << discRealStepVector;
#endif // DEBUG
}


bool ParamStudy::
check_sets(const IntVector& c_steps,  const IntVector& di_steps,
	   const IntVector& ds_steps, const IntVector& dr_steps)
{
  // checks for vector and centered cases: admissibility of step vectors
  // and number of steps among int/real sets
  // > check terminal set indices for out of range
  // > don't enforce that range variables remain within bounds (for now)
  // Note: this check is performed at construct time and is dependent on the
  // initial points; therefore, it is not a definitive check in the case of
  // multi-iterator execution with updated initial points.  Nonetheless,
  // verify proper set support for specified steps.

  const BitArray&      di_set_bits = iteratedModel.discrete_int_sets();
  const IntSetArray&    dsi_values = iteratedModel.discrete_set_int_values();
  const StringSetArray& dss_values = iteratedModel.discrete_set_string_values();
  const RealSetArray&   dsr_values = iteratedModel.discrete_set_real_values();
  size_t j, dsi_cntr;
  bool err = false;

  // active discrete int: ranges and sets
  for (j=0, dsi_cntr=0; j<numDiscreteIntVars; ++j)
    if (di_set_bits[j]) {
      const IntSet& dsi_vals_j = dsi_values[dsi_cntr];
      int terminal_index = set_value_to_index(initialDIVPoint[j], dsi_vals_j)
	+ discIntStepVector[j] * di_steps[j];
      if (terminal_index < 0 || terminal_index >= dsi_vals_j.size()) {
	Cerr << "\nError: ParamStudy index " << terminal_index
	     << " not admissible for discrete int set of size "
	     << dsi_vals_j.size() << '.' << std::endl;
	err = true;
      }
      ++dsi_cntr;
    }

  // active discrete string: sets only
  for (j=0; j<numDiscreteStringVars; ++j) {
    const StringSet& dss_vals_j = dss_values[j];
    int terminal_index = set_value_to_index(initialDSVPoint[j], dss_vals_j)
      + discStringStepVector[j] * ds_steps[j];
    if (terminal_index < 0 || terminal_index >= dss_vals_j.size()) {
      Cerr << "\nError: ParamStudy index " << terminal_index
	   << " not admissible for discrete string set of size "
	   << dss_vals_j.size() << '.' << std::endl;
      err = true;
    }
  }

  // active discrete real: sets only
  for (j=0; j<numDiscreteRealVars; ++j) {
    const RealSet& dsr_vals_j = dsr_values[j];
    int terminal_index = set_value_to_index(initialDRVPoint[j], dsr_vals_j)
      + discRealStepVector[j] * dr_steps[j];
    if (terminal_index < 0 || terminal_index >= dsr_vals_j.size()) {
      Cerr << "\nError: ParamStudy index " << terminal_index
	   << " not admissible for discrete real set of size "
	   << dsr_vals_j.size() << '.' << std::endl;
      err = true;
    }
  }

  return err;
}


void ParamStudy::archive_allocate_cps() const
{
  StringMultiArrayConstView cv_labels
    = iteratedModel.continuous_variable_labels();
  StringMultiArrayConstView div_labels
    = iteratedModel.discrete_int_variable_labels();
  StringMultiArrayConstView dsv_labels
    = iteratedModel.discrete_string_variable_labels();
  StringMultiArrayConstView drv_labels
    = iteratedModel.discrete_real_variable_labels();
  const StringArray& resp_labels
    = iteratedModel.response_labels();

  DimScaleMap resp_scales;
  resp_scales.emplace(1, StringScale("responses", resp_labels));

  // For centered parameter study, allocate a group per variable
  for (size_t i=0; i<numContinuousVars; ++i) {
    resultsDB.allocate_vector
      ( run_identifier(),
	{String("variable_slices"), cv_labels[i], String("steps")},
	Dakota::ResultsOutputType::REAL,
	2*contStepsPerVariable[i] + 1 );
    resultsDB.allocate_matrix
      (run_identifier(),
       {String("variable_slices"), cv_labels[i], String("responses")},
       Dakota::ResultsOutputType::REAL,
       2*contStepsPerVariable[i] + 1, numFunctions, resp_scales );
  }
  for (size_t i=0; i<numDiscreteIntVars; ++i) {
    resultsDB.allocate_vector\
      ( run_identifier(),
	{String("variable_slices"), div_labels[i], String("steps")},
	Dakota::ResultsOutputType::INTEGER,
	2*discIntStepsPerVariable[i] + 1 );
    resultsDB.allocate_matrix
      ( run_identifier(),
	{"variable_slices", div_labels[i], String("responses")},
	Dakota::ResultsOutputType::REAL,
	2*discIntStepsPerVariable[i] + 1, numFunctions, resp_scales );
  }
  for (size_t i=0; i<numDiscreteStringVars; ++i) {
    resultsDB.allocate_vector
      ( run_identifier(),
	{String("variable_slices"), dsv_labels[i], String("steps")},
	Dakota::ResultsOutputType::STRING,
	2*discStringStepsPerVariable[i] + 1 );
    resultsDB.allocate_matrix
      ( run_identifier(),
	{String("variable_slices"), dsv_labels[i], String("responses")},
	Dakota::ResultsOutputType::REAL,
	2*discStringStepsPerVariable[i] + 1, numFunctions, resp_scales );
  }
  for (size_t i=0; i<numDiscreteRealVars; ++i) {
    resultsDB.allocate_vector
      ( run_identifier(),
	{String("variable_slices"), drv_labels[i], String("steps")},
	Dakota::ResultsOutputType::REAL,
	2*discRealStepsPerVariable[i] + 1 );
    resultsDB.allocate_matrix
      ( run_identifier(),
	{String("variable_slices"), drv_labels[i], String("responses")},
	Dakota::ResultsOutputType::REAL,
	2*discRealStepsPerVariable[i] + 1, numFunctions, resp_scales );
  }
}

void ParamStudy::archive_cps_vars(const Model& model, size_t idx) const
{
  const RealVector& c_vars  = model.continuous_variables();
  const IntVector & di_vars = model.discrete_int_variables();
  StringMultiArrayConstView ds_vars = model.discrete_string_variables();
  const RealVector& dr_vars = model.discrete_real_variables();

  StringMultiArrayConstView cv_labels
    = iteratedModel.continuous_variable_labels();
  StringMultiArrayConstView div_labels
    = iteratedModel.discrete_int_variable_labels();
  StringMultiArrayConstView dsv_labels
    = iteratedModel.discrete_string_variable_labels();
  StringMultiArrayConstView drv_labels
    = iteratedModel.discrete_real_variable_labels();
  const StringArray& resp_labels
    = iteratedModel.response_labels();

  // center point: store values in midpoint of each var's step results
  if (idx == 0) {
    for (size_t i=0; i<numContinuousVars; ++i)
      resultsDB.insert_into
	( run_identifier(),
	  {String("variable_slices"), cv_labels[i], String("steps")},
	  c_vars[i], contStepsPerVariable[i] );
    for (size_t i=0; i<numDiscreteIntVars; ++i)
      resultsDB.insert_into
	( run_identifier(),
	  {String("variable_slices"), div_labels[i], String("steps")},
	  di_vars[i], discIntStepsPerVariable[i] );
    for (size_t i=0; i<numDiscreteStringVars; ++i)
      resultsDB.insert_into
	( run_identifier(),
	  {String("variable_slices"), dsv_labels[i], String("steps")},
	  ds_vars[i], discStringStepsPerVariable[i] );
    for (size_t i=0; i<numDiscreteRealVars; ++i)
      resultsDB.insert_into
	( run_identifier(),
	  {String("variable_slices"), drv_labels[i], String("steps")},
	  dr_vars[i], discRealStepsPerVariable[i] );
    return;
  }

  // non-center points; get the variable and within-variable step indices
  size_t var_idx = 0, step_idx = 0;
  index_to_var_step(idx, var_idx, step_idx);

  if (var_idx < numContinuousVars) {
    size_t cv_idx = var_idx;
    resultsDB.insert_into
      ( run_identifier(),
	{String("variable_slices"), cv_labels[cv_idx], String("steps")},
	c_vars[cv_idx], step_idx );
  }
  else if (var_idx < numContinuousVars + numDiscreteIntVars) {
    size_t div_idx = var_idx - numContinuousVars;
    resultsDB.insert_into
      ( run_identifier(),
        {String("variable_slices"), div_labels[div_idx], String("steps")},
        di_vars[div_idx], step_idx );
  }
  else if (var_idx <
	   numContinuousVars + numDiscreteIntVars + numDiscreteStringVars) {
    size_t dsv_idx = var_idx - numContinuousVars - numDiscreteIntVars;
    resultsDB.insert_into
      ( run_identifier(),
	{String("variable_slices"), dsv_labels[dsv_idx], String("steps")},
	ds_vars[dsv_idx], step_idx );
  }
  else {
    size_t drv_idx =
      var_idx - numContinuousVars - numDiscreteIntVars - numDiscreteStringVars;
    resultsDB.insert_into
      ( run_identifier(),
        {String("variable_slices"), drv_labels[drv_idx], String("steps")},
        dr_vars[drv_idx], step_idx );
  }
}


void ParamStudy::archive_cps_resp(const Response& response, size_t idx) const
{
  StringMultiArrayConstView cv_labels
    = iteratedModel.continuous_variable_labels();
  StringMultiArrayConstView div_labels
    = iteratedModel.discrete_int_variable_labels();
  StringMultiArrayConstView dsv_labels
    = iteratedModel.discrete_string_variable_labels();
  StringMultiArrayConstView drv_labels
    = iteratedModel.discrete_real_variable_labels();
  const StringArray& resp_labels
    = iteratedModel.response_labels();

  const RealVector& resp_vec = response.function_values();

  // center point: store values in midpoint of each var's response results
  if (idx == 0) {
    for (size_t i=0; i<numContinuousVars; ++i)
      resultsDB.insert_into
	( run_identifier(),
	  {String("variable_slices"), cv_labels[i], String("responses")},
	  resp_vec, contStepsPerVariable[i] );
    for (size_t i=0; i<numDiscreteIntVars; ++i)
      resultsDB.insert_into
	( run_identifier(),
	  {String("variable_slices"), div_labels[i], String("responses")},
	  resp_vec, discIntStepsPerVariable[i] );
    for (size_t i=0; i<numDiscreteStringVars; ++i)
      resultsDB.insert_into
	( run_identifier(),
	  {String("variable_slices"), dsv_labels[i], String("responses")},
	  resp_vec, discStringStepsPerVariable[i] );
    for (size_t i=0; i<numDiscreteRealVars; ++i)
      resultsDB.insert_into
	( run_identifier(),
	  {String("variable_slices"), drv_labels[i], String("responses")},
	  resp_vec, discRealStepsPerVariable[i] );
    return;
  }

  // non-center points; get the variable and within-variable step indices
   size_t var_idx = 0, step_idx = 0;
   index_to_var_step(idx, var_idx, step_idx);

   if (var_idx < numContinuousVars) {
     size_t cv_idx = var_idx;
     resultsDB.insert_into
       ( run_identifier(),
	 {String("variable_slices"), cv_labels[cv_idx], String("responses")},
	 resp_vec, step_idx );
   }
   else if (var_idx < numContinuousVars + numDiscreteIntVars) {
     size_t div_idx = var_idx - numContinuousVars;
     resultsDB.insert_into
       ( run_identifier(),
	 {String("variable_slices"), div_labels[div_idx], String("responses")},
	 resp_vec, step_idx );
   }
   else if (var_idx <
	    numContinuousVars + numDiscreteIntVars + numDiscreteStringVars) {
     size_t dsv_idx = var_idx - numContinuousVars - numDiscreteIntVars;
     resultsDB.insert_into
       ( run_identifier(),
	 {String("variable_slices"), dsv_labels[dsv_idx], String("responses")},
	 resp_vec, step_idx );
   }
   else {
     size_t drv_idx =
       var_idx - numContinuousVars - numDiscreteIntVars - numDiscreteStringVars;
     resultsDB.insert_into
       ( run_identifier(),
	 {String("variable_slices"), drv_labels[drv_idx], String("responses")},
	 resp_vec, step_idx );
   }
}


void ParamStudy::index_to_var_step(const size_t study_idx,
				   size_t& var_idx, size_t& step_idx) const
{
  size_t num_vars = numContinuousVars + numDiscreteIntVars
    + numDiscreteStringVars + numDiscreteRealVars;

  // 0 <= study_idx <= sum_i(2*spv[i])

  // the zero-th eval is the center point; the steps for the first
  // variable start at offset index = 1
  size_t offset = 1;
  for (var_idx = 0; var_idx < num_vars; ++var_idx) {
    if (study_idx < offset + 2*stepsPerVariable[var_idx])
      break;
    offset += 2*stepsPerVariable[var_idx];
  }

  // determine the index into the var_idx-th variable steps,
  // accounting for the missing center point in the middle, where
  // step_idx is the middle index
  step_idx = ( study_idx - offset < stepsPerVariable[var_idx] ) ?
    (study_idx - offset) : (study_idx - offset + 1);
}


} // namespace Dakota
