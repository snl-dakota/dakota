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

enum var_t { E1, E2, E3, E4, E5, E6, E7, E8, E9, E10 };


int main(int argc, char** argv)
{
  // This application program reads and writes parameter and response data 
  // directly so no input/output filters are needed.
  std::ifstream fin(argv[1]);
  if (!fin) {
    std::cerr << "\nError: failure opening " << argv[1] << std::endl;
    exit(-1);
  }
  size_t i, j, num_vars, num_fns, num_deriv_vars;
  std::string vars_text, fns_text, dvv_text;

  // define the std::string to enumeration map
  std::map<std::string, var_t> var_t_map;
  var_t_map["e1"]  = E1; var_t_map["e2"] = E2; var_t_map["e3"] = E3;
  var_t_map["e4"]  = E4; var_t_map["e5"] = E5; var_t_map["e6"] = E6;
  var_t_map["e7"]  = E7; var_t_map["e8"] = E8; var_t_map["e9"] = E9;
  var_t_map["e10"] = E10;

  // Get the parameter std::vector and ignore the labels
  fin >> num_vars >> vars_text;
  if (num_vars != 8 && num_vars != 10) {
    std::cerr << "Wrong number of variables for the trajectory problem\n";
    exit(-1);
  }
  std::map<var_t, double> vars;
  //std::vector<var_t> labels(num_vars);
  double value_i; std::string label_i; var_t v_i;
  std::map<std::string, var_t>::iterator v_iter;
  for (i=0; i<num_vars; i++) {
    fin >> value_i >> label_i;
    transform(label_i.begin(), label_i.end(), label_i.begin(),
	      (int(*)(int))tolower);
    v_iter = var_t_map.find(label_i);
    // if (v_iter == var_t_map.end()) {
    //   std::cerr << "Error: label \"" << label_i << "\" not supported in analysis "
    // 	   << "driver." << std::endl;
    //   exit(-1);
    // }
    // else
    //   v_i = v_iter->second;
    // vars[v_i] = value_i;
    // labels[i] = v_i;

    // ignore any failure thresholds; only need the epistemic variables
    if (v_iter != var_t_map.end())
      vars[v_iter->second] = value_i;
  }

  // Get the ASV std::vector and ignore the labels
  fin >> num_fns >> fns_text;
  // ignore num_fns
  // if (num_fns < 1 || num_fns > 2) { // 1 fn -> opt, 2 fns -> least sq
  //   std::cerr << "Wrong number of functions in trajectory.\n";
  //   exit(-1);
  // }
  std::vector<short> ASV(num_fns);
  for (i=0; i<num_fns; i++) {
    fin >> ASV[i];
    fin.ignore(256, '\n');
  }

  // Get the DVV std::vector and ignore the labels
  fin >> num_deriv_vars >> dvv_text;
  //std::vector<var_t> DVV(num_deriv_vars);
  unsigned int dvv_i;
  for (i=0; i<num_deriv_vars; i++) {
    fin >> dvv_i;
    fin.ignore(256, '\n');
    //DVV[i] = labels[dvv_i-1];
  }

  std::ofstream hist_out("time_history.dat");
  if (!hist_out) {
    std::cerr << "\nError: failure creating time history output file." << std::endl;
    exit(-1);
  }
  hist_out.precision(15); // 16 total digits
  hist_out.setf(std::ios::scientific);
  hist_out.setf(std::ios::right);

  // Compute and output time step data
  size_t num_delta = 1000;
  double t0 = 0., tf = 10., delta_t = (tf - t0) / num_delta, f1, f2;
  hist_out << num_delta << std::endl;

  // Compute and output time histories
  double t = t0;
  for (i=0; i<num_delta; ++i) {
    f1 = vars[E1] + t * (vars[E3] + t * (vars[E5] + t * vars[E7]));
    f2 = vars[E2] + t * (vars[E4] + t * (vars[E6] + t * vars[E8]));
    hist_out << t << '\t' << f1 << '\t' << f2 << '\n';
    t += delta_t;
  }

  hist_out.flush();
  hist_out.close();

  // create empty results file
  std::ofstream fout(argv[2]);
  if (!fout) {
    std::cerr << "\nError: failure creating " << argv[2] << std::endl;
    exit(-1);
  }
  fout.flush();
  fout.close();

  return 0;
}
