/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       DiscrepancyCorrection
//- Description: Implementation code for the DiscrepancyCorrection class
//- Owner:       Mike Eldred
//- Checked by:

#include "system_defs.h"
#include "data_io.h"
#include "DiscrepancyCorrection.H"
#include "ParamResponsePair.H"
#include "PRPMultiIndex.H"
#include "SurrogateData.hpp"

static const char rcsId[]="@(#) $Id: DiscrepancyCorrection.C 7024 2010-10-16 01:24:42Z mseldre $";

//#define DEBUG


namespace Dakota {

void DiscrepancyCorrection::
initialize(Model& surr_model, const IntSet& surr_fn_indices,
	   const String& corr_type, short corr_order)
{
  surrModel = surr_model; // shallow copy
  surrogateFnIndices = surr_fn_indices;
  numFns = surr_model.num_functions(); numVars = surr_model.cv();
  correctionType = corr_type; correctionOrder = corr_order;

  initialize_corrections();
}


void DiscrepancyCorrection::
initialize(const IntSet& surr_fn_indices, size_t num_fns, size_t num_vars,
	   const String& corr_type, short corr_order)
{
  surrogateFnIndices = surr_fn_indices;
  numFns = num_fns; numVars = num_vars;
  correctionType = corr_type; correctionOrder = corr_order;

  initialize_corrections();

  // in this case, surrModel is null and must be protected
}


void DiscrepancyCorrection::initialize_corrections()
{
  // initialize correction data
  correctionComputed = badScalingFlag = combinedFlag = computeAdditive
    = computeMultiplicative = false;
  if (correctionType == "additive")
    computeAdditive = true;
  else if (correctionType == "multiplicative")
    computeMultiplicative = true;
  else if (correctionType == "combined") {
    combinedFlag = computeAdditive = computeMultiplicative = true;
    combineFactors.resize(numFns);
    combineFactors = 1.; // used on 1st cycle prior to existence of prev pt.
  }
  UShortArray approx_order(numVars, correctionOrder);
  switch (correctionOrder) {
  case 2: dataOrder = 7; break;
  case 1: dataOrder = 3; break;
  case 0: default: dataOrder = 1; break;
  }
  ISIter it;
  if (computeAdditive) {
    addCorrections.resize(numFns);
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it)
      addCorrections[*it] = Approximation("local_taylor", approx_order, 
					   numVars, dataOrder);
  }
  if (computeMultiplicative) {
    multCorrections.resize(numFns);
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it)
      multCorrections[*it] = Approximation("local_taylor", approx_order, 
					   numVars, dataOrder);
  }
}


/** Compute an additive or multiplicative correction that corrects the
    approx_response to have 0th-order consistency (matches values),
    1st-order consistency (matches values and gradients), or 2nd-order
    consistency (matches values, gradients, and Hessians) with the
    truth_response at a single point (e.g., the center of a trust
    region).  The 0th-order, 1st-order, and 2nd-order corrections use
    scalar values, linear scaling functions, and quadratic scaling
    functions, respectively, for each response function. */
void DiscrepancyCorrection::
compute(const RealVector& c_vars, const Response& truth_response, 
	const Response& approx_response, bool quiet_flag)
{
  // The incoming approx_response is assumed to be uncorrected (i.e.,
  // correction has not been applied to it previously).  In this case,
  // it is not necessary to back out a previous correction, and the
  // computation of the new correction is straightforward.

  const RealVector&    truth_fns =  truth_response.function_values();
  const RealVector&   approx_fns = approx_response.function_values();
  const RealMatrix&  truth_grads =  truth_response.function_gradients();
  const RealMatrix& approx_grads = approx_response.function_gradients();
  const RealSymMatrixArray& truth_hessians
    = truth_response.function_hessians();
  const RealSymMatrixArray& approx_hessians
    = approx_response.function_hessians();

  // update previous center data arrays for combined corrections
  // TO DO: augment approxFnsPrevCenter logic for data fit surrogates.  May
  // require additional fn evaluation of previous pt on current surrogate.
  // This could combine with DB lookups within apply_multiplicative()
  // (approx re-evaluated if not found in DB search).
  int index; size_t j, k; ISIter it;
  if (combinedFlag && correctionComputed) {
    approxFnsPrevCenter = approxFnsCenter;
    truthFnsPrevCenter  = truthFnsCenter;
    it = surrogateFnIndices.begin();
    correctionPrevCenterPt = (computeAdditive || badScalingFlag) ?
      addCorrections[*it].approximation_data().anchor_continuous_variables() :
      multCorrections[*it].approximation_data().anchor_continuous_variables();
  }
  // update current center data arrays
  bool fall_back
    = (computeMultiplicative && correctionOrder >= 1 && surrModel.is_null());
  if (combinedFlag)                 truthFnsCenter = truth_fns;
  if (combinedFlag || fall_back)   approxFnsCenter = approx_fns;
  if (fall_back)                 approxGradsCenter = approx_grads;

  // detect numerical issues with multiplicative scaling
  badScalingFlag = (computeMultiplicative) ?
    check_scaling(truth_fns, approx_fns) : false;
  if (badScalingFlag && addCorrections.empty()) {
    addCorrections.resize(numFns);
    UShortArray approx_order(numVars, correctionOrder);
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it)
      addCorrections[*it] = Approximation("local_taylor", approx_order, 
					  numVars, dataOrder);
  }

  Pecos::SurrogateDataVars sdv(c_vars, Pecos::DEEP_COPY);
  if (computeAdditive || badScalingFlag) {
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      Pecos::SurrogateDataResp sdr(dataOrder, numVars);
      // -----------------------------
      // Additive 0th order correction
      // -----------------------------
      if (dataOrder & 1)
	sdr.response_function(truth_fns[index] - approx_fns[index]);
      // -----------------------------
      // Additive 1st order correction
      // -----------------------------
      if (dataOrder & 2) {
	const Real*  truth_grad =  truth_grads[index];
	const Real* approx_grad = approx_grads[index];
	for (j=0; j<numVars; ++j)
	  sdr.response_gradient(truth_grad[j] - approx_grad[j], j);
      }
      // -----------------------------
      // Additive 2nd order correction
      // -----------------------------
      if (dataOrder & 4) {
	const RealSymMatrix&  truth_hess =  truth_hessians[index];
	const RealSymMatrix& approx_hess = approx_hessians[index];
	for (j=0; j<numVars; ++j)
	  for (k=0; k<=j; ++k) // lower half
	    sdr.response_hessian(truth_hess(j,k) - approx_hess(j,k), j, k);
      }
      // update anchor data
      addCorrections[index].add(sdv, true); // shallow copy into SurrogateData
      addCorrections[index].add(sdr, true); // shallow copy into SurrogateData

      if (!quiet_flag)
	Cout << "\nAdditive correction computed:\n" << sdr;
    }
  }

  if (computeMultiplicative && !badScalingFlag) {
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      Pecos::SurrogateDataResp sdr(dataOrder, numVars);
      // -----------------------------------
      // Multiplicative 0th order correction
      // -----------------------------------
      const Real&  truth_fn =  truth_fns[index];
      const Real& approx_fn = approx_fns[index];
      Real ratio = truth_fn / approx_fn;
      if (dataOrder & 1)
	sdr.response_function(ratio);
      // -----------------------------------
      // Multiplicative 1st order correction
      // -----------------------------------
      // The beta-correction method is based on the work of Chang and Haftka,
      // and Alexandrov.  It is a multiplicative correction like the "scaled"
      // correction method, but it uses gradient information to achieve
      // 1st-order consistency (matches the high-fidelity function values and
      // the high-fidelity gradients at the center of the approximation region).
      if (dataOrder & 2) {
	const Real*  truth_grad =  truth_grads[index];
	const Real* approx_grad = approx_grads[index];
	for (j=0; j<numVars; ++j)
	  sdr.response_gradient( ( truth_grad[j] - approx_grad[j] * ratio )
				 / approx_fn, j);
      }
      // -----------------------------------
      // Multiplicative 2nd order correction
      // -----------------------------------
      if (dataOrder & 4) {
	const Real*           truth_grad =     truth_grads[index];
	const Real*          approx_grad =    approx_grads[index];
	const RealSymMatrix&  truth_hess =  truth_hessians[index];
	const RealSymMatrix& approx_hess = approx_hessians[index];
	// consider use of Teuchos assign and operator-=
	Real f_lo_2 = approx_fn * approx_fn;
	for (j=0; j<numVars; ++j)
	  for (k=0; k<=j; ++k) // lower half
	    sdr.response_hessian( ( truth_hess(j,k) * approx_fn - truth_fn *
	      approx_hess(j,k) + 2. * ratio * approx_grad[j] * approx_grad[k] -
	      truth_grad[j] * approx_grad[k] - approx_grad[j] * truth_grad[k] )
	      / f_lo_2, j, k);
      }
      // update anchor data
      multCorrections[index].add(sdv, true); // shallow copy into SurrogateData
      multCorrections[index].add(sdr, true); // shallow copy into SurrogateData

      if (!quiet_flag)
	Cout << "\nMultiplicative correction computed:\n" << sdr;
    }
  }

  // Compute combination factors once for each new correction.  combineFactors =
  // [f_hi(x_pp) - f_hi_beta(x_pp)]/[f_hi_alpha(x_pp) - f_hi_beta(x_pp)].  This
  // ratio goes -> 1 (use additive alone) if f_hi_alpha(x_pp) -> f_hi(x_pp) and
  // it goes -> 0 (use multiplicative alone) if f_hi_beta(x_pp) -> f_hi(x_pp).
  if (combinedFlag && !badScalingFlag && correctionComputed) {
    RealVector alpha_corr_fns = approxFnsPrevCenter,
                beta_corr_fns = approxFnsPrevCenter;
    apply_additive(correctionPrevCenterPt, alpha_corr_fns);
    apply_multiplicative(correctionPrevCenterPt, beta_corr_fns);
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      Real numer = truthFnsPrevCenter[index] - beta_corr_fns[index];
      Real denom =     alpha_corr_fns[index] - beta_corr_fns[index];
      combineFactors[index] = (std::fabs(denom) > 1.e-25) ? numer/denom : 1.;
#ifdef DEBUG
      Cout << "truth prev = " << truthFnsPrevCenter[index]
	   << " additive prev = " << alpha_corr_fns[index]
	   << " multiplicative prev = " << beta_corr_fns[index]
	   << "\nnumer = " << numer << " denom = " << denom << '\n';
#endif
    }
    if (!quiet_flag)
      Cout << "\nCombined correction computed: combination factors =\n"
	   << combineFactors << '\n';

#ifdef DEBUG
    Cout << "Testing final match at previous point\n";
    Response approx_copy = approx_response.copy();
    ActiveSet fns_set = approx_response.active_set(); // copy
    fns_set.request_values(1); // correct fn values only
    approx_copy.active_set(fns_set);
    approx_copy.function_values(approxFnsPrevCenter);
    apply(correctionPrevCenterPt, approx_copy);
#endif
  }

  if (computeAdditive || computeMultiplicative)
    correctionComputed = true;
}


void DiscrepancyCorrection::
compute(const RealVector& c_vars, const Response& truth_response, 
	const Response& approx_response, Response& discrepancy_response)
{
  // The incoming approx_response is assumed to be uncorrected (i.e.,
  // correction has not been applied to it previously).  In this case,
  // it is not necessary to back out a previous correction, and the
  // computation of the new correction is straightforward.

  const RealVector&    truth_fns =  truth_response.function_values();
  const RealVector&   approx_fns = approx_response.function_values();
  const RealMatrix&  truth_grads =  truth_response.function_gradients();
  const RealMatrix& approx_grads = approx_response.function_gradients();
  const RealSymMatrixArray& truth_hessians
    = truth_response.function_hessians();
  const RealSymMatrixArray& approx_hessians
    = approx_response.function_hessians();

  // update previous center data arrays for combined corrections
  // TO DO: augment approxFnsPrevCenter logic for data fit surrogates.  May
  // require additional fn evaluation of previous pt on current surrogate.
  // This could combine with DB lookups within apply_multiplicative()
  // (approx re-evaluated if not found in DB search).
  int index; size_t j, k; ISIter it;
  if (combinedFlag && correctionComputed) {
    approxFnsPrevCenter = approxFnsCenter;
    truthFnsPrevCenter  = truthFnsCenter;
    it = surrogateFnIndices.begin();
    // TO DO
    correctionPrevCenterPt = (computeAdditive || badScalingFlag) ?
      addCorrections[*it].approximation_data().anchor_continuous_variables() :
      multCorrections[*it].approximation_data().anchor_continuous_variables();
  }
  // update current center data arrays
  bool fall_back
    = (computeMultiplicative && correctionOrder >= 1 && surrModel.is_null());
  if (combinedFlag)                 truthFnsCenter = truth_fns;
  if (combinedFlag || fall_back)   approxFnsCenter = approx_fns;
  if (fall_back)                 approxGradsCenter = approx_grads;

  // detect numerical issues with multiplicative scaling
  badScalingFlag = (computeMultiplicative) ?
    check_scaling(truth_fns, approx_fns) : false;

  if (computeAdditive || badScalingFlag) {
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      // -----------------------------
      // Additive 0th order correction
      // -----------------------------
      if (dataOrder & 1)
	discrepancy_response.function_value(truth_fns[index] - 
					    approx_fns[index], index);
      // -----------------------------
      // Additive 1st order correction
      // -----------------------------
      if (dataOrder & 2) {
	const Real*  truth_grad =  truth_grads[index];
	const Real* approx_grad = approx_grads[index];
	RealVector discrep_grad
	  = discrepancy_response.function_gradient_view(index);
	for (j=0; j<numVars; ++j) // update view
	  discrep_grad[j] = truth_grad[j] - approx_grad[j];
      }
      // -----------------------------
      // Additive 2nd order correction
      // -----------------------------
      if (dataOrder & 4) {
	const RealSymMatrix&  truth_hess =  truth_hessians[index];
	const RealSymMatrix& approx_hess = approx_hessians[index];
	RealSymMatrix discrep_hess
	  = discrepancy_response.function_hessian_view(index);
	for (j=0; j<numVars; ++j)
	  for (k=0; k<=j; ++k) // lower half
	    discrep_hess(j,k) = truth_hess(j,k) - approx_hess(j,k);
      }

      //if (!quiet_flag)
      //  Cout << "\nAdditive correction computed:\n" << sdr;
    }
  }

  if (computeMultiplicative && !badScalingFlag) {
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      Pecos::SurrogateDataResp sdr(dataOrder, numVars);
      // -----------------------------------
      // Multiplicative 0th order correction
      // -----------------------------------
      const Real&  truth_fn =  truth_fns[index];
      const Real& approx_fn = approx_fns[index];
      Real ratio = truth_fn / approx_fn;
      if (dataOrder & 1)
	discrepancy_response.function_value(ratio, index);
      // -----------------------------------
      // Multiplicative 1st order correction
      // -----------------------------------
      // The beta-correction method is based on the work of Chang and Haftka,
      // and Alexandrov.  It is a multiplicative correction like the "scaled"
      // correction method, but it uses gradient information to achieve
      // 1st-order consistency (matches the high-fidelity function values and
      // the high-fidelity gradients at the center of the approximation region).
      if (dataOrder & 2) {
	const Real*  truth_grad =  truth_grads[index];
	const Real* approx_grad = approx_grads[index];
	RealVector discrep_grad
	  = discrepancy_response.function_gradient_view(index);
	for (j=0; j<numVars; ++j) // update view
	  discrep_grad[j] = (truth_grad[j] - approx_grad[j] * ratio)/approx_fn;
      }
      // -----------------------------------
      // Multiplicative 2nd order correction
      // -----------------------------------
      if (dataOrder & 4) {
	const Real*           truth_grad =     truth_grads[index];
	const Real*          approx_grad =    approx_grads[index];
	const RealSymMatrix&  truth_hess =  truth_hessians[index];
	const RealSymMatrix& approx_hess = approx_hessians[index];
	RealSymMatrix discrep_hess
	  = discrepancy_response.function_hessian_view(index);
	// consider use of Teuchos assign and operator-=
	Real f_lo_2 = approx_fn * approx_fn;
	for (j=0; j<numVars; ++j)
	  for (k=0; k<=j; ++k) // lower half
	    discrep_hess(j,k) = ( truth_hess(j,k) * approx_fn - truth_fn *
	      approx_hess(j,k) + 2. * ratio * approx_grad[j] * approx_grad[k] -
	      truth_grad[j] * approx_grad[k] - approx_grad[j] * truth_grad[k] )
	      / f_lo_2;
      }

      //if (!quiet_flag)
      //  Cout << "\nMultiplicative correction computed:\n" << sdr;
    }
  }
}


bool DiscrepancyCorrection::
check_scaling(const RealVector& truth_fns, const RealVector& approx_fns)
{
  // Multiplicative will fail if response functions are near zero.
  //   0th order:     a truth_val == 0 causes a zero scaling which will cause
  //                  optimization failure; an approx_val == 0 will cause a
  //                  division by zero FPE.
  //   1st/2nd order: a truth_val == 0 is OK (so long as the total scaling
  //                  function != 0); an approx_val == 0 will cause a division
  //                  by zero FPE.
  // In either case, automatically transition to additive correction.  Current
  // logic transitions back to multiplicative as soon as the response fns are
  // no longer near zero.
  bool bad_scaling = false; int index; ISIter it;
  for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
    index = *it;
    if ( std::fabs(approx_fns[index]) < 1.e-25 ||
	 ( correctionOrder == 0 && std::fabs(truth_fns[index]) < 1.e-25 ) )
      { bad_scaling = true; break; }
  }
  if (bad_scaling)
    Cout << "\nWarning: Multiplicative correction temporarily deactivated "
	 << "due to functions near zero.\n         Additive correction will "
	 << "be used.\n";
  return bad_scaling;
}


void DiscrepancyCorrection::
apply(const RealVector& c_vars, Response& approx_response, bool quiet_flag)
{
  if (!correctionComputed)
    return;

  // update approx_response with the alpha/beta/combined corrected data
  if (correctionType == "additive" || badScalingFlag) // use alpha_corrected_*
    apply_additive(c_vars, approx_response);
  else if (correctionType == "multiplicative") // use beta_corrected_*
    apply_multiplicative(c_vars, approx_response);
  else if (combinedFlag) { // use both alpha_corrected_* and beta_corrected_*

    // compute {add,mult}_response contributions to combined correction
    Response add_response = approx_response.copy(),
            mult_response = approx_response.copy();
    apply_additive(c_vars, add_response);
    apply_multiplicative(c_vars, mult_response);

    // compute convex combination of add_response and mult_response
    ISIter it; int index; size_t j, k;
    const ShortArray& asv = approx_response.active_set_request_vector();
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      Real cf = combineFactors[index], ccf = 1. - cf;
      if (asv[index] & 1) {
	Real corrected_fn =  cf *  add_response.function_value(index)
	                  + ccf * mult_response.function_value(index);
	approx_response.function_value(corrected_fn, index);
      }
      if (asv[index] & 2) {
	RealVector corrected_grad
	  = approx_response.function_gradient_view(index);
	const Real*  add_grad =  add_response.function_gradient(index);
	const Real* mult_grad = mult_response.function_gradient(index);
	for (j=0; j<numVars; j++)
	  corrected_grad[j] = cf * add_grad[j] + ccf * mult_grad[j];
      }
      if (asv[index] & 4) {
	RealSymMatrix corrected_hess
	  = approx_response.function_hessian_view(index);
	const RealSymMatrix&  add_hess =  add_response.function_hessian(index);
	const RealSymMatrix& mult_hess = mult_response.function_hessian(index);
	for (j=0; j<numVars; ++j)
	  for (k=0; k<=j; ++k)
	    corrected_hess(j,k) = cf * add_hess(j,k) + ccf * mult_hess(j,k);
      }
    }
  }

  if (!quiet_flag)
    Cout << "\nCorrection applied: corrected response =\n" << approx_response;
}


void DiscrepancyCorrection::
apply_additive(const RealVector& c_vars, Response& approx_response)
{
  size_t index; ISIter it;
  const ShortArray& asv = approx_response.active_set_request_vector();
  for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
    index = *it;
    Approximation& add_corr = addCorrections[index];
    if (asv[index] & 1)
      approx_response.function_value(approx_response.function_value(index) +
				     add_corr.get_value(c_vars), index);
    if (correctionOrder >= 1 && asv[index] & 2) {
      // update view (no reassignment):
      RealVector approx_grad = approx_response.function_gradient_view(index);
      approx_grad += add_corr.get_gradient(c_vars);
    }
    if (correctionOrder == 2 && asv[index] & 4) {
      // update view (no reassignment):
      RealSymMatrix approx_hess = approx_response.function_hessian_view(index);
      approx_hess += add_corr.get_hessian(c_vars);
    }
  }
}


void DiscrepancyCorrection::
apply_multiplicative(const RealVector& c_vars, Response& approx_response)
{
  // Determine needs for retrieving uncorrected data due to cases where
  // the data required to apply the correction is different from the
  // active data being corrected.
  bool fn_db_search = false, grad_db_search = false;
  const ShortArray& asv = approx_response.active_set_request_vector();
  ShortArray fn_db_asv, grad_db_asv; ISIter it; size_t j, k, index;
  for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
    index = *it;
    if ( !(asv[index] & 1) && ( ((asv[index] & 2) && correctionOrder >= 1) ||
				((asv[index] & 4) && correctionOrder == 2) ) ) {
      if (fn_db_asv.empty()) fn_db_asv.assign(numFns, 0);
      fn_db_asv[index] = 1; fn_db_search = true;
    }
    if ( !(asv[index] & 2) && (asv[index] & 4) && correctionOrder >= 1) {
      if (grad_db_asv.empty()) grad_db_asv.assign(numFns, 0);
      grad_db_asv[index] = 2; grad_db_search = true;
    }
  }
  // Retrieve any uncorrected fn values/gradients required for use in gradient
  // and Hessian corrections.  They are not immediately available in cases where
  // the correction is applied only to the fn gradient (i.e., when the current
  // asv contains 2's due to DOT/CONMIN/OPT++ requesting gradients separately)
  // or only to the fn Hessian (i.e., when the current asv contains 4's due to
  // OPT++ requesting Hessians separately).  If surrModel is initialized, we
  // lookup the data in data_pairs and re-evaluate if not found.  If surrModel
  // is not initialized, we fall back to using uncorrected data from the center
  // of the current trust region (TR).  This still satisifies the required
  // consistency at the TR center, but is less accurate over the rest of the TR.
  RealVector uncorr_fns; RealMatrix uncorr_grads; RealVector empty_rv;
  if (fn_db_search) {
    uncorr_fns.sizeUninitialized(numFns);
    if (surrModel.is_null()) { // fallback position
      Cerr << "Warning: original function values not available at the current "
	   << "point.\n         Multiplicative correction falling back to "
	   << "function values from the correction point." << std::endl;
      for (size_t i=0; i<numFns; ++i)
	if (fn_db_asv[i])
	  uncorr_fns[i] = approxFnsCenter[i];
    }
    else {
      const Response& db_resp = search_db(c_vars, fn_db_asv);
      for (size_t i=0; i<numFns; ++i)
	if (fn_db_asv[i])
	  uncorr_fns[i] = db_resp.function_value(i);
    }
  }
  if (grad_db_search) {
    uncorr_grads.shapeUninitialized(numVars, numFns);
    if (surrModel.is_null()) { // fallback position
      Cerr << "Warning: original function gradients not available at the "
	   << "current point.\n         Multiplicative correction falling back "
	   << "to function gradients from the correction point." << std::endl;
      for (int i=0; i<numFns; ++i)
	if (grad_db_asv[i])
	  Teuchos::setCol(Teuchos::getCol(Teuchos::View, approxGradsCenter, i),
			  i, uncorr_grads);
    }
    else {
      const Response& db_resp = search_db(c_vars, grad_db_asv);
      for (int i=0; i<numFns; ++i)
	if (grad_db_asv[i])
	  Teuchos::setCol(db_resp.function_gradient_view(i), i, uncorr_grads);
    }
  }

  for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
    index = *it;
    Approximation&    mult_corr = multCorrections[index];
    const Real&         fn_corr = mult_corr.get_value(c_vars);
    const RealVector& grad_corr = (correctionOrder >= 1 && (asv[index] & 6)) ?
      mult_corr.get_gradient(c_vars) : empty_rv;
    // apply corrections in descending derivative order to avoid
    // disturbing original approx fn/grad values
    if (asv[index] & 4) {
      // update view (no reassignment):
      RealSymMatrix approx_hess = approx_response.function_hessian_view(index);
      const Real*   approx_grad = (grad_db_search) ? uncorr_grads[index] :
	approx_response.function_gradient(index);
      switch (correctionOrder) {
      case 2: {
	const RealSymMatrix& hess_corr = mult_corr.get_hessian(c_vars);
	const Real& approx_fn = (fn_db_search) ? uncorr_fns[index] :
	  approx_response.function_value(index);
	for (j=0; j<numVars; ++j)
	  for (k=0; k<=j; ++k)
	    approx_hess(j,k) = approx_hess(j,k) * fn_corr
	      + hess_corr(j,k) * approx_fn + grad_corr[j] * approx_grad[k]
	      + grad_corr[k] * approx_grad[j];
	break;
      }
      case 1:
	for (j=0; j<numVars; ++j)
	  for (k=0; k<=j; ++k)
	    approx_hess(j,k) = approx_hess(j,k) * fn_corr
	      + grad_corr[j] * approx_grad[k] + grad_corr[k] * approx_grad[j];
	break;
      case 0:
	approx_hess *= fn_corr;
	break;
      }
    }
    if (asv[index] & 2) {
      // update view (no reassignment):
      RealVector approx_grad = approx_response.function_gradient_view(index);
      const Real& approx_fn  = (fn_db_search) ? uncorr_fns[index] :
	approx_response.function_value(index);
      approx_grad *= fn_corr; // all correction orders
      if (correctionOrder >= 1)
	for (j=0; j<numVars; ++j)
	  approx_grad[j] += grad_corr[j] * approx_fn;
    }
    if (asv[index] & 1)
      approx_response.function_value(approx_response.function_value(index) *
				     fn_corr, index);
  }
}


const Response& DiscrepancyCorrection::
search_db(const RealVector& c_vars, const ShortArray& search_asv)
{
  // Retrieve missing uncorrected approximate data for use in derivative
  // multiplicative corrections.  The correct approach is to retrieve the
  // missing data for the current point in parameter space, and this approach
  // is when data is either available directly as indicated by the asv, can be
  // retrieved via a data_pairs search, or can be recomputed).  A final fallback
  // is to employ approx center data; this approach is used when surrModel has
  // not been initialized (see apply_multiplicative()).  Recomputation can occur
  // either for ApproximationInterface data in DataFitSurrModels or low fidelity
  // data in HierarchSurrModels that involves additional model recursions, since
  // neither of these data sets are catalogued in data_pairs.

  // query data_pairs to extract the response at the current pt
  Variables search_vars = surrModel.current_variables().copy();     // copy
  search_vars.continuous_variables(c_vars);
  ActiveSet search_set = surrModel.current_response().active_set(); // copy
  search_set.request_vector(search_asv);
  extern PRPCache data_pairs; // global container
  PRPCacheHIter cache_it = lookup_by_val(data_pairs, surrModel.interface_id(),
					 search_vars, search_set);

  if (cache_it == data_pairs.get<hashed>().end()) {
    // perform approx fn eval to retrieve missing data
    surrModel.continuous_variables(c_vars);
    surrModel.compute_response(search_set);
    return surrModel.current_response();
  }
  else
    return cache_it->prp_response();
}

} // namespace Dakota
