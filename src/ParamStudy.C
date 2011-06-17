/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ParamStudy
//- Description: Implementation code for the ParamStudy class
//- Owner:       Mike Eldred

#include "system_defs.h"
#include "ParamStudy.H"
#include "ProblemDescDB.H"
#include "ParallelLibrary.H"
#include "PolynomialApproximation.hpp"
#include <boost/lexical_cast.hpp>

static const char rcsId[]="@(#) $Id: ParamStudy.C 7024 2010-10-16 01:24:42Z mseldre $";

//#define DEBUG


namespace Dakota {

// special values for pStudyType
enum { LIST=1, VECTOR_SV, VECTOR_FP, CENTERED, MULTIDIM };


ParamStudy::ParamStudy(Model& model): PStudyDACE(model), pStudyType(0)
{
  // use allVariables instead of default allSamples
  compactMode = false;

  // Set pStudyType
  const RealVector& step_vector
    = probDescDB.get_rdv("method.parameter_study.step_vector");
  if (methodName.begins("list_"))
    pStudyType = LIST;
  else if (methodName.begins("vector_"))
    pStudyType = (step_vector.empty()) ? VECTOR_FP : VECTOR_SV;
  else if (methodName.begins("centered_"))
    pStudyType = CENTERED;
  else if (methodName.begins("multidim_"))
    pStudyType = MULTIDIM;

  // Extract specification from ProblemDescDB, perform sanity checking, and
  // compute/estimate maxConcurrency.
  bool err_flag = false;
  switch (pStudyType) {
  case LIST: // list_parameter_study
    if (distribute_list_of_points(
	probDescDB.get_rdv("method.parameter_study.list_of_points")))
      err_flag = true;
    break;
  case VECTOR_FP: // vector_parameter_study (final_point & num_steps spec.)
    if (check_final_point(
	probDescDB.get_rdv("method.parameter_study.final_point")))
      err_flag = true;
    if (check_num_steps(probDescDB.get_int("method.parameter_study.num_steps")))
      err_flag = true;
    // precompute steps (using construct-time initialPoint) and perform error
    // checks only if in check mode; else avoid additional overhead and rely
    // on run-time checks for run-time initialPoint.
    if (numSteps && iteratedModel.parallel_library().command_line_check()) {
      initialCVPoint  = iteratedModel.continuous_variables();    // view
      initialDIVPoint = iteratedModel.discrete_int_variables();  // view
      initialDRVPoint = iteratedModel.discrete_real_variables(); // view
      final_point_to_step_vector(); // covers check_ranges_sets(numSteps)
    }
    break;
  case VECTOR_SV: // vector_parameter_study (step_vector & num_steps spec.)
    if (distribute_step_vector(step_vector))
      err_flag = true;
    if (check_num_steps(probDescDB.get_int("method.parameter_study.num_steps")))
      err_flag = true;
    initialDIVPoint = iteratedModel.discrete_int_variables();  // view
    initialDRVPoint = iteratedModel.discrete_real_variables(); // view
    if (check_ranges_sets(numSteps))
      err_flag = true;
    break;
  case CENTERED: { // centered_parameter_study
    if (distribute_step_vector(step_vector))
      err_flag = true;
    if (check_steps_per_variable(
	probDescDB.get_idv("method.parameter_study.steps_per_variable")))
      err_flag = true;
    initialDIVPoint = iteratedModel.discrete_int_variables();  // view
    initialDRVPoint = iteratedModel.discrete_real_variables(); // view
    if (check_ranges_sets(stepsPerVariable))
      err_flag = true;
    break;
  }
  case MULTIDIM: // multidim_parameter_study
    if (check_variable_partitions(probDescDB.get_dusa("method.partitions")))
      err_flag = true;
    if (check_finite_bounds())
      err_flag = true;
    // precompute steps (using construct-time bounds) and perform error checks
    // only if in check mode; else avoid additional overhead and rely on
    // run-time checks for run-time bounds.
    if (iteratedModel.parallel_library().command_line_check())
      distribute_partitions();
    break;
  default:
    Cerr << "\nError: bad pStudyType (" << pStudyType
	 << ") in ParamStudy constructor." << std::endl;
    err_flag = true;
  }
  if (err_flag)
    abort_handler(-1);

  maxConcurrency *= numEvals;
}


void ParamStudy::pre_run()
{
  Iterator::pre_run();  // for completeness

  // Capture any changes in initialCVPoint resulting from the strategy layer's
  // passing of best variable info between iterators.  If no such variable 
  // passing has occurred, then this reassignment is merely repetitive of the 
  // one in the ParamStudy constructor.  If there is a final_point 
  // specification, then contStepVector and numSteps must be (re)computed.
  if (pStudyType == VECTOR_SV || pStudyType == VECTOR_FP ||
      pStudyType == CENTERED) {
    copy_data(iteratedModel.continuous_variables(),    initialCVPoint);  // copy
    copy_data(iteratedModel.discrete_int_variables(),  initialDIVPoint); // copy
    copy_data(iteratedModel.discrete_real_variables(), initialDRVPoint); // copy
  }

  size_t av_size = allVariables.size();
  if (av_size != numEvals) {
    allVariables.resize(numEvals);
    const SharedVariablesData& svd
      = iteratedModel.current_variables().shared_data();
    for (size_t i=av_size; i<numEvals; ++i)
      allVariables[i] = Variables(svd); // use minimal data ctor
    if ( outputLevel > SILENT_OUTPUT &&
	 ( pStudyType == VECTOR_SV || pStudyType == VECTOR_FP ||
	   pStudyType == CENTERED ) )
      allHeaders.resize(numEvals);
  }

  switch (pStudyType) {
  case LIST: // list_parameter_study
    if (outputLevel > SILENT_OUTPUT)
      Cout << "\nList parameter study for " << numEvals << " samples\n\n";
    sample();
    break;
  case VECTOR_FP: // vector_parameter_study (final_point & num_steps)
    if (outputLevel > SILENT_OUTPUT) {
      Cout << "\nVector parameter study from\n";
      write_ordered(Cout, initialCVPoint, initialDIVPoint, initialDRVPoint);
      Cout << "to\n";
      write_data(Cout, finalPoint);
      Cout << "using " << numSteps << " steps\n\n";
    }
    if (numSteps) // define step vectors from initial, final, & num steps
      final_point_to_step_vector();
    vector_loop();
    break;
  case VECTOR_SV: // vector_parameter_study (step_vector & num_steps)
    if (outputLevel > SILENT_OUTPUT) {
      Cout << "\nVector parameter study for " << numSteps
	   << " steps starting from\n";
      write_ordered(Cout, initialCVPoint, initialDIVPoint, initialDRVPoint);
      Cout << "with a step vector of\n";
      write_ordered(Cout, contStepVector, discStepVector);
      Cout << '\n';
    }
    vector_loop();
    break;
  case CENTERED: // centered_parameter_study
    if (outputLevel > SILENT_OUTPUT) {
      Cout << "\nCentered parameter study with steps per variable\n";
      write_data(Cout, stepsPerVariable);
      Cout << "and increments of\n";
      write_ordered(Cout, contStepVector, discStepVector);
      Cout << "with the following center point:\n";
      write_ordered(Cout, initialCVPoint, initialDIVPoint, initialDRVPoint);
      Cout << '\n';
    }
    centered_loop();
    break;
  case MULTIDIM: // multidim_parameter_study
    if (outputLevel > SILENT_OUTPUT)
      Cout << "\nMultidimensional parameter study for variable partitions of\n"
           << variablePartitions << '\n';
    distribute_partitions();
    multidim_loop();
    break;
  default:
    Cerr << "\nError: bad pStudyType in ParamStudy::extract_trends().\n       "
         << "pStudyType = " << pStudyType << std::endl;
    abort_handler(-1);
  }
}


void ParamStudy::extract_trends()
{
  // perform the evaluations; multidim exception
  bool log_resp_flag = (pStudyType == MULTIDIM) ? (!subIteratorFlag) : false;
  bool log_best_flag = (numObjFns || numLSqTerms); // opt or NLS data set
  evaluate_parameter_sets(iteratedModel, log_resp_flag, log_best_flag);
}


void ParamStudy::post_input()
{
  // call convenience function from Analyzer
  read_variables_responses(numEvals, numContinuousVars + numDiscreteIntVars + 
    numDiscreteRealVars);
}


void ParamStudy::post_run(std::ostream& s)
{
  bool log_resp_flag = (!subIteratorFlag);
  if (pStudyType == MULTIDIM && log_resp_flag)
    pStudyDACESensGlobal.compute_correlations(allVariables, allResponses);

  Iterator::post_run(s);
}


void ParamStudy::sample()
{
  // populate allVariables
  for (size_t i=0; i<numEvals; ++i) {
    if (numContinuousVars)
      allVariables[i].continuous_variables(listCVPoints[i]);
    if (numDiscreteIntVars)
      allVariables[i].discrete_int_variables(listDIVPoints[i]);
    if (numDiscreteRealVars)
      allVariables[i].discrete_real_variables(listDRVPoints[i]);
  }
  // free up redundant memory
  listCVPoints.clear();
  listDIVPoints.clear();
  listDRVPoints.clear();
}


void ParamStudy::vector_loop()
{
  // Steps along a n-dimensional vector through numSteps additions of
  // continuous/discrete step vectors.  The step is an absolute step defining
  // magnitude & direction.  The number of fn. evaluations in the study is
  // numSteps + 1 since the initial point is also evaluated.

  // Loop over num_steps
  RealVector c_vars, dr_vars; IntVector di_vars;
  const IntSetArray&  ddsi_values
    = iteratedModel.discrete_design_set_int_values();
  const RealSetArray& ddsr_values
    = iteratedModel.discrete_design_set_real_values();
  const IntSetArray& dssi_values
    = iteratedModel.discrete_state_set_int_values();
  const RealSetArray& dssr_values
    = iteratedModel.discrete_state_set_real_values();
  const Pecos::RealVectorArray& h_pt_prs
    = iteratedModel.distribution_parameters().histogram_point_pairs();
  const SharedVariablesData& svd
    = iteratedModel.current_variables().shared_data();
  const SizetArray& vc_totals = svd.components_totals();
  size_t i, j, num_cdv = vc_totals[0],
    num_ddriv = svd.vc_lookup(DISCRETE_DESIGN_RANGE),
    num_ddsiv = svd.vc_lookup(DISCRETE_DESIGN_SET_INT),
    num_ddsrv = vc_totals[2], num_cauv = vc_totals[3], num_dauiv = vc_totals[4],
    num_daurv = vc_totals[5], num_ceuv = vc_totals[6],
    //num_deuiv = vc_totals[7], num_deurv = vc_totals[8],
    num_csv   = vc_totals[9], num_dsriv = svd.vc_lookup(DISCRETE_STATE_RANGE),
    num_dssiv = svd.vc_lookup(DISCRETE_STATE_SET_INT),
    num_dssrv = vc_totals[11];

  RealSetArray x_sets(num_daurv);
  for (i=0; i<num_daurv; ++i)
    x_y_pairs_to_x_set(h_pt_prs[i], x_sets[i]);

  for (i=0; i<=numSteps; ++i) {
    Variables& vars = allVariables[i];
    size_t c_cntr = 0, d_cntr = 0, di_cntr = 0, dr_cntr = 0;
    for (j=0; j<num_cdv; ++j, ++c_cntr)                                   // cdv
      c_step(c_cntr, i, vars);
    for (j=0; j<num_ddriv; ++j, ++d_cntr, ++di_cntr)                    // ddriv
      dri_step(d_cntr, di_cntr, i, vars);
    for (j=0; j<num_ddsiv; ++j, ++d_cntr, ++di_cntr)                    // ddsiv
      dsi_step(d_cntr, di_cntr, i, ddsi_values[j], vars);
    for (j=0; j<num_ddsrv; ++j, ++d_cntr, ++dr_cntr)                    // ddsrv
      dsr_step(d_cntr, dr_cntr, i, ddsr_values[j], vars);
    for (j=0; j<num_cauv; ++j, ++c_cntr)                                 // cauv
      c_step(c_cntr, i, vars);
    for (j=0; j<num_dauiv; ++j, ++d_cntr, ++di_cntr)                    // dauiv
      dri_step(d_cntr, di_cntr, i, vars);
    for (j=0; j<num_daurv; ++j, ++d_cntr, ++dr_cntr)                    // daurv
      dsr_step(d_cntr, dr_cntr, i, x_sets[j], vars);
    for (j=0; j<num_ceuv + num_csv; ++j, ++c_cntr)                   // ceuv,csv
      c_step(c_cntr, i, vars);
    for (j=0; j<num_dsriv; ++j, ++d_cntr, ++di_cntr)                    // dsriv
      dri_step(d_cntr, di_cntr, i, vars);
    for (j=0; j<num_dssiv; ++j, ++d_cntr, ++di_cntr)                    // dssiv
      dsi_step(d_cntr, di_cntr, i, dssi_values[j], vars);
    for (j=0; j<num_dssrv; ++j, ++d_cntr, ++dr_cntr)                    // dssrv
      dsr_step(d_cntr, dr_cntr, i, dssr_values[j], vars);

    // store each output header in allHeaders
    if (outputLevel > SILENT_OUTPUT) {
      std::string h_string;
      if (asynchFlag)
	h_string += "\n\n";
      if (numSteps == 0) // Allow numSteps == 0 case
	h_string += ">>>>> Initial_point only (no steps)\n";
      h_string += ">>>>> Vector parameter study evaluation for ";
      h_string += boost::lexical_cast<std::string>(i*100./numSteps);
      h_string += "% along vector\n";
      allHeaders[i] = h_string;
    }
  }
}


void ParamStudy::centered_loop()
{
  const IntSetArray&  ddsi_values
    = iteratedModel.discrete_design_set_int_values();
  const RealSetArray& ddsr_values
    = iteratedModel.discrete_design_set_real_values();
  const IntSetArray& dssi_values
    = iteratedModel.discrete_state_set_int_values();
  const RealSetArray& dssr_values
    = iteratedModel.discrete_state_set_real_values();
  const Pecos::RealVectorArray& h_pt_prs
    = iteratedModel.distribution_parameters().histogram_point_pairs();
  const SharedVariablesData& svd
    = iteratedModel.current_variables().shared_data();
  const SizetArray& vc_totals = svd.components_totals();
  size_t i, j, k, cntr = 0,
    num_vars = numContinuousVars + numDiscreteIntVars + numDiscreteRealVars,
    num_cdv   = vc_totals[0],
    num_ddriv = svd.vc_lookup(DISCRETE_DESIGN_RANGE),
    num_ddsiv = svd.vc_lookup(DISCRETE_DESIGN_SET_INT),
    num_ddsrv = vc_totals[2],
    num_ddv   = num_ddriv + num_ddsiv + num_ddsrv, num_dv = num_cdv + num_ddv,
    num_cauv  = vc_totals[3],  num_dauiv = vc_totals[4],
    num_daurv = vc_totals[5],  num_dauv = num_dauiv + num_daurv,
    num_auv = num_cauv + num_dauv, num_ceuv = vc_totals[6],
    //num_deuiv = vc_totals[7], num_deurv = vc_totals[8],
    num_uv = num_auv + num_ceuv,   num_csv = vc_totals[9],
    num_dsriv = svd.vc_lookup(DISCRETE_STATE_RANGE),
    num_dssiv = svd.vc_lookup(DISCRETE_STATE_SET_INT),
    num_dssrv = vc_totals[11];

  // Always evaluate center point, even if steps_per_variable = 0
  if (outputLevel > SILENT_OUTPUT) {
    if (asynchFlag)
      allHeaders[cntr] =
        "\n\n>>>>> Centered parameter study evaluation for center point\n";
    else
      allHeaders[cntr] =
        ">>>>> Centered parameter study evaluation for center point\n";
  }
  if (numContinuousVars)
    allVariables[cntr].continuous_variables(initialCVPoint);
  if (numDiscreteIntVars)
    allVariables[cntr].discrete_int_variables(initialDIVPoint);
  if (numDiscreteRealVars)
    allVariables[cntr].discrete_real_variables(initialDRVPoint);
  ++cntr;

  RealSetArray x_sets(num_daurv);
  for (i=0; i<num_daurv; ++i)
    x_y_pairs_to_x_set(h_pt_prs[i], x_sets[i]);

  // Evaluate +/- steps for each variable
  for (k=0; k<num_vars; ++k) {

    // Loop over steps_per_variable[k]
    int i, num_steps_k = stepsPerVariable[k];
    for (i=-num_steps_k; i<=num_steps_k; ++i) {
      if (i) { // skip the center

        // store each output header in allHeaders
        if (outputLevel > SILENT_OUTPUT) {
          std::string h_string;
          if (asynchFlag)
            h_string += "\n\n";
          if (i<0) {
            h_string += ">>>>> Centered parameter study evaluation for x[";
            h_string += boost::lexical_cast<std::string>(k+1) + "] - ";
            h_string += boost::lexical_cast<std::string>(-i) + "delta:\n";
          }
          else {
            h_string += ">>>>> Centered parameter study evaluation for x[";
            h_string += boost::lexical_cast<std::string>(k+1) + "] + ";
            h_string += boost::lexical_cast<std::string>(i) + "delta:\n";
          }
        }

	Variables& vars = allVariables[cntr];
	if (numContinuousVars)
	  vars.continuous_variables(initialCVPoint);
	if (numDiscreteIntVars)
	  vars.discrete_int_variables(initialDIVPoint);
	if (numDiscreteRealVars)
	  vars.discrete_real_variables(initialDRVPoint);

	// compute modified parameter by adding multiple of step_k to start
	if (k < num_cdv)                                                // cdv
	  c_step(k, i, vars);
	else if (k < num_cdv + num_ddriv) {                             // ddriv
	  size_t offset = k - num_cdv;
	  dri_step(offset, offset, i, vars);
	}
	else if (k < num_cdv + num_ddriv + num_ddsiv) {                 // ddsiv
	  size_t offset = k - num_cdv;
	  dsi_step(offset, offset, i, ddsi_values[offset - num_ddriv], vars);
	}
	else if (k < num_dv) {                                          // ddsrv
	  size_t offset_d  = k - num_cdv,
	         offset_dr = offset_d - num_ddriv - num_ddsiv;
	  dsr_step(offset_d, offset_dr, i, ddsr_values[offset_dr], vars);
	}
	else if (k < num_dv + num_cauv)                                 // cauv
	  c_step(k - num_ddv, i, vars);
	else if (k < num_dv + num_cauv + num_dauiv) {                   // dauiv
	  size_t offset_d  = k - num_cdv - num_cauv,
	         offset_di = offset_d - num_ddsrv;
	  dri_step(offset_d, offset_di, i, vars);
	}
	else if (k < num_dv + num_auv) {                                // daurv
	  size_t offset_d   = k - num_cdv - num_cauv,
	         offset_dr  = offset_d - num_ddriv - num_ddsiv - num_dauiv,
	         offset_dsr = offset_dr - num_ddsrv;
	  dsr_step(offset_d, offset_dr, i, x_sets[offset_dsr], vars);
	}
	else if (k < num_dv + num_uv + num_csv)                      // ceuv,csv
	  c_step(k - num_ddv - num_dauv, i, vars);
	else if (k < num_dv + num_uv + num_csv + num_dsriv) {           // dsriv
	  size_t offset_d  = k - num_cdv - num_cauv - num_ceuv - num_csv,
	         offset_di = offset_d - num_ddsrv - num_daurv;
	  dri_step(offset_d, offset_di, i, vars);
	}
	else if (k < num_dv + num_uv + num_csv + num_dsriv + num_dssiv) {//dssiv
	  size_t offset_d   = k - num_cdv - num_cauv - num_ceuv - num_csv,
	         offset_di  = offset_d - num_ddsrv - num_daurv,
	         offset_dsi = offset_di - num_ddriv - num_ddsiv - num_dauiv
	                    - num_dsriv;
	  dsi_step(offset_d, offset_di, i, dssi_values[offset_dsi], vars);
	}
	else {                                                          // dssrv
	  size_t offset_d   = k - num_cdv - num_cauv - num_ceuv - num_csv,
	         offset_dr  = offset_d - num_ddriv - num_ddsiv - num_dauiv
	                    - num_dsriv - num_dssiv,
	         offset_dsr = offset_dr - num_ddsrv - num_daurv;
	  dsr_step(offset_d, offset_dr, i, ddsr_values[offset_dsr], vars);
	}
	++cntr;
      }
    }
  }
}


void ParamStudy::multidim_loop()
{
  // Perform a multidimensional parameter study based on the number of 
  // partitions specified for each variable.

  const IntSetArray& ddsi_values
    = iteratedModel.discrete_design_set_int_values();
  const RealSetArray& ddsr_values
    = iteratedModel.discrete_design_set_real_values();
  const IntSetArray& dssi_values
    = iteratedModel.discrete_state_set_int_values();
  const RealSetArray& dssr_values
    = iteratedModel.discrete_state_set_real_values();
  const Pecos::RealVectorArray& h_pt_prs
    = iteratedModel.distribution_parameters().histogram_point_pairs();
  const SharedVariablesData& svd
    = iteratedModel.current_variables().shared_data();
  const SizetArray& vc_totals = svd.components_totals();
  size_t i, j,
    num_vars = numContinuousVars + numDiscreteIntVars + numDiscreteRealVars,
    num_cdv   = vc_totals[0], num_ddriv = svd.vc_lookup(DISCRETE_DESIGN_RANGE),
    num_ddsiv = svd.vc_lookup(DISCRETE_DESIGN_SET_INT),
    num_ddsrv = vc_totals[2], num_cauv = vc_totals[3], num_dauiv = vc_totals[4],
    num_daurv = vc_totals[5], num_ceuv = vc_totals[6],
    //num_deuiv = vc_totals[7], num_deurv = vc_totals[8],
    num_csv   = vc_totals[9], num_dsriv = svd.vc_lookup(DISCRETE_STATE_RANGE),
    num_dssiv = svd.vc_lookup(DISCRETE_STATE_SET_INT),
    num_dssrv = vc_totals[11];

  UShortArray multidim_indices(num_vars, 0);
  RealSetArray x_sets(num_daurv);
  for (i=0; i<num_daurv; ++i)
    x_y_pairs_to_x_set(h_pt_prs[i], x_sets[i]);
  for (i=0; i<numEvals; ++i) {
    Variables& vars = allVariables[i];
    size_t p_cntr = 0, c_cntr = 0, d_cntr = 0, di_cntr = 0, dr_cntr = 0;
    for (j=0; j<num_cdv; ++j, ++p_cntr, ++c_cntr)                         // cdv
      c_step(c_cntr, multidim_indices[p_cntr], vars);
    for (j=0; j<num_ddriv; ++j, ++p_cntr, ++d_cntr, ++di_cntr)          // ddriv
      dri_step(d_cntr, di_cntr, multidim_indices[p_cntr], vars);
    for (j=0; j<num_ddsiv; ++j, ++p_cntr, ++d_cntr, ++di_cntr)          // ddsiv
      dsi_step(d_cntr, di_cntr, multidim_indices[p_cntr], ddsi_values[j], vars);
    for (j=0; j<num_ddsrv; ++j, ++p_cntr, ++d_cntr, ++dr_cntr)          // ddsrv
      dsr_step(d_cntr, dr_cntr, multidim_indices[p_cntr], ddsr_values[j], vars);
    for (j=0; j<num_cauv; ++j, ++p_cntr, ++c_cntr)                       // cauv
      c_step(c_cntr, multidim_indices[p_cntr], vars);
    for (j=0; j<num_dauiv; ++j, ++p_cntr, ++d_cntr, ++di_cntr)          // dauiv
      dri_step(d_cntr, di_cntr, multidim_indices[p_cntr], vars);
    for (j=0; j<num_daurv; ++j, ++p_cntr, ++d_cntr, ++dr_cntr)          // daurv
      dsr_step(d_cntr, dr_cntr, multidim_indices[p_cntr], x_sets[j], vars);
    for (j=0; j<num_ceuv + num_csv; ++j, ++p_cntr, ++c_cntr)         // ceuv,csv
      c_step(c_cntr, multidim_indices[p_cntr], vars);
    for (j=0; j<num_dsriv; ++j, ++p_cntr, ++d_cntr, ++di_cntr)          // dsriv
      dri_step(d_cntr, di_cntr, multidim_indices[p_cntr], vars);
    for (j=0; j<num_dssiv; ++j, ++p_cntr, ++d_cntr, ++di_cntr)          // dssiv
      dsi_step(d_cntr, di_cntr, multidim_indices[p_cntr], dssi_values[j], vars);
    for (j=0; j<num_dssrv; ++j, ++p_cntr, ++d_cntr, ++dr_cntr)          // dssrv
      dsr_step(d_cntr, dr_cntr, multidim_indices[p_cntr], dssr_values[j], vars);

    // increment the multidimensional index set
    Pecos::PolynomialApproximation::increment_indices(multidim_indices,
						      variablePartitions,false);
  }
}


bool ParamStudy::distribute_list_of_points(const RealVector& list_of_pts)
{
  size_t len_lop = list_of_pts.length(),
    num_vars = numContinuousVars + numDiscreteIntVars + numDiscreteRealVars;
  if (len_lop % num_vars) {
    Cerr << "\nError: length of list_of_points ("  << len_lop
	 << ") must be evenly divisable among number of active variables ("
	 << num_vars << ")." << std::endl;
    return true;
  }
  numEvals = len_lop / num_vars;
  if (numContinuousVars)   listCVPoints.resize(numEvals);
  if (numDiscreteIntVars)  listDIVPoints.resize(numEvals);
  if (numDiscreteRealVars) listDRVPoints.resize(numEvals);

  const IntSetArray& ddsi_values
    = iteratedModel.discrete_design_set_int_values();
  const RealSetArray& ddsr_values
    = iteratedModel.discrete_design_set_real_values();
  const IntSetArray& dssi_values
    = iteratedModel.discrete_state_set_int_values();
  const RealSetArray& dssr_values
    = iteratedModel.discrete_state_set_real_values();
  const Pecos::RealVectorArray& h_pt_prs
    = iteratedModel.distribution_parameters().histogram_point_pairs();
  const SharedVariablesData& svd
    = iteratedModel.current_variables().shared_data();
  const SizetArray& vc_totals = svd.components_totals();
  size_t i, j, num_cdv = vc_totals[0],
    num_ddriv = svd.vc_lookup(DISCRETE_DESIGN_RANGE),
    num_ddsiv = svd.vc_lookup(DISCRETE_DESIGN_SET_INT),
    num_ddsrv = vc_totals[2], num_cauv = vc_totals[3], num_dauiv = vc_totals[4],
    num_daurv = vc_totals[5], num_ceuv  = vc_totals[6],
    //num_deuiv = vc_totals[7], num_deurv = vc_totals[8],
    num_csv   = vc_totals[9], num_dsriv =svd.vc_lookup(DISCRETE_STATE_RANGE),
    num_dssiv = svd.vc_lookup(DISCRETE_STATE_SET_INT),
    num_dssrv = vc_totals[11], num_dsrv = num_dssrv, l_cntr = 0;

  RealSetArray x_sets(num_daurv);
  for (i=0; i<num_daurv; ++i)
    x_y_pairs_to_x_set(h_pt_prs[i], x_sets[i]);

  for (i=0; i<numEvals; ++i) {
    // Extract in order: cdv/ddiv/ddrv, cauv/dauiv/daurv, ceuv/deuiv/deurv,
    //                   csv/dsiv/dsrv
    size_t c_cntr = 0, di_cntr = 0, dr_cntr = 0;
    if (numContinuousVars)
      listCVPoints[i].sizeUninitialized(numContinuousVars);
    if (numDiscreteIntVars)
      listDIVPoints[i].sizeUninitialized(numDiscreteIntVars);
    if (numDiscreteRealVars)
      listDRVPoints[i].sizeUninitialized(numDiscreteRealVars);
    for (j=0; j<num_cdv; ++j, ++l_cntr, ++c_cntr)
      listCVPoints[i][c_cntr] = list_of_pts[l_cntr];
    for (j=0; j<num_ddriv; ++j, ++l_cntr, ++di_cntr)
      listDIVPoints[i][di_cntr] = truncate(list_of_pts[l_cntr]);
    for (j=0; j<num_ddsiv; ++j, ++l_cntr, ++di_cntr) {
      int list_val = truncate(list_of_pts[l_cntr]);
      if (ddsi_values[j].find(list_val) != ddsi_values[j].end())
	listDIVPoints[i][di_cntr] = list_val;
      else {
	Cerr << "Error: discrete design integer set does not contain value "
	     << list_val << '.'  << std::endl;
	abort_handler(-1);
      }
    }
    for (j=0; j<num_ddsrv; ++j, ++l_cntr, ++dr_cntr) {
      const Real& list_val = list_of_pts[l_cntr];
      if (ddsr_values[j].find(list_val) != ddsr_values[j].end())
	listDRVPoints[i][dr_cntr] = list_val;
      else {
	Cerr << "Error: discrete design real set does not contain value "
	     << list_val << '.' << std::endl;
	abort_handler(-1);
      }
    }
    for (j=0; j<num_cauv; ++j, ++l_cntr, ++c_cntr)
      listCVPoints[i][c_cntr] = list_of_pts[l_cntr];
    for (j=0; j<num_dauiv; ++j, ++l_cntr, ++di_cntr)
      listDIVPoints[i][di_cntr] = truncate(list_of_pts[l_cntr]);
    for (j=0; j<num_daurv; ++j, ++l_cntr, ++dr_cntr) {
      const Real& list_val = list_of_pts[l_cntr];
      if (x_sets[j].find(list_val) != x_sets[j].end())
	listDRVPoints[i][dr_cntr] = list_val;
      else {
	Cerr << "Error: discrete aleatory real set does not contain value "
	     << list_val << '.' << std::endl;
	abort_handler(-1);
      }
    }
    for (j=0; j<num_ceuv; ++j, ++l_cntr, ++c_cntr)
      listCVPoints[i][c_cntr] = list_of_pts[l_cntr];
    //for (j=0; j<num_deuiv; ++j, ++l_cntr, ++di_cntr)
    //  listDIVPoints[i][di_cntr] = truncate(list_of_pts[l_cntr]);
    //for (j=0; j<num_deurv; ++j, ++l_cntr, ++dr_cntr)
    //  listDRVPoints[i][dr_cntr] = list_of_pts[l_cntr];
    for (j=0; j<num_csv; ++j, ++l_cntr, ++c_cntr)
      listCVPoints[i][c_cntr] = list_of_pts[l_cntr];
    for (j=0; j<num_dsriv; ++j, ++l_cntr, ++di_cntr)
      listDIVPoints[i][di_cntr] = truncate(list_of_pts[l_cntr]);
    for (j=0; j<num_dssiv; ++j, ++l_cntr, ++di_cntr) {
      int list_val = truncate(list_of_pts[l_cntr]);
      if (dssi_values[j].find(list_val) != dssi_values[j].end())
	listDIVPoints[i][di_cntr] = list_val;
      else {
	Cerr << "Error: discrete state integer set does not contain value "
	     << list_val << '.' << std::endl;
	abort_handler(-1);
      }
    }
    for (j=0; j<num_dssrv; ++j, ++l_cntr, ++dr_cntr) {
      const Real& list_val = list_of_pts[l_cntr];
      if (dssr_values[j].find(list_val) != dssr_values[j].end())
	listDRVPoints[i][dr_cntr] = list_val;
      else {
	Cerr << "Error: discrete state real set does not contain value "
	     << list_val << '.' << std::endl;
	abort_handler(-1);
      }
    }
  }

#ifdef DEBUG
  Cout << "distribute_list_of_points():\n";
  for (i=0; i<numEvals; ++i) {
    if (numContinuousVars) {
      Cout << "Eval " << i << " continuous:\n";
      write_data(Cout, listCVPoints[i]);
    }
    if (numDiscreteIntVars) {
      Cout << "Eval " << i << " discrete int:\n";
      write_data(Cout, listDIVPoints[i]);
    }
    if (numDiscreteRealVars) {
      Cout << "Eval " << i << " discrete real:\n";
      write_data(Cout, listDRVPoints[i]);
    }
  }
#endif // DEBUG

  return false;
}


bool ParamStudy::distribute_step_vector(const RealVector& step_vector)
{
  size_t num_dv   = numDiscreteIntVars + numDiscreteRealVars,
         num_vars = numContinuousVars  + num_dv;
  if (step_vector.length() != num_vars) {
    Cerr << "\nError: Parameter Study step_vector length must be " << num_vars
	 << '.' << std::endl;
    return true;
  }
  contStepVector.sizeUninitialized(numContinuousVars);
  discStepVector.sizeUninitialized(num_dv);

  // Extract in order: cdv/ddiv/ddrv, cauv/dauiv/daurv, ceuv/deuiv/deurv,
  //                   csv/dsiv/dsrv
  const SizetArray& vc_totals
    = iteratedModel.current_variables().variables_components_totals();
  size_t i, num_cdv = vc_totals[0],
    num_ddv  = vc_totals[1] + vc_totals[2], num_cauv = vc_totals[3],
    num_dauv = vc_totals[4] + vc_totals[5], num_ceuv = vc_totals[6],
    //num_deuv = vc_totals[7] + vc_totals[8],
    num_csv = vc_totals[9], num_dsv = vc_totals[10] + vc_totals[11],
    s_cntr = 0, c_cntr = 0, d_cntr = 0;
  for (i=0; i<num_cdv; ++i, ++s_cntr, ++c_cntr)
    contStepVector[c_cntr] = step_vector[s_cntr];
  for (i=0; i<num_ddv; ++i, ++s_cntr, ++d_cntr)
    discStepVector[d_cntr] = truncate(step_vector[s_cntr]);
  for (i=0; i<num_cauv; ++i, ++s_cntr, ++c_cntr)
    contStepVector[c_cntr] = step_vector[s_cntr];
  for (i=0; i<num_dauv; ++i, ++s_cntr, ++d_cntr)
    discStepVector[d_cntr] = truncate(step_vector[s_cntr]);
  for (i=0; i<num_ceuv; ++i, ++s_cntr, ++c_cntr)
    contStepVector[c_cntr] = step_vector[s_cntr];
  //for (i=0; i<num_deuv; ++i, ++s_cntr, ++d_cntr)
  //  discStepVector[d_cntr] = truncate(step_vector[s_cntr]);
  for (i=0; i<num_csv; ++i, ++s_cntr, ++c_cntr)
    contStepVector[c_cntr] = step_vector[s_cntr];
  for (i=0; i<num_dsv; ++i, ++s_cntr, ++d_cntr)
    discStepVector[d_cntr] = truncate(step_vector[s_cntr]);

#ifdef DEBUG
  Cout << "distribute_step_vector():\n";
  if (numContinuousVars) {
    Cout << "continuous step vector:\n";
    write_data(Cout, contStepVector);
  }
  if (numDiscreteIntVars || numDiscreteRealVars) {
    Cout << "discrete step vector:\n";
    write_data(Cout, discStepVector);
  }
#endif // DEBUG

  return false;
}


void ParamStudy::distribute_partitions()
{
  size_t num_dv   = numDiscreteIntVars + numDiscreteRealVars,
         num_vars = numContinuousVars  + num_dv;
  contStepVector.sizeUninitialized(numContinuousVars);
  discStepVector.sizeUninitialized(num_dv);
  initialCVPoint.sizeUninitialized(numContinuousVars);
  initialDIVPoint.sizeUninitialized(numDiscreteIntVars);
  initialDRVPoint.sizeUninitialized(numDiscreteRealVars);

  const RealVector&    c_vars = iteratedModel.continuous_variables();
  const IntVector&    di_vars = iteratedModel.discrete_int_variables();
  const RealVector&   dr_vars = iteratedModel.discrete_real_variables();
  const RealVector&  c_l_bnds = iteratedModel.continuous_lower_bounds();
  const RealVector&  c_u_bnds = iteratedModel.continuous_upper_bounds();
  const IntVector&  di_l_bnds = iteratedModel.discrete_int_lower_bounds();
  const IntVector&  di_u_bnds = iteratedModel.discrete_int_upper_bounds();
  const RealVector& dr_l_bnds = iteratedModel.discrete_real_lower_bounds();
  const RealVector& dr_u_bnds = iteratedModel.discrete_real_upper_bounds();
  const IntSetArray& ddsi_values
    = iteratedModel.discrete_design_set_int_values();
  const RealSetArray& ddsr_values
    = iteratedModel.discrete_design_set_real_values();
  const IntSetArray& dssi_values
    = iteratedModel.discrete_state_set_int_values();
  const RealSetArray& dssr_values
    = iteratedModel.discrete_state_set_real_values();
  const Pecos::RealVectorArray& h_pt_prs
    = iteratedModel.distribution_parameters().histogram_point_pairs();
  const SharedVariablesData& svd
    = iteratedModel.current_variables().shared_data();
  const SizetArray& vc_totals = svd.components_totals();
  size_t i, num_cdv = vc_totals[0],
    num_ddriv = svd.vc_lookup(DISCRETE_DESIGN_RANGE),
    num_ddsiv = svd.vc_lookup(DISCRETE_DESIGN_SET_INT),
    num_ddsrv = vc_totals[2], num_cauv = vc_totals[3], num_dauiv = vc_totals[4],
    num_daurv = vc_totals[5], num_ceuv  = vc_totals[6],
    //num_deuiv = vc_totals[7], num_deurv = vc_totals[8],
    num_csv   = vc_totals[9], num_dsriv = svd.vc_lookup(DISCRETE_STATE_RANGE),
    num_dssiv = svd.vc_lookup(DISCRETE_STATE_SET_INT),
    num_dssrv = vc_totals[11], p_cntr = 0, c_cntr = 0, d_cntr = 0,
    di_cntr = 0, dr_cntr = 0;

  for (i=0; i<num_cdv; ++i, ++p_cntr, ++c_cntr) {
    unsigned short part = variablePartitions[p_cntr];
    if (part) {
      initialCVPoint[c_cntr] = c_l_bnds[c_cntr];
      contStepVector[c_cntr] = (c_u_bnds[c_cntr] - c_l_bnds[c_cntr]) / part;
    }
    else {
      initialCVPoint[c_cntr] = c_vars[c_cntr];
      contStepVector[c_cntr] = 0.;
    }
  }
  for (i=0; i<num_ddriv; ++i, ++p_cntr, ++d_cntr, ++di_cntr) {
    unsigned short part = variablePartitions[p_cntr];
    if (part) {
      initialDIVPoint[di_cntr] = di_l_bnds[di_cntr];
      discStepVector[d_cntr]
	= integer_step(di_u_bnds[di_cntr]-di_l_bnds[di_cntr], part);
    }
    else {
      initialDIVPoint[di_cntr] = di_vars[di_cntr];
      discStepVector[d_cntr]   = 0;
    }
  }
  for (i=0; i<num_ddsiv; ++i, ++p_cntr, ++d_cntr, ++di_cntr) {
    unsigned short part = variablePartitions[p_cntr];
    if (part) {
      initialDIVPoint[di_cntr] = di_l_bnds[di_cntr];
      discStepVector[d_cntr]   = integer_step(ddsi_values[i].size()-1, part);
    }
    else {
      initialDIVPoint[di_cntr] = di_vars[di_cntr];
      discStepVector[d_cntr]   = 0;
    }
  }
  for (i=0; i<num_ddsrv; ++i, ++p_cntr, ++d_cntr, ++dr_cntr) {
    unsigned short part = variablePartitions[p_cntr];
    if (part) {
      initialDRVPoint[dr_cntr] = dr_l_bnds[dr_cntr];
      discStepVector[d_cntr]   = integer_step(ddsr_values[i].size()-1, part);
    }
    else {
      initialDRVPoint[dr_cntr] = dr_vars[dr_cntr];
      discStepVector[d_cntr]   = 0;
    }
  }
  for (i=0; i<num_cauv; ++i, ++p_cntr, ++c_cntr) {
    unsigned short part = variablePartitions[p_cntr];
    if (part) {
      initialCVPoint[c_cntr] = c_l_bnds[c_cntr];
      contStepVector[c_cntr] = (c_u_bnds[c_cntr] - c_l_bnds[c_cntr]) / part;
    }
    else {
      initialCVPoint[c_cntr] = c_vars[c_cntr];
      contStepVector[c_cntr] = 0.;
    }
  }
  for (i=0; i<num_dauiv; ++i, ++p_cntr, ++d_cntr, ++di_cntr) {
    unsigned short part = variablePartitions[p_cntr];
    if (part) {
      initialDIVPoint[di_cntr] = di_l_bnds[di_cntr];
      discStepVector[d_cntr]
	= integer_step(di_u_bnds[di_cntr]-di_l_bnds[di_cntr], part);
    }
    else {
      initialDIVPoint[di_cntr] = di_vars[di_cntr];
      discStepVector[d_cntr]   = 0;
    }
  }
  for (i=0; i<num_daurv; ++i, ++p_cntr, ++d_cntr, ++dr_cntr) {
    unsigned short part = variablePartitions[p_cntr];
    if (part) {
      initialDRVPoint[dr_cntr] = dr_l_bnds[dr_cntr];
      discStepVector[d_cntr]   = integer_step(h_pt_prs[i].length()/2-1, part);
    }
    else {
      initialDRVPoint[dr_cntr] = dr_vars[dr_cntr];
      discStepVector[d_cntr]   = 0;
    }
  }
  for (i=0; i<num_ceuv; ++i, ++p_cntr, ++c_cntr) {
    unsigned short part = variablePartitions[p_cntr];
    if (part) {
      initialCVPoint[c_cntr] = c_l_bnds[c_cntr];
      contStepVector[c_cntr] = (c_u_bnds[c_cntr] - c_l_bnds[c_cntr]) / part;
    }
    else {
      initialCVPoint[c_cntr] = c_vars[c_cntr];
      contStepVector[c_cntr] = 0.;
    }
  }
  //for (i=0; i<num_deuiv; ++i, ++p_cntr, ++d_cntr, ++di_cntr)
  //  discStepVector[d_cntr] = integer_step(, variablePartitions[p_cntr]);
  //for (i=0; i<num_deurv; ++i, ++p_cntr, ++d_cntr, ++dr_cntr)
  //  discStepVector[d_cntr] = integer_step(, variablePartitions[p_cntr]);
  for (i=0; i<num_csv; ++i, ++p_cntr, ++c_cntr) {
    unsigned short part = variablePartitions[p_cntr];
    if (part) {
      initialCVPoint[c_cntr] = c_l_bnds[c_cntr];
      contStepVector[c_cntr] = (c_u_bnds[c_cntr] - c_l_bnds[c_cntr]) / part;
    }
    else {
      initialCVPoint[c_cntr] = c_vars[c_cntr];
      contStepVector[c_cntr] = 0.;
    }
  }
  for (i=0; i<num_dsriv; ++i, ++p_cntr, ++d_cntr, ++di_cntr) {
    unsigned short part = variablePartitions[p_cntr];
    if (part) {
      initialDIVPoint[di_cntr] = di_l_bnds[di_cntr];
      discStepVector[d_cntr]
	= integer_step(di_u_bnds[di_cntr]-di_l_bnds[di_cntr], part);
    }
    else {
      initialDIVPoint[di_cntr] = di_vars[di_cntr];
      discStepVector[d_cntr]   = 0;
    }
  }
  for (i=0; i<num_dssiv; ++i, ++p_cntr, ++d_cntr, ++di_cntr) {
    unsigned short part = variablePartitions[p_cntr];
    if (part) {
      initialDIVPoint[di_cntr] = di_l_bnds[di_cntr];
      discStepVector[d_cntr]   = integer_step(dssi_values[i].size()-1, part);
    }
    else {
      initialDIVPoint[di_cntr] = di_vars[di_cntr];
      discStepVector[d_cntr]   = 0;
    }
  }
  for (i=0; i<num_dssrv; ++i, ++p_cntr, ++d_cntr, ++dr_cntr) {
    unsigned short part = variablePartitions[p_cntr];
    if (part) {
      initialDRVPoint[dr_cntr] = dr_l_bnds[dr_cntr];
      discStepVector[d_cntr]   = integer_step(dssr_values[i].size()-1, part);
    }
    else {
      initialDRVPoint[dr_cntr] = dr_vars[dr_cntr];
      discStepVector[d_cntr]   = 0;
    }
  }

#ifdef DEBUG
  Cout << "distribute_partitions():\n";
  if (numContinuousVars) {
    Cout << "c_vars:\n";
    write_data(Cout, c_vars);
    Cout << "c_l_bnds:\n";
    write_data(Cout, c_l_bnds);
    Cout << "c_u_bnds:\n";
    write_data(Cout, c_u_bnds);
    Cout << "initialCVPoint:\n";
    write_data(Cout, initialCVPoint);
    Cout << "contStepVector:\n";
    write_data(Cout, contStepVector);
  }
  if (numDiscreteIntVars) {
    Cout << "di_vars:\n";
    write_data(Cout, di_vars);
    Cout << "di_l_bnds:\n";
    write_data(Cout, di_l_bnds);
    Cout << "di_u_bnds:\n";
    write_data(Cout, di_u_bnds);
    Cout << "initialDIVPoint:\n";
    write_data(Cout, initialDIVPoint);
    Cout << "discStepVector discrete int:\n";
    write_data_partial(Cout, 0, numDiscreteIntVars, discStepVector);
  }
  if (numDiscreteRealVars) {
    Cout << "dr_vars:\n";
    write_data(Cout, dr_vars);
    Cout << "dr_l_bnds:\n";
    write_data(Cout, dr_l_bnds);
    Cout << "dr_u_bnds:\n";
    write_data(Cout, dr_u_bnds);
    Cout << "initialDRVPoint:\n";
    write_data(Cout, initialDRVPoint);
    Cout << "discStepVector discrete real:\n";
    write_data_partial(Cout, numDiscreteIntVars, numDiscreteRealVars,
		       discStepVector);
  }
#endif // DEBUG
}


void ParamStudy::final_point_to_step_vector()
{
  const IntSetArray& ddsi_values
    = iteratedModel.discrete_design_set_int_values();
  const RealSetArray& ddsr_values
    = iteratedModel.discrete_design_set_real_values();
  const IntSetArray& dssi_values
    = iteratedModel.discrete_state_set_int_values();
  const RealSetArray& dssr_values
    = iteratedModel.discrete_state_set_real_values();
  const Pecos::RealVectorArray& h_pt_prs
    = iteratedModel.distribution_parameters().histogram_point_pairs();
  contStepVector.sizeUninitialized(numContinuousVars);
  discStepVector.sizeUninitialized(numDiscreteIntVars+numDiscreteRealVars);

  // Convert in order: cdv/ddriv/ddsiv/ddsrv, cauv/dauiv/daurv,
  //                   ceuv/deuiv/deurv, csv/dsriv/dssiv/dssrv
  const SharedVariablesData& svd
    = iteratedModel.current_variables().shared_data();
  const SizetArray& vc_totals = svd.components_totals();
  size_t i, num_cdv = vc_totals[0],
    num_ddriv = svd.vc_lookup(DISCRETE_DESIGN_RANGE),
    num_ddsiv = svd.vc_lookup(DISCRETE_DESIGN_SET_INT),
    num_ddsrv = vc_totals[2], num_cauv = vc_totals[3], num_dauiv = vc_totals[4],
    num_daurv = vc_totals[5], num_ceuv = vc_totals[6],
    //num_deuiv = vc_totals[7], num_deurv = vc_totals[8],
    num_csv   = vc_totals[9],  num_dsriv = svd.vc_lookup(DISCRETE_STATE_RANGE),
    num_dssiv = svd.vc_lookup(DISCRETE_STATE_SET_INT),
    num_dssrv  = vc_totals[11], f_cntr = 0, c_cntr = 0, d_cntr = 0,
    di_cntr = 0, dr_cntr = 0;
  bool not_found_err = false, modulo_err = false;

  // design
  for (i=0; i<num_cdv; ++i, ++c_cntr, ++f_cntr)
    contStepVector[c_cntr]
      = (finalPoint[f_cntr] - initialCVPoint[c_cntr]) / numSteps;
  for (i=0; i<num_ddriv; ++i, ++d_cntr, ++di_cntr, ++f_cntr)
    discStepVector[d_cntr] = integer_step(
      truncate(finalPoint[f_cntr])-initialDIVPoint[di_cntr], numSteps);
  for (i=0; i<num_ddsiv; ++i, ++d_cntr, ++di_cntr, ++f_cntr)
    discStepVector[d_cntr] = index_step(
      set_value_to_index(initialDIVPoint[di_cntr],     ddsi_values[i]),
      set_value_to_index(truncate(finalPoint[f_cntr]), ddsi_values[i]),
      numSteps);
  for (i=0; i<num_ddsrv; ++i, ++d_cntr, ++dr_cntr, ++f_cntr)
    discStepVector[d_cntr] = index_step(
      set_value_to_index(initialDRVPoint[dr_cntr], ddsr_values[i]),
      set_value_to_index(finalPoint[f_cntr],       ddsr_values[i]), numSteps);

  // aleatory uncertain
  for (i=0; i<num_cauv; ++i, ++c_cntr, ++f_cntr)
    contStepVector[c_cntr]
      = (finalPoint[f_cntr] - initialCVPoint[c_cntr]) / numSteps;
  for (i=0; i<num_dauiv; ++i, ++d_cntr, ++di_cntr, ++f_cntr)
    discStepVector[d_cntr] = integer_step(
      truncate(finalPoint[f_cntr])-initialDIVPoint[di_cntr], numSteps);
  for (i=0; i<num_daurv; ++i, ++d_cntr, ++dr_cntr, ++f_cntr) {
    RealSet x_set; x_y_pairs_to_x_set(h_pt_prs[i], x_set);
    discStepVector[d_cntr] = index_step(
      set_value_to_index(initialDRVPoint[dr_cntr], x_set),
      set_value_to_index(finalPoint[f_cntr],       x_set), numSteps);
  }

  // epistemic uncertain
  for (i=0; i<num_ceuv; ++i, ++c_cntr, ++f_cntr)
    contStepVector[c_cntr]
      = (finalPoint[f_cntr] - initialCVPoint[c_cntr]) / numSteps;
  // discrete int  epistemic uncertain
  // discrete real epistemic uncertain

  // state
  for (i=0; i<num_csv; ++i, ++c_cntr, ++f_cntr)
    contStepVector[c_cntr]
      = (finalPoint[f_cntr] - initialCVPoint[c_cntr]) / numSteps;
  for (i=0; i<num_dsriv; ++i, ++d_cntr, ++di_cntr, ++f_cntr)
    discStepVector[d_cntr] = integer_step(
      truncate(finalPoint[f_cntr])-initialDIVPoint[di_cntr], numSteps);
  for (i=0; i<num_dssiv; ++i, ++d_cntr, ++di_cntr, ++f_cntr)
    discStepVector[d_cntr] = index_step(
      set_value_to_index(initialDIVPoint[di_cntr],     dssi_values[i]),
      set_value_to_index(truncate(finalPoint[f_cntr]), dssi_values[i]),
      numSteps);
  for (i=0; i<num_dssrv; ++i, ++d_cntr, ++dr_cntr, ++f_cntr)
    discStepVector[d_cntr] = index_step(
      set_value_to_index(initialDRVPoint[dr_cntr], dssr_values[i]),
      set_value_to_index(finalPoint[f_cntr],       dssr_values[i]), numSteps);

#ifdef DEBUG
  Cout << "final_point_to_step_vector():\n";
  if (numContinuousVars) {
    Cout << "continuous step vector:\n";
    write_data(Cout, contStepVector);
  }
  if (numDiscreteIntVars || numDiscreteRealVars) {
    Cout << "discrete step vector:\n";
    write_data(Cout, discStepVector);
  }
#endif // DEBUG
}


bool ParamStudy::check_sets(const IntVector& steps)
{
  // checks for admissibility of step vectors and # of steps among int/real sets

  // checks for vector and centered cases:
  // > don't enforce that range variables remain within bounds (for now)
  // > check terminal set indices for out of range
  // Note: this check is performed at construct time and is dependent on the
  // initial points; therefore, it is not a definitive check in the case of
  // multi-iterator execution with updated initial points.  Nonetheless, verify
  // proper set support for specified steps.
  bool err = false;
  const SharedVariablesData& svd
    = iteratedModel.current_variables().shared_data();
  const SizetArray& vc_totals = svd.components_totals();
  size_t i, num_cdv = vc_totals[0],
    num_ddriv = svd.vc_lookup(DISCRETE_DESIGN_RANGE),
    num_ddsiv = svd.vc_lookup(DISCRETE_DESIGN_SET_INT),
    num_ddsrv = vc_totals[2], num_cauv = vc_totals[3], num_dauiv = vc_totals[4],
    num_daurv = vc_totals[5], num_ceuv = vc_totals[6],
    num_csv   = vc_totals[9], num_dsriv = svd.vc_lookup(DISCRETE_STATE_RANGE),
    num_dssiv = svd.vc_lookup(DISCRETE_STATE_SET_INT),
    num_dssrv = vc_totals[11], s_index = num_cdv + num_ddriv,
    d_index = num_ddriv, di_index = num_ddriv, dr_index = 0;
  if (num_ddsiv) {
    const IntSetArray& ddsi_values
      = iteratedModel.discrete_design_set_int_values();
    for (i=0; i<num_ddsiv; ++i, ++s_index, ++d_index, ++di_index) {
      int terminal_index = set_value_to_index(initialDIVPoint[di_index], 
	ddsi_values[i]) + discStepVector[d_index] * steps[s_index];
      if (terminal_index < 0 || terminal_index >= ddsi_values[i].size()) {
	Cerr << "\nError: index " << terminal_index << " not admissible for "
	     << "set of size " << ddsi_values[i].size() << '.' << std::endl;
	err = true;
      }
    }
  }
  if (num_ddsrv) {
    const RealSetArray& ddsr_values
      = iteratedModel.discrete_design_set_real_values();
    for (i=0; i<num_ddsrv; ++i, ++s_index, ++d_index, ++dr_index) {
      int terminal_index = set_value_to_index(initialDRVPoint[dr_index],
	ddsr_values[i]) + discStepVector[d_index] * steps[s_index];
      if (terminal_index < 0 || terminal_index >= ddsr_values[i].size()) {
	Cerr << "\nError: index " << terminal_index << " not admissible for "
	     << "set of size " << ddsr_values[i].size() << '.' << std::endl;
	err = true;
      }
    }
  }
  s_index  += num_cauv + num_dauiv;
  d_index  += num_dauiv;
  di_index += num_dauiv;
  for (i=0; i<num_daurv; ++i)
  if (num_daurv) {
    const Pecos::RealVectorArray& h_pt_prs
      = iteratedModel.distribution_parameters().histogram_point_pairs();
    RealSet x_set;
    for (i=0; i<num_daurv; ++i, ++s_index, ++d_index, ++dr_index) {
      x_y_pairs_to_x_set(h_pt_prs[i], x_set);
      int terminal_index = set_value_to_index(initialDRVPoint[dr_index], x_set)
	+ discStepVector[d_index] * steps[s_index];
      if (terminal_index < 0 || terminal_index >= h_pt_prs[i].length()/2) {
	Cerr << "\nError: index " << terminal_index << " not admissible for "
	     << h_pt_prs[i].length()/2 << " abscissas." << std::endl;
	err = true;
      }
    }
  }
  s_index  += num_ceuv + num_csv + num_dsriv;
  d_index  += num_dsriv;
  di_index += num_dsriv;
  if (num_dssiv) {
    const IntSetArray& dssi_values
      = iteratedModel.discrete_state_set_int_values();
    for (i=0; i<num_dssiv; ++i, ++s_index, ++d_index, ++di_index) {
      int terminal_index = set_value_to_index(initialDIVPoint[di_index],
	dssi_values[i]) + discStepVector[d_index] * steps[s_index];
      if (terminal_index < 0 || terminal_index >= dssi_values[i].size()) {
	Cerr << "\nError: index " << terminal_index << " not admissible for "
	     << "set of size " << dssi_values[i].size() << '.' << std::endl;
	err = true;
      }
    }
  }
  if (num_dssrv) {
    const RealSetArray& dssr_values
      = iteratedModel.discrete_state_set_real_values();
    for (i=0; i<num_dssrv; ++i, ++s_index, ++d_index, ++dr_index) {
      int terminal_index = set_value_to_index(initialDRVPoint[dr_index],
	dssr_values[i]) + discStepVector[d_index] * steps[s_index];
      if (terminal_index < 0 || terminal_index >= dssr_values[i].size()) {
	Cerr << "\nError: index " << terminal_index << " not admissible for "
	     << "set of size " << dssr_values[i].size() << '.' << std::endl;
	err = true;
      }
    }
  }

  return err;
}

} // namespace Dakota
