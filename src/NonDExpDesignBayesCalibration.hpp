/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDExpDesignBayesCalibration
//- Description: Derived class for ExpDesignive Bayesian inference
//- Owner:       Laura Swiler and Kathryn Maupin
//- Checked by:
//- Version:

#ifndef NOND_EXPDESIGN_BAYES_CALIBRATION_H
#define NOND_EXPDESIGN_BAYES_CALIBRATION_H

#include "NonDBayesCalibration.hpp"
#include "NonDExpansion.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "DakotaModel.hpp"
#include "PRPMultiIndex.hpp"
#include "NonDQUESOBayesCalibration.hpp"

namespace Dakota {

/** This class performs adaptive Bayesian calibration.  A high fidelity model is 
     evaluated as the "truth" model over an initial set of LHS points.  These are 
     sent to the low fidelity, which uses QUESO to calibrate the low fidelity model 
     based on the high fidelity points.  The mutual information from the posterior 
     of the low fidelity calibration is used as an iterative stopping criteria. 
    */

class NonDExpDesignBayesCalibration: public NonDBayesCalibration
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDExpDesignBayesCalibration(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDExpDesignBayesCalibration();


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

  /// number of initial samples where the high-fidelity model should be evaluated
  int initSamples;
  /// pointer to the low fidelity model which will be calibrated against high fidelity data
  ///const String& lowFidModelPtr;

private:

  //
  // - Heading: Data
  // 

};

} // namespace Dakota

#endif
