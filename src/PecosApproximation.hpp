/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        PecosApproximation
//- Description:  Base Class for Pecos polynomial approximations
//-               
//- Owner:        Mike Eldred

#ifndef PECOS_APPROXIMATION_H
#define PECOS_APPROXIMATION_H

#include "DakotaApproximation.hpp"
#include "DakotaVariables.hpp"
#include "RegressOrthogPolyApproximation.hpp"

namespace Dakota {

class SharedApproxData;


/// Derived approximation class for global basis polynomials.

/** The PecosApproximation class provides a global approximation
    based on basis polynomials.  This includes orthogonal polynomials
    used for polynomial chaos expansions and interpolation polynomials
    used for stochastic collocation. */

class PecosApproximation: public Approximation
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  PecosApproximation();
  /// standard ProblemDescDB-driven constructor
  PecosApproximation(ProblemDescDB& problem_db,
		     const SharedApproxData& shared_data,
                     const String& approx_label);
  /// alternate constructor
  PecosApproximation(const SharedApproxData& shared_data);
  /// destructor
  ~PecosApproximation();

  //
  //- Heading: Member functions
  //

  /// set pecosBasisApprox.configOptions.expansionCoeffFlag
  void expansion_coefficient_flag(bool coeff_flag);
  /// get pecosBasisApprox.configOptions.expansionCoeffFlag
  bool expansion_coefficient_flag() const;

  /// set pecosBasisApprox.configOptions.expansionGradFlag
  void expansion_gradient_flag(bool grad_flag);
  /// get pecosBasisApprox.configOptions.expansionGradFlag
  bool expansion_gradient_flag() const;

  /// Performs global sensitivity analysis using Sobol' Indices by
  /// computing component (main and interaction) effects
  void compute_component_effects();
  /// Performs global sensitivity analysis using Sobol' Indices by
  /// computing total effects
  void compute_total_effects();

  /// return polyApproxRep->sobolIndices
  const Pecos::RealVector& sobol_indices() const;
  /// return polyApproxRep->totalSobolIndices
  const Pecos::RealVector& total_sobol_indices() const;
  /// return RegressOrthogPolyApproximation::sparseSobolIndexMap
  Pecos::ULongULongMap sparse_sobol_index_map() const;

  /// return OrthogPolyApproximation::decayRates
  const Pecos::RealVector& dimension_decay_rates() const;

  /// invoke Pecos::PolynomialApproximation::allocate_arrays()
  void allocate_arrays();

  /// return the mean of the expansion, treating all variables as random
  Real mean();
  /// return the mean of the expansion for a given parameter vector,
  /// treating a subset of the variables as random
  Real mean(const Pecos::RealVector& x);
  /// return the gradient of the expansion mean for a given parameter
  /// vector, treating all variables as random
  const Pecos::RealVector& mean_gradient();
  /// return the gradient of the expansion mean for a given parameter vector
  /// and given DVV, treating a subset of the variables as random
  const Pecos::RealVector& mean_gradient(const Pecos::RealVector& x,
					 const Pecos::SizetArray& dvv);

  /// return the variance of the expansion, treating all variables as random
  Real variance();
  /// return the variance of the expansion for a given parameter vector,
  /// treating a subset of the variables as random
  Real variance(const Pecos::RealVector& x);
  /// return the gradient of the expansion variance for a given parameter
  /// vector, treating all variables as random
  const Pecos::RealVector& variance_gradient();
  /// return the gradient of the expansion variance for a given parameter
  /// vector and given DVV, treating a subset of the variables as random
  const Pecos::RealVector& variance_gradient(const Pecos::RealVector& x,
					     const Pecos::SizetArray& dvv);

  /// return the covariance between two response expansions, treating
  /// all variables as random
  Real covariance(PecosApproximation* pecos_approx_2);
  /// return the covariance between two response expansions, treating
  /// a subset of the variables as random
  Real covariance(const Pecos::RealVector& x,
		  PecosApproximation* pecos_approx_2);

  /// return the change in covariance between two response expansions,
  /// treating all variables as random
  Real delta_covariance(PecosApproximation* pecos_approx_2);
  /// return the change in covariance between two response expansions,
  /// treating a subset of the variables as random
  Real delta_covariance(const Pecos::RealVector& x,
			PecosApproximation* pecos_approx_2);

  /// return the change in mean between two response expansions,
  /// treating all variables as random
  Real delta_mean();
  /// return the change in mean between two response expansions,
  /// treating a subset of variables as random
  Real delta_mean(const RealVector& x);
  /// return the change in standard deviation between two response
  /// expansions, treating all variables as random
  Real delta_std_deviation();
  /// return the change in standard deviation between two response
  /// expansions, treating a subset of variables as random
  Real delta_std_deviation(const RealVector& x);
  /// return the change in reliability index (mapped from z_bar) between
  /// two response expansions, treating all variables as random
  Real delta_beta(bool cdf_flag, Real z_bar);
  /// return the change in reliability index (mapped from z_bar) between
  /// two response expansions, treating a subset of variables as random
  Real delta_beta(const RealVector& x, bool cdf_flag, Real z_bar);
  /// return the change in response level (mapped from beta_bar) between
  /// two response expansions, treating all variables as random
  Real delta_z(bool cdf_flag, Real beta_bar);
  /// return the change in response level (mapped from beta_bar) between
  /// two response expansions, treating a subset of the variables as random
  Real delta_z(const RealVector& x, bool cdf_flag, Real beta_bar);

  /// compute moments up to the order supported by the Pecos
  /// polynomial approximation
  void compute_moments();
  /// compute moments in all-variables mode up to the order supported
  /// by the Pecos polynomial approximation
  void compute_moments(const Pecos::RealVector& x);
  /// return virtual Pecos::PolynomialApproximation::moments()
  const RealVector& moments() const;
  /// return Pecos::PolynomialApproximation::expansionMoments
  const RealVector& expansion_moments() const;
  /// return Pecos::PolynomialApproximation::numericalMoments
  const RealVector& numerical_integration_moments() const;
  /// standardize the central moments returned from Pecos
  void standardize_moments(const Pecos::RealVector& central_moments,
			   Pecos::RealVector& std_moments);

  /// construct the Vandermonde matrix "A" for PCE regression for Ax = b
  void build_linear_system(RealMatrix& A, const UShort2DArray& multi_index);
  // add chain (allSamples): A size = num current+num chain by P,
  // with current pts as 1st rows 
  void augment_linear_system(const RealVectorArray& samples, RealMatrix& A,
			     const UShort2DArray& multi_index);

  // get the surrData instance
  const Pecos::SurrogateData& surrogate_data() const;

  /// return pecosBasisApprox
  Pecos::BasisApproximation& pecos_basis_approximation();

protected:

  //
  //- Heading: Virtual function redefinitions
  //
  
  // redocumenting these since they use Pecos:: qualification

  /// retrieve the approximate function value for a given parameter vector
  Real                        value(const Variables& vars);
  /// retrieve the approximate function gradient for a given parameter vector
  const Pecos::RealVector&    gradient(const Variables& vars);
  /// retrieve the approximate function Hessian for a given parameter vector
  const Pecos::RealSymMatrix& hessian(const Variables& vars);

  int min_coefficients() const;
  //int num_constraints() const; // use default implementation

  void build();
  void rebuild();
  void pop(bool save_data);
  void push();
  void finalize();
  void store(size_t index = _NPOS);
  void restore(size_t index = _NPOS);
  void remove_stored(size_t index = _NPOS);
  void combine(short corr_type, size_t swap_index);

  void print_coefficients(std::ostream& s, bool normalized);

  /// return expansion coefficients in a form consistent with the
  /// shared multi-index
  RealVector approximation_coefficients(bool normalized) const;
  /// set expansion coefficients in a form consistent with the shared
  /// multi-index
  void approximation_coefficients(const RealVector& approx_coeffs,
				  bool normalized);

  void coefficient_labels(std::vector<std::string>& coeff_labels) const;

  //
  //- Heading: Data
  //

private:

  //
  //- Heading: Convenience member functions
  //

  /// utility to convert Dakota type string to Pecos type enumeration
  void approx_type_to_basis_type(const String& approx_type, short& basis_type);

  //
  //- Heading: Data
  //

  /// the Pecos basis approximation, encompassing OrthogPolyApproximation
  /// and InterpPolyApproximation
  Pecos::BasisApproximation pecosBasisApprox;
  /// convenience pointer to representation of Pecos polynomial approximation
  Pecos::PolynomialApproximation* polyApproxRep;

  // convenience pointer to shared data representation
  //SharedPecosApproxData* sharedPecosDataRep;
};


inline PecosApproximation::PecosApproximation():
  polyApproxRep(NULL) //, sharedPecosDataRep(NULL)
{ }


inline PecosApproximation::~PecosApproximation()
{ }


inline void PecosApproximation::expansion_coefficient_flag(bool coeff_flag)
{ polyApproxRep->expansion_coefficient_flag(coeff_flag); }


inline bool PecosApproximation::expansion_coefficient_flag() const
{ return polyApproxRep->expansion_coefficient_flag(); }


inline void PecosApproximation::expansion_gradient_flag(bool grad_flag)
{ polyApproxRep->expansion_coefficient_gradient_flag(grad_flag); }


inline bool PecosApproximation::expansion_gradient_flag() const
{ return polyApproxRep->expansion_coefficient_gradient_flag(); }


inline void PecosApproximation::compute_component_effects()
{ polyApproxRep->compute_component_sobol(); }


inline void PecosApproximation::compute_total_effects()
{ polyApproxRep->compute_total_sobol(); }


inline const Pecos::RealVector& PecosApproximation::sobol_indices() const
{ return polyApproxRep->sobol_indices(); }


inline const Pecos::RealVector& PecosApproximation::total_sobol_indices() const
{ return polyApproxRep->total_sobol_indices(); }


inline Pecos::ULongULongMap PecosApproximation::sparse_sobol_index_map() const
{ return polyApproxRep->sparse_sobol_index_map(); }


inline const Pecos::RealVector& PecosApproximation::
dimension_decay_rates() const
{
  return ((Pecos::OrthogPolyApproximation*)polyApproxRep)->
    dimension_decay_rates();
}


inline void PecosApproximation::allocate_arrays()
{ polyApproxRep->allocate_arrays(); }


inline Real PecosApproximation::mean()
{ return polyApproxRep->mean(); }


inline Real PecosApproximation::mean(const Pecos::RealVector& x)
{ return polyApproxRep->mean(x); }


inline const Pecos::RealVector& PecosApproximation::mean_gradient()
{ return polyApproxRep->mean_gradient(); }


inline const Pecos::RealVector& PecosApproximation::
mean_gradient(const Pecos::RealVector& x, const Pecos::SizetArray& dvv)
{ return polyApproxRep->mean_gradient(x, dvv); }


inline Real PecosApproximation::variance()
{ return polyApproxRep->variance(); }


inline Real PecosApproximation::variance(const Pecos::RealVector& x)
{ return polyApproxRep->variance(x); }


inline const Pecos::RealVector& PecosApproximation::variance_gradient()
{ return polyApproxRep->variance_gradient(); }


inline const Pecos::RealVector& PecosApproximation::
variance_gradient(const Pecos::RealVector& x, const Pecos::SizetArray& dvv)
{ return polyApproxRep->variance_gradient(x, dvv); }


inline Real PecosApproximation::
covariance(PecosApproximation* pecos_approx_2)
{ return polyApproxRep->covariance(pecos_approx_2->polyApproxRep); }


inline Real PecosApproximation::
covariance(const Pecos::RealVector& x, PecosApproximation* pecos_approx_2)
{ return polyApproxRep->covariance(x, pecos_approx_2->polyApproxRep); }


inline Real PecosApproximation::
delta_covariance(PecosApproximation* pecos_approx_2)
{ return polyApproxRep->delta_covariance(pecos_approx_2->polyApproxRep); }


inline Real PecosApproximation::
delta_covariance(const Pecos::RealVector& x, PecosApproximation* pecos_approx_2)
{ return polyApproxRep->delta_covariance(x, pecos_approx_2->polyApproxRep); }


inline Real PecosApproximation::delta_mean()
{ return polyApproxRep->delta_mean(); }


inline Real PecosApproximation::delta_mean(const RealVector& x)
{ return polyApproxRep->delta_mean(x); }


inline Real PecosApproximation::delta_std_deviation()
{ return polyApproxRep->delta_std_deviation(); }


inline Real PecosApproximation::delta_std_deviation(const RealVector& x)
{ return polyApproxRep->delta_std_deviation(x); }


inline Real PecosApproximation::delta_beta(bool cdf_flag, Real z_bar)
{ return polyApproxRep->delta_beta(cdf_flag, z_bar); }


inline Real PecosApproximation::
delta_beta(const RealVector& x, bool cdf_flag, Real z_bar)
{ return polyApproxRep->delta_beta(x, cdf_flag, z_bar); }


inline Real PecosApproximation::delta_z(bool cdf_flag, Real beta_bar)
{ return polyApproxRep->delta_z(cdf_flag, beta_bar); }


inline Real PecosApproximation::
delta_z(const RealVector& x, bool cdf_flag, Real beta_bar)
{ return polyApproxRep->delta_z(x, cdf_flag, beta_bar); }


inline void PecosApproximation::compute_moments()
{ polyApproxRep->compute_moments(); }


inline void PecosApproximation::compute_moments(const Pecos::RealVector& x)
{ polyApproxRep->compute_moments(x); }


inline const RealVector& PecosApproximation::moments() const
{ return polyApproxRep->moments(); }


inline const RealVector& PecosApproximation::expansion_moments() const
{ return polyApproxRep->expansion_moments(); }


inline const RealVector& PecosApproximation::
numerical_integration_moments() const
{ return polyApproxRep->numerical_integration_moments(); }


inline void PecosApproximation::
standardize_moments(const Pecos::RealVector& central_moments,
		    Pecos::RealVector& std_moments)
{ polyApproxRep->standardize_moments(central_moments, std_moments); }


inline void PecosApproximation::
build_linear_system(RealMatrix& A, const UShort2DArray& multi_index)
{
  ((Pecos::RegressOrthogPolyApproximation*)polyApproxRep)->
    build_linear_system(A, multi_index);
}


inline void PecosApproximation::
augment_linear_system(const RealVectorArray& samples, RealMatrix& A,
		      const UShort2DArray& multi_index)
{
  ((Pecos::RegressOrthogPolyApproximation*)polyApproxRep)->
    augment_linear_system(samples, A, multi_index);
}


inline const Pecos::SurrogateData& PecosApproximation::surrogate_data() const
{
  return ((Pecos::PolynomialApproximation*)polyApproxRep)->surrogate_data();
}


inline Pecos::BasisApproximation& PecosApproximation::
pecos_basis_approximation()
{ return pecosBasisApprox; }


// ignore discrete variables for now
inline Real PecosApproximation::value(const Variables& vars)
{ return pecosBasisApprox.value(vars.continuous_variables()); }


// ignore discrete variables for now
inline const Pecos::RealVector& PecosApproximation::
gradient(const Variables& vars)
{
  //return pecosBasisApprox.gradient(x); // bypass this layer
  return polyApproxRep->gradient_basis_variables(vars.continuous_variables());
}


// ignore discrete variables for now
inline const Pecos::RealSymMatrix& PecosApproximation::
hessian(const Variables& vars)
{
  //return pecosBasisApprox.hessian(vars.continuous_variables()); // bypass
  return polyApproxRep->hessian_basis_variables(vars.continuous_variables());
}


inline int PecosApproximation::min_coefficients() const
{ return pecosBasisApprox.min_coefficients(); }


inline void PecosApproximation::build()
{
  // base class implementation checks data set against min required
  Approximation::build();
  // map to Pecos::BasisApproximation
  pecosBasisApprox.compute_coefficients();
}


inline void PecosApproximation::rebuild()
{
  // base class default invokes build() for derived Approximations
  // that do not supply rebuild()
  //Approximation::rebuild();

  // TO DO: increment_coefficients() below covers current usage of
  // append_approximation() in NonDExpansion.  For more general
  // support of both update and append, need a mechanism to detect
  // the +/- direction of discrepancy between data and coefficients.

  //size_t curr_pts  = approxData.points(),
  //  curr_pecos_pts = polyApproxRep->data_size();
  //if (curr_pts > curr_pecos_pts)
    pecosBasisApprox.increment_coefficients();
  //else if (curr_pts < curr_pecos_pts)
    //pecosBasisApprox.decrement_coefficients();
  // else, if number of points is consistent, leave as is
}


inline void PecosApproximation::pop(bool save_data)
{
  // base class implementation removes data from currentPoints
  Approximation::pop(save_data);
  // map to Pecos::BasisApproximation
  pecosBasisApprox.decrement_coefficients();
}


inline void PecosApproximation::push()
{
  // base class implementation updates currentPoints
  Approximation::push();
  // map to Pecos::BasisApproximation
  pecosBasisApprox.push_coefficients();
}


inline void PecosApproximation::finalize()
{
  // base class implementation appends currentPoints with popped data sets
  Approximation::finalize();
  // map to Pecos::BasisApproximation
  pecosBasisApprox.finalize_coefficients();
}


inline void PecosApproximation::store(size_t index)
{
  // base class implementation manages approx data
  Approximation::store(index);
  // map to Pecos::BasisApproximation
  pecosBasisApprox.store_coefficients(index);
}


inline void PecosApproximation::restore(size_t index)
{
  // base class implementation manages approx data
  Approximation::restore(index);
  // map to Pecos::BasisApproximation
  pecosBasisApprox.restore_coefficients(index);
}


inline void PecosApproximation::remove_stored(size_t index)
{
  // base class implementation manages approx data
  Approximation::remove_stored(index);
  // map to Pecos::BasisApproximation
  pecosBasisApprox.remove_stored_coefficients(index);
}


inline void PecosApproximation::combine(short corr_type, size_t swap_index)
{
  // base class implementation manages approxData state
  //Approximation::combine(corr_type);
  if (swap_index != _NPOS) approxData.swap(swap_index);

  // map to Pecos::BasisApproximation.  Note: DAKOTA correction and
  // PECOS combination type enumerations coincide.
  pecosBasisApprox.combine_coefficients(corr_type, swap_index);
}


inline void PecosApproximation::
print_coefficients(std::ostream& s, bool normalized)
{ pecosBasisApprox.print_coefficients(s, normalized); }


inline RealVector PecosApproximation::
approximation_coefficients(bool normalized) const
{ return pecosBasisApprox.approximation_coefficients(normalized); }


inline void PecosApproximation::
approximation_coefficients(const RealVector& approx_coeffs, bool normalized)
{ pecosBasisApprox.approximation_coefficients(approx_coeffs, normalized); }


inline void PecosApproximation::
coefficient_labels(std::vector<std::string>& coeff_labels) const
{ polyApproxRep->coefficient_labels(coeff_labels); }

} // namespace Dakota

#endif
