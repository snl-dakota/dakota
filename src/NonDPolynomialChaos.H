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
  /// convert an expansion_terms specification into the lowest order isotropic
  /// total-order expansion (scalar) that contains at least this many terms
  void terms_to_total_order(size_t tgt_exp_terms, unsigned short& p);
  /// convert an expansion_terms specification into the lowest order isotropic
  /// total-order expansion (vector) that contains at least this many terms
  void terms_to_total_order_expansion(size_t tgt_exp_terms,
				      UShortArray& to_exp);
  /// convert an isotropic/anisotropic expansion_order vector into a scalar
  /// plus a dimension preference vector
  void exp_order_to_dim_preference(const UShortArray& exp_order,
				   unsigned short& p, RealVector& dim_pref);

  //
  //- Heading: Data
  //

  /// filename for import of chaos coefficients
  String expansionImportFile;
  /// user specification of PCE terms
  int expansionTerms;
  /// factor applied to expansionTerms^termsOrder in computing number of
  /// regression points, either user specified or inferred
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
  size_t data_per_pt = (useDerivs) ? numContinuousVars + 1 : 1;;
  Real min_pts = std::pow((Real)num_exp_terms, terms_order) / (Real)data_per_pt;
  //int min_samples = (int)std::ceil(min_pts),                    // lower bound
  int tgt_samples = (int)std::floor(colloc_ratio*min_pts + .5); // rounded
  return tgt_samples;
  //return std::max(min_samples, tgt_samples);
}


inline Real NonDPolynomialChaos::
terms_samples_to_ratio(size_t num_exp_terms, int samples, Real terms_order)
{
  size_t data_per_pt = (useDerivs) ? numContinuousVars + 1 : 1;;
  return (Real)(samples * data_per_pt) /
    std::pow((Real)num_exp_terms, terms_order);
}


inline void NonDPolynomialChaos::
terms_to_total_order(size_t tgt_exp_terms, unsigned short& p)
{
  p = 0; size_t p_exp_terms = 1;
  while (p_exp_terms < tgt_exp_terms) {
    ++p;
    p_exp_terms = (size_t)std::floor(
      Pecos::BasisPolynomial::n_choose_k(p+numContinuousVars, p)+.5); // rounded
  }
}


inline void NonDPolynomialChaos::
terms_to_total_order_expansion(size_t tgt_exp_terms, UShortArray& to_exp)
{
  unsigned short p;
  terms_to_total_order(tgt_exp_terms, p);
  if (to_exp.size() != numContinuousVars)
    to_exp.resize(numContinuousVars);
  to_exp.assign(numContinuousVars, p);
}


inline void NonDPolynomialChaos::
exp_order_to_dim_preference(const UShortArray& exp_order, unsigned short& p,
			    RealVector& dim_pref)
{
  // Note: this function is the inverse of NonDQuadrature::
  // anisotropic_preference(unsigned short, const RealVector&, UShortArray&)

  p = exp_order[0];
  size_t i, max_index = 0;
  bool anisotropic = false;
  for (i=1; i<numContinuousVars; ++i)
    if (exp_order[i] > p)
      { p = exp_order[i]; max_index = i; anisotropic = true; }

  if (anisotropic) { // preserve ratios; normalization not required
    dim_pref.resize(numContinuousVars);
    for (i=0; i<numContinuousVars; ++i)
      dim_pref[i] = (Real)exp_order[i];
  }
  else
    dim_pref.resize(0);
}

} // namespace Dakota

#endif
