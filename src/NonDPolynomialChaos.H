/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
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

  void initialize_expansion();

  /// form or import an orthogonal polynomial expansion using PCE methods
  void compute_expansion();

  /// print the PCE coefficient array for the orthogonal basis
  void print_coefficients(std::ostream& s);
  /// print the expansion/numerical moments
  void print_moments(std::ostream& s);

private:

  //
  //- Heading: Data
  //

  String expansionImportFile; ///< filename for import of chaos coefficients

  int expansionTerms;         ///< user specification of PCE terms

  /// derivative of the PCE with respect to the x-space variables
  /// evaluated at the means (used as uncertainty importance metrics)
  RealMatrix pceGradsMeanX;
};

} // namespace Dakota

#endif
