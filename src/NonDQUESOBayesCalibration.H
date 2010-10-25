/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDBayesCalibration
//- Description: Base class for generic Bayesian inference
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#ifndef NOND_QUESO_BAYES_CALIBRATION_H
#define NOND_QUESO_BAYES_CALIBRATION_H

#include "NonDBayesCalibration.H"
#include "uqGenericScalarFunctionClass.H"


namespace Dakota {


/// Bayesian inference using the QUESO library from UT Austin

/** This class ...  */

class NonDQUESOBayesCalibration: public NonDBayesCalibration,
  uqBaseScalarFunctionClass<uqGslVectorClass,uqGslMatrixClass>
{
public:

  //
  //- Heading: Constructors and destructor
  //

  NonDQUESOBayesCalibration(Model& model); ///< standard constructor
  ~NonDQUESOBayesCalibration();            ///< destructor

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  /// redefined from DakotaNonD
  void quantify_uncertainty();
  // redefined from DakotaNonD
  //void print_results(std::ostream& s);

  /// redefined from QUESO::
  Real actualValue(...);
  /// redefined from QUESO::
  Real lnValue(...);

  //
  //- Heading: Data
  //

private:

};


inline Real NonDQUESOBayesCalibration::actualValue(...x...)
{ return std::exp(lnValue(...x...)); }

} // namespace Dakota

#endif
