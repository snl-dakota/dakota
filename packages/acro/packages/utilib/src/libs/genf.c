/* This is third-party software that is distributed with UTILIB.
 * For licensing information concerning this file, see the UTILIB home page:
 * http://software.sandia.gov/trac/utilib
 */
/* genf.c
 *
 */
#ifdef HAVE_FSIGN

#include <utilib_config.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <utilib/Random.h>


double genf(double dfn,double dfd)
/*
**********************************************************************
     double genf(double dfn,double dfd)
                GENerate random deviate from the F distribution
                              Function
     Generates a random deviate from the F (variance ratio)
     distribution with DFN degrees of freedom in the numerator
     and DFD degrees of freedom in the denominator.
                              Arguments
     dfn --> Numerator degrees of freedom
             (Must be positive)
     dfd --> Denominator degrees of freedom
             (Must be positive)
                              Method
     Directly generates ratio of chisquare variates
**********************************************************************
*/
{
static double genf,xden,xnum;

    if(!(dfn <= 0.0 || dfd <= 0.0)) goto S10;
    puts("Degrees of freedom nonpositive in GENF - abort!");
    printf("DFN value: %16.6EDFD value: %16.6E\n",dfn,dfd);
    exit(1);
S10:
    xnum = genchi(dfn)/dfn;
/*
      GENF = ( GENCHI( DFN ) / DFN ) / ( GENCHI( DFD ) / DFD )
*/
    xden = genchi(dfd)/dfd;
    if(!(xden <= 9.999999999998E-39*xnum)) goto S20;
    puts(" GENF - generated numbers would cause overflow");
    printf(" Numerator %16.6E Denominator %16.6E\n",xnum,xden);
    puts(" GENF returning 1.0E38");
    genf = 1.0E38;
    goto S30;
S20:
    genf = xnum/xden;
S30:
    return genf;
}
#endif
