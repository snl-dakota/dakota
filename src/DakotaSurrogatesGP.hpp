/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_GP_H
#define DAKOTA_SURROGATES_GP_H

#include "DakotaModel.hpp"
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

protected:

  // Minimum number of data points required to build
  int min_coefficients() const override;

  ///  Do the build
  void build() override;

  Real prediction_variance(const Variables& vars) override;

  Real prediction_variance(const RealVector& c_vars) override;

};

// free function for setting up experimental GPs with an
// advanced options file
void set_model_gp_options(Model& model, const String& options_file);

} // namespace Dakota
#endif
