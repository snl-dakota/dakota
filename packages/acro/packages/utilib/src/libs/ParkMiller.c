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

/**
 * \file	ParkMiller.c
 *
 * Implementation of the \c PMrand function
 *
 * \author	William Hart
 * \date	Dec, 1999
 */

#include <utilib_config.h>
#include <math.h>

int PMrand(int* state);

/** */
#define MPLIER 16807
/** */
#define MODLUS 2147483647
/** */
#define MOBYMP 127773
/** */
#define MOMDMP 2836

/**
 * This function returns a pseudo-random number for each invocation.
 * This C implementation, is adapted from a FORTRAN 77 adaptation of 
 * the "Integer Version 2" minimal standard number generator whose 
 * Pascal code described by 
 * \if GeneratingLaTeX Park and Miller~\cite{ParMil88}. \endif
 * \if GeneratingHTML [\ref ParMil88 "ParMil88"]. \endif
 *
 * This code is portable to any machine that has a
 * maximum integer greater than, or equal to, 2**31-1.  Thus, this code
 * should run on any 32-bit machine.  The code meets all of the
 * requirements of the "minimal standard" as described by
 * \if GeneratingLaTeX Park and Miller~\cite{ParMil88}. \endif
 * \if GeneratingHTML [\ref ParMil88 "ParMil88"]. \endif
 * Park and Miller note that the execution times for running the portable 
 * code were 2 to 50 times slower than the random number generator supplied 
 * by the vendor. 
 */
int PMrand(int* state)
{
register int hvlue, lvlue, testv;
int nextn = *state;

hvlue = nextn / MOBYMP;
lvlue = nextn % MOBYMP;			       /* nextn MOD MOBYMP */
testv = MPLIER*lvlue - MOMDMP*hvlue;
if (testv > 0) 
   nextn = testv;
else
   nextn = testv + MODLUS;
*state = nextn;
return nextn;
}

