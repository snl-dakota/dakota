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

  std::ifstream fin(argv[1]);
  if (!fin) {
    std::cerr << "\nError: failure opening " << argv[1] << std::endl;
    exit(-1);
  }
  size_t i, j, num_vars, num_fns, num_deriv_vars;
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

  if (num_vars != 3 || num_fns != 1) {
    std::cerr << "Error: wrong number of inputs/outputs in steel_column_cost."<<std::endl;
    exit(-1);
  }

  // Compute the results and output them directly to argv[2] (the NO_FILTER
  // option is used).  Response tags are optional; output them for ease
  // of results readability.
  std::ofstream fout(argv[2]);
  if (!fout) {
    std::cerr << "\nError: failure creating " << argv[2] << std::endl;
    exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(std::ios::scientific);
  fout.setf(std::ios::right);

  // In the steel column description in Kuschel & Rackwitz, Cost is _not_
  // defined as a random variable.  That is Cost is not a fn(B, D, H), but
  // is rather defined as a fn(b, d, h).  Since dCost/dX|_{X=mean} is not the
  // same as dCost/dmean for non-normal X (jacobian_dX_dS is not 1), dCost/dX
  // may not be used and an optional interface must be defined for Cost.

  // b  = x[0] = flange breadth   (lognormal unc. var., mean is design var.)
  // d  = x[1] = flange thickness (lognormal unc. var., mean is design var.)
  // h  = x[2] = profile height   (lognormal unc. var., mean is design var.)

  double b = x[0], d = x[1], h = x[2];

  // **** f (objective = bd + 5h = cost of column):
  if (ASV[0] & 1)
    fout << "                     " << b*d + 5.*h << " f\n";

  // **** df/dx:
  if (ASV[0] & 2)
    fout << "[ " << d << ' ' << b << " 5. ]\n";

  // **** d^2f/dx^2:
  if (ASV[0] & 4)
    fout << "[[ 0. 1. 0. 1. 0. 0. 0. 0. 0. ]]\n";

  fout.flush();
  fout.close();
  return 0;
}
