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
//#include "unistd.h" // for sleep


int main(int argc, char** argv)
{
  ifstream fin(argv[1]);
  if (!fin) {
    cerr << "\nError: failure opening " << argv[1] << endl;
    exit(-1);
  }
  size_t i, j, k, num_vars, num_fns, num_deriv_vars;
  string vars_text, fns_text, dvv_text;

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

  // Get the DVV vector and ignore the labels
  fin >> num_deriv_vars >> dvv_text;
  vector<int> DVV(num_deriv_vars);
  for (i=0; i<num_deriv_vars; i++) {
    fin >> DVV[i];
    fin.ignore(256, '\n');
  }


  //  srand ( (unsigned int) (time(NULL)/x[0]) );
  //sleep( (int)(3.0*((double)rand()/RAND_MAX)));

  //sleep(5);

  // Compute the results and output them directly to argv[2] (the NO_FILTER
  // option is used).  Response tags are now optional; output them for ease
  // of results readability.
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
    double value = 0.;
    for (i=0; i<num_vars; i++)
      value += pow(x[i]-1.0, 4.0);
    fout << "                     " << value << " f\n";
  }

  // **** c1:
  if (num_fns > 1 && (ASV[1] & 1))
    fout << "                     " << x[0]*x[0] - 0.5*x[1] << " c1\n";

  // **** c2:
  if (num_fns > 2 && (ASV[2] & 1))
    fout << "                     " << x[1]*x[1] - 0.5*x[0] << " c2\n";

  // **** c3 through cn (allow an arbitrary # of response fns.):
  for (i=3; i<num_fns; i++)
    if (ASV[i] & 1)
      fout << "                     0.0 c" << i << '\n';

  // **** df/dx:
  if (ASV[0] & 2) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++)
      fout << 4.*pow(x[DVV[i]-1] - 1., 3) << ' ';
    fout << "]\n";
  }

  // **** dc1/dx:
  if (num_fns > 1 && (ASV[1] & 2)) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++) {
      int var_index = DVV[i] - 1;
      if (var_index == 0)
        fout << 2.*x[0] << ' ';
      else if (var_index == 1)
        fout << -0.5 << ' ';
      else
	fout << "0. ";
    }
    fout << "]\n";
  }

  // **** dc2/dx:
  if (num_fns > 2 && (ASV[2] & 2)) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++) {
      int var_index = DVV[i] - 1;
      if (var_index == 0)
        fout << -0.5 << ' ';
      else if (var_index == 1)
        fout << 2.*x[1] << ' ';
      else
	fout << "0. ";
    }
    fout << "]\n";
  }

  // **** dc3/dx through dcn/dx (allow an arbitrary # of response fns.):
  for (i=3; i<num_fns; i++) {
    if (ASV[i] & 2) {
      fout << "[ ";
      for (j=0; j<num_deriv_vars; j++)
        fout << "0. ";
      fout << "]\n";
    }
  }

  // **** d^2f/dx^2: (full Newton unconstrained opt.)
  if (ASV[0] & 4) {
    fout << "[[ ";
    for (i=0; i<num_deriv_vars; i++)
      for (j=0; j<num_deriv_vars; j++)
        if (i==j)
          fout << 12.*pow(x[DVV[i]-1] - 1., 2) << ' ';
        else
          fout << "0. ";
    fout << "]]\n";
  }

  // **** d^2c1/dx^2: (ParamStudy testing of multiple Hessian matrices)
  if (num_fns > 1 && (ASV[1] & 4)) {
    fout << "[[ ";
    for (i=0; i<num_deriv_vars; i++) {
      int var_index_i = DVV[i] - 1;
      for (j=0; j<num_deriv_vars; j++) {
	int var_index_j = DVV[j] - 1;
        if (var_index_i==0 && var_index_j==0)
          fout << 2. << ' ';
        else
          fout << "0. ";
      }
    }
    fout << "]]\n";
  }

  // **** d^2c2/dx^2: (ParamStudy testing of multiple Hessian matrices)
  if (num_fns > 2 && (ASV[2] & 4)) {
    fout << "[[ ";
    for (i=0; i<num_deriv_vars; i++) {
      int var_index_i = DVV[i] - 1;
      for (j=0; j<num_deriv_vars; j++) {
	int var_index_j = DVV[j] - 1;
        if (var_index_i==1 && var_index_j==1)
          fout << 2. << ' ';
        else
          fout << "0. ";
      }
    }
    fout << "]]\n";
  }

  // **** d^2c3/dx^2 through d^2cn/dx^2 (allow an arbitrary # of response fns.):
  for (i=3; i<num_fns; i++) {
    if (ASV[i] & 4) {
      fout << "[[ ";
      for (j=0; j<num_deriv_vars; j++)
        for (k=0; k<num_deriv_vars; k++)
          fout << "0. ";
      fout << "]]\n";
    }
  }

  fout.flush();
  fout.close();
  return 0;
}
