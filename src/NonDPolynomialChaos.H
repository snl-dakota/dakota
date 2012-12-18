/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDPolynomialChaos
//- Description: Iterator to compute/employ Polynomial Chaos expansions
//- Owner:       Mike Eldred, Sandia National Laboratories

#ifndef NOND_POLYNOMIAL_CHAOS_H
#define NOND_POLYNOMIAL_CHAOS_H

#include "NonDExpansion.H"
#include "BasisPolynomial.hpp"

namespace Dakota {

/// Nonintrusive polynomial chaos expansion approaches to uncertainty
/// quantification

/** The NonDPolynomialChaos class uses a polynomial chaos expansion
    (PCE) approach to approximate the effect of parameter
    uncertainties on response functions of interest.  It utilizes the
    OrthogPolyApproximation class to manage multiple types of
    orthogonal polynomials within a Wiener-Askey scheme to PCE.  It
    supports PCE coefficient estimation via sampling, quadrature,
    point-collocation, and file import. */

class NonDPolynomialChaos: public NonDExpansion
{
public:

  //
  //- Heading: Constructors and destructor
  //
 
  /// standard constructor
  NonDPolynomialChaos(Model& model);
  /// alternate constructor
  NonDPolynomialChaos(Model& model, short exp_coeffs_approach,
		      unsigned short num_int_level, short u_space_type,
		      bool piecewise_basis, bool use_derivs);
  /// destructor
  ~NonDPolynomialChaos();

  //
  //- Heading: Virtual function redefinitions
  //

  void resolve_inputs(short& u_space_type, short& data_order);
  void initialize_u_space_model();

  /// form or import an orthogonal polynomial expansion using PCE methods
  void compute_expansion();

  /// uniformly increment the order of the polynomial chaos expansion
  void increment_order();

  /// print the PCE coefficient array for the orthogonal basis
  void print_coefficients(std::ostream& s);

private:

  /// convert number of expansion terms and collocation ratio to a
  /// number of collocation samples
  int  terms_ratio_to_samples(size_t num_exp_terms, Real colloc_ratio,
			      Real terms_order);
  /// convert number of expansion terms and number of collocation samples
  /// to a collocation ratio
  Real terms_samples_to_ratio(size_t num_exp_terms, int samples,
			      Real terms_order);
  /// convert an isotropic/anisotropic expansion_order vector into a scalar
  /// plus a dimension preference vector
  void order_to_dim_preference(const UShortArray& order, unsigned short& p,
			       RealVector& dim_pref);

  //
  //- Heading: Data
  //

  /// filename for import of chaos coefficients
  String expansionImportFile;
  /// factor applied to terms^termsOrder in computing number of regression
  /// points, either user specified or inferred
  Real collocRatio;
  /// exponent applied to number of expansion terms for computing
  /// number of regression points
  Real termsOrder;
  /// option for regression PCE using a filtered set tensor-product points
  bool tensorRegression;
  /// flag for use of cross-validation for selection of parameter settings
  /// in regression approaches
  bool crossValidation;
  /// noise tolerance for compressive sensing algorithms; vector form used
  /// in cross-validation
  RealVector noiseTols;
  /// L2 penalty for LASSO algorithm (elastic net variant)
  Real l2Penalty;

  /// derivative of the PCE with respect to the x-space variables
  /// evaluated at the means (used as uncertainty importance metrics)
  RealMatrix pceGradsMeanX;
};


inline int NonDPolynomialChaos::
terms_ratio_to_samples(size_t num_exp_terms, Real colloc_ratio,
		       Real terms_order)
{
  // min_samples logic inactive due to support of under-determined solves with
  // colloc_ratio < 1
  size_t data_per_pt = (useDerivs) ? numContinuousVars + 1 : 1;;
  Real min_pts = std::pow((Real)num_exp_terms, terms_order) / (Real)data_per_pt;
  int tgt_samples = (int)std::floor(colloc_ratio*min_pts + .5); // rounded
  if (colloc_ratio >= 1.) {
    // logic is to round to the nearest integral sample count for the given
    // colloc_ratio, but with a lower bound determined by rounding up with a
    // unit colloc_ratio.  The lower bound prevents creating an under-determined
    // system due to rounding down when the intent is over- or uniquely
    // determined (can only happen with non-integral min_pts from use of
    // derivatives and colloc_ratio at or near 1).
    int min_samples = (int)std::ceil(min_pts); // lower bound
    return std::max(min_samples, tgt_samples);
  }
  else
    // for under-determined systems, data starvation is not a problem and we
    // just need at least one sample.
    return std::max(tgt_samples, 1);
}


inline Real NonDPolynomialChaos::
terms_samples_to_ratio(size_t num_exp_terms, int samples, Real terms_order)
{
  size_t data_per_pt = (useDerivs) ? numContinuousVars + 1 : 1;;
  return (Real)(samples * data_per_pt) /
    std::pow((Real)num_exp_terms, terms_order);
}


inline void NonDPolynomialChaos::
order_to_dim_preference(const UShortArray& order, unsigned short& scalar,
			RealVector& dim_pref)
{
  // Note: this function is the inverse of NonDQuadrature::
  // anisotropic_preference(unsigned short, const RealVector&, UShortArray&)

  scalar = order[0];
  size_t i; bool anisotropic = false;
  for (i=1; i<numContinuousVars; ++i)
    if (order[i] > scalar)
      { scalar = order[i]; anisotropic = true; }

  if (anisotropic) { // preserve ratios; normalization not required
    dim_pref.resize(numContinuousVars);
    for (i=0; i<numContinuousVars; ++i)
      dim_pref[i] = (Real)order[i];
  }
  else
    dim_pref.resize(0);
}

} // namespace Dakota

#endif
