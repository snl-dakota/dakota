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
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/variate_generator.hpp>


int main(int argc, char** argv)
{
  // The 20-input/1-output screening design test problem that appeared in 
  //   Morris, M.D. Technometrics. 33 (1991), 161--174.
  // See also p. 88 in Sensitivity Analysis in Practice by Saltelli, Tarantola,
  // Campolongo, and Ratto where it's indicated that one should be able to 
  // conclude the following:
  //  (i)   the first ten factors are important;
  //  (ii)  of these, the first seven have significant effects that involve 
  //        either interactions or curvatures
  //  (iii) the other three are important mainly because of their first-order 
  //        effect

  // This application program reads and writes parameter and response data 
  // directly so no input/output filters are needed.
  std::ifstream fin(argv[1]);
  if (!fin) {
    std::cerr << "\nError: failure opening " << argv[1] << std::endl;
    std::exit(-1);
  }
  std::size_t i, num_vars, num_fns;
  std::string vars_text, fns_text;

  // Get the parameter vector and ignore the labels
  // (using 1-based indexing for clarity with Saltelli text
  fin >> num_vars >> vars_text;
  std::vector<double> x(num_vars+1);
  for (i=1; i<=num_vars; i++) {
    fin >> x[i];
    fin.ignore(256, '\n');
  }

  // Get the ASV vector and ignore the labels
  fin >> num_fns >> fns_text;
  std::vector<int> ASV(num_fns);
  for (i=0; i<num_fns; i++) {
    fin >> ASV[i];
    fin.ignore(256, '\n');
  }

  if (num_vars != 20) {
    std::cerr << "Wrong number of variables for the morris problem\n";
    std::exit(-1);
  }
  if (num_fns != 1) { 
    std::cerr << "Wrong number of functions in morris problem\n";
    std::exit(-1);
  }

  // deprecated: need total of 185 random numbers to read in from randn_185.dat
  // if reverting, use randnums[rc++] where gsl_ran_gaussian appears below
  // NOTE!  This driver depends on a file randn_185.dat, which must contain 185
  //        numbers sampled from a standard normal distribution, one per line
  /*
  std::vector<double> randnums(185);
  std::ifstream rin("randn_185.dat");
  if (!rin) {
    std::cerr << "\nError: failure opening randn_185.dat" << std::endl;
    std::exit(-1);
  }
  for (int rc=0; rc<185; rc++) {
    rin >> randnums[rc];
    rin.ignore(256, '\n');
  }
  */

  // random number generation (Boost mersenne_twister) with fixed seed
  // for repeatability
  namespace bran = boost::random;
  bran::mt19937 generator(41u);
  bran::normal_distribution<> n;
  bran::variate_generator<bran::mt19937&,bran::normal_distribution<> > ran_gaussian(generator, n);

  // compute response
  // create weights from inputs x
  std::vector<double> w(num_vars+1);
  for (int i=1; i<=20; i++) {
    
    if (i == 3 || i == 5 || i == 7)
      w[i] = 2 * (1.1 * x[i] / (x[i] + 0.1) - 0.5);
    else
    w[i] = 2 * (x[i] - 0.5);
    
  }
  // iterate and add terms to the cumulative sum in y
  double y = 0, beta;
  for (int i=1; i<=20; i++) {  

    if (i<=10)
      beta = 20;
    else
      beta = ran_gaussian();
    y += beta*w[i];

    for (int j=i+1; j<=20; j++) {

      //if (i <= 6 && j <= 6)
      if (j <= 6)
	beta = -15;
      else
	beta = ran_gaussian();
      y += beta*w[i]*w[j];

      for (int l=j+1; l<=20; l++) {

	//if (i <= 5 && j <= 5 && l <= 5)
	if (l <= 5) {
	  beta = -10;
	  y += beta*w[i]*w[j]*w[l];
	}

	for (int s=l+1; s<=20; s++) {

	  //if (i <= 4 && j <= 4 && l <= 4 && s <= 4)
	  if (s <= 4) {
	    beta = 5;
	    y += beta*w[i]*w[j]*w[l]*w[s];
	  }

	} //end for s

      } //end for l

    } //end for j

  } // end for i

  // output response
  std::ofstream fout(argv[2]);
  if (!fout) {
    std::cerr << "\nError: failure creating " << argv[2] << std::endl;
    std::exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(std::ios::scientific);
  fout.setf(std::ios::right);

  if (ASV[0] & 1) // **** f:
    fout << "                     " << y << " f\n";

  fout.flush();
  fout.close();
  return 0;
}

