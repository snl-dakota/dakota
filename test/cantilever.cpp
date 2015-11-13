/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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

  // This test problem is an OUU example from Applied Research Associates
  // (42nd AIAA SDM conference, April 2001).

  std::ifstream fin(argv[1]);
  if (!fin) {
    std::cerr << "\nError: failure opening " << argv[1] << std::endl;
    exit(-1);
  }
  size_t i, j, num_vars, num_fns, num_deriv_vars;
  std::string vars_text, fns_text, dvv_text;

  // define the std::string to enumeration map
  std::map<std::string, var_t> var_t_map;
  var_t_map["w"] = W; var_t_map["t"] = T;
  var_t_map["r"] = R; var_t_map["e"] = E;
  var_t_map["x"] = X; var_t_map["y"] = Y;

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

  if (num_vars != 4 && num_vars != 6) {
    std::cerr << "Error: Wrong number of variables in cantilever test fn." << std::endl;
    exit(-1);
  }
  if (num_fns < 2 || num_fns > 3) {
    std::cerr << "Error: wrong number of response functions in cantilever test fn."
         << std::endl;
    exit(-1);
  }

  // Compute the cross-sectional area, stress, and displacement of the
  // cantilever beam.  This simulator is unusual in that it supports both
  // the case of design variable insertion and the case of design variable
  // augmentation.  It does not support mixed insertion/augmentation.  In
  // the 6 variable case, w,t,R,E,X,Y are all passed in; in the 4 variable
  // case, w,t assume local values.
  std::map<var_t, double>::iterator m_iter = vars.find(W);
  double w = (m_iter == vars.end()) ? 2.5 : m_iter->second; // beam width
  m_iter = vars.find(T);
  double t = (m_iter == vars.end()) ? 2.5 : m_iter->second; // beam thickness
  double r = vars[R]; // yield strength
  double e = vars[E]; // Young's modulus
  double x = vars[X]; // horizontal load
  double y = vars[Y]; // vertical load

  // allow f,c1,c2 (optimization) or just c1,c2 (calibration)
  bool objective; size_t c1i, c2i;
  if (num_fns == 2) { objective = false; c1i = 0; c2i = 1; }
  else              { objective = true;  c1i = 1; c2i = 2; }

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
  double D3 = D1/sqrt(D2)/D0,     D4 = D1*sqrt(D2)/D0;

  std::ofstream fout(argv[2]); // do not instantiate until ready to write results
  if (!fout) {
    std::cerr << "\nError: failure creating " << argv[2] << std::endl;
    exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(std::ios::scientific);
  fout.setf(std::ios::right);

  // **** f:
  if (objective && (ASV[0] & 1))
    fout << "                     " << area << '\n';

  // **** c1:
  if (ASV[c1i] & 1)
    fout << "                     " << stress/r - 1. << '\n';

  // **** c2:
  if (ASV[c2i] & 1)
    fout << "                     " << D4 - 1. << '\n';

  // **** df/dx:
  if (objective && (ASV[0] & 2)) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++)
      switch (DVV[i]) {
      case W:  fout << t << ' '; break; // design var derivative
      case T:  fout << w << ' '; break; // design var derivative
      default: fout << "0. ";    break; // uncertain var derivative
      }
    fout << "]\n";
  }

  // **** dc1/dx:
  if (ASV[c1i] & 2) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++)
      switch (DVV[i]) {
      case W: fout << -600.*(y/t + 2.*x/w)/w_sq/t/r << ' '; break; // des var
      case T: fout << -600.*(2.*y/t + x/w)/w/t_sq/r << ' '; break; // des var
      case R: fout << -stress/r_sq  << ' '; break; // uncertain var deriv
      case E: fout << "0. ";                break; // uncertain var deriv
      case X: fout << 600./w_sq/t/r << ' '; break; // uncertain var deriv
      case Y: fout << 600./w/t_sq/r << ' '; break; // uncertain var deriv
      }
    fout << "]\n";
  }

  // **** dc2/dx:
  if (ASV[c2i] & 2) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++)
      switch (DVV[i]) {
      case W: fout << -D3*2.*x_sq/w_sq/w_sq/w - D4/w << ' '; break; // des var
      case T: fout << -D3*2.*y_sq/t_sq/t_sq/t - D4/t << ' '; break; // des var
      case R: fout << "0. ";                 break; // unc var deriv
      case E: fout << -D4/e          << ' '; break; // unc var deriv
      case X: fout << D3*x/w_sq/w_sq << ' '; break; // unc var deriv
      case Y: fout << D3*y/t_sq/t_sq << ' '; break; // unc var deriv
      }
    fout << "]\n";
  }

  // **** d^2f/dx^2:
  if (objective && (ASV[0] & 4)) {
    fout << "[[ ";
    for (i=0; i<num_deriv_vars; i++)
      for (j=0; j<num_deriv_vars; j++)
	if ( (DVV[i] == W && DVV[j] == T) || (DVV[i] == T && DVV[j] == W) )
	  fout << "1. ";
	else
	  fout << "0. ";
    fout << "]]\n";
  }

  // **** d^2c1/dx^2:
  if (ASV[c1i] & 4) {
    fout << "[[ ";
    for (i=0; i<num_deriv_vars; i++)
      for (j=0; j<num_deriv_vars; j++)
	if (DVV[i] == W && DVV[j] == W)          // d^2g/dw^2
	  fout << 1200.*(y/t + 3.*x/w)/w_sq/area/r << ' ';
	else if (DVV[i] == T && DVV[j] == T)     // d^2g/dt^2
	  fout << 1200.*(3.*y/t + x/w)/t_sq/area/r << ' ';
	else if (DVV[i] == R && DVV[j] == R)     // d^2g/dr^2
	  fout << 2.*stress/pow(r, 3) << ' ';
	else if ( (DVV[i] == W && DVV[j] == T) ||
		  (DVV[i] == T && DVV[j] == W) ) // d^2g/dwdt
	  fout << 1200.*(y/t + x/w)/w_sq/t_sq/r << ' ';
	else if ( (DVV[i] == W && DVV[j] == R) ||
		  (DVV[i] == R && DVV[j] == W) ) // d^2g/dwdr
	  fout << 600.*(y/t + 2.*x/w)/w_sq/t/r_sq << ' ';
	else if ( (DVV[i] == W && DVV[j] == X) ||
		  (DVV[i] == X && DVV[j] == W) ) // d^2g/dwdx
	  fout << -1200./w_sq/w/t/r << ' ';
	else if ( (DVV[i] == W && DVV[j] == Y) ||
		  (DVV[i] == Y && DVV[j] == W) ) // d^2g/dwdy
	  fout << -600./w_sq/t_sq/r << ' ';
	else if ( (DVV[i] == T && DVV[j] == R) ||
		  (DVV[i] == R && DVV[j] == T) ) // d^2g/dtdr
	  fout << 600.*(2.*y/t + x/w)/w/t_sq/r_sq << ' ';
	else if ( (DVV[i] == T && DVV[j] == X) ||
		  (DVV[i] == X && DVV[j] == T) ) // d^2g/dtdx
	  fout << -600./w_sq/t_sq/r << ' ';
	else if ( (DVV[i] == T && DVV[j] == Y) ||
		  (DVV[i] == Y && DVV[j] == T) ) // d^2g/dtdy
	  fout << -1200./w/t_sq/t/r << ' ';
	else if ( (DVV[i] == R && DVV[j] == X) ||
		  (DVV[i] == X && DVV[j] == R) ) // d^2g/drdx
	  fout << -600./w_sq/t/r_sq << ' ';
	else if ( (DVV[i] == R && DVV[j] == Y) ||
		  (DVV[i] == Y && DVV[j] == R) ) // d^2g/drdy
	  fout << -600./w/t_sq/r_sq << ' ';
	else
	  fout << "0. ";
    fout << "]]\n";
  }

  // **** d^2c2/dx^2:
  if (ASV[c2i] & 4) {
    double D5 = 1./sqrt(D2)/D0, D6 = -D1/2./D0/pow(D2,1.5);
    double D7 = sqrt(D2)/D0,    D8 =  D1/2./D0/sqrt(D2);
    double dD2_dx = 2.*x/w_sq/w_sq, dD3_dx = D6*dD2_dx, dD4_dx = D8*dD2_dx;
    double dD2_dy = 2.*y/t_sq/t_sq, dD3_dy = D6*dD2_dy, dD4_dy = D8*dD2_dy;
    double dD1_dw = -D1/w, dD2_dw = -4.*x_sq/w_sq/w_sq/w,
      dD3_dw = D5*dD1_dw + D6*dD2_dw, dD4_dw = D7*dD1_dw + D8*dD2_dw;
    double dD1_dt = -D1/t, dD2_dt = -4.*y_sq/t_sq/t_sq/t,
      dD3_dt = D5*dD1_dt + D6*dD2_dt, dD4_dt = D7*dD1_dt + D8*dD2_dt;
    fout << "[[ ";
    for (i=0; i<num_deriv_vars; i++)
      for (j=0; j<num_deriv_vars; j++)
	if (DVV[i] == W && DVV[j] == W)            // d^2g/dw^2
	  fout << D3*10.*x_sq/pow(w_sq,3)
	    - 2.*x_sq/w_sq/w_sq/w*dD3_dw + D4/w_sq - dD4_dw/w << ' ';
	else if (DVV[i] == T && DVV[j] == T)       // d^2g/dt^2
	  fout << D3*10.*y_sq/pow(t_sq,3)
	    - 2.*y_sq/t_sq/t_sq/t*dD3_dt + D4/t_sq - dD4_dt/t << ' ';
	else if (DVV[i] == E && DVV[j] == E) {     // d^2g/de^2
	  double dD1_de = -D1/e, dD4_de = D7*dD1_de;
	  fout << D4/e/e - dD4_de/e << ' ';
	}
	else if (DVV[i] == X && DVV[j] == X)       // d^2g/dx^2
	  fout << D3/w_sq/w_sq + x/w_sq/w_sq*dD3_dx << ' ';
	else if (DVV[i] == Y && DVV[j] == Y)       // d^2g/dy^2
	  fout << D3/t_sq/t_sq + y/t_sq/t_sq*dD3_dy << ' ';
	else if ( (DVV[i] == W && DVV[j] == T) ||
		  (DVV[i] == T && DVV[j] == W) )   // d^2g/dwdt
	  fout << -2.*x_sq/w_sq/w_sq/w*dD3_dt - dD4_dt/w << ' ';
	else if ( (DVV[i] == W && DVV[j] == E) ||
		  (DVV[i] == E && DVV[j] == W) )   // d^2g/dwde
	  fout << -dD4_dw/e << ' ';
	else if ( (DVV[i] == W && DVV[j] == X) ||
		  (DVV[i] == X && DVV[j] == W) )   // d^2g/dwdx
	  fout << -4.*x*D3/w_sq/w_sq/w + x/w_sq/w_sq*dD3_dw << ' ';
	else if ( (DVV[i] == W && DVV[j] == Y) ||
		  (DVV[i] == Y && DVV[j] == W) )   // d^2g/dwdy
	  fout << y/t_sq/t_sq*dD3_dw << ' ';
	else if ( (DVV[i] == T && DVV[j] == E) ||
		  (DVV[i] == E && DVV[j] == T) )   // d^2g/dtde
	  fout << -dD4_dt/e << ' ';
	else if ( (DVV[i] == T && DVV[j] == X) ||
		  (DVV[i] == X && DVV[j] == T) )   // d^2g/dtdx
	  fout << x/w_sq/w_sq*dD3_dt << ' ';
	else if ( (DVV[i] == T && DVV[j] == Y) ||
		  (DVV[i] == Y && DVV[j] == T) )   // d^2g/dtdy
	  fout << -4.*y*D3/t_sq/t_sq/t + y/t_sq/t_sq*dD3_dt << ' ';
	else if ( (DVV[i] == E && DVV[j] == X) ||
		  (DVV[i] == X && DVV[j] == E) )   // d^2g/dedx
	  fout << -dD4_dx/e << ' ';
	else if ( (DVV[i] == E && DVV[j] == Y) ||
		  (DVV[i] == Y && DVV[j] == E) )   // d^2g/dedy
	  fout << -dD4_dy/e << ' ';
	else if ( (DVV[i] == X && DVV[j] == Y) ||
		  (DVV[i] == Y && DVV[j] == X) )   // d^2g/dxdy
	  fout << x/w_sq/w_sq*dD3_dy << ' ';
	else
	  fout << "0. ";
    fout << "]]\n";
  }

  fout.flush();
  fout.close();  
  return 0;
}
