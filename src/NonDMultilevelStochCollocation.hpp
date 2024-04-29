/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_MULTILEVEL_STOCH_COLLOCATION_H
#define NOND_MULTILEVEL_STOCH_COLLOCATION_H

#include "NonDStochCollocation.hpp"


namespace Dakota {


/// Nonintrusive stochastic collocation approaches to uncertainty
/// quantification

/** The NonDMultilevelStochCollocation class uses a stochastic
    collocation (SC) approach to approximate the effect of parameter
    uncertainties on response functions of interest.  It utilizes the
    InterpPolyApproximation class to manage multidimensional Lagrange
    polynomial interpolants. */

class NonDMultilevelStochCollocation: public NonDStochCollocation
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDMultilevelStochCollocation(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor
  NonDMultilevelStochCollocation(Model& model, short exp_coeffs_approach,
    const UShortArray& num_int_seq, const RealVector& dim_pref,
    short u_space_type, short refine_type, short refine_control,
    short covar_control, short ml_alloc_cntl, short ml_discrep,
    short rule_nest, short rule_growth, bool piecewise_basis, bool use_derivs);
  /// destructor
  ~NonDMultilevelStochCollocation();

  //
  //- Heading: Virtual function redefinitions
  //

  bool resize();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void initialize_u_space_model();

  void core_run();

  int random_seed() const;
  int first_seed() const;

  void assign_specification_sequence();
  void increment_specification_sequence();

  //void combined_to_active();

  void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

private:

  //
  //- Heading: Utility functions
  //


  //
  //- Heading: Data
  //

  /// user request of quadrature order
  UShortArray quadOrderSeqSpec;
  /// user request of sparse grid level
  UShortArray ssgLevelSeqSpec;

  /// sequence index for {quadOrder,ssgLevel}SeqSpec
  size_t sequenceIndex;
};


inline int NonDMultilevelStochCollocation::random_seed() const
{ return NonDExpansion::seed_sequence(sequenceIndex); }


inline int NonDMultilevelStochCollocation::first_seed() const
{ return NonDExpansion::seed_sequence(0); }

} // namespace Dakota

#endif
