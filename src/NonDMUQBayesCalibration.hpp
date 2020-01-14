/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDMUQBayesCalibration
//- Description: Derived class for MUQ-based Bayesian inference
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_MUQ_BAYES_CALIBRATION_H
#define NOND_MUQ_BAYES_CALIBRATION_H

#include "NonDBayesCalibration.hpp"

#include "MUQ/Modeling/WorkGraph.h"
#include "MUQ/SamplingAlgorithms/MarkovChain.h"
#include "MUQ/SamplingAlgorithms/SampleCollection.h"
#include "MUQ/SamplingAlgorithms/MCMCFactory.h"
#include "MUQ/SamplingAlgorithms/SamplingProblem.h"
#include "MUQ/Modeling/LinearAlgebra/IdentityOperator.h"
#include "MUQ/Modeling/Distributions/Gaussian.h"
#include "MUQ/Modeling/Distributions/Density.h"
#include "MUQ/Modeling/Distributions/DensityProduct.h"


namespace Dakota {

class MUQPriorInterface;
class MUQModelInterface;
class MUQLikelihoodInterface;

/// Dakota interface to MUQ (MIT Uncertainty Quantification) library

/** This class performs Bayesian calibration using the MUQ library
    */

class NonDMUQBayesCalibration: public NonDBayesCalibration
{
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

  //
  //- Heading: Data
  //

  std::shared_ptr<muq::Modeling::WorkGraph> workGraph;

  std::shared_ptr<muq::Modeling::IdentityOperator>       thetaPtr;
  std::shared_ptr<muq::Modeling::Density>           priorPtr;
  std::shared_ptr<MUQModelInterface>           modelPtr;
  std::shared_ptr<muq::Modeling::Density> likelihoodPtr;
  std::shared_ptr<muq::Modeling::DensityProduct>     posteriorPtr;

  std::shared_ptr<muq::SamplingAlgorithms::SamplingProblem> samplingProbPtr;
  std::shared_ptr<muq::SamplingAlgorithms::SingleChainMCMC>     mcmc;
  std::shared_ptr<muq::SamplingAlgorithms::MarkovChain>    mcmcSampleSetPtr;
  std::shared_ptr<muq::SamplingAlgorithms::SampleCollection>    sampleCollPtr;

    /// MCMC type ("dram" or "delayed_rejection" or "adaptive_metropolis" 
    /// or "metropolis_hastings" or "multilevel",  within QUESO) 
  String mcmcType;

private:

  //
  // - Heading: Data
  // 

};


class MUQModelInterface: public muq::Modeling::ModPiece
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  MUQModelInterface(Model& model);
  /// destructor
  ~MUQModelInterface() { }

  //
  //- Heading: Static callback functions required by MUQ
  //


protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void EvaluateImpl(muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs);
//  void GradientImpl(muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs,
//		    ...index_of_input_like_DVV...);

  //
  //- Heading: Data
  //

  /// shallow copy of Dakota::Model being evaluated (Simulation, Surrogate, ...)
  Model dakotaModel;

  bool useDerivs;
  
};


inline MUQModelInterface::MUQModelInterface(Model & model):
// TODO: input and output dimensionality from Dakota model
  muq::Modeling::ModPiece(1*Eigen::VectorXi::Ones(1), // inputSizes  = [1]
			  1*Eigen::VectorXi::Ones(1)) // outputSizes = [1]
{ }


class MUQPriorInterface: public muq::Modeling::Density
{
public:

  //
  //- Heading: Constructors and destructor
  //

  // Eigen::VectorXd mu = Eigen::VectorXd(1);
  // mu(0) = 0.1;
  // // mu << 1.0;

  // Eigen::MatrixXd cov = Eigen::MatrixXd(1,1);
  // cov(0,0) = 1.0;
  // cov << 1.0;

  // auto prior = std::make_shared<muq::Modeling::Gaussian>(mu, cov);


  /// standard constructor
  MUQPriorInterface() : Density(std::make_shared<muq::Modeling::Gaussian>(0.0, 1.0)->AsDensity()) { }
  // MUQPriorInterface() : Density(std::make_shared<muq::Modeling::Gaussian>(mu, cov)->AsDensity()) { }
  /// destructor
  ~MUQPriorInterface() { }

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  // evaluate log prior(x)
  //double LogDensityImpl(muq::Modeling::ref_vector< Eigen::VectorXd > const& inputs) 
  //{ return 0.0; }

  //
  //- Heading: Data
  //

  /// shallow copy of Dakota MultivarDistribution instance
  //MultivarDistribution randomDist;
};


class MUQLikelihoodInterface: public muq::Modeling::Density
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  MUQLikelihoodInterface() : Density(std::make_shared<muq::Modeling::Gaussian>(0.0, 1.0)->AsDensity()){ }
  /// destructor
  ~MUQLikelihoodInterface() { }

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  // This function implements the Misfit(theta, d)
  //
  // incoming is G(theta) as inputs.at(0) from MUQModelInterface ModPiece
  // Approach 1:
  //   observational data as inputs.at(1) from another ModPiece
  //   (another inherited class or as a "ConstantPiece")
  // Approach 2: pass obs data is through the ctor and store as member data
  double LogDensityImpl(muq::Modeling::ref_vector< Eigen::VectorXd > const& inputs)
  { return 0.0; }

  //
  //- Heading: Data
  //

  // observation data or ResidualModel ?
  //RealVector obsData;
};

} // namespace Dakota

#endif
