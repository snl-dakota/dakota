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
#include <map>
#include <algorithm>
#include <cctype>
using namespace std;

enum var_t { X1, X2 };


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
  var_t_map["x1"] = X1;
  var_t_map["x2"] = X2;

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

  if (num_vars != 2) {
    cerr << "Wrong number of variables for the rosenbrock problem\n";
    exit(-1);
  }
  if (num_fns < 1 || num_fns > 2) { // 1 fn -> opt, 2 fns -> least sq
    cerr << "Wrong number of functions in rosenbrock problem\n";
    exit(-1);
  }

  // Compute and output responses
  bool least_sq_flag = (num_fns > 1) ? true : false;
  double x1 = vars[X1], x2 = vars[X2];
  double f1 = x2-x1*x1;
  double f2 = 1.-x1;

  ofstream fout(argv[2]);
  if (!fout) {
    cerr << "\nError: failure creating " << argv[2] << endl;
    exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(ios::scientific);
  fout.setf(ios::right);

  if (least_sq_flag) {
    if (ASV[0] & 1) // **** Residual R1:
      fout << "                     " << 10.*f1  << " f1\n";
    if (ASV[1] & 1) // **** Residual R2:
      fout << "                     " << f2  << " f2\n";

    if (ASV[0] & 2) { // **** dR1/dx:
      fout << "[ ";
      for (i=0; i<num_deriv_vars; i++)
	switch (DVV[i]) {
	case X1: fout << -20.*x1 << ' '; break;
	case X2: fout <<  10.    << ' '; break;
	}
      fout << "]\n ";
    }
    if (ASV[1] & 2) { // **** dR2/dx:
      fout << "[ ";
      for (i=0; i<num_deriv_vars; i++)
	switch (DVV[i]) {
	case X1: fout << -1. << ' '; break;
	case X2: fout <<  0. << ' '; break;
	}
      fout << "]\n";
    }

    if (ASV[0] & 4) { // **** d^2R1/dx^2:
      fout << "[[ ";
      for (i=0; i<num_deriv_vars; i++)
	for (j=0; j<num_deriv_vars; j++)
	  if (DVV[i] == X1 && DVV[j] == X1)
	    fout <<  -20. << ' ';
	  else
	    fout <<    0. << ' ';
      fout << "]]\n";
    }
    if (ASV[1] & 4) { // **** d^2R2/dx^2:
      fout << "[[ ";
      for (i=0; i<num_deriv_vars; i++)
	for (j=0; j<num_deriv_vars; j++)
	  fout << 0. << ' ';
      fout << "]]\n";
    }
  }
  else {
    if (ASV[0] & 1) { // **** f:
      double f = 100.*f1*f1+f2*f2;
      bool fail = (f > 250.); // fail away from min value (less problematic)
      //bool fail = (f < 5.); // fail near min value (more problematic)
      if (fail) fout << "                     nan f\n";
      else      fout << "                     " << f << " f\n";
    }

    if (ASV[0] & 2) { // **** df/dx:
      fout << "[ ";
      for (i=0; i<num_deriv_vars; i++)
	switch (DVV[i]) {
	case X1: { // failures for high gradients
	  double df_dx1 = -400.*f1*x1 - 2.*f2;
	  if (df_dx1 > 800.) fout << "nan ";
	  else               fout << df_dx1 << ' ';
	  break;
	}
	case X2: {
	  double df_dx2 = 200.*f1;
	  if (df_dx2 > 200.) fout << "nan ";
	  else               fout << df_dx2 << ' ';
	  break;
	}
	}
      fout << "]\n";
    }

    if (ASV[0] & 4) { // **** d^2f/dx^2:
      fout << "[[ ";
      for (i=0; i<num_deriv_vars; i++)
	for (j=0; j<num_deriv_vars; j++)
	  if (DVV[i] == X1 && DVV[j] == X1)
	    fout << -400.*(x2 - 3.*x1*x1) + 2. << ' ';
	  else if ( (DVV[i] == X1 && DVV[j] == X2) ||
		    (DVV[i] == X2 && DVV[j] == X1) )
	    fout << -400.*x1 << ' ';
	  else if (DVV[i] == X2 && DVV[j] == X2)
	    fout <<  200. << ' ';
      fout << "]]\n";
    }
  }

  fout.flush();
  fout.close();
  return 0;
}
