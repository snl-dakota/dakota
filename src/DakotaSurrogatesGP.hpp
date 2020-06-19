/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_GP_H
#define DAKOTA_SURROGATES_GP_H

#include "DakotaSurrogates.hpp"


namespace Dakota {

/// Derived approximation class for Surrogates approximation classes.

/** This class interfaces Dakota to the Dakota Surrogates Gaussian
    Process Module.  */
class SurrogatesGPApprox: public SurrogatesBaseApprox
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  SurrogatesGPApprox() { }
  /// standard constructor: 
  SurrogatesGPApprox(const ProblemDescDB& problem_db,
		     const SharedApproxData& shared_data,
		     const String& approx_label);
  /// alternate constructor
  SurrogatesGPApprox(const SharedApproxData& shared_data);
  /// destructor
  ~SurrogatesGPApprox() { }

  // diagnostics: all documented at base class

  bool diagnostics_available() override;

  Real diagnostic(const String& metric_type) override;

  void primary_diagnostics(int fn_index) override;

  void challenge_diagnostics(int fn_index, const RealMatrix& challenge_points,
                             const RealVector& challenge_responses) override;

protected:

  // Minimum number of data points required to build
  int min_coefficients() const override;

  ///  Do the build
  void build() override;

};

} // namespace Dakota
#endif
