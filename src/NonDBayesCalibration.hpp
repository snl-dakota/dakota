/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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

#include "NonDCalibration.hpp"


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

  /// standard constructor
  NonDBayesCalibration(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDBayesCalibration();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void derived_init_communicators(ParLevLIter pl_iter);
  void derived_set_communicators(ParLevLIter pl_iter);
  void derived_free_communicators(ParLevLIter pl_iter);

  //void print_results(std::ostream& s);

  void quantify_uncertainty();

  const Model& algorithm_space_model() const;

  //
  //- Heading: Data
  //

  /// number of samples in the chain (e.g. number of MCMC samples)
  int numSamples;
 
  /// random seed for MCMC process
  int randomSeed;

  // technically doesn't apply to GPMSA, but leaving here for now
  /// the emulator type: NO_EMULATOR, GP_EMULATOR, PCE_EMULATOR, or SC_EMULATOR
  short emulatorType;

  /// Model instance employed in the likelihood function; provides response
  /// function values from Gaussian processes, stochastic expansions (PCE/SC),
  /// or direct access to simulations (no surrogate option)
  Model emulatorModel;

  /// flag indicating use of a variable transformation to standardized
  /// probability space for the model or emulator
  bool standardizedSpace;
  /// NonDPolynomialChaos or NonDStochCollocation instance for defining a
  /// PCE/SC-based emulatorModel
  Iterator stochExpIterator;
  /// LHS iterator for generating samples for GP 
  Iterator lhsIterator;

private:

  //
  //- Heading: Data
  //

};


inline const Model& NonDBayesCalibration::algorithm_space_model() const
{ return emulatorModel; }

} // namespace Dakota

#endif
