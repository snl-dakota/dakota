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
 * \file DOT.h
 *
 * Defines the interfaces::DOT_CG class.
 */

#include <acro_config.h>
#if 0

#ifndef COLINY_WITHOUT_DOT

#ifndef interfaces_DOT_h
#define interfaces_DOT_h

#include <colin/ColinSolver.h>


namespace interfaces {

/** An interface to the DOT Conjugate Gradient algorithm.
*/
class DOT_CG : public colin::Solver<vector<double> >
{
public:

  ///
  DOT_CG();

  ///
  void reset();

  ///
  void minimize();

protected:

  ///
  void initialize_best_point()
	{
	if ((best_point.size() > 0) &&
	    (best_point.size() != problem.num_real_vars()))
	   EXCEPTION_MNGR(runtime_error, "initialize_best_point - user-provided best point has length " << best_point.size() << " but the problem size is " << problem.num_real_vars() << endl);
        best_point.resize(problem.num_real_vars());
	}

  ///
  double ftol;

  ///
  double fdss;

  ///
  int numConstraints;
 
  ///
  int dotInfo;

  ///
  int dotFDSinfo;

  ///
  int dotMethod;

  ///
  int printControl;

  ///
  int optimizationType;

  ///
  vector<double> realWorkSpace;

  ///
  vector<int>  intWorkSpace;

  ///
  vector<double> constraintArray;
 
  ///
  vector<double> realCntlParmArray;

  ///
  vector<int> intCntlParmArray;

  ///
  vector<double> X;

  ///
  vector<double> bestX;

  ///
  vector<double> lowerBoundsArray;

  ///
  vector<double> upperBoundsArray;

};

} // namespace interfaces

#endif

#endif

#endif
