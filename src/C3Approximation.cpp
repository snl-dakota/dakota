/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ProblemDescDB.hpp"
#include "C3Approximation.hpp"

#include "SharedC3ApproxData.hpp"
//#include "NonDIntegration.hpp"

//#define DEBUG

namespace Dakota {


void C3FnTrainPtrs::ft_derived_functions_init_null()
{
  ft_derived_functions->set = 0;
        
  ft_derived_functions->ft_squared                = NULL;
  ft_derived_functions->ft_cubed                  = NULL;
  ft_derived_functions->ft_constant_at_mean       = NULL;
  ft_derived_functions->ft_diff_from_mean         = NULL;
  ft_derived_functions->ft_diff_from_mean_squared = NULL;
  ft_derived_functions->ft_diff_from_mean_cubed   = NULL;        

  ft_derived_functions->ft_diff_from_mean_tesseracted = NULL;
  ft_derived_functions->ft_diff_from_mean_normalized  = NULL;

  ft_derived_functions->ft_diff_from_mean_normalized_squared = NULL;
  ft_derived_functions->ft_diff_from_mean_normalized_cubed   = NULL;
}


void C3FnTrainPtrs::ft_derived_functions_create(struct MultiApproxOpts * opts)
{
  // printf("CREATE DERIVED_FUNCTIONS\n");
        
  ft_derived_functions->ft_squared     = function_train_product(ft,ft);

  ft_derived_functions->ft_cubed       = function_train_product(ft_derived_functions->ft_squared,ft);
  // ft_derived_functions->ft_tesseracted = function_train_product(ft_derived_functions->ft_squared, ft_derived_functions->ft_squared);

  ft_derived_functions->first_moment        = function_train_integrate_weighted(ft);
  ft_derived_functions->ft_constant_at_mean = function_train_constant(-ft_derived_functions->first_moment,opts);
  ft_derived_functions->ft_diff_from_mean   = function_train_sum(ft,ft_derived_functions->ft_constant_at_mean);
  ft_derived_functions->ft_diff_from_mean_squared =
    function_train_product(ft_derived_functions->ft_diff_from_mean,
			   ft_derived_functions->ft_diff_from_mean);
  ft_derived_functions->ft_diff_from_mean_cubed =
    function_train_product(ft_derived_functions->ft_diff_from_mean_squared,
			   ft_derived_functions->ft_diff_from_mean);        
  ft_derived_functions->ft_diff_from_mean_tesseracted =
    function_train_product(ft_derived_functions->ft_diff_from_mean_squared,
			   ft_derived_functions->ft_diff_from_mean_squared);

  ft_derived_functions->second_central_moment = function_train_integrate_weighted(ft_derived_functions->ft_diff_from_mean_squared); // var
        
  ft_derived_functions->third_central_moment = function_train_integrate_weighted(ft_derived_functions->ft_diff_from_mean_cubed); // var        
        
  ft_derived_functions->fourth_central_moment = function_train_integrate_weighted(ft_derived_functions->ft_diff_from_mean_tesseracted);

  ft_derived_functions->second_moment = function_train_integrate_weighted(ft_derived_functions->ft_squared);
  ft_derived_functions->third_moment =  function_train_integrate_weighted(ft_derived_functions->ft_cubed);

  ft_derived_functions->std_dev =  sqrt(ft_derived_functions->second_central_moment);

  ft_derived_functions->ft_diff_from_mean_normalized = function_train_copy(ft_derived_functions->ft_diff_from_mean);
  function_train_scale(ft_derived_functions->ft_diff_from_mean_normalized,1.0/ft_derived_functions->std_dev);


  ft_derived_functions->ft_diff_from_mean_normalized_squared =
    function_train_product(ft_derived_functions->ft_diff_from_mean_normalized,
			   ft_derived_functions->ft_diff_from_mean_normalized);

  ft_derived_functions->ft_diff_from_mean_normalized_cubed =
    function_train_product(ft_derived_functions->ft_diff_from_mean_normalized_squared,
			   ft_derived_functions->ft_diff_from_mean_normalized);

  ft_derived_functions->skewness = function_train_integrate_weighted(ft_derived_functions->ft_diff_from_mean_normalized_cubed);
  ft_derived_functions->kurtosis = ft_derived_functions->fourth_central_moment / ft_derived_functions->second_central_moment / ft_derived_functions->second_central_moment;

  ft_derived_functions->set = 1;
}


void C3FnTrainPtrs::ft_derived_functions_free()
{
  function_train_free(ft_derived_functions->ft_squared);                ft_derived_functions->ft_squared          = NULL;
  function_train_free(ft_derived_functions->ft_cubed);                  ft_derived_functions->ft_cubed            = NULL;
  function_train_free(ft_derived_functions->ft_constant_at_mean);       ft_derived_functions->ft_constant_at_mean = NULL;
  function_train_free(ft_derived_functions->ft_diff_from_mean);         ft_derived_functions->ft_diff_from_mean   = NULL;
  function_train_free(ft_derived_functions->ft_diff_from_mean_squared); ft_derived_functions->ft_diff_from_mean_squared = NULL;

  function_train_free(ft_derived_functions->ft_diff_from_mean_cubed); ft_derived_functions->ft_diff_from_mean_cubed = NULL;        

  function_train_free(ft_derived_functions->ft_diff_from_mean_tesseracted); ft_derived_functions->ft_diff_from_mean_tesseracted = NULL;
  function_train_free(ft_derived_functions->ft_diff_from_mean_normalized);  ft_derived_functions->ft_diff_from_mean_normalized  = NULL;

  function_train_free(ft_derived_functions->ft_diff_from_mean_normalized_squared);
  ft_derived_functions->ft_diff_from_mean_normalized_squared = NULL;

  function_train_free(ft_derived_functions->ft_diff_from_mean_normalized_cubed);
  ft_derived_functions->ft_diff_from_mean_normalized_cubed = NULL;

  ft_derived_functions->set = 0;
}


C3Approximation::
C3Approximation(ProblemDescDB& problem_db,
		const SharedApproxData& shared_data,
		const String& approx_label):
  Approximation(BaseConstructor(), problem_db, shared_data, approx_label),
  sharedC3DataRep((SharedC3ApproxData*)sharedDataRep)
{ base_init(); }


C3Approximation::C3Approximation(const SharedApproxData& shared_data):
  Approximation(NoDBBaseConstructor(), shared_data),
  sharedC3DataRep((SharedC3ApproxData*)sharedDataRep)
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
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Calling C3Approximation::build()\n";
        
  // base class implementation checks data set against min required
  Approximation::build();

  if (sharedDataRep->adaptConstruct) {
    Cerr << "Error: Adaptive construction not yet implemented in "
	 << "C3Approximation." << std::endl;
    abort_handler(APPROX_ERROR);
  }
  else {
    double absxtol = 1e-10;
    struct c3Opt* optimizer = c3opt_create(BFGS);
    c3opt_set_maxiter(optimizer,sharedC3DataRep->maxSolverIterations);
    c3opt_set_gtol   (optimizer,sharedC3DataRep->solverTol);
    c3opt_set_relftol(optimizer,sharedC3DataRep->solverTol);
    c3opt_set_absxtol(optimizer,absxtol);
    c3opt_set_verbose(optimizer,0);

    size_t i, j, num_v = sharedDataRep->numVars;
    SizetVector start_ranks(num_v+1);  
    start_ranks(0) = 1;  start_ranks(num_v) = 1;
    for (i = 1; i < num_v; ++i)
      start_ranks(i) = sharedC3DataRep->startRank;

    struct FTRegress * ftr
      = ft_regress_alloc(num_v, sharedC3DataRep->approxOpts,
			 start_ranks.values());
	    
    if (sharedC3DataRep->regressType == FT_RLS2) {
      ft_regress_set_alg_and_obj(ftr,AIO,FTLS_SPARSEL2);
      // reg param is required (no reasonable default due to scaling)
      ft_regress_set_regularization_weight(ftr,
					   sharedC3DataRep->regressRegParam);
    }
    else // default
      ft_regress_set_alg_and_obj(ftr,AIO,FTLS);

    size_t r_adapt = sharedC3DataRep->adaptRank ? 1 : 0;
    ft_regress_set_adapt(   ftr,r_adapt);
    ft_regress_set_maxrank( ftr,sharedC3DataRep->maxRank);
    ft_regress_set_kickrank(ftr,sharedC3DataRep->kickRank);
    ft_regress_set_roundtol(ftr,sharedC3DataRep->roundingTol);
    ft_regress_set_verbose( ftr,sharedC3DataRep->c3Verbosity);

    c3opt_set_verbose(optimizer,sharedC3DataRep->c3Verbosity);
    c3opt_set_maxiter(optimizer,sharedC3DataRep->maxSolverIterations);
    c3opt_set_gtol   (optimizer,sharedC3DataRep->solverTol);
    c3opt_set_relftol(optimizer,sharedC3DataRep->solverTol);

    // free if previously built
    levApproxIter->free_ft();

    if (sharedDataRep->crossVal) // future capability for poly orders
      Cerr << "Warning: CV is not yet implemented in C3Approximation.  "
	   << "Ignoring CV request.\n";

    const Pecos::SurrogateData& approx_data = surrogate_data();
    const Pecos::SDVArray& sdv_array = approx_data.variables_data();
    const Pecos::SDRArray& sdr_array = approx_data.response_data();
    size_t ndata = approx_data.points();

    // JUST 1 QOI
    // Transfer the training data to the Teuchos arrays used by the GP
    // input variables (reformats surrData for C3)
    double* xtrain = (double*)calloc(num_v*ndata,sizeof(double));
    // QoI observations (reformats surrData for C3)
    double* ytrain = (double*)calloc(ndata,sizeof(double));

    // process currentPoints
    for (i=0; i<ndata; ++i) {
      const RealVector& c_vars = sdv_array[i].continuous_variables();
      for (j=0; j<num_v; j++)
	xtrain[j + i*num_v] = c_vars[j];
      ytrain[i] = sdr_array[i].response_function();
    }

    // Build FT model
    levApproxIter->ft = ft_regress_run(ftr,optimizer,ndata,xtrain,ytrain);
    // *** TO DO: add flag/final_asv control of this (wasteful calculations if not needed)
    levApproxIter->ft_gradient = function_train_gradient(levApproxIter->ft);
    levApproxIter->ft_hessian  = ft1d_array_jacobian(levApproxIter->ft_gradient);

    // free approximation stuff
    free(xtrain);          xtrain    = NULL;
    free(ytrain);          ytrain    = NULL;
    ft_regress_free(ftr);  ftr       = NULL;
    c3opt_free(optimizer); optimizer = NULL;
  }
}


void C3Approximation::compute_all_sobol_indices(size_t interaction_order)
{
  if (ft_sobol == NULL){
    ft_sobol
      = c3_sobol_sensitivity_calculate(levApproxIter->ft,interaction_order);
  }
  else{
    c3_sobol_sensitivity_free(ft_sobol);
    ft_sobol
      = c3_sobol_sensitivity_calculate(levApproxIter->ft,interaction_order);
  }
}


void C3Approximation::compute_derived_statistics(bool overwrite)
{
  if (levApproxIter->ft_derived_functions.set == 0)
    levApproxIter->ft_derived_functions_create(sharedC3DataRep->approxOpts);
  else if (overwrite == true) {
    levApproxIter->ft_derived_functions_free();
    levApproxIter->ft_derived_functions_create(sharedC3DataRep->approxOpts);
  }
}


void C3Approximation::compute_moments(bool full_stats, bool combined_stats)
{
  expansionMoments[0] = mean();
  expansionMoments[1] = variance();
  expansionMoments[2] = third_central();
  expansionMoments[3] = fourth_central();
}


void C3Approximation::
compute_moments(const RealVector& x, bool full_stats, bool combined_stats)
{
  expansionMoments[0] = mean(x);
  expansionMoments[1] = variance(x);
}


Real C3Approximation::mean()
{
  compute_derived_statistics(false);
  return levApproxIter->ft_derived_functions.first_moment;
}


Real C3Approximation::mean(const RealVector &x)
{
  // compute_derived_statistics(false);

  const SizetVector& rand_indices = sharedDataRep->randomIndices;
  struct FunctionTrain * ftnonrand =
    function_train_integrate_weighted_subset(levApproxIter->ft,
      rand_indices.length(),rand_indices.values());

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
  return levApproxIter->ft_derived_functions.second_central_moment;
}


Real C3Approximation::variance(const RealVector &x)
{
  compute_derived_statistics(false);

  const SizetVector& rand_indices = sharedDataRep->randomIndices;
  struct FunctionTrain * ftnonrand =
    function_train_integrate_weighted_subset(
      levApproxIter->ft_derived_functions.ft_squared,
      rand_indices.length(),rand_indices.values());

  //size_t num_det = sharedDataRep->numVars - num_rand;
  //for (size_t ii = 0; ii < num_det;ii++)
  //  pt_det[ii] = x(ii);
  //double var = function_train_eval(ftnonrand,pt_det) - mean * mean;

  Real mean = mean(),
        var = function_train_eval(ftnonrand,x.values()) - mean * mean;
    
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
  struct FunctionTrain * ftconst = function_train_constant(val,sharedC3DataRep->approxOpts);
  struct FunctionTrain * updated = function_train_sum(levApproxIter->ft,ftconst);

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

    
// ignore discrete variables for now
Real C3Approximation::value(const Variables& vars)
{
  return function_train_eval(levApproxIter->ft,
			     vars.continuous_variables().values());
}


// ignore discrete variables for now
const RealVector& C3Approximation::gradient(const Variables& vars)
{
  size_t num_v = sharedDataRep->numVars;
  if (approxGradient.empty())
    approxGradient.sizeUninitialized(num_v);
  const Real* c_vars = vars.continuous_variables().values();
  for (size_t i = 0; i < num_v; ++i)
    approxGradient(i)
      = function_train_eval(levApproxIter->ft_gradient->ft[i], c_vars);
  return approxGradient;
}


// ignore discrete variables for now
const RealSymMatrix& C3Approximation::hessian(const Variables& vars)
{
  size_t num_v = sharedDataRep->numVars;
  if (approxHessian.empty())
    approxHessian.shapeUninitialized(num_v);
  const Real* c_vars = vars.continuous_variables().values();
  for (size_t i = 0; i < num_v; ++i)
    for (size_t j = 0; j <= i; ++j)
      approxHessian(i,j)
	= function_train_eval(levApproxIter->ft_hessian->ft[i+j*num_v], c_vars);
  return approxHessian;
}

} // namespace Dakota
