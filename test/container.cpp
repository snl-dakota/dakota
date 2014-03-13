/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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
  using namespace std;

  //******************************
  // read the input from DAKOTA
  //******************************
  ifstream fin(argv[1]);
  if (!fin) {
    cerr << "\nError: failure opening " << argv[1] << endl;
    exit(-1);
  }
  size_t i, num_vars, num_fns;
  string vars_text, fns_text;

  // Get the parameter vector and ignore the labels
  fin >> num_vars >> vars_text;
  vector<double> x(num_vars);
  for (i=0; i<num_vars; i++) {
    fin >> x[i];
    fin.ignore(256, '\n');
  }

  // Get the ASV vector and ignore the labels
  fin >> num_fns >> fns_text;
  vector<int> ASV(num_fns);
  for (i=0; i<num_fns; i++) {
    fin >> ASV[i];
    fin.ignore(256, '\n');
  }

  if (num_vars != 2 || num_fns != 2) {
    cerr << "Error: wrong number of inputs/outputs in container test function."
         << endl;
    exit(-1);
  }

  //********************************************************
  // compute the objective function and constraint values
  // and write the response output for DAKOTA
  //********************************************************
  ofstream fout(argv[2]);
  if (!fout) {
    cerr << "\nError: failure creating " << argv[2] << endl;
    exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(ios::scientific);
  fout.setf(ios::right);

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

