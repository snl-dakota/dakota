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

  //
  //- Heading: Virtual function redefinitions
  //

  bool resize();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

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
  /// Perform control variate Monte Carlo across two model forms
  void control_variate_mc(const SizetSizetPair& lf_form_level,
			  const SizetSizetPair& hf_form_level);
  /// Perform multilevel Monte Carlo across levels in combination with
  /// control variate Monte Carlo across model forms at each level
  void multilevel_control_variate_mc(size_t lf_model_form,
				     size_t hf_model_form);

  /// perform a shared increment of LF and HF samples for purposes of
  /// computing/updating the evaluation ratio and the MSE ratio
  void shared_increment(size_t iter, const SizetSizetPair& lf_form_level,
			const SizetSizetPair& hf_form_level);
  /// perform final LF sample increment as indicated by the evaluation ratio
  bool lf_increment(Real avg_eval_ratio, size_t N_hf,
		    size_t& delta_N_lf, size_t& N_lf);

  /// initialize the CV bookkeeping for sums, means, variances, and
  /// covariances across fidelity levels
  void initialize_ml_sums(IntRealMatrixMap& sum_Y, size_t num_lev);
  /// initialize the CV bookkeeping for sums, means, variances, and
  /// covariances across fidelity levels
  void initialize_cv_sums_moments(IntRealVectorMap& sum_L_shared,
				  IntRealVectorMap& sum_L_refined,
				  IntRealVectorMap& sum_H,
				  IntRealVectorMap& sum_LL,
				  IntRealVectorMap& sum_LH);
  /// initialize the CV bookkeeping for sums, means, variances, and
  /// covariances across fidelity levels
  void initialize_cv_sums_moments(IntRealMatrixMap& sum_L_shared,
				  IntRealMatrixMap& sum_L_refined,
				//IntRealMatrixMap& sum_H,
				  IntRealMatrixMap& sum_LL,
				  IntRealMatrixMap& sum_LH, size_t num_lev);

  /// update running sums for one model (sum_map) using set of model
  /// evaluations within allResponses
  void accumulate_cv_sums(IntRealVectorMap& sum_map);
  /// update running sums for two models (sum_L, sum_H, and sum_LH)
  /// from set of low/high fidelity model evaluations within allResponses
  void accumulate_cv_sums(IntRealVectorMap& sum_L_shared,
			  IntRealVectorMap& sum_L_refined,
			  IntRealVectorMap& sum_H, IntRealVectorMap& sum_LL,
			  RealVector& sum_HH, IntRealVectorMap& sum_LH);
  /// update running sums for one model (sum_map) using set of model
  /// evaluations within allResponses
  void accumulate_cv_sums(IntRealMatrixMap& sum_map, size_t lev);
  /// update running sums for two models (sum_L, sum_H, and sum_LH)
  /// from set of low/high fidelity model evaluations within lf/hf_resp_map
  void accumulate_cv_sums(const IntResponseMap& lf_resp_map,
			  const IntResponseMap& hf_resp_map,
			  IntRealMatrixMap& sum_L_shared,
			  IntRealMatrixMap& sum_L_refined,
			//IntRealMatrixMap& sum_H,
			  IntRealMatrixMap& sum_LL, //RealMatrix& sum_HH,
			  IntRealMatrixMap& sum_LH, size_t lev);

  /// update accumulators for multilevel telescoping running sums
  /// using set of model evaluations within allResponses
  void accumulate_ml_sums(IntRealMatrixMap& sum_Y, RealMatrix& sum_YY,
			  size_t lev);

  /// compute the LF/HF evaluation ratio, averaged over the QoI
  Real eval_ratio(const RealVector& sum_L_shared, const RealVector& sum_H,
		  const RealVector& sum_LL,       const RealVector& sum_HH,
		  const RealVector& sum_LH,       Real cost_ratio,
		  RealVector& var_H, RealVector& rho2_LH, size_t N_shared);
  /// compute the LF/HF evaluation ratio, averaged over the QoI
  Real eval_ratio(RealMatrix& sum_L_shared, RealMatrix& sum_H,
		  RealMatrix& sum_LL, RealMatrix& sum_HH, RealMatrix& sum_LH,
		  Real cost_ratio, size_t lev, RealMatrix& var_H,
		  RealMatrix& rho2_LH, size_t N_shared);
  /// compute ratio of MC and CVMC mean squared errors, averaged over the QoI
  Real MSE_ratio(Real avg_eval_ratio, const RealVector& var_H,
		 const RealVector& rho2_LH, size_t iter, size_t N_hf);

  /// compute control variate parameter and estimate raw moments
  void cv_raw_moments(IntRealVectorMap& sum_L_shared,
		      IntRealVectorMap& sum_L_refined, IntRealVectorMap& sum_H,
		      IntRealVectorMap& sum_LL,        IntRealVectorMap& sum_LH,
		      const RealVector& rho2_LH,     //Real cost_ratio,
		      size_t N_shared, size_t N_refined, RealMatrix& H_raw_mom);
  /// compute control variate parameter and estimate raw moments
  void cv_raw_moments(IntRealMatrixMap& sum_L_shared,
		      IntRealMatrixMap& sum_L_refined, IntRealMatrixMap& sum_H,
		      IntRealMatrixMap& sum_LL,        IntRealMatrixMap& sum_LH,
		      const RealMatrix& rho2_LH,     //Real cost_ratio,
		      size_t lev, size_t N_shared, size_t N_refined,
		      RealMatrix& H_raw_mom);

  /// compute average of a set of observations
  Real average(const RealVector& vec) const;
  /// compute average of a set of observations
  Real average(const Real* vec, size_t vec_len) const;

  /// convert uncentered raw moments (multilevel expectations) to
  /// standardized moments
  void convert_moments(const RealMatrix& raw_moments,
		       RealMatrix& standard_moments);

  //
  //- Heading: Data
  //

  /// total number of samples performed per model form, per discretization
  /// level, or both
  Sizet2DArray NLev;
  /// increment in number of samples required for each model form,
  /// discretization level, or both
  Sizet2DArray deltaNLev;
  
  /// mean squared error of mean estimator from pilot sample MC on HF model
  RealVector mcMSEIter0;
  /// equivalent number of high fidelity evaluations accumulated using samples
  /// across multiple model forms and/or discretization levels
  Real equivHFEvals;
};


inline Real NonDMultilevelSampling::
eval_ratio(RealMatrix& sum_L_shared, RealMatrix& sum_H, RealMatrix& sum_LL,
	   RealMatrix& sum_HH, RealMatrix& sum_LH, Real cost_ratio, size_t lev,
	   RealMatrix& var_H, RealMatrix& rho2_LH, size_t N_shared)
{
  RealVector var_H_l(Teuchos::View,   var_H[lev], numFunctions),
           rho2_LH_l(Teuchos::View, rho2_LH[lev], numFunctions);
  int i_lev = (int)lev;
  return eval_ratio(getCol(Teuchos::View, sum_L_shared, i_lev),
		    getCol(Teuchos::View, sum_H,  i_lev),
		    getCol(Teuchos::View, sum_LL, i_lev),
		    getCol(Teuchos::View, sum_HH, i_lev),
		    getCol(Teuchos::View, sum_LH, i_lev), cost_ratio,
		    var_H_l, rho2_LH_l, N_shared);
}


inline Real NonDMultilevelSampling::average(const RealVector& vec) const
{
  Real avg = 0.;
  size_t i, vec_len = vec.length();
  for (i=0; i<vec_len; ++i)
    avg += vec[i];
  return avg / vec_len;
}


inline Real NonDMultilevelSampling::
average(const Real* vec, size_t vec_len) const
{
  Real avg = 0.;
  for (size_t i=0; i<vec_len; ++i)
    avg += vec[i];
  return avg / vec_len;
}

} // namespace Dakota

#endif
