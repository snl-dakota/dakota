/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_MUQ_BAYES_CALIBRATION_H
#define NOND_MUQ_BAYES_CALIBRATION_H

#include "NonDBayesCalibration.hpp"

#include "MUQ/Modeling/WorkGraph.h"
#include "MUQ/Modeling/ModGraphPiece.h"
#include "MUQ/SamplingAlgorithms/MarkovChain.h"
#include "MUQ/SamplingAlgorithms/SampleCollection.h"
#include "MUQ/SamplingAlgorithms/MCMCFactory.h"
#include "MUQ/SamplingAlgorithms/SingleChainMCMC.h"

#include "MUQ/SamplingAlgorithms/SLMCMC.h"
#include "MUQ/SamplingAlgorithms/GreedyMLMCMC.h"
#include "MUQ/SamplingAlgorithms/MIMCMC.h"
#include "MUQ/SamplingAlgorithms/CrankNicolsonProposal.h"
#include "MUQ/SamplingAlgorithms/SamplingProblem.h"
#include "MUQ/SamplingAlgorithms/SamplingState.h"
#include "MUQ/SamplingAlgorithms/SubsamplingMIProposal.h"
#include "MUQ/SamplingAlgorithms/MultiIndexEstimator.h"
#include "MUQ/SamplingAlgorithms/MHProposal.h"
#include "MUQ/SamplingAlgorithms/AMProposal.h"

#include "MUQ/Modeling/LinearAlgebra/IdentityOperator.h"
#include "MUQ/Modeling/Distributions/Gaussian.h"
#include "MUQ/Modeling/Distributions/Density.h"
#include "MUQ/Modeling/Distributions/DensityProduct.h"

namespace Dakota {

class MUQLikelihood;
class MUQPrior;
class MLMCMCLikelihood;
class MUQMLMCMCInterpolation;
class MLMCMCComponentFactory;

/// Dakota interface to MUQ (MIT Uncertainty Quantification) library

/** This class performs Bayesian calibration using the MUQ library
    */

class NonDMUQBayesCalibration: public NonDBayesCalibration
{

    friend class MUQLikelihood;
    friend class MUQPrior;
    friend class MLMCMCLikelihood;
    friend class MLMCMCComponentFactory;

public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDMUQBayesCalibration(ProblemDescDB& problem_db, std::shared_ptr<Model> model);
  /// destructor
  ~NonDMUQBayesCalibration();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void calibrate();


  void map_pre_solve() override;

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

  std::shared_ptr<muq::Modeling::IdentityOperator> parameterPtr;
  std::shared_ptr<muq::Modeling::DensityProduct>   posteriorPtr;
  std::shared_ptr<MUQLikelihood>                   MUQLikelihoodPtr;
  std::shared_ptr<MUQPrior>                        MUQPriorPtr; // Used by all MUQ MCMC methods except DILI
  std::shared_ptr<muq::Modeling::Gaussian>         muqGaussianPrior; // Used by DILI only

  std::shared_ptr<muq::SamplingAlgorithms::SingleChainMCMC>  mcmc;
  std::shared_ptr<muq::SamplingAlgorithms::SampleCollection> samps;

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

  /// DR num stages
  int drNumStages;

  /// DR scale type
  String drScaleType;

  /// DR scale
  Real drScale;

  /// AM period num steps
  int amPeriodNumSteps;

  /// AM staring step
  int amStartingStep;

  /// AM scale
  Real amScale;

  /// MALA step size
  Real malaStepSize;

  /// DILI Hessian type
  String diliHessianType;

  /// DILI adaptation interval
  int diliAdaptInterval;

  /// DILI adaptation start
  int diliAdaptStart;

  /// DILI adaptation end
  int diliAdaptEnd;

  /// DILI initial weight
  int diliInitialWeight;

  /// DILI Hessian tolerance
  Real diliHessTolerance;

  /// DILI LIS tolerance
  Real diliLISTolerance;

  /// DILI stochastic eigensolver maximum number of eigenvalues to compute
  int diliSesNumEigs;

  /// DILI stochastic eigensolver relative tolerance
  Real diliSesRelTol;

  /// DILI stochastic eigensolver absolute tolerance
  Real diliSesAbsTol;

  /// DILI stochastic eigensolver expected number of eigenvalues that are larger than the tolerances
  int diliSesExpRank;

  /// DILI stochastic eigensolver oversampling factor
  int diliSesOversFactor;

  /// DILI stochastic eigensolver block size
  int diliSesBlockSize;

  // MLMCMC initial number of samples 
  int mlmcmcInitialNumSamples;

  // MLMCMC target variance to achieve
  Real mlmcmcTargetVariance;

  // MLMCMC subsampling steps for each level
  IntVector mlmcmcSubsamplingSteps;

  // MLMCMC number of levels
  int mlmcmcNumLevels;

  // MLMCMC resampling factor needed by the MUQ greedy algorithm
  Real mlmcmcGreedyResamplingFactor;

  // MLMCMC MUQ component factory for setting up mlmcmc
  std::shared_ptr<MLMCMCComponentFactory> mlmcmcCompFactory;

  // MLMCMC instance of MUQ greedy sample
  std::shared_ptr<muq::SamplingAlgorithms::GreedyMLMCMC> mlmcmcGreedy;

private:
  // method for initializing the bayesian solver for single chains,
  // this is called for all methods *except* mlmcmc
  void init_bayesian_solver_single_chain();

  // method for initializing the bayesian solver for mlmcmc 
  void init_bayesian_solver_mlmcmc();

  // calibration implementation called for all methods *except* mlmcmc
  void calibrate_single_chain();
  
  // calibration implementation called for mlmcmc  
  void calibrate_mlmcmc();
};

class MUQLikelihood : public muq::Modeling::DensityBase {
public:

  inline MUQLikelihood( NonDMUQBayesCalibration       * nond_muq_ptr
                      , Eigen::VectorXi         const & input_sizes
                      )
    : muq::Modeling::DensityBase(input_sizes)
    , nonDMUQInstancePtr        (nond_muq_ptr)
  {
    // Nothing extra to do
  };

  double LogDensityImpl(muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs);

  Eigen::VectorXd GradLogDensityImpl(unsigned int wrt,
                                     muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs);

private:
  NonDMUQBayesCalibration * nonDMUQInstancePtr;
};

class MUQPrior : public muq::Modeling::DensityBase {
public:

  inline MUQPrior( NonDMUQBayesCalibration       * nond_muq_ptr
                 , Eigen::VectorXi         const & input_sizes
                 )
    : muq::Modeling::DensityBase(input_sizes)
    , nonDMUQInstancePtr        (nond_muq_ptr)
  {
    // Nothing extra to do
  };

  double LogDensityImpl(muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs);

  Eigen::VectorXd GradLogDensityImpl(unsigned int wrt,
                                     muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs);

private:
  NonDMUQBayesCalibration * nonDMUQInstancePtr;
};




class MLMCMCLikelihood : public muq::Modeling::DensityBase
{
  /*
    we need to have a special likelihood for MLMCMC because
    we need to store one for each level in the hierarchy.
    And then we need to use this level to toggle off/on
    the corresponding model inside the dakota evaluator.
  */
private:
  int m_myLevel = {};
  NonDMUQBayesCalibration* m_nonDMUQInstance;

public:
  inline MLMCMCLikelihood(Eigen::VectorXi const & input_sizes,
                      int myLevel,
                      NonDMUQBayesCalibration* nonDMUQInstance)
    : muq::Modeling::DensityBase(input_sizes)
    , m_myLevel(myLevel)
    , m_nonDMUQInstance(nonDMUQInstance)
    {
      if (m_nonDMUQInstance->outputLevel >= DEBUG_OUTPUT) {
        Cout << "MUQ: MLMCMC: MLMCMCLikelihood:  Constructor : level = " << m_myLevel << "\n";
      }
    };

  double LogDensityImpl(muq::Modeling::ref_vector<Eigen::VectorXd> const& state)
  {
    Eigen::VectorXd const& c_vars = state.at(0);
    size_t num_cv = c_vars.size();
    RealVector& all_params = m_nonDMUQInstance->residualModel->current_variables().continuous_variables_view();
    for (size_t i(0); i < num_cv; ++i) { all_params[i] = c_vars[i]; }

    /*
      use the level at which the instance of this class is defined
      to toggle off/on the corresponding model inside the dakota evaluator.
    */
    const Pecos::ActiveKey & curr_key = m_nonDMUQInstance->residualModel->active_model_key();
    if (m_myLevel != curr_key.retrieve_resolution_level()){
      Pecos::ActiveKey key;
      key.form_key(m_myLevel, curr_key.retrieve_model_form(), m_myLevel);
      m_nonDMUQInstance->residualModel->active_model_key(key);
    }

    m_nonDMUQInstance->residualModel->evaluate();
    const RealVector& residuals = m_nonDMUQInstance->residualModel->current_response().function_values();
    double log_like = m_nonDMUQInstance->log_likelihood(residuals, all_params);

    if (m_nonDMUQInstance->outputLevel >= DEBUG_OUTPUT) {
      Cout << "Log likelihood is " << log_like << " Likelihood is "
          << std::exp(log_like) << '\n';

      std::ofstream log_like_output;
      log_like_output.open("NonDMUQLogLike.txt", std::ios::out | std::ios::app);
      // Note: parameter values are in scaled space, if scaling is
      // active; residuals may be scaled by covariance
      size_t num_total_params = m_nonDMUQInstance->numContinuousVars + m_nonDMUQInstance->numHyperparams;
      for (size_t i(0); i < num_total_params; ++i){
        log_like_output << c_vars[i] << ' ' ;
      }
      for (size_t i(0); i < residuals.length(); ++i){
        log_like_output << residuals[i] << ' ' ;
      }
      log_like_output << log_like << '\n';
      log_like_output.close();
    }

    return log_like;
  }
};

class MUQMLMCMCInterpolation : public muq::SamplingAlgorithms::MIInterpolation{
public:
  std::shared_ptr<muq::SamplingAlgorithms::SamplingState> Interpolate (
      std::shared_ptr<muq::SamplingAlgorithms::SamplingState> const& coarseProposal,
      std::shared_ptr<muq::SamplingAlgorithms::SamplingState> const& /*fineProposal*/
    )
  {
    // for now, we do not support calibrating different params count at different
    // levels, so here the state proposed at the coarse level is just copied
    return std::make_shared<muq::SamplingAlgorithms::SamplingState>(coarseProposal->state);
  }
};

class MLMCMCComponentFactory : public muq::SamplingAlgorithms::MIComponentFactory
{
  boost::property_tree::ptree pt;
  Eigen::VectorXd m_startingPoint;
  NonDMUQBayesCalibration * m_nonDMUQInstancePtr;
  int m_numLevels = {};
  size_t m_num_cv;

public:
  MLMCMCComponentFactory(
    boost::property_tree::ptree pt,
    int numLevels,
    size_t num_cv,
    const RealVector & init_point_vector,
    NonDMUQBayesCalibration * nonDMUQInstancePtr)
    : pt(pt),
    m_startingPoint(num_cv),
    m_nonDMUQInstancePtr(nonDMUQInstancePtr),
    m_numLevels(numLevels),
    m_num_cv(num_cv)
  {
    if (m_nonDMUQInstancePtr->outputLevel >= DEBUG_OUTPUT) {
      Cout << "MUQ: MLMCMC: MLMCMCComponentFactory: Constructor \n";
      Cout << "num of levels = " << m_numLevels << std::endl;
      Cout << "num_cv        = " << m_num_cv << std::endl;
    }

    // upon construction, initialize the starting point
    for (size_t i(0); i < m_startingPoint.size(); ++i){
      m_startingPoint[i] = init_point_vector[i];
    }
  }

  virtual std::shared_ptr<muq::SamplingAlgorithms::MCMCProposal> Proposal (
      std::shared_ptr<muq::Utilities::MultiIndex> const& index,
      std::shared_ptr<muq::SamplingAlgorithms::AbstractSamplingProblem> const& samplingProblem) override
  {
    if (m_nonDMUQInstancePtr->outputLevel >= DEBUG_OUTPUT) {
      Cout << "MUQ: MLMCMC: MLMCMCComponentFactory: creating the proposal \n";
    }

    // create the proposal for any given level
    // for now, we have the same problem for every level because the same uncertain param dim
    // we use the same proposal using the covariance matrix setup during initialization

    boost::property_tree::ptree local_pt;
    local_pt.put("BlockIndex", 0 /*leave = 0 */);

    // Nov. 2024, FRIZZI: for now, use MH proposal by default
    // later on we can add another option to the input file to switch
#if 1
    Eigen::VectorXd prop_mu = Eigen::VectorXd::Zero(m_num_cv);
    auto propDist = std::make_shared<muq::Modeling::Gaussian>(prop_mu, m_nonDMUQInstancePtr->proposalCovMatrix);
    return std::make_shared<muq::SamplingAlgorithms::MHProposal>(pt, samplingProblem, propDist);
#else
    // crank-nic does not work here, something is wrong
    // Eigen::VectorXd prop_mu = Eigen::VectorXd::Zero(m_nonDMUQInstancePtr->numContinuousVars);
    // auto prior = std::make_shared<muq::Modeling::Gaussian>(prop_mu, m_nonDMUQInstancePtr->proposalCovMatrix);
    // return std::make_shared<muq::SamplingAlgorithms::CrankNicolsonProposal>(pt, samplingProblem, prior);

    local_pt.put("Proposal", "MyProposal");
    local_pt.put("MyProposal.Method","AMProposal");
    local_pt.put("MyProposal.AdaptSteps", 10);
    local_pt.put("MyProposal.AdaptStart", 10);
    local_pt.put("MyProposal.AdaptScale", m_nonDMUQInstancePtr->amScale);
    return std::make_shared<muq::SamplingAlgorithms::AMProposal>(
            local_pt.get_child("MyProposal"), samplingProblem, m_nonDMUQInstancePtr->proposalCovMatrix
            );
#endif
  }

  virtual std::shared_ptr<muq::Utilities::MultiIndex> FinestIndex() override {
    // MLMCMC in MUQ is implemented internally using some of the MIMCMC classes.
    // Here, we need to pass 1 = dimension of the hierarchy, for ML this is 1 always.
    auto index = std::make_shared<muq::Utilities::MultiIndex>(1);
    // define the levels: 0,1,2,3..., such that 0 is coarsest, m_numLevels-1 is the finest
    index->SetValue(0, m_numLevels-1);
    return index;
  }

  virtual std::shared_ptr<muq::SamplingAlgorithms::MCMCProposal>
  CoarseProposal (std::shared_ptr<muq::Utilities::MultiIndex> const& fineIndex,
                  std::shared_ptr<muq::Utilities::MultiIndex> const& coarseIndex,
                  std::shared_ptr<muq::SamplingAlgorithms::AbstractSamplingProblem> const& coarseProblem,
                  std::shared_ptr<muq::SamplingAlgorithms::SingleChainMCMC> const& coarseChain) override
  {
    // this proposal drives the coarse chain by a bit and then take the final result
    // Nov. 2024: FRIZZI interacted with Linus Seelinger (author of MLMCM in MUQ) who suggested
    // to leave it as it is for the basecase
    pt::ptree pt_proposal = pt;
    pt_proposal.put("BlockIndex", 0 /*leave = 0 */);
    return std::make_shared<muq::SamplingAlgorithms::SubsamplingMIProposal>(pt_proposal, coarseProblem, coarseIndex, coarseChain);
  }

  virtual std::shared_ptr<muq::SamplingAlgorithms::AbstractSamplingProblem>
  SamplingProblem(std::shared_ptr<muq::Utilities::MultiIndex> const& index) override
  {
   const int level = index->GetValue(0 /*for MLMCMC always pass 0*/);
    if (m_nonDMUQInstancePtr->outputLevel >= DEBUG_OUTPUT) {
      Cout << "MUQ: MLMCMC: creating sampling problem for level = " << level << '\n';
    }

    // use the workgraph to create the likelihood for this level and posterior
    auto my_work_graph = std::make_shared<muq::Modeling::WorkGraph>();
    my_work_graph->AddNode(m_nonDMUQInstancePtr->parameterPtr, "Parameters");

    auto vxi = Eigen::VectorXi::Constant(1, m_nonDMUQInstancePtr->numContinuousVars);
    auto this_level_lkl = std::make_shared<MLMCMCLikelihood>(vxi, level, m_nonDMUQInstancePtr);
    my_work_graph->AddNode(this_level_lkl, "Likelihood");
    my_work_graph->AddNode(m_nonDMUQInstancePtr->MUQPriorPtr, "Prior");
    my_work_graph->AddNode(m_nonDMUQInstancePtr->posteriorPtr, "Posterior");

    my_work_graph->AddEdge("Parameters", 0, "Prior",      0); // 0 = index of input,output
    my_work_graph->AddEdge("Parameters", 0, "Likelihood", 0); // 0 = index of input,output
    my_work_graph->AddEdge("Prior",      0, "Posterior",  0);
    my_work_graph->AddEdge("Likelihood", 0, "Posterior",  1);

    auto dens = my_work_graph->CreateModPiece("Posterior");
    return std::make_shared<muq::SamplingAlgorithms::SamplingProblem>(dens);
  }

  virtual std::shared_ptr<muq::SamplingAlgorithms::MIInterpolation>
  Interpolation(std::shared_ptr<muq::Utilities::MultiIndex> const& index) override {
    // This is something that is supposed to know how to interpolate from a coarse to a finer level.
    // This is only needed if the # of calibration parameters changes across levels.
    // For example, suppose one is doing calibration on the coefficients of a
    // KL expansion and there is a level 0 with 10 KL coeffs, and level 1 has 56 KL coeffs, etc.
    // MUQ MLMCMC needs to have something that specifies what to do across levels,
    // specifically what to do at a fine level when given a propose state at a corser level.
    // For now, calibrating different params count at different levels is not supported
    // so we do not need to specify anything special: the "interpolation" is basically
    // just going to copy the values when needed, see the class for more info.
    return std::make_shared<MUQMLMCMCInterpolation>();
  }

  virtual Eigen::VectorXd
  StartingPoint (std::shared_ptr<muq::Utilities::MultiIndex> const& /*index*/) override{
    // this method is supposed to return the starting point at a given level
    // here we just use the same starting point for all level and this starting point
    // is provided to the constructor and cached.
    return m_startingPoint;
  }
};


} // namespace Dakota

#endif
