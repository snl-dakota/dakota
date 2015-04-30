/* This is third-party software that is distributed with UTILIB.
 * For licensing information concerning this file, see the UTILIB home page:
 * http://software.sandia.gov/trac/utilib
 */
/*
 * genprm.c
 *
 */

#ifdef HAVE_IGNUIN 

#include <utilib_config.h>
#include <math.h>
#include <utilib/Random.h>

void genprm(long *iarray,int larray);

void genprm(long *iarray,int larray)
/*
**********************************************************************
    void genprm(long *iarray,int larray)
               GENerate random PeRMutation of iarray
                              Arguments
     iarray <--> On output IARRAY is a random permutation of its
                 value on input
     larray <--> Length of IARRAY
**********************************************************************
*/
{
static long i,itmp,iwhich,D1,D2;

    for(i=1,D1=1,D2=(larray-i+D1)/D1; D2>0; D2--,i+=D1) {
        iwhich = ignuin(i,larray);
        itmp = *(iarray+iwhich-1);
        *(iarray+iwhich-1) = *(iarray+i-1);
        *(iarray+i-1) = itmp;
    }
}

#endif
