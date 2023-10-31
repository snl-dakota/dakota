/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_MULTILEVEL_POLYNOMIAL_CHAOS_H
#define NOND_MULTILEVEL_POLYNOMIAL_CHAOS_H

#include "NonDPolynomialChaos.hpp"

namespace Dakota {

/// Nonintrusive polynomial chaos expansion approaches to uncertainty
/// quantification

/** The NonDMultilevelPolynomialChaos class uses a polynomial chaos
    expansion (PCE) approach to approximate the effect of parameter
    uncertainties on response functions of interest.  It utilizes the
    OrthogPolyApproximation class to manage multiple types of
    orthogonal polynomials within a Wiener-Askey scheme to PCE.  It
    supports PCE coefficient estimation via sampling, quadrature,
    point-collocation, and file import. */

class NonDMultilevelPolynomialChaos: public NonDPolynomialChaos
{
public:

  //
  //- Heading: Constructors and destructor
  //
 
  /// standard constructor
  NonDMultilevelPolynomialChaos(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor for numerical integration (tensor, sparse, cubature)
  NonDMultilevelPolynomialChaos(/*unsigned short method_name,*/ Model& model,
				short exp_coeffs_approach,
				const UShortArray& num_int_seq,
				const RealVector& dim_pref, short u_space_type,
				short refine_type, short refine_control,
				short covar_control, short ml_alloc_cntl,
				short ml_discrep, short rule_nest,
				short rule_growth, bool piecewise_basis,
				bool use_derivs);
  /// alternate constructor for regression (least squares, CS, OLI)
  NonDMultilevelPolynomialChaos(unsigned short method_name, Model& model,
				short exp_coeffs_approach,
				const UShortArray& exp_order_seq,
				const RealVector& dim_pref,
				const SizetArray& colloc_pts_seq,
				Real colloc_ratio, const SizetArray& seed_seq,
				short u_space_type, short refine_type,
				short refine_control, short covar_control,
				short ml_alloc_cntl, short ml_discrep,
				//short rule_nest, short rule_growth,
				bool piecewise_basis, bool use_derivs,
				bool cv_flag,
				const String& import_build_pts_file,
				unsigned short import_build_format,
				bool import_build_active_only);
  /// destructor
  ~NonDMultilevelPolynomialChaos();

  //
  //- Heading: Virtual function redefinitions
  //

  //bool resize();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void initialize_u_space_model();
  void core_run();

  void assign_specification_sequence();
  void increment_specification_sequence();

  size_t collocation_points() const;
  int random_seed() const;
  int first_seed() const;

  void initialize_ml_regression(size_t num_lev, bool& import_pilot);
  void infer_pilot_sample(/*Real ratio, */size_t num_steps,
			  SizetArray& delta_N_l);
  void increment_sample_sequence(size_t new_samp, size_t total_samp,
				 size_t step);
  void compute_sample_increment(const RealVector& sparsity,
				const SizetArray& N_l, SizetArray& delta_N_l);

  void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

  //
  //- Heading: Member functions
  //

  /// assign defaults related to allocation control (currently for ML
  /// regression approaches)
  void assign_allocation_control();

private:

  //
  //- Heading: Utility functions
  //

  size_t expansion_samples(size_t index) const;
  unsigned short expansion_order(size_t index) const;
  unsigned short quadrature_order(size_t index) const;
  unsigned short sparse_grid_level(size_t index) const;

  size_t expansion_samples() const;
  unsigned short expansion_order() const;
  unsigned short quadrature_order() const;
  unsigned short sparse_grid_level() const;

  /// perform specification updates (shared code from
  // {assign,increment}_specification_sequence())
  void update_from_specification(bool update_exp, bool update_sampler,
				 bool update_from_ratio);

  //
  //- Heading: Data
  //

  /// user specification for expansion_order (array for multifidelity)
  UShortArray expOrderSeqSpec;
  /// user specification for expansion_samples (array for multifidelity)
  SizetArray expSamplesSeqSpec;
  /// user request of quadrature order
  UShortArray quadOrderSeqSpec;
  /// user request of sparse grid level
  UShortArray ssgLevelSeqSpec;

  /// sequence index for {...}SeqSpec
  size_t sequenceIndex;
};


inline size_t NonDMultilevelPolynomialChaos::collocation_points() const
{ return NonDExpansion::collocation_points(sequenceIndex); }


inline int NonDMultilevelPolynomialChaos::random_seed() const
{ return NonDExpansion::seed_sequence(sequenceIndex); }


inline int NonDMultilevelPolynomialChaos::first_seed() const
{ return NonDExpansion::seed_sequence(0); }


inline size_t NonDMultilevelPolynomialChaos::
expansion_samples(size_t index) const
{
  if (expSamplesSeqSpec.empty()) return SZ_MAX;
  else
    return (index < expSamplesSeqSpec.size()) ?
      expSamplesSeqSpec[index] : expSamplesSeqSpec.back();
}


inline unsigned short NonDMultilevelPolynomialChaos::
expansion_order(size_t index) const
{
  if (expOrderSeqSpec.empty()) return USHRT_MAX;
  else
    return (index < expOrderSeqSpec.size()) ?
      expOrderSeqSpec[index] : expOrderSeqSpec.back();
}


inline unsigned short NonDMultilevelPolynomialChaos::
quadrature_order(size_t index) const
{
  if (quadOrderSeqSpec.empty()) return USHRT_MAX;
  else
    return (index < quadOrderSeqSpec.size()) ?
      quadOrderSeqSpec[index] : quadOrderSeqSpec.back();
}


inline unsigned short NonDMultilevelPolynomialChaos::
sparse_grid_level(size_t index) const
{
  if (ssgLevelSeqSpec.empty()) return USHRT_MAX;
  else
    return (index < ssgLevelSeqSpec.size()) ?
      ssgLevelSeqSpec[index] : ssgLevelSeqSpec.back();
}


inline size_t NonDMultilevelPolynomialChaos::expansion_samples() const
{ return expansion_samples(sequenceIndex); }


inline unsigned short NonDMultilevelPolynomialChaos::expansion_order() const
{ return expansion_order(sequenceIndex); }


inline unsigned short NonDMultilevelPolynomialChaos::quadrature_order() const
{ return quadrature_order(sequenceIndex); }


inline unsigned short NonDMultilevelPolynomialChaos::sparse_grid_level() const
{ return sparse_grid_level(sequenceIndex); }

} // namespace Dakota

#endif
