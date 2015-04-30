/* This is third-party software that is distributed with UTILIB.
 * For licensing information concerning this file, see the UTILIB home page:
 * http://software.sandia.gov/trac/utilib
 */
/* gengam.c
 *
 */
#ifdef HAVE_FSIGN
#include <utilib_config.h>
#include <math.h>
#include <utilib/Random.h>


double gengam(double a,double r)
/*
**********************************************************************
     double gengam(double a,double r)
           GENerates random deviates from GAMma distribution
                              Function
     Generates random deviates from the gamma distribution whose
     density is
          (A**R)/Gamma(R) * X**(R-1) * Exp(-A*X)
                              Arguments
     a --> Location parameter of Gamma distribution
     r --> Shape parameter of Gamma distribution
                              Method
     Renames SGAMMA from TOMS as slightly modified by BWB to use RANF
     instead of SUNIF.
     For details see:
               (Case R >= 1.0)
               Ahrens, J.H. and Dieter, U.
               Generating Gamma Variates by a
               Modified Rejection Technique.
               Comm. ACM, 25,1 (Jan. 1982), 47 - 54.
     Algorithm GD
               (Case 0.0 <= R <= 1.0)
               Ahrens, J.H. and Dieter, U.
               Computer Methods for Sampling from Gamma,
               Beta, Poisson and Binomial Distributions.
               Computing, 12 (1974), 223-246/
     Adapted algorithm GS.
**********************************************************************
*/
{
static double gengam;

    gengam = sgamma(r);
    gengam /= a;
    return gengam;
}
#endif
