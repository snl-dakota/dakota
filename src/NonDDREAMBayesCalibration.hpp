/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDDREAMBayesCalibration
//- Description: Derived class for DREAM-based Bayesian inference
//- Owner:       Brian Adams
//- Checked by:
//- Version:

#ifndef NOND_DREAM_BAYES_CALIBRATION_H
#define NOND_DREAM_BAYES_CALIBRATION_H

#include "NonDBayesCalibration.hpp"
#include "dakota_mersenne_twister.hpp"

namespace Dakota {


/// Bayesian inference using the DREAM approach 

/** This class performed Bayesian calibration using the DREAM (Markov
    Chain Monte Carlo acceleration by Differential Evolution)
    implementation of John Burkhardt (FSU), adapted from that of
    Guannan Zhang (ORNL) */

class NonDDREAMBayesCalibration: public NonDBayesCalibration
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDDREAMBayesCalibration(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDDREAMBayesCalibration();

  //
  //- Heading: Static callback functions required by DREAM
  //

  /// initializer for problem size characteristics in DREAM
  static void 
  problem_size (int &chain_num, int &cr_num, int &gen_num, int &pair_num, 
		int &par_num);

  /// Filename and data initializer for DREAM
  static void 
  problem_value (std::string *chain_filename, std::string *gr_filename,
		 double &gr_threshold, int &jumpstep, double limits[], 
		 int par_num, int &printstep, std::string *restart_read_filename, 
		 std::string *restart_write_filename);

  /// Compute the prior density at specified point zp
  static double prior_density (int par_num, double zp[]);

  // NOTE: Memory is freed inside the dream core
  /// Sample the prior and return an array of parameter values
  static double* prior_sample (int par_num);

  // Called by chain_init and dream_algm
  //   par_num: number of parameters
  //   zp:      point at which to sample the likelihood
  //   returns: real valued log-likelihood
  /// Likelihood function for call-back from DREAM to DAKOTA for evaluation
  static double sample_likelihood (int par_num, double zp[]);
         
protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void calibrate();
  //void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

  // Member functions

  /// Callback to archive the chain from DREAM, potentially leaving it in u-space
  static void cache_chain(const double* const z);
  /// save the final x-space acceptance chain and corresponding function values
  void archive_acceptance_chain();

  //
  //- Heading: Data

  /// lower bounds on calibrated parameters
  RealVector paramMins;
  /// upper bounds on calibrated parameters
  RealVector paramMaxs;

  // DREAM Algorithm controls

  /// number of concurrent chains
  int numChains;
  /// number of generations
  int numGenerations;
  /// number of CR-factors
  int numCR;
  /// number of crossover chain pairs
  int crossoverChainPairs;
  /// threshold for the Gelmin-Rubin statistic
  Real grThreshold;
  /// how often to perform a long jump in generations
  int jumpStep;

  /// random number engine for sampling the prior
  boost::mt19937 rnumGenerator;

private:

  //
  // - Heading: Data
  // 

  /// Pointer to current class instance for use in static callback functions
  static NonDDREAMBayesCalibration* nonDDREAMInstance;
  
};

} // namespace Dakota

#endif
