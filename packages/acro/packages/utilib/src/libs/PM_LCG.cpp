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
// PM_LCG.cpp
//
// Implement the methods for LCG
//

#include <utilib_config.h>
#include <utilib/PM_LCG.h>
#include <utilib/seconds.h>
#include <utilib/_generic.h>
#include <utilib/_math.h>
 
extern "C" int PMrand(int*);

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif

namespace utilib {

//
// The globally available random number generate that is used as a default
//
namespace {
PM_LCG default_PM_LCG;
}
RNG& default_rng = default_PM_LCG;



//
//
// Methods for LCG
//
//
unsigned long PM_LCG::asLong()
{
////
//// The first use of &state is not changed here
///
unsigned long temp = (unsigned long) PMrand(&state);
return(temp);
}


double PM_LCG::asDouble()
{
double ans = asLong()/ (double)(2147483647);
return ans;
}


void PM_LCG::reset()
{
if (jseed == 0) // generate a random seed
   jseed = abs(std::max((int) CurrentTime(), 1));
state = jseed;
}


#ifndef UTILIB_HAVE_NAMESPACES
void PM_LCG::write(ostream& os) const
#else
void PM_LCG::write(std::ostream& os) const
#endif
{
os << jseed << " ";
os << state << "\n";
}

#ifndef UTILIB_HAVE_NAMESPACES
void PM_LCG::read(istream& is)
#else
void PM_LCG::read(std::istream& is)
#endif
{
is >> jseed >> state;
}

} // namespace utilib

