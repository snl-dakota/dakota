/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef C3_APPROXIMATION_H
#define C3_APPROXIMATION_H

#include "DakotaApproximation.hpp"
#include "DakotaVariables.hpp"
#include "dakota_c3_include.hpp"

namespace Dakota {
    
struct FTDerivedFunctions
{

    int set;
    
    struct FunctionTrain * ft_squared;
    struct FunctionTrain * ft_cubed;
    struct FunctionTrain * ft_constant_at_mean;
    struct FunctionTrain * ft_diff_from_mean;
    struct FunctionTrain * ft_diff_from_mean_squared;
    struct FunctionTrain * ft_diff_from_mean_cubed;    
    struct FunctionTrain * ft_diff_from_mean_tesseracted;// courtesy of dan 
    struct FunctionTrain * ft_diff_from_mean_normalized;
    struct FunctionTrain * ft_diff_from_mean_normalized_squared;
    struct FunctionTrain * ft_diff_from_mean_normalized_cubed;

    // raw moments
    double first_moment;
    double second_moment;
    double third_moment;

    // central moments
    double second_central_moment;
    double third_central_moment;
    double fourth_central_moment;

    // standardized moments
    double std_dev;
    double skewness;
    double kurtosis;
};

void ft_derived_functions_init_null(struct FTDerivedFunctions * );
    
void ft_derived_functions_create(struct FTDerivedFunctions * func,
                                 struct FunctionTrain * ft,
                                 struct MultiApproxOpts * opts);

void ft_derived_functions_free(struct FTDerivedFunctions *);


    
class SharedC3ApproxData;


/// Derived approximation class for global basis polynomials.

/** The PecosApproximation class provides a global approximation
    based on basis polynomials.  This includes orthogonal polynomials
    used for polynomial chaos expansions and interpolation polynomials
    used for stochastic collocation. */


    
class C3Approximation: public Approximation
{
public:

    //
    //- Heading: Constructor and destructor
    //

    /// default constructor
    C3Approximation();
    /// standard ProblemDescDB-driven constructor
    C3Approximation(ProblemDescDB& problem_db,
                    const SharedApproxData& shared_data,
                    const String& approx_label);
    /// alternate constructor
    C3Approximation(const SharedApproxData& shared_data);
    ~C3Approximation(); // destructor

    //
    //- Heading: Member functions
    //



    /// I Dont know what the next 4 are for, but I will leave them in
    /// in case I ever find out!
    
    /// set pecosBasisApprox.configOptions.expansionCoeffFlag
    void expansion_coefficient_flag(bool coeff_flag);
    /// get pecosBasisApprox.configOptions.expansionCoeffFlag
    bool expansion_coefficient_flag() const;

    /// set pecosBasisApprox.configOptions.expansionGradFlag
    void expansion_gradient_flag(bool grad_flag);
    /// get pecosBasisApprox.configOptions.expansionGradFlag
    bool expansion_gradient_flag() const;

    void compute_moments(bool full_stats = true, bool combined_stats = false);
    void compute_moments(const Pecos::RealVector& x, bool full_stats = true,
			 bool combined_stats = false);
    const RealVector& moments() const;
    Real moment(size_t i) const;
    void moment(Real mom, size_t i);

    /// Performs global sensitivity analysis using Sobol' Indices by
    /// computing component (main and interaction) effects
    void compute_component_effects();
    /// Performs global sensitivity analysis using Sobol' Indices by
    /// computing total effects
    void compute_total_effects();

    void compute_all_sobol_indices(size_t); // computes total and interacting sobol indices
    Real total_sobol_index(size_t);         // returns total sobol index
    Real main_sobol_index(size_t);          // returns main sobol index
    // iterate over sobol indices and apply a function
    void sobol_iterate_apply(void (*)(double, size_t, size_t*,void*), void*); 
    
    Real mean();                            // expectation with respect to all variables
    Real mean(const RealVector &);          // expectation with respect to uncertain variables
    const RealVector& mean_gradient();      // NOT SURE
    // gradient with respect fixed variables
    const RealVector& mean_gradient(const RealVector &, const SizetArray &); 
    
//     inline const Pecos::RealVector& PecosApproximation::
// mean_gradient(const Pecos::RealVector& x, const Pecos::SizetArray& dvv)
// { return polyApproxRep->mean_gradient(x, dvv); }

    Real variance();                        // variance with respect to all variables
    Real variance(const RealVector&);       // variance with respect to RV, others fixed
    const RealVector& variance_gradient();      // NOT SURE
    // gradient with respect fixed variables
    const RealVector& variance_gradient(const RealVector &, const SizetArray &); 

    Real covariance(Approximation& approx_2);                    // covariance between two functions
    Real covariance(const RealVector& x, Approximation& approx_2); // covariance with respect so subset

    Real skewness();
    Real kurtosis();
    Real third_central();
    Real fourth_central();

    const RealVector& expansion_moments() const;
    const RealVector& numerical_integration_moments() const;

protected:

    //
    //- Heading: Virtual function redefinitions
    //
  
    Real                 value(const Variables& vars);
    const RealVector&    gradient(const Variables& vars);
    const RealSymMatrix& hessian(const Variables& vars);

    void build();
    //void rebuild();
    //void finalize();
    //void store(size_t index);
    //void restore(size_t index);
    //void remove_stored(size_t index);

    bool expansion_coefficient;
    bool expansion_gradient;
    int min_coefficients() const;


    SharedC3ApproxData * sharedC3DataRep;
private:

    void base_init();
    
    bool expansionCoeffFlag; // build a function_train for the quantity of interest
    bool expansionCoeffGradFlag; // build a function_train for the gradient of a quantity of interest
    
    //
    //- Heading: Convenience member functions
    //
    void compute_derived_statistics(bool overwrite);
    struct FunctionTrain * subtract_const(Real val);

    //
    //- Heading: Data
    //


    ////////////////
    // Reading in from dakota interface
    int adaptive_construction; // 0 if
    size_t dim;
    size_t num_random;
    size_t num_det;
    
    size_t * ind_random;
    
    RealVector grad;
    RealSymMatrix hess;
    RealVector moment_vector, num_moment_vector;

    // Regression information
    SizetVector start_ranks;
    bool cv;
    struct c3Opt * optimizer;
    size_t ndata;
    double * xtrain;
    double * ytrain;

    // Model
    //std::vector<struct FunctionTrain *> storedFT;
    struct FunctionTrain * ft;
    struct FT1DArray * ft_gradient;
    struct FT1DArray * ft_hessian;
    struct MultiApproxOpts * ft_opts; // <-- This is really shared
    struct FTDerivedFunctions ft_derived_functions;
    struct C3SobolSensitivity * ft_sobol;
};

inline void C3Approximation::expansion_coefficient_flag(bool coeff_flag)
{ this->expansionCoeffFlag = coeff_flag; }


inline bool C3Approximation::expansion_coefficient_flag() const
{ return this->expansionCoeffFlag; }


inline void C3Approximation::expansion_gradient_flag(bool grad_flag)
{ this->expansionCoeffGradFlag = grad_flag; }


inline bool C3Approximation::expansion_gradient_flag() const
{ return this->expansionCoeffGradFlag; }


inline const RealVector& C3Approximation::expansion_moments() const
{ return moment_vector; } // populated


inline const RealVector& C3Approximation::numerical_integration_moments() const
{ return num_moment_vector; } // empty


// Next two. Should access through compute_all_sobol_indices()
// Just need these two because NonDExpansion insists on computing
// all the analysis in one huge function (compute_analytic_statistcs)
// instead of smaller ones for different analysis.
// Need these two functions to reuse that code with no errors
inline void C3Approximation::compute_component_effects()
{ } // C3 does not distinguish so defer to fn below


inline void C3Approximation::compute_total_effects()
{
  // *** TO DO: mirror expConfigOptions.vbdOrderLimit in Pecos

  size_t interaction_order =//(vbdOrderLimit) ? vbdOrderLimit :
    sharedDataRep->numVars;
  compute_all_sobol_indices(interaction_order); 
}    
    
    
} // end namespace

#endif
