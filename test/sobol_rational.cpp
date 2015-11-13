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

  if (num_vars != 2) {
    std::cerr << "Wrong number of variables for this problem\n";
    exit(-1);
  }
  if (num_fns != 1) { 
    std::cerr << "Wrong number of functions for this problem\n";
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

  // f = (x2 + 0.5)^4 / (x1 + 0.5)^2 
  // See Storlie et al. SAND2008-6570

  // **** f: 
  fout << "                     "
       << pow((x[1] + 0.5), 4.0) / pow((x[0] + 0.5), 2.0) << " f\n";

  // **** df/dx:
  if (ASV[0] & 2) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++) {
      int var_index = DVV[i] - 1;
      switch (var_index) {
      case 0: // x1
	fout << -2.*pow(x[1] + 0.5, 4.0)/pow(x[0] + 0.5, 3.0) << ' ';
	break;
      case 1: // x2
	fout <<  4.*pow(x[1] + 0.5, 3.0)/pow(x[0] + 0.5, 2.0) << ' ';
	break;
      }
    }
    fout << "]\n";
  }

  fout.flush();
  fout.close();
  return 0;
}
