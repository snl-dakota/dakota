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

  if (num_vars != 2 || num_fns != 1) {
    std::cerr << "Error: wrong number of inputs/outputs in steel_section." << std::endl;
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
  // Taken from Haldar & Mahadevan, 2000.
  //   g = F_y * Z - 1140.
  // "A W16x31 steel section made of A36 steel is suggested to carry an
  // applied deterministic bending moment of 1140 kip-in"

  double Z = x[0], F_y = x[1];

  // **** g (limit state = portal frame response):
  if (ASV[0] & 1)
    fout << "                     " << F_y * Z - 1140. << " g\n";

  // **** dg/dx (w.r.t. active/uncertain variables):
  if (ASV[0] & 2) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++) {
      int var_index = DVV[i] - 1;
      switch (var_index) {
      case 0:
	fout << F_y << ' '; // dg/dZ
	break;
      case 1:
	fout << Z << ' ';   // dg/dF_y
	break;
      }
    }
    fout << "]\n";
  }

  // **** d^2g/dx^2: (SORM)
  if (ASV[0] & 4) {
    fout << "[[ ";
    for (i=0; i<num_deriv_vars; i++) {
      int var_index_i = DVV[i] - 1;
      for (j=0; j<num_deriv_vars; j++) {
	int var_index_j = DVV[j] - 1;
	if ( (var_index_i == 0 && var_index_j == 1) ||
	     (var_index_i == 1 && var_index_j == 0) ) // d^2g/dF_ydZ
	  fout << "1. ";
	else
	  fout << "0. ";
      }
    }
    fout << "]]\n";
  }

  fout.flush();
  fout.close();
  return 0;
}
