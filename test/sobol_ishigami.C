/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
#include <cmath>
using namespace std;

enum var_t { X1, X2, X3 };


int main(int argc, char** argv)
{
  // This application program reads and writes parameter and response data 
  // directly so no input/output filters are needed.
  ifstream fin(argv[1]);
  if (!fin) {
    cerr << "\nError: failure opening " << argv[1] << endl;
    exit(-1);
  }
  size_t i, num_vars, num_fns, num_deriv_vars;
  string vars_text, fns_text, dvv_text;

  // define the string to enumeration map
  map<string, var_t> var_t_map;
  var_t_map["x1"] = X1; var_t_map["x2"] = X2; var_t_map["x3"] = X3;

  // Get the parameter vector and ignore the labels
  fin >> num_vars >> vars_text;
  map<var_t, double> vars;
  vector<var_t> labels(num_vars);
  double var_i; string label_i; var_t v_i;
  map<string, var_t>::iterator v_iter;
  for (i=0; i<num_vars; i++) {
    fin >> var_i >> label_i;
    transform(label_i.begin(), label_i.end(), label_i.begin(),
	      (int(*)(int))tolower);
    v_iter = var_t_map.find(label_i);
    if (v_iter == var_t_map.end()) {
      cerr << "Error: label \"" << label_i << "\" not supported in analysis "
	   << "driver." << endl;
      exit(-1);
    }
    else
      v_i = v_iter->second;
    vars[v_i] = var_i;
    labels[i] = v_i;
  }

  // Get the ASV vector and ignore the labels
  fin >> num_fns >> fns_text;
  vector<short> ASV(num_fns);
  for (i=0; i<num_fns; i++) {
    fin >> ASV[i];
    fin.ignore(256, '\n');
  }

  // Get the DVV vector and ignore the labels
  fin >> num_deriv_vars >> dvv_text;
  vector<var_t> DVV(num_deriv_vars);
  unsigned int dvv_i;
  for (i=0; i<num_deriv_vars; i++) {
    fin >> dvv_i;
    fin.ignore(256, '\n');
    DVV[i] = labels[dvv_i-1];
  }

  if (num_vars != 3) {
    cerr << "Wrong number of variables for the Ishigami function" << endl;
    exit(-1);
  }
  if (num_fns != 1) { 
    cerr << "Wrong number of functions for the Ishigami function" << endl;
    exit(-1);
  }

  ofstream fout(argv[2]);
  if (!fout) {
    cerr << "\nError: failure creating " << argv[2] << endl;
    exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(ios::scientific);
  fout.setf(ios::right);

  // Ishigami Function: see Storlie et al. SAND2008-6570
  const double pi = 3.14159265358979324;
  double x1 = vars[X1], x2 = vars[X2], x3 = vars[X3];

  // **** f: 
  if (ASV[0] & 1)
    fout << "                     " << ( 1. + 0.1 * pow(2.*pi*x3 - pi, 4.0) )
      * sin(2.*pi*x1 - pi) + 7. * pow(sin(2*pi*x2 - pi), 2.0) << " f\n";

  // **** df/dx
  if (ASV[0] & 2) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++)
      switch (DVV[i]) {
      case X1: // x1
	fout << 2.*pi * ( 1. + 0.1*pow(2.*pi*x3 - pi, 4.) ) * cos(2.*pi*x1 - pi)
	     << ' ';
	break;
      case X2: // x2
	fout << 28.*pi * sin(2.*pi*x2 - pi)   * cos(2.*pi*x2 - pi) << ' ';
	break;
      case X3: // x3
	fout << 0.8 * pow(2.*pi*x3 - pi, 3.0) * sin(2.*pi*x1 - pi) << ' ';
	break;
      }
    fout << "]\n";
  }

  fout.flush();
  fout.close();
  return 0;
}
