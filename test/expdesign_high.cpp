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
  if(num_fns!=1) {
    std::cerr << "\nError: bwlow doesn't support constraints but you said #constraints=" << num_fns-1 << std::endl;
    exit(-1);
  }
  std::vector<int> ASV(num_fns);
  fin >> ASV[0];
  fin.ignore(256, '\n');

  // Get the DVV std::vector and ignore the labels
  fin >> num_deriv_vars >> dvv_text;
  std::vector<int> DVV(num_deriv_vars);
  for (i=0; i<num_deriv_vars; i++) {
    fin >> DVV[i];
    fin.ignore(256, '\n');
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

  // **** f:
  double phi = -18.6448;
  double h = 0.0019;
  double amb_al = 21.289686;
  double a=0.95;
  double b=0.95;
  double L=70;
  double k_al = 2.37;
  double gamma, c1, c2; 
  double result; 

  gamma = std::sqrt(2*(a+b)*h/(a*b*k_al));
  c1 = -(phi/(k_al*gamma))*std::exp(gamma*L)*(h+k_al*gamma)/(std::exp(-gamma*L)*(h-k_al*gamma)+std::exp(gamma*L)*(h+k_al*gamma));
  c2 = (phi/(k_al*gamma))+c1;
  result = c1*std::exp(-gamma*x[0])+c2*std::exp(gamma*x[0])+amb_al;
  //result = c1*std::exp(-gamma*x[3])+c2*std::exp(gamma*x[3])+amb_al;

  fout << "                     " << result << "\n";
  
  fout.flush();
  fout.close();
  return 0;
}
