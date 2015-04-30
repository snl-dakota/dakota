/* This is third-party software that is distributed with UTILIB.
 * For licensing information concerning this file, see the UTILIB home page:
 * http://software.sandia.gov/trac/utilib
 */
/* gennor.c
 *
 */

#include <utilib_config.h>
#include <math.h>
#include <utilib/Random.h>


double gennor(double av,double sd)
/*
**********************************************************************
     double gennor(double av,double sd)
         GENerate random deviate from a NORmal distribution
                              Function
     Generates a single random deviate from a normal distribution
     with mean, AV, and standard deviation, SD.
                              Arguments
     av --> Mean of the normal distribution.
     sd --> Standard deviation of the normal distribution.
**********************************************************************
*/
{
static double gennor;

    gennor = sd*snorm1()+av;
    return gennor;
}

