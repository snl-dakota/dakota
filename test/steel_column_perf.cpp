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

enum var_t { FS, P1, P2, P3, B, D, H, F0, E }; // order in Kuschel & Rackwitz


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
  var_t_map["fs"] = FS; var_t_map["p1"] = P1; var_t_map["p2"] = P2;
  var_t_map["p3"] = P3; var_t_map["b"]  = B;  var_t_map["d"]  = D;
  var_t_map["h"]  = H;  var_t_map["f0"] = F0; var_t_map["e"]  = E;

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

  if (num_vars != 9 || num_fns != 1) {
    std::cerr << "Error: wrong number of inputs/outputs in steel_column_perf."<<std::endl;
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

  // In the steel column description in Kuschel & Rackwitz, Cost is _not_
  // defined as a random variable.  That is Cost is not a fn(B, D, H), but
  // is rather defined as a fn(b, d, h).  Since dCost/dX|_{X=mean} is not the
  // same as dCost/dmean for non-normal X (jacobian_dX_dS is not 1), dCost/dX
  // may not be used and an optional interface must be defined for Cost.

  // set effective length s based on assumed boundary conditions
  // actual length of the column is 7500 mm
  double s = 7500;

  // fs = yield stress     (lognormal unc. var.)
  // p1 = dead weight load (normal    unc. var.)
  // p2 = variable load    (gumbel    unc. var.)
  // p3 = variable load    (gumbel    unc. var.)
  // b  = flange breadth   (lognormal unc. var., mean is design var.)
  // d  = flange thickness (lognormal unc. var., mean is design var.)
  // h  = profile height   (lognormal unc. var., mean is design var.)
  // f0 = init. deflection (normal    unc. var.)
  // e  = elastic modulus  (weibull   unc. var.)

  double f0 = vars[F0], b = vars[B], d = vars[D], h = vars[H], fs = vars[FS],
    e = vars[E], p = vars[P1]+vars[P2]+vars[P3], Pi = 3.1415926535897932385,
    Pi2 = Pi*Pi, Pi4 = Pi2*Pi2, Pi6 = Pi2*Pi4, b2 = b*b, d2 = d*d, h2 = h*h,
    h3 = h*h2, h5 = h2*h3, e2 = e*e, e3 = e*e2, s2 = s*s,
    X = Pi2*e*b*d*h2 - 2.*s2*p, X2 = X*X, X3 = X*X2;

  // **** g (limit state):
  if (ASV[0] & 1)
    fout << "                     "
	 << fs - p*(1./2./b/d + Pi2*f0*e*h/X) << " g\n";

  // **** dg/dx (w.r.t. active/uncertain variables):
  if (ASV[0] & 2) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++)
      switch (DVV[i]) {
      case F0: // df/df0
	fout << -e*h*p*Pi2/X << ' ';
	break;
      case P1: case P2: case P3: // df/dp1, df/dp2, df/dp3
	fout << -1./2./b/d - b*d*e2*f0*h3*Pi4/X2 << ' ';
	break;
      case B: // df/db
	fout << p*(1./2./b2/d + d*e2*f0*h3*Pi4/X2) << ' ';
	break;
      case D: // df/dd
	fout << p*(1./2./b/d2 + b*e2*f0*h3*Pi4/X2) << ' ';
	break;
      case H: // df/dh
	fout << e*f0*p*Pi2*(X + 4.*p*s2)/X2 << ' ';
	break;
      case FS: // df/dfs
	fout << "1. ";
	break;
      case E: // df/de
	fout << 2.*f0*h*p*p*Pi2*s2/X2 << ' ';
	break;
      }
    fout << "]\n";
  }

  // **** d^2g/dx^2: (SORM)
  if (ASV[0] & 4) {
    fout << "[[ ";
    for (i=0; i<num_deriv_vars; i++)
      for (j=0; j<num_deriv_vars; j++)
	if (DVV[i] == FS || DVV[j] == FS)          // d^2g/dfs^2
	  fout << "0. ";
	else if ( (DVV[i] == P1 && DVV[j] == P1) ||
                  (DVV[i] == P1 && DVV[j] == P2) ||
                  (DVV[i] == P1 && DVV[j] == P3) ||
		  (DVV[i] == P2 && DVV[j] == P1) ||
		  (DVV[i] == P2 && DVV[j] == P2) ||
		  (DVV[i] == P2 && DVV[j] == P3) ||
		  (DVV[i] == P3 && DVV[j] == P1) ||
		  (DVV[i] == P3 && DVV[j] == P2) ||
		  (DVV[i] == P3 && DVV[j] == P3) ) // d^2g/dpdp
	  fout << -4.*b*d*e2*f0*h3*Pi4*s2/X3 << ' ';
	else if ( (DVV[i] == P1 && DVV[j] == B) ||
 		  (DVV[i] == P2 && DVV[j] == B) ||
		  (DVV[i] == P3 && DVV[j] == B) ||
		  (DVV[i] == B  && DVV[j] == P1) ||
		  (DVV[i] == B  && DVV[j] == P2) ||
		  (DVV[i] == B  && DVV[j] == P3) ) // d^2g/dpdb
	  fout << 1./2./b2/d + d*e2*f0*h3*Pi4/X2*(2.*b*d*e*h2*Pi2/X - 1.) <<' ';
	else if ( (DVV[i] == P1 && DVV[j] == D) ||
 		  (DVV[i] == P2 && DVV[j] == D) ||
		  (DVV[i] == P3 && DVV[j] == D) ||
		  (DVV[i] == D  && DVV[j] == P1) ||
		  (DVV[i] == D  && DVV[j] == P2) ||
		  (DVV[i] == D  && DVV[j] == P3) ) // d^2g/dpdd
	  fout << 1./2./b/d2 + b*e2*f0*h3*Pi4/X2*(2.*b*d*e*h2*Pi2/X - 1.) <<' ';
	else if ( (DVV[i] == P1 && DVV[j] == H) ||
 		  (DVV[i] == P2 && DVV[j] == H) ||
		  (DVV[i] == P3 && DVV[j] == H) ||
		  (DVV[i] == H  && DVV[j] == P1) ||
		  (DVV[i] == H  && DVV[j] == P2) ||
		  (DVV[i] == H  && DVV[j] == P3) ) // d^2g/dpdh
	  fout << b*d*e2*f0*h2*Pi4*(X+8.*p*s2)/X3 << ' ';
	else if ( (DVV[i] == P1 && DVV[j] == F0) ||
 		  (DVV[i] == P2 && DVV[j] == F0) ||
		  (DVV[i] == P3 && DVV[j] == F0) ||
		  (DVV[i] == F0 && DVV[j] == P1) ||
		  (DVV[i] == F0 && DVV[j] == P2) ||
		  (DVV[i] == F0 && DVV[j] == P3) ) // d^2g/dpdf0
	  fout << -b*d*e2*h3*Pi4/X2 << ' ';
	else if ( (DVV[i] == P1 && DVV[j] == E) ||
 		  (DVV[i] == P2 && DVV[j] == E) ||
		  (DVV[i] == P3 && DVV[j] == E) ||
		  (DVV[i] == E  && DVV[j] == P1) ||
		  (DVV[i] == E  && DVV[j] == P2) ||
		  (DVV[i] == E  && DVV[j] == P3) ) // d^2g/dpde
	  fout << 4.*b*d*e*f0*h3*p*Pi4*s2/X3 << ' ';
	else if (DVV[i] == B && DVV[j] == B)     // d^2g/db^2
	  fout << -p*(1./b2/b/d + 2.*d2*e3*f0*h5*Pi6/X3) << ' ';
	else if ( (DVV[i] == B && DVV[j] == D) ||
		  (DVV[i] == D && DVV[j] == B) ) // d^2g/dbdd
	  fout << -p*(1./2./b2/d2 + e2*f0*h3*Pi4/X2*(2.*b*d*e*h2*Pi2/X - 1.))
               << ' ';
	else if ( (DVV[i] == B && DVV[j] == H) ||
		  (DVV[i] == H && DVV[j] == B) ) // d^2g/dbdh
	  fout << -d*e2*f0*h2*p*Pi4*(X + 8.*p*s2)/X3 << ' ';
	else if ( (DVV[i] == F0 && DVV[j] == B) ||
		  (DVV[i] == B  && DVV[j] == F0) ) // d^2g/dbdf0
	  fout << d*e2*h3*p*Pi4/X2 << ' ';
	else if ( (DVV[i] == B && DVV[j] == E) ||
		  (DVV[i] == E && DVV[j] == B) ) // d^2g/dbde
	  fout << -4.*d*e*f0*h3*p*p*Pi4*s2/X3 << ' ';
	else if (DVV[i] == D && DVV[j] == D)     // d^2g/dd^2
	  fout << -p*(1./b/d2/d + 2.*b2*e3*f0*h5*Pi6/X3) << ' ';
	else if ( (DVV[i] == D && DVV[j] == H) ||
		  (DVV[i] == H && DVV[j] == D) ) // d^2g/dddh
	  fout << -b*e2*f0*h2*p*Pi4*(X + 8.*p*s2)/X3 << ' ';
	else if ( (DVV[i] == F0 && DVV[j] == D) ||
		  (DVV[i] == D  && DVV[j] == F0) ) // d^2g/dddf0
	  fout << b*e2*h3*p*Pi4/X2 << ' ';
	else if ( (DVV[i] == D && DVV[j] == E) ||
		  (DVV[i] == E && DVV[j] == D) ) // d^2g/ddde
	  fout << -4.*b*e*f0*h3*p*p*Pi4*s2/X3 << ' ';
	else if (DVV[i] == H && DVV[j] == H)     // d^2g/dh^2
	  fout << -2.*b*d*e2*f0*h*p*Pi4*(X + 8.*p*s2)/X3 << ' ';
	else if ( (DVV[i] == F0 && DVV[j] == H) ||
		  (DVV[i] == H  && DVV[j] == F0) ) // d^2g/dhdf0
	  fout << e*p*Pi2*(X + 4.*p*s2)/X2 << ' ';
	else if ( (DVV[i] == H && DVV[j] == E) ||
		  (DVV[i] == E && DVV[j] == H) ) // d^2g/dhde
	  fout << -2.*f0*p*p*Pi2*s2*(3.*X + 8.*p*s2)/X3 << ' ';
	else if (DVV[i] == F0 && DVV[j] == F0)     // d^2g/df0^2
	  fout << "0. ";
	else if ( (DVV[i] == F0 && DVV[j] == E) ||
		  (DVV[i] == E  && DVV[j] == F0) ) // d^2g/df0de
	  fout << 2.*h*p*p*Pi2*s2/X2 << ' ';
	else if (DVV[i] == E && DVV[j] == E)     // d^2g/de^2
	  fout << -4.*b*d*f0*h3*p*p*Pi4*s2/X3 << ' ';
	else { // unsupported derivative
	  std::cerr << "Error: unsupported Hessian cross term in steel_column."
	       << std::endl;
	  exit(-1);
	}
    fout << "]]\n";
  }

  fout.flush();
  fout.close();
  return 0;
}
