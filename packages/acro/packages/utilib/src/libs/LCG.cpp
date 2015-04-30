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
// LCG.cpp
//
// Implement the methods for LCG
//


#include <utilib_config.h>

#if UTILIB_TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else            
#if UTILIB_HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#include <cstdlib>

#ifdef __SUNPRO_CC
// for nrand48, which isn't in cstdlib on Solaris for some reason
#include <stdlib.h>
#endif

#include <utilib/_generic.h>
#include <utilib/LCG.h>
 
using namespace std;

namespace utilib {

//
// The globally available random number generate that is used as a default
//
// LCG default_rng;




//
//
// Methods for LCG
//
//


unsigned long LCG::asLong()
{
#ifndef UTILIB_HAVE_NRAND48
unsigned long temp = (unsigned long) 0.0;
double foo=0.0,tmp = 1.0/foo;
#else
nrand48(State);
unsigned long temp = (unsigned long) nrand48(State);
#endif
return(temp);
}



void LCG::reset()
{ 
/* Use _ftime _timeb stuff here
struct timeval tv;
struct timezone tz;

if (Seed == 0) {
#ifdef UTILIB_HAVE_GETTIMEOFDAY
   gettimeofday(&tv,&tz);
#else
#error "Modify LCG::reset() to use another time function"
#endif
   State[0] = (unsigned short) tv.tv_usec;
   }
else {
   State[0] = (unsigned short) Seed;
   }
*/

State[1] = 0;
State[2] = 0;
}


void LCG::reseed(unsigned seed)
{
Seed = seed;
State[0] = State[1] = State[2] = 0;
reset();
}


LCG::LCG(unsigned short seed)
{ reseed(seed); }

#ifndef UTILIB_HAVE_NAMESPACES
void LCG::write(ostream& os) const
#else
void LCG::write(std::ostream& os) const
#endif
{
os << Seed << "\n";
os << State[0] << " " << State[1] << " " << State[2] << "\n";
}

#ifndef UTILIB_HAVE_NAMESPACES
void LCG::read(istream& is)
#else
void LCG::read(std::istream& is)
#endif
{
is >> Seed;
is >> State[0] >> State[1] >> State[2];
}

} // namespace utilib

