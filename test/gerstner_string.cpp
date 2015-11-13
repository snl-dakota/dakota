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
  size_t i, num_vars, num_fns;
  std::string vars_text, fns_text;

  // Get the parameter std::vector and ignore the labels
  fin >> num_vars >> vars_text;
  std::vector<double> x(num_vars-1);
  for (i=0; i<num_vars-1; i++) {
    fin >> x[i];
    fin.ignore(256, '\n');
  }

  std::string thisstring; 
  fin >> thisstring;
  fin.ignore(256, '\n');
  //cout << thisstring << "this std::string";

  // Get the ASV std::vector and ignore the labels
  fin >> num_fns >> fns_text;
  std::vector<int> ASV(num_fns);
  for (i=0; i<num_fns; i++) {
    fin >> ASV[i];
    fin.ignore(256, '\n');
  }

  if (num_vars != 3) {
    std::cerr << "Wrong number of variables for the gerstner problem\n";
    exit(-1);
  }
  if (num_fns != 1) {
    std::cerr << "Wrong number of functions for the gerstner problem\n";
    exit(-1);
  }
 
  std::ofstream fout(argv[2]);
  if (!fout) {
    std::cerr << "\nError: failure creating " << argv[2] << std::endl;
    exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(std::ios::scientific);
  fout.setf(std::ios::right);

  short test_fn; double even_coeff, odd_coeff, inter_coeff;
  if (thisstring        == "iso1")
    { test_fn = 1; even_coeff = odd_coeff = 10.; }
  else if (thisstring   == "iso2")
    { test_fn = 2; even_coeff = odd_coeff = inter_coeff = 1.; }
  else if (thisstring   == "iso3")
    { test_fn = 3; even_coeff = odd_coeff = 10.; }
  else if (thisstring == "aniso1")
    { test_fn = 1; even_coeff =  1.; odd_coeff = 10.; }
  else if (thisstring == "aniso2")
    { test_fn = 2; even_coeff =  1.; odd_coeff = inter_coeff = 10.; }
  else if (thisstring == "aniso3")
    { test_fn = 3; even_coeff = 10.; odd_coeff = 5.; }
  else {
    std::cerr << "Error: analysis component specification required in gerstner "
         << "direct fn." << std::endl;
    exit(-1);
  }
  //cout << even_coeff << "even_coeff" << "\n";
  //cout << odd_coeff << "odd_coeff" << "\n";
  std::vector<double> xC;
  xC.resize(2);
  xC[0]=x[0];
  xC[1]=x[1];

  double fnVal = 0.; 
  // ***** f
  if (ASV[0] & 1) {
    switch (test_fn) {
    case 1:
      fnVal = 0.;
      for (size_t i=0; i<num_vars; ++i)
        fnVal += (i%2) ? odd_coeff*exp(-xC[i]*xC[i]) :
                         even_coeff*exp(-xC[i]*xC[i]); break;
    case 2:
      fnVal = 0.;
      for (size_t i=0; i<num_vars; ++i)
        if (i%2)
          fnVal +=  odd_coeff*exp(xC[i])
                      + inter_coeff*exp(xC[i]*xC[i-1]);
        else
          fnVal += even_coeff*exp(xC[i]);
      break;
    case 3: {
      double sum = 0;
      for (size_t i=0; i<num_vars; ++i)
        sum -= (i%2) ? odd_coeff*xC[i]*xC[i] : even_coeff*xC[i]*xC[i];
      fnVal = exp(sum); break;
    }
    }
  }

  fout << "                     " << fnVal << " f\n";

  fout.flush();
  fout.close();
  return 0;
}

  
