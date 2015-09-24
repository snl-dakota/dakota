/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDSparseGrid
//- Description: Wrapper class for C++ code from Pecos/packages/VPISparseGrid
//- Owner:       Mike Eldred
//- Revised by:  
//- Version:

#ifndef NOND_SPARSE_GRID_H
#define NOND_SPARSE_GRID_H

#include "dakota_data_types.hpp"
#include "DataMethod.hpp"
#include "NonDIntegration.hpp"
#include "SparseGridDriver.hpp"

namespace Dakota {


/// Derived nondeterministic class that generates N-dimensional
/// Smolyak sparse grids for numerical evaluation of expectation
/// integrals over independent standard random variables.

/** This class is used by NonDPolynomialChaos and
    NonDStochCollocation, but could also be used for general numerical
    integration of moments.  It employs 1-D Clenshaw-Curtis and Gaussian
    quadrature rules within Smolyak sparse grids. */

class NonDSparseGrid: public NonDIntegration
{
public:

  //
  //- Heading: Constructors and destructor
  //

  // alternate constructor for instantiations "on the fly"
  NonDSparseGrid(Model& model, const UShortArray& ssg_level_seq,
		 const RealVector& dim_pref,
		 short exp_coeffs_soln_approach, short driver_mode,
		 short growth_rate = Pecos::MODERATE_RESTRICTED_GROWTH,
		 //short refine_type  = Pecos::NO_REFINEMENT,
		 short refine_control = Pecos::NO_CONTROL,
		 bool track_uniq_prod_wts = true,
		 bool track_colloc_indices = true);

  //
  //- Heading: Virtual function redefinitions
  //

  /// increment ssgDriver::ssgLevel
  void increment_grid();
  /// update ssgDriver::ssgAnisoLevelWts and increment ssgDriver::ssgLevel
  /// based on specified anisotropic weighting
  void increment_grid_weights(const RealVector& aniso_wts);
  /// advance to next nevel in ssgLevelSeqSpec sequence
  void increment_specification_sequence();

  /// returns SparseGridDriver::active_multi_index()
  const std::set<UShortArray>& active_multi_index() const;

  /// invokes SparseGridDriver::print_smolyak_multi_index()
  void print_smolyak_multi_index() const;

  /// invokes SparseGridDriver::initialize_sets()
  void initialize_sets();
  /// invokes SparseGridDriver::update_reference()
  void update_reference();
  /// invokes SparseGridDriver::push_trial_set()
  void increment_set(const UShortArray& set);
  /// invokes SparseGridDriver::unique_trial_points()
  int increment_size() const;
  /// invokes SparseGridDriver::restore_set()
  void restore_set();
  /// invokes SparseGridDriver::compute_trial_grid()
  void evaluate_set();
  /// invokes SparseGridDriver::pop_trial_set()
  void decrement_set();
  /// invokes SparseGridDriver::update_sets()
  void update_sets(const UShortArray& set_star);
  /// invokes SparseGridDriver::finalize_sets()
  void finalize_sets(bool output_sets, bool converged_within_tol);

  /// invokes SparseGridDriver::evaluate_grid_increment()
  void evaluate_grid_increment();

  int num_samples() const;

protected:

  //
  //- Heading: Constructors and destructor
  //

  NonDSparseGrid(ProblemDescDB& problem_db, Model& model); ///< constructor
  ~NonDSparseGrid();                                       ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  void initialize_grid(const std::vector<Pecos::BasisPolynomial>& poly_basis);

  void get_parameter_sets(Model& model);

  //void check_variables(const Pecos::ShortArray& x_types);

  void reset();
  void sampling_reset(int min_samples, bool all_data_flag, bool stats_flag);

private:

  //
  //- Heading: Data
  //

  /// convenience pointer to the numIntDriver representation
  Pecos::SparseGridDriver* ssgDriver;

  /// the user specification for the Smolyak sparse grid level, defining a
  /// sequence of refinement levels.
  UShortArray ssgLevelSeqSpec;

  /// reference point (e.g., lower bound) for the Smolyak sparse grid level
  /// maintained within ssgDriver
  unsigned short ssgLevelRef;
};


inline void NonDSparseGrid::reset()
{
  // restore user specification state prior to any uniform/adaptive refinement
  ssgLevelRef = ssgLevelSeqSpec[sequenceIndex];
  ssgDriver->level(ssgLevelRef);
  ssgDriver->dimension_preference(dimPrefSpec);
}


inline void NonDSparseGrid::increment_specification_sequence()
{
  if (sequenceIndex+1 < ssgLevelSeqSpec.size())
    ++sequenceIndex;
  reset();
}


inline const std::set<UShortArray>& NonDSparseGrid::active_multi_index() const
{ return ssgDriver->active_multi_index(); }


inline void NonDSparseGrid::print_smolyak_multi_index() const
{ return ssgDriver->print_smolyak_multi_index(); }


inline void NonDSparseGrid::initialize_sets()
{ ssgDriver->initialize_sets(); }


inline void NonDSparseGrid::update_reference()
{ ssgDriver->update_reference(); }


inline void NonDSparseGrid::increment_set(const UShortArray& set)
{ ssgDriver->push_trial_set(set); }


inline int NonDSparseGrid::increment_size() const
{ return ssgDriver->unique_trial_points(); }


inline void NonDSparseGrid::restore_set()
{ ssgDriver->restore_set(); }


inline void NonDSparseGrid::evaluate_set()
{
  ssgDriver->compute_trial_grid(allSamples);
  evaluate_parameter_sets(iteratedModel, true, false);
  ++numIntegrations;
}


inline void NonDSparseGrid::decrement_set()
{ ssgDriver->pop_trial_set(); }


inline void NonDSparseGrid::update_sets(const UShortArray& set_star)
{ ssgDriver->update_sets(set_star); }


inline void NonDSparseGrid::
finalize_sets(bool output_sets, bool converged_within_tol)
{ ssgDriver->finalize_sets(output_sets, converged_within_tol); }


inline void NonDSparseGrid::evaluate_grid_increment()
{
  ssgDriver->compute_grid_increment(allSamples);
  evaluate_parameter_sets(iteratedModel, true, false);
  ++numIntegrations;
}


inline int NonDSparseGrid::num_samples() const
{ return ssgDriver->grid_size(); }

} // namespace Dakota

#endif
