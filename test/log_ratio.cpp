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

  if (num_vars != 2 || num_fns != 1) {
    std::cerr << "Error: wrong number of inputs/outputs in log_ratio test function."
         << std::endl;
    exit(-1);
  }

  // Compute the results and output them directly to argv[2] (the NO_FILTER
  // option is used).  Response tags are now optional; output them for ease
  // of results readability.
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
    fout << "                     " << x[0]/x[1] << " f\n";

  // **** df/dx:
  if (ASV[0] & 2)
    fout << "[ " << 1./x[1] << ' ' << -x[0]/(x[1]*x[1]) << " ]\n";

  // **** d^2f/dx^2: (full Newton opt., SORM integration)
  if (ASV[0] & 4)
    fout << "[[ 0.0 " << -1./(x[1]*x[1]) << ' ' << -1./(x[1]*x[1]) << ' '
         << 2.*x[0]/pow(x[1],3) << " ]]\n";

  fout.flush();
  fout.close();
  return 0;
}
