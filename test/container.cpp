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

//**********************************************************************
// container.cpp - C++ container optimization example  
//**********************************************************************

int main(int argc, char** argv)
{

  //******************************
  // read the input from DAKOTA
  //******************************
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

  if (num_vars != 2 || num_fns != 2) {
    std::cerr << "Error: wrong number of inputs/outputs in container test function."
         << std::endl;
    exit(-1);
  }

  //********************************************************
  // compute the objective function and constraint values
  // and write the response output for DAKOTA
  //********************************************************
  std::ofstream fout(argv[2]);
  if (!fout) {
    std::cerr << "\nError: failure creating " << argv[2] << std::endl;
    exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(std::ios::scientific);
  fout.setf(std::ios::right);

  double H = x[0], D = x[1];
  const double PI = 3.14159265358979;
  if (ASV[0] & 1) {
    double fval = 0.644*PI*D*D + 1.04*PI*D*H;
    fout << fval << " area\n";
  }
  if (ASV[1] & 1) {
    double cval = 0.25*PI*H*D*D - 63.525;
    fout << cval << " volume_constraint\n";
  }

  fout.flush();
  fout.close();
  return 0;
}

