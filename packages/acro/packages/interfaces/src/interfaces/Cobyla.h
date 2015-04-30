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
 * \file Cobyla.h
 *
 * Defines the interfaces::Cobyla class.
 */

#ifndef interfaces_Cobyla_h
#define interfaces_Cobyla_h

#include <acro_config.h>
#ifdef ACRO_USING_COBYLA
#include <colin/Solver.h>
#include <colin/CommonOptions.h>

namespace interfaces {

/** An interface to the COBYLA2 direct search method.
  *
  * This interface uses the utilib::BasicArray class to avoid copying between
  * this C++ and COBYLA's Fortran code.
  */
class Cobyla : public colin::Solver<colin::NLP0_problem>, 
               public colin::CommonOptions
{
public:

  /// Constructor
  Cobyla();

  ///
  void optimize();

  /// The function used to compute the constraints and objective
  static void calcfc(int n, int m, double* x, double* f, double* con );

protected:

  ///
  static Cobyla* instance;

  /// The function used to compute the constraints and objective
  void calcfc_impl(int n, double* x, double* f, double* con );

  ///
  std::string define_solver_type() const
        { return "Cobyla"; }

  ///
  double step_tolerance;

  ///
  double initial_step;

  ///
  utilib::BasicArray<colin::real> clower;
  ///
  utilib::BasicArray<colin::real> cupper;
  ///
  // Cobyla needs the bound constraints to be double.  Need to keep
  // the colin::real around so that finite works on Windows.
  utilib::BasicArray<double> blower_d;
  utilib::BasicArray<colin::real> blower;
  ///
  utilib::BasicArray<double> bupper_d;
  utilib::BasicArray<colin::real> bupper;
private:

  ///
  void reset_Cobyla();

};

} // namespace interfaces
#endif

#endif
