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
 * \file TCC.h
 *
 * Defines the interfaces::TCC class.
 */

#ifndef interfaces_TCC_h
#define interfaces_TCC_h

#include <acro_config.h>
#ifdef USING_TCC

#include <utilib/DoubleMatrix.h>
#include <colin/Solver.h>
#include <colin/real.h>

namespace interfaces {

using colin::real;

/** An interface to the cluster-based global optimization routine
implemented by Tibor Csendes.
*/
class TCC : public STDOPTSOLVER_SUBCLASS(utilib::BasicArray<double>,utilib::BasicArray)
{
public:

  /**@name General */
  //@{
  ///
  TCC();

  ///
  void reset();

  ///
  void minimize();
  //@}


protected:

  ///
  void initialize_best_point()
	{
	if ((best_point.size() > 0) &&
	    (best_point.size() != problem.num_real_vars()))
	   EXCEPTION_MNGR(runtime_error, "initialize_best_point - user-provided best point has length " << best_point.size() << " but the problem size is " << problem.num_real_vars() << endl);
        best_point.resize(problem.num_real_vars());
	}

  /**@name Configuration Controls */
  //@{
  /// The maximum number of local mins
  int max_nmins;

  /// The number of random samples selected;  20 <= x <=10000
  int num_samples;

  /// The number of points selected to form clusters; 1  <= x <= 20
  int num_selected;

  /// The number of significant digits required by the LS operator
  int nsig;
  //@}

  /**@ Iteration Controls */
  //@{
  /// A work array
  utilib::BasicArray<double> work;

  /// Lower bound constraints
  utilib::BasicArray<real> lower;

  /// Upper bound constraints
  utilib::BasicArray<real> upper;

  ///
  utilib::DoubleMatrix x0;

  ///
  utilib::BasicArray<double> f0;
  //@}

};

} // namespace interfaces

#endif

#endif
