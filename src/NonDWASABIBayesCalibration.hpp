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

  void compute_responses(RealMatrix & samples, RealMatrix & responses);

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void calibrate();
  void print_results(std::ostream& s);

  /// Extract a subset of samples for posterior eval according to the
  /// indices in points_to_keep
  void extract_selected_posterior_samples(const std::vector<int> &points_to_keep,
				   const RealMatrix &samples_for_posterior_eval,
				   const RealVector &posterior_density,
				   RealMatrix &posterior_data ) const;

  /// Export posterior_data to file
  void export_posterior_samples_to_file( const std::string filename, 
					 const RealMatrix &posterior_data) const;
  

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
  /// The filename of the import file containing samples at which the 
  /// posterior will be evaluated
  std::string posteriorSamplesImportFile;
  /// Format of imported posterior samples file
  unsigned short posteriorSamplesImportFormat;
  /// The filename of the export file containing an arbitrary set of samples and 
  /// their corresponding density values
  std::string exportPosteriorDensityFile;
  /// The filename of the export file containing samples from the posterior and 
  /// their corresponding density values
  std::string exportPosteriorSamplesFile;
  /// Format of imported posterior samples and values file
  unsigned short exportFileFormat;
  /// Flag specifying whether to generate random samples from the posterior
  bool generateRandomPosteriorSamples;
  /// Flag specifying whether to evaluate the posterior density at a 
  /// set of samples
  bool evaluatePosteriorDensity;

  /// lower bounds on calibrated parameters
  RealVector paramMins;
  /// upper bounds on calibrated parameters
  RealVector paramMaxs;

  /// random number engine for sampling the prior
  boost::mt19937 rnumGenerator;

private:

  //
  // - Heading: Data
  // 

};

} // namespace Dakota

#endif
