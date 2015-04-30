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

/**
 * \file PEAreal.h
 *
 * Defines the scolib::PEreal class.
 */

#ifndef scolib_PEAreal_h
#define scolib_PEAreal_h

#include <acro_config.h>
#include <scolib/PEAbase.h>
#include <scolib/EAops_real.h>

namespace scolib {


/** An evolutionary algorithm with genotype=RealVector and phenotype=RealVector */

class PEAreal : virtual public PEAbase<EApoint_real, NumArray<double> >, 
		virtual public EAops_real {

public:

  ///
  PEAreal();

  ///
  void reset();

  ///
  void write(std::ostream& os) const;

protected:

  ///
  void initialize_best_point()
	{
	if ((best().point.size() > 0) &&
	    (best().point.size() != problem.num_real_vars()))
	   EXCEPTION_MNGR(std::runtime_error, "initialize_best_point - user-provided best point has length " << best().point.size() << " but the problem size is " << problem.num_real_vars() << std::endl);
	best().point.resize(problem.num_real_vars());
	}
};

} // namespace scolib

#endif
