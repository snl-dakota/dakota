/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

//
// EAops.h
//

#if 0
/**
 * \file FOO 
 */

#ifndef scolib_EAops_h
#define scolib_EAops_h

#include <acro_config.h>
#include <utilib/Uniform.h>
#include <utilib/Normal.h>
#include <utilib/Cauchy.h>
#include <scolib/EAbase.h>

namespace scolib {


/** Class that defines operations used with GApoint_real genotypes.
*/
template <class PointT, class DomainT>
class EAops : virtual public EAbase<PointT,DomainT>
{
public:

  ///
  EAops() {}

  ///
  void write(std::ostream& ) const {}

  ///
  void reset() {}

  ///
  virtual void randomize() {}

protected:

  ///
  virtual bool apply_mutation(EAindividual<PointT,DomainT>& point, int parent_ndx) = 0;

  ///
  virtual void apply_xover(EAindividual<PointT,DomainT>& parent1_, EAindividual<PointT,DomainT>& parent2_,
                        EAindividual<PointT,DomainT>& child_) = 0;

  ///
  enum mut_types {
        MUTATION_UNIFORM,
        MUTATION_NORMAL,
        MUTATION_CAUCHY,
        MUTATION_INTERVAL,
        MUTATION_DISCRETE,
        MUTATION_UNARY_DISCRETE,
        MUTATION_GENERAL_DIR
        };

  ///
  enum xover_types {
        XOVER_BLEND,
        XOVER_TWOPOINT,
        XOVER_UNIFORM
        };

};

} // namespace scolib

#endif
#endif
