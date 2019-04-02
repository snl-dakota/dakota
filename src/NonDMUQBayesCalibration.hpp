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


namespace Dakota {


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

  MUQ::WorkGraph workGraph;

  std::shared_ptr<MUQ::IdentityOperator>       thetaPtr;
  std::shared_ptr<MUQPriorInterface>           priorPtr;
  std::shared_ptr<MUQModelInterface>           modelPtr;
  std::shared_ptr<MUQLikelihoodInterface> likelihoodPtr;
  std::shared_ptr<MUQ::DensityProduct>     posteriorPtr;

  std::shared_ptr<MUQ::SamplingProblem> samplingProbPtr;
  std::shared_ptr<MUQ::MCMCSampling>     mcmcSamplerPtr;
  std::shared_ptr<MUQ::MarkovChain>    mcmcSampleSetPtr;

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
  ~MUQModelInterface();

  //
  //- Heading: Static callback functions required by MUQ
  //


protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void EvaluateImpl(muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs);
  void GradientImpl(muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs,
		    ...index_of_input_like_DVV...);

  //
  //- Heading: Data
  //

  /// shallow copy of Dakota::Model being evaluated (Simulation, Surrogate, ...)
  Model dakotaModel;

  bool useDerivs;
};


inline MUQModelInterface::MUQModelInterface():
  muq::Modeling::ModPiece(2*Eigen::VectorXi::Ones(1), // inputSizes  = [1]
			  2*Eigen::VectorXi::Ones(1)) // outputSizes = [1]
{ }


class MUQPriorInterface: public muq::Modeling::Density
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  MUQPriorInterface();
  /// destructor
  ~MUQPriorInterface();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  // evaluate log prior(x)
  double LogDensityImpl(ref_vector< Eigen::VectorXd > const& inputs)

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
  MUQLikelihoodInterface();
  /// destructor
  ~MUQLikelihoodInterface();

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
  double LogDensityImpl(ref_vector< Eigen::VectorXd > const& inputs)

  //
  //- Heading: Data
  //

  // observation data or ResidualModel ?
  //RealVector obsData;
};

} // namespace Dakota

#endif
