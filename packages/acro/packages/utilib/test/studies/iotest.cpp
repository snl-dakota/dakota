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
// CommonIO Test
//

#include <utilib_config.h>
#ifndef UTILIB_DISABLE_COMMONIO 

#ifdef WARNINGS
#undef WARNINGS
#endif
#ifdef UTILIB_VALIDATING
#undef UTILIB_VALIDATING
#endif

#include <utilib/std_headers.h>
#include <utilib/exception_mngr.h>
#include <utilib/CommonIO.h>
#include <utilib/mpiUtil.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif

using namespace utilib;

namespace {

class A : public CommonIO {
public:
A() : a(0) {}
void foo() {DEBUGPR(1,ucout << "A IS HERE" << endl);}
int a;
};

class B : public A {
public:
B() : b(1) {}
void foob() {DEBUGPR(2,ucout << "B IS HERE\n\n" << endl);}
int b;
};

void foo()
{
ucout << "Beginning CommonIO block." << endl;
CommonIO::begin();
A a;
B b;

DEBUGPRX(0,&(global_CommonIO()), "Debug value 0 works" << endl);

ucout << "TEST 1" << endl;
a.foo();
ucout << endl << Flush;

ucout << "TEST 2" << endl;
b.foob();
ucout << endl << Flush;

ucout << "TEST 3" << endl;
a.debug = 1;
a.foo();
ucout << endl << Flush;

ucout << "TEST 4" << endl;
b.debug = 1;
b.foo();
b.foob();
ucout << endl << Flush;

ucout << "TEST 5" << endl;
b.debug = 2;
b.foo();
b.foob();
ucout << endl << Flush;

ucout << "TEST FINAL" << endl;
CommonIO::end();
ucout << "Ending CommonIO block." << endl;
}

}


int test_iotest(int argc, char** argv)
{
utilib::exception_mngr::set_mode(utilib::exception_mngr::Abort);

try {
#ifdef UTILIB_HAVE_MPI
uMPI::init(&argc, &argv);
if (CommonIO::numProcs() > 1) {
   CommonIO::setIORank(-1);
   ucout << "Rank: " << CommonIO::rank() << " IOProc: " << uMPI::ioProc << endl;
   }
#else
// Avoid compiler warnings
argc=0;
argv=0;
#endif

CommonIO::begin();
ucout << "ucout_flag=" << (CommonIO::io_mapping || CommonIO::common_cout) <<  endl;
ucout << "----------------------------------------------------------" << endl;
ucout << "FOO -- default settings" << endl << Flush;
foo();
ucout << endl;
ucout << "----------------------------------------------------------" << endl;
CommonIO::end();


ucout << "----------------------------------------------------------" << endl;
ucout << "FOO -- output to a file iotest.temp.*" << endl << Flush;
char tmp[256];
#if _MSC_VER
sprintf_s(tmp,256,"iotest.temp.%d",CommonIO::rank());
#else
sprintf(tmp,"iotest.temp.%d",CommonIO::rank());
#endif
ofstream ofstr(tmp);
CommonIO::set_streams(&ofstr,&ofstr,&stdcin);
foo();
CommonIO::reset();
ofstr.close();
ucout << endl;
ucout << "----------------------------------------------------------" << endl;


ucout << "----------------------------------------------------------" << endl;
ucout << "FOO -- tagged output with numDigits=5" << endl << Flush;
CommonIO::begin_tagging(5);
foo();
CommonIO::begin_tagging(0);
ucout << endl;
ucout << "----------------------------------------------------------" << endl;


ucout << "----------------------------------------------------------" << endl;
ucout << "FOO -- default settings" << endl << Flush;
foo();
ucout << endl;
ucout << "----------------------------------------------------------" << endl;


ucout << "----------------------------------------------------------" << endl;
ucout << "FOO -- buffered output" << endl << Flush;
CommonIO::begin_buffered();
foo();
CommonIO::end();
ucout << endl;
ucout << "----------------------------------------------------------" << endl;


ucout << "----------------------------------------------------------" << endl;
ucout << "FOO -- default settings" << endl << Flush;
foo();
ucout << endl;
ucout << "----------------------------------------------------------" << endl;


ucout << "----------------------------------------------------------" << endl;
ucout << "FOO -- buffered output to a file with tagging" << endl << Flush;
CommonIO::begin_buffered();
CommonIO::begin_tagging(5);
#ifdef _MSC_VER
sprintf_s(tmp,256,"iotest.temp2.%d",CommonIO::rank());
#else
sprintf(tmp,"iotest.temp2.%d",CommonIO::rank());
#endif
ofstr.open(tmp);
CommonIO::set_streams(&ofstr,&ofstr,&stdcin);
foo();
CommonIO::end_tagging();
CommonIO::end();
CommonIO::reset();
ofstr.close();
ucout << endl;
ucout << "----------------------------------------------------------" << endl;


ucout << "Closing MPI" << endl << Flush;
#ifdef UTILIB_HAVE_MPI
uMPI::done();
#endif
ucout << "Ending IO" << endl << Flush;
 }
catch (const std::exception& err) {
   cout << "ERROR: " << err.what() << endl;
}
   
utilib::exception_mngr::set_mode(utilib::exception_mngr::Standard);
return 0;
}

#else

#include <iostream>

int test_iotest(int,char**)
{
std::cout << "No testing done since CommonIO is not configured!\n";
return 1;
}
#endif
