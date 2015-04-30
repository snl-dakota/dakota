/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

//
// Test the exception manager.
//

#include <utilib/std_headers.h>
#include <utilib/exception_mngr.h>

namespace {

int option=0;

void foo()
{
EXCEPTION_MNGR(std::runtime_error, "This is a test. option=" << option);
}

}


int test_exception_test(int argc, char** argv)
{
if (argc == 1) {
	std::cout << "exception <num>\n";
	std::cout << "    Options:  1 - Throw exception,  2 - Abort,  3 - Exit\n";
   return 0;
   }
#ifdef _MSC_VER
sscanf_s(argv[1],"%d",&option);
#else
std::sscanf(argv[1],"%d",&option);
#endif

switch (option) {
  case 1:
	utilib::exception_mngr::set_mode(utilib::exception_mngr::Standard);
	break;
  case 2:
	utilib::exception_mngr::set_mode(utilib::exception_mngr::Abort);
	break;
  case 3:
	utilib::exception_mngr::set_mode(utilib::exception_mngr::Exit);
	break;
  };

try {
  foo();
} catch (std::runtime_error& foo) {
	std::cout << "Caught Exception: " << foo.what() << std::endl;
	}

utilib::exception_mngr::set_mode(utilib::exception_mngr::Standard);
return 0;
}
