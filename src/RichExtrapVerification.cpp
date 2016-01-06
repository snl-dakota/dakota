/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       RichExtrapVerification
//- Description: Implementation code for the RichExtrapVerification class
//- Owner:       Mike Eldred/Ben Pax

#include "dakota_system_defs.hpp"
#include "RichExtrapVerification.hpp"
#include "ProblemDescDB.hpp"

static const char rcsId[]="@(#) $Id: RichExtrapVerification.cpp 6972 2010-09-17 22:18:50Z briadam $";

//#define DEBUG


namespace Dakota {

RichExtrapVerification::
RichExtrapVerification(ProblemDescDB& problem_db, Model& model):
  Verification(problem_db, model),
  studyType(probDescDB.get_ushort("method.sub_method")),
  refinementRate(probDescDB.get_real("method.verification.refinement_rate"))
{
  // one iter with 3 pts, possibly followed by iters. with 1 addtnl pt
  //numEvals = 3;
  maxEvalConcurrency *= 3; //numEvals;
}


/*
void RichExtrapVerification::pre_run()
{
  Analyzer::pre_run();

  // Capture any changes resulting from the strategy layer's
  // passing of best variable info between iterators.
  if (studyType == ) {
    copy_data(iteratedModel.continuous_variables(),     initialCVPoint); // copy
    copy_data(iteratedModel.discrete_int_variables(),   initialDIVPoint);// copy
    copy_data(iteratedModel.discrete_string_variables(),initialDSVPoint);// copy
    copy_data(iteratedModel.discrete_real_variables(),  initialDRVPoint);// copy
  }

  size_t i, num_vars = numContinuousVars     + numDiscreteIntVars
                     + numDiscreteStringVars + numDiscreteRealVars;
  if (allSamples.numRows() != num_vars || allSamples.numCols() != numEvals)
    allSamples.shapeUninitialized(num_vars, numEvals);
  if ( outputLevel > SILENT_OUTPUT &&
       ( studyType == VECTOR_SV || studyType == VECTOR_FP ||
	 studyType == CENTERED ) )
    allHeaders.resize(numEvals);
}
*/


void RichExtrapVerification::core_run()
{
  /*
  // perform the evaluations; multidim exception
  bool log_resp_flag = (studyType == MULTIDIM) ? (!subIteratorFlag) : false;
  bool log_best_flag = (numObjFns || numLSqTerms); // opt or NLS data set
  evaluate_parameter_sets(iteratedModel, log_resp_flag, log_best_flag);
  */

  initialCVars = iteratedModel.continuous_variables();
  numFactors   = initialCVars.length();
  if (refinementRefPt.empty())
    refinementRefPt.sizeUninitialized(numFunctions);
  if (numErrorQOI.empty() || extrapQOI.empty() || convOrder.empty()) {
    numErrorQOI.shapeUninitialized(numFunctions, numFactors);
    extrapQOI.shapeUninitialized(numFunctions, numFactors);
    convOrder.shapeUninitialized(numFunctions, numFactors);
  }

  switch (studyType) {
  case SUBMETHOD_ESTIMATE_ORDER:
    if (outputLevel > SILENT_OUTPUT)
      Cout << "\nEstimating order of convergence.\n\n";
    estimate_order(); break;
  case SUBMETHOD_CONVERGE_ORDER:
    if (outputLevel > SILENT_OUTPUT)
      Cout << "\nConverging estimate of order of convergence.\n\n";
    converge_order(); break;
  case SUBMETHOD_CONVERGE_QOI:
    if (outputLevel > SILENT_OUTPUT)
      Cout << "\nConverging estimates of quantities of interest.\n\n";
    converge_qoi();   break;
  default:
    Cerr << "\nError: bad study type in RichExtrapVerification::core_run()."
	 << "\n       studyType = " << studyType << std::endl;
    abort_handler(-1);
  }
}


void RichExtrapVerification::
extrapolation(const RealVector& refine_triple, RealMatrix& qoi_triples)
{
  if (outputLevel == DEBUG_OUTPUT)
    { Cout << "refine_triple:\n"; write_data(Cout, refine_triple); }

  ShortArray asrv(numFunctions, 1); // all fns can be evaluated in this case
  activeSet.request_vector(asrv);
  iteratedModel.continuous_variables(initialCVars);// reset prior to eval triple

  for (size_t i=0; i<3; i++) {
    iteratedModel.continuous_variable(refine_triple[i], factorIndex);
    //iteratedModel.discrete_int_variables(di_vars);
    //iteratedModel.discrete_string_variables(ds_vars);
    //iteratedModel.discrete_real_variables(dr_vars);
    iteratedModel.evaluate_nowait(activeSet);
  }
  const IntResponseMap& response_map = iteratedModel.synchronize();
  IntRespMCIter   r_cit = response_map.begin();
  const Response& resp0 = r_cit->second;
  ++r_cit; const Response& resp1 = r_cit->second;
  ++r_cit; const Response& resp2 = r_cit->second;

  Real r21, r32, e21, e32, *conv_order = convOrder[factorIndex];
  if (qoi_triples.empty())
    qoi_triples.shapeUninitialized(numFunctions, 3);
  for (size_t i=0; i<numFunctions; ++i) {
    qoi_triples(i,0) = resp0.function_value(i);
    qoi_triples(i,1) = resp1.function_value(i);
    qoi_triples(i,2) = resp2.function_value(i);

    // Assumes uniform refinement rate
    r21 = refine_triple[1]/refine_triple[0]; // could replace w/ refinementRate
    r32 = refine_triple[2]/refine_triple[1]; // not used
    e21 = qoi_triples(i,1)-qoi_triples(i,0);
    e32 = qoi_triples(i,2)-qoi_triples(i,1);

    // convergence order for constant refinement rate
    if (outputLevel == DEBUG_OUTPUT)
      Cout << "r21 r32 e21 e32: " << r21 << '\t' << r32 << '\t' << e21 << '\t'
	   << e32 << '\n';
    conv_order[i] = (std::abs(e32) > DBL_MIN && std::abs(e21) > DBL_MIN) ?
      std::log(std::abs(e32/e21))/std::log(r21) : 0.;
  }
  if (outputLevel == DEBUG_OUTPUT) {
    Cout << "qoi_triples:\n"; write_data(Cout, qoi_triples, false, true, true);
    Cout << "Convergence order:\n"; write_data(Cout, conv_order, numFunctions);
  }
}


/** This algorithm executes a single refinement triple and returns
    convergence order estimates. */
void RichExtrapVerification::estimate_order()
{
  RealVector refine_triple(3, false);
  RealMatrix qoi_triples;
  size_t i;
  Real refine_level, *qoi_0, *err_qoi, *extrap_qoi;
  for (factorIndex=0; factorIndex<numFactors; ++factorIndex) {
    refine_level = initialCVars[factorIndex];
    for (i=0; i<3; ++i)
      { refine_triple[i] = refine_level; refine_level /= refinementRate; }

    extrapolation(refine_triple, qoi_triples);
    extrapolate_result(refine_triple, qoi_triples);

    qoi_0 = qoi_triples[0]; err_qoi = numErrorQOI[factorIndex];
    extrap_qoi = extrapQOI[factorIndex];
    for (i=0; i<numFunctions; ++i)
      err_qoi[i] = extrap_qoi[i] - qoi_0[i];
    refinementRefPt[factorIndex] = refine_triple[0];
  }
}


/** This algorithm continues to refine until the convergence order
    estimate converges. */
void RichExtrapVerification::converge_order()
{
  RealVector refine_triple(3, false), prev_order(numFunctions),
    delta_order(numFunctions, false);
  RealMatrix qoi_triples;
  size_t i, counter = 0;
  Real tolerance = 1., refine_level, *qoi_0, *err_qoi, *extrap_qoi;
  for (factorIndex=0; factorIndex<numFactors; ++factorIndex) {
    refine_level = initialCVars[factorIndex];
    while (tolerance > convergenceTol && ++counter <= maxIterations) {
      for (i=0; i<3; ++i)
	{ refine_triple[i] = refine_level; refine_level /= refinementRate; }
      extrapolation(refine_triple, qoi_triples);
    
      // convergence assessment
      copy_data(convOrder[factorIndex], (int)numFunctions, delta_order);
      delta_order -= prev_order;
      tolerance    = delta_order.normFrobenius();
      // scalar: tolerance = std::fabs(prev_order - convOrder)/convOrder;
      if (outputLevel == DEBUG_OUTPUT) {
	Cout << "Change in orders:\n"; write_data(Cout, delta_order);
	Cout << "converge_order() tolerance = " << tolerance << '\n';
      }

      // updates for next iteration
      copy_data(convOrder[factorIndex], (int)numFunctions, prev_order);
      // TO DO: eliminate duplication of first two evals on next cycle
      refine_level = refine_triple[1]; // start pt is middle of previous triple
    }
    extrapolate_result(refine_triple, qoi_triples);

    qoi_0 = qoi_triples[0]; err_qoi = numErrorQOI[factorIndex];
    extrap_qoi = extrapQOI[factorIndex];
    for (i=0; i<numFunctions; ++i)
      err_qoi[i] = extrap_qoi[i] - qoi_0[i];
    refinementRefPt[factorIndex] = refine_triple[0];
  }
}


/** This algorithm continues to refine until the discretization error
    lies within a prescribed tolerance. */
void RichExtrapVerification::converge_qoi()
{ 
  RealVector refine_triple(3, false), err_qoi_rv;
  RealMatrix qoi_triples;
  size_t i, counter = 0;
  Real tolerance = 1., refine_level, *qoi_0, *err_qoi, *extrap_qoi;
  for (factorIndex=0; factorIndex<numFactors; ++factorIndex) {
    refine_level = initialCVars[factorIndex];
    while (tolerance > convergenceTol && ++counter <= maxIterations) {
      for (i=0; i<3; ++i)
	{ refine_triple[i] = refine_level; refine_level /= refinementRate; }
      extrapolation(refine_triple, qoi_triples);
      extrapolate_result(refine_triple, qoi_triples);

      // convergence assessment based on extrapolated value
      qoi_0 = qoi_triples[0]; err_qoi = numErrorQOI[factorIndex];
      extrap_qoi = extrapQOI[factorIndex];
      for (i=0; i<numFunctions; ++i)
	err_qoi[i] = extrap_qoi[i] - qoi_0[i];
      // Alternate assessment based on two most refined QOI values
      //Real *qoi_1 = qoi_triples[1], *qoi_2 = qoi_triples[2];
      //for (i=0; i<numFunctions; ++i)
      //  err_qoi[i] = qoi_2[i] - qoi_1[i];
      RealVector  err_qoi_rv(Teuchos::View, err_qoi, numFunctions);
      tolerance = err_qoi_rv.normFrobenius();
      if (outputLevel == DEBUG_OUTPUT) {
	Cout << "Change in quantity of interest:\n";
	write_data(Cout, err_qoi, numFunctions);
	Cout << "converge_qoi() tolerance = " << tolerance << '\n';
      }
      // updates for next iteration
      // TO DO: eliminate duplication of first two evals on next cycle
      refine_level = refine_triple[1]; // start pt is middle of previous triple
    }
    refinementRefPt[factorIndex] = refine_triple[0];
  }
}


/* BMA will push this up to a higher level in the hierarchy
void RichExtrapVerification::post_input()
{
  // call convenience function from Analyzer
  read_variables_responses(numEvals);
}
*/


void RichExtrapVerification::
extrapolate_result(const RealVector& refine_triple,
		   const RealMatrix& qoi_triples)
{
  // Computes the extrapolated value of the QOI Phi
  Real coeff_extrap, *extrap_qoi = extrapQOI[factorIndex],
      *conv_order = convOrder[factorIndex];
  for (size_t i=0; i<numFunctions; i++) {
    coeff_extrap = (qoi_triples(i,1) - qoi_triples(i,0))
	         / (std::pow(refine_triple[0], conv_order[i]) -
		    std::pow(refine_triple[1], conv_order[i]));
    extrap_qoi[i] = qoi_triples(i,0)
                  + coeff_extrap*std::pow(refine_triple[0], conv_order[i]);
  }
  if (outputLevel == DEBUG_OUTPUT) {
    Cout << "Extrapolated QOI:\n";
    write_data(Cout, extrap_qoi, numFunctions);
  }
}


/*
void RichExtrapVerification::post_run(std::ostream& s)
{ Analyzer::post_run(s); }
*/


void RichExtrapVerification::print_results(std::ostream& s)
{
  StringArray cv_labels;
  copy_data(iteratedModel.continuous_variable_labels(), cv_labels);
  const StringArray& fn_labels = iteratedModel.response_labels();

  // Print resulting order and error estimates
  Cout << "\nRefinement Rate = " << refinementRate;
  Cout << "\nRefinement Reference Pt  =\n"; write_data(Cout, refinementRefPt);
  Cout << "\nFinal Convergence Rates  =\n";
  write_data(Cout,   convOrder, fn_labels, cv_labels);
  Cout << "\nExtrapolated QOI         =\n";
  write_data(Cout,   extrapQOI, fn_labels, cv_labels);
  Cout << "\nFinal QOI Error Estimate =\n";
  write_data(Cout, numErrorQOI, fn_labels, cv_labels);
  Cout << '\n';

  Verification::print_results(s);
}

} // namespace Dakota
