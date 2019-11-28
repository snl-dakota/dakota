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
  // Free the rest of the stuff
  // TODO FREE OPTIONS

  function_train_free(levApproxIter->ft);
  levApproxIter->ft          = NULL;
  ft1d_array_free(levApproxIter->ft_gradient);
  levApproxIter->ft_gradient = NULL;
  ft1d_array_free(levApproxIter->ft_hessian);
  levApproxIter->ft_hessian  = NULL;
  c3_sobol_sensitivity_free(levApproxIter->ft_sobol);
  levApproxIter->ft_sobol    = NULL;
  ft_derived_functions_free(&(levApproxIter->ft_derived_functions));
}


void C3Approximation::base_init()
{
  // initialize function train data structures
  levApproxIter->ft          = NULL;
  levApproxIter->ft_gradient = NULL;
  levApproxIter->ft_hessian  = NULL;
  levApproxIter->ft_sobol    = NULL;
  ft_derived_functions_init_null(&(levApproxIter->ft_derived_functions));

  ////////////////////////////////////////////
  // Initialize memory for the rest of private data
  this->moment_vector.size(4);
}


void C3Approximation::build()
{
  printf("Calling build from c3 approximation\n");
        
  // base class implementation checks data set against min required
  Approximation::build();

  if (sharedDataRep->adaptConstruct) {
    Cerr << "Error: Adaptive construction not implemented yet in "
	 << "C3Approximation yet" << std::endl;
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

    levApproxIter->ft_opts = sharedC3DataRep->approxOpts;
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

    // *** TO DO (AAG): add mappings for latest XML attributes

    // free if previously built
    function_train_free(levApproxIter->ft);
    levApproxIter->ft          = NULL;
    ft1d_array_free(levApproxIter->ft_gradient);
    levApproxIter->ft_gradient = NULL;
    ft1d_array_free(levApproxIter->ft_hessian);
    levApproxIter->ft_hessian  = NULL;

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


void ft_derived_functions_init_null(struct FTDerivedFunctions * func)
{
  func->set = 0;
        
  func->ft_squared                = NULL;
  func->ft_cubed                  = NULL;
  func->ft_constant_at_mean       = NULL;
  func->ft_diff_from_mean         = NULL;
  func->ft_diff_from_mean_squared = NULL;
  func->ft_diff_from_mean_cubed   = NULL;        

  func->ft_diff_from_mean_tesseracted = NULL;
  func->ft_diff_from_mean_normalized  = NULL;

  func->ft_diff_from_mean_normalized_squared = NULL;
  func->ft_diff_from_mean_normalized_cubed   = NULL;
    
}


void ft_derived_functions_create(struct FTDerivedFunctions * func,
				 struct FunctionTrain * ft,
				 struct MultiApproxOpts * opts)
{

  // printf("CREATE DERIVED_FUNCTIONS\n");
        
  func->ft_squared     = function_train_product(ft,ft);

  func->ft_cubed       = function_train_product(func->ft_squared,ft);
  // func->ft_tesseracted = function_train_product(func->ft_squared, func->ft_squared);

  func->first_moment        = function_train_integrate_weighted(ft);
  func->ft_constant_at_mean = function_train_constant(-func->first_moment,opts);
  func->ft_diff_from_mean   = function_train_sum(ft,func->ft_constant_at_mean);
  func->ft_diff_from_mean_squared =
    function_train_product(func->ft_diff_from_mean,
			   func->ft_diff_from_mean);
  func->ft_diff_from_mean_cubed =
    function_train_product(func->ft_diff_from_mean_squared,
			   func->ft_diff_from_mean);        
  func->ft_diff_from_mean_tesseracted =
    function_train_product(func->ft_diff_from_mean_squared,
			   func->ft_diff_from_mean_squared);

  func->second_central_moment = function_train_integrate_weighted(func->ft_diff_from_mean_squared); // var
        
  func->third_central_moment = function_train_integrate_weighted(func->ft_diff_from_mean_cubed); // var        
        
  func->fourth_central_moment = function_train_integrate_weighted(func->ft_diff_from_mean_tesseracted);

  func->second_moment = function_train_integrate_weighted(func->ft_squared);
  func->third_moment =  function_train_integrate_weighted(func->ft_cubed);

  func->std_dev =  sqrt(func->second_central_moment);

  func->ft_diff_from_mean_normalized = function_train_copy(func->ft_diff_from_mean);
  function_train_scale(func->ft_diff_from_mean_normalized,1.0/func->std_dev);


  func->ft_diff_from_mean_normalized_squared =
    function_train_product(func->ft_diff_from_mean_normalized,
			   func->ft_diff_from_mean_normalized);

  func->ft_diff_from_mean_normalized_cubed =
    function_train_product(func->ft_diff_from_mean_normalized_squared,
			   func->ft_diff_from_mean_normalized);

  func->skewness = function_train_integrate_weighted(func->ft_diff_from_mean_normalized_cubed);
  func->kurtosis = func->fourth_central_moment / func->second_central_moment / func->second_central_moment;

  func->set = 1;
    
}

void ft_derived_functions_free(struct FTDerivedFunctions * func)
{
  function_train_free(func->ft_squared);                func->ft_squared          = NULL;
  function_train_free(func->ft_cubed);                  func->ft_cubed            = NULL;
  function_train_free(func->ft_constant_at_mean);       func->ft_constant_at_mean = NULL;
  function_train_free(func->ft_diff_from_mean);         func->ft_diff_from_mean   = NULL;
  function_train_free(func->ft_diff_from_mean_squared); func->ft_diff_from_mean_squared = NULL;

  function_train_free(func->ft_diff_from_mean_cubed); func->ft_diff_from_mean_cubed = NULL;        

  function_train_free(func->ft_diff_from_mean_tesseracted); func->ft_diff_from_mean_tesseracted = NULL;
  function_train_free(func->ft_diff_from_mean_normalized);  func->ft_diff_from_mean_normalized  = NULL;

  function_train_free(func->ft_diff_from_mean_normalized_squared);
  func->ft_diff_from_mean_normalized_squared = NULL;

  function_train_free(func->ft_diff_from_mean_normalized_cubed);
  func->ft_diff_from_mean_normalized_cubed = NULL;

  func->set = 0;
}


void C3Approximation::compute_all_sobol_indices(size_t interaction_order)
{
  if (ft_sobol == NULL){
    ft_sobol = c3_sobol_sensitivity_calculate(levApproxIter->ft,interaction_order);
  }
  else{
    c3_sobol_sensitivity_free(ft_sobol);
    ft_sobol = c3_sobol_sensitivity_calculate(levApproxIter->ft,interaction_order);
  }
}


Real C3Approximation::main_sobol_index(size_t dim)
{
  return c3_sobol_sensitivity_get_main(levApproxIter->ft_sobol,dim);
}

Real C3Approximation::total_sobol_index(size_t dim)
{
  return c3_sobol_sensitivity_get_total(levApproxIter->ft_sobol,dim);
}

void C3Approximation::sobol_iterate_apply(
					  void (*f)(double val, size_t ninteract, size_t*interactions,void* arg), void* args)
{
  c3_sobol_sensitivity_apply_external(levApproxIter->ft_sobol,f,args);
}

void C3Approximation::compute_derived_statistics(bool overwrite)
{
  if (levApproxIter->ft_derived_functions.set == 0)
    {
      ft_derived_functions_create(&(levApproxIter->ft_derived_functions),levApproxIter->ft,levApproxIter->ft_opts);
    }
  else if (overwrite == true){
    ft_derived_functions_free(&(levApproxIter->ft_derived_functions));
    ft_derived_functions_create(&(levApproxIter->ft_derived_functions),levApproxIter->ft,levApproxIter->ft_opts);
  }
}

void C3Approximation::compute_moments(bool full_stats, bool combined_stats)
{
  // printf("mean in compute_moments = %G\n",this->mean());
  // printf("length of moment vector = %d\n",this->moment_vector.length());
  this->moment_vector(0) = this->mean();
  this->moment_vector(1) = this->variance();
  this->moment_vector(2) = this->third_central();
  this->moment_vector(3) = this->fourth_central();
}

void C3Approximation::compute_moments(const RealVector& x, bool full_stats,
				      bool combined_stats) {
  this->moment_vector(0) = this->mean(x);
  this->moment_vector(1) = this->variance(x);
}

const RealVector& C3Approximation::moments() const{
  return this->moment_vector;
}

Real C3Approximation::moment(size_t i) const{
  return this->moment_vector(i);
}

void C3Approximation::moment(Real mom, size_t i) {
  this->moment_vector(i) = mom;
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
  fprintf(stderr,"mean_gradient() in C3Approximation is not implemented because Alex\n");
  fprintf(stderr,"is not sure how what it means\n");
  exit(1);
}

const RealVector & C3Approximation::mean_gradient(const RealVector &x,const SizetArray & dvv)
{
  fprintf(stderr,"mean_gradient(x,dvv) in C3Approximation is not implemented because Alex\n");
  fprintf(stderr,"is not sure how what it means\n");
  exit(1);        
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
  //for (size_t ii = 0; ii < this->num_det;ii++)
  //  this->pt_det[ii] = x(ii);
  //double var = function_train_eval(ftnonrand,this->pt_det) - mean * mean;

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
  fprintf(stderr,"mean_gradient() in C3Approximation is not implemented because Alex\n");
  fprintf(stderr,"is not sure how what it means\n");
  exit(1);
}

const RealVector & C3Approximation::variance_gradient(const RealVector &x,const SizetArray & dvv)
{
  // size of output is size of dvv, only take gradients with respect to dvv fix design and epistemic at x
  fprintf(stderr,"mean_gradient(x,dvv) in C3Approximation is not implemented because Alex\n");
  fprintf(stderr,"is not sure how what it means\n");
  exit(1);        
}

struct FunctionTrain * C3Approximation::subtract_const(Real val)
{
  struct FunctionTrain * ftconst = function_train_constant(val,levApproxIter->ft_opts);
  struct FunctionTrain * updated = function_train_sum(levApproxIter->ft,ftconst);

  function_train_free(ftconst); ftconst = NULL;
  return updated;
}


Real C3Approximation::covariance(Approximation& approx_2)
{
  C3Approximation* ft2 = (C3Approximation*)approx_2.approx_rep();
  Real mean1 = this->mean();
  Real mean2 = ft2->mean();

  struct FunctionTrain * fttemp = this->subtract_const(mean1);
  struct FunctionTrain * fttemp2 = ft2->subtract_const(mean2);

  double retval = function_train_inner_weighted(fttemp,fttemp2);

  function_train_free(fttemp);  fttemp = NULL;
  function_train_free(fttemp2); fttemp2 = NULL;

  return retval;
}

Real C3Approximation::covariance(const RealVector& x, Approximation& approx_2)
{
  C3Approximation* ft2 = (C3Approximation*)approx_2.approx_rep();

  fprintf(stderr,"covariance(x,ft2) in C3Approximation is not implemented because Alex\n");
  fprintf(stderr,"is not sure how what it means\n");
  exit(1);
}


Real C3Approximation::third_central()
{
  compute_derived_statistics(false);
  return levApproxIter->ft_derived_functions.third_central_moment;
}

Real C3Approximation::fourth_central()
{
  compute_derived_statistics(false);
  return levApproxIter->ft_derived_functions.fourth_central_moment;
}


Real C3Approximation::skewness()
{
  compute_derived_statistics(false);
  return levApproxIter->ft_derived_functions.skewness;
}

Real C3Approximation::kurtosis()
{
  compute_derived_statistics(false);
  return levApproxIter->ft_derived_functions.kurtosis;
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
