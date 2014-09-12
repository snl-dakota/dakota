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
#include <boost/algorithm/string.hpp>
using namespace std;

enum var_t { FT1,  FT2 };

#define HIERARCH_TAG


int main(int argc, char** argv)
{
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
  if (num_vars != 2 && num_vars != 10) {
    cerr << "Error: wrong number of variables for trajectory_post().\n";
    exit(-1);
  }
  map<var_t, double> vars;
  //vector<var_t> labels(num_vars);
  double value_i; string label_i; //var_t v_i;
  map<string, var_t>::iterator v_iter;
  for (i=0; i<num_vars; i++) {
    fin >> value_i >> label_i;
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
    // vars[v_i] = value_i;
    // labels[i] = v_i;

    // ignore any epistemic variables; only need the failure thresholds
    if (v_iter != var_t_map.end())
      vars[v_iter->second] = value_i;
  }

  // Get the ASV vector and ignore the labels
  fin >> num_fns >> text;
  if (num_fns != 1) {
    cerr << "Error: wrong number of functions in trajectory_post.\n";
    exit(-1);
  }
  vector<short> ASV(num_fns);
  for (i=0; i<num_fns; i++) {
    fin >> ASV[i];
    fin.ignore(256, '\n');
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

#ifdef HIERARCH_TAG
  // Extract the hierarchical fn eval tags (required to link to opt interface)
  fin >> hier_tag; fin.ignore(256, '\n');
  vector<string> tags;
  boost::split(tags, hier_tag, boost::is_any_of(".: "));
  size_t num_tags = tags.size();
  if (num_tags < 2) {
    cerr << "Error: insufficient hierarchical tag depth." << endl;
    exit(-1);
  }
  string e_tag   = tags[0];                 // if file_tag is on
  //string e_tag = tags[0] + '.' + tags[0]; // if file_tag is off (hack)
  for (i=1; i<num_tags-1; ++i)
    e_tag += '.' + tags[i]; // up one level from last tag

  // Compute and output responses
  string history = "../epistemic_simulation." + e_tag + "/time_history.dat";
#else
  string history = "time_history.dat";
#endif

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
  double delta_t, t, t_prev, time_fail1, time_fail2, f1, f2, f1_prev, f2_prev,
    fail_thresh1 = vars[FT1], fail_thresh2 = vars[FT2];
  bool failed1 = false, failed2 = false, last_step;

  hist_in >> t_prev >> f1_prev >> f2_prev; // initial time step
  //cout << "Init: " << t_prev << ' ' << f1_prev << ' ' << f2_prev << std::endl;

  for (i=1; i<num_delta; ++i) {
    hist_in >> t >> f1 >> f2;
    //cout << "Step: " << t << ' ' << f1 << ' ' << f2 << std::endl;

    last_step = (i == num_delta-1);
    if (!failed1) {
      // Check if f1 has reached failure threshold 1;
      // if yes, then linearly interpolate for failure time.
      // Or if last time step, linearly extrapolate for failure time.
      if (f1 >= fail_thresh1 || last_step) {
	time_fail1 = t_prev +
	  (fail_thresh1 - f1_prev) / (f1 - f1_prev) * (t - t_prev);
	failed1 = true;
	//cout << "Fail 1: " <<  time_fail1 << std::endl;
      }
      else
	f1_prev = f1;
    }

    if (!failed2) {
      // Check if f2 has reached failure threshold 2;
      // if yes, then linearly interpolate for failure time.
      // Or if last time step, linearly extrapolate for failure time.
      if (f2 >= fail_thresh2 || last_step) {
	time_fail2 = t_prev +
	  (fail_thresh2 - f2_prev) / (f2 - f2_prev) * (t - t_prev);
	failed2 = true;
	//cout << "Fail 2: " <<  time_fail2 << std::endl;
      }
      else
	f2_prev = f2;
    }

    // break out if done
    if (failed1 && failed2)
      break;
    t_prev = t;
  }

  fout << time_fail2 - time_fail1 << " deltafailtime\n";
  fout.flush();
  fout.close();
  return 0;
}
