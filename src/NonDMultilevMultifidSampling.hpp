/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDHierarchSampling
//- Description: class for multilevel-control variate sampling
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_MULTILEV_MULTIFID_SAMPLING_H
#define NOND_MULTILEV_MULTIFID_SAMPLING_H

#include "NonDHierarchSampling.hpp"
#include "DataMethod.hpp"

namespace Dakota {

/// Performs MultilevMultifid Monte Carlo sampling for uncertainty quantification.

/** MultilevMultifid Monte Carlo (MLMC) is a variance-reduction technique
    that utilitizes lower fidelity simulations that have response QoI
    that are correlated with the high-fidelity response QoI. */

class NonDMultilevMultifidSampling: public NonDHierarchSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDMultilevMultifidSampling(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDMultilevMultifidSampling();

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
  void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

private:

  //
  //- Heading: Helper functions
  //

  /// Perform multilevel Monte Carlo across levels in combination with
  /// control variate Monte Carlo across model forms at each level; CV
  /// computes correlations for Y (LH correlations for level discrepancies)
  void multilevel_control_variate_mc_Ycorr(unsigned short lf_model_form,
					   unsigned short hf_model_form);
  /// Perform multilevel Monte Carlo across levels in combination with
  /// control variate Monte Carlo across model forms at each level; CV
  /// computes correlations for Q (LH correlations for QoI)
  void multilevel_control_variate_mc_Qcorr(unsigned short lf_model_form,
					   unsigned short hf_model_form);

  /// initialize the MLCV accumulators for computing means, variances, and
  /// covariances across fidelity levels
  void initialize_mlcv_sums(IntRealMatrixMap& sum_L_shared,
			    IntRealMatrixMap& sum_L_refined,
			    IntRealMatrixMap& sum_H,
			    IntRealMatrixMap& sum_LL, IntRealMatrixMap& sum_LH,
			    IntRealMatrixMap& sum_HH, size_t num_ml_lev,
			    size_t num_cv_lev);
  /// initialize the MLCV accumulators for computing means, variances, and
  /// covariances across fidelity levels
  void initialize_mlcv_sums(IntRealMatrixMap& sum_Ll,
			    IntRealMatrixMap& sum_Llm1,
			    IntRealMatrixMap& sum_Ll_refined,
			    IntRealMatrixMap& sum_Llm1_refined,
			    IntRealMatrixMap& sum_Hl,
			    IntRealMatrixMap& sum_Hlm1,
			    IntRealMatrixMap& sum_Ll_Ll,
			    IntRealMatrixMap& sum_Ll_Llm1,
			    IntRealMatrixMap& sum_Llm1_Llm1,
			    IntRealMatrixMap& sum_Hl_Ll,
			    IntRealMatrixMap& sum_Hl_Llm1,
			    IntRealMatrixMap& sum_Hlm1_Ll,
			    IntRealMatrixMap& sum_Hlm1_Llm1,
			    IntRealMatrixMap& sum_Hl_Hl,
			    IntRealMatrixMap& sum_Hl_Hlm1,
			    IntRealMatrixMap& sum_Hlm1_Hlm1,
			    size_t num_ml_lev, size_t num_cv_lev);

  /// update running QoI sums for one model at two levels (sum_Ql, sum_Qlm1)
  /// using set of model evaluations within allResponses
  void accumulate_mlcv_Qsums(IntRealMatrixMap& sum_Ql,
			     IntRealMatrixMap& sum_Qlm1, size_t lev,
			     const RealVector& offset, SizetArray& num_Q);
  /// update running discrepancy sums for one model (sum_Y) using
  /// set of model evaluations within allResponses
  void accumulate_mlcv_Ysums(IntRealMatrixMap& sum_Y, size_t lev,
			     const RealVector& offset, SizetArray& num_Y);
  /// update running QoI sums for two models (sum_L, sum_H, sum_LL, sum_LH,
  /// and sum_HH) from set of low/high fidelity model evaluations within
  /// {lf,hf}_resp_map; used for level 0 from other accumulators
  void accumulate_mlcv_Qsums(const IntResponseMap& lf_resp_map,
			     const IntResponseMap& hf_resp_map,
			     IntRealMatrixMap& sum_L_shared,
			     IntRealMatrixMap& sum_L_refined,
			     IntRealMatrixMap& sum_H,  IntRealMatrixMap& sum_LL,
			     IntRealMatrixMap& sum_LH, IntRealMatrixMap& sum_HH,
			     size_t lev, const RealVector& lf_offset,
			     const RealVector& hf_offset, SizetArray& num_L,
			     SizetArray& num_H);
  /// update running two-level discrepancy sums for two models (sum_L,
  /// sum_H, sum_LL, sum_LH, and sum_HH) from set of low/high fidelity
  /// model evaluations within {lf,hf}resp_map
  void accumulate_mlcv_Ysums(const IntResponseMap& lf_resp_map,
			     const IntResponseMap& hf_resp_map,
			     IntRealMatrixMap& sum_L_shared,
			     IntRealMatrixMap& sum_L_refined,
			     IntRealMatrixMap& sum_H,  IntRealMatrixMap& sum_LL,
			     IntRealMatrixMap& sum_LH, IntRealMatrixMap& sum_HH,
			     size_t lev, const RealVector& lf_offset,
			     const RealVector& hf_offset,
			     SizetArray& num_L, SizetArray& num_H);
  /// update running QoI sums for two models and two levels from set
  /// of low/high fidelity model evaluations within {lf,hf}_resp_map
  void accumulate_mlcv_Qsums(const IntResponseMap& lf_resp_map,
			     const IntResponseMap& hf_resp_map,
			     IntRealMatrixMap& sum_Ll,
			     IntRealMatrixMap& sum_Llm1,
			     IntRealMatrixMap& sum_Ll_refined,
			     IntRealMatrixMap& sum_Llm1_refined,
			     IntRealMatrixMap& sum_Hl,
			     IntRealMatrixMap& sum_Hlm1,
			     IntRealMatrixMap& sum_Ll_Ll,
			     IntRealMatrixMap& sum_Ll_Llm1,
			     IntRealMatrixMap& sum_Llm1_Llm1,
			     IntRealMatrixMap& sum_Hl_Ll,
			     IntRealMatrixMap& sum_Hl_Llm1,
			     IntRealMatrixMap& sum_Hlm1_Ll,
			     IntRealMatrixMap& sum_Hlm1_Llm1,
			     IntRealMatrixMap& sum_Hl_Hl,
			     IntRealMatrixMap& sum_Hl_Hlm1,
			     IntRealMatrixMap& sum_Hlm1_Hlm1, size_t lev,
			     const RealVector& lf_offset,
			     const RealVector& hf_offset,
			     SizetArray& num_L, SizetArray& num_H);

  //
  //- Heading: Data
  //

  /// if defined, complete the final CV refinement when terminating MLCV based
  /// on maxIterations (the total number of refinements beyond the pilot sample
  /// will be one more for CV than for ML).  This approach is consistent with
  /// normal termination based on l1_norm(delta_N_hf) = 0.
  bool finalCVRefinement;
};

} // namespace Dakota

#endif
