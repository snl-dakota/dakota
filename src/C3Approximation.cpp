/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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
// the copy ctor of the body and just use the default copy for the
// handle
C3FnTrainPtrs C3FnTrainPtrs::copy() const
{
  C3FnTrainPtrs ftp; // new envelope with ftpRep default allocated

  ftp.ftpRep->ft          = function_train_copy(ftpRep->ft);
  ftp.ftpRep->ft_gradient = ft1d_array_copy(ftpRep->ft_gradient);
  ftp.ftpRep->ft_hessian  = ft1d_array_copy(ftpRep->ft_hessian);

  // ft_derived_fns,ft_sobol have been assigned NULL and can be allocated
  // downsteam when needed for stats,indices

  return ftp;
}


void C3FnTrainPtrs::swap(C3FnTrainPtrs& ftp)
{
  ftpRep.swap(ftp.ftpRep);
}


// TO DO: shallow copy would be better for this case, but requires ref counting
C3FnTrainPtrs::C3FnTrainPtrs(const C3FnTrainPtrs& ftp)
{
  ftpRep = ftp.ftpRep;
}


C3FnTrainPtrs::~C3FnTrainPtrs()
{
}


C3FnTrainPtrs& C3FnTrainPtrs::operator=(const C3FnTrainPtrs& ftp)
{
  ftpRep = ftp.ftpRep;
  return *this;
}

// Note: the following functions init/create/free ft memory within an ftpRep
//       but do not alter ftpRep accounting

void C3FnTrainPtrs::free_ft()
{ ftpRep->free_ft(); }


void C3FnTrainPtrs::free_all()
{ ftpRep->free_all(); }


void C3FnTrainPtrs::derived_functions_init_null()
{ ftpRep->ft_derived_functions_init_null(); }


void C3FnTrainPtrs::
derived_functions_create(struct MultiApproxOpts * opts)
{ ftpRep->ft_derived_functions_create(opts); }


void C3FnTrainPtrs::derived_functions_free()
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





C3Approximation::
C3Approximation(ProblemDescDB& problem_db,
		const SharedApproxData& shared_data,
		const String& approx_label):
  Approximation(BaseConstructor(), problem_db, shared_data, approx_label),
  levApproxIter(levelApprox.end())
{ base_init(); }


C3Approximation::C3Approximation(const SharedApproxData& shared_data):
  Approximation(NoDBBaseConstructor(), shared_data),
  levApproxIter(levelApprox.end())
{ base_init(); }


C3Approximation::~C3Approximation()
{
  // memory deallocations now managed by C3FnTrainPtrs dtor
}


void C3Approximation::base_init()
{
  // NULL ptr initializations now managed by C3FnTrainPtrs ctor

  // Initialize memory for the rest of private data
  expansionMoments.size(4);
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
    size_t i, j, num_v = sharedDataRep->numVars;
    SizetVector start_ranks(num_v+1);  
    start_ranks(0) = 1;  start_ranks(num_v) = 1;
    for (i=1; i<num_v; ++i)
      start_ranks(i) = data_rep->startRank;

    struct FTRegress * ftr
      = ft_regress_alloc(num_v, data_rep->approxOpts, start_ranks.values());
	    
    if (data_rep->regressType == FT_RLS2) {
      ft_regress_set_alg_and_obj(ftr,AIO,FTLS_SPARSEL2);
      // reg param is required (no reasonable default due to scaling)
      ft_regress_set_regularization_weight(ftr, data_rep->regressRegParam);
    }
    else // default
      ft_regress_set_alg_and_obj(ftr,AIO,FTLS);

    size_t r_adapt = data_rep->adaptRank ? 1 : 0;
    ft_regress_set_adapt(   ftr,r_adapt);
    ft_regress_set_maxrank( ftr,data_rep->maxRank);
    ft_regress_set_kickrank(ftr,data_rep->kickRank);
    ft_regress_set_roundtol(ftr,data_rep->roundingTol);
    ft_regress_set_verbose( ftr,data_rep->c3Verbosity);

    double absxtol = 1e-10;
    struct c3Opt* optimizer = c3opt_create(BFGS);
    c3opt_set_maxiter(optimizer,data_rep->maxSolverIterations);
    c3opt_set_gtol   (optimizer,data_rep->solverTol);
    c3opt_set_relftol(optimizer,data_rep->solverTol);
    c3opt_set_absxtol(optimizer,absxtol);
    c3opt_set_verbose(optimizer,data_rep->c3Verbosity);

    // free if previously built
    levApproxIter->second.free_ft();

    if (data_rep->crossVal) // future capability for poly orders
      Cerr << "Warning: CV is not yet implemented in C3Approximation.  "
	   << "Ignoring CV request.\n";

    const Pecos::SDVArray& sdv_array = approxData.variables_data();
    const Pecos::SDRArray& sdr_array = approxData.response_data();
    size_t ndata = approxData.points();

    // JUST 1 QOI
    // Transfer the training data to the Teuchos arrays used by the GP
    // input variables (reformats approxData for C3)
    double* xtrain = (double*)calloc(num_v*ndata,sizeof(double));
    // QoI observations (reformats approxData for C3)
    double* ytrain = (double*)calloc(ndata,sizeof(double));

    // process currentPoints
    for (i=0; i<ndata; ++i) {
      const RealVector& c_vars = sdv_array[i].continuous_variables();
      for (j=0; j<num_v; j++)
	xtrain[j + i*num_v] = c_vars[j];
      ytrain[i] = sdr_array[i].response_function();
    }

    // Build FT model
    struct FunctionTrain * ft
      = ft_regress_run(ftr,optimizer,ndata,xtrain,ytrain);
    levApproxIter->second.function_train(ft);
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
    //  levApproxIter->second.ft_gradient(ftg);
    //  levApproxIter->second.ft_hessian(ft1d_array_jacobian(ftg));
    //}

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
  prevC3FTPtrs = levApproxIter->second.copy(); // deep copy

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
      levApproxIter->second = *rv_it; // shallow copy of popped state
      ftp_deque.erase(rv_it);         // removal of original
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
  // Option 1: adds x to y and overwrites y (I allocate x and y)
  combinedC3FTPtrs.free_ft();
  std::map<UShortArray, C3FnTrainPtrs>::iterator it = levelApprox.begin();
  struct FunctionTrain * y = function_train_copy(it->second.function_train());
  ++it;
  for (; it!= levelApprox.end(); ++it)
    c3axpy(1., it->second.function_train(), &y, 1.e-8);
  combinedC3FTPtrs.function_train(y);

  // Could also do this at the C3FnTrainPtrs level with ft1d_array support:
  //combinedC3FTPtrs = it->second.copy(); ++it;
  //for (; it!= levelApprox.end(); ++it)
  //  sum ft,ft_gradient,ft_hessian...

  // Option 2: function_train_sum (I allocate a and b and C3 allocates c)
  // > remember to deallocate c when done
  //struct FunctionTrain* c = function_train_sum(a, b);
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
  short discrep_type = data_rep->discrepancyType;
  if (!discrep_type ||
      !Pecos::DiscrepancyCalculator::aggregated_key(active_key))
    return;

  switch (discrep_type) {
  case Pecos::RECURSIVE_DISCREP:
    //response_data_to_surplus_data();
    break;
  case Pecos::DISTINCT_DISCREP:
    // If an aggregated (discrepancy) key is active, compute the necessary
    // aggregation from the latest model datasets
    Pecos::DiscrepancyCalculator::compute(approxData, active_key,
					  data_rep->combineType);
    break;
  }
}


/*
void C3Approximation::response_data_to_surplus_data()
{
  // ...

  // compute discrepancy faults from scratch (mostly mirrors HF failures but
  // might possibly add new ones for multiplicative FPE)
  approxData.data_checks();
}
*/


void C3Approximation::compute_all_sobol_indices(size_t interaction_order)
{
  C3SobolSensitivity* fts = levApproxIter->second.sobol();
  if (fts) c3_sobol_sensitivity_free(fts);
  fts = c3_sobol_sensitivity_calculate(levApproxIter->second.function_train(),
				       interaction_order);
  levApproxIter->second.sobol(fts);
}


void C3Approximation::compute_derived_statistics(bool overwrite)
{
  SharedC3ApproxData* data_rep = (SharedC3ApproxData*)sharedDataRep;
  if (levApproxIter->second.derived_functions().set == 0)
    levApproxIter->second.derived_functions_create(data_rep->approxOpts);
  else if (overwrite == true) {
    levApproxIter->second.derived_functions_free();
    levApproxIter->second.derived_functions_create(data_rep->approxOpts);
  }
}


void C3Approximation::compute_moments(bool full_stats, bool combined_stats)
{
  expansionMoments[0] = mean();          expansionMoments[1] = variance();
  expansionMoments[2] = third_central(); expansionMoments[3] = fourth_central();
}


void C3Approximation::
compute_moments(const RealVector& x, bool full_stats, bool combined_stats)
{ expansionMoments[0] = mean(x);         expansionMoments[1] = variance(x); }


Real C3Approximation::mean()
{
  compute_derived_statistics(false);
  return levApproxIter->second.derived_functions().first_moment;
}


Real C3Approximation::mean(const RealVector &x)
{
  // compute_derived_statistics(false);

  SharedC3ApproxData* data_rep = (SharedC3ApproxData*)sharedDataRep;
  const SizetVector& rand_indices = data_rep->randomIndices;
  struct FunctionTrain * ftnonrand =
    function_train_integrate_weighted_subset(
      levApproxIter->second.function_train(), rand_indices.length(),
      rand_indices.values());

  double out = function_train_eval(ftnonrand,x.values());
  function_train_free(ftnonrand); //ftnonrand = NULL;
    
  return out;
}


const RealVector& C3Approximation::mean_gradient()
{
  Cerr << "Error: mean_gradient() in C3Approximation is not implemented "
       << "because\n       Alex is not sure how what it means" << std::endl;
  abort_handler(APPROX_ERROR);
}


const RealVector& C3Approximation::
mean_gradient(const RealVector &x,const SizetArray & dvv)
{
  Cerr << "Error: mean_gradient(x,dvv) in C3Approximation is not implemented "
       << "because\n       Alex is not sure how what it means" << std::endl;
  abort_handler(APPROX_ERROR);        
}


Real C3Approximation::variance()
{
  compute_derived_statistics(false);
  return levApproxIter->second.derived_functions().second_central_moment;
}


Real C3Approximation::variance(const RealVector &x)
{
  compute_derived_statistics(false);

  SharedC3ApproxData* data_rep = (SharedC3ApproxData*)sharedDataRep;
  const SizetVector& rand_indices = data_rep->randomIndices;
  struct FunctionTrain * ftnonrand =
    function_train_integrate_weighted_subset(
      levApproxIter->second.derived_functions().ft_squared,
      rand_indices.length(),rand_indices.values());

  //size_t num_det = sharedDataRep->numVars - num_rand;
  //for (size_t ii = 0; ii < num_det;ii++)
  //  pt_det[ii] = x(ii);
  //double var = function_train_eval(ftnonrand,pt_det) - mean * mean;

  Real mu = mean(),
      var = function_train_eval(ftnonrand,x.values()) - mu * mu;
    
  function_train_free(ftnonrand); //ftnonrand = NULL;
    
  return var;
}


const RealVector& C3Approximation::variance_gradient()
{
  // Gradient of the Variance with respect to design variables *NOT VARIANCE OF GRADIENT*
  // I need to manage low-rank representations of the gradient, then evaluate the variance of each
  // of the partial derivatives
  Cerr << "Error: mean_gradient() in C3Approximation is not implemented "
       << "because\n       Alex is not sure how what it means" << std::endl;
  abort_handler(APPROX_ERROR);
}


const RealVector & C3Approximation::
variance_gradient(const RealVector &x,const SizetArray & dvv)
{
  // size of output is size of dvv, only take gradients with respect to dvv fix design and epistemic at x
  Cerr << "Error: mean_gradient(x,dvv) in C3Approximation is not implemented "
       << "because\n       Alex is not sure how what it means" << std::endl;
  abort_handler(APPROX_ERROR);        
}


struct FunctionTrain * C3Approximation::subtract_const(Real val)
{
  SharedC3ApproxData* data_rep = (SharedC3ApproxData*)sharedDataRep;
  struct FunctionTrain * ftconst
    = function_train_constant(val,data_rep->approxOpts);
  struct FunctionTrain * updated
    = function_train_sum(levApproxIter->second.function_train(),ftconst);

  function_train_free(ftconst); ftconst = NULL;
  return updated;
}


Real C3Approximation::covariance(Approximation& approx_2)
{
  C3Approximation* ft2 = (C3Approximation*)approx_2.approx_rep();
  Real mean1 = mean(), mean2 = ft2->mean();

  struct FunctionTrain * fttemp =       subtract_const(mean1);
  struct FunctionTrain * fttemp2 = ft2->subtract_const(mean2);

  double retval = function_train_inner_weighted(fttemp,fttemp2);

  function_train_free(fttemp);  fttemp = NULL;
  function_train_free(fttemp2); fttemp2 = NULL;

  return retval;
}


Real C3Approximation::covariance(const RealVector& x, Approximation& approx_2)
{
  C3Approximation* ft2 = (C3Approximation*)approx_2.approx_rep();

  Cerr << "Error: covariance(x,ft2) in C3Approximation is not implemented "
       << "because\n       Alex is not sure how what it means" << std::endl;
  abort_handler(APPROX_ERROR);
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


/** this replaces the need to model data requirements as O(p r^2 d) */
size_t C3Approximation::regression_size()
{ return function_train_get_nparams(levApproxIter->second.function_train()); }


size_t C3Approximation::average_rank()
{ return function_train_get_avgrank(levApproxIter->second.function_train()); }


size_t C3Approximation::maximum_rank()
{ return function_train_get_maxrank(levApproxIter->second.function_train()); }


Real C3Approximation::third_central()
{
  compute_derived_statistics(false);
  return levApproxIter->second.derived_functions().third_central_moment;
}


Real C3Approximation::fourth_central()
{
  compute_derived_statistics(false);
  return levApproxIter->second.derived_functions().fourth_central_moment;
}


Real C3Approximation::skewness()
{
  compute_derived_statistics(false);
  return levApproxIter->second.derived_functions().skewness;
}


Real C3Approximation::kurtosis()
{
  compute_derived_statistics(false);
  return levApproxIter->second.derived_functions().kurtosis;
}


Real C3Approximation::main_sobol_index(size_t dim)
{ return c3_sobol_sensitivity_get_main(levApproxIter->second.sobol(),dim); }


Real C3Approximation::total_sobol_index(size_t dim)
{ return c3_sobol_sensitivity_get_total(levApproxIter->second.sobol(),dim); }


void C3Approximation::
sobol_iterate_apply(void (*f)(double val, size_t ninteract,
			      size_t*interactions,void* arg), void* args)
{ c3_sobol_sensitivity_apply_external(levApproxIter->second.sobol(),f,args); }



} // namespace Dakota
