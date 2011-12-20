/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDLocalRefinement
//- Description: 
//- Owner:       Mike Eldred
//- Revised by:  
//- Version:

#ifndef NOND_LOCAL_REFINEMENT_H
#define NOND_LOCAL_REFINEMENT_H

#include "data_types.h"
#include "NonDIntegration.H"
#include "LocalRefinableDriver.hpp"

namespace Dakota {


/// Derived nondeterministic class that generates locally refined grids.

/** This class is used by NonDStochCollocation for hierarchical refinement. */

class NonDLocalRefinement: public NonDIntegration
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// alternate constructor for instantiations "on the fly" based on a
  /// local refinement level specification
  NonDLocalRefinement(Model& model, const UShortArray& lr_level,
		      const RealVector& dim_pref);

  //
  //- Heading: Member functions
  //

protected:

  //
  //- Heading: Constructors and destructor
  //

  //NonDLocalRefinement(Model& model); ///< constructor
  ~NonDLocalRefinement();            ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  void initialize_grid(const std::vector<Pecos::BasisPolynomial>& poly_basis);

  void get_parameter_sets(Model& model);

  void reset();
  void sampling_reset(int min_samples,bool all_data_flag, bool stats_flag);

  void increment_grid();
  //void increment_grid_preference(const RealVector& dim_pref);
  void increment_refinement_sequence();

  int num_samples() const;

private:

  //
  //- Heading: Data
  //

  /// convenience pointer to the numIntDriver representation
  Pecos::LocalRefinableDriver* lrDriver;

  /// a sequence of scalar local refinement levels, one per grid specification
  UShortArray lrLevelSpec;
};


inline void NonDLocalRefinement::reset()
{
  // TO DO (code below is a placeholder)
  size_t i, start = (sequenceIndex) ? 0 : lrLevelSpec[sequenceIndex-1];
  for (i=start; i<lrLevelSpec[sequenceIndex]; ++i)
    lrDriver->refine_globally();
}


inline void NonDLocalRefinement::increment_grid()
{ lrDriver->refine_globally(); } // TO DO (this code is a placeholder)


inline void NonDLocalRefinement::increment_refinement_sequence()
{
  if (sequenceIndex+1 < lrLevelSpec.size())
    ++sequenceIndex;
  reset();
}


inline int NonDLocalRefinement::num_samples() const
{ return lrDriver->grid_size(); }

} // namespace Dakota

#endif
