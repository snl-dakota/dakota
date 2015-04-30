/* This is third-party software that is distributed with UTILIB.
 * For licensing information concerning this file, see the UTILIB home page:
 * http://software.sandia.gov/trac/utilib
 */

/* LEC_seed.c
 *
 */

#include <utilib_config.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "LEC_rng.h"


long Xm1,Xm2,Xa1,Xa2,Xcg1[32],Xcg2[32],Xa1w,Xa2w,Xig1[32],Xig2[32],Xlg1[32],
    Xlg2[32],Xa1vw,Xa2vw;
long Xqanti[32];

void setall(long iseed1,long iseed2)
/*
**********************************************************************
     void setall(long iseed1,long iseed2)
               SET ALL random number generators
     Sets the initial seed of generator 1 to ISEED1 and ISEED2. The
     initial seeds of the other generators are set accordingly, and
     all generators states are set to these seeds.
     This is a transcription from Pascal to Fortran of routine
     Set_Initial_Seed from the paper
     L'Ecuyer, P. and Cote, S. "Implementing a Random Number Package
     with Splitting Facilities." ACM Transactions on Mathematical
     Software, 17:98-111 (1991)
                              Arguments
     iseed1 -> First of two integer seeds
     iseed2 -> Second of two integer seeds
**********************************************************************
*/
{
#define numg 32L
extern void gsrgs(long getset,long *qvalue);
extern void gssst(long getset,long *qset);
extern void gscgn(long getset,long *g);
extern void inrgcm(void);
extern long Xm1,Xm2,Xa1vw,Xa2vw,Xig1[],Xig2[];
static long T1;
static long g,ocgn;
static long qrgnin;
    T1 = 1;
/*
     TELL IGNLGI, THE ACTUAL NUMBER GENERATOR, THAT THIS ROUTINE
      HAS BEEN CALLED.
*/
    gssst(1,&T1);
    gscgn(0L,&ocgn);
/*
     Initialize Common Block if Necessary
*/
    gsrgs(0L,&qrgnin);
    if(!qrgnin) inrgcm();
    *Xig1 = iseed1;
    *Xig2 = iseed2;
    initgn(-1L);
    for(g=2; g<=numg; g++) {
        *(Xig1+g-1) = mltmod(Xa1vw,*(Xig1+g-2),Xm1);
        *(Xig2+g-1) = mltmod(Xa2vw,*(Xig2+g-2),Xm2);
        gscgn(1L,&g);
        initgn(-1L);
    }
    gscgn(1L,&ocgn);
#undef numg
}


void setant(long qvalue)
/*
**********************************************************************
     void setant(long qvalue)
               SET ANTithetic
     Sets whether the current generator produces antithetic values.  If
     X   is  the value  normally returned  from  a uniform [0,1] random
     number generator then 1  - X is the antithetic  value. If X is the
     value  normally  returned  from a   uniform  [0,N]  random  number
     generator then N - 1 - X is the antithetic value.
     All generators are initialized to NOT generate antithetic values.
     This is a transcription from Pascal to Fortran of routine
     Set_Antithetic from the paper
     L'Ecuyer, P. and Cote, S. "Implementing a Random Number Package
     with Splitting Facilities." ACM Transactions on Mathematical
     Software, 17:98-111 (1991)
                              Arguments
     qvalue -> nonzero if generator G is to generating antithetic
                    values, otherwise zero
**********************************************************************
*/
{
#define numg 32L
extern void gsrgs(long getset,long *qvalue);
extern void gscgn(long getset,long *g);
extern long Xqanti[];
static long g;
static long qrgnin;
/*
     Abort unless random number generator initialized
*/
    gsrgs(0L,&qrgnin);
    if(qrgnin) goto S10;
    printf(
      " SETANT called before random number generator  initialized -- abort!\n");    exit(1);
S10:
    gscgn(0L,&g);
    Xqanti[g-1] = qvalue;
#undef numg
}




void setsd(long iseed1,long iseed2)
/*
**********************************************************************
     void setsd(long iseed1,long iseed2)
               SET S-ee-D of current generator
     Resets the initial  seed of  the current  generator to  ISEED1 and
     ISEED2. The seeds of the other generators remain unchanged.
     This is a transcription from Pascal to Fortran of routine
     Set_Seed from the paper
     L'Ecuyer, P. and Cote, S. "Implementing a Random Number Package
     with Splitting Facilities." ACM Transactions on Mathematical
     Software, 17:98-111 (1991)
                              Arguments
     iseed1 -> First integer seed
     iseed2 -> Second integer seed
**********************************************************************
*/
{
#define numg 32L
extern void gsrgs(long getset,long *qvalue);
extern void gscgn(long getset,long *g);
extern long Xig1[],Xig2[];
static long g;
static long qrgnin;
/*
     Abort unless random number generator initialized
*/
    gsrgs(0L,&qrgnin);
    if(qrgnin) goto S10;
    printf(
      " SETSD called before random number generator  initialized -- abort!\n");
    exit(1);
S10:
    gscgn(0L,&g);
    *(Xig1+g-1) = iseed1;
    *(Xig2+g-1) = iseed2;
    initgn(-1L);
#undef numg
}



void getsd(long *iseed1,long *iseed2)
/*
**********************************************************************
     void getsd(long *iseed1,long *iseed2)
               GET SeeD
     Returns the value of two integer seeds of the current generator
     This  is   a  transcription from  Pascal   to  Fortran  of routine
     Get_State from the paper
     L'Ecuyer, P. and  Cote,  S. "Implementing a Random Number  Package
     with   Splitting Facilities."  ACM  Transactions   on Mathematical
     Software, 17:98-111 (1991)
                              Arguments
     iseed1 <- First integer seed of generator G
     iseed2 <- Second integer seed of generator G
**********************************************************************
*/
{
#define numg 32L
extern void gsrgs(long getset,long *qvalue);
extern void gscgn(long getset,long *g);
extern long Xcg1[],Xcg2[];
static long g;
static long qrgnin;
/*
     Abort unless random number generator initialized
*/
    gsrgs(0L,&qrgnin);
    if(qrgnin) goto S10;
    printf(
      " GETSD called before random number generator  initialized -- abort!\n");
    exit(0);
S10:
    gscgn(0L,&g);
    *iseed1 = *(Xcg1+g-1);
    *iseed2 = *(Xcg2+g-1);
#undef numg
}

