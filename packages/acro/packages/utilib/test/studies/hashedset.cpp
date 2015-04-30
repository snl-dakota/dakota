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
#include <utilib/HashedSet.h>
#include <utilib/BasicArray.h>

using namespace utilib;


int test_hashedset(int,char**)
{
BasicArray<int> tmp(3);
tmp << 0;
HashedSet<BasicArray<int> > set1;

int ndx=0;
for (int i=0; i<10; i++) {
  tmp[ndx]++;
  ndx = (ndx+1) % 3;
  set1.insert(tmp);
  }

HashedSet<BasicArray<int> >::iterator curr = set1.begin();
HashedSet<BasicArray<int> >::iterator end  = set1.end();
while (curr != end) {
  std::cout << (*curr) << std::endl;
  curr++;
  };

std::cout << set1;
PackBuffer pack;
pack << set1;

#if 0
UnPackBuffer unpack(pack);
HashedSet<BasicArray<int> > set1a;
unpack >> set1a;
std::cout << set1a;
#endif

HashedSet<BasicArray<int>,true> set2;

return 0;
}
