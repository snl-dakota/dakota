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

  NonDPolynomialChaos(Model& model); ///< constructor
  ~NonDPolynomialChaos();            ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  /// initialize the polynomial chaos expansion
  void initialize_expansion();
  /// form or import an orthogonal polynomial expansion using PCE methods
  void compute_expansion();
  /// uniformly increment the order of the polynomial chaos expansion
  void increment_expansion();

  /// print the PCE coefficient array for the orthogonal basis
  void print_coefficients(std::ostream& s);
  /// print the expansion/numerical moments
  void print_moments(std::ostream& s);

private:

  /// convert number of expansion terms and collocation ratio to a
  /// number of collocation samples
  int  terms_ratio_to_samples(size_t num_exp_terms, Real colloc_ratio);
  /// convert number of expansion terms and number of collocation samples
  /// to a collocation ratio
  Real terms_samples_to_ratio(size_t num_exp_terms, int samples);

  //
  //- Heading: Data
  //

  String expansionImportFile; ///< filename for import of chaos coefficients
  String pointReuse;  ///< user spec of reuse_points for regression-based PCE

  int expansionTerms; ///< user specification of PCE terms

  Real collocRatio; ///< collocation ratio, either user specified or inferred

  /// derivative of the PCE with respect to the x-space variables
  /// evaluated at the means (used as uncertainty importance metrics)
  RealMatrix pceGradsMeanX;
};


inline int NonDPolynomialChaos::
terms_ratio_to_samples(size_t num_exp_terms, Real colloc_ratio)
{
  size_t data_per_pt = (useDerivsFlag) ? numContinuousVars + 1 : 1;
  Real min_pts = (Real)num_exp_terms/(Real)data_per_pt;
  int min_samples = (int)std::ceil(min_pts),                    // lower bound
      tgt_samples = (int)std::floor(colloc_ratio*min_pts + .5); // rounded
  return std::max(min_samples, tgt_samples);
}


inline Real NonDPolynomialChaos::
terms_samples_to_ratio(size_t num_exp_terms, int samples)
{
  size_t data_per_pt = (useDerivsFlag) ? numContinuousVars + 1 : 1;
  Real min_pts = (Real)num_exp_terms/(Real)data_per_pt;
  return (Real)samples/min_pts;
}

} // namespace Dakota

#endif
