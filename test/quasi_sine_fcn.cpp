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


int main(int argc, char** argv)
{

  // The "quasi-sine" test function from my thesis (p. 92) for 
  // num_vars = 2.
  //
  // The global minimum is at the point (x1,x2) = (0.177,0.177)
  // with a function value of 0.060.
  // 
  // This program computes the function value and gradients, but
  // not the Hessian.
  //
  // A. A. Giunta, 15 June 2000 

  // !!! Hessian not available !!!

  // This application program reads and writes parameter and response data 
  // directly so that the NO_FILTER option of dakota may be used.

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
    std::cerr << "Wrong number of variables for the quasi-sine problem\n";
    exit(-1);
  }
  if (num_fns != 1) {
    std::cerr << "Wrong number of functions for the quasi-sine problem\n";
    exit(-1);
  }

  // Compute and output responses
  double alpha = 16.0*x[0]/15.0 - 0.7;
  double beta  = 16.0*x[1]/15.0 - 0.7;
  double gamma = 40.0*alpha;
  double delta = 40.0*beta;

  std::ofstream fout(argv[2]);
  if (!fout) {
    std::cerr << "\nError: failure creating " << argv[2] << std::endl;
    exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(std::ios::scientific);
  fout.setf(std::ios::right);

  // **** f:
  if (ASV[0] & 1) {
    double fcn_eval = 0.6 + sin(alpha) + sin(beta) + sin(alpha)*sin(alpha)
                    + sin(beta)*sin(beta) + 0.02*sin(gamma) + 0.02*sin(delta);
    fout << "                     " << fcn_eval << " f\n";
  }

  // **** df/dx:
  if (ASV[0] & 2) {
    double grad1 = cos(alpha)*16.0/15.0 + sin(alpha)*cos(alpha)*32.0/15.0 
                 + cos(gamma)*64.0/75.0;
    double grad2 = cos(beta)*16.0/15.0 + sin(beta)*cos(beta)*32.0/15.0 
                 + cos(delta)*64.0/75.0;
    fout << "[ " << grad1 << ' ' << grad2 << " ]\n";
  }

  // **** hessian ddf/dxidxj
  if (ASV[0] & 4) {
    fout << "Hessian not available for the quasi-sine test function";
    exit(-1);
  }

  fout.flush();
  fout.close();
  return 0;
}
