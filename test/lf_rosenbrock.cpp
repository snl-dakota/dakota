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


int main(int argc, char** argv)
{

  // A modified "low fidelity" Rosenbrock function for use in testing SBO
  // with heirarchical approximations.  The true Rosenbrock function is:
  // Objective function = 100.*(x1-x0^2)^2 + (1-x0)^2

  // This application program reads and writes parameter and response data 
  // directly so no input/output filters are needed.

  std::ifstream fin(argv[1]);
  if (!fin) {
    std::cerr << "\nError: failure opening " << argv[1] << std::endl;
    exit(-1);
  }
  size_t i, num_vars, num_fns;
  std::string vars_text, fns_text;

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

  if (num_vars != 2) {
    std::cerr << "Wrong number of variables for the rosenbrock problem\n";
    exit(-1);
  }
  if (num_fns != 1) {
    std::cerr << "Wrong number of functions for the rosenbrock problem\n";
    exit(-1);
  }

  // Compute and output responses
  double f0 = x[1]-x[0]*x[0]+.2; // offset
  double f1 = 0.8-x[0];          // offset

  std::ofstream fout(argv[2]);
  if (!fout) {
    std::cerr << "\nError: failure creating " << argv[2] << std::endl;
    exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(std::ios::scientific);
  fout.setf(std::ios::right);

  // **** f:
  if (ASV[0] & 1)
    fout << "                     " << 100.*f0*f0+f1*f1  << " f\n";

  // **** df/dx:
  if (ASV[0] & 2)
    fout << "[ " << -400.*f0*x[0] - 2.*f1 << ' ' << 200.*f0 << " ]\n";

  // **** d^2f/dx^2:
  if (ASV[0] & 4) {
    double fx = x[1]-3.*x[0]*x[0]+.2;
    fout << "[[ " << -400.*fx + 2.0 << ' ' << -400.*x[0] << ' ' << -400.*x[0]
         <<" 200. ]]\n";
  }

  fout.flush();
  fout.close();
  return 0;
}

  
