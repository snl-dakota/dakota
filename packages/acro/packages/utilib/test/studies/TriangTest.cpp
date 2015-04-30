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
#include <utilib/LCG.h>
#include <utilib/Triangular.h>
#include <utilib/Triang_bal.h>
#include <utilib/Triang_trunc.h>

using utilib::LCG;
using utilib::Triang_bal;
using utilib::Triang_trunc;
using utilib::Triangular;
using utilib::Triang;

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif

int test_triang2()
{
  double upr=1.0,lwr=0.0,delta,x=0.8;
  double randvar,dist[101],mom=0.0;
  int i,rvar,maxdist=0;

  //  Triang_bal *tb;
  //  tb = new Triang_bal(new LCG(100));
  //  delta = 0.4;
  //  tb->set_base(delta,lwr,upr,x);

  Triang_trunc *tb;
  tb = new Triang_trunc(new LCG(100));
  delta = 0.4;
  tb->set_base(delta,lwr,upr,x);

  //  Triang *tb;
  //  tb = new Triangular(new LCG(100));
  //  delta=0.2;

  for(i=0; i<=100; i++)
    dist[i]=0.0;

  for(i=0; i<1000000; i++) {
    randvar=(*tb)();
    rvar=floor((x+delta*randvar)*100.0);
    dist[rvar]=dist[rvar]+1.0;
    if(dist[rvar]>(double)maxdist)
      maxdist++;
  }

  
  for(i=0; i<=100; i++) {
    dist[i]=dist[i]/(double)maxdist;
    ucout << i << " " << dist[i] << "\n";
    mom=mom+dist[i]*((double)i/100.0-0.8);
  }
  ucout << "expected step = " << mom << "\n";

}
