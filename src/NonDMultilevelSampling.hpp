/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDSampling
//- Description: Wrapper class for Fortran 90 LHS library
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_MULTILEVEL_SAMPLING_H
#define NOND_MULTILEVEL_SAMPLING_H

#include "NonDSampling.hpp"
#include "DataMethod.hpp"


namespace Dakota {

/// Performs Multilevel Monte Carlo sampling for uncertainty quantification.

/** Multilevel Monte Carlo (MLMC) is a variance-reduction technique
    that utilitizes lower fidelity simulations that have response QoI
    that are correlated with the high-fidelity response QoI. */

class NonDMultilevelSampling: public NonDSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDMultilevelSampling(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDMultilevelSampling();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void resize();
  void pre_run();
  void core_run();
  void post_run(std::ostream& s);
  void print_results(std::ostream& s);

private:

  //
  //- Heading: Helper functions
  //

  /// Perform multilevel Monte Carlo across the discretization levels for a
  /// particular model form
  void multilevel_mc(size_t model_form);

  /// Perform control variate Monte Carlo across two model forms for a
  /// particular discretization level
  void control_variate_mc(size_t lf_model_form, size_t hf_model_form,
			  size_t soln_level);
  /// update running sums (sum_L, sum_H, and sum_LH) from (matched) set
  /// of LF and HF evaluations
  void accumulate_sums(IntRealVectorMap& sum_L, IntRealVectorMap& sum_H,
		       IntRealVectorMap& sum_LH);
  /// compute the LF/HF evaluation ratio, averaged over the QoI
  Real eval_ratio(const IntRealVectorMap& sum_L, const IntRealVectorMap& sum_H,
		  const IntRealVectorMap& sum_LH, size_t total_N,
		  Real cost_ratio, RealVector& mean_L, RealVector& mean_H,
		  RealVector& var_L, RealVector& var_H, RealVector& covar_LH,
		  RealVector& rho2_LH);
  /// compute ratio of MC and CVMC mean squared errors, averaged over the QoI
  Real MSE_ratio(Real avg_eval_ratio, const RealVector& var_H,
		 const RealVector& rho2_LH, size_t N_hf, size_t iter);

  /// convert uncentered raw moments (multilevel expectations) to
  /// standardized moments
  void convert_moments(const RealMatrix& raw_moments,
		       RealMatrix& standard_moments);

  //
  //- Heading: Data
  //

  /// number of pilot samples to perform per level, to initialize the iteration
  SizetArray pilotSamples;
  /// mean squared error of mean estimator from pilot sample MC on HF model
  RealVector mcMSEIter0;
};



} // namespace Dakota

#endif
