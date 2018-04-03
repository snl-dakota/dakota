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
#include <cmath>
#include <string.h>
#include "fbeamGV.h"
#include "gbeamGV.h"
#include "fbeamGV_types.h"
#include "fbeamGV_emxAPI.h"

int main(int argc, char** argv)
{

/* The Vanderplaats cantilever beam has nonlinear objective and constraints:
   Vanderplaats, Garret N. (1984) Example 5-1
   Numerical Optimization Techniques for Engineering Design: With Applications
   New York: McGraw-Hill, pp. 147-150.
  
   Written by: Robert A. Canfield 2/22/18  */

  std::ifstream fin(argv[1]);
  if (!fin) {
    std::cerr << "\nError: failure opening " << argv[1] << std::endl;
    exit(-1);
  }
  
  // Create name for a history file
  std::string his_file_name( argv[2] );
  int len_file_name = strlen( argv[2] );
  // Assume 3-character extension for the output file; replace with '.his'
  his_file_name.replace( len_file_name-3, 3, "his" );
  //std::cout << his_file_name << std::endl;
  
  size_t i, j, n, num_vars, num_fns;
  std::string vars_text, fns_text;

  // Get the design variables std::vector and ignore the labels
  fin >> num_vars >> vars_text;
  std::vector<double> x(num_vars);
  emxArray_real_T *X;
    
  static int iv0[1] = { num_vars };
  /* Set the size of the array.
     Change this size to the value that the application requires. */
  X = emxCreateND_real_T(1, *(int (*)[1])&iv0[0]);
  
  X->size[0] = num_vars;
  X->size[1] = 1;
  for (i=0; i<num_vars; i++) {
    fin >> x[i] >> vars_text;
    fin.ignore(256, '\n');
    X->data[i] = x[i];
  }
  num_fns = num_vars + 1;
  
  // Compute the results and output them directly to argv[2] (the NO_FILTER
  // option is used).
  
  double f, gmax;
  int       gidx;
//std::vector<double> df(num_vars), dg(num_vars);
  emxArray_real_T *g;
  emxArray_real_T *df, *dg;
  emxInitArray_real_T(&g,  1);
  emxInitArray_real_T(&df, 1);
  emxInitArray_real_T(&dg, 2);
  
  fbeamGV( X, &f,  g );
  gbeamGV( X, df, dg );

  std::ofstream fout(argv[2]); // do not instantiate until ready to write results
  if (!fout) {
    std::cerr << "\nError: failure creating " << argv[2] << std::endl;
    exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(std::ios::scientific);
  fout.setf(std::ios::right);
  
    std::ofstream fhis( his_file_name.c_str(), std::ios_base::app );

  // **** f:
     fout << f << '\n';

  // **** g:
     for (j = 0; j < num_fns; j++) { fout << g->data[j] << '\n'; }
//   gmax = std::max_element( g->data[0], g->data[0]+num_fns );
//   gidx = std::distance( g->data, gmax );

  // **** df/dx:
     fout << "[ ";
     for (i = 0; i < num_vars; i++) { fout << df->data[i] << " "; }
         fout << " ]\n"; 

  // **** dg/dx:
     n = 0;
     gmax = g->data[0];
     gidx = 1;
     for (j = 0; j < num_fns;  j++) {
         fout << "[ ";
         for (i = 0; i < num_vars; i++) { 
             fout << dg->data[n] << " "; n += 1; }
             fout << " ]\n";
         if ( g->data[j] > gmax ) { gmax = g->data[j]; gidx = j+1; }
     }

  // Write objective and max constraint to history file
	  fhis << "Objective = " << f
          << ", Max constraint index = " << gidx 
          <<                ", g_max = " << gmax << std::endl;

  fout.close();  
  fhis.close();  

  return 0;
}
