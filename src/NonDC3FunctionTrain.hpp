/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDC3FunctionTrain
//- Description: Iterator for the 
//- Owner:       Alex Gorodetsky, Sandia National Laboratories

#ifndef NOND_C3_FUNCTION_TRAIN_H
#define NOND_C3_FUNCTION_TRAIN_H

#include "DakotaNonD.hpp"
// #include "c3/lib_funcs.h"
// #include "c3/lib_linalg.h"
// #include "c3/lib_clinalg.h"

extern "C"
{
    // utilities
    double * linspace(double,double,size_t);
    double ** malloc_dd(size_t);
    void free_dd(size_t, double **);

    // function wrapping
    struct Fwrap;
    struct Fwrap * fwrap_create(size_t, const char *);
    void fwrap_set_fvec(struct Fwrap*,int(*)(size_t,const double*,double*,void*),void*);
    void fwrap_destroy(struct Fwrap *);

    // polynomial approximation options
    struct OpeOpts;
    enum poly_type {LEGENDRE, CHEBYSHEV, HERMITE, STANDARD};
    struct OpeOpts * ope_opts_alloc(enum poly_type);
    void ope_opts_set_lb(struct OpeOpts *, double);
    double ope_opts_get_lb(const struct OpeOpts *);
    void ope_opts_set_ub(struct OpeOpts *, double);
    double ope_opts_get_ub(const struct OpeOpts *);

    
    // One dimensional approximation options
    enum function_class {CONSTANT,PIECEWISE, POLYNOMIAL,
                         LINELM, RATIONAL, KERNEL};
    struct OneApproxOpts;
    struct OneApproxOpts * one_approx_opts_alloc(enum function_class, void *);
    void one_approx_opts_free_deep(struct OneApproxOpts **);


    // Function Train and analysis
    struct FunctionTrain;
    void function_train_free(struct FunctionTrain *);
    double function_train_eval(struct FunctionTrain *, const double *);
    double function_train_integrate(const struct FunctionTrain *);
    double function_train_inner(const struct FunctionTrain *, 
                                const struct FunctionTrain * );

    
    // General approximation options
    enum C3ATYPE { CROSS, REGRESS };
    struct C3Approx;
    struct C3Approx * c3approx_create(enum C3ATYPE, size_t);
    void c3approx_destroy(struct C3Approx *);
    void c3approx_set_approx_opts_dim(struct C3Approx *,size_t,
                                  struct OneApproxOpts *);
    //setting cross approximation arguments
    void c3approx_init_cross(struct C3Approx * c3a, size_t, int,
                             double **);
    void c3approx_set_round_tol(struct C3Approx *, double);
    void c3approx_set_cross_tol(struct C3Approx *, double);
    void c3approx_set_verbose(struct C3Approx *, int);
    void c3approx_set_adapt_kickrank(struct C3Approx *, size_t);
    void c3approx_set_adapt_maxrank_all(struct C3Approx *, size_t);
    //void c3approx_set_adapt_maxiter(struct C3Approx *, size_t);
    void c3approx_set_cross_maxiter(struct C3Approx *, size_t);

    void c3approx_init_cross(struct C3Approx * c3a,size_t,int,double **);
    struct FunctionTrain * c3approx_do_cross(struct C3Approx *,struct Fwrap *,int);
}


namespace Dakota {


/// Nonintrusive uncertainty quantification with the C3 library ...

/** The NonDC3FunctionTrain class uses ... */

class NonDC3FunctionTrain: public NonD
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDC3FunctionTrain(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDC3FunctionTrain();

  //
  //- Heading: Virtual function redefinitions
  //

  /// TODO
  void pre_run();
  /// perform a forward uncertainty propagation using PCE/SC methods
  void core_run();
  /// TODO
  void post_run(std::ostream& s);
  /// print the final statistics
  void print_results(std::ostream& s);

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void resolve_inputs(short& u_space_type, short& data_order);
  void initialize(short u_space_type);
    

private:

  /// OBJFUN in NPSOL manual: computes the value and first derivatives of the
  /// objective function (passed by function pointer to NPSOL).
 static int qoi_eval(size_t num_samp,        // number of evaluations
		     const double* var_sets, // num_vars x num_evals
		     double* qoi_sets,       // num_fns x num_evals
		     void* args);            // optional arguments

  //
  //- Heading: Data
  //

  /// pointer to the active object instance used within the static evaluator
  /// functions in order to avoid the need for static data
  static NonDC3FunctionTrain* c3Instance;

  // other data ...
  /// The number of samples used to evaluate the emulator
  int numSamplesOnEmulator;

  /// The maximum rank of the function train
  size_t maxRank;

};

} // namespace Dakota

#endif
