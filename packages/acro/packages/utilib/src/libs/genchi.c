/* This is third-party software that is distributed with UTILIB.
 * For licensing information concerning this file, see the UTILIB home page:
 * http://software.sandia.gov/trac/utilib
 */
/* genchi.c
 *
 */

#ifdef HAVE_FSIGN

#include <utilib_config.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <utilib/Random.h>


double genchi(double df)
/*
**********************************************************************
     double genchi(double df)
                Generate random value of CHIsquare variable
                              Function
     Generates random deviate from the distribution of a chisquare
     with DF degrees of freedom random variable.
                              Arguments
     df --> Degrees of freedom of the chisquare
            (Must be positive)

                              Method
     Uses relation between chisquare and gamma.
**********************************************************************
*/
{
static double genchi;

    if(!(df <= 0.0)) goto S10;
    if(!(df <= 0.0)) goto S10;
    puts("DF <= 0 in GENCHI - ABORT");
    printf("Value of DF: %16.6E\n",df);
    exit(1);
S10:
    genchi = 2.0*gengam(1.0,df/2.0);
    return genchi;
}
#endif
