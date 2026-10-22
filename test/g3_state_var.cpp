/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

// G3(n=2) with one design variable (X1) and one fixed state variable (X0).
// Objective: -(X0*X1)*sqrt(2)^2 = -(X0*X1)*2
// Constraint (equality): X0^2 + X1^2 - 1 = 0

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>

int main(int argc, char** argv)
{
  std::ifstream fin(argv[1]);
  if (!fin) {
    std::cerr << "\nError: failure opening " << argv[1] << std::endl;
    exit(-1);
  }

  size_t num_vars, num_fns, num_deriv_vars;
  std::string vars_text, fns_text, dvv_text;

  fin >> num_vars >> vars_text;
  std::map<std::string, double> vars;
  for (size_t i = 0; i < num_vars; ++i) {
    double val; std::string label;
    fin >> val >> label;
    vars[label] = val;
  }

  fin >> num_fns >> fns_text;
  std::vector<int> ASV(num_fns);
  for (size_t i = 0; i < num_fns; ++i) {
    fin >> ASV[i];
    fin.ignore(256, '\n');
  }

  fin >> num_deriv_vars >> dvv_text;
  for (size_t i = 0; i < num_deriv_vars; ++i)
    fin.ignore(256, '\n');

  double x0 = vars.count("X0") ? vars["X0"] : 0.0;
  double x1 = vars.count("X1") ? vars["X1"] : 0.0;

  std::ofstream fout(argv[2]);
  if (!fout) {
    std::cerr << "\nError: failure creating " << argv[2] << std::endl;
    exit(-1);
  }
  fout.precision(15);
  fout.setf(std::ios::scientific);
  fout.setf(std::ios::right);

  if (ASV[0] & 1)
    fout << "                     " << -(x0 * x1) * 2.0 << " f\n";

  if (num_fns > 1 && (ASV[1] & 1))
    fout << "                     " << x0*x0 + x1*x1 - 1.0 << " c1\n";

  return 0;
}
