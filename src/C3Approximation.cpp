/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "C3Approximation.hpp"
#include "C3FnTrainPtrsRep.hpp"
#include "ProblemDescDB.hpp"
#include "SharedC3ApproxData.hpp"
#include "DiscrepancyCalculator.hpp"

// NOTE: only include this header in isolated compilation units
#include "dakota_c3_include.hpp"

//#define DEBUG

namespace Dakota {

// Definitions for C3FnTrainPtrs handle functions (in .cpp to isolate
// the implementation details of the Rep, which require C3 APIs)

C3FnTrainPtrs::C3FnTrainPtrs(): ftpRep(new C3FnTrainPtrsRep())
{ } // body allocated with null FT pointers


// BMA: If we don't anticipate needing a full deep copy (with stats as
// opposed to the partial deep copy implemented here, could make this
// the copy ctor of the body and just use the default copy for the handle
C3FnTrainPtrs C3FnTrainPtrs::copy() const
{
  C3FnTrainPtrs ftp; // new envelope with ftpRep default allocated

  ftp.ftpRep->ft          = (ftpRep->ft          == NULL) ? NULL :
    function_train_copy(ftpRep->ft);
  ftp.ftpRep->ft_gradient = (ftpRep->ft_gradient == NULL) ? NULL :
    ft1d_array_copy(ftpRep->ft_gradient);
  ftp.ftpRep->ft_hessian  = (ftpRep->ft_hessian  == NULL) ? NULL :
    ft1d_array_copy(ftpRep->ft_hessian);

  // ft_derived_fns,ft_sobol have been assigned NULL and can be allocated
  // downsteam when needed for stats,indices

  return ftp;
}


void C3FnTrainPtrs::swap(C3FnTrainPtrs& ftp)
{ ftpRep.swap(ftp.ftpRep); }


// TO DO: shallow copy would be better for this case, but requires ref counting
C3FnTrainPtrs::C3FnTrainPtrs(const C3FnTrainPtrs& ftp)
{ ftpRep = ftp.ftpRep; }


C3FnTrainPtrs::~C3FnTrainPtrs()
{ }


C3FnTrainPtrs& C3FnTrainPtrs::operator=(const C3FnTrainPtrs& ftp)
{ ftpRep = ftp.ftpRep; return *this; }

// Note: the following functions init/create/free ft memory within an ftpRep
//       but do not alter ftpRep accounting

void C3FnTrainPtrs::free_ft()
{ ftpRep->free_ft(); }


void C3FnTrainPtrs::free_all()
{ ftpRep->free_all(); }


void C3FnTrainPtrs::ft_derived_functions_init_null()
{ ftpRep->ft_derived_functions_init_null(); }


void C3FnTrainPtrs::
ft_derived_functions_create(struct MultiApproxOpts * opts, size_t num_mom,
			    Real round_tol)
{ ftpRep->ft_derived_functions_create(opts, num_mom, round_tol); }


void C3FnTrainPtrs::
ft_derived_functions_create_av(struct MultiApproxOpts * opts,
			       const SizetArray& rand_indices, Real round_tol)
{ ftpRep->ft_derived_functions_create_av(opts, rand_indices, round_tol); }


void C3FnTrainPtrs::ft_derived_functions_free()
{ ftpRep->ft_derived_functions_free(); }


struct FunctionTrain * C3FnTrainPtrs::function_train()
{ return ftpRep->ft; }


void C3FnTrainPtrs::function_train(struct FunctionTrain * ft)
{ ftpRep->ft = ft; }


struct FT1DArray * C3FnTrainPtrs::ft_gradient()
{ return ftpRep->ft_gradient; }


void C3FnTrainPtrs::ft_gradient(struct FT1DArray * ftg)
{ ftpRep->ft_gradient = ftg; }


struct FT1DArray * C3FnTrainPtrs::ft_hessian()
{ return ftpRep->ft_hessian; }


void C3FnTrainPtrs::ft_hessian(struct FT1DArray * fth)
{ ftpRep->ft_hessian = fth; }


const struct FTDerivedFunctions& C3FnTrainPtrs::derived_functions()
{ return ftpRep->ft_derived_fns; }


struct C3SobolSensitivity * C3FnTrainPtrs::sobol()
{ return ftpRep->ft_sobol; }


void C3FnTrainPtrs::sobol(struct C3SobolSensitivity * ss)
{ ftpRep->ft_sobol = ss; }


////////////////////////////////////////////////////////////////////////////////


C3Approximation::
C3Approximation(ProblemDescDB& problem_db,
		const SharedApproxData& shared_data,
		const String& approx_label):
  Approximation(BaseConstructor(), problem_db, shared_data, approx_label),
  levApproxIter(levelApprox.end()), primaryMomIter(primaryMoments.end())
{ } // FT memory allocations managed by C3FnTrainPtrs


C3Approximation::C3Approximation(const SharedApproxData& shared_data):
  Approximation(NoDBBaseConstructor(), shared_data),
  levApproxIter(levelApprox.end()), primaryMomIter(primaryMoments.end())
{ } // FT memory allocations managed by C3FnTrainPtrs


C3Approximation::~C3Approximation()
{ } // FT memory deallocations managed by C3FnTrainPtrs


SizetVector C3Approximation::function_train_ranks()
{
  return SizetVector(Teuchos::View,
    function_train_get_ranks(levApproxIter->second.function_train()),
    sharedDataRep->numVars + 1);
}


void C3Approximation::build()
{
  if (sharedDataRep->outputLevel >= DEBUG_OUTPUT)
    Cout << "Calling C3Approximation::build()\n";

  // compute modifications to surrogate data, if any
  synchronize_surrogate_data();

  // base class implementation checks data set against min required
  Approximation::build();

  SharedC3ApproxData* data_rep = (SharedC3ApproxData*)sharedDataRep;
  if (data_rep->adaptConstruct) {
    Cerr << "Error: Adaptive construction not yet implemented in "
	 << "C3Approximation." << std::endl;
    abort_handler(APPROX_ERROR);
  }
  else {
    size_t i, j, num_v = sharedDataRep->numVars, kick_r = data_rep->kickRank,
      max_r = data_rep->max_rank(), // bounds CV candidates for adapt_rank
      start_r = std::min(data_rep->start_rank(), max_r);
    SizetVector start_ranks(num_v+1);
    start_ranks(0) = 1;     start_ranks(num_v) = 1;
    for (i=1; i<num_v; ++i) start_ranks(i) = start_r;

    struct FTRegress * ftr = ft_regress_alloc(num_v, data_rep->multiApproxOpts,
					      start_ranks.values());

    if (data_rep->regressType == FT_RLS2) {
      ft_regress_set_alg_and_obj(ftr, AIO, FTLS_SPARSEL2);
      // reg param is required (no reasonable default due to scaling)
      ft_regress_set_regularization_weight(ftr, data_rep->regressRegParam);
    }
    else // default
      ft_regress_set_alg_and_obj(ftr, AIO, FTLS);

    size_t r_adapt = data_rep->adaptRank ? 1 : 0;
    ft_regress_set_adapt(ftr, r_adapt);
    if (r_adapt) {
      ft_regress_set_kickrank(ftr, kick_r); // default is 1

      // if not user-specified, use internal C3 default (in src/lib_superlearn/
      // regress.c, maxrank = 10 assigned in ft_regress_alloc())
      // > default could become an issue for UNIFORM_START_RANK advancement
      if (max_r != std::numeric_limits<size_t>::max())
	ft_regress_set_maxrank(ftr, max_r);

      ft_regress_set_kfold(ftr, 5);//kfold);//match Alex's Python (C3 default=3)
    }
    ft_regress_set_roundtol(ftr, data_rep->solverRoundingTol);
    short output_lev = data_rep->outputLevel;
    if (output_lev > NORMAL_OUTPUT)
      ft_regress_set_verbose(ftr, 1); // helpful adapt_rank diagnostics

    struct c3Opt* optimizer = c3opt_create(BFGS);
    int max_solver_iter = data_rep->maxSolverIterations;
    if (max_solver_iter >= 0) { // Dakota default is -1 -> leave at C3 default
      c3opt_set_maxiter(   optimizer, max_solver_iter);
      c3opt_ls_set_maxiter(optimizer, max_solver_iter); // line search
    }
    c3opt_set_gtol   (optimizer, data_rep->solverTol);
    c3opt_set_relftol(optimizer, data_rep->solverTol);
    double absxtol = 1e-30;//1e-10; // match Alex's Python
    c3opt_set_absxtol(optimizer, absxtol);
    if (output_lev >= DEBUG_OUTPUT)
      c3opt_set_verbose(optimizer, 1); // per opt iter diagnostics (a bit much)

    // free if previously built
    C3FnTrainPtrs& ftp = levApproxIter->second;
    ftp.free_all();

    if (data_rep->crossVal) // future capability for poly orders
      Cerr << "Warning: CV is not yet implemented in C3Approximation.  "
	   << "Ignoring CV request.\n";

    const Pecos::SDVArray& sdv_array = approxData.variables_data();
    const Pecos::SDRArray& sdr_array = approxData.response_data();
    size_t ndata = approxData.points();

    // Training data for 1 QoI: transfer data from approxData to double* for C3
    double* xtrain = (double*)calloc(num_v*ndata, sizeof(double)); // vars
    double* ytrain = (double*)calloc(ndata,       sizeof(double)); // QoI
    for (i=0; i<ndata; ++i) {
      const RealVector& c_vars = sdv_array[i].continuous_variables();
      for (j=0; j<num_v; j++)
	xtrain[j + i*num_v] = c_vars[j];
      ytrain[i] = sdr_array[i].response_function();
    }
#ifdef DEBUG
    RealMatrix  in(Teuchos::View, xtrain, num_v, num_v, ndata);
    RealVector out(Teuchos::View, ytrain, ndata);
    Cout << "C3 training data:\n" << in << out << std::endl;
#endif // DEBUG

    // Build FT model
    ft_regress_set_seed(ftr, data_rep->randomSeed);
    struct FunctionTrain * ft
      = ft_regress_run(ftr, optimizer, ndata, xtrain, ytrain);
    ftp.function_train(ft);
    // Important distinction among derivative cases:
    // > expansionCoeffGradFlag: expansions of derivs w.r.t. inactive/non-build/
    //   non-random vars (not yet supported, but would be managed here)
    // > derivative-enhanced regression (derivs w.r.t. active/build vars;
    //   not yet supported, but would be managed here)
    // > evaluation of derivs of expansions w.r.t. build variables: build a
    //   separate FT expansion by differentiating the value-based expansion;
    //   these new functions that can then be interrogated at particular points.
    //   >> This _is_ currently supported, but rather than precomputing based
    //      on flags / potential for future deriv evals, compute derivative
    //      expansions on demand using helper fns within evaluators
    //if (...supportDerivEvals...) {
    //  struct FT1DArray * ftg = function_train_gradient(ft);
    //  ftp.ft_gradient(ftg);
    //  ftp.ft_hessian(ft1d_array_jacobian(ftg));
    //}
    if (data_rep->outputLevel > SILENT_OUTPUT) {
      Cout << "\nFunction train build() results:\n  Ranks ";
      if (data_rep->adaptRank)
	Cout << "(adapted with start = " << start_r << " kick = " << kick_r
	     << " max = " << max_r << "):\n";
      else Cout << "(non-adapted):\n";
      write_data(Cout, function_train_get_ranks(ft), num_v+1);
      Cout << "  Polynomial order (non-adapted):\n";
      std::vector<OneApproxOpts*> opts = data_rep->oneApproxOpts;
      for (i=0; i<num_v; ++i)
	Cout << "                     " << std::setw(write_precision+7)
	     << one_approx_opts_get_nparams(opts[i]) - 1 << '\n';
      Cout << "  C3 regression size:  " << function_train_get_nparams(ft)
	   << std::endl;
    }

    // free approximation stuff
    free(xtrain);          xtrain    = NULL;
    free(ytrain);          ytrain    = NULL;
    ft_regress_free(ftr);  ftr       = NULL;
    c3opt_free(optimizer); optimizer = NULL;
  }
}


void C3Approximation::rebuild()
{
  SharedC3ApproxData* data_rep = (SharedC3ApproxData*)sharedDataRep;
  active_model_key(data_rep->activeKey);

  // for use in pop_coefficients()
  prevC3FTPtrs = levApproxIter->second.copy(); // deep copy

  build(); // updates levApproxIter->second
}


void C3Approximation::pop_coefficients(bool save_data)
{
  SharedC3ApproxData* data_rep = (SharedC3ApproxData*)sharedDataRep;
  const UShortArray& key = data_rep->activeKey;

  // likely overkill, but multilevel roll up after increment modifies and
  // then restores active key
  active_model_key(key);

  C3FnTrainPtrs& active_ftp = levApproxIter->second;
  // store the incremented coeff state for possible push
  if (save_data) // shallow copy enabled by swap to follow
    poppedLevelApprox[key].push_back(active_ftp);
  // reset expansion to previous state.  After swap of reps / ref counts,
  // poppedLevelApprox[key].back() shares Rep with prevC3FTPtrs.  prevC3FTPtrs
  // then gets replaced with a new instance in rebuild or push_coefficients().
  active_ftp.swap(prevC3FTPtrs);

  //clear_computed_bits();
}


void C3Approximation::push_coefficients()
{
  SharedC3ApproxData* data_rep = (SharedC3ApproxData*)sharedDataRep;
  const UShortArray& key = data_rep->activeKey;

  // synchronize expansionCoeff{s,Grads} and approxData
  active_model_key(key);

  // store current state for use in pop_coefficients()
  C3FnTrainPtrs& active_ftp = levApproxIter->second;
  prevC3FTPtrs = active_ftp.copy(); // deep copy

  // retrieve a previously popped state
  std::map<UShortArray, std::deque<C3FnTrainPtrs> >::iterator prv_it
    = poppedLevelApprox.find(key);
  bool err_flag = false;
  if (prv_it == poppedLevelApprox.end())
    err_flag = true;
  else {
    // SharedPolyApproxData::candidate_index() currently returns 0 for
    // all cases other than generalized sparse grids
    size_t p_index = data_rep->push_index(key); // *** TO DO
    std::deque<C3FnTrainPtrs>& ftp_deque = prv_it->second;
    if (p_index >= ftp_deque.size())
      err_flag = true;
    else {
      std::deque<C3FnTrainPtrs>::iterator rv_it	= ftp_deque.begin() + p_index;
      // reset expansion to popped state.  Shallow copy is appropriate
      // (levApprox assumes popped state prior to erase from bookkeeping).
      active_ftp = *rv_it;    // shallow copy of popped state
      ftp_deque.erase(rv_it); // removal of original
    }
  }

  if (err_flag) {
    Cerr << "Error: lookup of previously popped data failed in C3Approximation"
	 << "::push_coefficients()." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  //clear_computed_bits();
}


void C3Approximation::combine_coefficients()
{
  // SharedC3ApproxData::pre_combine() invokes update_basis(combinedOrders)
  // > opts below reflect the maximum basis order from all model indices

  // Option 1: adds x to y and overwrites y (I allocate x and y)
  combinedC3FTPtrs.free_ft();
  std::map<UShortArray, C3FnTrainPtrs>::iterator it = levelApprox.begin();
  struct FunctionTrain * y = function_train_copy(it->second.function_train());
  ++it;
  // Note: the FT rounding tolerance is relative and default (1.e-8) is too
  // tight for this context --> use arithmetic tol.  Memory overhead is strongly
  // correlated with this tolerance and 1.e-3 did not result in significant
  // accuracy gain in some numerical experiments (dakota_uq_heat_eq_mlft.in).
  SharedC3ApproxData* data_rep = (SharedC3ApproxData*)sharedDataRep;
  Real arith_tol = data_rep->statsRoundingTol;
  struct MultiApproxOpts * opts = data_rep->multiApproxOpts;
  for (; it!= levelApprox.end(); ++it)
    c3axpy(1., it->second.function_train(), &y, arith_tol, opts);
  combinedC3FTPtrs.function_train(y);

  // Could also do this at the C3FnTrainPtrs level with ft1d_array support:
  //combinedC3FTPtrs = it->second.copy(); ++it;
  //for (; it!= levelApprox.end(); ++it)
  //  sum ft,ft_gradient,ft_hessian...

  // Option 2: function_train_sum (I allocate a and b and C3 allocates c)
  // > remember to deallocate c when done
  //struct FunctionTrain* c = function_train_sum(a, b);

  // This replaces fine-grained moment bit trackers with a single override
  const SizetArray& rand_ind = data_rep->randomIndices;
  size_t num_mom = combinedMoments.length();
  if (rand_ind.empty() || rand_ind.size() == data_rep->numVars)
    compute_derived_statistics(   combinedC3FTPtrs, num_mom, true);// overwrite
  else
    compute_derived_statistics_av(combinedC3FTPtrs, num_mom, true);// overwrite
}


void C3Approximation::combined_to_active_coefficients(bool clear_combined)
{
  SharedC3ApproxData* data_rep = (SharedC3ApproxData*)sharedDataRep;
  active_model_key(data_rep->activeKey);

  levApproxIter->second = combinedC3FTPtrs;//.copy();
  //if (clear_combined)
  //  combinedC3FTPtrs.free_all();

  //allocate_component_sobol();  // size sobolIndices from shared sobolIndexMap

  // If outgoing stats type is active (e.g., as in Dakota::NonDExpansion::
  // multifidelity_expansion()), then previous active stats are invalidated.
  // But if outgoing stats type is combined, then can avoid recomputation
  // and carry over current moment stats from combined to active. 
  // Note: this reuse optimization introduces an order dependency --> updating
  //       stats type from COMBINED to ACTIVE must occur after this function
  //if (data_rep->refineStatsType == ACTIVE_EXPANSION_STATS)
  //  clear_computed_bits();
}


void C3Approximation::clear_inactive_coefficients()
{
  std::map<UShortArray, C3FnTrainPtrs>::iterator it = levelApprox.begin();
  while (it != levelApprox.end())
    if (it == levApproxIter) // preserve active
      ++it;
    else // clear inactive: postfix increments manage iterator invalidations
      levelApprox.erase(it++);
}


/*
void C3Approximation::link_multilevel_surrogate_data()
{
  // Manage {surr,modSurr}Data instances:
  // > SurrogateModel::aggregate_response() uses order of HF,LF
  // > ApproximationInterface::{mixed,shallow}_add() assigns aggregate response
  //   data to each approxData instance in turn.

  SharedC3ApproxData* data_rep = (SharedC3ApproxData*)sharedDataRep;
  switch (data_rep->discrepancyType) {
  case Pecos::DISTINCT_DISCREP:  case Pecos::RECURSIVE_DISCREP: {
    // push another SurrogateData instance for modSurrData
    // (allows consolidation of Approximation::push/pop operations)
    const UShortArray& key = approxData.back().active_key();
    Pecos::SurrogateData mod_surr(key); // new instance
    approxData.push_back(mod_surr);
    // Note: {orig,mod}SurrDataIndex set to {0,1} in SharedC3ApproxData::
    //       link_multilevel_surrogate_data()
    break;
  }
  default: // default ctor linkages are sufficient
    break;
  }
}
*/


void C3Approximation::synchronize_surrogate_data()
{
  SharedC3ApproxData* data_rep = (SharedC3ApproxData*)sharedDataRep;
  const UShortArray& active_key = data_rep->activeKey;
  if (active_key != approxData.active_key()) {
    PCerr << "Error: active key mismatch in C3Approximation::"
	  << "synchronize_surrogate_data()." << std::endl;
    abort_handler(-1);
  }

  // level 0: approxData non-aggregated key stores raw data
  short discrep_type = data_rep->discrepancyType,
        combine_type = data_rep->combineType;
  if (!discrep_type ||
      !Pecos::DiscrepancyCalculator::aggregated_key(active_key))
    return;

  switch (discrep_type) {
  case Pecos::RECURSIVE_DISCREP:
    // When using a recursive discrepancy with additive/multiplicative corr,
    // we will subtract/divide the current polynomial approx prediction from
    // the new surrData so that we form an expansion on the surplus.  Prior
    // to using compute() to form the surplus, LF-hat must be generated and
    // will be stored within surrData in a format that compute() can utilize.
    generate_synthetic_data(approxData, active_key, combine_type);
    break;
  //case Pecos::DISTINCT_DISCREP:
    // When using a distinct discrepancy with additive/multiplicative corr,
    // we will subtract/divide the HF,LF pairs.  In this case, the data is
    // already provided within surrData and specific pairings are identified
    // by data groups.
  }
  // now compute the discrepancy between {HF,LF} or {HF,LF-hat} datasets
  Pecos::DiscrepancyCalculator::compute(approxData, active_key, combine_type);
}


void C3Approximation::
generate_synthetic_data(Pecos::SurrogateData& surr_data,
			const UShortArray& active_key, short combine_type)
{
  // Modeled after Pecos::PolynomialApproximation::generate_synthetic_data()

  UShortArray hf_key, lf0_key, lf_hat_key; // LF-hat in surplus case
  Pecos::DiscrepancyCalculator::extract_keys(active_key, hf_key, lf_hat_key);
  lf0_key = surr_data.filtered_key(Pecos::RAW_DATA_FILTER, 0);

  // initialize surr_data[lf_hat_key]
  surr_data.active_key(lf_hat_key); // active key restored at fn end
  surr_data.variables_data(surr_data.variables_data(hf_key)); // shallow copies
  surr_data.anchor_index(surr_data.anchor_index(hf_key));
  surr_data.pop_count_stack(surr_data.pop_count_stack(hf_key));

  const Pecos::SDRArray& hf_sdr_array = surr_data.response_data(hf_key);
  surr_data.size_active_sdr(hf_sdr_array); // size lf_hat_sdr_array
  const Pecos::SDVArray&  sdv_array = surr_data.variables_data();
  Pecos::SDRArray& lf_hat_sdr_array = surr_data.response_data();

  // extract all discrepancy data sets (which have expansions supporting
  // stored_{value,gradient} evaluations)
  const std::map<UShortArray, Pecos::SDRArray>& discrep_resp_map
    = surr_data.filtered_response_data_map(Pecos::AGGREGATED_DATA_FILTER);
  std::map<UShortArray, Pecos::SDRArray>::const_iterator cit;
  size_t i, num_pts = hf_sdr_array.size();
  switch (combine_type) {
  case Pecos::MULT_COMBINE: {
    Real stored_val, fn_val_j, fn_val_jm1;
    //RealVector fn_grad_j, fn_grad_jm1;
    size_t j;//, k, num_deriv_vars = surr_data.num_derivative_variables();
    for (i=0; i<num_pts; ++i) {
      const RealVector& c_vars = sdv_array[i].continuous_variables();
      Pecos::SurrogateDataResp& lf_hat_sdr  = lf_hat_sdr_array[i];
      short                     lf_hat_bits = lf_hat_sdr.active_bits();
      // start from emulation of lowest fidelity QoI (LF-hat)
      fn_val_j = stored_value(c_vars, lf0_key); // coarsest fn
      //if (lf_hat_bits & 2)                      // coarsest grad
      //  fn_grad_j = stored_gradient_nonbasis_variables(c_vars, lf0_key);
      // augment w/ emulation of discrepancies (Delta-hat) preceding active_key
      for (cit = discrep_resp_map.begin(), j=0;
	   cit->first != active_key; ++cit, ++j) {
	stored_val = stored_value(c_vars, cit->first); // Delta-hat
	/*
	if (lf_hat_bits & 2) { / recurse using levels j and j-1
	  const RealVector& stored_grad   // discrepancy gradient-hat
	    = stored_gradient_nonbasis_variables(c_vars, cit->first);
	  fn_val_jm1 = fn_val_j;  fn_grad_jm1 = fn_grad_j;
	  for (k=0; k<num_deriv_vars; ++k) // grad corrected to level j
	    fn_grad_j[k] = ( fn_grad_jm1[k] * stored_val +
			     fn_val_jm1 * stored_grad[k] );
	}
	*/
	fn_val_j *= stored_val; // fn corrected to level j
      }
      if (lf_hat_bits & 1)
	lf_hat_sdr.response_function(fn_val_j);
      //if (lf_hat_bits & 2)
      //  lf_hat_sdr.response_gradient(fn_grad_j);
    }
    break;
  }
  default: { //case Pecos::ADD_COMBINE: (correction specification not required)
    Real sum_val;  RealVector sum_grad;
    for (i=0; i<num_pts; ++i) {
      const RealVector& c_vars = sdv_array[i].continuous_variables();
      Pecos::SurrogateDataResp& lf_hat_sdr  = lf_hat_sdr_array[i];
      short                     lf_hat_bits = lf_hat_sdr.active_bits();
      if (lf_hat_bits & 1) {
	sum_val = stored_value(c_vars, lf0_key);
	for (cit = discrep_resp_map.begin(); cit->first != active_key; ++cit)
	  sum_val += stored_value(c_vars, cit->first);
	lf_hat_sdr.response_function(sum_val);
      }
      /*
      if (lf_hat_bits & 2) {
	sum_grad = stored_gradient_nonbasis_variables(c_vars, lf0_key);
	for (cit = discrep_resp_map.begin(); cit->first != active_key; ++cit)
	  sum_grad += stored_gradient_nonbasis_variables(c_vars, cit->first);
	lf_hat_sdr.response_gradient(sum_grad);
      }
      */
    }
    break;
  }
  }
  surr_data.active_key(active_key); // restore

  // compute discrepancy faults from scratch (mostly mirrors HF failures but
  // might possibly add new ones for multiplicative FPE)
  surr_data.data_checks();
}


void C3Approximation::compute_all_sobol_indices(size_t interaction_order)
{
  C3FnTrainPtrs& ftp = levApproxIter->second;
  C3SobolSensitivity* fts = ftp.sobol();
  if (fts) c3_sobol_sensitivity_free(fts);
  fts = c3_sobol_sensitivity_calculate(ftp.function_train(), interaction_order);
  ftp.sobol(fts);
}


void C3Approximation::
compute_derived_statistics(C3FnTrainPtrs& ftp, size_t num_mom, bool overwrite)
{
  SharedC3ApproxData* data_rep = (SharedC3ApproxData*)sharedDataRep;
  if (overwrite) {
    ftp.ft_derived_functions_free();
    ftp.ft_derived_functions_create(data_rep->multiApproxOpts, num_mom,
				    data_rep->statsRoundingTol);
  }
  else if (ftp.derived_functions().allocated < num_mom) // incremental update
    ftp.ft_derived_functions_create(data_rep->multiApproxOpts, num_mom,
				    data_rep->statsRoundingTol);
}


void C3Approximation::
compute_derived_statistics_av(C3FnTrainPtrs& ftp, size_t num_mom,
			      bool overwrite)
{
  SharedC3ApproxData* data_rep = (SharedC3ApproxData*)sharedDataRep;
  // no incremental update implemented for allVars case
  if (overwrite || ftp.derived_functions().allocated < num_mom) {
    ftp.ft_derived_functions_free();
    ftp.ft_derived_functions_create_av(data_rep->multiApproxOpts,
				       data_rep->randomIndices,
				       data_rep->statsRoundingTol);
  }
}


void C3Approximation::compute_moments(bool full_stats, bool combined_stats)
{
  int len = (full_stats) ? 4 : 2;

  if (combined_stats) {
    if (combinedMoments.length() != len) combinedMoments.sizeUninitialized(len);
    combinedMoments[0] = combined_mean();
    combinedMoments[1] = combined_variance();
    if (full_stats) {// should not happen: combined exp is an intermediate state
      combinedMoments[2] = combined_third_central();
      combinedMoments[3] = combined_fourth_central();
    }
  }
  else {
    RealVector& primary_mom = primaryMomIter->second;
    if (primary_mom.length() != len) primary_mom.sizeUninitialized(len);
    primary_mom[0] = mean();  primary_mom[1] = variance();
    if (full_stats)
      { primary_mom[2] = third_central();  primary_mom[3] = fourth_central(); }
  }

  // Note: moment standardization occurs in NonDExpansion::print_moments()
}


void C3Approximation::
compute_moments(const RealVector& x, bool full_stats, bool combined_stats)
{
  if (combined_stats) {
    if (combinedMoments.length() != 2) combinedMoments.sizeUninitialized(2);
    combinedMoments[0] = combined_mean(x);
    combinedMoments[1] = combined_variance(x);
  }
  else {
    RealVector& primary_mom = primaryMomIter->second;
    if (primary_mom.length() != 2) primary_mom.sizeUninitialized(2);
    primary_mom[0] = mean(x);  primary_mom[1] = variance(x);
  }
}


Real C3Approximation::mean(C3FnTrainPtrs& ftp)//, size_t num_mom)
{
  compute_derived_statistics(ftp, 2);//num_mom); // if not already computed
  return ftp.derived_functions().first_moment;
}


Real C3Approximation::
mean(const RealVector &x, C3FnTrainPtrs& ftp)//, size_t num_mom)
{
  compute_derived_statistics_av(ftp, 2);//num_mom); // if not already computed
  return function_train_eval(ftp.derived_functions().ft_nonrand, x.values());
}


Real C3Approximation::variance(C3FnTrainPtrs& ftp)//, size_t num_mom)
{
  compute_derived_statistics(ftp, 2);//num_mom); // if not already computed
  return ftp.derived_functions().second_central_moment;
}


Real C3Approximation::
variance(const RealVector &x, C3FnTrainPtrs& ftp)//, size_t num_mom)
{
  compute_derived_statistics_av(ftp, 2);//num_mom); // if not already computed
  Real mu = mean(x, ftp);
  return function_train_eval(ftp.derived_functions().ft_squared_nonrand,
			     x.values()) - mu * mu;
}


Real C3Approximation::third_central(C3FnTrainPtrs& ftp)//, size_t num_mom)
{
  compute_derived_statistics(ftp, 4);//num_mom); // if not already computed
  return ftp.derived_functions().third_central_moment;
}


Real C3Approximation::fourth_central(C3FnTrainPtrs& ftp)//, size_t num_mom)
{
  compute_derived_statistics(ftp, 4);//num_mom); // if not already computed
  return ftp.derived_functions().fourth_central_moment;
}


Real C3Approximation::skewness(C3FnTrainPtrs& ftp)//, size_t num_mom)
{
  compute_derived_statistics(ftp, 4);//num_mom); // if not already computed
  return ftp.derived_functions().skewness;
}


Real C3Approximation::kurtosis(C3FnTrainPtrs& ftp)//, size_t num_mom)
{
  compute_derived_statistics(ftp, 4);//num_mom); // if not already computed
  return ftp.derived_functions().excess_kurtosis;
}


const RealVector& C3Approximation::mean_gradient()
{
  Cerr << "Error: mean_gradient() in C3Approximation is not implemented "
       << "because\n       Alex is not sure what it means" << std::endl;
  abort_handler(APPROX_ERROR);
}


const RealVector& C3Approximation::
mean_gradient(const RealVector &x, const SizetArray & dvv)
{
  Cerr << "Error: mean_gradient(x,dvv) in C3Approximation is not implemented "
       << "because\n       Alex is not sure what it means" << std::endl;
  abort_handler(APPROX_ERROR);        
}


const RealVector& C3Approximation::variance_gradient()
{
  // Gradient of Variance with respect to design vars *NOT VARIANCE OF GRADIENT*
  // I need to manage low-rank representations of the gradient, then evaluate
  // the variance of each of the partial derivatives
  Cerr << "Error: mean_gradient() in C3Approximation is not implemented "
       << "because\n       Alex is not sure what it means" << std::endl;
  abort_handler(APPROX_ERROR);
}


const RealVector & C3Approximation::
variance_gradient(const RealVector &x,const SizetArray & dvv)
{
  // size of output is size of dvv, only take gradients with respect to dvv fix
  // design and epistemic at x
  Cerr << "Error: mean_gradient(x,dvv) in C3Approximation is not implemented "
       << "because\n       Alex is not sure what it means" << std::endl;
  abort_handler(APPROX_ERROR);        
}


Real C3Approximation::covariance(C3FnTrainPtrs& ftp1, C3FnTrainPtrs& ftp2)
{
  Real mean1 = mean(ftp1), mean2 = mean(ftp2);

  // Sanity check:
  //Real alt_cov = function_train_inner_weighted(ftp1.function_train(),
  //  ftp2.function_train()) - mean1 * mean2;
  //Cout << "Alt covariance = " << alt_cov << std::endl;

  SharedC3ApproxData* data_rep = (SharedC3ApproxData*)sharedDataRep;
  struct MultiApproxOpts * opts = data_rep->multiApproxOpts;
  struct FunctionTrain * ft_tmp1
    = C3FnTrainPtrsRep::subtract_const(ftp1.function_train(), mean1, opts);
  struct FunctionTrain * ft_tmp2
    = C3FnTrainPtrsRep::subtract_const(ftp2.function_train(), mean2, opts);

  // No need to form product FT expansion and round result
  Real cov = function_train_inner_weighted(ft_tmp1, ft_tmp2);

  function_train_free(ft_tmp1); //ft_tmp1 = NULL;
  function_train_free(ft_tmp2); //ft_tmp2 = NULL;

  return cov;
}


Real C3Approximation::
covariance(const RealVector& x, C3FnTrainPtrs& ftp1, C3FnTrainPtrs& ftp2)
{
  Cerr << "Error: C3Approximation::covariance(x, ftp1, ftp2) in  is not "
       << "implemented because\n       Alex is not sure what it means"
       << std::endl;
  abort_handler(APPROX_ERROR);

  Real mean1 = mean(x, ftp1), mean2 = mean(x, ftp2);
  // ...
}


int C3Approximation::min_coefficients() const
{
  // min number of samples required to build the network is equal to
  // the number of design variables + 1

  // Note: Often this is too few samples.  It is better to have about
  // O(n^2) samples, where 'n' is the number of variables.

  return sharedDataRep->numVars + 1;
}


void C3Approximation::check_function_gradient()
{
  C3FnTrainPtrs& ftp = levApproxIter->second;
  if (ftp.ft_gradient() == NULL) {
    struct FunctionTrain * ft = ftp.function_train();
    if (ft == NULL) {
      Cerr << "Error: function train required in C3Approximation::"
	   << "check_function_gradient()" << std::endl;
      abort_handler(APPROX_ERROR);
    }
    else
      ftp.ft_gradient(function_train_gradient(ft)); // differentiate ft
  }

  if (approxGradient.empty())
    approxGradient.sizeUninitialized(sharedDataRep->numVars);
}


void C3Approximation::check_function_hessian()
{
  C3FnTrainPtrs& ftp = levApproxIter->second;
  if (ftp.ft_hessian() == NULL) {
    check_function_gradient(); // allocate ftg if needed
    struct FT1DArray * ftg = ftp.ft_gradient();
    ftp.ft_hessian(ft1d_array_jacobian(ftg)); // differentiate ftg
  }

  if (approxHessian.empty())
    approxHessian.shapeUninitialized(sharedDataRep->numVars);
}


// ignore discrete variables for now
Real C3Approximation::value(const Variables& vars)
{
  return function_train_eval(levApproxIter->second.function_train(),
			     vars.continuous_variables().values());
}


// These derivative routines correspond to *_basis_variables() cases
const RealVector& C3Approximation::gradient(const Variables& vars)
{
  check_function_gradient(); // compute on demand

  struct FT1DArray * ftg = levApproxIter->second.ft_gradient();
  const Real* c_vars = vars.continuous_variables().values();
  size_t i, num_v = sharedDataRep->numVars;
  for (i=0; i<num_v; ++i)
    approxGradient(i) = function_train_eval(ftg->ft[i], c_vars);
  return approxGradient;
}


const RealSymMatrix& C3Approximation::hessian(const Variables& vars)
{
  check_function_hessian(); // compute on demand

  struct FT1DArray * fth = levApproxIter->second.ft_hessian();
  const Real* c_vars = vars.continuous_variables().values();
  size_t i, j, num_v = sharedDataRep->numVars;
  for (i=0; i<num_v; ++i)
    for (j=0; j<=i; ++j)
      approxHessian(i,j) = function_train_eval(fth->ft[i+j*num_v], c_vars);
  return approxHessian;
}


// used for stored contributions to synthetic data:
Real C3Approximation::
stored_value(const RealVector& c_vars, const UShortArray& key)
{
  return function_train_eval(levelApprox[key].function_train(),
			     c_vars.values());
}


/* TO DO: synthetic data can utilize *_nonbasis_variables() derivative cases,
   but code below differentiates the FT w.r.t. the basis vars
const RealVector& C3Approximation::
gradient(const Variables& vars, const UShortArray& key)
{
  check_function_gradient(key); // compute on demand

  struct FT1DArray * ftg = levelApprox[key].ft_gradient();
  const Real* c_vars = vars.continuous_variables().values();
  size_t i, num_v = sharedDataRep->numVars;
  for (i=0; i<num_v; ++i)
    approxGradient(i) = function_train_eval(ftg->ft[i], c_vars);
  return approxGradient;
}
*/


size_t C3Approximation::regression_size()
{
  SharedC3ApproxData* data_rep = (SharedC3ApproxData*)sharedDataRep;

  // Reflects most recent FT build; omits any order increments prior to build:
  //return function_train_get_nparams(ftp.function_train());

  // Capture most recent rank adaptation (from build with adapt_rank), if any,
  // as well as any polynomial order increments (since last build)
  struct FunctionTrain * ft = levApproxIter->second.function_train();
  SizetVector ft_ranks(Teuchos::View, function_train_get_ranks(ft),
		       data_rep->numVars+1);
  return regression_size(ft_ranks,                 data_rep->max_rank(),
			 data_rep->start_orders(), data_rep->max_order());
}


/** compute the regression size (number of unknowns) for ranks per
    dimension and (polynomial) orders per dimension */
size_t C3Approximation::
regression_size(const SizetVector& ranks,  size_t max_rank,
		const UShortArray& orders, unsigned short max_order)
{
  // Each dimension has its own rank within the product of function cores.
  // This fn estimates for the case where rank varies per dimension/core
  // and basis order is constant.  Using 1-based indexing:
  // > first core is a 1 x r_1 row vector and contributes p_0   * r_1   terms
  // >  last core is a r_v x 1 col vector and contributes p_vm1 * r_vm1 terms
  // > middle v-2 cores are matrices that contribute r_i * r_{i+1} * p_i terms
  // > neighboring vec/mat dimensions must match, so there are v-1 unique ranks
  size_t num_v = sharedDataRep->numVars;
  if (ranks.length() != num_v + 1 || // both ends padded with 1's
      orders.size()  != num_v) {     // no padding
    Cerr << "Error: wrong ranks/orders array sizes in C3Approximation::"
	 << "regression_size()." << std::endl;
    abort_handler(APPROX_ERROR);
  }
  unsigned short p;
  switch (num_v) {
  case 1:
    p = std::min(orders[0], max_order) + 1;
    return p;  break; // collapses to 1D PCE
  default: { // first, last, and num_v-2 middle cores
    size_t core, vm1 = num_v - 1, sum;
    p = std::min(orders[0],   max_order) + 1;
    sum  = p * std::min(ranks[1],   max_rank); // first
    p = std::min(orders[vm1], max_order) + 1;
    sum += p * std::min(ranks[vm1], max_rank); // last
    for (core=1; core<vm1; ++core) {
      p = std::min(orders[core], max_order) + 1;
      sum += std::min(ranks[core],   max_rank)
	  *  std::min(ranks[core+1], max_rank) * p; // num_v-2 middle cores
    }
    return sum;  break;
  }
  }
}


//size_t C3Approximation::average_rank()
//{ return function_train_get_avgrank(levApproxIter->second.function_train()); }


//size_t C3Approximation::maximum_rank()
//{ return function_train_get_maxrank(levApproxIter->second.function_train()); }


Real C3Approximation::main_sobol_index(size_t dim)
{ return c3_sobol_sensitivity_get_main(levApproxIter->second.sobol(),dim); }


Real C3Approximation::total_sobol_index(size_t dim)
{ return c3_sobol_sensitivity_get_total(levApproxIter->second.sobol(),dim); }


void C3Approximation::
sobol_iterate_apply(void (*f)(double val, size_t ninteract,
			      size_t*interactions,void* arg), void* args)
{ c3_sobol_sensitivity_apply_external(levApproxIter->second.sobol(),f,args); }

} // namespace Dakota
