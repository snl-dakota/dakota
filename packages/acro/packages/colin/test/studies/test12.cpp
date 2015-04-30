/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

//
// Test the set_subsolver interface
//

#include <acro_config.h>
#include <colin/ColinUtilib.h>
#include "TestOptSolver.h"

using namespace colin;
using namespace std;


int main()
{
   Problem<vector<double> > prob;
   OptSetup(prob, "shell_func9", "[-10.0,10.0]^3", 0, 0, true);

   TestOptSolver opt;
   AnotherOptSolver sub;
   opt.set_solver(&opt);
   opt.set_solver(&sub);

   vector<double> point(4);
   point << 10.0;

   opt.set_initial_point(point);
   opt.set_problem(prob);
   opt["sufficient_objective_value"] = 39.97;
   opt["max_function_evaluations"] = 100;

   opt.reset();
   try
   {
      opt.minimize();
   }
   catch (...)
   { ucout << "HERE" << endl;}

   opt.write_parameters(cout);
   return 0;
}
