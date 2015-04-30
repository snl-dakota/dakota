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
// Globals.C
//
// o  Stuff for making the extern*Rand* stuff work
// o  Stream operators for RNG classes
//

#include <utilib_config.h>
#include <utilib/Uniform.h>
#include <utilib/Random.h>
#include <utilib/AnyRNG.h>
#include <utilib/default_rng.h>

using namespace std;

utilib::Uniform local_runif(&utilib::default_rng);

extern "C" double global_Uniform_func(void)
{
return local_runif();
}

//
// Note: the global_runif function used to rely on the LEC routines that 
// came with the ranlib routine.  However, to standardize, we now use
// the Uniform class and the default random number generator, which uses the
// PM_LCG code.
//
//extern "C" double (*global_runif)(void) = LEC_runif;
extern "C" {
double (*global_runif)(void) = global_Uniform_func;
};


namespace utilib {

void set_global_RNG(AnyRNG* rng)
{
local_runif.generator(rng);
set_global_runif(global_Uniform_func);
}


} // namespace utilib
