/* This is third-party software that is distributed with UTILIB.
 * For licensing information concerning this file, see the UTILIB home page:
 * http://software.sandia.gov/trac/utilib
 */

/* genunf.c
 *
 */

#include <utilib_config.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <utilib/Random.h>


double genunf(double low,double high)
/*
**********************************************************************
     double genunf(double low,double high)
               GeNerate Uniform Real between LOW and HIGH
                              Function
     Generates a real uniformly distributed between LOW and HIGH.
                              Arguments
     low --> Low bound (exclusive) on real value to be generated
     high --> High bound (exclusive) on real value to be generated
**********************************************************************
*/
{
static double genunf;

    if(!(low > high)) goto S10;
    printf("LOW > HIGH in GENUNF: LOW %16.6E HIGH: %16.6E\n",low,high);
    puts("Abort");
    exit(1);
S10:
    genunf = low+(high-low)*ranf();
    return genunf;
}

