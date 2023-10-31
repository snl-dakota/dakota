/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "C3Approximation.hpp"
#include "C3FnTrainData.hpp"
#include "ProblemDescDB.hpp"
#include "SharedC3ApproxData.hpp"
#include "DiscrepancyCalculator.hpp"
#include "ActiveKey.hpp"

// NOTE: only include this header in isolated compilation units
#include "dakota_c3_include.hpp"

//#define DEBUG

namespace Dakota {


C3Approximation::
C3Approximation(ProblemDescDB& problem_db,
		const SharedApproxData& shared_data,
		const String& approx_label):
  Approximation(BaseConstructor(), problem_db, shared_data, approx_label),
  levApproxIter(levelApprox.end()), expansionCoeffFlag(true),
  expansionCoeffGradFlag(false)
{ } // FT memory allocations managed by C3FnTrainData



C3Approximation::C3Approximation(const SharedApproxData& shared_data):
  Approximation(NoDBBaseConstructor(), shared_data),
  levApproxIter(levelApprox.end()), expansionCoeffFlag(true),
  expansionCoeffGradFlag(false)
{ } // FT memory allocations managed by C3FnTrainData


C3Approximation::~C3Approximation()
{ } // FT memory deallocations managed by C3FnTrainData


void C3Approximation::recover_function_train_ranks(struct FunctionTrain * ft)
{
  // returns the recovered ranks, reflecting the latest CV if adapt_rank

  // Note: this recovery uses active levApproxIter and can be used anytime

  SizetVector ft_ranks(Teuchos::Copy, function_train_get_ranks(ft),
		       sharedDataRep->numVars + 1);

  levApproxIter->second.recovered_ranks(ft_ranks);
}


/** returns the recovered orders, reflecting the latest CV if adapt_order */
void C3Approximation::
recover_function_train_orders(const std::vector<OneApproxOpts*>& a_opts)
{
  // returns the recovered orders, reflecting the latest CV if adapt_order

  // Note: this recovery is not keyed and must transfer results from
  // one_approx_opts_get_nparams() to keyed C3FnTrainData::recoveredOrders
  // within build() immediately following the regression solve

  std::shared_ptr<SharedC3ApproxData> data_rep =
    std::static_pointer_cast<SharedC3ApproxData>(sharedDataRep);
  size_t v, num_v = data_rep->numVars;
  UShortArray ft_orders(num_v);
  for (v=0; v<num_v; ++v)
    ft_orders[v] = one_approx_opts_get_nparams(a_opts[v]) - 1;

  levApproxIter->second.recovered_orders(ft_orders);
}


bool C3Approximation::advancement_available()
{
  std::shared_ptr<SharedC3ApproxData> shared_data_rep =
    std::static_pointer_cast<SharedC3ApproxData>(sharedDataRep);

  bool m_r_advance = false, m_o_advance = false;
     //s_r_reduce  = false, s_o_reduce  = false;
  switch (shared_data_rep->c3AdvancementType) {

  // these options query recovered ranks/orders for each C3Approximation
  case MAX_RANK_ADVANCEMENT: // check adapted FT ranks against maxRank
    //s_r_reduce  = start_rank_reduction_available();
    m_r_advance = max_rank_advancement_available();  break;
  case MAX_ORDER_ADVANCEMENT:
    //s_o_reduce  = start_order_reduction_available();
    m_o_advance = max_order_advancement_available(); break;
  case MAX_RANK_ORDER_ADVANCEMENT:
    //s_r_reduce  =  start_rank_reduction_available();
    m_r_advance =  max_rank_advancement_available();
    //s_o_reduce  = start_order_reduction_available();
    m_o_advance = max_order_advancement_available(); break;
  }
  // Need available advancements in Shared increment_order() to advance only
  // the unsaturated bounds (CV remains active over both rank/order but only
  // one or both bounds may change).  Therefore, we accumulate the available
  // advancement types in SharedC3, communicated back from C3Approx instances
  // for use in {increment,decrement}_order().
  //if (s_r_reduce)  shared_data_rep->start_rank_reduction(s_r_reduce);
  if (m_r_advance) shared_data_rep->max_rank_advancement(m_r_advance);
  //if (s_o_reduce)  shared_data_rep->start_order_reduction(s_o_reduce);
  if (m_o_advance) shared_data_rep->max_order_advancement(m_o_advance);

  return (m_r_advance || m_o_advance);// || s_r_reduce || s_o_reduce);
}


/*
bool C3Approximation::start_rank_reduction_available()
{
  std::shared_ptr<SharedC3ApproxData> shared_data_rep =
    std::static_pointer_cast<SharedC3ApproxData>(sharedDataRep);
  size_t start_r  = shared_data_rep->start_rank(), // adapted value
         v, num_v = shared_data_rep->numVars;
  const SizetVector& ft_ranks = levApproxIter->second.recovered_ranks();
  for (v=1; v<num_v; ++v) // ranks len = num_v+1 with 1's @ ends
    if (ft_ranks[v] <= start_r) // recovery potentially limited by lower bound
      return true;
  return false;
}


bool C3Approximation::start_order_reduction_available()
{
  std::shared_ptr<SharedC3ApproxData> shared_data_rep =
    std::static_pointer_cast<SharedC3ApproxData>(sharedDataRep);
  unsigned short start_o = shared_data_rep->start_order(); // adapted value
  const UShortArray& ft_ords = levApproxIter->second.recovered_orders();
  size_t v, num_v = shared_data_rep->numVars;
  for (v=0; v<num_v; ++v) // ords len = num_v
    if (ft_ords[v] <= start_o) // recovery potentially limited by lower bound
      return true;
  return false;
}
*/


bool C3Approximation::max_rank_advancement_available()
{
  std::shared_ptr<SharedC3ApproxData> shared_data_rep =
    std::static_pointer_cast<SharedC3ApproxData>(sharedDataRep);
  size_t max_r = shared_data_rep->max_rank(), // adapted value
      v, num_v = shared_data_rep->numVars;
  const SizetVector& ft_ranks = levApproxIter->second.recovered_ranks();
  for (v=1; v<num_v; ++v) // ranks len = num_v+1 with 1's @ ends
    if (ft_ranks[v] >= max_r) // recovery potentially limited by upper bound
      return true;
  return false;
}


bool C3Approximation::max_order_advancement_available()
{
  std::shared_ptr<SharedC3ApproxData> shared_data_rep =
    std::static_pointer_cast<SharedC3ApproxData>(sharedDataRep);
  unsigned short max_o = shared_data_rep->max_order(); // adapted value
  const UShortArray& ft_ords = levApproxIter->second.recovered_orders();
  size_t v, num_v = shared_data_rep->numVars;
  for (v=0; v<num_v; ++v) // ords len = num_v
    if (ft_ords[v] >= max_o) // recovery potentially limited by upper bound
      return true;
  return false;
}


void C3Approximation::build()
{
  if (sharedDataRep->outputLevel >= DEBUG_OUTPUT)
    Cout << "Calling C3Approximation::build()\n";

  // compute modifications to surrogate data, if any
  synchronize_surrogate_data();

  // base class implementation checks data set against min required
  Approximation::build();

  std::shared_ptr<SharedC3ApproxData> data_rep =
    std::static_pointer_cast<SharedC3ApproxData>(sharedDataRep);
  //if (data_rep->adaptConstruct) {
  //  Cerr << "Error: Adaptive construction not yet implemented in "
  //	   << "C3Approximation." << std::endl;
  //  abort_handler(APPROX_ERROR);
  //}

  // -----------------------
  // Set up C3 FT regression
  // -----------------------

  size_t i, j, num_v = sharedDataRep->numVars, kick_r, max_cv_r,
    max_r = data_rep->max_rank(), // upper bound for adapt_rank
    start_r = std::min(data_rep->start_rank(), max_r),
    adapt_r = (data_rep->adaptRank && max_r > start_r) ? 1 : 0;
  unsigned short max_o = data_rep->max_order(), kick_o, max_cv_o,
    start_o = find_max(data_rep->start_orders()); // flatten dim_pref
  bool adapt_o = (data_rep->adaptOrder && max_o > start_o);
  SizetVector start_ranks(num_v+1);  start_ranks[0] = start_ranks[num_v] = 1;
  UShortArray start_orders(num_v);
  C3FnTrainData& ftd = levApproxIter->second;

  // This simple approach uses an upper bound on the number of candidates per
  // CV dimension, rather than adapting the range in both directions (it does
  // not adapt when the range based on whether the best CV
  // solution is active on its lower bound.  We also check recovered values:
  // while at least one QoI must be active on the upper bound to prevent a
  // saturation condition; other QoI may be lower and we don't want to exclude
  // the previous solution for any of them.  In this case, the max rank/order
  // is shared but the start rank/order is managed per QoI.

  if (adapt_r) {
    kick_r   = data_rep->kickRank;
    max_cv_r = data_rep->max_cross_validation_rank_candidates();
    if (max_r == SZ_MAX) // default upper bound -> define appropriate range
      max_r = (max_cv_r == SZ_MAX) ? start_r + 4 * kick_r : // 5 candidates
	start_r + (max_cv_r - 1) * kick_r;
    else { // pull up start rank based on max number of CV candidates
      // use signed operands to avoid type coercion error in comparing unsigned
      int start_cand = max_r, start_lb = start_r;  i = 0;
      while ( start_cand > start_lb && i < max_cv_r )
	{ start_cand -= kick_r;  ++i; }
      // couple of options for dealing with possible overshoot due to kick > 1:
      // > remove overshot candidate (respect max over start)
      // > repair overshot candidate to start (include start/max, violate kick)
      // > shift to respect start over max (chosen option)
      if (start_cand > start_lb) start_r = start_cand;
      // take care to not prune access to a previouly recovered solution
      const SizetVector& recov_ranks = ftd.recovered_ranks();
      size_t min_recov_r = (recov_ranks.empty()) ? SZ_MAX :
	find_min(&recov_ranks[1], num_v-1); // skip 1's @ first,last
      if (start_r > min_recov_r) start_r = min_recov_r; // include prev recovery
    }
  }
  for (i=1; i<num_v; ++i)
    start_ranks[i] = start_r;

  if (adapt_o) {
    kick_o   = data_rep->kickOrder;
    max_cv_o = data_rep->max_cross_validation_order_candidates();
    if (max_o == USHRT_MAX) // default upper bound -> define appropriate range
      max_o = (max_cv_o == SZ_MAX) ? start_o + 4 * kick_o : // 5 candidates
	start_o + (max_cv_o - 1) * kick_o;
    else { // pull up start order based on max number of CV candidates
      // use signed operands to avoid type coercion error in comparing unsigned
      int start_cand = max_o, start_lb = start_o;  i = 0;
      while ( start_cand > start_lb && i < max_cv_o )
	{ start_cand -= kick_o;  ++i; }
      // couple of options for dealing with possible overshoot due to kick > 1:
      // > remove overshot candidate (respect max over start)
      // > repair overshot candidate to start (include start/max, violate kick)
      // > shift to respect start over max (chosen option)
      if (start_cand > start_lb) start_o = start_cand;
      // take care to not prune access to a previouly recovered solution
      unsigned short min_recov_o = find_min(ftd.recovered_orders());
      if (start_o > min_recov_o) start_o = min_recov_o; // include prev recovery
    }
  }
  for (i=0; i<num_v; ++i)
    start_orders[i] = start_o;

  // opts are shared data since poly type, bounds, etc. are invariant,
  // but a subset of this data (nparams, maxnum) vary per QoI
  // > retain as shared data but update approx opts on every build()
  // > recovered QoI rank data is stored in C3FnTrainData
  data_rep->update_basis(start_orders, max_o);
  // reassigns active orders from SharedC3 start,max maps; does not start
  // from a previous adapt_order recovery
  //data_rep->update_basis();

  struct FTRegress * ftr
    = ft_regress_alloc(num_v, data_rep->multiApproxOpts, start_ranks.values());

  if (data_rep->regressType == FT_RLS2) {
    ft_regress_set_alg_and_obj(ftr, AIO, FTLS_SPARSEL2);
    // reg param is required (no reasonable default due to scaling)
    ft_regress_set_regularization_weight(ftr, data_rep->regressRegParam);
  }
  else // default
    ft_regress_set_alg_and_obj(ftr, AIO, FTLS);

  // this should precede any solves, including the cross-validation loop(s)
  ft_regress_set_seed(ftr, data_rep->randomSeed);

  ft_regress_set_adapt(ftr, adapt_r);
  if (adapt_r) {
    ft_regress_set_kickrank(ftr, kick_r); // default is 1

    // if not user-specified, use internal C3 default (in src/lib_superlearn/
    // regress.c, maxrank = 10 assigned in ft_regress_alloc())
    // > default could become an issue for START_RANK_ADVANCEMENT
    if (max_r != SZ_MAX)
      ft_regress_set_maxrank(ftr, max_r);

    ft_regress_set_kfold(ftr, 5);//kfold);//match Alex's Python (C3 default=3)
  }
  ft_regress_set_roundtol(ftr, data_rep->solverRoundingTol);
  short output_lev = data_rep->outputLevel;
  if (output_lev > NORMAL_OUTPUT)
    ft_regress_set_verbose(ftr, 1); // helpful adapt_rank diagnostics

  // -------------------
  // Set up C3 optimizer
  // -------------------

  struct c3Opt* optimizer = c3opt_create(BFGS);
  int max_solver_iter = data_rep->maxSolverIterations;
  if (max_solver_iter != SZ_MAX) { // if Dakota default, leave at C3 default
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
  ftd.free_all();

  // ---------------------
  // Pack FT training data
  // ---------------------

  size_t ndata = approxData.points();

  // Manage scaling in a localized manner for now: scale the data here and then
  // unscale the FT at bottom.  Scaling is important for FT regression with
  // absolute tolerances, especially for small ML/MF discrepancy expansions.
  if (data_rep->respScaling) approxData.compute_response_function_scaling();
  else                       approxData.clear_response_function_scaling();
  bool apply_scaling       = approxData.valid_response_scaling();

  // Training data for 1 QoI: transfer data from approxData to double* for C3
  double* xtrain = (double*)calloc(num_v*ndata, sizeof(double)); // vars
  double* ytrain = (double*)calloc(ndata,       sizeof(double)); // QoI
  for (i=0; i<ndata; ++i) {
    const RealVector& c_vars = approxData.continuous_variables(i);
    for (j=0; j<num_v; j++)
      xtrain[j + i*num_v] = c_vars[j];
    ytrain[i] = (apply_scaling) ? approxData.scaled_response_function(i) :
      approxData.response_function(i);
  }
#ifdef DEBUG
  RealMatrix  in(Teuchos::View, xtrain, num_v, num_v, ndata);
  RealVector out(Teuchos::View, ytrain, ndata);
  Cout << "C3 training data:\n" << in << out << std::endl;
#endif // DEBUG

  // ---------------------------------------
  // Configure outer loop CV for basis order
  // ---------------------------------------
  // > separate from adapt_rank inner loop: one/both/neither can be used

  if (adapt_o) { // CV for basis orders, with/without adapt_rank

    // initialize cross validation options
    size_t kfold = (ndata > 5) ? 5 : ndata; // ndata = max folds = leave-one-out
    int cvverbose = (output_lev >= DEBUG_OUTPUT) ? 1 : 0;  // verbosity
    struct CrossValidate * cv
      = cross_validate_init(ndata, num_v, xtrain, ytrain, kfold, cvverbose);

    // example of a cross validation run with extractor error
    // (don't need this if running over grid)
    //double err = cross_validate_run(cv,ftr,optimizer);

    // Set up a grid over which to search for the best FT parameters
    struct CVOptGrid * cvgrid = cv_opt_grid_init(1); // allocate a 1D grid
    cv_opt_grid_set_verbose(cvgrid, cvverbose);
    // define a scalar range of (isotropic) basis orders
    size_t num_opts = 1 + (max_o - start_o) / kick_o;
    SizetArray np_opts(num_opts);
    np_opts[0] = start_o + 1; // offset by 1 for nparams
    for (i=1; i<num_opts; ++i)
      np_opts[i] = np_opts[i-1] + kick_o;
    // cross validate over "num_param", choose from options given in np_opts
    String cv_target("num_param");
    cv_opt_grid_add_param(cvgrid, &cv_target[0], num_opts, &np_opts[0]); 

    // run cross validation with ftr setup
    cross_validate_grid_opt(cv, cvgrid, ftr, optimizer);
    // free the cross validation grid and cross validator
    cv_opt_grid_free(cvgrid); cross_validate_free(cv);
  }

  // -------------------------------------------
  // Solve the regression problem to form the FT
  // -------------------------------------------
  // If CV grid is active above for adapt_order or adapt_{rank,order}, this
  // regress_run uses the full data set with the best CV config found.  Note:
  // if adaptive rank selection is active within the order-based CV grid, we
  // perform this selection again here since (1) the data partition for rank
  // CV is expanded from before (no order-based CV fold) and (2) rank-based
  // selection involves more fine-grained adaptation (per variable).
  struct FunctionTrain * ft_soln
    = ft_regress_run(ftr, optimizer, ndata, xtrain, ytrain);
  // cache C3FTD recovered{Ranks,Orders} prior to any transformation (unscaling)
  std::vector<OneApproxOpts*>& a_opts = data_rep->oneApproxOpts;
  if (data_rep->adaptRank)  recover_function_train_ranks(ft_soln);
  if (data_rep->adaptOrder) recover_function_train_orders(a_opts);
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
  //  struct FT1DArray * ftg_soln = function_train_gradient(ft_soln);
  //  ftd.ft_gradient(ftg_soln);
  //  ftd.ft_hessian(ft1d_array_jacobian(ftg_soln));
  //}
  if (data_rep->outputLevel >= QUIET_OUTPUT) {
    Cout << "\nFunction train build() results:\n  Ranks ";
    if (adapt_r)
      Cout << "(adapted with start = " << start_r << " kick = " << kick_r
	   << " max = " << max_r << "):\n";
    else Cout << "(non-adapted):\n";
    write_data(Cout, function_train_get_ranks(ft_soln), num_v+1);
    Cout << "  Polynomial order ";
    if (adapt_o)
      Cout << "(adapted with start = " << start_o << " kick = " << kick_o
	   << " max = " << max_o << "):\n";
    else Cout << "(non-adapted):\n";
    for (i=0; i<num_v; ++i)
      Cout << "                     " << std::setw(write_precision+7)
	   << one_approx_opts_get_nparams(a_opts[i]) - 1 << '\n';
    Cout << "  C3 regression size:  " << function_train_get_nparams(ft_soln);
    if (apply_scaling) Cout << "  Response scaling on";
    Cout << std::endl;
  }

  // Note: could set c3Max{Rank,Order}Advance after current build instead of
  // preceding next build (using cached ranks,orders) if advancement_available()
  // was the only concern.  However, regression_size() is another requirement
  // prior to next build, so just cache the relevant build data rather than a
  // potentially growing list of inferences from that data.
  //if (shared_data_rep->c3AdvancementType & MAX_RANK_ADVANCEMENT) {
  //  if (test_ranks_without_requiring_cached_data)
  //    shared_data_rep->max_rank_advancement(true);
  //}
  //if (shared_data_rep->c3AdvancementType & MAX_ORDER_ADVANCEMENT) {
  //  if (test_orders_without_requiring_cached_data)
  //    shared_data_rep->max_order_advancement(true);
  //}

  // NOTE: function_train_sum will increment the rank by 1 which can interfere
  // with the adaptation logic, but since the [start,max] range is reinitialized
  // each time, this may be Ok as a final step (roll-ups are not impacted, but
  // posthumous rank checks would be).
  if (apply_scaling) { // unscale the FT expansion: fn = y * range + min
    // emulate c3axpy with epsilon=0 (no rounding applied for scale + constant)
    const RealRealPair& factors = approxData.response_function_scaling();
    function_train_scale(ft_soln, factors.second); // scaling applied in place
    struct FunctionTrain * ft_offset
      = function_train_constant(factors.first, data_rep->multiApproxOpts);
    // this operation will increase the FT rank by 1:
    struct FunctionTrain * sum_ft = function_train_sum(ft_soln, ft_offset);
    ftd.function_train(sum_ft); // store result
    function_train_free(ft_soln);  function_train_free(ft_offset);
  }
  else
    ftd.function_train(ft_soln); // store result

  // store the current scaling values
  //ftd.scaling_factors(range, min_fn); // multiplicative,additive factors

  // free approximation stuff
  free(xtrain);          xtrain    = NULL;
  free(ytrain);          ytrain    = NULL;
  ft_regress_free(ftr);  ftr       = NULL;
  c3opt_free(optimizer); optimizer = NULL;
}


void C3Approximation::rebuild()
{
  active_model_key(sharedDataRep->activeKey);

  // for use in pop_coefficients()
  prevC3FTData = levApproxIter->second.copy(); // deep copy

  build(); // updates levApproxIter->second
}


void C3Approximation::pop_coefficients(bool save_data)
{
  const Pecos::ActiveKey& key = sharedDataRep->activeKey;

  // likely overkill, but multilevel roll up after increment modifies and
  // then restores active key
  active_model_key(key);

  C3FnTrainData& active_ftd = levApproxIter->second;
  // store the incremented coeff state for possible push
  if (save_data) // shallow copy enabled by swap to follow
    poppedLevelApprox[key].push_back(active_ftd);
  // reset expansion to previous state.  After swap of reps / ref counts,
  // poppedLevelApprox[key].back() shares Rep with prevC3FTData.  prevC3FTData
  // then gets replaced with a new instance in rebuild or push_coefficients().
  active_ftd.swap(prevC3FTData);

  //clear_computed_bits();
}


void C3Approximation::push_coefficients()
{
  std::shared_ptr<SharedC3ApproxData> data_rep =
    std::static_pointer_cast<SharedC3ApproxData>(sharedDataRep);
  const Pecos::ActiveKey& key = data_rep->activeKey;

  // synchronize expansionCoeff{s,Grads} and approxData
  active_model_key(key);

  // store current state for use in pop_coefficients()
  C3FnTrainData& active_ftd = levApproxIter->second;
  prevC3FTData = active_ftd.copy(); // deep copy

  // retrieve a previously popped state
  std::map<Pecos::ActiveKey, std::deque<C3FnTrainData> >::iterator prv_it
    = poppedLevelApprox.find(key);
  bool err_flag = false;
  if (prv_it == poppedLevelApprox.end())
    err_flag = true;
  else {
    // SharedPolyApproxData::candidate_index() currently returns 0 for
    // all cases other than generalized sparse grids
    size_t p_index = data_rep->push_index(key); // *** TO DO
    std::deque<C3FnTrainData>& ftd_deque = prv_it->second;
    if (p_index >= ftd_deque.size())
      err_flag = true;
    else {
      std::deque<C3FnTrainData>::iterator rv_it	= ftd_deque.begin() + p_index;
      // reset expansion to popped state.  Shallow copy is appropriate
      // (levApprox assumes popped state prior to erase from bookkeeping).
      active_ftd = *rv_it;    // shallow copy of popped state
      ftd_deque.erase(rv_it); // removal of original
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
  combinedC3FTData.free_ft();
  std::map<Pecos::ActiveKey, C3FnTrainData>::iterator it = levelApprox.begin();
  struct FunctionTrain * y = function_train_copy(it->second.function_train());
  ++it;
  // Note: the FT rounding tolerance is relative and default (1.e-8) is too
  // tight for this context --> use arithmetic tol.  Memory overhead is strongly
  // correlated with this tolerance and 1.e-3 did not result in significant
  // accuracy gain in some numerical experiments (dakota_uq_heat_eq_mlft.in).
  std::shared_ptr<SharedC3ApproxData> data_rep =
    std::static_pointer_cast<SharedC3ApproxData>(sharedDataRep);
  Real rnd_tol = data_rep->statsRoundingTol;
  struct MultiApproxOpts* ma_opts = data_rep->multiApproxOpts;
  struct FunctionTrain *z;
  for (; it!= levelApprox.end(); ++it) {
    //c3axpy(1., it->second.function_train(), &y, arith_tol, ma_opts);

    // Streamline c3axpy steps for this use case (avoid scale + its inner copy)
    z = function_train_sum(it->second.function_train(), y);
    function_train_free(y);
    if (rnd_tol > 0.) // round scales as rank^3 so do each pass, not at end
      { y = function_train_round(z, rnd_tol, ma_opts); function_train_free(z); }
    else
      y = z;
  }
  combinedC3FTData.function_train(y);

  // Could also do this at the C3FnTrainData level with ft1d_array support:
  //combinedC3FTData = it->second.copy(); ++it;
  //for (; it!= levelApprox.end(); ++it)
  //  sum ft,ft_gradient,ft_hessian...

  // Option 2: function_train_sum (I allocate a and b and C3 allocates c)
  // > remember to deallocate c when done
  //struct FunctionTrain* c = function_train_sum(a, b);

  // This replaces fine-grained moment bit trackers with a single override
  const SizetArray& rand_ind = data_rep->randomIndices;
  size_t num_mom = combinedMoments.length();
  if (rand_ind.empty() || rand_ind.size() == data_rep->numVars)
    compute_derived_statistics(   combinedC3FTData, num_mom, true);// overwrite
  else
    compute_derived_statistics_av(combinedC3FTData, num_mom, true);// overwrite
}


void C3Approximation::combined_to_active_coefficients(bool clear_combined)
{
  active_model_key(sharedDataRep->activeKey);

  levApproxIter->second = combinedC3FTData;//.copy();
  //if (clear_combined)
  //  combinedC3FTData.free_all();

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
  std::map<Pecos::ActiveKey, C3FnTrainData>::iterator it = levelApprox.begin();
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
  // > SurrogateModel::aggregate_response() uses order of low to high fidelity
  // > ApproximationInterface::{mixed,shallow}_add() assigns aggregate response
  //   data to each approxData instance in turn.

  std::shared_ptr<SharedC3ApproxData> data_rep =
    std::static_pointer_cast<SharedC3ApproxData>(sharedDataRep);
  switch (data_rep->discrepReduction) {
  case Pecos::DISTINCT_DISCREPANCY:  case Pecos::RECURSIVE_DISCREPANCY: {
    // push another SurrogateData instance for modSurrData
    // (allows consolidation of Approximation::push/pop operations)
    const Pecos::ActiveKey& key = approxData.back().active_key();
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
  std::shared_ptr<SharedC3ApproxData> data_rep =
    std::static_pointer_cast<SharedC3ApproxData>(sharedDataRep);
  const Pecos::ActiveKey& active_key = data_rep->activeKey;
  if (active_key != approxData.active_key()) {
    PCerr << "Error: active key mismatch in C3Approximation::"
	  << "synchronize_surrogate_data()." << std::endl;
    abort_handler(-1);
  }

  // level 0: approxData non-aggregated key stores raw data
  short discrep_type = data_rep->discrepReduction,
        combine_type = data_rep->combineType;
  if (!discrep_type || !active_key.aggregated() ||
      !active_key.raw_with_reduction_data())
    return;

  switch (discrep_type) {
  case Pecos::RECURSIVE_DISCREPANCY:
    // When using a recursive discrepancy with additive/multiplicative corr,
    // we will subtract/divide the current polynomial approx prediction from
    // the new surrData so that we form an expansion on the surplus.  Prior
    // to using compute() to form the surplus, LF-hat must be generated and
    // will be stored within surrData in a format that compute() can utilize.
    generate_synthetic_data(approxData, active_key, combine_type);
    break;
  //case Pecos::DISTINCT_DISCREPANCY:
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
			const Pecos::ActiveKey& active_key, short combine_type)
{
  // Modeled after Pecos::PolynomialApproximation::generate_synthetic_data()

  Pecos::ActiveKey hf_key, lf0_key, lf_hat_key; // LF-hat in surplus case
  active_key.extract_keys(lf_hat_key, hf_key);
  lf0_key = surr_data.filtered_key(Pecos::SINGLETON_FILTER, 0); // *** Note: ActiveKey first sorts on group id

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
  const std::map<Pecos::ActiveKey, Pecos::SDRArray>& discrep_resp_map =
    surr_data.filtered_response_data_map(Pecos::RAW_WITH_REDUCTION_DATA_FILTER);
  std::map<Pecos::ActiveKey, Pecos::SDRArray>::const_iterator cit;
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
  C3FnTrainData& ftd = levApproxIter->second;
  C3SobolSensitivity* fts = ftd.sobol();
  if (fts) c3_sobol_sensitivity_free(fts);
  fts = c3_sobol_sensitivity_calculate(ftd.function_train(), interaction_order);
  ftd.sobol(fts);
}


void C3Approximation::
compute_derived_statistics(C3FnTrainData& ftd, size_t num_mom, bool overwrite)
{
  std::shared_ptr<SharedC3ApproxData> data_rep =
    std::static_pointer_cast<SharedC3ApproxData>(sharedDataRep);
  if (overwrite) {
    ftd.ft_derived_functions_free();
    ftd.ft_derived_functions_create(data_rep->multiApproxOpts, num_mom,
				    data_rep->statsRoundingTol);
  }
  else if (ftd.derived_functions().allocated < num_mom) // incremental update
    ftd.ft_derived_functions_create(data_rep->multiApproxOpts, num_mom,
				    data_rep->statsRoundingTol);
}


void C3Approximation::
compute_derived_statistics_av(C3FnTrainData& ftd, size_t num_mom,
			      bool overwrite)
{
  std::shared_ptr<SharedC3ApproxData> data_rep =
    std::static_pointer_cast<SharedC3ApproxData>(sharedDataRep);
  // no incremental update implemented for allVars case
  if (overwrite || ftd.derived_functions().allocated < num_mom) {
    ftd.ft_derived_functions_free();
    ftd.ft_derived_functions_create_av(data_rep->multiApproxOpts,
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
    RealVector& primary_mom = levApproxIter->second.moments();
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
    RealVector& primary_mom = levApproxIter->second.moments();
    if (primary_mom.length() != 2) primary_mom.sizeUninitialized(2);
    primary_mom[0] = mean(x);  primary_mom[1] = variance(x);
  }
}


Real C3Approximation::mean(C3FnTrainData& ftd)//, size_t num_mom)
{
  compute_derived_statistics(ftd, 2);//num_mom); // if not already computed
  return ftd.derived_functions().first_moment;
}


Real C3Approximation::
mean(const RealVector &x, C3FnTrainData& ftd)//, size_t num_mom)
{
  compute_derived_statistics_av(ftd, 2);//num_mom); // if not already computed
  return function_train_eval(ftd.derived_functions().ft_nonrand, x.values());
}


Real C3Approximation::variance(C3FnTrainData& ftd)//, size_t num_mom)
{
  compute_derived_statistics(ftd, 2);//num_mom); // if not already computed
  return ftd.derived_functions().second_central_moment;
}


Real C3Approximation::
variance(const RealVector &x, C3FnTrainData& ftd)//, size_t num_mom)
{
  compute_derived_statistics_av(ftd, 2);//num_mom); // if not already computed
  Real mu = mean(x, ftd);
  return function_train_eval(ftd.derived_functions().ft_squared_nonrand,
			     x.values()) - mu * mu;
}


Real C3Approximation::third_central(C3FnTrainData& ftd)//, size_t num_mom)
{
  compute_derived_statistics(ftd, 4);//num_mom); // if not already computed
  return ftd.derived_functions().third_central_moment;
}


Real C3Approximation::fourth_central(C3FnTrainData& ftd)//, size_t num_mom)
{
  compute_derived_statistics(ftd, 4);//num_mom); // if not already computed
  return ftd.derived_functions().fourth_central_moment;
}


Real C3Approximation::skewness(C3FnTrainData& ftd)//, size_t num_mom)
{
  compute_derived_statistics(ftd, 4);//num_mom); // if not already computed
  return ftd.derived_functions().skewness;
}


Real C3Approximation::kurtosis(C3FnTrainData& ftd)//, size_t num_mom)
{
  compute_derived_statistics(ftd, 4);//num_mom); // if not already computed
  return ftd.derived_functions().excess_kurtosis;
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


Real C3Approximation::covariance(C3FnTrainData& ftd1, C3FnTrainData& ftd2)
{
  Real mean1 = mean(ftd1), mean2 = mean(ftd2);

  // Sanity check:
  //Real alt_cov = function_train_inner_weighted(ftd1.function_train(),
  //  ftd2.function_train()) - mean1 * mean2;
  //Cout << "Alt covariance = " << alt_cov << std::endl;

  std::shared_ptr<SharedC3ApproxData> data_rep =
    std::static_pointer_cast<SharedC3ApproxData>(sharedDataRep);
  struct FunctionTrain * ft_tmp1 = C3FnTrainDataRep::subtract_const(
    ftd1.function_train(), mean1, data_rep->multiApproxOpts);
  struct FunctionTrain * ft_tmp2 = C3FnTrainDataRep::subtract_const(
    ftd2.function_train(), mean2, data_rep->multiApproxOpts);

  // No need to form product FT expansion and round result
  Real cov = function_train_inner_weighted(ft_tmp1, ft_tmp2);

  function_train_free(ft_tmp1); //ft_tmp1 = NULL;
  function_train_free(ft_tmp2); //ft_tmp2 = NULL;

  return cov;
}


Real C3Approximation::
covariance(const RealVector& x, C3FnTrainData& ftd1, C3FnTrainData& ftd2)
{
  Cerr << "Error: C3Approximation::covariance(x, ftd1, ftd2) in  is not "
       << "implemented because\n       Alex is not sure what it means"
       << std::endl;
  abort_handler(APPROX_ERROR);

  Real mean1 = mean(x, ftd1), mean2 = mean(x, ftd2);
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
  C3FnTrainData& ftd = levApproxIter->second;
  if (ftd.ft_gradient() == NULL) {
    struct FunctionTrain * ft = ftd.function_train();
    if (ft == NULL) {
      Cerr << "Error: function train required in C3Approximation::"
	   << "check_function_gradient()" << std::endl;
      abort_handler(APPROX_ERROR);
    }
    else
      ftd.ft_gradient(function_train_gradient(ft)); // differentiate ft
  }

  if (approxGradient.empty())
    approxGradient.sizeUninitialized(sharedDataRep->numVars);
}


void C3Approximation::check_function_hessian()
{
  C3FnTrainData& ftd = levApproxIter->second;
  if (ftd.ft_hessian() == NULL) {
    check_function_gradient(); // allocate ftg if needed
    struct FT1DArray * ftg = ftd.ft_gradient();
    ftd.ft_hessian(ft1d_array_jacobian(ftg)); // differentiate ftg
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
stored_value(const RealVector& c_vars, const Pecos::ActiveKey& key)
{
  return function_train_eval(levelApprox[key].function_train(),
			     c_vars.values());
}


/* TO DO: synthetic data can utilize *_nonbasis_variables() derivative cases,
   but code below differentiates the FT w.r.t. the basis vars
const RealVector& C3Approximation::
gradient(const Variables& vars, const Pecos::ActiveKey& key)
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
  std::shared_ptr<SharedC3ApproxData> data_rep =
    std::static_pointer_cast<SharedC3ApproxData>(sharedDataRep);

  // Intent: capture latest ranks/orders recovered from most recent FT build
  //   or from rank/order advancements.
  // Usage:  this function is only currently used from NonDC3FunctionTrain::
  //   sample_allocation_metric() for non-MAX uniform refinements.
  // For START_*_ADVANCEMENT + adapt_*: increments for the former may overwrite
  //   recovery for the latter; in this case, we use the latest incremented/
  //   decremented state from sharedDataRep.

  size_t         max_r = data_rep->max_rank();
  unsigned short max_o = data_rep->max_order();
  switch (data_rep->c3AdvancementType) {
  case START_RANK_ADVANCEMENT: { // use start ranks + recovered orders
    SizetVector start_r;  data_rep->assign_start_ranks(start_r);
    const UShortArray& ft_o = (data_rep->adaptOrder) ?
      levApproxIter->second.recovered_orders() : data_rep->start_orders();
    return regression_size(start_r, max_r, ft_o, max_o);  break;
  }
  case START_ORDER_ADVANCEMENT: { // use start orders + recovered ranks
    const UShortArray& start_o = data_rep->start_orders(); // anisotropic is OK
    if (data_rep->adaptRank)
      return regression_size(levApproxIter->second.recovered_ranks(),
			     max_r, start_o, max_o);
    else {
      SizetVector start_r;  data_rep->assign_start_ranks(start_r);
      return regression_size(start_r, max_r, start_o, max_o);
    }
    break;
  }
  default: { // use recovered orders + recovered ranks
    const UShortArray& ft_o = (data_rep->adaptOrder) ?
      levApproxIter->second.recovered_orders() : data_rep->start_orders();
    if (data_rep->adaptRank)
      return regression_size(levApproxIter->second.recovered_ranks(),
			     max_r, ft_o, max_o);
    else {
      SizetVector start_r;  data_rep->assign_start_ranks(start_r);
      return regression_size(start_r, max_r, ft_o, max_o);
    }
    break;
  }
  }
}


/** compute the regression size (number of unknowns) for ranks per
    dimension and (polynomial) basis orders per dimension */
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
  size_t num_v = sharedDataRep->numVars;  bool err_flag = false;
  if (ranks.length() != num_v + 1) { // both ends padded with 1's
    Cerr << "Error: wrong rank array size (" << ranks.length() << ", "
	 << num_v+1 << " expected) in C3Approximation::regression_size()."
	 << std::endl;
    err_flag = true;
  }
  if (orders.size() != num_v) {      // no padding
    Cerr << "Error: wrong order array size (" << orders.size() << ", " << num_v
	 << " expected) in C3Approximation::regression_size()." << std::endl;
    err_flag = true;
  }
  if (err_flag)
    abort_handler(APPROX_ERROR);

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
