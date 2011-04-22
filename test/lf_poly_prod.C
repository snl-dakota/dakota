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


int main(int argc, char** argv)
{
  using namespace std;

  // A modified "low fidelity" Rosenbrock function for use in testing SBO
  // with heirarchical approximations.  The true Rosenbrock function is:
  // Objective function = 100.*(x1-x0^2)^2 + (1-x0)^2

  // This application program reads and writes parameter and response data 
  // directly so no input/output filters are needed.

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

  if (num_vars != 2) {
    cerr << "Wrong number of variables for the LF CompExp2 problem\n";
    exit(-1);
  }
  if (num_fns != 1) {
    cerr << "Wrong number of functions for the LF CompExp2 problem\n";
    exit(-1);
  }

  // Compute and output responses

  ofstream fout(argv[2]);
  if (!fout) {
    cerr << "\nError: failure creating " << argv[2] << endl;
    exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(ios::scientific);
  fout.setf(ios::right);

  // **** f:
  if (ASV[0] & 1)
    fout << "                     " << x[0]*x[0] - 0.5*x[1] << " f\n";

  // **** df/dx:
  if (ASV[0] & 2)
    fout << "[ " << 2.*x[0] << " -0.5 ]\n";

  // **** d^2f/dx^2:
  if (ASV[0] & 4)
    fout << "[[ 2. 0. 0. 0. ]]\n";

  fout.flush();
  fout.close();
  return 0;
}

  
