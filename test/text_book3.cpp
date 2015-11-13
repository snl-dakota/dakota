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

// eventually just use _WIN32 here
#if defined(_WIN32) || defined(_MSC_VER) || defined(__MINGW32__)
#include <windows.h> // for Sleep()
#elif defined(HAVE_UNISTD_H)
#include <unistd.h> // for usleep()
#endif


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

#if defined(_WIN32) || defined(_MSC_VER) || defined(__MINGW32__)
  //Sleep(500); // 500 milliseconds = 0.5 seconds
#elif defined(HAVE_UNISTD_H)
  //sleep(1);
#endif // SLEEP

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

  // text_book3 calculates active c2 data & outputs 0's for active f & c1 data

  // **** c2:
  for (i=0; i<num_fns; i++) {
    if (i==2) {
      if (ASV[2] & 1)
        fout << "                     " << x[1]*x[1] - 0.5*x[0] << " fn2\n";
    }
    else if (ASV[i] & 1)
      fout << "                     0.0 fn" << i << '\n';
  }

  // **** dc2/dx:
  for (i=0; i<num_fns; i++) {
    if (i==2) {
      if (ASV[2] & 2) {
	fout << "[ ";
	for (j=0; j<num_deriv_vars; j++) {
	  int var_index = DVV[j] - 1;
	  if (var_index == 0)
	    fout << -0.5 << ' ';
	  else if (var_index == 1)
	    fout << 2.*x[1] << ' ';
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

  // **** d^2c2/dx^2: (ParamStudy testing of multiple Hessian matrices)
  for (i=0; i<num_fns; i++) {
    if (i==2) {
      if (ASV[2] & 4) {
	fout << "[[ ";
	for (j=0; j<num_deriv_vars; j++) {
	  int var_index_j = DVV[j] - 1;
	  for (k=0; k<num_deriv_vars; k++) {
	    int var_index_k = DVV[k] - 1;
	    if (var_index_j==1 && var_index_k==1)
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
