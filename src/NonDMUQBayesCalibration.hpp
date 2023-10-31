/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_MUQ_BAYES_CALIBRATION_H
#define NOND_MUQ_BAYES_CALIBRATION_H

#include "NonDBayesCalibration.hpp"

#include "MUQ/Modeling/WorkGraph.h"
#include "MUQ/SamplingAlgorithms/MarkovChain.h"
#include "MUQ/SamplingAlgorithms/SampleCollection.h"
#include "MUQ/SamplingAlgorithms/MCMCFactory.h"
#include "MUQ/SamplingAlgorithms/SamplingProblem.h"
#include "MUQ/SamplingAlgorithms/SamplingState.h"
#include "MUQ/Modeling/LinearAlgebra/IdentityOperator.h"
#include "MUQ/Modeling/Distributions/Gaussian.h"
#include "MUQ/Modeling/Distributions/Density.h"
#include "MUQ/Modeling/Distributions/DensityProduct.h"


namespace Dakota {

class MUQLikelihood;
class MUQPrior;

/// Dakota interface to MUQ (MIT Uncertainty Quantification) library

/** This class performs Bayesian calibration using the MUQ library
    */

class NonDMUQBayesCalibration: public NonDBayesCalibration
{

    friend class MUQLikelihood;
    friend class MUQPrior;

public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDMUQBayesCalibration(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDMUQBayesCalibration();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void calibrate();
  void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

  /// convenience function to print calibration parameters, e.g., for
  /// MAP / best parameters
  void print_variables(std::ostream& s, const RealVector& c_vars);

  /// cache the chain to acceptanceChain and acceptedFnVals
  void cache_chain();

  /// log at most batchSize best chain points into bestSamples
  void log_best();

  /// initialize the MUQ parameter space, min, max, initial, domain, and prior
  /// define solver options, likelihood callback, posterior RV, and inverse problem
  void specify_prior() override;
  void specify_likelihood() override;
  void init_bayesian_solver() override;
  void specify_posterior() override;

  /// set the proposal covariance matrix
  void init_proposal_covariance();

  /// use covariance of prior distribution for setting proposal covariance
  void prior_proposal_covariance();

  /// set proposal covariance from user-provided diagonal or matrix
  void user_proposal_covariance(const String& input_fmt, 
        const RealVector& cov_data, 
        const String& cov_filename);

  // perform sanity checks on proposalCovMatrix
  void validate_proposal();


  //
  //- Heading: Data
  //

  std::shared_ptr<muq::Modeling::WorkGraph> workGraph;

  std::shared_ptr<muq::Modeling::IdentityOperator>            parameterPtr;
  std::shared_ptr<muq::Modeling::Distribution>                distPtr;
  std::shared_ptr<muq::Modeling::DensityProduct>              posteriorPtr;
  std::shared_ptr<MUQLikelihood>                              MUQLikelihoodPtr;
  std::shared_ptr<MUQPrior>    MUQPriorPtr;

  std::shared_ptr<muq::SamplingAlgorithms::SingleChainMCMC>   mcmc;
  std::shared_ptr<muq::SamplingAlgorithms::SampleCollection>  samps;

  /// MCMC type ("dram" or "delayed_rejection" or "adaptive_metropolis" 
  /// or "metropolis_hastings" or "multilevel",  within QUESO) 
  String mcmcType;

  /// Pointer to current class instance for use in static callback functions
  static NonDMUQBayesCalibration* nonDMUQInstance;

  /// number of best samples (max log_posterior values) to keep 
  unsigned int numBestSamples;

  /// proposal covariance for MCMC
  Eigen::MatrixXd proposalCovMatrix;

  /// optional multiplier to scale prior-based proposal covariance 
  double priorPropCovMult;

  /// initial guess (user-specified or default initial values)
  RealVector init_point;

private:

  //
  // - Heading: Data
  //
  

};

class MUQLikelihood : public muq::Modeling::Density {

public:

  inline MUQLikelihood(NonDMUQBayesCalibration* nond_muq_ptr,
    std::shared_ptr<muq::Modeling::Distribution> distPtr) :
    muq::Modeling::Density(distPtr), nonDMUQInstancePtr(nond_muq_ptr) {};

  double LogDensityImpl(muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs);

protected:

private:

    NonDMUQBayesCalibration* nonDMUQInstancePtr;
};

class MUQPrior : public muq::Modeling::Density {
public:

  inline MUQPrior(NonDMUQBayesCalibration* nond_muq_ptr, std::shared_ptr<muq::Modeling::Distribution> distPtr) :
  muq::Modeling::Density(distPtr), nonDMUQInstancePtr(nond_muq_ptr) { };

  double LogDensityImpl(muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs);

protected:

private:

  NonDMUQBayesCalibration* nonDMUQInstancePtr;

};

} // namespace Dakota

#endif
