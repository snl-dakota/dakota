/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description: A tester for DAKOTA's DLL API
//- Owner:       Brian Adams
//- Checked by:
//- Version: $Id$

/** \file dll_tester.cpp
    \brief Test the DLL with a DAKOTA input file */

#include <cstdlib>
#include <cstring>
#include <iostream>
#include "dakota_dll_api.h"

/// The main program for exercising the DLL API with a simple command-line

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: dll_tester <ntrials> <dakota.in>" << std::endl;
    return -1;
  }
  int ntrials = atoi(argv[1]);

  // TODO: an improved test would allocate all the runners, then
  // configure, then run them asynchronously, but concurrent Dakota
  // instances in core are likely to be problematic.
  for (int i=0; i<ntrials; i++) {
    int id;
    dakota_create(&id, "dakota_dll_tester");
    int retcode = dakota_readInput(id, argv[2]);
    if (retcode != 0) {
      dakota_destroy(id);
      return retcode;
    }
    dakota_start(id);
    dakota_destroy(id);

    set_mc_ptr_int(1001);
    int tmp = get_mc_ptr_int();

    // return code will be different if modelcenter not enabled
    if (tmp != 1001)
      std::cout << "Warning: set/get of mc_ptr_int differ; modelcenter not "
		<< "working properly." << std::endl;

    // must be set to zero for subsequent runs or memory corruption will occur
    set_mc_ptr_int(0);

    std::cout << "Final Dakota Output:" << std::endl;
    std::cout << dakota_getStatus(id) << std::endl;
  }

  return 0;
}

