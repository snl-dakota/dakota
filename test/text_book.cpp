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
//#include "unistd.h" // for sleep


int main(int argc, char** argv)
{

  std::ifstream fin(argv[1]);
  if (!fin) {
    std::cerr << "\nError: failure opening " << argv[1] << std::endl;
    exit(-1);
  }
  size_t i, j, k, num_vars, num_fns, num_deriv_vars;
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

  //srand ( (unsigned int) (time(NULL)/x[0]) );
  //sleep( (int)(5.*((double)rand()/RAND_MAX)));

  //sleep(5);

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

  // **** f:
  if (ASV[0] & 1) {
    double val = 0.;
    for (i=0; i<num_vars; i++)
      val += pow(x[i]-1.0, 4.0);
    fout << "                     " << val << " f\n";
  }

  // **** c: ****
  // "symmetric" constraint pairs are defined from pairs of variables
  // (although odd constraint or variable counts are also allowable):
  // for i=1:num_fns-1, c[i] = x[i-1]^2 - x[i]/2    for  odd i 
  //                    c[i] = x[i-1]^2 - x[i-2]/2  for even i
  for (i=1; i<num_fns; i++) {
    if (ASV[i] & 1) {
      double val = (i-1 < num_vars) ? x[i-1]*x[i-1] : 0;
      if (i%2)
	{ if (i   < num_vars) val -= x[i]/2.; }   //  odd constraint
      else
	{ if (i-2 < num_vars) val -= x[i-2]/2.; } // even constraint
      fout << "                     " << val << " c" << i << '\n';
    }
  }

  // **** df/dx:
  if (ASV[0] & 2) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++)
      fout << 4.*pow(x[DVV[i]-1] - 1., 3) << ' ';
    fout << "]\n";
  }

  // **** dc/dx:
  for (i=1; i<num_fns; i++) {
    if (ASV[i] & 2) {
      fout << "[ ";
      for (j=0; j<num_deriv_vars; j++) {
	int var_index = DVV[j] - 1;
	if (i-1 < num_vars && var_index == i-1) // both constraints
	  fout << 2.*x[i-1] << ' ';
	else if ( (i   < num_vars &&   i%2  && var_index == i) || //  odd constr
		  (i-2 < num_vars && !(i%2) && var_index == i-2) )// even constr
	  fout << -0.5 << ' ';
	else
	  fout <<  0.  << ' ';
      }
      fout << "]\n";
    }
  }

  // **** d^2f/dx^2:
  if (ASV[0] & 4) {
    fout << "[[ ";
    for (i=0; i<num_deriv_vars; i++)
      for (j=0; j<num_deriv_vars; j++)
        if (i==j) fout << 12.*pow(x[DVV[i]-1] - 1., 2) << ' ';
        else      fout << 0. << ' ';
    fout << "]]\n";
  }

  // **** d^2c/dx^2:
  for (i=1; i<num_fns; i++) {
    if (ASV[i] & 4) {
      fout << "[[ ";
      for (j=0; j<num_deriv_vars; j++)
	for (k=0; k<num_deriv_vars; k++)
	  if (i-1<num_vars && DVV[j]==i && DVV[k]==i) fout << 2. << ' ';
	  else                                        fout << 0. << ' ';
      fout << "]]\n";
    }
  }

  fout.flush();
  fout.close();
  return 0;
}
