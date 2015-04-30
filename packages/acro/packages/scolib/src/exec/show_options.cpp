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
// show_options.cpp
//

#include <acro_config.h>
#include <utilib/sort.h>
#include <scolib/Factory.h>

using namespace std;
using namespace utilib;

int main(int argc, char* argv[])
{
#if COLINDONE
if (argc < 2) {
   cerr << "show_options [-defaults -html] <solver>" << endl;
   scolib::print_factory_solvers(cerr);
   return -1;
   }

bool defaults=false;
bool html=false;
int ctr=1;
try {
  if (strcmp(argv[ctr],"-defaults")==0) {
     defaults=true;
     ctr++;
     }
  if (strcmp(argv[ctr],"-html")==0) {
     html=true;
     ctr++;
     }

  colin::OptSolverBase* opt = scolib::factory_create_base(argv[ctr]);
  if (!opt) {
     cerr << "Invalid solver name: " << argv[ctr] << endl;
     exit(1);
     }
  if (defaults)
    opt->write_parameter_values(cout);
  else if (html)
    opt->write_parameters_html(cout);
  else
    opt->write_parameters(cout,true);
  }
catch (std::exception& err) {
  cerr << "ERROR: " << err.what() << endl;
  }
#endif
return 0;
}
