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

#include <utilib/std_headers.h>
#include <utilib/_math.h>
#include <iostream>

using namespace std;
using namespace utilib;

namespace {

void run_one(double num) {
  cout << "default  : " << pscientific(num) << endl;
  cout << "4/2      : " << pscientific(num,4,2) << endl;
  cout << "8/4      : " << pscientific(num,8,4) << endl;
  cout << "15       : " << pscientific(num,15) << endl;
  cout << "6/2 flags: " << pscientific(num,6,2,&cout) << endl;
}

void run_test(double num) {
  ios::fmtflags f;
  
  f = cout.flags();		// save flags

  run_one(num);

  cout.setf(ios::uppercase);
  cout << "== flag uppercase" << endl;
  run_one(num);

  cout.flags(f);			// reset flags
  cout.setf(ios::showpos);
  cout << "== flag showpos" << endl;
  run_one(num);

  cout.flags(f);			// reset flags
  cout.setf(ios::showpos | ios::uppercase);
  cout << "== flag showpos|uc" << endl;
  run_one(num);

  cout.flags(f);			// reset original flags
  cout << "--------------------------------------" << endl;
}


void run_tests() {
  cout << "test 0" << endl;
  cout << "--------------------------------------" << endl;
  run_test(0.0);
  cout << "test 1" << endl;
  cout << "--------------------------------------" << endl;
  run_test(10.0);
  run_test(1.0);
  run_test(0.1);
  run_test(0.01);
  cout << "test 2" << endl;
  cout << "--------------------------------------" << endl;
  run_test(0.01234567890123456789);
  run_test(-0.01234567890123456789);
  cout << "test 3" << endl;
  cout << "--------------------------------------" << endl;
  run_test(1.234567890123456789e11);
  run_test(-1.234567890123456789e11);
  cout << "test 4" << endl;
  cout << "--------------------------------------" << endl;
  run_test(1.234567890123456789e-11);
  run_test(-1.234567890123456789e-11);
  cout << "test 5" << endl;
  cout << "--------------------------------------" << endl;
  run_test(1.234567890123456789e111);
  run_test(-1.234567890123456789e111);
  cout << "test 6" << endl;
  cout << "--------------------------------------" << endl;
  run_test(1.234567890123456789e-111);
  run_test(-1.234567890123456789e-111);
  cout << "test 7" << endl;
  cout << "--------------------------------------" << endl;
  run_test(0.199999999999);
}

}

int test_math(int,char**) {
  cout << "=========================================================================" << endl;
  cout << "Testing pscientific" << endl;
  cout << "=========================================================================" << endl;
  run_tests();

  return 0;
}
