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
// median-test.cpp
//
#include <utilib_config.h>
#include "nr.double.h"
#include "IntVector.h"

int test_median(int,char**)
{
DoubleVector vec(101);
IntVector ws(101);
LCG* rng = new LCG(100);

for (unsigned int i=0; i<10; i++) {
  int ans=0;
  for (unsigned int j=0; j<vec.size(); j++)
    vec[j] = j;
  shuffle(vec,rng);
  ans = argmedian(vec,vec.size(),ws);
  cout << "Problem:\n" << vec << endl << ans << " " << vec[ans] << endl;
  //if (vec[ans] != 49)
  //   cout << vec[ans] << endl;
  }
return 0;
}
