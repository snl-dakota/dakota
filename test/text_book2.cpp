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
#include <thread>

int main(int argc, char** argv)
{

  std::ifstream fin(argv[1]);
  size_t i, j, k, num_vars, num_fns, num_deriv_vars;
  std::string vars_text, fns_text, dvv_text;

  // Get the parameter std::vector and ignore the labels
  fin >> num_vars >> vars_text;
  if (!fin) {
    std::cerr << "\nError: failure opening " << argv[1] << std::endl;
    exit(-1);
  }
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

  //std::this_thread::sleep_for(std::chrono::seconds(1));

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

  // text_book2 calculates active c1 data & outputs 0's for active f & c2 data

  // **** c1:
  for (i=0; i<num_fns; i++) {
    if (i==1) {
      if (ASV[1] & 1)
        fout << "                     " << x[0]*x[0] - 0.5*x[1] << " fn1\n";
    }
    else if (ASV[i] & 1)
      fout << "                     0.0 fn" << i << '\n';
  }

  // **** dc1/dx:
  for (i=0; i<num_fns; i++) {
    if (i==1) {
      if (ASV[1] & 2) {
	fout << "[ ";
	for (j=0; j<num_deriv_vars; j++) {
	  int var_index = DVV[j] - 1;
	  if (var_index == 0)
	    fout << 2.*x[0] << ' ';
	  else if (var_index == 1)
	    fout << -0.5 << ' ';
	  else
	    fout << "0. ";
	}
	fout << "]\n";
      }
    }
    else if (ASV[i] & 2) {
      fout << "[ ";
      for (j=0; j<num_deriv_vars; j++)
        fout << "0. ";
      fout << "]\n";
    }
  }

  // **** d^2c1/dx^2: (ParamStudy testing of multiple Hessian matrices)
  for (i=0; i<num_fns; i++) {
    if (i==1) {
      if (ASV[1] & 4) {
	fout << "[[ ";
	for (j=0; j<num_deriv_vars; j++) {
	  int var_index_j = DVV[j] - 1;
	  for (k=0; k<num_deriv_vars; k++) {
	    int var_index_k = DVV[k] - 1;
	    if (var_index_j==0 && var_index_k==0)
	      fout << 2. << ' ';
	    else
	      fout << "0. ";
	  }
	}
	fout << "]]\n";
      }
    }
    else if (ASV[i] & 4) {
      fout << "[[ ";
      for (j=0; j<num_deriv_vars; j++)
        for (k=0; k<num_deriv_vars; k++)
          fout << "0. ";
      fout << "]]\n";
    }
  }

  fout.flush();
  fout.close();
  return 0;
}
