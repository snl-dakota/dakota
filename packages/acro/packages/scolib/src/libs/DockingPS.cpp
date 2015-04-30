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
// Coliny_DockingPS.cpp
//

#include <acro_config.h>

#include <scolib/DockingPS.h>

#include <colin/SolverMngr.h>

#define M_TWO_PI   (2.0*M_PI)
#define TESTING

using namespace std;
using namespace utilib;

namespace scolib {

namespace {

void renormalize(double& nx, double& ny, double& nz)
{
double norm = sqrt(nx*nx + ny*ny + nz*nz);
nx /= norm;
ny /= norm;
nz /= norm;
}

double compute_z(double x, double y)
{
double tmp = 1.0 - x*x - y*y;
if (tmp < -1e-7)
   EXCEPTION_MNGR(runtime_error, "Bad x/y values!");
return sqrt(max(0.0,tmp));
}


}

DockingPS::DockingPS()
{
num_rotations=4;
last_x=last_y=last_theta=0.0;
properties.privilegedGet("expansion_factor").set_readonly();
initial_angle=80.0;

reset_signal.connect(boost::bind(&DockingPS::reset_DockingPS, this));
}



void DockingPS::reset_DockingPS()
{
if ( problem.empty() ) return;

if (Delta_init > 1.0)
   Delta_init = 1.0;

basis_str = "coordinate";

ntrials = ncore_trials = 2*problem->num_real_vars.as<size_t>() 
   - 4 + num_rotations;
ex_factor=1.0;
}



void DockingPS::generate_trial(int id, const DoubleVector& x_, 
				   DoubleVector& trial, double scale, 
				   bool& feasible, const DoubleVector& _bias)
{
//
// Initial setup
//
feasible=true;
if (debug > 4) {
   ucout << "Current Point:   " << x_ << endl;
   }
//
// The index of the dimension that is being changed
//     0    - 2		positive steps in the positional parameters
//     3    - n-4	positive steps in the torsion angles
//     n-3  - n-1	negative steps in the positional parameters
//     n    - 2n-7	negative steps in the torsion angles
//     2n-6 - 2n-5      positive/negative steps in rotations
//     2n-4 - 2n-4+k    k rotations of quarternion angles
//
size_type curr = ndx[id];
size_type n    = x_.size();
trial << x_;

if (curr <= 2) {
   trial[curr] += scale*Sigma[curr];
   if (trial[curr] > upper_bc[curr])
      feasible=false;
   }

else if (curr <= (n-4)) {
   trial[curr+3] += scale*Sigma[curr+3];
   if (trial[curr+3] > (2*upper_bc[curr+3]-lower_bc[curr+3]))
      feasible=false;
   }

else if (curr <= (n-1)) {
   trial[curr-n+3] -= scale*Sigma[curr-n+3];
   if (trial[curr-n+3] < lower_bc[curr-n+3])
      feasible=false;
   }

else if (curr <= (2*n-7)) {
   trial[curr-n+6] -= scale*Sigma[curr-n+6];
   if (trial[curr-n+6] < (2*lower_bc[curr-n+6]-upper_bc[curr-n+6]))
      feasible=false;
   if (trial[curr-n+6] < 0.0)
      trial[curr-n+6] += M_TWO_PI;
   }

else if (curr == (2*n-6)) {
   trial[5] -= scale*Sigma[5];
   if (trial[5] < (2*lower_bc[5]-upper_bc[5]))
      feasible=false;
   if (trial[5] < 0.0)
      trial[5] += M_TWO_PI;
   }

else if (curr == (2*n-5)) {
   trial[5] += scale*Sigma[5];
   if (trial[5] > (2*upper_bc[5]-lower_bc[5]))
      feasible=false;
   if (trial[5] > M_TWO_PI)
      trial[5] -= M_TWO_PI;
   }

else {
   if (scale != Delta_min)
      EXCEPTION_MNGR(runtime_error, "BUG HERE");

      rotation_trial(min(scale,Delta_min), curr-(2*n-4), x_[3], x_[4], x_[5], 
					trial[3], trial[4], trial[5]);
#if 0
   else {
      int tmp = curr-(2*n-4);
      if (tmp == 0) {
         trial[3] += scale*Sigma[3];
         if (trial[3] > (2*upper_bc[3]-lower_bc[3]))
            feasible=false;
         }
      else if (tmp == 1) {
         trial[4] += scale*Sigma[4];
         if (trial[4] > (2*upper_bc[4]-lower_bc[4]))
            feasible=false;
         }
      else if (tmp == 2) {
         trial[3] -= scale*Sigma[3];
         if (trial[3] > lower_bc[3])
            feasible=false;
         }
      else {
         trial[4] -= scale*Sigma[4];
         if (trial[4] > lower_bc[4])
            feasible=false;
         }
      if (feasible && ((trial[3]*trial[3]+trial[4]*trial[4]) > 1.0)) {
	 double z=0.0;
	 renormalize(trial[3],trial[4],z);
	 }
      }
#endif
   }

if (debug > 4) {
   ucout << "Trial Point:   " << trial << endl;
   ucout << "Feasible: " << feasible << endl;
   }
if (debug > 4) {
   double val=0.0;
   for (unsigned int i=0; i<x_.size(); i++)
     val += (trial[i]-x_[i])*(trial[i]-x_[i]);
   ucout << "Trial Point Length: " << sqrt(val) << endl;
   }
  
//
// Collect running stats
//
if (output_level == 3) {
   ntrial_points++;
   if (feasible == true) nfeasible_points++;
   }
}



void DockingPS::update_pattern(DoubleVector& prev_iter,
                      		DoubleVector& curr_iter,
                      		bool flag)
{
if (update_flag == 3)
   EXCEPTION_MNGR(runtime_error, "DockingPS::update_pattern - bad step length update!");

bool quaternion_diff = ((curr_iter[3] != prev_iter[3]) ||
 			(curr_iter[4] != prev_iter[4]));
DEBUGPR(1000, ucout << "DockingPS::update_pattern - update=" << update_flag <<
        " succ=" << flag << 
        " quaternion-diff=" << quaternion_diff << endl);
//
// Don't update the pattern if the quaternion info hasn't changed.
// Update the angle if it is negative.
//
if (curr_iter[5] > M_TWO_PI) 
   EXCEPTION_MNGR(runtime_error, "BUG HERE 3");
if ((curr_iter[5] < 0.0) && !flag)
   EXCEPTION_MNGR(runtime_error, "BUG HERE 4");
//
// After an unsuccessful iteration, we don't change the pattern orientation,
// but we _do_ reduce it in size
//
if (!flag) {
   if (update_flag == 2) {
      last_x = (prev_iter[3] + last_x)/2.0;
      last_y = (prev_iter[4] + last_y)/2.0;
      last_z = (last_z + compute_z(prev_iter[3],prev_iter[4]))/2.0;
      renormalize(last_x,last_y,last_z);
      last_theta = prev_iter[5];
      }
   else
      EXCEPTION_MNGR(runtime_error, "BUG HERE 2");
   return;
   }
//
// This is a successful iteration, but don't change the quaternion unless
// it was the reason for the new iterate.
//
if (!quaternion_diff) {
   return;
   }
//
// After a successful iteration, last_x,y,z is the previous iteration.
//
last_x = prev_iter[3];
last_y = prev_iter[4];
last_z = compute_z(prev_iter[3],prev_iter[4]);
if (curr_iter[5] < 0.0) {
   curr_iter[5] += M_TWO_PI;
   last_x *= -1;
   last_y *= -1;
   last_z *= -1;
   }
renormalize(last_x,last_y,last_z);
#if 0
WEH - do we need to rotate the pattern?????

//
// After a successful iteration, we need to rotate the pattern.
// We may also need to rescale it larger, but we won't be doing that for now
//
//
// Compute a vector 'n' which is perpendicular to the previous iteration
// and current iteration
//
double nx,ny,nz;
double cz = sqrt(1-curr_iter[3]*curr_iter[3]-curr_iter[4]*curr_iter[4]);
if (flag && quaternion_diff) {
  double pz = sqrt(1-prev_iter[3]*prev_iter[3]-prev_iter[4]*prev_iter[4]);
  
  nx = curr_iter[4]*pz           - cz*prev_iter[4];
  ny = cz*prev_iter[3]           - curr_iter[3]*pz;
  nz = curr_iter[3]*prev_iter[4] - curr_iter[4]*prev_iter[3];
  }
else {
  nx = curr_iter[4]*last_z - cz*last_y;
  ny = cz*last_x           - curr_iter[3]*last_z;
  nz = curr_iter[3]*last_y - curr_iter[4]*last_x;
  }
renormalize(nx,ny,nz);
//
//
//
#if defined(TESTING)
double tmp = nx*prev_iter[3] + 
	     ny*prev_iter[4] + 
	     nz*sqrt(1-prev_iter[3]*prev_iter[3]-prev_iter[4]*prev_iter[4]);
if (fabs(tmp) > 1e-5)
   EXCEPTION_MNGR(runtime_error, "The normal vector doesn't seem normal enough.");
if (fabs(min(nx,min(ny,nz))) < 1e-5)
   EXCEPTION_MNGR(runtime_error, "The normal vector doesn't seem long enough.");
#endif
//
// Rotate the initial point M_PI/2 times Delta about n.
// This gives a reference value
// for a 'last point'.  Rotations of this 'last point' will define subsequent
// steps.
//
rotate(angle, curr_iter[3],  curr_iter[4], cz,
                       nx,     ny,     nz,
                       last_x, last_y, last_z);
#endif
//
// Misc testing
//
#if 0
#if defined(TESTING)
double angle = min(1.0, Sigma[3]*Delta)*M_PI/2.0;
if (curr_iter[5] < 0.0) 
   EXCEPTION_MNGR(runtime_error, "THIS CANNOT HAPPEN, RIGHT?");
double est_dist = 2*sin(angle/2.0);
double dist = sqrt((curr_iter[3]-last_x)*(curr_iter[3]-last_x) +
                   (curr_iter[4]-last_y)*(curr_iter[4]-last_y) +
                   (cz-last_z)*(cz-last_z));
DEBUGPR(1000, ucout << "UpdatePattern Test: angle=" << angle << 
        " est=" << est_dist << " dist=" << dist << endl << Flush);
DEBUGPR(1000, ucout << "x=" << curr_iter[3] << " y=" << curr_iter[4] 
        << " z=" << cz << "  rx=" << last_x << " ry=" << last_y 
        << " rz=" << last_z << endl << Flush);
if (fabs(est_dist - dist) > Delta*1e-5)
   EXCEPTION_MNGR(runtime_error,"The last_* data appears to be incorrect.");
#endif
#endif
}



void DockingPS::rotation_trial(double scale, int id, 
			double x, double y, double theta,
			double& new_x, double& new_y, double& new_theta)
{
double z = compute_z(x,y);
double new_z;
if (id==0) {
   new_x = last_x;
   new_y = last_y;
   new_z = last_z;
   //new_z = sqrt(1-new_x*new_x-new_y*new_y);
   }
else {
   rotate( id*2*M_PI/num_rotations,
	last_x, last_y, last_z,
	x, y, z,
	new_x, new_y, new_z);
   }


//
// Misc testing
//
#if defined(TESTING)
double angle = id*2*M_PI/num_rotations;
double est_dist = 2*sin(angle/2.0);
double dist = sqrt((x-new_x)*(x-new_x) +
                   (y-new_y)*(y-new_y) +
                   (z-new_z)*(z-new_z));
double dist2 = sqrt((last_x-new_x)*(last_x-new_x) +
                   (last_y-new_y)*(last_y-new_y) +
                   (last_z-new_z)*(last_z-new_z));
DEBUGPR(1000, ucout << "RotationTest: angle=" << angle << " est=" << 
        est_dist << " dist=" << dist << " DIST=" << dist2 << endl << Flush);
DEBUGPR(1000, ucout << "x=" << x << " y=" << y << " z=" << z << 
        "  rx=" << new_x << " ry=" << new_y << " rz=" << new_z << 
        endl << Flush);
DEBUGPR(1000, ucout << "lx=" << last_x << " ly=" << last_y << " lz=" 
        << last_z << "  rx=" << new_x << " ry=" << new_y << " rz=" 
        << new_z << endl << Flush);
#if 0
if (fabs(est_dist - dist) > scale*1e-5)
   EXCEPTION_MNGR(runtime_error,"The new_* data appears to be incorrect.");
#endif
#endif

if (new_z < 0.0) {
   new_x *= -1.0;
   new_y *= -1.0;
   new_z *= -1.0;
   new_theta *= -1.0;
   }
}



//
// Rotate the 'p' vector about 'n', returning the value in 'r'
//
void DockingPS::rotate(double angle,
			double  px, double  py, double  pz,
			double  nx, double  ny, double  nz, 
			double& rx, double& ry, double& rz)
{
if (angle == 0.0) {
   rx = px;
   ry = py;
   rz = pz;
   return;
   }

double c = cos(angle);
double s = sin(angle);
double t = 1-c;

#if 0
double p11 = t*nx*nx + c;
double p12 = t*nx*ny + s*nz;
double p13 = t*nx*nz + s*ny;
double p21 = t*nx*ny - s*nz;
double p22 = t*ny*ny + c;
double p23 = t*ny*nz + s*nz;
double p31 = t*nx*nz + s*ny;
double p32 = t*ny*nz - s*nx;
double p33 = t*nz*nz + c;
#endif

double p11 = t*nx*nx + c;
double p21 = t*ny*nx + s*nz;
double p31 = t*nz*nx - s*ny;
double p12 = t*nx*ny - s*nz;
double p22 = t*ny*ny + c;
double p32 = t*nz*ny + s*nx;
double p13 = t*nx*nz + s*ny;
double p23 = t*ny*nz - s*nx;
double p33 = t*nz*nz + c;

#if 0
rx = px*p11 + py*p12 + pz*p13;
ry = px*p21 + py*p22 + pz*p23;
rz = px*p31 + py*p32 + pz*p33;
#endif
rx = px*p11 + py*p21 + pz*p31;
ry = px*p12 + py*p22 + pz*p32;
rz = px*p13 + py*p23 + pz*p33;
}



void DockingPS::set_initial_point(const utilib::AnyRef& point)
{
initial_point_flag=true;
utilib::TypeManager()->lexical_cast(point, initial_point);
//
// Compute an (x,y,z) vector that we assume is co-planar with 'initial_point'
//
double x=0.0;
double y=0.0;
double z=0.0;
double pz = compute_z(initial_point[3],initial_point[4]);
if (initial_point[3] < initial_point[4]) {
   if (initial_point[3] < pz)
      x = 1.0;
   else
      z = 1.0;
   }
else {
   if (initial_point[4] > pz)
      z = 1.0;
   else
      y = 1.0;
   }
//
// Compute a vector 'n' which is perpendicular to the initial point and
// a vector (x,y,z) defined along the largest dimension of the initial point
//
double nx = y*pz    - z*initial_point[4];
double ny = z*initial_point[3] - x*pz;
double nz = x*initial_point[4] - y*initial_point[3];
renormalize(nx,ny,nz);
//
// Rotate the initial point M_PI/2 times Delta.  This gives a reference value
// for a 'last point'.  Rotations of this 'last point' will define subsequent
// steps.
//
rotate((min(75.0,initial_angle)/90)*M_PI/2.0, initial_point[3],  initial_point[4],  pz,
		       nx,     ny,     nz,
		       last_x, last_y, last_z);
//
// Misc testing
//
#if defined(TESTING)
#if 0
double est_dist = 2*sin(angle/2.0);
double dist = sqrt((initial_point[3]-last_x)*(initial_point[3]-last_x) +
                   (initial_point[4]-last_y)*(initial_point[4]-last_y) +
                   (pz-last_z)*(pz-last_z));
if (fabs(est_dist - dist) > 1e-5)
   EXCEPTION_MNGR(runtime_error,"The last_* data appears to be incorrect.");
#endif
#endif
}


// Defines scolib::StaticInitializers::DockingPS_bool
REGISTER_COLIN_SOLVER_WITH_ALIAS(DockingPS,"sco:DockingPS","sco:dockingps", "The SCO DockingPS optimizer")

} // namespace scolib
