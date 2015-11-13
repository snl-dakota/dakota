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

  // This sample Dakota program implements the MOP2 problem 
  // defined on page 5-13 of David Van Veldhuizen's Ph.D. 
  // thesis at the Air Force Institute of Technology, titled 
  // "Multiobjective Evolutionary Algorithms:  Classifications, 
  // Analyses, and New Innovations.
  // This application program reads and writes parameter and response data 
  // directly so that the NO_FILTER option of dakota may be used.
        
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

  if (num_vars != 2) {
    std::cerr << "Wrong number of variables for the MOP6 problem\n";
    exit(-1);
  }
  if (num_fns != 2) {
    std::cerr << "Wrong number of functions for the MOP6 problem\n";
    exit(-1);
  }

  // Compute and output responses
  double f0=0;
  double f1=0;
  const double PI = 3.14159265358979;
  f0 = x[0];
  f1 = sin(2*PI*4*x[0]);
  f1 = f1*(-1*x[0]/(1+10*x[1]));
  f1 = f1+1-pow((x[0]/(1+10*x[1])),2);
  f1 = f1*(1+10*x[1]);

  std::ofstream fout(argv[2]);
  if (!fout) {
    std::cerr << "\nError: failure creating " << argv[2] << std::endl;
    exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(std::ios::scientific);
  fout.setf(std::ios::right);

  // **** f:
  if (ASV[0] & 1)
    fout << "                     " << f0 << " f0\n";
    fout << "                     " << f1 << " f1\n";

  fout.flush();
  fout.close();
  return 0;
}

  
