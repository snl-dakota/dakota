/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        .....
//- Description:  .....
//-               
//- Owner:        .....
 

#ifndef DAKOTA_SURROGATES_POLY_H
#define DAKOTA_SURROGATES_POLY_H

#include "DakotaApproximation.hpp"

#include <memory>

namespace dakota {
   namespace surrogates {
     class PolynomialRegression;
   }
}

namespace Dakota {

class SharedApproxData;


/// Derived approximation class for Surrogates Polynomial approximation classes.

/** The GPApproximation class is the interface between Dakota
    and the Surrogates Gaussian Process Module.  */
class SurrogatesPolyApprox: public Approximation
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

  /// Return the value of the surrogate for a given Variables object vars
  Real value(const Variables& vars) override;

  const RealVector& gradient(const Variables& vars);

  /// Return the value of the surrogate for a given parameter vector x
  Real value(const RealVector& c_vars) override;

  const RealVector& gradient(const RealVector& c_vars);

private:

  //
  //- Heading: Convenience functions
  //

  /// The native surrogate model
  std::shared_ptr<dakota::surrogates::PolynomialRegression> model;
};

} // namespace Dakota
#endif
