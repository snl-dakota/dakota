/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#ifdef HAVE_CONFIG_H
#include "dakota_config.h"
#endif // HAVE_CONFIG_H
#ifdef HAVE_STD
#include <cmath>
#else
#include <math.h>
#endif // HAVE_STD
using namespace std;


int main(int argc, char** argv)
{ 
  // The illumination example in Boyd as a general minimization problem
  // Objective function = ...

  // This application program reads and writes parameter and response data 
  // directly so that the NO_FILTER option of dakota may be used.

  ifstream fin(argv[1]);
  if (!fin) {
    cerr << "\nError: failure opening " << argv[1] << endl;
    exit(-1);
  }
  size_t i, j, num_vars, num_fns;
  string vars_text, fns_text;

  // Get the parameter vector and ignore the labels
  fin >> num_vars >> vars_text;
  vector<double> x(num_vars);
  for (i=0; i<num_vars; i++) {
    fin >> x[i];
    fin.ignore(256, '\n');
  }

  // Get the ASV vector and ignore the labels
  fin >> num_fns >> fns_text;
  vector<int> ASV(num_fns);
  for (i=0; i<num_fns; i++) {
    fin >> ASV[i];
    fin.ignore(256, '\n');
  }

  if (num_vars != 7) {
    cerr << "Wrong number of variables for the illumination problem" << endl;
    exit(-1);
  }
  if (num_fns != 1) {
    cerr << "Wrong number of functions for the illumination problem" << endl;
    exit(-1);
  }

  // compute function and gradient values
  double A[11][7] ={ 
  { 0.347392, 0.205329, 0.191987, 0.077192, 0.004561, 0.024003, 0.000000},
  { 0.486058, 0.289069, 0.379202, 0.117711, 0.006667, 0.032256, 0.000000},
  { 0.752511, 0.611283, 2.417907, 0.701700, 0.473047, 0.285597, 0.319187},
  { 0.303582, 0.364620, 1.898185, 0.693173, 0.607718, 0.328582, 0.437394},
  { 0.540946, 0.411549, 1.696545, 0.391735, 0.177832, 0.110119, 0.083817},
  { 0.651840, 0.540687, 3.208793, 0.639020, 0.293811, 0.156842, 0.128499},
  { 0.098008, 0.245771, 0.742564, 0.807976, 0.929739, 0.435144, 0.669797},
  { 0.000000, 0.026963, 0.000000, 0.246606, 0.414657, 0.231777, 0.372202},
  { 0.285597, 0.320457, 0.851227, 0.584677, 0.616436, 0.341447, 0.477329},
  { 0.324622, 0.306394, 0.991904, 0.477744, 0.376266, 0.158288, 0.198745},
  { 0.000000, 0.050361, 0.000000, 0.212042, 0.434397, 0.286455, 0.462731} };

  double harray[7][7] ={ 
  { 1.929437, 1.572662, 6.294004, 1.852205, 1.222324, 0.692036, 0.768564},
  { 1.572662, 1.354287, 5.511537, 1.787932, 1.320048, 0.724301, 0.870382},
  { 6.294004, 5.511537, 25.064512, 7.358494, 5.133563, 2.791970, 3.257364},
  { 1.852205, 1.787932, 7.358494, 2.883178, 2.497491, 1.321922, 1.747230},
  { 1.222324, 1.320048, 5.133563, 2.497491, 2.457733, 1.295927, 1.816568},
  { 0.692036, 0.724301, 2.791970, 1.321922, 1.295927, 0.694642, 0.968982},
  { 0.768564, 0.870382, 3.257364, 1.747230, 1.816568, 0.968982, 1.385357} };

  ofstream fout(argv[2]);
  if (!fout) {
    cerr << "\nError: failure creating " << argv[2] << endl;
    exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(ios::scientific);
  fout.setf(ios::right);

  // **** f:
  if (ASV[0] & 1) {
    double fx = 0.0;
    for (i=0; i<11; i++) {
      double dtmp = 0.0;
      for (j=0; j<num_vars; j++)
        dtmp += A[i][j] * x[j];
      dtmp = (1.0 - dtmp) * (1.0 - dtmp);
      fx = fx + dtmp;
    }
    fx = sqrt(fx);
    fout << "                     " << fx << " f\n";
  }

  // **** df/dx:
  if (ASV[0] & 2) {
    fout << "[ ";
    for (i=0; i<num_vars; i++) {
      double dtmp = 0.0;
      for (j=0; j<num_vars; j++) 
        dtmp += harray[i][j] * x[j];
      for (j=0; j<11; j++) 
        dtmp -= A[j][i];
      dtmp *= 2.0;
      fout << dtmp << " ";
    }
    fout << "]\n";
  }

  // **** hessian ddf/dxidxj
  if (ASV[0] & 4) {
    fout << "[[ ";
    for (i=0; i<num_vars; i++) {
      for (j=0; j<num_vars; j++) 
        fout << harray[i][j] << " ";
      fout << "\n";
    }
    fout <<" ]] \n";
  }

  fout.flush();
  fout.close();
  return 0;
}
