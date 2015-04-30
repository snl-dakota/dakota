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
// rngtest.cpp
//

#include <utilib/std_headers.h>
#include <utilib/pvector.h>
#include <utilib/AnyRNG.h>
#include <utilib/Binomial.h>
#include <utilib/Cauchy.h>
#include <utilib/DUniform.h>
#include <utilib/Exponential.h>
#include <utilib/Geometric.h>
#include <utilib/MNormal.h>
#include <utilib/MUniform.h>
#include <utilib/Normal.h>
#include <utilib/Triangular.h>
#include <utilib/Uniform.h>
#include <utilib/stl_auxiliary.h>
#include <utilib/CommonIO.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
using namespace utilib;
#endif

namespace {

void foo()
{
AnyRNG rng;

{ Binomial rv(&rng), foo; cerr << rv() << endl; }
#ifdef CAUCHY_VTABLE
{ Cauchy rv(&rng), foo; cerr << rv() << endl; }
#endif
{ DUniform<int> rv(&rng), foo; cerr << rv() << endl; }
{ Exponential rv(&rng), foo; cerr << rv() << endl; }
{ Geometric rv(&rng), foo; cerr << rv() << endl; }
{ MNormal rv(&rng), foo; cerr << rv() << endl; }
{ Normal rv(&rng), foo; cerr << rv() << endl; }
{ Triangular rv(&rng), foo; cerr << rv() << endl; }
{ Uniform rv(&rng), foo; cerr << rv() << endl; }

}


void test1(RNG& rng)
{
Binomial rv(&rng,0.5,9), foo;
pvector<int> val(10);
val << 0;

for (unsigned int i=0; i<100; i++) {
  int tmp = rv();
  val[tmp]++;
  }
ucout << "Binomial" << endl << val << endl;
}

#ifdef CAUCHY_VTABLE
void test2(RNG& rng)
{
Cauchy rv(&rng), foo;
pvector<double> val(11);
val << 0.0;

for (unsigned int i=0; i<100; i++) {
  double tmp = rv();
  int ndx = (int)floor(tmp+5.5);
  ndx = (ndx < 0 ? 0 : ndx > 10 ? 10 : ndx);
  val[ndx]++;
  }
ucout << "Cauchy" << endl << val << endl;
}
#endif


void test3(RNG& rng)
{
DUniform<int> rv(&rng,0,9), foo;
pvector<int> val(10);
val << 0;

for (unsigned int i=0; i<100; i++) {
  int tmp = rv();
  val[tmp]++;
  }
ucout << "DUniform" << endl << val << endl;
}



void test4(RNG& rng)
{
Exponential rv(&rng), foo;
pvector<double> val(11);
val << 0.0;

for (unsigned int i=0; i<100; i++) {
  double tmp = rv();
  int ndx = (int)floor(tmp);
  ndx = (ndx < 0 ? 0 : ndx > 10 ? 10 : ndx);
  val[ndx]++;
  }
ucout << "Exponential" << endl << val << endl;
}


void test5(RNG& rng)
{
Geometric rv(&rng,0.5), foo;
pvector<int> val(11);
val << 0;

for (unsigned int i=0; i<100; i++) {
  int tmp = rv();
  int ndx = (tmp < 0 ? 0 : tmp > 10 ? 10 : tmp);
  val[ndx]++;
  }
ucout << "Geometric" << endl << val << endl;
}



void test6(RNG& rng)
{
MNormal rv(&rng,2), foo;
pvector<pvector<int> > val;

val.resize(11);
for (unsigned int i=0; i<val.size(); i++) {
  val[i].resize(11);
  val[i] << 0;
  }

for (unsigned int i=0; i<100; i++) {
  DoubleVector& vec = rv();
  int ndx1 = (int)floor(vec[0]+5.5);
  int ndx2 = (int)floor(vec[1]+5.5);
  ndx1 = (ndx1 < 0 ? 0 : ndx1 > 10 ? 10 : ndx1);
  ndx2 = (ndx2 < 0 ? 0 : ndx2 > 10 ? 10 : ndx2);
  val[ndx1][ndx2]++;
  }
ucout << "MNormal" << endl;
for (unsigned int i=0; i<val.size(); i++) {
  for (unsigned int j=0; j<val.size(); j++) {
    ucout << val[i][j] << " ";
    }
  ucout << endl;
  }
}


void test7(RNG& rng)
{
Normal rv(&rng), foo;
pvector<double> val(11);
val << 0.0;

for (unsigned int i=0; i<100; i++) {
  double tmp = rv();
  int ndx = (int)floor(tmp+5.5);
  ndx = (ndx < 0 ? 0 : ndx > 10 ? 10 : ndx);
  val[ndx]++;
  }
ucout << "Normal" << endl << val << endl;
}


void test8(RNG& rng)
{
Triangular rv(&rng), foo;
pvector<double> val(11);
val << 0.0;

for (unsigned int i=0; i<100; i++) {
  double tmp = rv()*10;
  int ndx = (int)floor(tmp);
  ndx = (ndx < 0 ? 0 : ndx > 10 ? 10 : ndx);
  val[ndx]++;
  }
ucout << "Triangular" << endl << val << endl;
}


void test9(RNG& rng)
{
Uniform rv(&rng), foo;
pvector<double> val(11);
val << 0.0;

for (unsigned int i=0; i<100; i++) {
  double tmp = rv()*10;
  int ndx = (int)floor(tmp);
  ndx = (ndx < 0 ? 0 : ndx > 10 ? 10 : ndx);
  val[ndx]++;
  }
ucout << "Uniform" << endl << val << endl;
}

}


#include <utilib/PM_LCG.h>

int test_rng(int,char**)
{
utilib::exception_mngr::set_mode(utilib::exception_mngr::Abort);

try {

PM_LCG foo;

ucout << "Default RNG Value" << endl;
for (int i=0; i<10; i++)
  ucout << foo.asLong() << endl;

foo.reset();

ucout << endl << "Reseting RNG Value" << endl;
for (int i=0; i<10; i++)
  ucout << foo.asLong() << endl;

ucout << endl << "Reseting RNG Value" << endl;
foo.set_seed(0);
for (int i=0; i<10; i++)
  ucout << foo.asLong() << endl;

foo.reset(); test1(foo);
#ifdef CAUCHY_VTABLE
foo.reset(); test2(foo);
#endif
foo.reset(); test3(foo);
foo.reset(); test4(foo);
foo.reset(); test5(foo);
foo.reset(); test6(foo);
foo.reset(); test7(foo);
foo.reset(); test8(foo);
foo.reset(); test9(foo);

AnyRNG rng(&foo);

if (rng.operator==(&foo) != true)
   EXCEPTION_MNGR(runtime_error, "AnyRNG operator== not working properly.");
if (rng.operator!=(&foo) != false)
   EXCEPTION_MNGR(runtime_error, "AnyRNG operator!= not working properly.");
   
}
catch (std::exception& str) {
  ucout << "Caught Exception!\n\t" << str.what() << endl;
  }
  
utilib::exception_mngr::set_mode(utilib::exception_mngr::Standard);
return 0;
}
