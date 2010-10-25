/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2001, Sandia National Laboratories.
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

#include "data_types.h"
#include "DataMethod.H"
#include "NonDIntegration.H"
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
  NonDSparseGrid(Model& model, const Pecos::ShortArray& u_types,
		 unsigned short ssg_level, const RealVector& dim_pref,
		 //const String& sparse_grid_usage,
		 bool  nested_rules   = true,
		 short refine_type    = Pecos::NO_REFINEMENT,
		 short refine_control = Pecos::GENERALIZED_SPARSE);

  //
  //- Heading: Virtual function redefinitions
  //

  /// increment ssgDriver::ssgLevel
  void increment_grid();
  /// update ssgDriver::ssgLevel and ssgDriver::ssgAnisoLevelWts
  void increment_grid(const RealVector& dim_pref);

  /// returns SparseGridDriver::active_multi_index()
  const std::set<UShortArray>& active_multi_index() const;
  /// returns SparseGridDriver::old_multi_index()
  const std::set<UShortArray>& old_multi_index() const;
  /// returns SparseGridDriver::smolyak_multi_index()
  const UShort2DArray& smolyak_multi_index() const;
  /// returns SparseGridDriver::smolyak_coefficients()
  const IntArray& smolyak_coefficients() const;

  /// invokes SparseGridDriver::initialize_sets()
  void initialize_sets();
  /// invokes SparseGridDriver::update_reference()
  void update_reference();
  /// invokes SparseGridDriver::push_trial_set()
  void increment_set(const UShortArray& set);
  /// invokes SparseGridDriver::compute_trial_grid
  void evaluate_set();
  /// invokes SparseGridDriver::pop_trial_set()
  void decrement_set();
  /// invokes SparseGridDriver::update_sets()
  void update_sets(const UShortArray& set_star);
  /// invokes SparseGridDriver::finalize_sets()
  void finalize_sets();

protected:

  //
  //- Heading: Constructors and destructor
  //

  NonDSparseGrid(Model& model); ///< constructor
  ~NonDSparseGrid();            ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  void get_parameter_sets(Model& model);

  //void check_variables(const Pecos::ShortArray& x_types);

  void sampling_reset(int min_samples, bool all_data_flag, bool stats_flag);

private:

  //
  //- Heading: Data
  //

  /// convenience pointer to the numIntDriver representation
  Pecos::SparseGridDriver* ssgDriver;

  // the user specification for the Smolyak sparse grid level
  //unsigned short ssgLevelSpec;
  /// reference point (e.g., lower bound) for the Smolyak sparse grid level
  /// maintained within ssgDriver
  unsigned short ssgLevelRef;
};


inline const std::set<UShortArray>& NonDSparseGrid::active_multi_index() const
{ return ssgDriver->active_multi_index(); }


inline const std::set<UShortArray>& NonDSparseGrid::old_multi_index() const
{ return ssgDriver->old_multi_index(); }


inline const UShort2DArray& NonDSparseGrid::smolyak_multi_index() const
{ return ssgDriver->smolyak_multi_index(); }


inline const IntArray& NonDSparseGrid::smolyak_coefficients() const
{ return ssgDriver->smolyak_coefficients(); }


inline void NonDSparseGrid::initialize_sets()
{ ssgDriver->initialize_sets(); }


inline void NonDSparseGrid::update_reference()
{ ssgDriver->update_reference(); }


inline void NonDSparseGrid::increment_set(const UShortArray& set)
{ ssgDriver->push_trial_set(set); }


inline void NonDSparseGrid::evaluate_set()
{
  ssgDriver->compute_trial_grid();
  const Pecos::RealMatrix& var_sets = ssgDriver->variable_sets();
  //samples_to_variables_array(var_sets, allVariables);
  allSamples = RealMatrix(Teuchos::View, var_sets, var_sets.numRows(),
			  var_sets.numCols());
  evaluate_parameter_sets(iteratedModel, true, false);
  ++numIntegrations;
}


inline void NonDSparseGrid::decrement_set()
{ ssgDriver->pop_trial_set(); }


inline void NonDSparseGrid::update_sets(const UShortArray& set_star)
{ ssgDriver->update_sets(set_star); }


inline void NonDSparseGrid::finalize_sets()
{ ssgDriver->finalize_sets(); }

} // namespace Dakota

#endif
