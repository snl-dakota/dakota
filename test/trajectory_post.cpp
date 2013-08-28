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

enum var_t { FT1,  FT2 };


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
  size_t i, j, num_vars, num_fns, num_deriv_vars, num_ac;
  string text, hier_tag;

  // define the string to enumeration map
  map<string, var_t> var_t_map;
  var_t_map["failthresh1"] = FT1; var_t_map["failthresh2"] = FT2;

  // Get the parameter vector and ignore the labels
  fin >> num_vars >> text;
  map<var_t, double> vars;
  //vector<var_t> labels(num_vars);
  double var_i; string label_i; var_t v_i;
  map<string, var_t>::iterator v_iter;
  for (i=0; i<num_vars; i++) {
    fin >> var_i >> label_i;
    transform(label_i.begin(), label_i.end(), label_i.begin(),
	      (int(*)(int))tolower);
    v_iter = var_t_map.find(label_i);
    // if (v_iter == var_t_map.end()) {
    //   cerr << "Error: label \"" << label_i << "\" not supported in analysis "
    // 	   << "driver." << endl;
    //   exit(-1);
    // }
    // else
    //   v_i = v_iter->second;
    // vars[v_i] = var_i;
    // labels[i] = v_i;

    // ignore any epistemic variables; only need the failure thresholds
    if (v_iter != var_t_map.end())
      { v_i = v_iter->second; vars[v_i] = var_i; }
  }
  if (vars.size() != 2) {
    cerr << "Wrong number of variables for trajectory_post\n";
    exit(-1);
  }

  // Get the ASV vector and ignore the labels
  fin >> num_fns >> text;
  vector<short> ASV(num_fns);
  for (i=0; i<num_fns; i++) {
    fin >> ASV[i];
    fin.ignore(256, '\n');
  }
  if (num_fns < 1 || num_fns > 2) {
    cerr << "Wrong number of functions in trajectory_post\n";
    exit(-1);
  }

  // Get the DVV vector and ignore the labels
  fin >> num_deriv_vars >> text;
  //vector<var_t> DVV(num_deriv_vars);
  unsigned int dvv_i;
  for (i=0; i<num_deriv_vars; i++) {
    fin >> dvv_i;
    fin.ignore(256, '\n');
    //DVV[i] = labels[dvv_i-1];
  }

  // Extract the AC vector and ignore the labels
  fin >> num_ac >> text;
  vector<string> AC(num_ac);
  for (i=0; i<num_ac; i++) {
    fin >> AC[i];
    fin.ignore(256, '\n');
  }

  // Extract the hierarchical fn eval tags (required to link to opt interface)
  fin >> hier_tag; fin.ignore(256, '\n');


  // Compute and output responses
  string h_tag_1 = hier_tag; // *** TO DO ***
  string history = "../epistemic_simulation." + h_tag_1 + "/time_history.dat";
  ifstream hist_in(history.c_str());
  if (!fin) {
    cerr << "\nError: failure opening " << history << endl;
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

  size_t num_delta;
  hist_in >> num_delta;
  double delta_t, t, t_prev = 0., f1, f2, f1_prev, f2_prev,
    fail_thresh1 = vars[FT1], fail_thresh2 = vars[FT2],
    time_fail1, time_fail2;
  bool failed1 = false, failed2 = false;

  for (i=0; i<num_delta; ++i) {
    hist_in >> t >> f1 >> f2;
    if (ASV[0] & 1 && !failed1) {
      // check if f1 has reached failure threshold 1;
      // if yes, then linearly interpolate for failure time
      if (f1 >= fail_thresh1) {
	time_fail1 = t_prev +
	  (fail_thresh1 - f1_prev) / (f1 - f1_prev) * (t - t_prev);
	failed1 = true;
      }
      else
	f1_prev = f1;
    }

    if (ASV[1] & 1) {
      // check if f2 has reached failure threshold 2;
      // if yes, then linearly interpolate for failure time
      if (f2 >= fail_thresh2) {
	time_fail2 = t_prev +
	  (fail_thresh2 - f2_prev) / (f2 - f2_prev) * (t - t_prev);
	failed2 = true;
      }
      else
	f2_prev = f2;
    }
    // break out if done
    if (failed1 && failed2)
      break;
    t_prev = t;
  }

  fout << time_fail1 << "failtime1\n" << time_fail2 << "failtime2\n";
  fout.flush();
  fout.close();
  return 0;
}
