/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description: A tester for DAKOTA's DLL API
//- Owner:       Bill Hart
//- Checked by:
//- Version: $Id$

/** \file dll_tester.cpp
    \brief Test the DLL with a DAKOTA input file */

#include <cstring>
#include <iostream>
#include "dll_api.h"

/// The main program for exercising the DLL API with a simple command-line

int main(int argc, char* argv[])
{
if (argc != 3) {
   std::cerr << "Usage: dll_tester <ntrials> <dakota.in>" << std::endl;
   return -1;
   }
int ntrials = atoi(argv[1]);

for (int i=0; i<ntrials; i++) {
    int id;
    dakota_create(&id, "dakota_dll");
    dakota_readInput(id, argv[2]);
    dakota_start(id);
    dakota_destroy(id);

    set_mc_ptr_int(1001);
    int tmp = get_mc_ptr_int();

    if (tmp != 1001)
        std::cout << "Set/Get of mc_ptr_int Failed!" << std::endl;

    std::cout << "Final Dakota Output:" << std::endl;
    std::cout << dakota_getStatus(id) << std::endl;
    }

return 0;
}

