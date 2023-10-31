/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_C3_INCLUDE
#define DAKOTA_C3_INCLUDE

#include <cstddef>

// Declarations of C functions provided by TPL C3
// NOTE: Direct inclusion of c3.h from the TPL even with extern C is problematic

extern "C"
{
  // #include "c3.h"
    
  // utilities
  double * linspace(double,double,size_t);
  double ** malloc_dd(size_t);
  void free_dd(size_t, double **);

  // function monitoring
  struct FunctionMonitor;
  void function_monitor_free(struct FunctionMonitor *);
  double function_monitor_eval(const double *, void *);
  struct FunctionMonitor * 
  function_monitor_initnd( double (*)(const double *, void *), void *, size_t, size_t);
    
  // function wrapping
  struct Fwrap;
  struct Fwrap * fwrap_create(size_t, const char *);
  void fwrap_set_fvec(struct Fwrap*,int(*)(size_t,const double*,double*,void*),void*);
  void fwrap_destroy(struct Fwrap *);

  // polynomial approximation options
  struct OpeOpts;
  enum poly_type {LEGENDRE, CHEBYSHEV, HERMITE, STANDARD, FOURIER};
  struct OpeOpts * ope_opts_alloc(enum poly_type);
  void ope_opts_set_lb(struct OpeOpts *, double);
  double ope_opts_get_lb(const struct OpeOpts *);
  void ope_opts_set_ub(struct OpeOpts *, double);
  double ope_opts_get_ub(const struct OpeOpts *);
  void ope_opts_set_start(struct OpeOpts *, size_t);
  void ope_opts_set_nparams(struct OpeOpts *, size_t);
  void ope_opts_set_maxnum(struct OpeOpts *,size_t);
  void ope_opts_set_tol(struct OpeOpts *, double);
    
  // One dimensional approximation options
  enum function_class {PIECEWISE, POLYNOMIAL, CONSTELM, LINELM, KERNEL};
  struct OneApproxOpts;
  struct OneApproxOpts * one_approx_opts_alloc(enum function_class, void *);
  void one_approx_opts_set_nparams(struct OneApproxOpts *, size_t);
  void one_approx_opts_set_maxnum(struct OneApproxOpts *, size_t);
  size_t one_approx_opts_get_nparams(const struct OneApproxOpts *);
  void one_approx_opts_free_deep(struct OneApproxOpts **);

  struct MultiApproxOpts;

  // Function Train and analysis
  struct FunctionTrain;
  size_t function_train_get_dim(const struct FunctionTrain *);
  size_t function_train_get_nparams(const struct FunctionTrain *);
  size_t * function_train_get_ranks(const struct FunctionTrain *);
  size_t function_train_get_avgrank(const struct FunctionTrain *);
  size_t function_train_get_maxrank(const struct FunctionTrain *);

  void function_train_free(struct FunctionTrain *);
  struct FunctionTrain * function_train_constant(double, struct MultiApproxOpts *);
  void function_train_scale(struct FunctionTrain *, double);
  struct FunctionTrain * function_train_copy(const struct FunctionTrain *);
  double function_train_eval(struct FunctionTrain *, const double *);
  double function_train_integrate(const struct FunctionTrain *);
  double function_train_integrate_weighted(const struct FunctionTrain *);
  struct FunctionTrain *
  function_train_integrate_weighted_subset(
    const struct FunctionTrain * ft, size_t,size_t *);
    
  double function_train_inner(const struct FunctionTrain *, 
			      const struct FunctionTrain * );
  double function_train_inner_weighted(const struct FunctionTrain *, 
				       const struct FunctionTrain * );
  struct FunctionTrain * 
  function_train_sum(const struct FunctionTrain *,const struct FunctionTrain *);
  struct FunctionTrain *
  function_train_round(struct FunctionTrain *, double,struct MultiApproxOpts *);
  struct FunctionTrain *
  function_train_product(const struct FunctionTrain *, 
			 const struct FunctionTrain *);
  int function_train_save(struct FunctionTrain *,char *);
  void function_train_sobol_sensitivities(const struct FunctionTrain *,double*,double*,size_t);


  struct FT1DArray;
  struct FT1DArray{
    size_t size;
    struct FunctionTrain ** ft;
  };
  struct FT1DArray * function_train_gradient(const struct FunctionTrain *);
  struct FT1DArray * ft1d_array_jacobian(const struct FT1DArray *);
  struct FT1DArray * ft1d_array_copy(const struct FT1DArray *);
  void ft1d_array_free(struct FT1DArray *);
    
  // General approximation options
  enum C3ATYPE { CROSS, REGRESS, C3UNSPEC };
  struct C3Approx;
  struct C3Approx * c3approx_create(enum C3ATYPE, size_t);
  void c3approx_destroy(struct C3Approx *);
  void c3approx_set_approx_opts_dim(struct C3Approx *,size_t,
				    struct OneApproxOpts *);
  //setting cross approximation arguments
  void c3approx_init_cross(struct C3Approx * c3a, size_t, int, double **);
  void c3approx_set_round_tol(struct C3Approx *, double);
  void c3approx_set_cross_tol(struct C3Approx *, double);
  void c3approx_set_verbose(struct C3Approx *, int);
  void c3approx_set_adapt_kickrank(struct C3Approx *, size_t);
  void c3approx_set_adapt_maxrank_all(struct C3Approx *, size_t);
  //void c3approx_set_adapt_maxiter(struct C3Approx *, size_t);
  void c3approx_set_cross_maxiter(struct C3Approx *, size_t);

  void c3approx_init_cross(struct C3Approx * c3a,size_t,int,double **);
  struct FunctionTrain * c3approx_do_cross(struct C3Approx *,struct Fwrap *,int);
  struct MultiApproxOpts * c3approx_get_approx_args(const struct C3Approx *);
  struct MultiApproxOpts * multi_approx_opts_alloc(size_t);
  void multi_approx_opts_free(struct MultiApproxOpts *);
  void multi_approx_opts_set_dim(struct MultiApproxOpts *,size_t,struct OneApproxOpts *);
                               
  void c3axpy (double a, struct FunctionTrain *x, struct FunctionTrain **y, double epsilon, struct MultiApproxOpts * opts);
    
  // sobol sensitivities;
  struct C3SobolSensitivity;
  struct C3SobolSensitivity *
  c3_sobol_sensitivity_calculate(const struct FunctionTrain *,size_t);
  void c3_sobol_sensitivity_free(struct C3SobolSensitivity *);
  double c3_sobol_sensitivity_get_interaction(const struct C3SobolSensitivity *,
					      size_t, const size_t *);
  double c3_sobol_sensitivity_get_total(const struct C3SobolSensitivity *, size_t);
  double c3_sobol_sensitivity_get_main(const struct C3SobolSensitivity *, size_t);
  double c3_sobol_sensitivity_get_variance(const struct C3SobolSensitivity *);
  void c3_sobol_sensitivity_print(const struct C3SobolSensitivity *);
  void c3_sobol_sensitivity_apply_external(const C3SobolSensitivity *,
					   void (*)(double, size_t, size_t*,void*),
					   void*);

  // optimizer
  enum c3opt_alg {BFGS, LBFGS, BATCHGRAD, BRUTEFORCE, SGD};
  struct c3Opt;
  struct c3Opt * c3opt_create(enum c3opt_alg);
  void c3opt_set_relftol(struct c3Opt *, double);
  void c3opt_set_gtol(struct c3Opt *, double);
  void c3opt_set_verbose(struct c3Opt *, int);
  void c3opt_set_maxiter(struct c3Opt *, size_t);
  void c3opt_ls_set_maxiter(struct c3Opt *, size_t);
  void c3opt_set_absxtol(struct c3Opt *, double);
  void c3opt_free(struct c3Opt *);

  enum REGTYPE {ALS,AIO,REGNONE};
  enum REGOBJ  {FTLS,FTLS_SPARSEL2,REGOBJNONE};
  struct FTRegress;
  struct FTRegress * ft_regress_alloc(size_t dim, struct MultiApproxOpts *,size_t * ranks);
  size_t ft_regress_get_dim(const struct FTRegress *);
  void ft_regress_set_adapt(struct FTRegress *, int);
  void ft_regress_set_maxrank(struct FTRegress *, size_t);
  void ft_regress_set_kickrank(struct FTRegress *, size_t);
  void ft_regress_set_roundtol(struct FTRegress *, double);
  void ft_regress_set_kfold(struct FTRegress *, size_t);
  void ft_regress_set_finalize(struct FTRegress *, int);
  void ft_regress_set_opt_restrict(struct FTRegress *, int);

  void ft_regress_free(struct FTRegress *);
  void ft_regress_reset_param(struct FTRegress*, struct MultiApproxOpts *, size_t *);
  void ft_regress_set_type(struct FTRegress *, enum REGTYPE);
  void ft_regress_set_obj(struct FTRegress *, enum REGOBJ);
  void ft_regress_set_alg_and_obj(struct FTRegress *, enum REGTYPE, enum REGOBJ);
  void ft_regress_set_stoch_obj(struct FTRegress *, int);

  void ft_regress_set_als_conv_tol(struct FTRegress *, double);
  void ft_regress_set_max_als_sweep(struct FTRegress *, size_t);
  void ft_regress_set_verbose(struct FTRegress *, int);
  void ft_regress_set_regularization_weight(struct FTRegress *, double);
  double ft_regress_get_regularization_weight(const struct FTRegress *);
  double * ft_regress_get_params(struct FTRegress *, size_t *);
  void ft_regress_update_params(struct FTRegress *, const double *);

  void ft_regress_set_seed(struct FTRegress *, unsigned int);

  struct FunctionTrain *
  ft_regress_run(struct FTRegress *,struct c3Opt *,size_t,const double* xdata, const double * ydata);

  // cross validation
  struct CrossValidate;
  void cross_validate_free(struct CrossValidate *);
  struct CrossValidate * cross_validate_init(size_t, size_t,
					     const double *,
					     const double *,
					     size_t, int);
  double cross_validate_run(struct CrossValidate *,
			    struct FTRegress *,
			    struct c3Opt *);

  struct CVOptGrid;
  struct CVOptGrid * cv_opt_grid_init(size_t);
  void cv_opt_grid_free(struct CVOptGrid *);
  void cv_opt_grid_set_verbose(struct CVOptGrid *, int);
  void cv_opt_grid_add_param(struct CVOptGrid *, char *, size_t,
			     void * paramlist);

  void cross_validate_grid_opt(struct CrossValidate *,
			       struct CVOptGrid *,
			       struct FTRegress *,
			       struct c3Opt *);
}


namespace Dakota {

struct FTDerivedFunctions
{
  size_t allocated;
    
  struct FunctionTrain * ft_nonrand; // needed for allVars
  struct FunctionTrain * ft_squared; // needed for allVars
  struct FunctionTrain * ft_squared_nonrand; // needed for allVars
  //struct FunctionTrain * ft_cubed;
  //struct FunctionTrain * ft_tesseracted;

  //struct FunctionTrain * ft_constant_at_mean;
  struct FunctionTrain * ft_diff_from_mean;
  struct FunctionTrain * ft_diff_from_mean_squared;
  //struct FunctionTrain * ft_diff_from_mean_cubed;
  //struct FunctionTrain * ft_diff_from_mean_tesseracted;// courtesy of dan

  //struct FunctionTrain * ft_diff_from_mean_normalized;
  //struct FunctionTrain * ft_diff_from_mean_normalized_squared;
  //struct FunctionTrain * ft_diff_from_mean_normalized_cubed;

  // raw moments
  double first_moment;
  //double second_moment;
  //double third_moment;
  //double fourth_moment;

  // central moments
  double second_central_moment;
  double third_central_moment;
  double fourth_central_moment;

  // standardized moments
  double std_dev;
  double skewness;
  double excess_kurtosis;
};

}

#endif
