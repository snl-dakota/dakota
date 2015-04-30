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
 * \file DockingPS.h
 *
 * Defines the scolib::DockingPS class.
 */
 
#ifndef scolib_DockingPS_h
#define scolib_DockingPS_h
 
#include <acro_config.h>
#include <scolib/PatternSearch.h>

namespace scolib {


/** A pattern search method that is explicitly tailored to optimize
    a search space for docking problems:

    [x,y,z] positional parameters
		These are typically bound-constrained values

    [X,Y,t] quarternion parameters
		These specify a quarternion of the form (X,Y,Z),t
		where X^2+Y^2+Z^2=1
		The t value is a rotational parameter that can be constrained
		to lie within [0,pi]

    [t1, t2, ..., tk] torsion parameters
		These are angular parameters that lie within [0,2pi]

*/
class DockingPS : public PatternSearch 
{ 
public:

  ///
  DockingPS();

  ///
  virtual ~DockingPS() {}

  ///
  void set_initial_point(const utilib::AnyRef& point);

protected:
   std::string define_solver_type() const
   { return "DockingPS"; }

  ///
  void generate_trial(int id, const DoubleVector& x, DoubleVector& trial, 
			double scale, bool& feasible, const DoubleVector& bias);

  ///
  void update_pattern(  DoubleVector& prev_iter, DoubleVector& curr_iter,
			bool flag);

  ///
  double last_x;

  ///
  double last_y;

  ///
  double last_z;

  ///
  double last_theta;

  /**
   * rotate from the point (x,y,z) to the point (new_x,new_y,new_z).  This
   * is the id-th of 0 ... DockingPS::num_rotations-1 that can be performed.
   */
  void rotation_trial(double scale, int id, double x, double y, double theta,
			      double& new_x, double& new_y, double& new_theta);

  /// Rotate the 'p' vector about 'n', returning the value in 'r'
  void rotate(double angle,
                double  px, double  py, double  pz,
                double  nx, double  ny, double  nz,
                double& rx, double& ry, double& rz);

  ///
  int num_rotations;

  /// The initial angle (in degrees
  double initial_angle;

private:
  ///
  void reset_DockingPS();

};

} // namespace scolib

#endif
