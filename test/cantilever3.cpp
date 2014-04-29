/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>

enum var_t { W, T, R, E, X, Y };


int main(int argc, char** argv)
{
  using namespace std;

  // This test problem is an OUU example from Applied Research Associates
  // (42nd AIAA SDM conference, April 2001).

  ifstream fin(argv[1]);
  if (!fin) {
    cerr << "\nError: failure opening " << argv[1] << endl;
    exit(-1);
  }
  size_t i, j, num_vars, num_fns, num_deriv_vars;
  string vars_text, fns_text, dvv_text;

  // define the string to enumeration map
  fin >> num_vars >> vars_text;
  vector<double> xin(num_vars);
  for (i=0; i<num_vars; i++) {
    fin >> xin[i];
    fin.ignore(256, '\n');
  }
  fin >> num_fns >> fns_text;
  vector<int> ASV(num_fns);
  for (i=0; i<num_fns; i++) {
    fin >> ASV[i];
    fin.ignore(256, '\n');
  }

  if (num_vars != 6) {
    cerr << "Error: Wrong number of variables in cantilever test fn." << endl;
    exit(-1);
  }
  if (num_fns != 2) {
    cerr << "Error: wrong number of response functions in cantilever test fn."
         << endl;
    exit(-1);
  }

  // Compute the cross-sectional area, stress, and displacement of the
  // cantilever beam.  This simulator is unusual in that it supports both
  // the case of design variable insertion and the case of design variable
  // augmentation.  It does not support mixed insertion/augmentation.  In
  // the 6 variable case, w,t,R,E,X,Y are all passed in; in the 4 variable
  // case, w,t assume local values.
  //map<var_t, double>::iterator m_iter = vars.find(W);
  //double w = (m_iter == vars.end()) ? 2.5 : m_iter->second; // beam width
  //m_iter = vars.find(T);
  //double t = (m_iter == vars.end()) ? 2.5 : m_iter->second; // beam thickness
  //double r = vars[R]; // yield strength
 // double e = vars[E]; // Young's modulus
 // double x = vars[X]; // horizontal load
  //double y = vars[Y]; // vertical load
 
  double e = xin[0];
  double w =  xin[1]; // horizontal load
  double r = xin[3]; // Young's modulus
  double t =  xin[2]; // beam width
  double x =  xin[4]; // beam thickness
  double y =  xin[5]; // beam thickness

  cout << " w " << w << 
          " t"  << t << 
          " e " << e << 
          " r " << r << 
          " x " << x << 
          "y "  <<y << '\n';
  // optimization inequality constraint: <= 0 and scaled O(1)
  //Real g_stress = stress/R - 1.0;
  //Real g_disp   = disp/D0  - 1.0;

  // Compute the results and output them directly to argv[2] (the NO_FILTER
  // option is used).  Response tags are optional; output them for ease of
  // results readability.
  double D0 = 2.2535, L = 100., area = w*t, w_sq = w*w, t_sq = t*t,
    r_sq = r*r, x_sq = x*x, y_sq = y*y;
  double stress = 600.*y/w/t_sq + 600.*x/w_sq/t;
  double D1 = 4.*pow(L,3)/e/area, D2 = pow(y/t_sq, 2)+pow(x/w_sq, 2);
  double D3 = D1/sqrt(D2)/D0,     D4 = D1*sqrt(D2);

  ofstream fout(argv[2]); // do not instantiate until ready to write results
  if (!fout) {
    cerr << "\nError: failure creating " << argv[2] << endl;
    exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(ios::scientific);
  fout.setf(ios::right);

  // **** f:

  // **** c1:
  if (ASV[0] & 1)
    fout << "                     " << stress -r  << '\n';

  // **** c2:
  if (ASV[1] & 1)
    fout << "                     " << D4 - D0 << '\n';


  fout.flush();
  fout.close();  
  return 0;
}
