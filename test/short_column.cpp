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

enum var_t { B, H, P, M, Y };


int main(int argc, char** argv)
{

  std::ifstream fin(argv[1]);
  if (!fin) {
    std::cerr << "\nError: failure opening " << argv[1] << std::endl;
    exit(-1);
  }
  size_t i, j, num_vars, num_fns, num_deriv_vars;
  std::string vars_text, fns_text, dvv_text;

  // define the std::string to enumeration map
  std::map<std::string, var_t> var_t_map;
  var_t_map["b"] = B; var_t_map["h"] = H;
  var_t_map["p"] = P; var_t_map["m"] = M; var_t_map["y"] = Y;

  // Get the parameter std::vector and ignore the labels
  fin >> num_vars >> vars_text;
  std::map<var_t, double> vars;
  std::vector<var_t> labels(num_vars);
  double var_i; std::string label_i; var_t v_i;
  std::map<std::string, var_t>::iterator v_iter;
  for (i=0; i<num_vars; i++) {
    fin >> var_i >> label_i;
    transform(label_i.begin(), label_i.end(), label_i.begin(),
	      (int(*)(int))tolower);
    v_iter = var_t_map.find(label_i);
    if (v_iter == var_t_map.end()) {
      std::cerr << "Error: label \"" << label_i << "\" not supported in analysis "
	   << "driver." << std::endl;
      exit(-1);
    }
    else
      v_i = v_iter->second;
    vars[v_i] = var_i;
    labels[i] = v_i;
  }

  // Get the ASV std::vector and ignore the labels
  fin >> num_fns >> fns_text;
  std::vector<short> ASV(num_fns);
  for (i=0; i<num_fns; i++) {
    fin >> ASV[i];
    fin.ignore(256, '\n');
  }

  // Get the DVV std::vector and ignore the labels
  fin >> num_deriv_vars >> dvv_text;
  std::vector<var_t> DVV(num_deriv_vars);
  unsigned int dvv_i;
  for (i=0; i<num_deriv_vars; i++) {
    fin >> dvv_i;
    fin.ignore(256, '\n');
    DVV[i] = labels[dvv_i-1];
  }

  if (num_vars != 5 || num_fns != 2) {
    std::cerr << "Error: wrong number of inputs/outputs in short_column." << std::endl;
    exit(-1);
  }

  // Compute the results and output them directly to argv[2] (the NO_FILTER
  // option is used).  Response tags are optional; output them for ease
  // of results readability.
  std::ofstream fout(argv[2]);
  if (!fout) {
    std::cerr << "\nError: failure creating " << argv[2] << std::endl;
    exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(std::ios::scientific);
  fout.setf(std::ios::right);

  // b = vars[B] = column base   (design var.)
  // h = vars[H] = column height (design var.)
  // p = vars[P] (normal uncertain var.)
  // m = vars[M] (normal uncertain var.)
  // y = vars[Y] (lognormal uncertain var.)
  double b = vars[B], h = vars[H], p = vars[P], m = vars[M], y = vars[Y],
         b_sq = b*b, h_sq = h*h, p_sq = p*p, y_sq = y*y;

  // **** f (objective = bh = cross sectional area):
  if (ASV[0] & 1)
    fout << "                     " << b*h << " f\n";

  // **** g (limit state = short column response):
  if (ASV[1] & 1)
    fout << "                     "
	 << 1. - 4.*m/(b*h_sq*y) - p_sq/(b_sq*h_sq*y_sq) << " g\n";

  // **** df/dx (w.r.t. active/uncertain variables):
  if (ASV[0] & 2) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++)
      switch (DVV[i]) {
      case B: // design variable derivative
	fout << h << ' ';
	break;
      case H: // design variable derivative
	fout << b << ' ';
	break;
      default: // uncertain variable derivative
	fout << "0. ";
	break;
      }
    fout << "]\n";
  }

  // **** dg/dx (w.r.t. active/uncertain variables):
  if (ASV[1] & 2) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++)
      switch (DVV[i]) {
      case B: // design variable derivative
	fout << 4.*m/(b_sq*h_sq*y) + 2.*p_sq/(b_sq*b*h_sq*y_sq) << ' ';
	break;
      case H: // design variable derivative
	fout << 8.*m/(b*h_sq*h*y)  + 2.*p_sq/(b_sq*h_sq*h*y_sq) << ' ';
	break;
      case P: // uncertain variable derivative
	fout << -2.*p/(b_sq*h_sq*y_sq) << ' ';
	break;
      case M: // uncertain variable derivative
	fout << -4./(b*h_sq*y) << ' ';
	break;
      case Y: // uncertain variable derivative
	fout << 4.*m/(b*h_sq*y_sq) + 2.*p_sq/(b_sq*h_sq*y_sq*y) << ' ';
	break;
      }
    fout << "]\n";
  }

  // **** d^2f/dx^2: (SORM)
  if (ASV[0] & 4) {
    fout << "[[ ";
    for (i=0; i<num_deriv_vars; i++)
      for (j=0; j<num_deriv_vars; j++)
	if ( (DVV[i] == B && DVV[j] == H) || (DVV[i] == H && DVV[j] == B) )
	  fout << "1. ";
	else
	  fout << "0. ";
    fout << "]]\n";
  }

  // **** d^2g/dx^2: (SORM)
  if (ASV[1] & 4) {
    fout << "[[ ";
    for (i=0; i<num_deriv_vars; i++)
      for (j=0; j<num_deriv_vars; j++)
	if (DVV[i] == B && DVV[j] == B)          // d^2g/db^2
	  fout << -8.*m/(b_sq*b*h_sq*y) - 6.*p_sq/(b_sq*b_sq*h_sq*y_sq) << ' ';
	else if ( (DVV[i] == B && DVV[j] == H) ||
		  (DVV[i] == H && DVV[j] == B) ) // d^2g/dbdh
	  fout << -8.*m/(b_sq*h_sq*h*y) - 4.*p_sq/(b_sq*b*h_sq*h*y_sq) << ' ';
	else if (DVV[i] == H && DVV[j] == H)     // d^2g/dh^2
	  fout << -24.*m/(b*h_sq*h_sq*y) - 6.*p_sq/(b_sq*h_sq*h_sq*y_sq) << ' ';
	else if (DVV[i] == P && DVV[j] == P)     // d^2g/dp^2
	  fout << -2./(b_sq*h_sq*y_sq) << ' ';
	else if ( (DVV[i] == P && DVV[j] == M) ||
		  (DVV[i] == M && DVV[j] == P) ) // d^2g/dpdm
	  fout << "0. ";
	else if ( (DVV[i] == P && DVV[j] == Y) ||
		  (DVV[i] == Y && DVV[j] == P) ) // d^2g/dpdy
	  fout << 4.*p/(b_sq*h_sq*y_sq*y) << ' ';
	else if (DVV[i] == M && DVV[j] == M)     // d^2g/dm^2
	  fout << "0. ";
	else if ( (DVV[i] == M && DVV[j] == Y) ||
		  (DVV[i] == Y && DVV[j] == M) ) // d^2g/dmdy
	  fout << 4./(b*h_sq*y_sq) << ' ';
	else if (DVV[i] == Y && DVV[j] == Y)     // d^2g/dy^2
	  fout << -8.*m/(b*h_sq*y_sq*y) - 6.*p_sq/(b_sq*h_sq*y_sq*y_sq) << ' ';
	else { // unsupported cross-derivative
	  std::cerr << "Error: unsupported Hessian cross term in short_column."
	       << std::endl;
	  exit(-1);
	}
    fout << "]]\n";
  }

  fout.flush();
  fout.close();
  return 0;
}
