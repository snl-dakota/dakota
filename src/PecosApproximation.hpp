/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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

  /// clear unused Sobol' indices
  void clear_component_effects();
  /// Performs global sensitivity analysis using Sobol' indices by
  /// computing component (main and interaction) effects
  void compute_component_effects();
  /// Performs global sensitivity analysis using Sobol' indices by
  /// computing total effects
  void compute_total_effects();

  /// return polyApproxRep->sobolIndices
  const Pecos::RealVector& sobol_indices() const;
  /// return polyApproxRep->totalSobolIndices
  const Pecos::RealVector& total_sobol_indices() const;

  /// return the number of non-zero coefficients for this QoI
  size_t sparsity() const;
  /// return RegressOrthogPolyApproximation::sparseSobolIndexMap
  Pecos::ULongULongMap sparse_sobol_index_map() const;

  /// return OrthogPolyApproximation::decayRates
  const Pecos::RealVector& dimension_decay_rates() const;

  /// invoke Pecos::PolynomialApproximation::allocate_arrays()
  void allocate_arrays();

  /// initialize covariance accumulators with pointers to other QoI
  void initialize_covariance(Approximation& approx_2);
  /// clear covariance pointers to other QoI
  void clear_covariance_pointers();
  /// initialize covariance accumulators (also reinitialize after change
  /// in stats type)
  void initialize_products();
  /// query whether product interpolants are defined (non-empty)
  bool product_interpolants();

  /// return the mean of the expansion, where all active variables are random
  Real mean();
  /// return the mean of the expansion for a given parameter vector,
  /// where a subset of the active variables are random
  Real mean(const Pecos::RealVector& x);
  /// return the mean of the combined expansion, treating all variables
  /// as random
  Real combined_mean();
  /// return the mean of the combined expansion for a given parameter vector,
  /// where a subset of the active variables are treated as random
  Real combined_mean(const Pecos::RealVector& x);
  /// return the gradient of the expansion mean for a given parameter
  /// vector, where all active variables are random
  const Pecos::RealVector& mean_gradient();
  /// return the gradient of the expansion mean for a given parameter vector
  /// and given DVV, where a subset of the active variables are random
  const Pecos::RealVector& mean_gradient(const Pecos::RealVector& x,
					 const Pecos::SizetArray& dvv);

  /// return the variance of the expansion, where all active vars are random
  Real variance();
  /// return the variance of the expansion for a given parameter vector,
  /// where a subset of the active variables are random
  Real variance(const Pecos::RealVector& x);
  /// return the gradient of the expansion variance for a given parameter
  /// vector, where all active variables are random
  const Pecos::RealVector& variance_gradient();
  /// return the gradient of the expansion variance for a given parameter
  /// vector and given DVV, where a subset of the active variables are random
  const Pecos::RealVector& variance_gradient(const Pecos::RealVector& x,
					     const Pecos::SizetArray& dvv);

  /// return the covariance between two response expansions, treating
  /// all variables as random
  Real covariance(Approximation& approx_2);
  /// return the covariance between two response expansions, treating
  /// a subset of the variables as random
  Real covariance(const Pecos::RealVector& x, Approximation& approx_2);
  /// return the covariance between two combined response expansions,
  /// where all active variables are random
  Real combined_covariance(Approximation& approx_2);
  /// return the covariance between two combined response expansions,
  /// where a subset of the active variables are random
  Real combined_covariance(const Pecos::RealVector& x,
			   Approximation& approx_2);

  /// return the reliability index (mapped from z_bar), where all active
  /// variables are random
  Real beta(bool cdf_flag, Real z_bar);
  /// return the reliability index (mapped from z_bar), treating a subset of
  /// variables as random
  Real beta(const RealVector& x, bool cdf_flag, Real z_bar);
  /// return the reliability index (mapped from z_bar), where all active
  /// variables are random
  Real combined_beta(bool cdf_flag, Real z_bar);
  /// return the reliability index (mapped from z_bar), treating a subset of
  /// variables as random
  Real combined_beta(const RealVector& x, bool cdf_flag, Real z_bar);

  /// return the change in mean resulting from expansion refinement,
  /// where all active variables are random
  Real delta_mean();
  /// return the change in mean resulting from expansion refinement,
  /// treating a subset of variables as random
  Real delta_mean(const RealVector& x);
  /// return the change in mean resulting from combined expansion refinement,
  /// where all active variables are random
  Real delta_combined_mean();
  /// return the change in mean resulting from combined expansion refinement,
  /// treating a subset of variables as random
  Real delta_combined_mean(const RealVector& x);

  /// return the change in standard deviation resulting from expansion
  /// refinement, where all active variables are random
  Real delta_std_deviation();
  /// return the change in standard deviation resulting from expansion
  /// refinement, treating a subset of variables as random
  Real delta_std_deviation(const RealVector& x);
  /// return the change in standard deviation resulting from combined
  /// expansion refinement, where all active variables are random
  Real delta_combined_std_deviation();
  /// return the change in standard deviation resulting from combined
  /// expansion refinement, treating a subset of variables as random
  Real delta_combined_std_deviation(const RealVector& x);

  /// return the change in variance resulting from expansion
  /// refinement, where all active variables are random
  Real delta_variance();
  /// return the change in variance resulting from expansion
  /// refinement, treating a subset of variables as random
  Real delta_variance(const RealVector& x);
  /// return the change in variance resulting from combined
  /// expansion refinement, where all active variables are random
  Real delta_combined_variance();
  /// return the change in variance resulting from combined
  /// expansion refinement, treating a subset of variables as random
  Real delta_combined_variance(const RealVector& x);

  /// return the change in covariance resulting from expansion refinement,
  /// where all active variables are random
  Real delta_covariance(Approximation& approx_2);
  /// return the change in covariance resulting from expansion refinement,
  /// where a subset of the active variables are random
  Real delta_covariance(const Pecos::RealVector& x, Approximation& approx_2);
  /// return the change in covariance resulting from expansion refinement,
  /// where all active variables are random
  Real delta_combined_covariance(Approximation& approx_2);
  /// return the change in covariance resulting from expansion refinement,
  /// where a subset of the active variables are random
  Real delta_combined_covariance(const Pecos::RealVector& x,
				 Approximation& approx_2);

  /// return the change in reliability index (mapped from z_bar) resulting
  /// from expansion refinement, where all active variables are random
  Real delta_beta(bool cdf_flag, Real z_bar);
  /// return the change in reliability index (mapped from z_bar) resulting
  /// from expansion refinement, treating a subset of variables as random
  Real delta_beta(const RealVector& x, bool cdf_flag, Real z_bar);
  /// return the change in reliability index (mapped from z_bar) resulting
  /// from expansion refinement, where all active variables are random
  Real delta_combined_beta(bool cdf_flag, Real z_bar);
  /// return the change in reliability index (mapped from z_bar) resulting
  /// from expansion refinement, treating a subset of variables as random
  Real delta_combined_beta(const RealVector& x, bool cdf_flag, Real z_bar);

  /// return the change in response level (mapped from beta_bar) resulting
  /// from expansion refinement, where all active variables are random
  Real delta_z(bool cdf_flag, Real beta_bar);
  /// return the change in response level (mapped from beta_bar) resulting from
  /// expansion refinement, where a subset of the active variables are random
  Real delta_z(const RealVector& x, bool cdf_flag, Real beta_bar);
  /// return the change in response level (mapped from beta_bar) resulting
  /// from expansion refinement, where all active variables are random
  Real delta_combined_z(bool cdf_flag, Real beta_bar);
  /// return the change in response level (mapped from beta_bar) resulting from
  /// expansion refinement, where a subset of the active variables are random
  Real delta_combined_z(const RealVector& x, bool cdf_flag, Real beta_bar);

  /// compute moments up to the order supported by the Pecos
  /// polynomial approximation
  void compute_moments(bool full_stats = true, bool combined_stats = false);
  /// compute moments in all-variables mode up to the order supported
  /// by the Pecos polynomial approximation
  void compute_moments(const Pecos::RealVector& x, bool full_stats = true,
		       bool combined_stats = false);

  /// return primary moments using Pecos::PolynomialApproximation::moments()
  const RealVector& moments() const;
  /// return expansion moments from Pecos::PolynomialApproximation
  const RealVector& expansion_moments() const;
  /// return numerical moments from Pecos::PolynomialApproximation
  const RealVector& numerical_integration_moments() const;
  /// return combined moments from multilevel-muktifidelity expansion roll-up
  const RealVector& combined_moments() const;

  /// return primary moment using Pecos::PolynomialApproximation::moment(i)
  Real moment(size_t i) const;
  /// set primary moment using Pecos::PolynomialApproximation::moment(i)
  void moment(Real mom, size_t i);
  /// return Pecos::PolynomialApproximation::combinedMoments[i]
  Real combined_moment(size_t i) const;
  /// set Pecos::PolynomialApproximation::combinedMoments[i]
  void combined_moment(Real mom, size_t i);

  /// clear tracking of computed moments, due to a change that invalidates
  /// previous results
  void clear_computed_bits();

  /// construct the Vandermonde matrix "A" for PCE regression for Ax = b
  void build_linear_system(RealMatrix& A, const UShort2DArray& multi_index);
  // add chain (allSamples): A size = num current+num chain by P,
  // with current pts as 1st rows 
  void augment_linear_system(const RealVectorArray& samples, RealMatrix& A,
			     const UShort2DArray& multi_index);

  /// return pecosBasisApprox
  Pecos::BasisApproximation& pecos_basis_approximation();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  // redocumenting these since they use Pecos:: qualification

  /// assign active key in approxData and update_active_iterators()
  void active_model_key(const Pecos::ActiveKey& key);

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
  void pop_coefficients(bool save_data);
  void push_coefficients();
  void finalize_coefficients();
  void combine_coefficients();
  void combined_to_active_coefficients(bool clear_combined = true);
  void clear_inactive_coefficients();

  bool advancement_available();

  void print_coefficients(std::ostream& s, bool normalized);

  /// return expansion coefficients in a form consistent with the
  /// shared multi-index
  RealVector approximation_coefficients(bool normalized) const;
  /// set expansion coefficients in a form consistent with the shared
  /// multi-index
  void approximation_coefficients(const RealVector& approx_coeffs,
				  bool normalized);

  //void link_multilevel_surrogate_data();

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

  /// the Pecos basis approximation, encompassing orthogonal and interpolation
  /// polynomial approximations
  Pecos::BasisApproximation pecosBasisApprox;
  /// convenience pointer to representation of Pecos polynomial approximation
  std::shared_ptr<Pecos::PolynomialApproximation> polyApproxRep;

  // convenience pointer to shared data representation
  //SharedPecosApproxData* sharedPecosDataRep;
};


inline PecosApproximation::PecosApproximation()
{ }


inline PecosApproximation::~PecosApproximation()
{ }


inline void PecosApproximation::active_model_key(const Pecos::ActiveKey& key)
{
  // sets approxData keys
  Approximation::active_model_key(key);

  // Almost all approximation operations can simply update active iterators
  // using the shared data active key when computing/updating coeffs, but a few
  // operations (NonDExpansion::reduce_{total_sobol,decay_rate}_sets()) access
  // a previous state prior to computing a new one, such that updating iterators
  // in allocate_arrays() is not early enough.  Therefore, ensure all iterators
  // are updated at initial key assignment.
  // *** TO DO: retire other redundant calls to update_active_iterators()
  polyApproxRep->update_active_iterators(key);
}


inline void PecosApproximation::expansion_coefficient_flag(bool coeff_flag)
{ polyApproxRep->expansion_coefficient_flag(coeff_flag); }


inline bool PecosApproximation::expansion_coefficient_flag() const
{ return polyApproxRep->expansion_coefficient_flag(); }


inline void PecosApproximation::expansion_gradient_flag(bool grad_flag)
{ polyApproxRep->expansion_coefficient_gradient_flag(grad_flag); }


inline bool PecosApproximation::expansion_gradient_flag() const
{ return polyApproxRep->expansion_coefficient_gradient_flag(); }


inline void PecosApproximation::clear_component_effects()
{ polyApproxRep->clear_component_sobol(); }


inline void PecosApproximation::compute_component_effects()
{ polyApproxRep->compute_component_sobol(); }


inline void PecosApproximation::compute_total_effects()
{ polyApproxRep->compute_total_sobol(); }


inline const Pecos::RealVector& PecosApproximation::sobol_indices() const
{ return polyApproxRep->sobol_indices(); }


inline const Pecos::RealVector& PecosApproximation::total_sobol_indices() const
{ return polyApproxRep->total_sobol_indices(); }


inline size_t PecosApproximation::sparsity() const
{ return polyApproxRep->expansion_terms(); }


inline Pecos::ULongULongMap PecosApproximation::sparse_sobol_index_map() const
{ return polyApproxRep->sparse_sobol_index_map(); }


inline const Pecos::RealVector& PecosApproximation::
dimension_decay_rates() const
{
  return std::static_pointer_cast<Pecos::OrthogPolyApproximation>
    (polyApproxRep)->dimension_decay_rates();
}


inline void PecosApproximation::allocate_arrays()
{ polyApproxRep->allocate_arrays(); }


inline void PecosApproximation::initialize_covariance(Approximation& approx_2)
{
  std::shared_ptr<PecosApproximation> pa_2 =
    std::static_pointer_cast<PecosApproximation>(approx_2.approx_rep());
  polyApproxRep->initialize_covariance(pa_2->polyApproxRep.get());
}


inline void PecosApproximation::clear_covariance_pointers()
{ polyApproxRep->clear_covariance_pointers(); }


inline void PecosApproximation::initialize_products()
{ polyApproxRep->initialize_products(); }


inline bool PecosApproximation::product_interpolants()
{ return polyApproxRep->product_interpolants(); }


inline Real PecosApproximation::mean()
{ return polyApproxRep->mean(); }


inline Real PecosApproximation::mean(const Pecos::RealVector& x)
{ return polyApproxRep->mean(x); }


inline Real PecosApproximation::combined_mean()
{ return polyApproxRep->combined_mean(); }


inline Real PecosApproximation::combined_mean(const Pecos::RealVector& x)
{ return polyApproxRep->combined_mean(x); }


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


inline Real PecosApproximation::covariance(Approximation& approx_2)
{
  std::shared_ptr<PecosApproximation> pa_2 =
    std::static_pointer_cast<PecosApproximation>(approx_2.approx_rep());
  return polyApproxRep->covariance(pa_2->polyApproxRep.get());
}


inline Real PecosApproximation::
covariance(const Pecos::RealVector& x, Approximation& approx_2)
{
  std::shared_ptr<PecosApproximation> pa_2 =
    std::static_pointer_cast<PecosApproximation>(approx_2.approx_rep());
  return polyApproxRep->covariance(x, pa_2->polyApproxRep.get());
}


inline Real PecosApproximation::combined_covariance(Approximation& approx_2)
{
  std::shared_ptr<PecosApproximation> pa_2 =
    std::static_pointer_cast<PecosApproximation>(approx_2.approx_rep());
  return polyApproxRep->combined_covariance(pa_2->polyApproxRep.get());
}


inline Real PecosApproximation::
combined_covariance(const Pecos::RealVector& x, Approximation& approx_2)
{
  std::shared_ptr<PecosApproximation> pa_2 =
    std::static_pointer_cast<PecosApproximation>(approx_2.approx_rep());
  return polyApproxRep->combined_covariance(x, pa_2->polyApproxRep.get());
}


inline Real PecosApproximation::beta(bool cdf_flag, Real z_bar)
{ return polyApproxRep->beta(cdf_flag, z_bar); }


inline Real PecosApproximation::
beta(const RealVector& x, bool cdf_flag, Real z_bar)
{ return polyApproxRep->beta(x, cdf_flag, z_bar); }


inline Real PecosApproximation::combined_beta(bool cdf_flag, Real z_bar)
{ return polyApproxRep->combined_beta(cdf_flag, z_bar); }


inline Real PecosApproximation::
combined_beta(const RealVector& x, bool cdf_flag, Real z_bar)
{ return polyApproxRep->combined_beta(x, cdf_flag, z_bar); }


inline Real PecosApproximation::delta_mean()
{ return polyApproxRep->delta_mean(); }


inline Real PecosApproximation::delta_mean(const RealVector& x)
{ return polyApproxRep->delta_mean(x); }


inline Real PecosApproximation::delta_combined_mean()
{ return polyApproxRep->delta_combined_mean(); }


inline Real PecosApproximation::delta_combined_mean(const RealVector& x)
{ return polyApproxRep->delta_combined_mean(x); }


inline Real PecosApproximation::delta_std_deviation()
{ return polyApproxRep->delta_std_deviation(); }


inline Real PecosApproximation::delta_std_deviation(const RealVector& x)
{ return polyApproxRep->delta_std_deviation(x); }


inline Real PecosApproximation::delta_combined_std_deviation()
{ return polyApproxRep->delta_combined_std_deviation(); }


inline Real PecosApproximation::
delta_combined_std_deviation(const RealVector& x)
{ return polyApproxRep->delta_combined_std_deviation(x); }


inline Real PecosApproximation::delta_variance()
{ return polyApproxRep->delta_variance(); }


inline Real PecosApproximation::delta_variance(const RealVector& x)
{ return polyApproxRep->delta_variance(x); }


inline Real PecosApproximation::delta_combined_variance()
{ return polyApproxRep->delta_combined_variance(); }


inline Real PecosApproximation::delta_combined_variance(const RealVector& x)
{ return polyApproxRep->delta_combined_variance(x); }


inline Real PecosApproximation::delta_covariance(Approximation& approx_2)
{
  std::shared_ptr<PecosApproximation> pa_2 =
    std::static_pointer_cast<PecosApproximation>(approx_2.approx_rep());
  return polyApproxRep->delta_covariance(pa_2->polyApproxRep.get());
}


inline Real PecosApproximation::
delta_covariance(const Pecos::RealVector& x, Approximation& approx_2)
{
  std::shared_ptr<PecosApproximation> pa_2 =
    std::static_pointer_cast<PecosApproximation>(approx_2.approx_rep());
  return polyApproxRep->delta_covariance(x, pa_2->polyApproxRep.get());
}


inline Real PecosApproximation::
delta_combined_covariance(Approximation& approx_2)
{
std::shared_ptr<PecosApproximation> pa_2 =
    std::static_pointer_cast<PecosApproximation>(approx_2.approx_rep());
 return polyApproxRep->delta_combined_covariance(pa_2->polyApproxRep.get());
}


inline Real PecosApproximation::
delta_combined_covariance(const Pecos::RealVector& x, Approximation& approx_2)
{
  std::shared_ptr<PecosApproximation> pa_2 =
    std::static_pointer_cast<PecosApproximation>(approx_2.approx_rep());
  return polyApproxRep->delta_combined_covariance(x, pa_2->polyApproxRep.get());
}


inline Real PecosApproximation::delta_beta(bool cdf_flag, Real z_bar)
{ return polyApproxRep->delta_beta(cdf_flag, z_bar); }


inline Real PecosApproximation::
delta_beta(const RealVector& x, bool cdf_flag, Real z_bar)
{ return polyApproxRep->delta_beta(x, cdf_flag, z_bar); }


inline Real PecosApproximation::delta_combined_beta(bool cdf_flag, Real z_bar)
{ return polyApproxRep->delta_combined_beta(cdf_flag, z_bar); }


inline Real PecosApproximation::
delta_combined_beta(const RealVector& x, bool cdf_flag, Real z_bar)
{ return polyApproxRep->delta_combined_beta(x, cdf_flag, z_bar); }


inline Real PecosApproximation::delta_z(bool cdf_flag, Real beta_bar)
{ return polyApproxRep->delta_z(cdf_flag, beta_bar); }


inline Real PecosApproximation::
delta_z(const RealVector& x, bool cdf_flag, Real beta_bar)
{ return polyApproxRep->delta_z(x, cdf_flag, beta_bar); }


inline Real PecosApproximation::delta_combined_z(bool cdf_flag, Real beta_bar)
{ return polyApproxRep->delta_combined_z(cdf_flag, beta_bar); }


inline Real PecosApproximation::
delta_combined_z(const RealVector& x, bool cdf_flag, Real beta_bar)
{ return polyApproxRep->delta_combined_z(x, cdf_flag, beta_bar); }


inline void PecosApproximation::
compute_moments(bool full_stats, bool combined_stats )
{ polyApproxRep->compute_moments(full_stats, combined_stats); }


inline void PecosApproximation::
compute_moments(const Pecos::RealVector& x, bool full_stats,
		bool combined_stats)
{ polyApproxRep->compute_moments(x, full_stats, combined_stats); }


inline const RealVector& PecosApproximation::moments() const
{ return polyApproxRep->moments(); }


inline const RealVector& PecosApproximation::expansion_moments() const
{ return polyApproxRep->expansion_moments(); }


inline const RealVector& PecosApproximation::
numerical_integration_moments() const
{ return polyApproxRep->numerical_integration_moments(); }


inline const RealVector& PecosApproximation::combined_moments() const
{ return polyApproxRep->combined_moments(); }


inline Real PecosApproximation::moment(size_t i) const
{ return polyApproxRep->moment(i); }


inline void PecosApproximation::moment(Real mom, size_t i)
{ polyApproxRep->moment(mom, i); }


inline Real PecosApproximation::combined_moment(size_t i) const
{ return polyApproxRep->combined_moment(i); }


inline void PecosApproximation::combined_moment(Real mom, size_t i)
{ polyApproxRep->combined_moment(mom, i); }


inline void PecosApproximation::
build_linear_system(RealMatrix& A, const UShort2DArray& multi_index)
{
  std::static_pointer_cast<Pecos::RegressOrthogPolyApproximation>
    (polyApproxRep)->build_linear_system(A, multi_index);
}


inline void PecosApproximation::
augment_linear_system(const RealVectorArray& samples, RealMatrix& A,
		      const UShort2DArray& multi_index)
{
  std::static_pointer_cast<Pecos::RegressOrthogPolyApproximation>
    (polyApproxRep)->augment_linear_system(samples, A, multi_index);
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
  // TO DO: increment_coefficients() below covers current usage of
  // append_approximation() in NonDExpansion.  For more general
  // support of both update and append, need a mechanism to detect
  // the +/- direction of discrepancy between data and coefficients.

  //size_t curr_pts  = data_rep->surrogate_data().points(),
  //  curr_pecos_pts = polyApproxRep->data_size();
  //if (curr_pts > curr_pecos_pts)
    pecosBasisApprox.increment_coefficients();
  //else if (curr_pts < curr_pecos_pts)
  //  pecosBasisApprox.pop_coefficients();
  //else: if number of points is consistent, leave as is
}


inline void PecosApproximation::pop_coefficients(bool save_data)
{ pecosBasisApprox.pop_coefficients(save_data); }


inline void PecosApproximation::push_coefficients()
{ pecosBasisApprox.push_coefficients(); }


inline void PecosApproximation::finalize_coefficients()
{ pecosBasisApprox.finalize_coefficients(); }


inline void PecosApproximation::combine_coefficients()
{ pecosBasisApprox.combine_coefficients(); }


inline void PecosApproximation::
combined_to_active_coefficients(bool clear_combined)
{ pecosBasisApprox.combined_to_active(clear_combined); }


inline void PecosApproximation::clear_computed_bits()
{ polyApproxRep->clear_computed_bits(); }


inline void PecosApproximation::clear_inactive_coefficients()
{ pecosBasisApprox.clear_inactive(); }


inline bool PecosApproximation::advancement_available()
{ return pecosBasisApprox.advancement_available(); }


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
