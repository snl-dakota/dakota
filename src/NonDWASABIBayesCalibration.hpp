/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDWASABIBayesCalibration
//- Description: Derived class for WASABI-based Bayesian inference
//- Owner:       Tim Wildey
//- Checked by:
//- Version:

#ifndef NOND_WASABI_BAYES_CALIBRATION_H
#define NOND_WASABI_BAYES_CALIBRATION_H

#include "NonDBayesCalibration.hpp"
#include "GaussianKDE.hpp"
// for uniform PDF
#include <boost/math/distributions/uniform.hpp>
#include <boost/math/distributions/normal.hpp>
// for uniform samples (uniform_real is deprecated)
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real.hpp>

namespace Dakota {


/// WASABI - Weighted Adaptive Surrogate Approximations for Bayesian Inference 

/** This class performs Bayesian calibration using the WASABI approach
    */

class NonDWASABIBayesCalibration: public NonDBayesCalibration
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDWASABIBayesCalibration(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDWASABIBayesCalibration();

  //
  //- Heading: Static callback functions required by WASABI
  //

  /// initializer for problem size characteristics in WASABI
  static void 
  problem_size (int &chain_num, int &cr_num, int &gen_num, int &pair_num, 
		int &par_num);

  /// Filename and data initializer for WASABI
  static void 
  problem_value (std::string *chain_filename, std::string *gr_filename,
		 double &gr_threshold, int &jumpstep, double limits[], 
		 int par_num, int &printstep, std::string *restart_read_filename, 
		 std::string *restart_write_filename);

  /// Compute the prior density at specified point zp
  static double prior_density (int par_num, double zp[]);

  // NOTE: Memory is freed inside the dream core
  /// Sample the prior and return an array of parameter values
  //static double* prior_sample (int par_num);

  void prior_sample ( RealVector & sample);
         
  void compute_responses(RealMatrix & samples, RealMatrix & responses);

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  /// redefined from DakotaNonD
  void quantify_uncertainty();
  // redefined from DakotaNonD
  void print_results(std::ostream& s);

  //
  //- Heading: Data

  /// The mean of the multivariate Gaussian distribution of the obs. data
  RealVector dataDistMeans;
  /// The covariance of the multivariate Gaussian distribution of the obs. data
  RealVector dataDistCovariance;
  /// The filename of the file containing the data that with density estimator
  /// defines the distribution of the obs. data
  std::string dataDistFilename;
  /// The type of covariance data provided ("diagonal","matrix")
  std::string dataDistCovType;

  /// lower bounds on calibrated parameters
  RealVector paramMins;
  /// upper bounds on calibrated parameters
  RealVector paramMaxs;

  /// uniform prior PDFs for each variable
  std::vector<boost::math::uniform> priorDistributions;
  /// random number engine for sampling the prior
  boost::mt19937 rnumGenerator;
  /// samplers for the uniform prior PDFs for each variable
  std::vector<boost::uniform_real<double> > priorSamplers;

private:

  //
  // - Heading: Data
  // 

  /// Pointer to current class instance for use in static callback functions
  static NonDWASABIBayesCalibration* NonDWASABIInstance;
  
};

} // namespace Dakota

#endif
