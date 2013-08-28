/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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
using namespace std;

enum var_t { E1, E2, E3, E4, E5, E6, E7, E8, E9, E10 };


int main(int argc, char** argv)
{
  // The Rosenbrock function may be solved as either a general minimization
  // problem with Objective function = 100.*(x1-x0^2)^2 + (1-x0)^2
  // or a least squares problem with Term1 = 10.*(x1-x0^2) and Term2 = (1-x0).
  // See p. 95 in Practical Optimization by Gill, Murray, and Wright. 

  // This application program reads and writes parameter and response data 
  // directly so no input/output filters are needed.
  ifstream fin(argv[1]);
  if (!fin) {
    cerr << "\nError: failure opening " << argv[1] << endl;
    exit(-1);
  }
  size_t i, j, num_vars, num_fns, num_deriv_vars;
  string vars_text, fns_text, dvv_text;

  // define the string to enumeration map
  map<string, var_t> var_t_map;
  var_t_map["e1"]  = E1; var_t_map["e2"] = E2; var_t_map["e3"] = E3;
  var_t_map["e4"]  = E4; var_t_map["e5"] = E5; var_t_map["e6"] = E6;
  var_t_map["e7"]  = E7; var_t_map["e8"] = E8; var_t_map["e9"] = E9;
  var_t_map["e10"] = E10;

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

  if (num_vars != 10) {
    cerr << "Wrong number of variables for the rosenbrock problem\n";
    exit(-1);
  }
  // ignore num_fns
  // if (num_fns < 1 || num_fns > 2) { // 1 fn -> opt, 2 fns -> least sq
  //   cerr << "Wrong number of functions in rosenbrock problem\n";
  //   exit(-1);
  // }

  ofstream fout("time_history.dat");
  if (!fout) {
    cerr << "\nError: failure creating time history output file." << endl;
    exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(ios::scientific);
  fout.setf(ios::right);

  // Compute and output time step data
  size_t num_delta = 1000;
  double t0 = 0., tf = 10., delta_t = (tf - t0) / num_delta, f1, f2;
  fout << num_delta << std::endl;

  // Compute and output time histories
  double t = t0;
  for (i=0; i<num_delta; ++i) {
    f1 = vars[E1] + t * (vars[E3] + t * (vars[E5] + t * vars[E7]));
    f2 = vars[E2] + t * (vars[E4] + t * (vars[E6] + t * vars[E8]));
    fout << '\t' << f1 << '\t' << f2 << '\n';
    t += delta_t;
  }

  fout.flush();
  fout.close();
  return 0;
}
