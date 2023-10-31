/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
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
  size_t i, num_vars, num_fns;
  std::string vars_text, fns_text;

  // Get the parameter std::vector and ignore the labels
  fin >> num_vars >> vars_text;
  // TODO: relax to arbitrary reals followed by 1 string
  if (num_vars != 3) {
    std::cerr << "gerstner_string expects 3 variables (2 real, 1 int); found "
	      << num_vars << "\n";
    exit(-1);
  }
  std::vector<double> xC(num_vars-1);
  for (i=0; i<num_vars-1; i++) {
    fin >> xC[i];
    fin.ignore(256, '\n');
  }

  size_t thisint; 
  fin >> thisint;
  fin.ignore(256, '\n');
  //std::cout << thisint << "thisint";

  // Get the ASV std::vector and ignore the labels
  fin >> num_fns >> fns_text;
  if (num_fns != 1) {
    std::cerr << "gerstner_string expects 1 function; Dakota sent " << num_fns << "\n";
    exit(-1);
  }
  std::vector<int> ASV(num_fns);
  for (i=0; i<num_fns; i++) {
    fin >> ASV[i];
    fin.ignore(256, '\n');
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
  // TNP NOTE: These are numbered according to the alphabetical
  // order of the string variables in the corresponding "gerstner_string"
  // model so that discrete string and integer variables have the
  // same order when specified in a Dakota input file.
  if (thisint        == 4)
    { test_fn = 1; even_coeff = odd_coeff = 10.; }
  else if (thisint   == 5)
    { test_fn = 2; even_coeff = odd_coeff = inter_coeff = 1.; }
  else if (thisint   == 6)
    { test_fn = 3; even_coeff = odd_coeff = 10.; }
  else if (thisint == 1)
    { test_fn = 1; even_coeff =  1.; odd_coeff = 10.; }
  else if (thisint == 2)
    { test_fn = 2; even_coeff =  1.; odd_coeff = inter_coeff = 10.; }
  else if (thisint == 3)
    { test_fn = 3; even_coeff = 10.; odd_coeff = 5.; }
  else {
    std::cerr << "Error: analysis component specification required in gerstner "
         << "direct fn." << std::endl;
    exit(-1);
  }
  //cout << even_coeff << "even_coeff" << "\n";
  //cout << odd_coeff << "odd_coeff" << "\n";

  double fnVal = 0.; 
  // ***** f
  if (ASV[0] & 1) {
    switch (test_fn) {
    case 1:
      fnVal = 0.;
      for (size_t i=0; i<num_vars-1; ++i)
        fnVal += (i%2) ? odd_coeff*exp(-xC[i]*xC[i]) :
                         even_coeff*exp(-xC[i]*xC[i]); break;
    case 2:
      fnVal = 0.;
      for (size_t i=0; i<num_vars-1; ++i)
        if (i%2)
          fnVal +=  odd_coeff*exp(xC[i])
                      + inter_coeff*exp(xC[i]*xC[i-1]);
        else
          fnVal += even_coeff*exp(xC[i]);
      break;
    case 3: {
      double sum = 0;
      for (size_t i=0; i<num_vars-1; ++i)
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

  
