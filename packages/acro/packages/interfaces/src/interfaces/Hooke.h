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
 * \file Hooke.h
 *
 * Defines the interfaces::Hooke class.
 */

#ifndef interfaces_Hooke_h
#define intefaces_Hooke_h

#include <acro_config.h>
#ifdef ACRO_USING_THREEPO
#include <colin/Solver.h>
#include <colin/CommonOptions.h>

namespace interfaces {

/** An interface to the hooke direct search method.
  */
class Hooke : public colin::Solver<colin::UNLP0_problem>,
              public colin::CommonOptions
{
public:

  /// Constructor
  Hooke();

  ///
  void optimize();

  ///
  static double test_problem(double* x, int n);

protected:

  ///
  std::string define_solver_type() const
    { return "Hooke"; }

  ///
  double rho;

  ///
  double epsilon;

  ///
  utilib::BasicArray<colin::real> lower;

  ///
  utilib::BasicArray<colin::real> upper;

  ///
  utilib::BasicArray<double> work_point;

  ///
  double work_value;

  ///
  static Hooke* curr_solver;

private:

  ///
  void reset_Hooke();

};

} // namespace interfaces
#endif
#endif
