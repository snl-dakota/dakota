/* This is third-party software that is distributed with UTILIB.
 * For licensing information concerning this file, see the UTILIB home page:
 * http://software.sandia.gov/trac/utilib
 */
/* genexp.c
 *
 */

#include <utilib_config.h>
#include <math.h>
#include <utilib/Random.h>


double genexp(double av)
/*
**********************************************************************
     double genexp(double av)
                    GENerate EXPonential random deviate
                              Function
     Generates a single random deviate from an exponential
     distribution with mean AV.
                              Arguments
     av --> The mean of the exponential distribution from which
            a random deviate is to be generated.
                              Method
     Renames SEXPO from TOMS as slightly modified by BWB to use RANF
     instead of SUNIF.
     For details see:
               Ahrens, J.H. and Dieter, U.
               Computer Methods for Sampling From the
               Exponential and Normal Distributions.
               Comm. ACM, 15,10 (Oct. 1972), 873 - 882.
**********************************************************************
*/
{
static double genexp;

    genexp = sexpo()*av;
    return genexp;
}

