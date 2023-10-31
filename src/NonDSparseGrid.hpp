/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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
  NonDSparseGrid(Model& model, unsigned short ssg_level,
		 const RealVector& dim_pref,
		 short exp_coeffs_soln_approach, short driver_mode,
		 short growth_rate = Pecos::MODERATE_RESTRICTED_GROWTH,
		 //short refine_type  = Pecos::NO_REFINEMENT,
		 short refine_control = Pecos::NO_CONTROL,
		 bool track_uniq_prod_wts = true);

  ~NonDSparseGrid();                                       ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  /// update the sparse grid level (e.g., from a level sequence)
  void sparse_grid_level(unsigned short ssg_level);

  /// increment ssgDriver::ssgLevel
  void increment_grid();
  /// update ssgDriver::ssgAnisoLevelWts and increment ssgDriver::ssgLevel
  /// based on specified anisotropic weighting
  void increment_grid_weights(const RealVector& aniso_wts);
  /// increment ssgDriver::ssgLevel based on existing anisotropic weighting
  void increment_grid_weights();
  /// decrement ssgDriver::ssgLevel
  void decrement_grid();

  void evaluate_grid_increment();
  void push_grid_increment();
  void pop_grid_increment();
  void merge_grid_increment();

  /// reset ssgDriver level and dimension preference back to
  /// {ssgLevel,dimPref}Spec for the active key, following refinement
  /// or sequence advancement
  void reset();
  /// blow away all data for all keys
  void reset_all();

  /// returns SparseGridDriver::active_multi_index()
  const std::set<UShortArray>& active_multi_index() const;

  /// invokes SparseGridDriver::print_smolyak_multi_index()
  void print_smolyak_multi_index() const;

  /// invokes SparseGridDriver::initialize_sets()
  void initialize_sets();
  /// invokes SparseGridDriver::update_reference()
  void update_reference();
  /// invokes SparseGridDriver::increment_smolyak_multi_index()
  void increment_set(const UShortArray& set);
  /// invokes SparseGridDriver::unique_trial_points()
  int increment_size() const;
  /// invokes SparseGridDriver::push_set()
  void push_set();
  /// invokes SparseGridDriver::compute_trial_grid()
  void evaluate_set();
  /// invokes SparseGridDriver::pop_set()
  void decrement_set();
  /// invokes SparseGridDriver::update_sets()
  void update_sets(const UShortArray& set_star);
  /// invokes SparseGridDriver::finalize_sets()
  void finalize_sets(bool output_sets, bool converged_within_tol,bool reverted);

  size_t num_samples() const;

protected:

  //
  //- Heading: Constructors and destructor
  //

  NonDSparseGrid(ProblemDescDB& problem_db, Model& model); ///< constructor

  //
  //- Heading: Virtual function redefinitions
  //

  void initialize_grid(const std::vector<Pecos::BasisPolynomial>& poly_basis);

  void get_parameter_sets(Model& model);

  //void check_variables(const Pecos::ShortArray& x_types);

  void sampling_reset(size_t min_samples, bool all_data_flag, bool stats_flag);

  //
  //- Heading: Member functions
  //

  const RealVector& anisotropic_weights() const;

private:

  //
  //- Heading: Data
  //

  /// type of sparse grid driver: combined, incremental, hierarchical, ...
  short ssgDriverType;
  /// convenience pointer to the numIntDriver representation
  std::shared_ptr<Pecos::SparseGridDriver> ssgDriver;

  /// the user specification for the Smolyak sparse grid level, rendered
  /// anisotropic via dimPrefSpec
  unsigned short ssgLevelSpec;
  /// value of ssgDriver->level() prior to increment_grid(), for restoration
  /// in decrement_grid() since increment must induce a change in grid size
  /// and this adaptive increment in not reversible
  unsigned short ssgLevelPrev;
};


inline void NonDSparseGrid::increment_grid_weights()
{ increment_grid_weights(ssgDriver->anisotropic_weights()); }


inline void NonDSparseGrid::sparse_grid_level(unsigned short ssg_level)
{ ssgLevelSpec = ssg_level; reset(); }


inline void NonDSparseGrid::reset()
{
  // reset the grid for the current active key to its original user spec,
  // prior to any grid refinement
  // > also invokes SparseGridDriver::clear_size() if change is induced
  // > updates to other keys are managed by {assign,increment}_specification_
  //   sequence() in multilevel expansion methods
  ssgDriver->level(ssgLevelSpec);
  ssgDriver->dimension_preference(dimPrefSpec);

  // Clear grid size (may vary with either dist param change or grid
  // level/anisotropy) and clear dist param update trackers
  ssgDriver->reset();

  // This fn does not clear history, such as accumulated 1D pts/wts -->
  // reset_all() or reset_1d_collocation_points_weights() should be used
  // when distribution param updates invalidate this history
}


inline void NonDSparseGrid::reset_all()
{
  // This "nuclear option" is not currently used

  ssgDriver->clear_keys();
  ssgDriver->update_active_iterators();
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
{ ssgDriver->increment_smolyak_multi_index(set); }


inline int NonDSparseGrid::increment_size() const
{ return ssgDriver->unique_trial_points(); }


inline void NonDSparseGrid::push_set()
{ ssgDriver->push_set(); }


inline void NonDSparseGrid::evaluate_set()
{
  ssgDriver->compute_trial_grid(allSamples);
  evaluate_parameter_sets(iteratedModel, true, false);
  ++numIntegrations;
}


inline void NonDSparseGrid::decrement_set()
{ ssgDriver->pop_set(); }


inline void NonDSparseGrid::update_sets(const UShortArray& set_star)
{ ssgDriver->update_sets(set_star); }


inline void NonDSparseGrid::
finalize_sets(bool output_sets, bool converged_within_tol, bool reverted)
{ ssgDriver->finalize_sets(output_sets, converged_within_tol, reverted); }


inline void NonDSparseGrid::evaluate_grid_increment()
{
  ssgDriver->compute_increment(allSamples);
  evaluate_parameter_sets(iteratedModel, true, false);
  ++numIntegrations;
}


inline void NonDSparseGrid::push_grid_increment()
{ ssgDriver->push_increment(); }


inline void NonDSparseGrid::pop_grid_increment()
{ ssgDriver->pop_increment(); }


inline void NonDSparseGrid::merge_grid_increment()
{ ssgDriver->merge_unique(); }


inline size_t NonDSparseGrid::num_samples() const
{ return ssgDriver->grid_size(); }


inline const RealVector& NonDSparseGrid::anisotropic_weights() const
{ return ssgDriver->anisotropic_weights(); }

} // namespace Dakota

#endif
