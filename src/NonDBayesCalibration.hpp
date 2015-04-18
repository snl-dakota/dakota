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

  const Model& algorithm_space_model() const;

  //
  //- Heading: Member functions
  //

  /// initialize emulator model and probability space transformations
  void initialize_model();

  //
  //- Heading: Data
  //

  // technically doesn't apply to GPMSA, but leaving here for now
  /// the emulator type: NO_EMULATOR, GP_EMULATOR, PCE_EMULATOR, or SC_EMULATOR
  short emulatorType;
  /// Model instance employed in the likelihood function; provides response
  /// function values from Gaussian processes, stochastic expansions (PCE/SC),
  /// or direct access to simulations (no surrogate option)
  Model mcmcModel;

  /// NonDPolynomialChaos or NonDStochCollocation instance for defining a
  /// PCE/SC-based mcmcModel
  Iterator stochExpIterator;

  /// number of samples in the chain (e.g. number of MCMC samples)
  int numSamples;
  /// number of update cycles for MCMC chain (implemented by restarting
  /// of short chains)
  int chainCycles;
  /// random seed for MCMC process
  int randomSeed;

  /// flag indicating use of a variable transformation to standardized
  /// probability space for the model or emulator
  bool standardizedSpace;
  /// flag indicating usage of adaptive posterior refinement; currently makes
  /// sense for unstructured grids in GP and PCE least squares/CS
  bool adaptPosteriorRefine;

  /// approach for defining proposal covariance
  String proposalCovarType;
  /// data from user input of proposal covariance
  RealVector proposalCovarData;
  /// filename for user-specified proposal covariance
  String proposalCovarFilename;
  /// approach for defining proposal covariance
  String proposalCovarInputType;

private:

  //
  //- Heading: Data
  //

};


inline const Model& NonDBayesCalibration::algorithm_space_model() const
{ return mcmcModel; }

} // namespace Dakota

#endif
