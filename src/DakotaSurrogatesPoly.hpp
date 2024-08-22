/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_POLY_H
#define DAKOTA_SURROGATES_POLY_H

#include "DakotaSurrogates.hpp"


namespace Dakota {

/// Derived approximation class for Surrogates Polynomial approximation classes.

/** This class interfaces Dakota to the Dakota Surrogates Polynomial Module. */
class SurrogatesPolyApprox: public SurrogatesBaseApprox
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  SurrogatesPolyApprox() { }
  /// standard constructor: 
  SurrogatesPolyApprox(const ProblemDescDB& problem_db,
		       const SharedApproxData& shared_data,
		       const String& approx_label);
  /// alternate constructor
  SurrogatesPolyApprox(const SharedApproxData& shared_data);
  /// destructor
  ~SurrogatesPolyApprox() { }

protected:

  // Minimum number of data points required to build
  int min_coefficients() const override;

  ///  Do the build
  void build() override;

};

} // namespace Dakota
#endif
