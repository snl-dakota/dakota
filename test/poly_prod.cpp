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
    std::cerr << "Wrong number of variables for the CompExp2 problem\n";
    exit(-1);
  }
  if (num_fns != 1) {
    std::cerr << "Wrong number of functions for the CompExp2 problem\n";
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

  // Compute and output responses
  double x0_sq = x[0]*x[0], x1_sq = x[1]*x[1];
  double t2 = x0_sq - 0.5*x[1];

  //double ratio = x[0]/x[1];
  //double t1 = 1. + ratio;

  //double t1 = 1. + x0_sq/2. + x1_sq/2.;

  //double t1 = 1. - x[0] + x1_sq/2.;

  //double t1 = 1. - x[0] - x1_sq/2.;

  double t1 = x[0] + x1_sq/2.;

  // **** f:
  if (ASV[0] & 1)
    fout << "                     " << t1*t2 << " f\n";

  // **** df/dx:
  if (ASV[0] & 2)
    //fout << "[ " << 2.*x[0]*t1 + t2/x[1] << ' ' << -t1/2. - t2*x[0]/x[1]/x[1]
    //     << " ]\n";
    //fout << "[ " << x[0]*(2.*t1 + t2) << ' ' << t2*x[1] - t1/2. << " ]\n";
    //fout << "[ " << 2.*x[0]*t1 - t2 << ' ' << t2*x[1] - t1/2. << " ]\n";
    //fout << "[ " << 2.*x[0]*t1 - t2 << ' ' << -t1/2. - t2*x[1] << " ]\n";
    fout << "[ " << 2.*x[0]*t1 + t2 << ' ' << t2*x[1] - t1/2. << " ]\n";

  // **** d^2f/dx^2:
  if (ASV[0] & 4)
    //fout << "[[ " << 2. + 6*ratio << ' ' << -3.*pow(ratio, 2) << ' '
    //     << -3.*pow(ratio, 2) << ' ' << 2*pow(ratio, 3) << " ]]\n";
    //fout << "[[ " << 4.*x0_sq + 2.*t1 + t2 << ' ' << x[0]*(2.*x[1] - 0.5)
    //     << ' ' << x[0]*(2.*x[1] - 0.5) << ' ' << -x[1] << " ]]\n";
    //fout << "[[ " << 2.*t1 - 4.*x[0] << ' ' << 2.*x[0]*x[1] + 0.5 << ' '
    //     << 2.*x[0]*x[1] + 0.5 << ' ' << t2 - x[1] << " ]]\n";
    //fout << "[[ " << 2.*t1 - 4.*x[0] << ' ' << 0.5 - 2.*x[0]*x[1] << ' '
    //     << 0.5 - 2.*x[0]*x[1] << ' ' << x[1] - t2 << " ]]\n";
    fout << "[[ " << 4.*x[0] + 2.*t1 << ' ' << 2.*x[0]*x[1] - 0.5 << ' '
	 << 2.*x[0]*x[1] - 0.5 << ' ' << t2 - x[1] << " ]]\n";

  fout.flush();
  fout.close();
  return 0;
}

  
