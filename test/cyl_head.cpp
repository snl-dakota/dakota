/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>


int main(int argc, char** argv)
{

  // This example problem arose as a demonstration for the Technologies 
  // Enabling Agile Manufacturing (TEAM) multi-lab project.

  std::ifstream fin(argv[1]);
  if (!fin) {
    std::cerr << "\nError: failure opening " << argv[1] << std::endl;
    exit(-1);
  }
  size_t i, num_vars, num_fns;
  std::string vars_text, fns_text;

  // Get the parameter std::vector and ignore the labels
  fin >> num_vars >> vars_text;
  std::vector<double> x(num_vars);
  for (i=0; i<num_vars; i++) {
    fin >> x[i];
    fin.ignore(256, '\n');
  }

  // Get the ASV std::vector and ignore the labels
  fin >> num_fns >> fns_text;
  std::vector<int> ASV(num_fns);
  for (i=0; i<num_fns; i++) {
    fin >> ASV[i];
    fin.ignore(256, '\n');
  }

  if (num_vars != 2 || num_fns != 4) {
    std::cerr << "Error: wrong number of inputs/outputs in cyl_head test function."
         << std::endl;
    exit(-1);
  }

  // Compute the results and output them directly to argv[2] (the NO_FILTER
  // option is used).  Response tags are now optional; output them for ease
  // of results readability.
  double exhaust_offset = 1.34;
  double exhaust_dia = 1.556;
  double intake_offset = 3.25;

  // Use nondimensional x[1]: 
  // (0. <= nondimensional <= 4.), (0. in <= dimensional <= 0.004 in)
  double warranty = 100000. + 15000. * (4. - x[1]);
  double cycle_time = 45. + 4.5*pow(4. - x[1], 1.5);
  double wall_thickness = intake_offset-exhaust_offset-(x[0]+exhaust_dia)/2.;
  double horse_power = 250.+200.*(x[0]/1.833-1.);
  double max_stress = 750. + pow(fabs(wall_thickness),-2.5);

  std::cout << "In cyl_head evaluator:\nwarranty = " << warranty 
       << "\ncycle_time = " << cycle_time
       << "\nwall_thickness = " << wall_thickness
       << "\nhorse_power = " << horse_power
       << "\nmax_stress = " << max_stress << std::endl;

  std::ofstream fout(argv[2]); // do not instantiate until ready to write results
  if (!fout) {
    std::cerr << "\nError: failure creating " << argv[2] << std::endl;
    exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(std::ios::scientific);
  fout.setf(std::ios::right);

  // **** f:
  if (ASV[0] & 1)
    fout << "                     " << -1.*(horse_power/250.+warranty/100000.)
         << '\n';

  // **** c1:
  if (ASV[1] & 1)
    fout << "                     " << max_stress/1500.-1. << '\n';

  // **** c2:
  if (ASV[2] & 1)
    fout << "                     " << 1.-warranty/100000. << '\n';

  // **** c3:
  if (ASV[3] & 1)
    fout << "                     " << cycle_time/60. - 1. << '\n';

  // **** c4: (Unnecessary if intake_dia upper bound reduced to 2.164)
  //if (ASV[4] & 1)
  //  fout << "                     " << 1.-20.*wall_thickness << '\n';

  // **** df/dx:
  if (ASV[0]& 2) {
      fout << "[ " << -.8/1.833 << " " << 0.15 << " ]\n"; 
  }

  // **** dc1/dx:
  if (ASV[1] & 2) {
    fout << "[ " << 1.25/1500*pow(wall_thickness, -3.5) << " " << 0. << " ]\n";
  }

  // **** dc2/dx:
  if (ASV[2] & 2) {
    fout << "[ " << 0. << " " << 0.15 << " ]\n";
  }

  // **** dc3/dx:
  if (ASV[3] & 2) {
    fout << "[ " << 0. << " " << -0.1125*sqrt(4. - x[1]) << " ]\n";
  }

  // **** dc4/dx:
  //if (ASV[4] & 2) {
  //  fout << "[ " << 10. << " " << 0. << " ]\n";
  //}

  fout.flush();
  fout.close();  
  return 0;
}
