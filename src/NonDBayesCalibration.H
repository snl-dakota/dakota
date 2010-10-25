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

#ifndef NOND_BAYES_CALIBRATION_H
#define NOND_BAYES_CALIBRATION_H

#include "NonDCalibration.H"


namespace Dakota {


/// Base class for Bayesian inference: generates posterior
/// distribution on model parameters given experimental data

/** This class will eventually provide a general-purpose framework for
    Bayesian inference.  In the short term, it only collects shared
    code between QUESO and GPMSA implementations. */

class NonDBayesCalibration: public NonDCalibration
{
public:

  //
  //- Heading: Constructors and destructor
  //

  NonDBayesCalibration(Model& model); ///< standard constructor
  ~NonDBayesCalibration();            ///< destructor

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  //void print_results(std::ostream& s);

  //
  //- Heading: Data
  //

  /// Model instance employed in the likelihood function; provides response
  /// function values from Gaussian processes, stochastic expansions (PCE/SC),
  /// or direct access to simulations (no surrogate option)
  Model emulatorModel;

  const int seedSpec;   ///< the user seed specification (default is 0)
  int       numSamples; ///< the current number of samples to evaluate
  String    rngName;	///< name of the random number generator

private:

};

} // namespace Dakota

#endif
