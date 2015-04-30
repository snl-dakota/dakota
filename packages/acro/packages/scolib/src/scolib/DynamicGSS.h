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
 * \file DynamicGSS.h
 *
 * Defines the scolib::DynamicGSS class.
 */

#ifndef scolib_DynamicGPS_h
#define scolib_DynamicGPS_h

#include <acro_config.h>
#include <utilib/default_rng.h>
#include <utilib/Normal.h>
#include <colin/solver/ColinSolver.h>

namespace scolib {

using colin::real;
using utilib::BasicArray;
using utilib::Normal;
using utilib::NumArray;


/** An implemention of a dynamic generating set search method that
decomposes the search into subproblems.
*/
class DynamicGSS 
   : public colin::ColinSolver<BasicArray<double>, colin::NLP0_problem>
{
public:

  ///
  DynamicGSS();

  ///
  void optimize();

  ///
  utilib::Normal nrnd;

protected:

  ///
  void initialize_best_point()
	{
	if ((initial_point.size() > 0) &&
	    ( problem->num_real_vars != initial_point.size() ))
	   EXCEPTION_MNGR(std::runtime_error, "initialize_best_point - user-provided best point has length " << initial_point.size() << " but the problem size is " << problem->num_real_vars << std::endl);
        initial_point.resize(problem->num_real_vars);
	}

private:
  ///
  void reset_DynamicGSS();

};

} // namespace scolib

#endif
