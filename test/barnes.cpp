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

  if (num_vars != 2 || num_fns != 4) {
    std::cerr << "Error: wrong number of inputs/outputs in barnes." << std::endl;
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

  // Verification test for SBO performance.
  // Taken from Rodriguez, Perez, Renaud, et al.
  // Constraints g >= 0.

  double a[] = { 75.196,   -3.8112,    0.12694,    -2.0567e-3,  1.0345e-5,
		 -6.8306,   0.030234, -1.28134e-3,  3.5256e-5, -2.266e-7,
		  0.25645, -3.4604e-3, 1.3514e-5, -28.106,     -5.2375e-6,
		 -6.3e-8,   7.0e-10,   3.4054e-4,  -1.6638e-6, -2.8673,
		  0.0005};
  double x1 = x[0], x2 = x[1], x1x2 = x1*x2, x2_sq = x2*x2, x1_sq = x1*x1;

  // **** f
  if (ASV[0] & 1) {
    double f = a[0] + a[1]*x1 + a[2]*x1_sq + a[3]*x1_sq*x1 + a[4]*x1_sq*x1_sq
      + a[5]*x2 + a[6]*x1x2 + a[7]*x1*x1x2 + a[8]*x1x2*x1_sq
      + a[9]*x2*x1_sq*x1_sq + a[10]*x2_sq + a[11]*x2*x2_sq + a[12]*x2_sq*x2_sq
      + a[13]/(x2+1.) + a[14]*x2_sq*x1_sq + a[15]*x1*x1_sq*x2_sq
      + a[16]*x1x2*x2_sq*x1_sq + a[17]*x1*x2_sq + a[18]*x1x2*x2_sq
      + a[19]*exp(a[20]*x1x2);
    fout << "                     " << f << " f\n";
  }

  // **** g1
  if (ASV[1] & 1)
    fout << "                     " << x1x2/700. - 1. << " g1\n";

  // **** g2
  if (ASV[2] & 1)
    fout << "                     " << x2/5. - x1_sq/625. << " g2\n";

  // **** g3
  if (ASV[3] & 1)
    fout << "                     " << pow(x2/50. - 1., 2.) - x1/500. + 0.11
	 << " g3\n";

  // **** df/dx
  if (ASV[0] & 2) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++) {
      int var_index = DVV[i] - 1;
      switch (var_index) {
      case 0:
	fout << a[1] + 2.*a[2]*x1 + 3.*a[3]*x1_sq + 4.*a[4]*x1_sq*x1
	  + a[6]*x2 + 2.*a[7]*x1x2 + 3.*a[8]*x2*x1_sq + 4.*a[9]*x1x2*x1_sq
	  + 2.*a[14]*x2_sq*x1 + 3.*a[15]*x1_sq*x2_sq + 3.*a[16]*x2*x2_sq*x1_sq
	  + a[17]*x2_sq + a[18]*x2*x2_sq + a[19]*a[20]*x2*exp(a[20]*x1x2) <<' ';
	break;
      case 1:
	fout << a[5] + a[6]*x1 + a[7]*x1_sq + a[8]*x1*x1_sq
	  + a[9]*x1_sq*x1_sq + 2.*a[10]*x2 + 3.*a[11]*x2_sq + 4.*a[12]*x2*x2_sq
	  - a[13]/pow(x2+1., 2.) + 2.*a[14]*x2*x1_sq + 2.*a[15]*x1*x1_sq*x2
	  + 3.*a[16]*x1*x2_sq*x1_sq + 2.*a[17]*x1x2 + 3.*a[18]*x1*x2_sq
	  + a[19]*a[20]*x1*exp(a[20]*x1x2) << ' ';
	break;
      }
    }
    fout << "]\n";
  }

  // **** dg1/dx
  if (ASV[1] & 2) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++) {
      int var_index = DVV[i] - 1;
      switch (var_index) {
      case 0:
	fout << x2/700. << ' ';
	break;
      case 1:
	fout << x1/700. << ' ';
	break;
      }
    }
    fout << "]\n";
  }

  // **** dg2/dx
  if (ASV[2] & 2) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++) {
      int var_index = DVV[i] - 1;
      switch (var_index) {
      case 0:
	fout << -2.*x1/625. << ' ';
	break;
      case 1:
	fout << 0.2 << ' ';
	break;
      }
    }
    fout << "]\n";
  }

  // **** dg3/dx
  if (ASV[3] & 2) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++) {
      int var_index = DVV[i] - 1;
      switch (var_index) {
      case 0:
	fout << -1./500. << ' ';
	break;
      case 1:
	fout << 2.*(x2/50. - 1.)/50. << ' ';
	break;
      }
    }
    fout << "]\n";
  }

  fout.flush();
  fout.close();
  return 0;
}
