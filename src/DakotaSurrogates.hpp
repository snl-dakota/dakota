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
 

#ifndef DAKOTA_SURROGATES_H 
#define DAKOTA_SURROGATES_H 

#include "DakotaApproximation.hpp"
#include <memory>

namespace dakota {
   namespace surrogates {
     class GaussianProcess;
   }
}

namespace Dakota {

class SharedApproxData;


/// Derived approximation class for Surrogates approximation classes.

/** The GPApproximation class is the interface between Dakota
    and the Surrogates Module.  */

class GPApproximation: public Approximation
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  GPApproximation() { }
  /// standard constructor: 
  //GPApproximation(const ProblemDescDB& problem_db,
  //      		const SharedApproxData& shared_data,
  //                      const String& approx_label);
  /// alternate constructor
  GPApproximation(const SharedApproxData& shared_data);
  /// destructor
  ~GPApproximation() { }

protected:

//  //
//  //- Heading: Virtual function redefinitions
//  //
//
  int min_coefficients() const override;

  ///  Do the build
  void build() override;

  /// Return the value of the surrogate for a given parameter vector x
  Real value(const RealVector& c_vars) override;
 

private:

  //
  //- Heading: Convenience functions
  //

  /// The native surrogate model
  std::shared_ptr<dakota::surrogates::GaussianProcess> model;
};

} // namespace Dakota
#endif
