/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

int main(int argc, char** argv)
{

  // The Svanberg five-segment beam has a linear objective and a nonlinear constraint:
  // Svanberg, Krister, "The Method of Moving Asymptotes--A New Method for
  // Structural Optimization," Intl. J. Num. Meth. Vol. 24, 1987, pp. 359-373.
  //
  // Written by: Robert A. Canfield 2/21/18

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
    fin >> x[i] >> vars_text;
    fin.ignore(256, '\n');
  }

  // Compute the results and output them directly to argv[2] (the NO_FILTER
  // option is used).
  
  double f, g;
  double df[5], dg[5];
//std::vector<double> df(num_vars), dg(num_vars);
  int k;
  static const short c_x[5] = { 61, 37, 19, 7, 1 };
  static const short b_x[5] = { -183, -111, -57, -21, -3 };

  /*  Function evaluation of Svanberg's 5-segment beam */
  /* --Input */
  /*  x....... Design variable vector = beam cros-sectional dimensions */
  /* --Output */
  /*  f....... Objective function value f(x)=weight */
  /*  g....... Constraint function value g(x)=tip deflection constraint<=0 */
  f = 0.0;
  g = -1.0;
  for (k = 0; k < 5; k++) {
    f += x[k];
    g += (double)c_x[k] / pow(x[k], 3.0);
  }
  f *= 0.0624;

  /*  Gradient evaluation of Svanberg's 5-segment beam */
  /* --Input */
  /*  x....... Design variable vector = beam cros-sectional dimensions */
  /* --Output */
  /*  df...... Derivatives of objective function value f(x)=weight */
  /*  dg...... Derivatives of constraint function value g(x)=tip deflection */
  for (k = 0; k < 5; k++) {
    df[k] = 0.0624;
    dg[k] = (double)b_x[k] / pow(x[k], 4.0);
  }


  std::ofstream fout(argv[2]); // do not instantiate until ready to write results
  if (!fout) {
    std::cerr << "\nError: failure creating " << argv[2] << std::endl;
    exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(std::ios::scientific);
  fout.setf(std::ios::right);

  // **** f:
     fout << f << '\n';

  // **** c1:
     fout << g << '\n';

  // **** df/dx:
     fout << "[ ";
         for (k = 0; k < 5; k++) { fout << df[k] << " "; }
         fout << " ]\n"; 

  // **** dc1/dx:
     fout << "[ ";
         for (k = 0; k < 5; k++) { fout << dg[k] << " "; }
         fout << " ]\n"; 

  fout.flush();
  fout.close();  
  return 0;
}
