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

  // typical usage is 2 design vars + 6 uncertain variables, although the
  // number of uncertain variables can be any factor of two.
  if (num_vars < 4 || num_vars % 2) {
    std::cerr << "Error: Bad number of variables in text_book_ouu direct fn." <<std::endl;
    exit(-1);
  }
  if (num_fns > 3) {
    std::cerr << "Error: Bad number of functions in text_book_ouu direct fn." <<std::endl;
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

  int split = 2 + (num_vars - 2)/2;

  // **** f:
  if (ASV[0] & 1) {
    double f = 0.;
    for(i=2; i<split; i++)
      f += pow(x[i]-10.*x[0], 4.0);
    for(i=split; i<num_vars; i++)
      f += pow(x[i]-10.*x[1], 4.0);
    fout << "                     " << f << " f\n";
  }

  // **** c1:
  if (num_fns>1 && (ASV[1] & 1))
    fout << "                     " << x[0]*(x[2]*x[2] - 0.5*x[3]) << " c1\n";

  // **** c2:
  if (num_fns>2 && (ASV[2] & 1))
    fout << "                     " << x[1]*(x[3]*x[3] - 0.5*x[2]) << " c2\n";


  // **** df/dx (w.r.t. active/uncertain variables):
  if (ASV[0] & 2) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++) {
      int var_index = DVV[i] - 1;
      double f0, f1, x0, x1, xvi;
      switch (var_index) {
      case 0: // design variable derivative
	f0 = 0.; x0 = x[0];
	for (size_t j=2; j<split; j++)
	  f0 += -40.*pow(x[j]-10.*x0, 3.0);
	fout << f0 << ' ';
	break;
      case 1: // design variable derivative
	f1 = 0.; x1 = x[1];
	for (size_t j=split; j<num_vars; j++)
	  f1 += -40.*pow(x[j]-10.*x1, 3.0);
	fout << f1 << ' ';
	break;
      default: // uncertain variable derivative
	xvi = x[var_index];
	if (var_index<split)
	  fout << 4*pow(xvi-10.*x[0], 3.0) << ' ';
	else
	  fout << 4*pow(xvi-10.*x[1], 3.0) << ' ';
	break;
      }
    }
    fout << "]\n";
  }

  // **** dc1/dx (w.r.t. active/uncertain variables):
  if (num_fns>1 && (ASV[1] & 2)) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++) {
      int var_index = DVV[i] - 1;
      switch (var_index) {
      case 0: // design variable derivative
	fout << x[2]*x[2] - 0.5*x[3] << ' '; break;
      case 2: // uncertain variable derivative
	fout << 2*x[0]*x[2] << ' ';          break;
      case 3: // uncertain variable derivative
	fout << -0.5*x[0] << ' ';            break;
      default: // all other derivatives
	fout << "0. ";                       break;
      }
    }
    fout << "]\n";
  }

  // **** dc2/dx (w.r.t. active/uncertain variables):
  if (num_fns>2 && (ASV[2] & 2)) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++) {
      int var_index = DVV[i] - 1;
      switch (var_index) {
      case 1: // design variable derivative
	fout << x[3]*x[3] - 0.5*x[2] << ' '; break;
      case 2: // uncertain variable derivative
	fout << -0.5*x[1] << ' ';            break;
      case 3: // uncertain variable derivative
	fout << 2*x[1]*x[3] << ' ';          break;
      default: // all other derivative
	fout << "0. ";                       break;
      }
    }
    fout << "]\n";
  }

  fout.flush();
  fout.close();
  return 0;
}
