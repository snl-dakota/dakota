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
// qlist.cpp
//

#include <utilib/std_headers.h>
#include <utilib/QueueList.h>
#include <utilib/AnyRNG.h>
#include <utilib/Uniform.h>
#include <utilib/default_rng.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
using namespace utilib;
#endif

int x0 = 1;
int xn;

double rng()
{
xn = (263*xn + 71) % 100;
return (xn/90.0);
}


bool tester_qlist(QueueList<int>& qlist)
{
QueueList<int>::iterator curr=qlist.begin();
QueueList<int>::iterator end=qlist.end();
if (curr==end) return true;
int prev = *curr;
curr++;
while (curr != end) {
  if (prev >= *curr) return false;
  curr++;
  }
return true;
}

int test_qlist(int argc, char** argv)
{
utilib::exception_mngr::set_mode(utilib::exception_mngr::Abort);

#if defined(UTILIB_HAVE_MPI)
uMPI::init(&argc,&argv,MPI_COMM_WORLD);
#else
argc=0;
argv=0;
#endif

//PM_LCG foo;
//foo.reseed(1);
//AnyRNG rng(&foo);
//Uniform urnd(&rng);

QueueList<int> qlist;

for (int i=1; i<=100; i++) {
  double tmp = rng();
  ucout << tmp << endl;
  if (tmp > 0.5) 
     qlist << i;
  else {
     int tmp;
     if (qlist.size() > 0)
        qlist >> tmp;
     }
  if (!tester_qlist(qlist)) {
     ucout << "ERROR: nonmonotonic QueueList: " << qlist << endl;
     return 1;
     }
  ucout << qlist << endl;
  //
  // Build and print a copy of qlist
  QueueList<int> foo(qlist);
  if (qlist != foo) {
     ucout << "ERROR: copy constructor generated a non-equivalent copy" << endl;
     ucout << "ERROR: " << qlist << endl;
     ucout << "ERROR: " << foo << endl;
     return 1;
     }
  }

qlist.clear();
for (int i=1; i<=10; i++)
  qlist << i;
ofstream of("qlist.tmp");
of << qlist;
of.close();
ucout << "Before IO: " << qlist << endl;
ifstream ifstr("qlist.tmp");
qlist.clear();
ifstr >> qlist;
ucout << "After IO: " << qlist << endl;

qlist.clear();
for (int i=1; i<=10; i++)
  qlist << i;
PackBuffer pack;
pack << qlist;
ucout << "Before IO: " << qlist << endl;
qlist.clear();
UnPackBuffer unpack(pack.buf(),pack.size());
unpack >> qlist;
ucout << "After IO: " << qlist << endl;

#if defined(UTILIB_HAVE_MPI)
uMPI::done();
#endif

utilib::exception_mngr::set_mode(utilib::exception_mngr::Standard);
return 0;
}
