/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
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
  // Taylor series of Barnes function about the point (p1,p2)
  double p1  = 30.0;
  double p2  = 40.0;
  double x1  = x[0]-p1; 
  double x12 = x1*x1;
  double x13 = x12*x1;
  double x2  = x[1]-p2; 
  double x22 = x2*x2;
  double x23 = x22*x2;
  
  // **** f
  if (ASV[0] & 1) {
    double f = 
      - 2.74465943148169
      + 0.01213957527281*x1
      + 0.00995748775273*x12 
      - 5.557060816484793e-04*x13 
      + (1.15084419109172+0.00947331101091*x1+2.994070392732408e-05*x12)*x2
      + (-0.02997939337414-1.676054720545071e-04*x1)*x22
      - 0.00132216646850*x23;
   fout << "                     " << f << " f\n";
  }

  // **** g1
  if (ASV[1] & 1)
    fout << "                     " << (x[0]+x[1]-50.)/10. << " g1\n";

  // **** g2
  if (ASV[2] & 1)
    fout << "                     " << (-0.64*x[0]+x[1])/6. << " g2\n";

  // **** g3
  if (ASV[3] & 1) {
    if (x[1] > 50)
      fout << "                     " << 
	-0.00599508167546*x[0] + 0.0134054101569*x[1] - 0.34054101569933
	   << " g3\n";
    else
      fout << "                     " << 
	-0.00599508167546*x[0] - 0.01340541015699*x[1] + 1. 
	   << " g3\n";
  }

  // **** df/dx
  if (ASV[0] & 2) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++) {
      int var_index = DVV[i] - 1;
      switch (var_index) {
      case 0:
	fout <<   
	  - 0.58530968989099+0.01991497550546*x[0]-0.00166711824495*x12 
	  + (0.00767686877527+ 5.988140785464816e-05*x[0])*x2 
	  - 1.676054720545071e-04*x22 
	     <<' ';
	break;
      case 1:
	fout << 
	    0.86664486076442+0.00947331101091*x[0]+2.994070392732408e-05*x12
	  + 2*(-0.02495122921250-1.676054720545071e-04*x[0])*x2
	  - 0.00396649940550*x22
	     << ' ';
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
	fout << 1./10. << ' ';
	break;
      case 1:
	fout << 1./10. << ' ';
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
	fout << -0.64/6. << ' ';
	break;
      case 1:
	fout << 1./6. << ' ';
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
	fout << -0.00599508167546 << ' ';
	break;
      case 1:
	if (x[1] > 50)
	  fout << 0.01340541015692 << ' ';
	else
	  fout << -0.01340541015692 << ' ';
	break;
      }
    }
    fout << "]\n";
  }

  fout.flush();
  fout.close();
  return 0;
}
