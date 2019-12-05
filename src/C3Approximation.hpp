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


class C3FnTrainPtrs
{
public:

  //
  //- Heading: Constructor and destructor
  //

  C3FnTrainPtrs();  ///< default constructor

  ~C3FnTrainPtrs(); ///< destructor

  //
  //- Heading: Member functions
  //

  void free_ft();

  void ft_derived_functions_init_null();

  // pass in sharedC3DataRep->approxOpts
  void ft_derived_functions_create(struct MultiApproxOpts * opts);

  void ft_derived_functions_free();

  //
  //- Heading: Data
  //

  struct FunctionTrain * ft;
  struct FT1DArray * ft_gradient;
  struct FT1DArray * ft_hessian;
  struct FTDerivedFunctions ft_derived_fns;
  struct C3SobolSensitivity * ft_sobol;
};


inline C3FnTrainPtrs::C3FnTrainPtrs():
  ft(NULL), ft_gradient(NULL), ft_hessian(NULL), ft_sobol(NULL)
{ ft_derived_functions_init_null(); }


inline void C3FnTrainPtrs::free_ft()
{
  if (ft)          function_train_free(ft);
  if (ft_gradient) ft1d_array_free(ft_gradient);
  if (ft_hessian)  ft1d_array_free(ft_hessian);

  ft = NULL;  ft_gradient = NULL;  ft_hessian = NULL;
}


inline C3FnTrainPtrs::~C3FnTrainPtrs()
{
  free_ft();

  ft_derived_functions_free();
  if (ft_sobol)  c3_sobol_sensitivity_free(ft_sobol);
  ft_sobol = NULL;
}


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

  void active_model_key(const UShortArray& key);
  void clear_model_keys();

  Real                 value(const Variables& vars);
  const RealVector&    gradient(const Variables& vars);
  const RealSymMatrix& hessian(const Variables& vars);

  void build();
  //void rebuild();
  //void finalize();

  bool expansion_coefficient;
  bool expansion_gradient;
  int min_coefficients() const;

  //SharedC3ApproxData* sharedC3DataRep;

private:

  void base_init();
    
  bool expansionCoeffFlag;     // build a fn train for the QoI
  bool expansionCoeffGradFlag; // build a fn train for the gradient of the QoI
    
  //
  //- Heading: Convenience member functions
  //

  void compute_derived_statistics(bool overwrite);
  struct FunctionTrain * subtract_const(Real val);

  //
  //- Heading: Data
  //

  // containers allowing const ref return of latest result (active key)
  RealVector expansionMoments;
  RealVector numericalMoments;

  /// set of pointers to QoI approximation data for each model key
  std::map<UShortArray, C3FnTrainPtrs> levelApprox;
  /// iterator to active levelApprox
  std::map<UShortArray, C3FnTrainPtrs>::iterator levApproxIter;
};


inline void C3Approximation::active_model_key(const UShortArray& key)
{
  // Test for change
  if (levApproxIter != levelApprox.end() && levApproxIter->first == key)
    return;

  levApproxIter = levelApprox.find(key);
  if (levApproxIter == levelApprox.end()) {
    // Note: C3FT pointers not allocated until build()
    std::pair<UShortArray, C3FnTrainPtrs> ftp_pair(key, C3FnTrainPtrs());
    levApproxIter = levelApprox.insert(ftp_pair).first;
  }

  // sets approxData keys
  Approximation::active_model_key(key);
}


inline void C3Approximation::expansion_coefficient_flag(bool coeff_flag)
{ expansionCoeffFlag = coeff_flag; }


inline bool C3Approximation::expansion_coefficient_flag() const
{ return expansionCoeffFlag; }


inline void C3Approximation::expansion_gradient_flag(bool grad_flag)
{ expansionCoeffGradFlag = grad_flag; }


inline bool C3Approximation::expansion_gradient_flag() const
{ return expansionCoeffGradFlag; }


inline const RealVector& C3Approximation::moments() const
{ return expansionMoments; }


inline Real C3Approximation::moment(size_t i) const
{ return expansionMoments[i]; }


inline void C3Approximation::moment(Real mom, size_t i)
{ expansionMoments[i] = mom; }


inline const RealVector& C3Approximation::expansion_moments() const
{ return expansionMoments; } // populated


inline const RealVector& C3Approximation::numerical_integration_moments() const
{ return numericalMoments; } // empty


inline Real C3Approximation::third_central()
{
  compute_derived_statistics(false);
  return levApproxIter->second.ft_derived_fns.third_central_moment;
}


inline Real C3Approximation::fourth_central()
{
  compute_derived_statistics(false);
  return levApproxIter->second.ft_derived_fns.fourth_central_moment;
}


inline Real C3Approximation::skewness()
{
  compute_derived_statistics(false);
  return levApproxIter->second.ft_derived_fns.skewness;
}


inline Real C3Approximation::kurtosis()
{
  compute_derived_statistics(false);
  return levApproxIter->second.ft_derived_fns.kurtosis;
}


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


inline Real C3Approximation::main_sobol_index(size_t dim)
{ return c3_sobol_sensitivity_get_main(levApproxIter->second.ft_sobol,dim); }


inline Real C3Approximation::total_sobol_index(size_t dim)
{ return c3_sobol_sensitivity_get_total(levApproxIter->second.ft_sobol,dim); }


inline void C3Approximation::
sobol_iterate_apply(void (*f)(double val, size_t ninteract,
			      size_t*interactions,void* arg), void* args)
{ c3_sobol_sensitivity_apply_external(levApproxIter->second.ft_sobol,f,args); }

} // end namespace

#endif
