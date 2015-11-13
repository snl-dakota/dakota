/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <cfloat>

// prototype for derivative calculation
double dfdxi(const int index, const int a[], const int num_vars, 
	     const std::vector<double>& x);

int main(int argc, char** argv)
{

  // This application program reads and writes parameter and response data 
  // directly so no input/output filters are needed.
  std::ifstream fin(argv[1]);
  if (!fin) {
    std::cerr << "\nError: failure opening " << argv[1] << std::endl;
    exit(-1);
  }
  size_t i, num_vars, num_fns, num_deriv_vars;
  std::string vars_text, fns_text, dvv_text;

  // Get the parameter std::vector and ignore the labels
  fin >> num_vars >> vars_text;
  std::vector<double> x(num_vars);
  for (i=0; i<num_vars; i++) {
    fin >> x[i];
    fin.ignore(256, '\n');
  }

  // Get the ASV std::vector and ignore the labels
  fin >> num_fns >> fns_text;
  std::vector<int> ASV(num_fns);
  for (i=0; i<num_fns; i++) {
    fin >> ASV[i];
    fin.ignore(256, '\n');
  }

  // Get the DVV std::vector and ignore the labels
  fin >> num_deriv_vars >> dvv_text;
  std::vector<int> DVV(num_deriv_vars);
  for (i=0; i<num_deriv_vars; i++) {
    fin >> DVV[i];
    fin.ignore(256, '\n');
  }

  if (num_vars < 1 || num_vars > 10) {
    std::cerr << "Wrong number of variables for the Sobol g-Function\n";
    exit(-1);
  }
  if (num_fns < 1 || num_fns > 2) { 
    std::cerr << "Wrong number of functions for the Sobol g-Function\n";
    exit(-1);
  }

  std::ofstream fout(argv[2]);
  if (!fout) {
    std::cerr << "\nError: failure creating " << argv[2] << std::endl;
    exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(std::ios::scientific);
  fout.setf(std::ios::right);

  // Sobol g-Function: see Storlie et al. SAND2008-6570
  int a[] = {0,1,2,4,8,99,99,99,99,99};

  // **** f: 
  if (ASV[0] & 1) {
    double y = 2;
    for (int i=0; i<num_vars; i++)
      y *= ( std::abs(4*x[i]-2) + a[i] ) / ( 1+a[i] );	
    fout << "                     " << y  << " f\n";
  }

  // **** df/dx:
  if (ASV[0] & 2) {
    fout << "[ ";
    double grad_0i;
    for (i=0; i<num_deriv_vars; i++) {
      int var_index = DVV[i] - 1;
      if (4.*x[var_index] == 2.) // zero gradient assumed at discontinuity
	grad_0i = 0.;
      else {
	grad_0i = (4.*x[var_index] > 2.) ?
	  8. / ( 1. + a[var_index] ) : -8. / ( 1. + a[var_index] );
	for (int j=0; j<num_vars; j++)
	  if (j != var_index)
	    grad_0i *= ( std::abs(4.*x[j] - 2.) + a[j] ) / ( 1. + a[j] );
      }
      fout << grad_0i << ' ';
    }
    fout << "]\n";
  }

  fout.flush();
  fout.close();
  return 0;
}
