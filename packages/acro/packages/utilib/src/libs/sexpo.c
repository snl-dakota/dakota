/* This is third-party software that is distributed with UTILIB.
 * For licensing information concerning this file, see the UTILIB home page:
 * http://software.sandia.gov/trac/utilib
 */

/* sexpo.c
 *
 */

#include <utilib_config.h>
#include <math.h>
#include <utilib/Random.h>


double sexpo()
/*
**********************************************************************


     (STANDARD-)  E X P O N E N T I A L   DISTRIBUTION


**********************************************************************
**********************************************************************

     FOR DETAILS SEE:

               AHRENS, J.H. AND DIETER, U.
               COMPUTER METHODS FOR SAMPLING FROM THE
               EXPONENTIAL AND NORMAL DISTRIBUTIONS.
               COMM. ACM, 15,10 (OCT. 1972), 873 - 882.

     ALL STATEMENT NUMBERS CORRESPOND TO THE STEPS OF ALGORITHM
     'SA' IN THE ABOVE PAPER (SLIGHTLY MODIFIED IMPLEMENTATION)

     Modified by Barry W. Brown, Feb 3, 1988 to use RANF instead of
     SUNIF.  The argument IR thus goes away.

**********************************************************************
     Q(N) = SUM(ALOG(2.0)**K/K!)    K=1,..,N ,      THE HIGHEST N
     (HERE 8) IS DETERMINED BY Q(N)=1.0 WITHIN STANDARD PRECISION
*/
{
static double q[8] = {
    0.6931472,0.9333737,0.9888778,0.9984959,0.9998293,0.9999833,0.9999986,1.0
};
static long i;
static double sexpo,a,u,ustar,umin;
static double *q1 = q;
    a = 0.0;
    u = ranf();
    goto S30;
S20:
    a += *q1;
S30:
    u += u;
    if(u <= 1.0) goto S20;
    u -= 1.0;
    if(u > *q1) goto S60;
    sexpo = a+u;
    return sexpo;
S60:
    i = 1;
    ustar = ranf();
    umin = ustar;
S70:
    ustar = ranf();
    if(ustar < umin) umin = ustar;
    i += 1;
    if(u > *(q+i-1)) goto S70;
    sexpo = a+umin**q1;
    return sexpo;
}

