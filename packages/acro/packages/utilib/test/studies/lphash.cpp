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
#include <utilib/LPHashTable.h>
#include <utilib/stl_auxiliary.h>
#include <utilib/BasicArray.h>

using namespace std;
using namespace utilib;

void test()
{
LPHashTable<BasicArray<double>, double > htable;
BasicArray<double> foo(3);
foo[0] = 1.0;
foo[1] = 2.0;
foo[2] = 3.0;
double value = 4.0;
htable.add(foo, value);

if (htable.find(foo) == htable.end())
   cerr << "THIS IS AN ERROR!" << endl;

LPHashTable<BasicArray<double>, double>::iterator item = htable.find(foo);
cout << "Value: " << item->info() << endl;
}

int test_lphash(int,char**)
{
test();

utilib::LPHashTable< BasicArray<double>, double > ht;

BasicArray<double> tmp(3);
tmp << 1.0;

int i=0;
int ndx=0;

while (ndx < 100) {
  tmp[i] /= 10.0;
  double val = ndx;
  cout << val << " ";
  cout << tmp.size() << " : ";	// pulled from BasicArray.h
  cout << " " << pscientific(tmp[0],2,3);
  cout << " " << pscientific(tmp[1],2,3);
  cout << " " << pscientific(tmp[2],2,3);
  cout << endl;
  ht.add(tmp,val);
  i = (i+1) % 3;
  ndx++;
  }

BasicArray<BasicArray<double> > data(21);
LPHashTable<BasicArray<double>, double>::iterator curr = ht.begin();
LPHashTable<BasicArray<double>, double>::iterator end  = ht.end();
while (curr != end) {
  data[ static_cast<int>(curr->info())] = curr->key();
  curr++;
  }

for (size_type i=0; i<data.size(); i++) {
  cout << i << "  ";
  cout << data[i].size() << " :";
  cout << " " << pscientific(data[i][0],2,3);
  cout << " " << pscientific(data[i][1],2,3);
  cout << " " << pscientific(data[i][2],2,3);
  cout << endl;
}

return 0;
}
