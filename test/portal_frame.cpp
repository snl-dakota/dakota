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

  if (num_vars != 6 || num_fns != 1) {
    std::cerr << "Error: wrong number of inputs/outputs in portal_frame." << std::endl;
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

  // Verification test for second-order integration in reliability methods.
  // Taken from Tvedt (1990), Hong (1999), etc.
  //   g = x1 + 2*x2 + 2*x3 + x4 - 5*x5 - 5*x6
  // "one plastic collapse mechanism of a simple portal frame"
  // Note: the limit state is linear in x-space, but the u-space transformation
  // for lognormals introduces curvature.

  // **** g (limit state = portal frame response):
  if (ASV[0] & 1)
    fout << "                     "
	 << x[0] + 2.*x[1] + 2.*x[2] + x[3] - 5.*x[4] - 5.*x[5] << " g\n";

  // **** dg/dx (w.r.t. active/uncertain variables):
  if (ASV[0] & 2) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++) {
      int var_index = DVV[i] - 1;
      switch (var_index) {
      case 0:
	fout << "1. ";
	break;
      case 1:
	fout << "2. ";
	break;
      case 2:
	fout << "2. ";
	break;
      case 3:
	fout << "1. ";
	break;
      case 4:
	fout << "-5. ";
	break;
      case 5:
	fout << "-5. ";
	break;
      }
    }
    fout << "]\n";
  }

  // **** d^2g/dx^2: (SORM)
  if (ASV[0] & 4) {
    fout << "[[ ";
    for (i=0; i<num_deriv_vars; i++)
      for (j=0; j<num_deriv_vars; j++)
	fout << "0. ";
    fout << "]]\n";
  }

  fout.flush();
  fout.close();
  return 0;
}
